import functools, itertools, math, operator, os, re, sys, time
import AgentTest
import argparse

me   = os.path.abspath(sys.argv[0])
base = os.path.dirname(me)
parser = argparse.ArgumentParser()
parser.add_argument('pattern', nargs='?', default=None)
parser.add_argument('--child', nargs='?', default=None)
parser.add_argument('--parent', nargs='?', default=None)
args = parser.parse_args()

# agents = ["hop0", "hop1", "hop2"] # add here any extra agent
numofagents = 3
agents = ["hop"+str(i) for i in range(numofagents)]
numofagents = len(agents)
memory_modes = ["dbengine", "save"] # add here any extra memory mode for testing
# mm_combinations = list(itertools.product(memory_modes, repeat=numofagents))
mm_combinations = [
    ("dbengine", "dbengine", "dbengine"),
    ("save", "dbengine", "dbengine"),
    ("ram", "dbengine", "dbengine")
    ]
print("Memory Mode combinations(" + str(len(mm_combinations)) + "): \n")
# print(*mm_combinations, sep="\n")

def add_agent_node(state, name, guid, port, mm, api_key, tls_on=False):
    agent_node = state.add_node(name)
    agent_node.port = port
    agent_node.guid = guid
    agent_node.db_mode = mm
    agent_node.api_key = api_key
    agent_node.tls = tls_on
    return agent_node

#################### BEGINNING OF TEST CASES ####################
#____Normal Operation/Start hops in asc/desc(eding) order_______#
# Start hops in descending order - hop2 -> hop1 -> hop0
def DescendingOrderHopStart(state):
    state.start("hop2")
    state.wait_up("hop2")
    state.start("hop1")
    state.wait_up("hop1")
    state.start("hop0")
    state.wait_up("hop0")
    state.wait_connected("hop0", "hop1") 
    state.wait_connected("hop1", "hop2")
    time.sleep(20)
    state.end_checks.append( lambda: state.check_sync_hops("hop1", max_pre=10))
    state.end_checks.append( lambda: state.check_sync_hops("hop2", max_pre=10))
    state.post_checks.append( lambda: state.check_rep() )

# Start hops in ascending order - hop0 -> hop1 -> hop2
def AscendingOrderHopStart(state):
    state.start("hop0")
    state.wait_up("hop0")
    state.start("hop1")
    state.wait_up("hop1")
    state.start("hop2")
    state.wait_up("hop2")
    state.wait_connected("hop0", "hop1")
    state.wait_connected("hop1", "hop2")
    time.sleep(20)
    state.end_checks.append( lambda: state.check_sync_hops("hop1", max_pre=10))
    state.end_checks.append( lambda: state.check_sync_hops("hop2", max_pre=10))
    state.post_checks.append( lambda: state.check_rep() )    

# Start hops in mixed order - hop1 -> hop2 -> hop0
def MixedOrderHopStart(state):
    state.start("hop1")
    state.wait_up("hop1")
    state.start("hop2")
    state.wait_up("hop2")    
    state.start("hop0")
    state.wait_up("hop0")
    state.wait_connected("hop0", "hop1")
    state.wait_connected("hop1", "hop2")
    time.sleep(20)
    state.end_checks.append( lambda: state.check_sync_hops("hop1", max_pre=10))
    state.end_checks.append( lambda: state.check_sync_hops("hop2", max_pre=10))
    state.post_checks.append( lambda: state.check_rep() )    

#____Interruption/Break/Only one hop is restarted_______#
# Hop1 goes down and restarts in seconds
def Hop1ShortRestartInSecs(state):
    state.start("hop0")
    state.wait_up("hop0")
    state.start("hop2")
    state.wait_up("hop2")    
    state.start("hop1")
    state.wait_up("hop1")
    state.wait_connected("hop0", "hop1") 
    state.wait_connected("hop1", "hop2")
    time.sleep(5)
    state.stop("hop1")
    time.sleep(10)
    state.restart("hop1")
    state.wait_isparent("hop1")
    time.sleep(20)
    # pylint: disable-msg=W0622
    state.end_checks.append( lambda: state.check_sync_hops("hop1",max_pre=10))
    state.end_checks.append( lambda: state.check_sync_hops("hop2",max_pre=10))
    # pylint: disable-msg=W0108
    state.post_checks.append( lambda: state.check_rep() )

