<!--
Describe the change in summary section, including rationale and design decisions.
Include "Fixes #nnn" if you are fixing an existing issue.

In "Component Name" section write which component is changed in this PR. This
will help us review your PR quicker.

In "Test Plan" provide enough detail on how you plan to test this PR so that a reviewer can validate your tests. If our CI covers sufficient tests, then state which tests cover the change.

If you have more information you want to add, write them in "Additional
Information" section. This is usually used to help others understand your
motivation behind this change. A step-by-step reproduction of the problem is
helpful if there is no related issue.
-->

##### Summary
| Fixes | Master | PR#9832 | Current Changes |
| - | - | - | - |
| **Image host creation** | It can create image hosts of netdata agents in a cascading architecture but will have gaps during reconnection/restarts of the proxy agent. | Can create image host ONLY in a hop0-hop1(parent-child) architecture | It can create image hosts of netdata agents in a cascading architecture but can eliminate gaps during reconnection/restarts of the proxy agent. The gaps fililng during replication if configurable through the stream.conf file. |
| **Handling empty slots and misaligments** | Curreclty skipping the empty slots with empty values | Partially handling the empty slots | Handling the empty slots in dbengine mode. More modes are also improved but not tested. |
| **Detection mechanism for last replication request** | N/A | N/A |  Supports the transmission of synchronized data samples. |
| **Sender buffer overflow**| N/A | Loading the sender buffer with >(1MB/2.0) replicated data would cause and overflow and the restart of the connection | The overflow protection mechanism is triggered but now the sender thread is stops reading replication data and start sending data from the sender buffer until it sends just have of the send buffer size|
| **Streaming version compatibility**| Supports stream_version 3 and below in all the agents | Supports hop0(stream_version=3/4) and hop1(ONLY stream_version=4) (No hop2)| Supports hop0(stream_version = 3/4) -> (stream_version=3/4)                                                                                      -> (ONLY stream_version=4)|

Fixes,
* **Image host creation**: for hop0 to hop2 through proxy hop1
* **Synchronization mechanism** to detect the receipt of the last replication request
* **Handling empty slots and misaligments in receiving samples**. We don't transmit empty slots and the receiver is responsible to handle empty slots in memory.
* **Sender buffer overflow** was occuring during the restart of hop1 (proxy) after a long down time. Restarting the connection would flash the data in the cbuffer of the sender thread. Now the sender thread stops reading replication commands until it sends half of the size of the buffer.
* **Streaming version compatibility**: The streaming replication protocol is stream_version = 4. The current streaming protocol is stream_version = 3. The compatibility between agent with different streaming protocols is shown in table

##### Component Name
Agent streaming Replication protocol (stream_version = 4)

##### Test Plan

### Cascading Architecture for Netdata agent connection
|                | hop 0             | hop 1    | hop 2    |
| -------------- | ----------------- | -------- | -------- |
| memory mode    | ram/save/dbengine | dbengine | dbengine |
| stream_version | 3/4               | 3/4      | 4        |
| tls            | off               | off      | off      |

<!---
Provide enough detail so that your reviewer can understand which test-cases you
have covered, and recreate them if necessary. If sufficient tests are covered
by our CI, then state which tests cover the change.
-->

##### Additional Information
This code is bind with the testing environment in the PR# . The docker-python testbed is adopted to host the casciding architecture of the netdata agent connections. It was incrementally developed from the PR#9890 with the goal to make it more generic and be able to accomodate a lot of different test cases for netdata.

#### Future work
1. Heavily tested