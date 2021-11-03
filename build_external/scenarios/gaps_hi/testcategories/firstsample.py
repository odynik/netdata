import time

#################### BEGINNING OF TEST CASES ####################
#____PR#11599 First Sample is missing from JSON API calls_______#
# Simply retrieve the inorder samples.
def Hop0RetrieveSamplesInorder(state):
    state.start("hop0")
    state.wait_up("hop0")
    time.sleep(10)
    state.end_checks.append( lambda: state.get_api_data("hop0"))
    state.post_checks.append( lambda: state.check_rep() )

# Simply retrieve a hop1 <- hop0 connection
def Hop01RetrieveSamplesInorder(state):
    state.start("hop0")
    state.wait_up("hop0")
    state.start("hop1")
    state.wait_up("hop1")
    state.wait_connected("hop0", "hop1")
    time.sleep(20)
    state.end_checks.append( lambda: state.check_sync_hops("hop1", max_pre=0))
    state.post_checks.append( lambda: state.check_rep() )    

# Simply retrieve a hop1 <- hop0 connection
def Hop012RetrieveSamplesInorder(state):
    state.start("hop0")
    state.wait_up("hop0")
    state.start("hop1")
    state.wait_up("hop1")
    state.start("hop2")
    state.wait_up("hop2")    
    state.wait_connected("hop0", "hop1")
    state.wait_connected("hop1", "hop2")
<<<<<<< HEAD
    time.sleep(10)
=======
    time.sleep(5)
>>>>>>> 9ec5f5a3d9e1463eee9136ddd7e3dfe8644525c4
    # state.end_checks.append( lambda: state.check_sync_hops("hop1", max_pre=1))
    # state.end_checks.append( lambda: state.check_sync_hops("hop2", max_pre=1))
    state.end_checks.append( lambda: state.compare_increment_value_and_duration("hop0", "memindex_testmemindex.inorder"))
    state.end_checks.append( lambda: state.compare_increment_value_and_duration("hop1", "memindex_testmemindex.inorder"))
    state.end_checks.append( lambda: state.compare_increment_value_and_duration("hop2", "memindex_testmemindex.inorder"))
    # state.end_checks.append( lambda: state.compare_increment_value_and_duration("hop0", "system.uptime"))
    # state.end_checks.append( lambda: state.compare_increment_value_and_duration("hop1", "system.uptime"))
    # state.end_checks.append( lambda: state.compare_increment_value_and_duration("hop2", "system.uptime"))

# Restart hop1
def Hop012ReStartHop1SamplesInorder(state):
    state.start("hop0")
    state.wait_up("hop0")
    state.start("hop1")
    state.wait_up("hop1")
    state.start("hop2")
    state.wait_up("hop2")    
    state.wait_connected("hop0", "hop1")
    state.wait_connected("hop1", "hop2")
    time.sleep(10)
    state.destroy("hop1")
    time.sleep(10)
    state.start("hop1")
    state.wait_isparent("hop1")
    time.sleep(15)
    state.end_checks.append( lambda: state.compare_increment_value_and_duration("hop0", "memindex_testmemindex.inorder"))
    state.end_checks.append( lambda: state.compare_increment_value_and_duration("hop1", "memindex_testmemindex.inorder"))
    state.end_checks.append( lambda: state.compare_increment_value_and_duration("hop2", "memindex_testmemindex.inorder"))
    # state.end_checks.append( lambda: state.compare_increment_value_and_duration("hop0", "system.uptime"))
    # state.end_checks.append( lambda: state.compare_increment_value_and_duration("hop1", "system.uptime"))
    # state.end_checks.append( lambda: state.compare_increment_value_and_duration("hop2", "system.uptime"))
    #################### END OF TEST CASES ####################