# Hop1 goes down and restarts in more than a minute
def Hop1ShortRestartGTmin(state):
    state.start("hop0")
    state.wait_up("hop0")
    state.start("hop2")
    state.wait_up("hop2")    
    state.start("hop1")
    state.wait_up("hop1")
    state.wait_connected("hop0", "hop1") 
    state.wait_connected("hop1", "hop2")
    time.sleep(10)
    state.kill("hop1")
    time.sleep(70)
    state.restart("hop1")
    state.wait_isparent("hop1")
    time.sleep(20)
    # pylint: disable-msg=W0622
    state.end_checks.append( lambda: state.check_sync_hops("hop1",max_pre=10))
    state.end_checks.append( lambda: state.check_sync_hops("hop2",max_pre=10))
    # pylint: disable-msg=W0108
    state.post_checks.append( lambda: state.check_rep() )

# Hop1 is going down after few seconds Hop2 is going down and the restart is overlapped
def Hop2RestartOverlapsHop1Restart(state):
    state.start("hop0")
    state.wait_up("hop0")
    state.start("hop1")
    state.wait_up("hop1")    
    state.start("hop2")
    state.wait_up("hop2")
    state.wait_connected("hop0", "hop1") 
    state.wait_connected("hop1", "hop2")
    time.sleep(10)
    state.kill("hop1")
    time.sleep(5)
    state.kill("hop2")
    time.sleep(5)
    state.restart("hop1")
    time.sleep(5)
    state.restart("hop2")
    state.wait_isparent("hop1")
    state.wait_isparent("hop2")
    time.sleep(20)
    # pylint: disable-msg=W0622
    state.end_checks.append( lambda: state.check_sync_hops("hop1",max_pre=10))
    state.end_checks.append( lambda: state.check_sync_hops("hop2",max_pre=10))
    # pylint: disable-msg=W0108
    state.post_checks.append( lambda: state.check_rep() )    
#################### END OF TEST CASES ####################

hop_configuration =[]
hop_test_cases = [
    # AscendingOrderHopStart,
    # DescendingOrderHopStart,
    # MixedOrderHopStart,
    Hop1ShortRestartInSecs,
    Hop1ShortRestartGTmin,
    # Hop2RestartOverlapsHop1Restart,
]    

for (L0_mm, L1_mm, L2_mm) in mm_combinations:
    state = AgentTest.State(os.path.join(base,"working"), f"{L0_mm}_{L1_mm}_{L2_mm}", f"hop0={L0_mm} hop1={L1_mm} hop2={L2_mm}")
    
    tls_on = False
    hop0 = add_agent_node(state, agents[0], "11111111-1111-1111-1111-111111111111", 20000, L0_mm, "00000000-0000-0000-0000-000000000000", tls_on)
    hop1 = add_agent_node(state, agents[1], "22222222-2222-2222-2222-222222222222", 20001, L1_mm, "00000000-0000-0000-0000-000000000001", tls_on)
    hop2 = add_agent_node(state, agents[2], "33333333-3333-3333-3333-333333333333", 20002, L2_mm, "00000000-0000-0000-0000-000000000002", tls_on)
    
    hop1.receive_from_api_key = hop0.api_key
    hop2.receive_from_api_key = hop1.api_key
    hop0.stream_to(hop1)
    hop1.stream_to(hop2)

    hop_configuration.append(state)

for tstcase in hop_test_cases:
    for state in hop_configuration:
        clean_state = state.copy()
        clean_state.nodes["hop0"].stream_to(clean_state.nodes["hop1"])
        clean_state.nodes["hop1"].stream_to(clean_state.nodes["hop2"])
        clean_state.wrap(tstcase)
