import json, os.path, requests
""" This class describes the functionality of the nodes"""
class Node(object):
    """ Initialize the variables of Node class"""
    def __init__(self, name, cname, parser):
        self.name = name
        self.container_name = cname
        self.port = None        # Exposed on host, default is not to expose
        self.guid = None        # Randomly generate
        self.log = None
        self.started = False
        self.parser = parser
        self.stream_target = None
        self.receiver = False
        self.db_mode = "dbengine"
        self.api_key = None
        self.receive_from_api_key = None
        self.tls = False
        self.http = "http://"
        self.config = {}

    """ Returns a copy of the node object """
    def copy(self):
        result = Node(self.name[:], self.container_name[:], self.parser)
        result.port = self.port
        result.guid = self.guid[:]
        # This will be in the old configuration, needs a lookup if this is a problem
        result.stream_target = self.stream_target
        result.db_mode = self.db_mode[:]
        result.api_key = self.api_key
        result.receive_from_api_key = self.receive_from_api_key
        if self.tls:
            self.http="https://"
        result.tls = self.tls
        result.http = self.http
        result.config = dict(self.config.items())
        return result


    def stream_to(self, target):
        self.stream_target = target
        target.receiver = True

    def create_config(self, base):
        compose = os.path.join(base, f"{self.name}-compose.yml")
        guid = os.path.join(base, f"{self.name}-guid")
        conf = os.path.join(base, f"{self.name}-netdata.conf")
        stream = os.path.join(base, f"{self.name}-stream.conf")
        tls_cert = os.path.abspath(os.path.join(base, "../../certificates"))
        with open(guid, "w") as f:
            print(self.guid,file=f)
        with open(compose, "w") as f:
            print(f"version: '3.3'", file=f)
            print(f"services:", file=f)
            print(f"    {self.name}:", file=f)
            print(f"        image: debian_10_dev", file=f)
            #print(f"        command: /usr/bin/gdb ", file=f)
            if self.port is not None:
                print(f"        ports:", file=f)
                print(f"            - {self.port}:19999", file=f)
            print(f"        volumes:", file=f)
            print(f"            - {stream}:/etc/netdata/stream.conf:ro", file=f)
            print(f"            - {guid}:/var/lib/netdata/registry/netdata.public.unique.id:ro", file=f)
            print(f"            - {conf}:/etc/netdata/netdata.conf:ro", file=f)
            if self.tls:
                print(f"#            - {tls_cert}:/etc/netdata/ssl:ro", file=f)
            print(f"        cap_add:", file=f)
            print(f"            - SYS_PTRACE", file=f)
        with open(conf, "w") as f:
            print(f"[global]", file=f)
            print(f"    debug flags = 0x0000000840000000", file=f)
            print(f"    errors flood protection period = 0", file=f)
            print(f"    hostname = {self.name}", file=f)
            print(f"    memory mode = {self.db_mode}", file=f)
            print(f"#    timezone = Europe/Athens", file=f)
            for k,v in self.config.items():
                file, section = k.split("/")
                if file == "netdata.conf" and section == "global":
                    print(f"    {v}")
            print(f"[web]", file=f)
            print(f"#    allow connections from = *", file=f)
            print(f"#    allow streaming from = *", file=f)
            print(f"#    tls version = 1.3", file=f)
            print(f"#    tls ciphers = TLS_AES_256_GCM_SHA384:TLS_CHACHA20_POLY1305_SHA256:TLS_AES_128_GCM_SHA256", file=f)            
            if self.receiver and self.tls:
                print(f"    ssl key = /etc/netdata/ssl/localhost.key", file=f)
                print(f"    ssl certificate = /etc/netdata/ssl/localhost.crt", file=f)
                print(f"#    bind to = *:{self.port}=dashboard|registry|streaming|netdata.conf|badges|management^SSL=force", file=f)
            for k,v in self.config.items():
                file, section = k.split("/")
                if file == "netdata.conf" and section == "web":
                    print(f"    {v}",file=f)
        with open(stream, "w") as f:
            if self.stream_target is not None:
                print(f"[stream]", file=f)
                print(f"    enabled = yes", file=f)
                print(f"    enable replication = yes", file=f)
                if self.tls:
                    print(f"    destination = tcp:{self.stream_target.name}:SSL", file=f)
                    print(f"    ssl skip certificate verification = yes", file=f)                    
                    print(f"    CApath = /etc/ssl/certs/", file=f)
                    print(f"    CAfile = /etc/ssl/certs/netdata_parent.pem", file=f)
                else:
                    print(f"    destination = tcp:{self.stream_target.name}", file=f)
                print(f"    api key = {self.api_key}", file=f)
                print(f"    timeout seconds = 60", file=f)
                print(f"    default port = 19999", file=f)
                print(f"    send charts matching = *", file=f)
                print(f"    buffer size bytes = 10485760", file=f)
                print(f"    reconnect delay seconds = 5", file=f)
                print(f"    initial clock resync iterations = 60", file=f)
                print(f"    gap replication block size = 60", file=f)
                print(f"    history gap replication = 60", file=f)
                print(f"    max gap replication = 60", file=f)
                for k,v in self.config.items():
                    file, section = k.split("/")
                    if file == "stream.conf" and section == "stream":
                        print(f"    {v}",file=f)
            if self.receiver:
                print(f"[{self.receive_from_api_key}]", file=f)
                print(f"    enabled = yes", file=f)
                print(f"    enable replication = yes", file=f)
                print(f"    allow from = *", file=f)
                print(f"    default history = 3600", file=f)
                print(f"    # default memory mode = ram", file=f)
                print(f"    health enabled by default = auto", file=f)
                print(f"    # postpone alarms for a short period after the sender is connected", file=f)
                print(f"    default postpone alarms on connect seconds = 60", file=f)
                print(f"    multiple connections = allow", file=f)
                print(f"    gap replication block size = 60", file=f)
                print(f"    history gap replication = 60", file=f)
                print(f"    max gap replication = 60", file=f)
                for k,v in self.config.items():
                    file, section = k.split("/")
                    if file == "stream.conf" and section == "API_KEY":
                        print(f"    {v}",file=f)

    def get_data(self, chart, host=None):
        if host is None:
            url = f"{self.http}localhost:{self.port}/api/v1/data?chart={chart}"
        else:
            url = f"{self.http}localhost:{self.port}/host/{host}/api/v1/data?chart={chart}"
        try:
            r = requests.get(url)
            return r.json()
        except json.decoder.JSONDecodeError:
            print(f"  Fetch failed {url} -> {r.text}")
            return None
        except requests.exceptions.ConnectionError:
            print(f"  Fetch failed {url} -> connection refused")
            return None

    def get_charts(self):
        url = f"{self.http}localhost:{self.port}/api/v1/charts"
        try:
            r = requests.get(url)
            return r.json()
        except json.decoder.JSONDecodeError:
            print(f"  Fetch failed {url} -> {r.text}")
            return None
        except requests.exceptions.ConnectionError:
            print(f"  Fetch failed {url} -> connection refused")
            return None

    def get_mirrored_hosts(self):
        '''This function will retrieve all the *first time* connected or created image host databases in the
        current hop.
        Hop=0 should have only its self image host (hop0).
        Hop=1 should have its self and hop=0 image hosts (hop1, hop0).
        Hop=2 should have its self and hop=1, hop=2 image hosts (hop2, hop1, hop0).'''
        if(self.receiver):
            url = f"{self.http}localhost:{self.port}/api/v1/info"        
            try:
                r = requests.get(url)
                info = r.json()
                if(len(info['mirrored_hosts'][1:]) > 0):
                    print(f"  {info['mirrored_hosts'][1:]} in mirrored_hosts on {self.name}")
                    return info['mirrored_hosts'][1:]
                elif(len(info['mirrored_hosts'][:]) == 1):
                    print(f"  {self.name} has no mirrored hosts")
                return None
            except json.decoder.JSONDecodeError:
                print(f"  Mirrored hosts info fetch failed {url} -> {r.text}")
                return None
            except requests.exceptions.ConnectionError:
                print(f"  Mirrored hosts info fetch failed {url} -> connection refused")
                return None