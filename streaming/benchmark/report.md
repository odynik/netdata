# Compression results for Netdata Streaming protocol
The protocol commands are described here because they are the constructive elements of the text files/buffers that need to be (de)compressed during transmission.

The netdata agent protocol has different version. the versions are `v1, v2, v3, v4, ..v5 `.
## Common protocol commands in all versions
1. **VARIABLE**: 
2. **HOST**:
3. **LABEL**:
4. **OVERWRITE**:
7. **CHART**:
8. **DIMENSION**:
9. **CLAIMED_ID**:
10. **FLUSH**:
11. **DISABLE**
12. **GUID**
13. **CONTEXT**
14. **TOMBSTONE**


## Version specific netdata agent protocol commands
## Streaming Protocol in master
### Chart transmission block (v=1, 2, 3)
- **BEGIN** `chart_id` `t_diff_μsec` `<optional: t_diff_trust>`
    - **SET** `dimension1_id` `value`
    - **SET** `dimension2_id` `value`
    - **SET** `dimension4_id` `value`
    - **SET** `dimensionX_id` `value`
- **END**

## Streaming protocol (v=4)
1. **CLABEL**:
2. **CLABEL_COMMIT**:

## Upcoming Replication protocol (v=5)
### Chart transmission block
- **REPBEGIN** `chart_id` `t_ws` `t_wf` `t_we`
   - **REPDIM** `dimension1_id` `timestamp` `value1`
   - **REPDIM** `dimension1_id` `timestamp` `value2`
   - **REPDIM** `dimension1_id` `timestamp` `valuex`
   - **REPDIM** `dimension2_id` `timestamp` `value1`
   - **REPDIM** `dimension2_id` `timestamp` `value2`
   - **REPDIM** `dimensionX_id` `timestamp` `value1`
- **REPEND** `#samples` `#col` `#col_total`

# Netdata streaming/replication protocol requirements
1. ..
2. ..

| Connection type                                               | Connection speeds (Mbytes/sec) | Reduce the transmitted protocol bytes from the rep cmds | Compression Ratio | Total number of bytes in Retention period | Compressed Data | Number of threads | Time to transmit the whole size of retention data (secs) | Time to transmit the whole size of retention data (mins) | Time to transmit the whole size of retention data (hrs) |
| ------------------------------------------------------------- | ------------------------------ | ------------------------------------------------------- | ----------------- | ----------------------------------------- | --------------- | ----------------- | -------------------------------------------------------- | -------------------------------------------------------- | ------------------------------------------------------- |
| TCP, iPv4, Public INET - Private NET(ISP), (London to Athens) | 0.9                            | 1                                                       | 10                | 5424.107229                               | 542.4107229     | 1                 | 602.678581                                               | 10.04464302                                              | 0.1674107169                                            |
| TCP, iPv4, Public INET - Private NET(ISP), (London to Athens) | 1.25                           | 1                                                       | 10                | 5424.107229                               | 542.4107229     | 1                 | 433.9285783                                              | 7.232142972                                              | 0.1205357162                                            |
| TCP, iPv4, Public INET - Private NET(ISP), (London to Athens) | 2.75                           | 1                                                       | 10                | 5424.107229                               | 542.4107229     | 1                 | 197.2402629                                              | 3.287337714                                              | 0.05478896191                                           |
| TCP, iPv4, Public INET - Private NET(ISP), (London to Athens) | 5                              | 1                                                       | 10                | 5424.107229                               | 542.4107229     | 10                | 10.84821446                                              | 0.1808035743                                             | 0.003013392905                                          |


# File samples
Location: ```lzbench/samples/```
## rep_dict.log
Copy of the REPlication streaming commands between a hop1<->hop0 netdata agent architecture.

