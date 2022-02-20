import time

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
    #################### END OF TEST CASES ####################