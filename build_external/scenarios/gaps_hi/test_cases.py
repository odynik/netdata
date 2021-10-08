import functools, itertools, math, operator, os, re, sys, time
import AgentTest
import argparse

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
    state.end_checks.append( lambda: state.check_sync_hops("hop1", max_score=20,max_pre=10))
    state.end_checks.append( lambda: state.check_sync_hops("hop2", max_score=20,max_pre=10))
    # pylint: disable-msg=W0108
    state.post_checks.append( lambda: state.check_rep() )

# Hop1 goes down and restarts in a long period of time
# Try to make child sender thread to overflow
def Hop1LongRestartHop0SenderBufferOverflow(state):
    state.start("hop0")
    state.start("hop2")
    state.start("hop1")
    state.wait_up("hop0")
    state.wait_up("hop1")
    state.wait_up("hop2")
    state.wait_connected("hop0", "hop1")
    state.wait_connected("hop1", "hop2")
    time.sleep(10)
    state.kill("hop1")
    time.sleep(240) # 4minutes wait - to make test faster change the default history - 3990 samples to 200 samples
    state.restart("hop1")
    state.wait_isparent("hop1")
    time.sleep(20)
    state.end_checks.append( lambda: state.check_sync_hops("hop1", max_pre=10))
    state.end_checks.append( lambda: state.check_sync_hops("hop2", max_pre=10))
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