```
VARIABLE HOST active_processors = 4.0000000
VARIABLE HOST ipc_semaphores_arrays_max = 32000.0000000
VARIABLE HOST netfilter_conntrack_max = 262144.0000000
VARIABLE HOST tcp_mem_high = 753720.0000000
VARIABLE HOST tcp_max_connections = -1.0000000
VARIABLE HOST ipc_semaphores_max = 1024000000.0000000
VARIABLE HOST tcp_max_orphans = 32768.0000000
VARIABLE HOST tcp_mem_pressure = 502484.0000000
VARIABLE HOST tcp_mem_low = 376860.0000000
LABEL "_os_name" = 0 Ubuntu
LABEL "_os_version" = 0 18.04.5 LTS (Bionic Beaver)
LABEL "_kernel_version" = 0 4.15.0-159-generic
LABEL "_system_cores" = 0 4
LABEL "_system_cpu_freq" = 0 2599000000
LABEL "_system_ram_total" = 0 8363524096
LABEL "_system_disk_space" = 0 0
LABEL "_architecture" = 0 x86_64
LABEL "_virtualization" = 0 kvm
LABEL "_container" = 0 none
LABEL "_container_detection" = 0 systemd-detect-virt
LABEL "_virt_detection" = 0 systemd-detect-virt
LABEL "_is_k8s_node" = 0 false
LABEL "_aclk_impl" = 0 Legacy
LABEL "_aclk_proxy" = 0 none
LABEL "_is_parent" = 0 false
LABEL "_streams_to" = 0 192.168.10.34
OVERWRITE labels
CHART "netdata.web_thread1_cpu" "" "Netdata web server thread No 1 CPU usage" "milliseconds/s" "web" "netdata.web_cpu" "stacked" 132000 1 "   " "web" "stats"
DIMENSION "user" "user" "incremental" 1 1000 "  "
DIMENSION "system" "system" "incremental" 1 1000 "  "
REPBEGIN "netdata.web_thread1_cpu" 1636629513 1636629513 1636629514
REPDIM "user" 1636629513 957150494
REPDIM "system" 1636629513 16777216
REPEND 2 2687 2687
REPBEGIN "netdata.web_thread1_cpu" 1636628914 1636628914 1636629514
REPDIM "user" 1636629510 958035622
REPDIM "user" 1636629511 958344021
REPDIM "user" 1636629512 956982511
REPDIM "user" 1636629513 957150494
REPDIM "system" 1636629510 16777216
REPDIM "system" 1636629511 16777216
REPDIM "system" 1636629512 16777216
REPDIM "system" 1636629513 16777216
REPEND 8 2687 2687
CHART "netdata.plugin_cgroups_cpu" "" "Netdata CGroups Plugin CPU usage" "milliseconds/s" "cgroups" "netdata.plugin_cgroups_cpu" "stacked" 132000 1 "   " "cgroups.plugin" "stats"
DIMENSION "user" "user" "incremental" 1 1000 "  "
DIMENSION "system" "system" "incremental" 1 1000 "  "
REPBEGIN "netdata.plugin_cgroups_cpu" 1636629513 1636629513 1636629514
REPDIM "user" 1636629513 956658990
REPDIM "system" 1636629513 16777216
REPEND 2 5057 5057
CHART "disk_space._dev" "" "Disk Space Usage for /dev udev" "GiB" "/dev" "disk.space" "stacked" 2023 1 "   " "diskspace.plugin" ""
DIMENSION "avail" "avail" "absolute" 4096 1073741824 "  "
DIMENSION "used" "used" "absolute" 4096 1073741824 "  "
DIMENSION "reserved_for_root" "reserved for root" "absolute" 4096 1073741824 "  "
REPBEGIN "disk_space._dev" 1636629513 1636629513 1636629514
REPDIM "avail" 1636629513 825965588
REPDIM "used" 1636629513 16777216
REPDIM "reserved_for_root" 1636629513 16777216
REPEND 3 1017644 1017644
CHART "system.cpu" "" "Total CPU utilization" "percentage" "cpu" "system.cpu" "stacked" 100 1 "   " "proc.plugin" "/proc/stat"
DIMENSION "guest_nice" "guest_nice" "percentage-of-incremental-row" 1 1 "  "
DIMENSION "guest" "guest" "percentage-of-incremental-row" 1 1 "  "
DIMENSION "steal" "steal" "percentage-of-incremental-row" 1 1 "  "
DIMENSION "softirq" "softirq" "percentage-of-incremental-row" 1 1 "  "
DIMENSION "irq" "irq" "percentage-of-incremental-row" 1 1 "  "
DIMENSION "user" "user" "percentage-of-incremental-row" 1 1 "  "
DIMENSION "system" "system" "percentage-of-incremental-row" 1 1 "  "
DIMENSION "nice" "nice" "percentage-of-incremental-row" 1 1 "  "
DIMENSION "iowait" "iowait" "percentage-of-incremental-row" 1 1 "  "
DIMENSION "idle" "idle" "percentage-of-incremental-row" 1 1 " hidden "
REPBEGIN "system.cpu" 1636629513 1636629513 1636629514
REPDIM "guest_nice" 1636629513 16777216
REPDIM "guest" 1636629513 16777216
REPDIM "steal" 1636629513 958813875
REPDIM "softirq" 1636629513 16777216
REPDIM "irq" 1636629513 16777216
REPDIM "user" 1636629513 830626298
REPDIM "system" 1636629513 824344891
REPDIM "nice" 1636629513 16777216
REPDIM "iowait" 1636629513 830123785
REPDIM "idle" 1636629513 695956308
REPEND 10 1070046304 1070046304
```

