Approved based on the test plan below.

## Test Plan
* ### Created a dummy collector that generates a known sequence of values (e.g. 1,2,3,4,...) at 1s collection interval.
#### **Action**: Creation of an incremental value python.d.plugin collector (`test_increment`) supporting various update_every in configuration file.
1. Supports various `update_every= 1,2,3,4,5,11,13` that can be set-up in the configuraton file or you can add any other update_every you want.
2. The collector runs each `update_every` configuration block as a job and creates a different chart with only one dimension (ie. chart_name = `test_increment_x1.increment_x1`, dimension_name = `x1`).
3. The configuration job title must follow the pattern `increment_xX` with `X = the desired update_every` (ie `increment_x5` for update_every = 5)
4. Test incremental python collector can be found [here](https://github.com/odynik/netdata/tree/vlad_pr11599/collectors/python.d.plugin/test_increment)

* ### Chart can be created with `store_first` or without.
#### **Action**: Tested with `store_first_entry` enabled/disabled

* ### Configure netdata to use dbengine.
#### **Action**: Tested in memory modes = `dbengine, alloc, ram, save`

* ### With netdata stopped, erase dbengine and metadata.
#### **Action**: The testbed environment is based on docker so for persistant memory modes such as `dbengine/save` the docker image/volume was destroyed and recreated from the original image for each test case. The test cases related with `ram` had the same treatment even if it was not necessary since a netdata agent restart would clean the memory.

* ### View chart in default dashboard. OR fetch the chart data via api/v1/data.
#### **Action**: Semi-automated testing was heavily based on the http (api/v1/data) netdata agent API. Manual testing as a user was based on dashboard view and log files.

## Test Results
### Expected outcome
[master](https://github.com/netdata/netdata/commit/fdc9f65260a35d648b81f8d72bddd6b6847a1c10): chart data was missing the first sample.

[PR11787](https://github.com/netdata/netdata/pull/11787): chart data includes the first sample.

`Note`: This behavior only affected the very first entry in the chart, immediately after initial creation. Upon resuming an archived chart, the subsequent initial values were correct.

| Params                                   | [master](https://github.com/netdata/netdata/commit/fdc9f65260a35d648b81f8d72bddd6b6847a1c10) | [master](https://github.com/netdata/netdata/commit/fdc9f65260a35d648b81f8d72bddd6b6847a1c10) | [PR11787](https://github.com/netdata/netdata/pull/11787) | [PR11787](https://github.com/netdata/netdata/pull/11787) |
| ---------------------------------------- | ---------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------- | ---------------------------------------------------------- | ---------------------------------------------------------- |
| store\_first\_entry                      | 1/0                                                                                            | 1/0                                                                                            | 1/0                                                        | 1/0                                                        |
| memory\_mode                             | dbengine                                                                                       | save/ram/alloc                                                                                 | dbengine                                                   | save/ram/alloc                                             |
| absolute query timestamps (after/before) | FALSE/TRUE                                                                                     | FALSE/TRUE                                                                                     | FALSE/TRUE                                                 | FALSE/TRUE                                                 |
| update\_every                            | 1,2,3,4,5,11,13                                                                                | 1,2,3,4,5,11,13                                                                                | 1,2,3,4,5,11,13                                            | 1,2,3,4,5,11,13                                            |
| results                                  | <span style="color:red">**FAIL**</span>: The first sample is saved but is NOT fetched                                             | <span style="color:red">**FAIL**</span>: The first sample is saved but is NOT fetched                                             | <span style="color:green">**PASS**</span>: The first sample is saved AND fetched                | <span style="color:green">**PASS**</span>: The first sample is saved AND fetched                |
## Additional Information
### How netdata agent treats the first entry?
The first time a collector will try to deliver the first entry to the netdata agent,
1. Netdata agent will save the first entry only if you enable `store_first`. Otherwise, it will use the time-metadata of the first entry to synchronize the chart's real-time metadata variables.
2. As a result, the actual first sample saved in the netdata agent charts is the second entry. Now, in case you have enabled the `store_first` variable the second entry is a sample that may be subject to interpolation. Interpolation happens if a collector delivers the first sample (second entry) slower than the `update_every`.
3. During interpolation a ghost-interpolated sample MAY be inserted between first entry and second entry. In the test incremental collector the values could look like [first_entry:1, second_entry:1.7, third_entry:2]. This means that the test incremental collector delivered sample with value=2 later than the second update_every time-window.
4. On the other side, when a collector fetches samples faster than the update_every then the real-time behavior of the netdata agent will reject the redundant samples. In the test incremental collector this could result in outputs like this [1,3,4,5,6,7,...]. This means that the test incremental value collector fetched the sample with value = 2 in the same update_every-time-window as the first sample with value = 1.