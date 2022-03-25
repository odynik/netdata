//Includes
#include "rrdpush.h"
#include "collectors/plugins.d/pluginsd_parser.h"

extern struct config stream_config;
extern int netdata_use_ssl_on_stream;

size_t replication_parser(struct replication_state *rpt, struct plugind *cd, FILE *fp);

static void replication_receiver_thread_cleanup_callback(void *host);
static void replication_sender_thread_cleanup_callback(void *ptr);
static void print_replication_state(REPLICATION_STATE *state);
static void print_replication_gap(GAP *a_gap);
int save_gap(GAP *a_gap);
int remove_gap(GAP *a_gap);
int load_gap(RRDHOST *host);
void gaps_init(RRDHOST *host);

// Thread Initialization
static void replication_state_init(REPLICATION_STATE *state)
{
    info("%s: REPlication State Initilization", REPLICATION_MSG);
    memset(state, 0, sizeof(*state));
    state->buffer = cbuffer_new(1024, 1024*1024);
    state->build = buffer_create(1);
    state->socket = -1;
#ifdef ENABLE_HTTPS
    state->ssl = (struct netdata_ssl *)callocz(1, sizeof(struct netdata_ssl));
#endif
    netdata_mutex_init(&state->mutex);
}

static void replication_state_destroy(REPLICATION_STATE *state)
{
    info("%s: Destroying replication state.", REPLICATION_MSG);
    pthread_mutex_destroy(&state->mutex);
    cbuffer_free(state->buffer);
    buffer_free(state->build);
    // freez(state->read_buffer);
#ifdef ENABLE_HTTPS
    if(state->ssl->conn){
        SSL_free(state->ssl->conn);
    }
    freez(state->ssl);
#endif
    freez(state);
}

void replication_sender_init(struct sender_state *sender){
    if(!default_rrdpush_replication_enabled)
        return;
    if(!sender || !sender->host){
        error("%s: Host or host's replication sender state is not initialized! - Tx thread Initialization failed!", REPLICATION_MSG);
        return;
    }

    sender->replication = (REPLICATION_STATE *)callocz(1, sizeof(REPLICATION_STATE));
    replication_state_init(sender->replication);
    sender->replication->host = sender->host;
    sender->replication->enabled = default_rrdpush_replication_enabled;
#ifdef ENABLE_HTTPS
    sender->replication->ssl = &sender->host->stream_ssl;
#endif
    info("%s: Initialize REP Tx state during host creation %s .", REPLICATION_MSG, sender->host->hostname);
    print_replication_state(sender->replication);
}

static unsigned int replication_rd_config(struct receiver_state *rpt, struct config *stream_config)
{
    info("%s: Reading config Rx for host %s ", REPLICATION_MSG, rpt->host->hostname);
    if(!default_rrdpush_replication_enabled)
        return default_rrdpush_replication_enabled;
    unsigned int rrdpush_replication_enable = default_rrdpush_replication_enabled;
    rrdpush_replication_enable = appconfig_get_boolean(stream_config, rpt->key, "enable replication", rrdpush_replication_enable);
    rrdpush_replication_enable = appconfig_get_boolean(stream_config, rpt->machine_guid, "enable replication", rrdpush_replication_enable);
    // Runtime replication enable status
    rrdpush_replication_enable = (default_rrdpush_replication_enabled && rrdpush_replication_enable && (rpt->stream_version >= VERSION_GAP_FILLING));
    
    info("%s: Configuration applied %u ", REPLICATION_MSG, rrdpush_replication_enable);
    return rrdpush_replication_enable;
}

void replication_receiver_init(struct receiver_state *receiver, struct config *stream_config)
{
    unsigned int rrdpush_replication_enable = replication_rd_config(receiver, stream_config);
    if(!rrdpush_replication_enable)
    {
        info("%s:  Could not initialize Rx replication thread. Replication is disabled or not supported!", REPLICATION_MSG);
        return;
    }
    // receiver->replication = (REPLICATION_STATE *)callocz(1, sizeof(REPLICATION_STATE));
    replication_state_init(receiver->replication);
    info("%s: REP Rx state initialized", REPLICATION_MSG);    
    receiver->replication->host = receiver->host;
    receiver->replication->enabled = rrdpush_replication_enable;
    info("%s: Initialize Rx for host %s ", REPLICATION_MSG, receiver->host->hostname);
    print_replication_state(receiver->replication);
}

// Connection management & socket handling functions
//Close the socket of the replication sender thread
//Do we need seperating as sender and receiver ??
static void replication_sender_thread_close_socket(RRDHOST *host) {
    host->sender->replication->connected = 0;

    if(host->sender->replication->socket != -1) {
        close(host->sender->replication->socket);
        host->sender->replication->socket = -1;
    }
}

static void replication_thread_close_socket(REPLICATION_STATE *replication) {
    replication->connected = 0;

    if(replication->socket != -1) {
        close(replication->socket);
        replication->socket = -1;
    }
}

extern void rrdpush_encode_variable(stream_encoded_t *se, RRDHOST *host);
extern void rrdpush_clean_encoded(stream_encoded_t *se);

