import time

#################### BEGINNING OF TEST CASES ####################
#____PR#11675 Send clabels_______#
# 1. Simply retrieve clabels for hop1 <- hop0 connection. (Hop0, Hop1) with PR#11675 code
def Hop01RetrieveCLabels(state):
    state.start("hop0")
    state.wait_up("hop0")
    state.start("hop1")
    state.wait_up("hop1")
    state.wait_connected("hop0", "hop1")
    time.sleep(20)
    # state.end_checks.append( lambda: state.check_sync_hops("hop1", max_pre=0))
    state.end_checks.append( lambda: state.compare_clabels("hop1"))
    state.post_checks.append( lambda: state.check_clabels() )    

# 2. Simply retrieve clabels for hop1 <- hop0 connection. (Hop0 - master, Hop1 - PR#11675)
# 3. Simply retrieve clabels for hop1 <- hop0 connection. (Hop0 - PR#11675, Hop1 - master)

# 4. Forward clabels to gparent
# 5. Compatibility version between protocols
# 6. No retransmission during runtime....check if restarts is enough to retransmit the data

# Simply retrieve clabels for hop2 <- hop1 <- hop0 connection
def Hop012RetrieveCLabels(state):
    state.start("hop0")
    state.wait_up("hop0")
    state.start("hop1")
    state.wait_up("hop1")
    state.start("hop2")
    state.wait_up("hop2")    
    state.wait_connected("hop0", "hop1")
    state.wait_connected("hop1", "hop2")
    time.sleep(20)
    # state.end_checks.append( lambda: state.check_sync_hops("hop1", max_pre=10))
    # state.end_checks.append( lambda: state.check_sync_hops("hop2", max_pre=10))
    # state.end_checks.append( lambda: state.get_api_data("hop2"))
    state.end_checks.append( lambda: state.compare_clabels("hop1"))
    state.end_checks.append( lambda: state.compare_clabels("hop2"))
    state.post_checks.append( lambda: state.check_clabels())

def Hop012RetrieveCLabelsHop1Restart(state):
    state.start("hop0")
    state.wait_up("hop0")
    state.start("hop1")
    state.wait_up("hop1")
    state.start("hop2")
    state.wait_up("hop2")    
    state.wait_connected("hop0", "hop1")
    state.wait_connected("hop1", "hop2")
    time.sleep(5)
    state.stop("hop1")
    time.sleep(10)
    state.restart("hop1")
    state.wait_isparent("hop1")
    time.sleep(20)
    # state.end_checks.append( lambda: state.check_sync_hops("hop1", max_pre=10))
    # state.end_checks.append( lambda: state.check_sync_hops("hop2", max_pre=10))
    # state.end_checks.append( lambda: state.get_api_data("hop2"))
    state.end_checks.append( lambda: state.compare_clabels("hop1"))
    state.end_checks.append( lambda: state.compare_clabels("hop2"))
    state.post_checks.append( lambda: state.check_clabels())    
    #################### END OF TEST CASES ####################