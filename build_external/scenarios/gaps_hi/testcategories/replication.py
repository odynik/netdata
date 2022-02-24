import time
#################### BEGINNING OF TEST CASES ####################
# Testing end-user scenarios for replication

### Section 1 - Parent Child architecture
# 1.1 Runtime short network disconnections
# Description: Start the child, start the parent. Disconnect the network. Reconnect the network.
# Expected Results: 
def Hop01RuntimeShortReconnections(state):
    state.start("hop0")
    state.wait_up("hop0")
    state.start("hop1")
    state.wait_up("hop1")
    state.wait_connected("hop0", "hop1")
    time.sleep(5)
    state.stop_net("hop0")
    time.sleep(5)
    state.start_net("hop0")
    state.wait_connected("hop0", "hop1")
    state.post_checks.append( lambda: state.check_rep())

# 1.2 Start the child, start the parent. Shutdown the child. Start the child.
# Description: Start the child, start the parent. Disconnect the network. Reconnect the network.
# Expected Results: 
def Hop01RuntimeShortRestartChild(state):
    state.start("hop0")
    state.wait_up("hop0")
    state.start("hop1")
    state.wait_up("hop1")
    state.wait_connected("hop0", "hop1")
    time.sleep(5)
    state.stop("hop0")
    time.sleep(5)
    state.restart("hop0")
    state.wait_connected("hop0", "hop1")
    state.post_checks.append( lambda: state.check_rep())

# 1.3 Start the child, start the parent. Shutdown the parent. STart the parent.
# Description: Start the child, start the parent. Disconnect the network. Reconnect the network.
# Expected Results: 
def Hop01RuntimeShortRestartParent(state):
    state.start("hop0")
    state.wait_up("hop0")
    state.start("hop1")
    state.wait_up("hop1")
    state.wait_connected("hop0", "hop1")
    time.sleep(5)
    state.stop("hop1")
    time.sleep(5)
    state.restart("hop1")
    state.wait_connected("hop0", "hop1")
    state.post_checks.append( lambda: state.check_rep())

#################### END OF TEST CASES ####################
