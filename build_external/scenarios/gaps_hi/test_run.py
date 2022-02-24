import functools, itertools, math, operator, os, re, sys, time
import AgentTest
import numpy as np
# from testcategories.interruptions import *
# from testcategories.startupinorder import *
# from testcategories.firstsample import *
from testcategories.replication import *

# Current directory
me   = os.path.abspath(sys.argv[0])
base = os.path.dirname(me)

#color code
HEADER = '\033[95m'
OKBLUE = '\033[94m'
OKCYAN = '\033[96m'
OKGREEN = '\033[92m'
WARNING = '\033[93m'
FAIL = '\033[91m'
ENDC = '\033[0m'
BOLD = '\033[1m'
UNDERLINE = '\033[4m'

# Define Netdata agent testing parameters
uuid_regex = "\b[0-9a-f]{8}\b-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-\b[0-9a-f]{12}\b"
# guid_base = np.zeros(32,dtype=int)
# guids = np.array([(guid_base + i) for i in range(numofagents)])
# guids = np.array([np.array_split(guid, [9,13,17,21]) for guid in guids])
# part_merge = lambda guid: [np.array2string(part) for part in guid]
# guids = map(part_merge, guids)
# guids = [np.array2string(guid, separator='-') for guid in guids]
# print(guids)
# guid_base = np.fromstring("11111111-1111-1111-1111-111111111111", dtype=int, sep='-')
# api_key_base = np.fromregex("00000000-0000-0000-0000-000000000000", uuid_regex, dtype=int)
# api_keys = api_key_base
# api_keys = [np.add(api_key_base, (api_key_base[4] + i)) for i in range(numofagents)]
# api_keys = [np.array2string(key, separator='-').strip() for key in api_keys]

numofagents = 2
port_base = 20000
agents = ["hop"+str(i) for i in range(numofagents)]
ports = np.arange(port_base, (port_base + numofagents), 1)
# protocols_version = {"master": 3, "rep":4}
protocols_version = {"rep":4}
# memory_modes = ["ram", "dbengine"] # add here any extra memory modes for testing
memory_modes = ["dbengine"] # add here any extra memory modes for testing

# Combinations based on maths or most common use cases.
mm_combinations = list(itertools.product(memory_modes, repeat=numofagents))
# mm_combinations = [
#     ("dbengine", "dbengine", "dbengine")
# ]

pv_combinations = list(itertools.product(protocols_version.keys(), repeat=numofagents))
# pv_combinations = [
    # ("rep", "rep", "rep"),
    # ("master", "rep", "rep"),
    # # ("rep", "rep", "master"),
    # ("rep", "master", "rep"),
    # ("master", "rep", "master"),
    # ("master", "master", "master"),
    # ("rep", "rep", "rep")
    # ]
    
# print("\GUID numbers(" + str(len(guids)) + "):")
# print(*guids)
print("\nPort numbers(" + str(len(ports)) + "):")
print(*ports, sep=",")
print("\nMemory Mode combinations(" + str(len(mm_combinations)) + "):")
print(*mm_combinations, sep="\n")
print(f"\nProtocol Version combinations(" + str(len(pv_combinations)) + "):")
print(*pv_combinations, sep="\n")


def add_agent_node(state, name, guid, port, mm, api_key, tls_on=False, stream_version={"rep":4}):
    agent_node = state.add_node(name)
    agent_node.port = port
    agent_node.guid = guid
    agent_node.db_mode = mm
    agent_node.api_key = api_key
    agent_node.tls = tls_on
    agent_node.stream_version = stream_version
    return agent_node

hop_configuration =[]
hop_test_cases = [
    Hop01RuntimeShortReconnections,
    Hop01RuntimeShortRestartChild,
    Hop01RuntimeShortRestartParent
    # Hop0RetrieveSamplesInorder,
    # Hop01RetrieveSamplesInorder,
    # Hop012RetrieveFirstSample,
    # Hop012RetrieveLimitedFirstSample,
    # Hop012RetrieveSamplesInorder,
    # Hop012ReStartHop1SamplesInorder
    # AscendingOrderHopStart,
    # DescendingOrderHopStart,
    # MixedOrderHopStart,
    # Hop1ShortRestartInSecs,
    # Hop1ShortRestartGTmin,
    # Hop2RestartOverlapsHop1Restart,
    # Hop1LongRestartHop0SenderBufferOverflow,
    # AscendingOrderHopStart,
    # DescendingOrderHopStart,
    # MixedOrderHopStart,
    # Hop0ShortRestartShort_stopnet,
    # Hop0ShortRestartLong_stopnet,
    # Hop0LongRestartShort_stopnet,
    # Hop0LongRestartLong_stopnet,
    # Hop1ShortRestartShort_stopnet,
    # Hop1ShortRestartLong_stopnet,
    # Hop1LongRestartShort_stopnet,
    # Hop1LongRestartLong_stopnet,
    # Hop0ShortRestartShort_kill,
    # Hop0ShortRestartLong_kill,
    # Hop0LongRestartShort_kill,
    # Hop0LongRestartLong_kill,
    # Hop1ShortRestartShort_kill,
    # Hop1ShortRestartLong_kill,
    # Hop1LongRestartShort_kill,
    # Hop1LongRestartLong_kill,
    # Hop2RestartOverlapsHop1Restart,
]    

for stream_version in pv_combinations:
    for mm in mm_combinations:
        # # Test parameter combinations
        # print(f"The current pv: {stream_version}")
        # params = list(zip(stream_version, mm))
        # print(f"\nTest parameter combinations(" + str(len(stream_version)*len(mm_combinations)) + "):")
        # print(*params, sep="\n")

        tls_on = False
        tc_folder_title = "".join([f"{mm[i]}#{protocols_version[stream_version[i]]}_" for i in range(numofagents)]).rstrip("_")
        tc_print_label = "".join([f"hop{i}="+WARNING+f"{mm[i]};"+ENDC+OKBLUE+f"{protocols_version[stream_version[i]]}"+ENDC+" -> " for i in range(numofagents)]).rstrip(" ->")
        # print(f"The title: {tc_folder_title}")
        # print(f"The labels: {tc_print_label}\n")

        state = AgentTest.State(os.path.join(base,"working"), tc_folder_title, tc_print_label)
        
        # Make the function add_agent_node with *args and create tuples with the parameters for each test.
        hop0 = add_agent_node(state, agents[0], "11111111-1111-1111-1111-111111111111", 20000, mm[0], "00000000-0000-0000-0000-000000000000", tls_on, {stream_version[0]:protocols_version[stream_version[0]]})
        hop1 = add_agent_node(state, agents[1], "22222222-2222-2222-2222-222222222222", 20001, mm[1], "00000000-0000-0000-0000-000000000001", tls_on, {stream_version[1]:protocols_version[stream_version[1]]})
        # hop2 = add_agent_node(state, agents[2], "33333333-3333-3333-3333-333333333333", 20002, mm[2], "00000000-0000-0000-0000-000000000002", tls_on, {stream_version[2]:protocols_version[stream_version[2]]})
        
        hop1.receive_from_api_key = hop0.api_key
        # hop2.receive_from_api_key = hop1.api_key
        hop0.stream_to(hop1)
        # hop1.stream_to(hop2)

        hop_configuration.append(state)

for tstcase in hop_test_cases:
    for state in hop_configuration:
        clean_state = state.copy()
        clean_state.nodes["hop0"].stream_to(clean_state.nodes["hop1"])
        # clean_state.nodes["hop1"].stream_to(clean_state.nodes["hop2"])
        clean_state.wrap(tstcase)