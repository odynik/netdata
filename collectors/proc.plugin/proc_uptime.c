// SPDX-License-Identifier: GPL-3.0-or-later

#include "plugin_proc.h"

int do_proc_uptime(int update_every, usec_t dt) {
    (void)dt;

    static char *uptime_filename = NULL;
    if(!uptime_filename) {
        char filename[FILENAME_MAX + 1];
        snprintfz(filename, FILENAME_MAX, "%s%s", netdata_configured_host_prefix, "/proc/uptime");

        uptime_filename = config_get("plugin:proc:/proc/uptime", "filename to monitor", filename);
    }

    static RRDSET *st = NULL;
    static RRDDIM *rd = NULL;

    if(unlikely(!st)) {

        st = rrdset_create_localhost(
                "system"
                , "uptime"
                , NULL
                , "uptime"
                , NULL
                , "System Uptime"
                , "seconds"
                , PLUGIN_PROC_NAME
                , "/proc/uptime"
                , NETDATA_CHART_PRIO_SYSTEM_UPTIME
                , update_every
                , RRDSET_TYPE_LINE
        );

        // rrdset_flag_set(st, RRDSET_FLAG_STORE_FIRST);

        rd = rrddim_add(st, "uptime", NULL, 1, 1, RRD_ALGORITHM_ABSOLUTE);
    }
    else
        rrdset_next(st);

    static int serial_number = 1;
    rrddim_set_by_pointer(st, rd, serial_number++);

    rrdset_done(st);

    return 0;
}
