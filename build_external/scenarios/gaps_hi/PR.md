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
This section describes the Docker-Python Testbed for integration testing of the streaming replication protocol. Original PR#9890 and credits to Andrew Moss.

| TC categories | PR#9890 | Current Changes |
| - | - | - |
| **Normal Operation with differnt netdata agent start order** | Possible | sfsafsd |
| **Hop1 Restart in differenttime intervals** | Possible | Cascading arch |
| **TLS on/off** | N/a | Cascading arch |
| **Stream Version Compatibility** | N/a | Cascading arch |
| **Stream Version Compatibility** | N/a | Cascading arch |

Extra feature development for parameters in netdata agent,
* **Separate TC files for each test category**: 
* **One function to add nodes in the test env**: 
* **Parameterization of the netdata.conf and stream.conf files**: 

##### Component Name
Agent streaming Replication protocol (stream_version = 4)

##### Test Plan
Writing tests to test the test plan is time consuming since the development of the source code is the mayor issue. At the moment, the integrity of the test environment is based on the pendantic hours spent monitoring the code and the testing environment to understand the source of the integration issues.

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


## Install the test environment
## Run the tests
## Twick the test cases
## Re-compile a change in source code
## Re-run the tests
:cookie: