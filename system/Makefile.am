# Copyright (C) 2015 Alon Bar-Lev <alon.barlev@gmail.com>
# SPDX-License-Identifier: GPL-3.0-or-later

MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
CLEANFILES = \
    netdata-openrc \
    netdata.logrotate \
    netdata.service \
    netdata.service.v235 \
    netdata-init-d \
    netdata-lsb \
    netdata-freebsd \
    netdata.plist \
    netdata.crontab \
    netdata-updater.service \
    $(NULL)

include $(top_srcdir)/build/subst.inc
SUFFIXES = .in

dist_config_SCRIPTS = \
    edit-config \
    $(NULL)

dist_config_DATA = \
    .install-type \
    $(NULL)

# Explicitly install directories to avoid permission issues due to umask
install-exec-local:
	$(INSTALL) -d $(DESTDIR)$(configdir)
	$(INSTALL) -d $(DESTDIR)$(libsysdir)

libexecnetdatadir=$(libexecdir)/netdata
nodist_libexecnetdata_SCRIPTS = \
    install-service.sh \
    $(NULL)

nodist_libsys_DATA = \
    netdata-openrc \
    netdata.logrotate \
    netdata.service \
    netdata.service.v235 \
    netdata-init-d \
    netdata-lsb \
    netdata-freebsd \
    netdata.plist \
    netdata.crontab \
    netdata-updater.service \
    $(NULL)

dist_libsys_DATA = \
    netdata-updater.timer \
    $(NULL)

dist_noinst_DATA = \
    install-service.sh.in \
    netdata-openrc.in \
    netdata.logrotate.in \
    netdata.service.in \
    netdata.service.v235.in \
    netdata-init-d.in \
    netdata-lsb.in \
    netdata-freebsd.in \
    netdata.plist.in \
    netdata.conf \
    netdata.crontab.in \
    netdata-updater.service.in \
    $(NULL)