// Connect to parent. The REPLICATE command over HTTP req triggers the receiver thread in parent.
// TODO: revise the logic of the communication
static int replication_sender_thread_connect_to_parent(RRDHOST *host, int default_port, int timeout, struct sender_state *s) {

    struct timeval tv = {
            .tv_sec = timeout,
            .tv_usec = 0
    };

    // make sure the socket is closed
    replication_sender_thread_close_socket(host);

    debug(D_REPLICATION, "%s: Attempting to connect...", REPLICATION_MSG);
    info("%s %s [send to %s]: connecting...", REPLICATION_MSG, host->hostname, host->rrdpush_send_destination);

    host->sender->replication->socket = connect_to_one_of(
            host->rrdpush_send_destination
            , default_port
            , &tv
            , &s->replication->reconnects_counter
            , s->replication->connected_to
            , sizeof(s->replication->connected_to)-1
    );

    if(unlikely(host->sender->replication->socket == -1)) {
        error("%s %s [send to %s]: failed to connect", REPLICATION_MSG, host->hostname, host->rrdpush_send_destination);
        return 0;
    }

    info("%s %s [send to %s]: initializing communication...", REPLICATION_MSG, host->hostname, s->replication->connected_to);

#ifdef ENABLE_HTTPS
    if( netdata_client_ctx ){
        host->ssl.flags = NETDATA_SSL_START;
        if (!host->ssl.conn){
            host->ssl.conn = SSL_new(netdata_client_ctx);
            if(!host->ssl.conn){
                error("Failed to allocate SSL structure.");
                host->ssl.flags = NETDATA_SSL_NO_HANDSHAKE;
            }
        }
        else{
            SSL_clear(host->ssl.conn);
        }

        if (host->ssl.conn)
        {
            if (SSL_set_fd(host->ssl.conn, host->sender->replication->socket) != 1) {
                error("Failed to set the socket to the SSL on socket fd %d.", host->sender->replication->socket);
                host->ssl.flags = NETDATA_SSL_NO_HANDSHAKE;
            } else{
                host->ssl.flags = NETDATA_SSL_HANDSHAKE_COMPLETE;
            }
        }
    }
    else {
        host->ssl.flags = NETDATA_SSL_NO_HANDSHAKE;
    }
#endif

    stream_encoded_t se;
    rrdpush_encode_variable(&se, host);
    // Add here any extra information to transmit with the.
    char http[HTTP_HEADER_SIZE + 1];
    int eol = snprintfz(http, HTTP_HEADER_SIZE,
            "%s key=%s&hostname=%s&registry_hostname=%s&machine_guid=%s&update_every=%d&timezone=%s&abbrev_timezone=%s&utc_offset=%d&hops=%d&tags=%s&ver=%u"
                 "&NETDATA_PROTOCOL_VERSION=%s"
                 " HTTP/1.1\r\n"
                 "User-Agent: %s\r\n"
                 "Accept: */*\r\n\r\n"
                 , REPLICATE_CMD
                 , host->rrdpush_send_api_key
                 , host->hostname
                 , host->registry_hostname
                 , host->machine_guid
                 , default_rrd_update_every
                 , host->timezone
                 , host->abbrev_timezone
                 , host->utc_offset
                 , host->system_info->hops + 1
                 , (host->tags) ? host->tags : ""
                 , STREAMING_PROTOCOL_CURRENT_VERSION
                 , host->program_version
                 , host->program_name);
    http[eol] = 0x00;
    rrdpush_clean_encoded(&se);


#ifdef ENABLE_HTTPS
    if (!host->ssl.flags) {
        ERR_clear_error();
        SSL_set_connect_state(host->ssl.conn);
        int err = SSL_connect(host->ssl.conn);
        if (err != 1){
            err = SSL_get_error(host->ssl.conn, err);
            error("SSL cannot connect with the server:  %s ",ERR_error_string((long)SSL_get_error(host->ssl.conn,err),NULL));
            if (netdata_use_ssl_on_stream == NETDATA_SSL_FORCE) {
                replication_sender_thread_close_socket(host);
                return 0;
            }else {
                host->ssl.flags = NETDATA_SSL_NO_HANDSHAKE;
            }
        }
        else {
            if (netdata_use_ssl_on_stream == NETDATA_SSL_FORCE) {
                if (netdata_validate_server == NETDATA_SSL_VALID_CERTIFICATE) {
                    if ( security_test_certificate(host->ssl.conn)) {
                        error("Closing the replication stream connection, because the server SSL certificate is not valid.");
                        replication_sender_thread_close_socket(host);
                        return 0;
                    }
                }
            }
        }
    }
    if(send_timeout(&host->ssl, host->sender->replication->socket, http, strlen(http), 0, timeout) == -1) {
#else
    if(send_timeout(host->sender->replication->socket, http, strlen(http), 0, timeout) == -1) {
#endif
        error("%s %s [send to %s]: failed to send HTTP header to remote netdata.", REPLICATION_MSG, host->hostname, s->replication->connected_to);
        replication_sender_thread_close_socket(host);
        return 0;
    }

    info("%s %s [send to %s]: waiting response from remote netdata...", REPLICATION_MSG, host->hostname, s->replication->connected_to);

    ssize_t received;
#ifdef ENABLE_HTTPS
    received = recv_timeout(&host->ssl,host->sender->replication->socket, http, HTTP_HEADER_SIZE, 0, timeout);
    if(received == -1) {
#else
    received = recv_timeout(host->sender->replication->socket, http, HTTP_HEADER_SIZE, 0, timeout);
    if(received == -1) {
#endif
        error("%s %s [send to %s]: remote netdata does not respond.", REPLICATION_MSG, host->hostname, s->replication->connected_to);
        replication_sender_thread_close_socket(host);
        return 0;
    }

    http[received] = '\0';
    // debug(D_REPLICATION, "Response to sender from far end: %s", http);
    info("%s: Response to sender from far end: %s", REPLICATION_MSG, http);
    print_replication_state(s->replication);
    // REP ack should be in the beggining of the response
    // TODO: Verify the final commands (strings, numbers???) - a simple parser function can be used here.
    if(unlikely(memcmp(http, REP_ACK_CMD, (size_t)strlen(REP_ACK_CMD)))) {
        error("%s %s [send to %s]: server is not replying properly (is it a netdata?).", REPLICATION_MSG, host->hostname, s->connected_to);
        replication_sender_thread_close_socket(host);
        return 0;
    }
    s->replication->connected = 1;
    // END of REP ack checking.

    info("%s %s [send to %s]: established replication communication with a parent using protocol version %d - ready to replicate metrics..."
         , REPLICATION_MSG
         , host->hostname
         , s->replication->connected_to
         , s->version);

    if(sock_setnonblock(host->sender->replication->socket) < 0)
        error("%s %s [send to %s]: cannot set non-blocking mode for socket.", REPLICATION_MSG, host->hostname, s->replication->connected_to);

    // TODO: Check the linux manual for sock tcp enlarge. If I remember well this does nothing. The SE Linux provides a file that needs priviledged access to modify this variable.
    if(sock_enlarge_out(host->sender->replication->socket) < 0)
        error("%s %s [send to %s]: cannot enlarge the socket buffer.", REPLICATION_MSG, host->hostname, s->replication->connected_to);

    debug(D_REPLICATION, "%s: Connected on fd %d...", REPLICATION_MSG, host->sender->replication->socket);

    return 1;
}

static void replication_sender_thread_data_flush(RRDHOST *host) {
    netdata_mutex_lock(&host->sender->replication->mutex);

    size_t len = cbuffer_next_unsafe(host->sender->replication->buffer, NULL);
    if (len)
        error("%s %s [send]: discarding %zu bytes of metrics already in the buffer.", REPLICATION_MSG, host->hostname, len);

    cbuffer_remove_unsafe(host->sender->replication->buffer, len);
    netdata_mutex_unlock(&host->sender->replication->mutex);
}

// Higher level wrap function for connection management and socket metadata updates.
static void replication_attempt_to_connect(struct sender_state *state)
{
    state->replication->send_attempts = 0;

    if(replication_sender_thread_connect_to_parent(state->host, state->default_port, state->timeout, state)) {
        state->replication->last_sent_t = now_monotonic_sec();

        // reset the buffer, to properly gaps and replicate commands
        replication_sender_thread_data_flush(state->host);

        // send from the beginning
        state->replication->begin = 0;

        // make sure the next reconnection will be immediate
        state->replication->not_connected_loops = 0;

        // reset the bytes we have sent for this session
        state->replication->sent_bytes_on_this_connection = 0;

        // Update the connection state flag
        state->replication->connected = 1;

        // Set file pointer
        state->replication->fp = fdopen(state->replication->socket, "r");
        if(!state->replication->fp) {
            log_stream_connection(state->replication->client_ip, state->replication->client_port, state->host->rrdpush_send_api_key, state->host->machine_guid, state->host->hostname, "SOCKET CONVERSION TO FD FAILED - SOCKET ERROR");
            error("%s %s [receive from [%s]:%s]: failed to get a FILE for FD %d.", REPLICATION_MSG, state->host->hostname, state->replication->client_ip, state->replication->client_port, state->replication->socket);
            close(state->replication->socket);
            fclose(state->replication->fp);
            // return 0;
        } 
    }
    else {
        // increase the failed connections counter
        state->replication->not_connected_loops++;

        // reset the number of bytes sent
        state->replication->sent_bytes_on_this_connection = 0;

        // slow re-connection on repeating errors
        sleep_usec(USEC_PER_SEC * state->replication->reconnect_delay); // seconds
    }
}

// Replication thread starting a transmission
void replication_start(struct replication_state *replication) {
    netdata_mutex_lock(&replication->mutex);
    buffer_flush(replication->build);
}

// Replication thread finishing a transmission
void replication_commit(struct replication_state *replication) {
    if(cbuffer_add_unsafe(replication->buffer, buffer_tostring(replication->build),
       replication->build->len))
        replication->overflow = 1;
    buffer_flush(replication->build);
    netdata_mutex_unlock(&replication->mutex);
}

void replication_attempt_read(struct replication_state *replication) {
int ret;
#ifdef ENABLE_HTTPS
    if (replication->ssl->conn && !replication->ssl->flags) {
        ERR_clear_error();
        int desired = sizeof(replication->read_buffer) - replication->read_len - 1;
        ret = SSL_read(replication->ssl->conn, replication->read_buffer, desired);
        if (ret > 0 ) {
            replication->read_len += ret;
            return;
        }
        int sslerrno = SSL_get_error(replication->ssl->conn, desired);
        if (sslerrno == SSL_ERROR_WANT_READ || sslerrno == SSL_ERROR_WANT_WRITE)
            return;
        u_long err;
        char buf[256];
        while ((err = ERR_get_error()) != 0) {
            ERR_error_string_n(err, buf, sizeof(buf));
            error("REPLICATION %s [send to %s] ssl error: %s", replication->host->hostname, replication->connected_to, buf);
        }
        error("Restarting connection");
        replication_thread_close_socket(replication);
        return;
    }
#endif
    ret = recv(replication->socket, replication->read_buffer + replication->read_len, sizeof(replication->read_buffer) - replication->read_len - 1,
               MSG_DONTWAIT);
    
    if (ret>0) {
        replication->read_len += ret;
        return;
    }
    debug(D_REPLICATION, "Socket was POLLIN, but req %zu bytes gave %d", sizeof(replication->read_buffer) - replication->read_len - 1, ret);
    
    if (ret<0 && (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR))
        return;
    if (ret==0)
        error("REPLICATION %s [send to %s]: connection closed by far end. Restarting connection", "s->host->hostname???", replication->connected_to);
    else
        error("REPLICATION %s [send to %s]: error during read (%d). Restarting connection", "s->host->hostname???", replication->connected_to,
              ret);
    replication_thread_close_socket(replication);
}

// TCP window is open and we have data to transmit.
void replication_attempt_to_send(struct replication_state *replication) {

#ifdef NETDATA_INTERNAL_CHECKS
    struct circular_buffer *cb = replication->buffer;
#endif

    netdata_thread_disable_cancelability();
    netdata_mutex_lock(&replication->mutex);
    char *chunk;
    size_t outstanding = cbuffer_next_unsafe(replication->buffer, &chunk);
    debug(D_REPLICATION, "REPLICATION: Sending data. Buffer r=%zu w=%zu s=%zu, next chunk=%zu", cb->read, cb->write, cb->size, outstanding);
    ssize_t ret;
#ifdef ENABLE_HTTPS
    SSL *conn = replication->ssl->conn ;
    if(conn && !replication->ssl->flags) {
        ret = SSL_write(conn, chunk, outstanding);
    } else {
        ret = send(replication->socket, chunk, outstanding, MSG_DONTWAIT);
    }
#else
    ret = send(replication->socket, chunk, outstanding, MSG_DONTWAIT);
#endif
    if (likely(ret > 0)) {
        cbuffer_remove_unsafe(replication->buffer, ret);
        replication->sent_bytes_on_this_connection += ret;
        replication->sent_bytes += ret;
        debug(D_REPLICATION, "REPLICATION %s [send to %s]: Sent %zd bytes", "s->host->hostname???", replication->connected_to, ret);
        replication->last_sent_t = now_monotonic_sec();
    }
    else if (ret == -1 && (errno == EAGAIN || errno == EINTR || errno == EWOULDBLOCK))
        debug(D_REPLICATION, "REPLICATION %s [send to %s]: unavailable after polling POLLOUT", replication->host->hostname,
              replication->connected_to);
    else if (ret == -1) {
        debug(D_REPLICATION, "REPLICATION: Send failed - closing socket...");
        error("REPLICATION %s [send to %s]: failed to send metrics - closing connection - we have sent %zu bytes on this connection.",  replication->host->hostname, replication->connected_to, replication->sent_bytes_on_this_connection);
        replication_thread_close_socket(replication);
    }
    else {
        debug(D_REPLICATION, "REPLICATION: send() returned 0 -> no error but no transmission");
    }

    netdata_mutex_unlock(&replication->mutex);
    netdata_thread_enable_cancelability();
}

// Thread creation
void *replication_sender_thread(void *ptr) {
    struct sender_state *s = (struct sender_state *) ptr;
    unsigned int rrdpush_replication_enabled = s->replication->enabled;
    info("%s Replication sender thread is starting", REPLICATION_MSG);

    // remove the non-blocking flag from the socket
    if(sock_delnonblock(s->replication->socket) < 0)
        error("%s %s [receive from [%s]:%s]: cannot remove the non-blocking flag from socket %d", REPLICATION_MSG, s->host->hostname, s->replication->client_ip, s->replication->client_port, s->replication->socket);

   /*
    // convert the socket to a FILE *
    FILE *fp = fdopen(s->replication->socket, "r");
    if(!fp) {
        log_stream_connection(s->replication->client_ip, s->replication->client_port, s->host->rrdpush_send_api_key, s->host->machine_guid, s->host->hostname, "SOCKET CONVERSION TO FD FAILED - SOCKET ERROR");
        error("%s %s [receive from [%s]:%s]: failed to get a FILE for FD %d.", REPLICATION_MSG, s->host->hostname, s->replication->client_ip, s->replication->client_port, s->replication->socket);
        close(s->replication->socket);
        return 0;
    }
  */
    // attempt to connect to parent
    // Read the config for sending in replication
    // Add here the sender initialization logic of the thread.
    netdata_thread_cleanup_push(replication_sender_thread_cleanup_callback, s->host);
    // Add here the thread loop
    // break condition on netdata_exit, disabled replication (for runtime configuration/restart)
    // for(; rrdpush_replication_enabled && !netdata_exit ;) {
        // check for outstanding cancellation requests
        // netdata_thread_testcancel();
    //     // try to connect
    //     // replcation parser
    //     // send hi
    //     // retrieve response
    //     // if reponse is REP off - exit
    // }
    //Implementation...

    for(;rrdpush_replication_enabled && !netdata_exit;)
    {
        // check for outstanding cancellation requests
        netdata_thread_testcancel();
        
        // try to connect
        // if(!s->replication->connected)
        if((s->replication->not_connected_loops < 3) && !s->replication->connected) {
            replication_attempt_to_connect(s);
            // Tmp solution to test the thread cleanup process
            s->replication->not_connected_loops++;            
        }
        else {
            //send_message(s->replication, "REP ON\n");
            replication_parser(s->replication, NULL, s->replication->fp);
            //sleep(1);

        }
    }
    // Closing thread - clean up any resources allocated here
    netdata_thread_cleanup_pop(1);
    return NULL;
}

void replication_sender_thread_spawn(RRDHOST *host) {
    netdata_mutex_lock(&host->sender->replication->mutex);

    //TDRemoved Replication
    print_replication_state(host->sender->replication);
    
    if(!host->sender->replication->spawned) {
        char tag[NETDATA_THREAD_TAG_MAX + 1];
        snprintfz(tag, NETDATA_THREAD_TAG_MAX, "REPLICATION_SENDER[%s]", host->hostname);

        if(netdata_thread_create(&host->sender->replication->thread, tag, NETDATA_THREAD_OPTION_JOINABLE, replication_sender_thread, (void *) host->sender))
            error("%s %s [send]: failed to create new thread for client.", REPLICATION_MSG, host->hostname);
        else
            host->sender->replication->spawned = 1;
    }
    netdata_mutex_unlock(&host->sender->replication->mutex);
}

void send_message(struct replication_state *replication, char* message){
    replication_start(replication);
    buffer_sprintf(replication->build, message);
    replication_commit(replication);
    replication_attempt_to_send(replication);
}

void *replication_receiver_thread(void *ptr){
    netdata_thread_cleanup_push(replication_receiver_thread_cleanup_callback, ptr);
    struct receiver_state *rpt = (struct receiver_state *)ptr;
    unsigned int rrdpush_replication_enabled =  rpt->replication->enabled;
    rpt->replication->exited = 0; //latch down the exited flag on Rx thread start up.
    // GAPS *gaps_timeline = rpt->replication->gaps_timeline;
    //read configuration
    //create pluginds cd object
    struct plugind cd = {
            .enabled = 1,
            .update_every = default_rrd_update_every,
            .pid = 0,
            .serial_failures = 0,
            .successful_collections = 0,
            .obsolete = 0,
            .started_t = now_realtime_sec(),
            .next = NULL,
            .version = 0,
    };

    // put the client IP and port into the buffers used by plugins.d
    snprintfz(cd.id,           CONFIG_MAX_NAME,  "%s:%s", rpt->replication->client_ip, rpt->replication->client_port);
    snprintfz(cd.filename,     FILENAME_MAX,     "%s:%s", rpt->replication->client_ip, rpt->replication->client_port);
    snprintfz(cd.fullfilename, FILENAME_MAX,     "%s:%s", rpt->replication->client_ip, rpt->replication->client_port);
    snprintfz(cd.cmd,          PLUGINSD_CMD_MAX, "%s:%s", rpt->replication->client_ip, rpt->replication->client_port);    
    // Respond with the REP ack command
    info("%s %s [receive from [%s]:%s]: initializing replication communication...", REPLICATION_MSG, rpt->host->hostname, rpt->replication->client_ip, rpt->replication->client_port);
    char initial_response[HTTP_HEADER_SIZE];
    if (rpt->stream_version >= VERSION_GAP_FILLING) {
        info("%s %s [receive from [%s]:%s]: Netdata acknowledged replication over stream version %u.", REPLICATION_MSG, rpt->host->hostname, rpt->replication->client_ip, rpt->replication->client_port, rpt->stream_version);
        sprintf(initial_response, "%s", REP_ACK_CMD);
    } 
    else {
        info("%s [receive from [%s]:%s]: Netdata stream protocol does not support replication.", rpt->host->hostname, rpt->replication->client_ip, rpt->replication->client_port);
        sprintf(initial_response, "%s", "REP off");
    }
    // debug(D_REPLICATION, "Initial REPLICATION response to %s: %s", rpt->client_ip, initial_response);
    info("%s: Initial REPLICATION response to [%s:%s]: %s", REPLICATION_MSG, rpt->replication->client_ip, rpt->replication->client_port, initial_response);
    #ifdef ENABLE_HTTPS
    rpt->host->stream_ssl.conn = rpt->replication->ssl->conn;
    rpt->host->stream_ssl.flags = rpt->replication->ssl->flags;
    if(send_timeout(rpt->replication->ssl, rpt->replication->socket, initial_response, strlen(initial_response), 0, 60) != (ssize_t)strlen(initial_response)) {
#else
    if(send_timeout(rpt->replication->socket, initial_response, strlen(initial_response), 0, 60) != strlen(initial_response)) {
#endif
        log_stream_connection(rpt->replication->client_ip, rpt->replication->client_port, rpt->key, rpt->host->machine_guid, rpt->host->hostname, "REPLICATION CONNECTION FAILED - THIS HOST FAILED TO REPLY");
        error("%s %s [receive from [%s]:%s]: failed to send replication acknowledgement command.", REPLICATION_MSG, rpt->host->hostname, rpt->replication->client_ip, rpt->replication->client_port);
        close(rpt->replication->socket);
        return 0;
    }
    // Here is the first proof of connection with the sender thread.

    // remove the non-blocking flag from the socket
    if(sock_delnonblock(rpt->replication->socket) < 0)
        error("%s %s [receive from [%s]:%s]: cannot remove the non-blocking flag from socket %d", REPLICATION_MSG, rpt->host->hostname, rpt->replication->client_ip, rpt->replication->client_port, rpt->replication->socket);

    // convert the socket to a FILE *
    FILE *fp = fdopen(rpt->replication->socket, "r");
    if(!fp) {
        log_stream_connection(rpt->replication->client_ip, rpt->replication->client_port, rpt->key, rpt->host->machine_guid, rpt->host->hostname, "SOCKET CONVERSION TO FD FAILED - SOCKET ERROR");
        error("%s %s [receive from [%s]:%s]: failed to get a FILE for FD %d.", REPLICATION_MSG, rpt->host->hostname, rpt->replication->client_ip, rpt->replication->client_port, rpt->replication->socket);
        close(rpt->replication->socket);
        return 0;
    }
    
    // call the plugins.d processor to receive the metrics
    info("%s %s [receive from [%s]:%s]: filling replication gaps...", REPLICATION_MSG, rpt->host->hostname, rpt->replication->client_ip, rpt->replication->client_port);
    log_stream_connection(rpt->replication->client_ip, rpt->replication->client_port, rpt->key, rpt->host->machine_guid, rpt->host->hostname, "CONNECTED");

    cd.version = rpt->stream_version;
    UNUSED(rrdpush_replication_enabled);

    // Add here the receiver thread logic
    // Need a host
    // Need a PARSER_USER_OBJECT
    // Need a socket
    // need flags to deactivate the no necessary keywords
    // Add here the thread loop
    // for(;rrdpush_replication_enabled && !netdata_exit;)
    // {
    //     // check for outstanding cancellation requests
    //     netdata_thread_testcancel();      

    //     if(gaps_timeline->queue_size == 0){
    //         // Send REP off CMD to the child agent.
    //         break;
    //     }

    //     // send GAP uid ts tf te to the child agent        
    //     // recv RDATA command from child agent
    // }    
    // Closing thread - clean any resources allocated in this thread function
    for(;rrdpush_replication_enabled && !netdata_exit;)
    {
        // check for outstanding cancellation requests
        netdata_thread_testcancel();
        replication_parser(rpt->replication, &cd, fp);
    }
    // Closing thread - clean up any resources allocated here
    netdata_thread_cleanup_pop(1);
    return NULL;   
}

extern int rrdpush_receiver_too_busy_now(struct web_client *w);

extern int rrdpush_receiver_permission_denied(struct web_client *w);

int replication_receiver_thread_spawn(struct web_client *w, char *url) {
    info("clients wants to REPLICATE metrics.");

    char *key = NULL, *hostname = NULL, *registry_hostname = NULL, *machine_guid = NULL, *os = "unknown", *timezone = "unknown", *abbrev_timezone = "UTC", *tags = NULL;
    int32_t utc_offset = 0;
    int update_every = default_rrd_update_every;
    uint32_t stream_version = UINT_MAX;
    char buf[GUID_LEN + 1];

    //parse url or REPLICATE command arguments
    struct rrdhost_system_info *system_info = callocz(1, sizeof(struct rrdhost_system_info));
    system_info->hops = 1;
    while(url) {
        char *value = mystrsep(&url, "&");
        if(!value || !*value) continue;

        char *name = mystrsep(&value, "=");
        if(!name || !*name) continue;
        if(!value || !*value) continue;

        if(!strcmp(name, "key"))
            key = value;
        else if(!strcmp(name, "hostname"))
            hostname = value;
        else if(!strcmp(name, "registry_hostname"))
            registry_hostname = value;
        else if(!strcmp(name, "machine_guid"))
            machine_guid = value;
        else if(!strcmp(name, "update_every"))
            update_every = (int)strtoul(value, NULL, 0);
        else if(!strcmp(name, "timezone"))
            timezone = value;
        else if(!strcmp(name, "abbrev_timezone"))
            abbrev_timezone = value;
        else if(!strcmp(name, "utc_offset"))
            utc_offset = (int32_t)strtol(value, NULL, 0);
        else if(!strcmp(name, "hops"))
            system_info->hops = (uint16_t) strtoul(value, NULL, 0);
        else if(!strcmp(name, "tags"))
            tags = value;
        else if(!strcmp(name, "ver"))
            stream_version = MIN((uint32_t) strtoul(value, NULL, 0), STREAMING_PROTOCOL_CURRENT_VERSION);
        else {
            // An old Netdata child does not have a compatible streaming protocol, map to something sane.
            if(!strcmp(name, "NETDATA_PROTOCOL_VERSION") && stream_version == UINT_MAX) {
                stream_version = 1;
            }

            if (unlikely(rrdhost_set_system_info_variable(system_info, name, value))) {
                infoerr("%s [receive from [%s]:%s]: request has parameter '%s' = '%s', which is not used.", REPLICATION_MSG, w->client_ip, w->client_port, name, value);
            }
        }
    }

    //  Verify URL parameters - Replication arg before the creation of the receiving thread
    if (stream_version == UINT_MAX)
        stream_version = 0;

    if(!key || !*key) {
        rrdhost_system_info_free(system_info);
        log_stream_connection(w->client_ip, w->client_port, (key && *key)?key:"-", (machine_guid && *machine_guid)?machine_guid:"-", (hostname && *hostname)?hostname:"-", "ACCESS DENIED - NO KEY");
        error("%s [receive from [%s]:%s]: Replicate request without an API key. Forbidding access.", REPLICATION_MSG, w->client_ip, w->client_port);
        return rrdpush_receiver_permission_denied(w);
    }

    if(!hostname || !*hostname) {
        rrdhost_system_info_free(system_info);
        log_stream_connection(w->client_ip, w->client_port, (key && *key)?key:"-", (machine_guid && *machine_guid)?machine_guid:"-", (hostname && *hostname)?hostname:"-", "ACCESS DENIED - NO HOSTNAME");
        error("%s [receive from [%s]:%s]: Replicate request without a hostname. Forbidding access.", REPLICATION_MSG, w->client_ip, w->client_port);
        return rrdpush_receiver_permission_denied(w);
    }

    if(!machine_guid || !*machine_guid) {
        rrdhost_system_info_free(system_info);
        log_stream_connection(w->client_ip, w->client_port, (key && *key)?key:"-", (machine_guid && *machine_guid)?machine_guid:"-", (hostname && *hostname)?hostname:"-", "ACCESS DENIED - NO MACHINE GUID");
        error("%s [receive from [%s]:%s]: Replicate request without a machine GUID. Forbidding access.",REPLICATION_MSG, w->client_ip, w->client_port);
        return rrdpush_receiver_permission_denied(w);
    }

    if(regenerate_guid(key, buf) == -1) {
        rrdhost_system_info_free(system_info);
        log_stream_connection(w->client_ip, w->client_port, (key && *key)?key:"-", (machine_guid && *machine_guid)?machine_guid:"-", (hostname && *hostname)?hostname:"-", "ACCESS DENIED - INVALID KEY");
        error("%s [receive from [%s]:%s]: API key '%s' is not valid GUID (use the command uuidgen to generate one). Forbidding access.", REPLICATION_MSG, w->client_ip, w->client_port, key);
        return rrdpush_receiver_permission_denied(w);
    }

    if(regenerate_guid(machine_guid, buf) == -1) {
        rrdhost_system_info_free(system_info);
        log_stream_connection(w->client_ip, w->client_port, (key && *key)?key:"-", (machine_guid && *machine_guid)?machine_guid:"-", (hostname && *hostname)?hostname:"-", "ACCESS DENIED - INVALID MACHINE GUID");
        error("%s [receive from [%s]:%s]: machine GUID '%s' is not GUID. Forbidding access.", REPLICATION_MSG, w->client_ip, w->client_port, machine_guid);
        return rrdpush_receiver_permission_denied(w);
    }

    if(!appconfig_get_boolean(&stream_config, key, "enabled", 0)) {
        rrdhost_system_info_free(system_info);
        log_stream_connection(w->client_ip, w->client_port, (key && *key)?key:"-", (machine_guid && *machine_guid)?machine_guid:"-", (hostname && *hostname)?hostname:"-", "ACCESS DENIED - KEY NOT ENABLED");
        error("%s [receive from [%s]:%s]: API key '%s' is not allowed. Forbidding access.", REPLICATION_MSG, w->client_ip, w->client_port, key);
        return rrdpush_receiver_permission_denied(w);
    }

    {
        SIMPLE_PATTERN *key_allow_from = simple_pattern_create(appconfig_get(&stream_config, key, "allow from", "*"), NULL, SIMPLE_PATTERN_EXACT);
        if(key_allow_from) {
            if(!simple_pattern_matches(key_allow_from, w->client_ip)) {
                simple_pattern_free(key_allow_from);
                rrdhost_system_info_free(system_info);
                log_stream_connection(w->client_ip, w->client_port, (key && *key)?key:"-", (machine_guid && *machine_guid)?machine_guid:"-", (hostname && *hostname) ? hostname : "-", "ACCESS DENIED - KEY NOT ALLOWED FROM THIS IP");
                error("%s [receive from [%s]:%s]: API key '%s' is not permitted from this IP. Forbidding access.", REPLICATION_MSG, w->client_ip, w->client_port, key);
                return rrdpush_receiver_permission_denied(w);
            }
            simple_pattern_free(key_allow_from);
        }
    }

    if(!appconfig_get_boolean(&stream_config, machine_guid, "enabled", 1)) {
        rrdhost_system_info_free(system_info);
        log_stream_connection(w->client_ip, w->client_port, (key && *key)?key:"-", (machine_guid && *machine_guid)?machine_guid:"-", (hostname && *hostname)?hostname:"-", "ACCESS DENIED - MACHINE GUID NOT ENABLED");
        error("%s [receive from [%s]:%s]: machine GUID '%s' is not allowed. Forbidding access.", REPLICATION_MSG, w->client_ip, w->client_port, machine_guid);
        return rrdpush_receiver_permission_denied(w);
    }

    {
        SIMPLE_PATTERN *machine_allow_from = simple_pattern_create(appconfig_get(&stream_config, machine_guid, "allow from", "*"), NULL, SIMPLE_PATTERN_EXACT);
        if(machine_allow_from) {
            if(!simple_pattern_matches(machine_allow_from, w->client_ip)) {
                simple_pattern_free(machine_allow_from);
                rrdhost_system_info_free(system_info);
                log_stream_connection(w->client_ip, w->client_port, (key && *key)?key:"-", (machine_guid && *machine_guid)?machine_guid:"-", (hostname && *hostname) ? hostname : "-", "ACCESS DENIED - MACHINE GUID NOT ALLOWED FROM THIS IP");
                error("%s [receive from [%s]:%s]: Machine GUID '%s' is not permitted from this IP. Forbidding access.", REPLICATION_MSG, w->client_ip, w->client_port, machine_guid);
                return rrdpush_receiver_permission_denied(w);
            }
            simple_pattern_free(machine_allow_from);
        }
    }
    UNUSED(update_every);
    UNUSED(abbrev_timezone);
    UNUSED(tags);
    UNUSED(timezone);
    UNUSED(os);                
    UNUSED(utc_offset);
    UNUSED(registry_hostname);                 

    // Replication request rate limit control
    if(unlikely(web_client_replication_rate_t > 0)) {
        static netdata_mutex_t replication_rate_mutex = NETDATA_MUTEX_INITIALIZER;
        static volatile time_t last_replication_accepted_t = 0;

        netdata_mutex_lock(&replication_rate_mutex);
        time_t now = now_realtime_sec();

        if(unlikely(last_replication_accepted_t == 0))
            last_replication_accepted_t = now;

        if(now - last_replication_accepted_t < web_client_replication_rate_t) {
            netdata_mutex_unlock(&replication_rate_mutex);
            rrdhost_system_info_free(system_info);
            error("%s [receive from [%s]:%s]: too busy to accept new streaming request. Will be allowed in %ld secs.", REPLICATION_MSG, w->client_ip, w->client_port, (long)(web_client_replication_rate_t - (now - last_replication_accepted_t)));
            return rrdpush_receiver_too_busy_now(w);
        }

        last_replication_accepted_t = now;
        netdata_mutex_unlock(&replication_rate_mutex);
    }

    // What it does: if host doesn't exist prepare the receiver state struct
    // and start the streaming receiver to create it.
    // What I want:  If the host doesn't exist I should depend on streaming to create it. At this point, streaming should have already call the receiver thread to create the host. So if the host exists we continue with the call to the replication rx thread. If the host doesn't exist and host->receiver is NULL means that there was a problem with host creation during streaming or the REPLICATE command arrived earlier than the respective STREAM command. So do not start the Rx replication thread.The replication Tx thread in child should try to reconnect.

    rrd_rdlock();
    RRDHOST *host = rrdhost_find_by_guid(machine_guid, 0);
    if (unlikely(host && rrdhost_flag_check(host, RRDHOST_FLAG_ARCHIVED))) /* Ignore archived hosts. */
        host = NULL;
    if(!host) {
            rrd_unlock();
            log_stream_connection(w->client_ip, w->client_port, key, machine_guid, hostname, "ABORT REPLICATION - HOST DOES NOT EXIST");
            infoerr("%s - [received from [%s]:%s]: Host(%s) with machine GUID %s does not exist - Abort replication.", REPLICATION_MSG, w->client_ip, w->client_port, hostname, machine_guid);
            return 409;
    }
    // Chase race condition in case of two REPLICATE requests hit the web server. One should start the receiver replication thread
    // and the other should be rejected.
    // Verify this code: Host exists and replication is active.
    rrdhost_wrlock(host);
    if (host->receiver->replication != NULL) {
        time_t age = now_realtime_sec() - host->receiver->replication->last_msg_t;
        rrdhost_unlock(host);
        rrd_unlock();
        log_stream_connection(w->client_ip, w->client_port, key, host->machine_guid, host->hostname, "REJECTED - ALREADY CONNECTED");
        info("%s %s [receive from [%s]:%s]: multiple connections for same host detected - existing connection is active (within last %ld sec), rejecting new connection.", REPLICATION_MSG, host->hostname, w->client_ip, w->client_port, age);
        // Have not set WEB_CLIENT_FLAG_DONT_CLOSE_SOCKET - caller should clean up
        buffer_flush(w->response.data);
        buffer_strcat(w->response.data, "This GUID is already replicating to this server");
        return 409;
    }
    rrdhost_unlock(host);
    rrd_unlock();

    // Host exists and replication is not active
    // Initialize replication receiver structure.
    host->receiver->replication = (REPLICATION_STATE *)callocz(1, sizeof(REPLICATION_STATE));    
    replication_receiver_init(host->receiver, &stream_config);
    host->receiver->replication->last_msg_t = now_realtime_sec();
    host->receiver->replication->socket = w->ifd;
    host->receiver->replication->client_ip = strdupz(w->client_ip);
    host->receiver->replication->client_port = strdupz(w->client_port);
#ifdef ENABLE_HTTPS
    host->receiver->replication->ssl->conn = w->ssl.conn;
    host->receiver->replication->ssl->flags = w->ssl.flags;
    w->ssl.conn = NULL;
    w->ssl.flags = NETDATA_SSL_START;
#endif

    if(w->user_agent && w->user_agent[0]) {
        char *t = strchr(w->user_agent, '/');
        if(t && *t) {
            *t = '\0';
            t++;
        }

        host->receiver->replication->program_name = strdupz(w->user_agent);
        if(t && *t) host->receiver->replication->program_version = strdupz(t);
    }

    debug(D_SYSTEM, "starting REPLICATE receive thread.");
    info("%s: Starting REPLICATE receive thread.", REPLICATION_MSG);

    char tag[FILENAME_MAX + 1];
    snprintfz(tag, FILENAME_MAX, "REPLICATION_RECEIVER[%s,[%s]:%s]", host->hostname, w->client_ip, w->client_port);

    if(netdata_thread_create(&host->receiver->replication->thread, tag, NETDATA_THREAD_OPTION_DEFAULT, replication_receiver_thread, (void *)(host->receiver)))
        error("Failed to create new REPLICATE receive thread for client.");

    // prevent the caller from closing the streaming socket
    if(web_server_mode == WEB_SERVER_MODE_STATIC_THREADED) {
        web_client_flag_set(w, WEB_CLIENT_FLAG_DONT_CLOSE_SOCKET);
    }
    else {
        if(w->ifd == w->ofd)
            w->ifd = w->ofd = -1;
        else
            w->ifd = -1;
    }

    buffer_flush(w->response.data);
    return 200;
}

// Thread clean-up & destroy
static void replication_sender_thread_cleanup_callback(void *ptr) {
    RRDHOST *host = (RRDHOST *)ptr;

    netdata_mutex_lock(&host->sender->replication->mutex);

    info("%s %s [send]: sending thread cleans up...", REPLICATION_MSG, host->hostname);

    //close replication sender thread socket or/and pipe
    replication_sender_thread_close_socket(host);
    // clean the structures and resources in the thread function
    // follow the shutdown sequence with the sender thread from the rrdhost.c file

    // TBD - Check if joining the streaming threads is good for shutting down the replication threads.
    if(!host->sender->replication->sender_thread_join) {
        info("%s %s [send]: sending thread detaches itself.", REPLICATION_MSG, host->hostname);
        netdata_thread_detach(netdata_thread_self());
    }

    host->sender->replication->spawned = 0;

    info("%s %s [send]: sending thread now exits.", REPLICATION_MSG, host->hostname);

    netdata_mutex_unlock(&host->sender->replication->mutex);
}

void replication_receiver_thread_cleanup_callback(void *ptr)
{
    // destroy the replication rx structs - TBD
    info("%s: Hey you, add something here...I need to cleanup the receiver thread!!! :P", REPLICATION_MSG);

    static __thread int executed = 0;
    if(!executed) {
        executed = 1;
        struct receiver_state *rpt = (struct receiver_state *) ptr;
        // If the shutdown sequence has started, and this receiver is still attached to the host then we cannot touch
        // the host pointer as it is unpredictable when the RRDHOST is deleted. Do the cleanup from rrdhost_free().
        if (netdata_exit && rpt->host) {
            rpt->replication->exited = 1;
            return;
        }

        // Make sure that we detach this thread and don't kill a freshly arriving receiver
        if (!netdata_exit && rpt->host) {
            netdata_mutex_lock(&rpt->replication->mutex);
            if (rpt->host->receiver == rpt){
                rpt->host->receiver = NULL;
                }
            netdata_mutex_unlock(&rpt->replication->mutex);
        }
        info("%s %s [receive from [%s]:%s]: receive thread ended (task id %d)", REPLICATION_MSG, rpt->hostname, rpt->replication->client_ip, rpt->replication->client_port, gettid());
        replication_state_destroy(rpt->replication);
        // // On a parent signal also the sender thread sending to a gparent to shutdown. Probably after the parsing. Check also the clean-up functionality in the rrdhost().        
    }
}

// Any join, start, stop, wait, etc thread function goes here.
// This function should be called when Rx thread is terminating. The Rx thread can start termination after a parser error and/or netdata_exit signal. On shutdown this function will be called to remove any sending replication thread.
void replication_sender_thread_stop(RRDHOST *host) {

    netdata_mutex_lock(&host->sender->replication->mutex);
    netdata_thread_t thr = 0;

    if(host->sender->replication->spawned) {
        info("%s %s [send]: signaling replication sending thread to stop...", REPLICATION_MSG, host->hostname);

        // Check if this is necessary for replication thread?
        //signal the thread that we want to join it
        host->sender->replication->sender_thread_join = 1;

        // copy the thread id, so that we will be waiting for the right one
        // even if a new one has been spawn
        thr = host->sender->replication->thread;

        // signal it to cancel
        netdata_thread_cancel(host->sender->replication->thread);
    }

    netdata_mutex_unlock(&host->sender->replication->mutex);

    if(thr != 0) {
        info("%s %s [send]: waiting for the replication sending thread to stop...", REPLICATION_MSG, host->hostname);
        void *result;
        netdata_thread_join(thr, &result);
        info("%s %s [send]: replication sending thread has exited.", REPLICATION_MSG, host->hostname);
    }
    // Clean-up the replication Tx thread structure.
    replication_state_destroy(host->sender->replication);
}

// static inline int parse_replication_ack(char *http)
// {
//     if(unlikely(memcmp(http, REP_ACK_CMD, (size_t)strlen(REP_ACK_CMD))))
//         return 1;
//     return 0;
// }

// Memory Mode access
void collect_replication_gap_data(){
    // collection of gap data in cache/temporary structure
}

void update_memory_index(){
    //dbengine
    //other memory modes?
}

// Store gap in agent metdata DB(sqlite)
int save_gap(GAP *a_gap)
{
    int rc;
    
    // TBR
    info("%s: SAVE in SQLITE this GAP:", REPLICATION_MSG);
    print_replication_gap(a_gap);

    if (unlikely(!db_meta) && default_rrd_memory_mode != RRD_MEMORY_MODE_DBENGINE)
        return 0;
    rc = sql_store_gap(
        &a_gap->gap_uuid,
        a_gap->host_mguid,
        a_gap->t_window.t_start,
        a_gap->t_window.t_first,
        a_gap->t_window.t_end,
        a_gap->status);
    if(!rc)
        info("%s: GAP saved in Netdata agent metadata DB", REPLICATION_MSG);

    return rc;
}

// load gaps from agent metdata db
int load_gap(RRDHOST *host)
{
    // Load on start up after a shutdown
    int rc;
    
    // TBR
    info("%s: LOAD from SQLITE this GAP:", REPLICATION_MSG);
    print_replication_gap(host->gaps_timeline->gap_data);

    if (unlikely(!db_meta) && default_rrd_memory_mode != RRD_MEMORY_MODE_DBENGINE)
        return SQLITE_ERROR;
    rc = sql_load_host_gap(host);
    if(!rc)
        info("%s: Load GAPS from metadata DB in host %s", REPLICATION_MSG, host->hostname);
    // Load on start up evaluate a crash
    // Update the queue values and let it consume the gaps on runtime

    return rc;
}

//delete gap from agent metdata db
int remove_gap(GAP *a_gap)
{
    int rc;
    
    // TBR
    info("%s: REMOVE in SQLITE this GAP: ", REPLICATION_MSG);
    print_replication_gap(a_gap);
    
    if (unlikely(!db_meta) && default_rrd_memory_mode != RRD_MEMORY_MODE_DBENGINE)
        return 0;
    rc = sql_delete_gap(&a_gap->gap_uuid);
    if(!rc)
        info("%s: Delete GAP from metadata DB", REPLICATION_MSG);

    return rc;
}

/* Produce a full line if one exists, statefully return where we start next time.
 * When we hit the end of the buffer with a partial line move it to the beginning for the next fill.
 */
static char *receiver_next_line(struct replication_state *r, int *pos) {
    int start = *pos, scan = *pos;
    if (scan >= r->read_len) {
        r->read_len = 0;
        return NULL;
    }
    while (scan < r->read_len && r->read_buffer[scan] != '\n')
        scan++;
    if (scan < r->read_len && r->read_buffer[scan] == '\n') {
        *pos = scan+1;
        r->read_buffer[scan] = 0;
        return &r->read_buffer[start];
    }
    memmove(r->read_buffer, &r->read_buffer[start], r->read_len - start);
    r->read_len -= start;
    return NULL;
}

/* The receiver socket is blocking, perform a single read into a buffer so that we can reassemble lines for parsing.
 */
static int receiver_read(struct replication_state *r, FILE *fp) {
#ifdef ENABLE_HTTPS
    if (r->ssl->conn && !r->ssl->flags) {
        ERR_clear_error();
        int desired = sizeof(r->read_buffer) - r->read_len - 1;
        int ret = SSL_read(r->ssl->conn, r->read_buffer + r->read_len, desired);
        if (ret > 0 ) {
            r->read_len += ret;
            return 0;
        }
        // Don't treat SSL_ERROR_WANT_READ or SSL_ERROR_WANT_WRITE differently on blocking socket
        u_long err;
        char buf[256];
        while ((err = ERR_get_error()) != 0) {
            ERR_error_string_n(err, buf, sizeof(buf));
            error("STREAM %s [receive from %s] ssl error: %s", r->host->hostname, r->client_ip, buf);
        }
        return 1;
    }
#endif
    if (!fgets(r->read_buffer, sizeof(r->read_buffer), fp))
        return 1;
    r->read_len = strlen(r->read_buffer);
    return 0;
}

// Replication parser & commands
size_t replication_parser(struct replication_state *rpt, struct plugind *cd, FILE *fp) {
    size_t result;
    PARSER_USER_OBJECT *user = callocz(1, sizeof(*user));
    //TODO cd?
    if(cd){
        user->enabled = cd->enabled;
    }
    user->host = rpt->host;
    user->opaque = rpt;
    user->cd = cd;
    user->trust_durations = 0;

    // flags & PARSER_NO_PARSE_INIT to avoid default keyword
    PARSER *parser = parser_init(rpt->host, user, fp, PARSER_INPUT_SPLIT);

    if (unlikely(!parser)) {
        error("Failed to initialize parser");
        //TODO cd?
        if(cd){
            cd->serial_failures++;
        }

        freez(user);
        return 0;
    }
    
    // Add keywords related with REPlication
    // REP on/off/pause/ack
    // GAP - Gap metdata. Information to describe the gap (window_start/end, uuid, chart/dim_id)
    // RDATA - gap data transmission
    // Do I need these two commands in replication?
    // parser_add_keyword(parser, "TIMESTAMP", pluginsd_suspend_this_action);
    // parser_add_keyword(parser, "CLAIMED_ID", pluginsd_suspend_this_action);

    // These are not necessary for the replication parser. Normally I would suggest to assign an inactive action so the replication won't be able to use other functions that can trigger function execution not related with its tasks.
    parser->plugins_action->begin_action     = &pluginsd_suspend_this_action;
    // discuss it with the team
    parser->plugins_action->flush_action     = &pluginsd_flush_action;
    parser->plugins_action->end_action       = &pluginsd_end_action;
    parser->plugins_action->disable_action   = &pluginsd_disable_action;
    parser->plugins_action->variable_action  = &pluginsd_variable_action;
    parser->plugins_action->dimension_action = &pluginsd_dimension_action;
    parser->plugins_action->label_action     = &pluginsd_label_action;
    parser->plugins_action->overwrite_action = &pluginsd_overwrite_action;
    parser->plugins_action->chart_action     = &pluginsd_chart_action;
    parser->plugins_action->set_action       = &pluginsd_set_action;
    parser->plugins_action->clabel_commit_action  = &pluginsd_clabel_commit_action;
    parser->plugins_action->clabel_action    = &pluginsd_clabel_action;
    // Add the actions related with replication here.
    // parser->plugins_action->gap_action    = &pluginsd_gap_action;
    // parser->plugins_action->rep_action    = &pluginsd_rep_action;
    // parser->plugins_action->rdata_action    = &pluginsd_rdata_action;

    user->parser = parser;

    do {
        if (receiver_read(rpt, fp))
            break;
        int pos = 0;
        char *line;
        while ((line = receiver_next_line(rpt, &pos))) {
            info("Parser received: %s \n", line);
            //TODO shutdown?
            if (unlikely(netdata_exit /*|| rpt->shutdown*/ || parser_action(parser,  line)))
                goto done;
        }
        rpt->last_msg_t = now_realtime_sec();
    }
    while(!netdata_exit);
done:
    result= user->count;
    freez(user);
    parser_destroy(parser);
    return result;
}

// // GAP creation and processing
// static GAP gap_init() {
//     GAP new_gap;
//     TIME_WINDOW new_tw;
//     new_gap.t_window = new_tw;
//     new_gap.status = "oninit";
//     print_replication_gap(&new_gap);
//     return new_gap;
// }

void gap_destroy(GAP *a_gap) {
    uuid_clear(a_gap->gap_uuid);
    freez(a_gap->host_mguid);
    freez(a_gap);
}



// static int gaps_init(GAPS *new_gaps) {
//     new_gaps = (GAPS *)callocz()
//     info("LIBQUEUE Initialization");    
//     new_gaps->gaps = queue_new(REPLICATION_RX_CMD_Q_MAX_SIZE);
//     if(!new_gaps->gaps){
//         error("%s Gaps timeline queue could not be created", REPLICATION_MSG);
//         return 1;
//         //Handle this case. Probably shutdown replication.
//     }
//     new_gaps->gaps_data = (GAP *)callocz(REPLICATION_RX_CMD_Q_MAX_SIZE, sizeof(GAP));
//     info("%s: GAPs Initialization", REPLICATION_MSG);
//     return 0;
// }

void gaps_init(RRDHOST *host) {
    info("%s: LibQueue Initialization", REPLICATION_MSG);
    host->gaps_timeline->gaps = queue_new(REPLICATION_RX_CMD_Q_MAX_SIZE);
    if(!host->gaps_timeline->gaps){
        error("%s Gaps timeline queue could not be created", REPLICATION_MSG);
        return;
        //Handle this case. Probably shutdown deactivate replication.
    }
    host->gaps_timeline->gap_data = (GAP *)callocz(1, sizeof(GAP));
    // load from agent metdata
    if(load_gap(host)){
        infoerr("%s: GAPs struct in SQLITE is either empty or failed", REPLICATION_MSG);
        return;
    }
    if(!queue_push(host->gaps_timeline->gaps, (void *)host->gaps_timeline->gap_data)){
        error("%s: Cannot insert the loaded GAP in the queue!", REPLICATION_MSG);
        print_replication_gap(host->gaps_timeline->gap_data);
        return;
    }
    info("%s: GAPs STRUCT Initialization/Loading", REPLICATION_MSG);
    return;
}

void gaps_destroy(RRDHOST *host) {
    // Save gaps before destroy
    info("%s: DESTROYING GAP for HOST %s", REPLICATION_MSG, host->hostname);
    if(save_gap(host->gaps_timeline->gap_data))
        error("%s: Cannot save GAP struct in metadata DB.", REPLICATION_MSG);
    queue_free(host->gaps_timeline->gaps);
    gap_destroy(host->gaps_timeline->gap_data);
}

void generate_new_gap(struct receiver_state *stream_recv) {
    GAP *newgap = stream_recv->host->gaps_timeline->gap_data;
    uuid_generate(newgap->gap_uuid);
    newgap->host_mguid = strdupz(stream_recv->machine_guid);
    newgap->t_window.t_start = stream_recv->last_msg_t;
    newgap->t_window.t_first = rrdhost_last_entry_t(stream_recv->host);
    newgap->t_window.t_end = 0;
    newgap->status = "oncreate";
    return;
}

int complete_new_gap(GAP *potential_gap){
    if(strcmp("oncreate", potential_gap->status)) {
        error("%s: This GAP cannot be completed. Need to create it first.", REPLICATION_MSG);
        return 1;
    }
    potential_gap->t_window.t_end = now_realtime_sec();
    potential_gap->status = "oncompletion";
    return 0;
}

int verify_new_gap(GAP *new_gap){
    UNUSED(new_gap);
    // stream_recv->host->gaps_timeline->beginoftime = rrdhost_first_entry_t(sender->host);
    // Access memory to first time_t for all charts?
    // Access memory to verify last time_t for all charts?
    // update the gap time_first
    // Update the gap timewindow
    // Respect any retention period
    // push the gap in the queue
    return 0;
}

void evaluate_gap_onconnection(struct receiver_state *stream_recv)
{
    info("%s: Evaluate GAPs on connection", REPLICATION_MSG);
    if (!stream_recv->host->gaps_timeline) {
        infoerr("%s: GAP Awareness mechanism is not ready - Continue...", REPLICATION_MSG);
        return;
    }
    int count = stream_recv->host->gaps_timeline->gaps->count;
    // load_gap(stream_recv->host);
    print_replication_gap(stream_recv->host->gaps_timeline->gap_data);
    if(count != 0) {
        GAP *front = (GAP *)stream_recv->host->gaps_timeline->gaps->front->item;
        // Re-connection
        if (complete_new_gap(front)) {
            error("%s: Broken GAP sequence. GAP status is %s", REPLICATION_MSG, front->status);
            // Need to take some action here? Maybe added in the back of the Q? OR Get remove it?
            GAP *gap_recycled = (GAP *)queue_pop(stream_recv->host->gaps_timeline->gaps);
            if (queue_push(stream_recv->host->gaps_timeline->gaps, (void *)gap_recycled))
                infoerr("%s: Broken GAP was recycled. GAP status was %s", REPLICATION_MSG, front->status);
            print_replication_gap(front);
            return;
        }
        info("%s: A new GAP was detected", REPLICATION_MSG);
        print_replication_gap(front);
        //verify it in memory
        //push it in the queue
        // save_gap(front);
        return;
    }
    // First connection or no GAPS
    // Handle the retention check here
    infoerr("%s The GAPs queue is empty", REPLICATION_MSG);
}

void evaluate_gap_ondisconnection(struct receiver_state *stream_recv){
    info("%s: Evaluate GAPs on dis-connection", REPLICATION_MSG);
    if (!stream_recv->host->gaps_timeline) {
        infoerr("%s: GAP Awareness mechanism is not ready - Continue...", REPLICATION_MSG);
        return;
    }
    GAPS *the_gaps = stream_recv->host->gaps_timeline;
    generate_new_gap(stream_recv);
    if(!queue_push(the_gaps->gaps, (void *)the_gaps->gap_data)){
        error("%s: Cannot insert the new GAP in the queue!", REPLICATION_MSG);
        print_replication_gap(the_gaps->gap_data);
        return;
    }
    info("%s: New GAP seed was queued!", REPLICATION_MSG);
    // save_gap(the_gaps->gap_data);
    print_replication_gap(the_gaps->gap_data);
}

// transmit the gap information to the child nodes - send the GAP command
int transmit_gap(){
    return 0;
}

// FSMs for replication protocol implementation
// REP on
// REP off
// REP pause/continue
// REP ack

// RDATA

// Replication FSM logic functions


// Helper and Debug functions
static void print_replication_state(REPLICATION_STATE *state)
{
    info(
        "%s: Replication State is ...\n pthread_id: %lu\n, enabled: %u\n, spawned: %u\n, socket: %d\n, connected: %u\n, connected_to: %s\n, reconnects_counter: %lu\n",
        REPLICATION_MSG,
        state->thread,
        state->enabled,
        state->spawned,
        state->socket,
        state->connected,
        state->connected_to,
        state->reconnects_counter);
}

static void print_replication_gap(GAP *a_gap)
{
    char gap_uuid_str[UUID_STR_LEN];
    uuid_unparse(a_gap->gap_uuid, gap_uuid_str);
    info("%s: GAP details are: \nstatus: %s\n, t_s: %ld t_f: %ld t_e: %ld\n, host_mguid: %s\n, gap_uuid_str: %s\n",
        REPLICATION_MSG,
        a_gap->status,
        a_gap->t_window.t_start,
        a_gap->t_window.t_first,
        a_gap->t_window.t_end,
        a_gap->host_mguid,
        gap_uuid_str);
}