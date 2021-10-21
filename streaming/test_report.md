## Review Suggestions
1. [[Already Fixed](https://github.com/netdata/netdata/commit/69e58c74daa4fd0a52ea34b718eaf9506b76a2f1)] The CLABEL_COMMIT command is being transmitted everytime, even for the charts that they don't have CLABELS.
2. Clean compilation warnings in ```pluginsd_parser.c line 162```. See ```Files Changed``` comments

## Terminology
### Hop Level of netdata agents
From now on, I am going to use the hop level of netdata agents to indicate the relation of the netdata agents connection instead of the child-parent-grandparent terms. See table below,

|Child|Parent/Proxy|GrandParent|
|--- |--- |--- |
|Hop0|Hop1|Hop2|

### Success in current tests
``` Sucess: The CLABEL was sucessfully transmitted and received through the streaming protocol between two netdata agents. In addition, it was succesfully collected from the netdata agent web api and matched the transmitted values.```

### Expected Behavior in current tests
``` Expected Behavior (EB): The CLABEL is introduced as another protocol version so it is not expected to be transmitted or received in netdata agents with streaming version protocol lower or equal to 3.```

## Test cases
All the tests are based on an end-to-end testbed made with docker and the clabels are collected from the netdata web api.
### 1. Hop0 and Hop1 with [PR#11657](https://github.com/netdata/netdata/pull/11675#issue-1030050637) code
#### Results
|Hop0|Hop1|Hop0 & Hop1 Code version|Negotiated stream_version|Result|
|--- |--- |---|--- |--- |
|dbengine/save/ram/none|dbengine/save/ram/none|[PR#11657](https://github.com/netdata/netdata/pull/11675#issue-1030050637)|4|Success|

### 2. Hop0 with [master-0882ed0](https://github.com/odynik/netdata/commit/0882ed03b4000b6f9e1f64743321e4cd6e2aa39f) and Hop1 with [PR#11657](https://github.com/netdata/netdata/pull/11675#issue-1030050637) code

#### Results
|Hop0|Hop1|Hop0 Code version|Hop1 Code version|Negotiated stream_version|Result|
|--- |--- |---|--- |--- |--- |
|dbengine/save/ram/none|dbengine/save/ram/none|[master-0882ed0](https://github.com/odynik/netdata/commit/0882ed03b4000b6f9e1f64743321e4cd6e2aa39f)|[PR#11657](https://github.com/netdata/netdata/pull/11675#issue-1030050637)|3|Expected Behavior|

### 3. Hop0 with [PR#11657](https://github.com/netdata/netdata/pull/11675#issue-1030050637) and Hop1 with code [master-0882ed0](https://github.com/odynik/netdata/commit/0882ed03b4000b6f9e1f64743321e4cd6e2aa39f)

#### Results
|Hop0|Hop1|Hop0 Code version|Hop1 Code version|Negotiated stream_version|Result|
|--- |--- |---|--- |--- |--- |
|dbengine/save/ram/none|dbengine/save/ram/none|[PR#11657](https://github.com/netdata/netdata/pull/11675#issue-1030050637)|[master-0882ed0](https://github.com/odynik/netdata/commit/0882ed03b4000b6f9e1f64743321e4cd6e2aa39f)|3|Expected Behavior|

## Testing cascading architectures with netdata agents
Cascading architecture: ```Hop0 -> Hop1 -> Hop2```. Test cases include,
1. Normal connection of the netdata agents
2. Stop and restart of Hop1.
## Results
|                | Hop 0           | Negotiated Stream version | Hop 1    | Negotiated Stream version | Hop 2    | Result  |
|----------------|-----------------|---------------------------|----------|---------------------------|----------|---------|
| memory mode    |   dbengine/save/ram/none              |                           | dbengine/save/ram/none |                           | dbengine/save/ram/none |  |
| Code version   | [PR#11657](https://github.com/netdata/netdata/pull/11675#issue-1030050637) |            4               | [PR#11657](https://github.com/netdata/netdata/pull/11675#issue-1030050637)      |              4             | [PR#11657](https://github.com/netdata/netdata/pull/11675#issue-1030050637)      |   Success      |
| Code version   | [master-0882ed0](https://github.com/odynik/netdata/commit/0882ed03b4000b6f9e1f64743321e4cd6e2aa39f) |            3               | [PR#11657](https://github.com/netdata/netdata/pull/11675#issue-1030050637)      |              4             | [PR#11657](https://github.com/netdata/netdata/pull/11675#issue-1030050637)      |   Success      |
| Code version   | [PR#11657](https://github.com/netdata/netdata/pull/11675#issue-1030050637) |            3               | [master-0882ed0](https://github.com/odynik/netdata/commit/0882ed03b4000b6f9e1f64743321e4cd6e2aa39f)      |              3             | [PR#11657](https://github.com/netdata/netdata/pull/11675#issue-1030050637)      |   Expected Behavior      |
| Code version   | [PR#11657](https://github.com/netdata/netdata/pull/11675#issue-1030050637) |            4               | [PR#11657](https://github.com/netdata/netdata/pull/11675#issue-1030050637)      |              3             | [master-0882ed0](https://github.com/odynik/netdata/commit/0882ed03b4000b6f9e1f64743321e4cd6e2aa39f)      |   Success      |
| Code version   | [master-0882ed0](https://github.com/odynik/netdata/commit/0882ed03b4000b6f9e1f64743321e4cd6e2aa39f) |            3               | [PR#11657](https://github.com/netdata/netdata/pull/11675#issue-1030050637)      |              3             | [master-0882ed0](https://github.com/odynik/netdata/commit/0882ed03b4000b6f9e1f64743321e4cd6e2aa39f)      |   Expected Behavior      |