# Compression benchmark results with lzbench
#### Note: In the table below, the variable `Ratio` is calculated as `(Compressed size / Uncompressed size) * 100`. Please do not confuse it with the `compression ratio = (Uncompressed size/Compressed size)`.
```
./lzbench -t16,16 -eall -o1 samples/rep_dict.log
lzbench 1.8 (64-bit Linux)  Intel(R) Xeon(R) Gold 6240 CPU @ 2.60GHz
Assembled by P.Skibinski
```

| Compressor name         | Compression| Decompress.| Compr. size | Ratio | Filename |
| ---------------         | -----------| -----------| ----------- | ----- | -------- |
| memcpy                  |  5017 MB/s |  5465 MB/s |      324581 |100.00 | samples/rep_dict.log|
| blosclz 2.0.0 -1        |  5842 MB/s |  7132 MB/s |      324581 |100.00 | samples/rep_dict.log|
| blosclz 2.0.0 -3        |   597 MB/s |  2666 MB/s |      227820 | 70.19 | samples/rep_dict.log|
| blosclz 2.0.0 -6        |   549 MB/s |  1091 MB/s |      106590 | 32.84 | samples/rep_dict.log|
| blosclz 2.0.0 -9        |   548 MB/s |  1149 MB/s |      106590 | 32.84 | samples/rep_dict.log|
| brieflz 1.3.0 -1        |   368 MB/s |   524 MB/s |       54368 | 16.75 | samples/rep_dict.log|
| brieflz 1.3.0 -3        |   279 MB/s |   577 MB/s |       48677 | 15.00 | samples/rep_dict.log|
| brieflz 1.3.0 -6        |    98 MB/s |   818 MB/s |       32832 | 10.12 | samples/rep_dict.log|
| brieflz 1.3.0 -8        |  7.41 MB/s |   858 MB/s |       31226 |  9.62 | samples/rep_dict.log|
| brotli 1.0.9 -0         |   719 MB/s |   758 MB/s |       47118 | 14.52 | samples/rep_dict.log|
| brotli 1.0.9 -2         |   246 MB/s |   668 MB/s |       39678 | 12.22 | samples/rep_dict.log|
| brotli 1.0.9 -5         |    92 MB/s |   880 MB/s |       27398 |  8.44 | samples/rep_dict.log|
| brotli 1.0.9 -8         |    58 MB/s |   915 MB/s |       25905 |  7.98 | samples/rep_dict.log|
| brotli 1.0.9 -11        |  0.89 MB/s |   813 MB/s |       22066 |  6.80 | samples/rep_dict.log|
| bzip2 1.0.8 -1          |    12 MB/s |    71 MB/s |       28348 |  8.73 | samples/rep_dict.log|
| bzip2 1.0.8 -5          |    11 MB/s |    62 MB/s |       23641 |  7.28 | samples/rep_dict.log|
| bzip2 1.0.8 -9          |    11 MB/s |    63 MB/s |       23641 |  7.28 | samples/rep_dict.log|
| crush 1.0 -0            |    26 MB/s |   608 MB/s |       41081 | 12.66 | samples/rep_dict.log|
| crush 1.0 -1            |    20 MB/s |   766 MB/s |       33489 | 10.32 | samples/rep_dict.log|
| crush 1.0 -2            |  7.54 MB/s |   805 MB/s |       31851 |  9.81 | samples/rep_dict.log|
| csc 2016-10-13 -1       |    52 MB/s |   155 MB/s |       34895 | 10.75 | samples/rep_dict.log|
| csc 2016-10-13 -3       |    26 MB/s |   161 MB/s |       31666 |  9.76 | samples/rep_dict.log|
| csc 2016-10-13 -5       |  6.10 MB/s |   210 MB/s |       24528 |  7.56 | samples/rep_dict.log|
| density 0.14.2 -1       |  1649 MB/s |  2070 MB/s |      186647 | 57.50 | samples/rep_dict.log|
| density 0.14.2 -2       |   652 MB/s |  1167 MB/s |       99845 | 30.76 | samples/rep_dict.log|
| density 0.14.2 -3       |   377 MB/s |   411 MB/s |       75401 | 23.23 | samples/rep_dict.log|
| fastlz 0.5.0 -1         |   614 MB/s |  1191 MB/s |       68486 | 21.10 | samples/rep_dict.log|
| fastlz 0.5.0 -2         |   613 MB/s |  1273 MB/s |       63979 | 19.71 | samples/rep_dict.log|
| fastlzma2 1.0.1 -1      |    33 MB/s |   159 MB/s |       33497 | 10.32 | samples/rep_dict.log|
| fastlzma2 1.0.1 -3      |    18 MB/s |   185 MB/s |       28495 |  8.78 | samples/rep_dict.log|
| fastlzma2 1.0.1 -5      |  8.96 MB/s |   233 MB/s |       23562 |  7.26 | samples/rep_dict.log|
| fastlzma2 1.0.1 -8      |  6.87 MB/s |   225 MB/s |       22964 |  7.07 | samples/rep_dict.log|
| fastlzma2 1.0.1 -10     |  3.73 MB/s |   226 MB/s |       22813 |  7.03 | samples/rep_dict.log|
| gipfeli 2016-07-13      |   593 MB/s |   830 MB/s |       57702 | 17.78 | samples/rep_dict.log|
| libdeflate 1.6 -1       |   277 MB/s |  1391 MB/s |       50673 | 15.61 | samples/rep_dict.log|
| libdeflate 1.6 -3       |   264 MB/s |  1615 MB/s |       41775 | 12.87 | samples/rep_dict.log|
| libdeflate 1.6 -6       |   162 MB/s |  1690 MB/s |       33484 | 10.32 | samples/rep_dict.log|
| libdeflate 1.6 -9       |    16 MB/s |  1581 MB/s |       33233 | 10.24 | samples/rep_dict.log|
| libdeflate 1.6 -12      |  5.62 MB/s |  1542 MB/s |       30349 |  9.35 | samples/rep_dict.log|
| lizard 1.0 -10          |   826 MB/s |  3895 MB/s |       63579 | 19.59 | samples/rep_dict.log|
| lizard 1.0 -12          |   230 MB/s |  3911 MB/s |       58837 | 18.13 | samples/rep_dict.log|
| lizard 1.0 -15          |   126 MB/s |  4679 MB/s |       44405 | 13.68 | samples/rep_dict.log|
| lizard 1.0 -19          |  2.32 MB/s |  4742 MB/s |       40791 | 12.57 | samples/rep_dict.log|
| lizard 1.0 -20          |   692 MB/s |  2868 MB/s |       63943 | 19.70 | samples/rep_dict.log|
| lizard 1.0 -22          |   467 MB/s |  3295 MB/s |       52179 | 16.08 | samples/rep_dict.log|
| lizard 1.0 -25          |    16 MB/s |  4006 MB/s |       40560 | 12.50 | samples/rep_dict.log|
| lizard 1.0 -29          |  1.93 MB/s |  4471 MB/s |       36063 | 11.11 | samples/rep_dict.log|
| lizard 1.0 -30          |   619 MB/s |  1893 MB/s |       50438 | 15.54 | samples/rep_dict.log|
| lizard 1.0 -32          |   227 MB/s |  1949 MB/s |       47267 | 14.56 | samples/rep_dict.log|
| lizard 1.0 -35          |   137 MB/s |  2648 MB/s |       39917 | 12.30 | samples/rep_dict.log|
| lizard 1.0 -39          |  1.97 MB/s |  3822 MB/s |       37401 | 11.52 | samples/rep_dict.log|
| lizard 1.0 -40          |   522 MB/s |  1941 MB/s |       51714 | 15.93 | samples/rep_dict.log|
| lizard 1.0 -42          |   393 MB/s |  2198 MB/s |       42708 | 13.16 | samples/rep_dict.log|
| lizard 1.0 -45          |    16 MB/s |  2924 MB/s |       35278 | 10.87 | samples/rep_dict.log|
| lizard 1.0 -49          |  1.72 MB/s |  3141 MB/s |       31297 |  9.64 | samples/rep_dict.log|
| lz4 1.9.3               |   997 MB/s |  4571 MB/s |       63513 | 19.57 | samples/rep_dict.log|
| lz4fast 1.9.3 -3        |  1053 MB/s |  4620 MB/s |       65827 | 20.28 | samples/rep_dict.log|
| lz4fast 1.9.3 -17       |  1209 MB/s |  4471 MB/s |       87197 | 26.86 | samples/rep_dict.log|
| lz4hc 1.9.3 -1          |   230 MB/s |  4811 MB/s |       51982 | 16.02 | samples/rep_dict.log|
| lz4hc 1.9.3 -4          |   174 MB/s |  5528 MB/s |       43394 | 13.37 | samples/rep_dict.log|
| lz4hc 1.9.3 -9          |    52 MB/s |  6069 MB/s |       40874 | 12.59 | samples/rep_dict.log|
| lz4hc 1.9.3 -12         |    18 MB/s |  6345 MB/s |       40369 | 12.44 | samples/rep_dict.log|
| lzf 3.6 -0              |   596 MB/s |  1089 MB/s |       72845 | 22.44 | samples/rep_dict.log|
| lzf 3.6 -1              |   648 MB/s |  1112 MB/s |       70511 | 21.72 | samples/rep_dict.log|
| lzfse 2017-03-08        |    97 MB/s |  1513 MB/s |       34814 | 10.73 | samples/rep_dict.log|
| lzg 1.0.10 -1           |  4.94 MB/s |   997 MB/s |       65562 | 20.20 | samples/rep_dict.log|
| lzg 1.0.10 -4           |  4.85 MB/s |  1077 MB/s |       53292 | 16.42 | samples/rep_dict.log|
| lzg 1.0.10 -6           |  4.75 MB/s |  1124 MB/s |       49533 | 15.26 | samples/rep_dict.log|
| lzg 1.0.10 -8           |  4.42 MB/s |  1200 MB/s |       44770 | 13.79 | samples/rep_dict.log|
| lzham 1.0 -d26 -0       |    13 MB/s |   332 MB/s |       39839 | 12.27 | samples/rep_dict.log|
| lzham 1.0 -d26 -1       |  4.41 MB/s |   410 MB/s |       29292 |  9.02 | samples/rep_dict.log|
| lzjb 2010               |   483 MB/s |   461 MB/s |       93741 | 28.88 | samples/rep_dict.log|
| lzlib 1.12-rc2 -0       |    71 MB/s |   129 MB/s |       35397 | 10.91 | samples/rep_dict.log|
| lzlib 1.12-rc2 -3       |    23 MB/s |   133 MB/s |       34465 | 10.62 | samples/rep_dict.log|
| lzlib 1.12-rc2 -6       |  6.62 MB/s |   167 MB/s |       24490 |  7.55 | samples/rep_dict.log|
| lzlib 1.12-rc2 -9       |  2.95 MB/s |   175 MB/s |       22557 |  6.95 | samples/rep_dict.log|
| lzma 19.00 -0           |    67 MB/s |   180 MB/s |       32464 | 10.00 | samples/rep_dict.log|
| lzma 19.00 -2           |    70 MB/s |   217 MB/s |       28385 |  8.75 | samples/rep_dict.log|
| lzma 19.00 -4           |    71 MB/s |   218 MB/s |       28349 |  8.73 | samples/rep_dict.log|
| lzma 19.00 -5           |  8.27 MB/s |   233 MB/s |       25259 |  7.78 | samples/rep_dict.log|
| lzma 19.00 -9           |  5.33 MB/s |   249 MB/s |       23065 |  7.11 | samples/rep_dict.log|
| lzo1 2.10 -1            |   626 MB/s |   656 MB/s |       70356 | 21.68 | samples/rep_dict.log|
| lzo1 2.10 -99           |   199 MB/s |   695 MB/s |       63659 | 19.61 | samples/rep_dict.log|
| lzo1a 2.10 -1           |   627 MB/s |   861 MB/s |       70126 | 21.61 | samples/rep_dict.log|
| lzo1a 2.10 -99          |   199 MB/s |   934 MB/s |       63431 | 19.54 | samples/rep_dict.log|
| lzo1b 2.10 -1           |   570 MB/s |  1173 MB/s |       61036 | 18.80 | samples/rep_dict.log|
| lzo1b 2.10 -3           |   538 MB/s |  1178 MB/s |       60252 | 18.56 | samples/rep_dict.log|
| lzo1b 2.10 -6           |   463 MB/s |  1148 MB/s |       58576 | 18.05 | samples/rep_dict.log|
| lzo1b 2.10 -9           |   281 MB/s |  1191 MB/s |       58591 | 18.05 | samples/rep_dict.log|
| lzo1b 2.10 -99          |   252 MB/s |  1416 MB/s |       46013 | 14.18 | samples/rep_dict.log|
| lzo1b 2.10 -999         |    29 MB/s |  1624 MB/s |       40191 | 12.38 | samples/rep_dict.log|
| lzo1c 2.10 -1           |   625 MB/s |  1206 MB/s |       63780 | 19.65 | samples/rep_dict.log|
| lzo1c 2.10 -3           |   582 MB/s |  1196 MB/s |       63256 | 19.49 | samples/rep_dict.log|
| lzo1c 2.10 -6           |   472 MB/s |  1144 MB/s |       61188 | 18.85 | samples/rep_dict.log|
| lzo1c 2.10 -9           |   280 MB/s |  1201 MB/s |       60769 | 18.72 | samples/rep_dict.log|
| lzo1c 2.10 -99          |   235 MB/s |  1350 MB/s |       50137 | 15.45 | samples/rep_dict.log|
| lzo1c 2.10 -999         |    46 MB/s |  1530 MB/s |       43605 | 13.43 | samples/rep_dict.log|
| lzo1f 2.10 -1           |   584 MB/s |  1082 MB/s |       65196 | 20.09 | samples/rep_dict.log|
| lzo1f 2.10 -999         |    41 MB/s |  1388 MB/s |       43554 | 13.42 | samples/rep_dict.log|
| lzo1x 2.10 -1           |   993 MB/s |  1118 MB/s |       64112 | 19.75 | samples/rep_dict.log|
| lzo1x 2.10 -11          |  1006 MB/s |  1106 MB/s |       64881 | 19.99 | samples/rep_dict.log|
| lzo1x 2.10 -12          |  1006 MB/s |  1110 MB/s |       64508 | 19.87 | samples/rep_dict.log|
| lzo1x 2.10 -15          |  1000 MB/s |  1110 MB/s |       64450 | 19.86 | samples/rep_dict.log|
| lzo1x 2.10 -999         |    14 MB/s |  1471 MB/s |       39239 | 12.09 | samples/rep_dict.log|
| lzo1y 2.10 -1           |   987 MB/s |  1021 MB/s |       62269 | 19.18 | samples/rep_dict.log|
| lzo1y 2.10 -999         |    15 MB/s |  1414 MB/s |       38527 | 11.87 | samples/rep_dict.log|
| lzo1z 2.10 -999         |    14 MB/s |  1456 MB/s |       39240 | 12.09 | samples/rep_dict.log|
| lzo2a 2.10 -999         |    56 MB/s |   835 MB/s |       45407 | 13.99 | samples/rep_dict.log|
| lzrw 15-Jul-1991 -1     |   560 MB/s |   849 MB/s |       88922 | 27.40 | samples/rep_dict.log|
| lzrw 15-Jul-1991 -3     |   530 MB/s |   898 MB/s |       77522 | 23.88 | samples/rep_dict.log|
| lzrw 15-Jul-1991 -4     |   552 MB/s |   882 MB/s |       75392 | 23.23 | samples/rep_dict.log|
| lzrw 15-Jul-1991 -5     |   222 MB/s |  1056 MB/s |       63592 | 19.59 | samples/rep_dict.log|
| lzsse2 2019-04-18 -1    |    24 MB/s |  5422 MB/s |       51837 | 15.97 | samples/rep_dict.log|
| lzsse2 2019-04-18 -6    |    13 MB/s |  6096 MB/s |       41529 | 12.79 | samples/rep_dict.log|
| lzsse2 2019-04-18 -12   |    13 MB/s |  6095 MB/s |       41529 | 12.79 | samples/rep_dict.log|
| lzsse2 2019-04-18 -16   |    13 MB/s |  6095 MB/s |       41529 | 12.79 | samples/rep_dict.log|
| lzsse4 2019-04-18 -1    |    24 MB/s |  6319 MB/s |       48331 | 14.89 | samples/rep_dict.log|
| lzsse4 2019-04-18 -6    |    13 MB/s |  6766 MB/s |       42326 | 13.04 | samples/rep_dict.log|
| lzsse4 2019-04-18 -12   |    13 MB/s |  6766 MB/s |       42326 | 13.04 | samples/rep_dict.log|
| lzsse4 2019-04-18 -16   |    13 MB/s |  6764 MB/s |       42326 | 13.04 | samples/rep_dict.log|
| lzsse8 2019-04-18 -1    |    22 MB/s |  5998 MB/s |       49478 | 15.24 | samples/rep_dict.log|
| lzsse8 2019-04-18 -6    |    13 MB/s |  6469 MB/s |       43264 | 13.33 | samples/rep_dict.log|
| lzsse8 2019-04-18 -12   |    13 MB/s |  6467 MB/s |       43264 | 13.33 | samples/rep_dict.log|
| lzsse8 2019-04-18 -16   |    13 MB/s |  6467 MB/s |       43264 | 13.33 | samples/rep_dict.log|
| lzvn 2017-03-08         |    98 MB/s |  1839 MB/s |       48266 | 14.87 | samples/rep_dict.log|
| pithy 2011-12-24 -0     |   949 MB/s |  1949 MB/s |       59904 | 18.46 | samples/rep_dict.log|
| pithy 2011-12-24 -3     |   946 MB/s |  1983 MB/s |       58266 | 17.95 | samples/rep_dict.log|
| pithy 2011-12-24 -6     |   914 MB/s |  1995 MB/s |       57246 | 17.64 | samples/rep_dict.log|
| pithy 2011-12-24 -9     |   839 MB/s |  2021 MB/s |       56955 | 17.55 | samples/rep_dict.log|
| quicklz 1.5.0 -1        |   723 MB/s |  1245 MB/s |       58372 | 17.98 | samples/rep_dict.log|
| quicklz 1.5.0 -2        |   463 MB/s |  1455 MB/s |       47776 | 14.72 | samples/rep_dict.log|
| quicklz 1.5.0 -3        |   112 MB/s |  1777 MB/s |       44793 | 13.80 | samples/rep_dict.log|
| slz_gzip 1.2.0 -1       |   557 MB/s |   541 MB/s |       61654 | 18.99 | samples/rep_dict.log|
| slz_gzip 1.2.0 -2       |   565 MB/s |   557 MB/s |       58709 | 18.09 | samples/rep_dict.log|
| slz_gzip 1.2.0 -3       |   546 MB/s |   560 MB/s |       58171 | 17.92 | samples/rep_dict.log|
| snappy 2020-07-11       |   800 MB/s |  3175 MB/s |       64269 | 19.80 | samples/rep_dict.log|
| ucl_nrv2b 1.03 -1       |    81 MB/s |   478 MB/s |       56442 | 17.39 | samples/rep_dict.log|
| ucl_nrv2b 1.03 -6       |    48 MB/s |   654 MB/s |       37450 | 11.54 | samples/rep_dict.log|
| ucl_nrv2b 1.03 -9       |  8.92 MB/s |   750 MB/s |       33685 | 10.38 | samples/rep_dict.log|
| ucl_nrv2d 1.03 -1       |    82 MB/s |   482 MB/s |       55920 | 17.23 | samples/rep_dict.log|
| ucl_nrv2d 1.03 -6       |    50 MB/s |   655 MB/s |       37429 | 11.53 | samples/rep_dict.log|
| ucl_nrv2d 1.03 -9       |  8.89 MB/s |   743 MB/s |       33345 | 10.27 | samples/rep_dict.log|
| ucl_nrv2e 1.03 -1       |    83 MB/s |   485 MB/s |       55317 | 17.04 | samples/rep_dict.log|
| ucl_nrv2e 1.03 -6       |    49 MB/s |   661 MB/s |       37156 | 11.45 | samples/rep_dict.log|
| ucl_nrv2e 1.03 -9       |  8.67 MB/s |   744 MB/s |       33124 | 10.21 | samples/rep_dict.log|
| xpack 2016-06-02 -1     |   261 MB/s |  1204 MB/s |       46651 | 14.37 | samples/rep_dict.log|
| xpack 2016-06-02 -6     |   153 MB/s |  1720 MB/s |       28606 |  8.81 | samples/rep_dict.log|
| xpack 2016-06-02 -9     |    62 MB/s |  1798 MB/s |       27670 |  8.52 | samples/rep_dict.log|
| xz 5.2.5 -0             |    61 MB/s |   172 MB/s |       32218 |  9.93 | samples/rep_dict.log|
| xz 5.2.5 -3             |    44 MB/s |   212 MB/s |       26824 |  8.26 | samples/rep_dict.log|
| xz 5.2.5 -6             |  6.03 MB/s |   226 MB/s |       23084 |  7.11 | samples/rep_dict.log|
| xz 5.2.5 -9             |  5.34 MB/s |   213 MB/s |       23084 |  7.11 | samples/rep_dict.log|
| yalz77 2015-09-19 -1    |   297 MB/s |   961 MB/s |       59505 | 18.33 | samples/rep_dict.log|
| yalz77 2015-09-19 -4    |   198 MB/s |  1124 MB/s |       49224 | 15.17 | samples/rep_dict.log|
| yalz77 2015-09-19 -8    |   151 MB/s |  1164 MB/s |       47448 | 14.62 | samples/rep_dict.log|
| yalz77 2015-09-19 -12   |   125 MB/s |  1177 MB/s |       46976 | 14.47 | samples/rep_dict.log|
| yappy 2014-03-22 -1     |   256 MB/s |  3931 MB/s |       64592 | 19.90 | samples/rep_dict.log|
| yappy 2014-03-22 -10    |   141 MB/s |  4265 MB/s |       56689 | 17.47 | samples/rep_dict.log|
| yappy 2014-03-22 -100   |    92 MB/s |  4554 MB/s |       54408 | 16.76 | samples/rep_dict.log|
| zlib 1.2.11 -1          |   167 MB/s |   544 MB/s |       48535 | 14.95 | samples/rep_dict.log|
| zlib 1.2.11 -6          |    76 MB/s |   684 MB/s |       33054 | 10.18 | samples/rep_dict.log|
| zlib 1.2.11 -9          |    24 MB/s |   695 MB/s |       32117 |  9.89 | samples/rep_dict.log|
| zling 2018-10-12 -0     |   155 MB/s |   352 MB/s |       30961 |  9.54 | samples/rep_dict.log|
| zling 2018-10-12 -1     |   154 MB/s |   366 MB/s |       29629 |  9.13 | samples/rep_dict.log|
| zling 2018-10-12 -2     |   149 MB/s |   370 MB/s |       29069 |  8.96 | samples/rep_dict.log|
| zling 2018-10-12 -3     |   140 MB/s |   374 MB/s |       28523 |  8.79 | samples/rep_dict.log|
| zling 2018-10-12 -4     |   135 MB/s |   377 MB/s |       28205 |  8.69 | samples/rep_dict.log|
| zstd 1.5.0 -1           |   664 MB/s |  1930 MB/s |       36972 | 11.39 | samples/rep_dict.log|
| zstd 1.5.0 -2           |   679 MB/s |  1906 MB/s |       37060 | 11.42 | samples/rep_dict.log|
| zstd 1.5.0 -5           |   212 MB/s |  2080 MB/s |       31614 |  9.74 | samples/rep_dict.log|
| zstd 1.5.0 -8           |   128 MB/s |  2400 MB/s |       28199 |  8.69 | samples/rep_dict.log|
| zstd 1.5.0 -11          |   110 MB/s |  2475 MB/s |       27451 |  8.46 | samples/rep_dict.log|
| zstd 1.5.0 -15          |    24 MB/s |  2647 MB/s |       26107 |  8.04 | samples/rep_dict.log|
| zstd 1.5.0 -18          |  6.54 MB/s |  2598 MB/s |       24264 |  7.48 | samples/rep_dict.log|
| zstd 1.5.0 -22          |  1.56 MB/s |  2559 MB/s |       23853 |  7.35 | samples/rep_dict.log|
| shrinker 0.1            |   634 MB/s |  1845 MB/s |       58732 | 18.09 | samples/rep_dict.log|
| wflz 2015-09-16         |   532 MB/s |  1387 MB/s |       79343 | 24.44 | samples/rep_dict.log|
| lzmat 1.01              |    76 MB/s |   618 MB/s |       38120 | 11.74 | samples/rep_dict.log|

# Run the lzbench(mark)
1. `git clone git@github.com:inikep/lzbench.git`
2. `cd lzbench/`
3. `make` - When I cloned it build was failing because `tornado` couldn't build the object files properly. If this is the case, run `make clean` and then `make DONT_BUILD_TORNADO=1`
4. Run an example with the cmd `./lzbench -ezstd samples/rep_dict.log`

# Resources
## Benchmarks
1. [Benchmark-quixdb](https://quixdb.github.io/squash-benchmark/#results): Inlcudes Algorithms, Benchmarks with transmission speeds
2. [TurboBench](https://sites.google.com/site/powturbo/compression-benchmark)
3. [lzbench from inikep](https://github.com/inikep/lzbench)
## Compression Algorithms and repos
4. [zstd - Facebook](https://github.com/facebook/zstd#the-case-for-small-data-compression) - Yann Collet, Nick Terrell, Przemysław Skibiński
5. [Dataset - Silesia corpus](http://sun.aei.polsl.pl/~sdeor/index.php?page=silesia)
6. [lz4 - Yann Collet](https://github.com/lz4/lz4)
7. [brotli - google](https://github.com/google/brotli)