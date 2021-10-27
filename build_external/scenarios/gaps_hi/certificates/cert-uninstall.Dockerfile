ARG DISTRO=arch
ARG VERSION=current

FROM ${DISTRO}_${VERSION}_dev:latest

# Uninstall certificates
RUN rm -rf /usr/local/share/ca-certificates/netdata
RUN dpkg-reconfigure ca-certificates
RUN sed -i '/netdata_parent.crt/d' /etc/ca-certificates.conf
RUN /usr/sbin/update-ca-certificates
