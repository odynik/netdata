// #include "libnetdata/libnetdata.h"
//Includes
#define REPLICATION_MSG "REPLICATION_STREAM"
#define REPLICATE_CMD "REPLICATE"
#define REP_CMD "REP"
#define REP_ACK_CMD "REP ACK"

// RDATA command with arguments TBD?? BEGIN SET END
#define RDATA_CMD "RDATA"
// GAP command with arguments TBD?? probably a timewindow struct
#define GAP_CMD "GAP"
#define REPLICATION_RX_CMD_Q_MAX_SIZE (64)

typedef struct gap GAP;
typedef struct time_window TIME_WINDOW;
typedef struct gaps_queue GAPS;
typedef struct rrddim_past_data RRDDIM_PAST_DATA;

// Replication structs
typedef struct replication_state {
    RRDHOST *host;
    // thread variables
    netdata_thread_t thread;
    netdata_mutex_t mutex;
    unsigned int enabled; // result of configuration and negotiation. Runtime flag
    unsigned int spawned;// if the replication thread has been spawned    
    volatile unsigned int sender_thread_join; // Following the normal shutdown seq need to verify the replication sender thread shutdown.
    //state variables
    int excom;   
    // connection variables
    int socket;
    unsigned int connected;
    //tx thread variables
    char connected_to[101];
    size_t reconnects_counter;
    size_t send_attempts;
    size_t begin;
    size_t not_connected_loops;
    size_t sent_bytes;
    size_t sent_bytes_on_this_connection;
    time_t last_sent_t;
    usec_t reconnect_delay;
    // buffer variables
    //TBD: is the mutex for thread management sufficient also for handling access management to the buffers.
    unsigned int overflow:1; //?
    struct circular_buffer *buffer;
    BUFFER *build;
    char read_buffer[512];
    int read_len;
    //rx thread variables    
    time_t last_msg_t;
    char *client_ip;
    char *client_port;
    FILE *fp;
#ifdef ENABLE_HTTPS
    struct netdata_ssl *ssl;
#endif
    char *program_name;
    char *program_version;
    unsigned int shutdown;    // Set it to 1 to tell the thread to exit
    unsigned int exited;      // Indicates that the thread has exited  (NOT A BITFIELD!)
    RRDDIM_PAST_DATA *dim_past_data;
} REPLICATION_STATE;

// GAP structs
typedef struct time_window {
    //check also struct timeval for time_t replacement
    time_t t_start; // window start
    time_t t_first; // first sample in the time window
    time_t t_end; // window end
} TIME_WINDOW;

typedef struct gap {
    uuid_t gap_uuid; // unique number for the GAP
    char *host_mguid; // unique number for the host id
    char *status; // a gap can be oncreation, ontransmission, filled
    TIME_WINDOW t_window; // This is the time window variables of a gap
} GAP;

typedef struct gaps_queue {
    queue_t gaps;   // handles the gap pointers in a queue struct
    GAP *gap_data; // hosting the gap data
    time_t beginoftime; // this should be the timestamp of the first sample in db OR the agents last_timestamp - uptime?
} GAPS;

typedef struct rrddim_past_data {
    char* rrdset_id;
    char* rrddim_id;
    RRDDIM *rd;
    void* page;
    uint32_t page_length;
    usec_t start_time;
    usec_t end_time;
    struct rrdeng_page_descr* descr;
    struct rrdengine_instance *ctx;
    unsigned long page_correlation_id;
} RRDDIM_PAST_DATA;

void replication_gap_to_str(GAP *a_gap, char **gap_str, size_t *len);
void replication_rdata_to_str(GAP *a_gap, char **rdata_str, size_t *len, int block_id);
void print_collected_metric_past_data(RRDDIM_PAST_DATA *past_data, REPLICATION_STATE *rep_state);
void replication_collect_past_metric_init(REPLICATION_STATE *rep_state, char *rrdset_id, char *rrddim_id);
void replication_collect_past_metric(REPLICATION_STATE *rep_state, time_t timestamp, storage_number number);
void replication_collect_past_metric_done(REPLICATION_STATE *rep_state);
void flush_collected_metric_past_data(RRDDIM_PAST_DATA *dim_past_data, REPLICATION_STATE *rep_state);
int save_gap(GAP *a_gap);
int remove_gap(GAP *a_gap);
int load_gap(RRDHOST *host);

extern void test_rrdeng_store_past_metric_page(RRDHOST *host, char *rrdset_id, char *rrddim_id);
extern int rrdeng_store_past_metrics_page_init(RRDDIM_PAST_DATA *dim_past_data, REPLICATION_STATE *rep_state);
extern void rrdeng_store_past_metrics_page(RRDDIM_PAST_DATA *dim_past_data, REPLICATION_STATE *rep_state);
extern void rrdeng_flush_past_metrics_page(RRDDIM_PAST_DATA *dim_past_data, REPLICATION_STATE *rep_state);
extern void rrdeng_store_past_metrics_page_finalize(RRDDIM_PAST_DATA *dim_past_data, REPLICATION_STATE *rep_state);
