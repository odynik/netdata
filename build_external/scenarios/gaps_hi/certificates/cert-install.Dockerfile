ARG DISTRO=arch
ARG VERSION=current

FROM ${DISTRO}_${VERSION}_dev:latest

# Install certificates
COPY certificates/localhost.crt /etc/netdata/ssl/localhost.crt
COPY certificates/localhost.key /etc/netdata/ssl/localhost.key
RUN chmod 755 /etc/netdata/ssl
RUN chmod 644 /etc/netdata/ssl/*.*
RUN chown -R netdata.netdata /etc/netdata/ssl
RUN mkdir -p /usr/local/share/ca-certificates/netdata
COPY certificates/localhost.crt /usr/local/share/ca-certificates/netdata/netdata_parent.crt
RUN chown -R netdata.netdata /usr/local/share/ca-certificates/netdata
RUN /usr/sbin/update-ca-certificates
RUN echo "netdata/netdata_parent.crt" >> /etc/ca-certificates.conf
# RUN chown netdata.netdata /etc/ssl/certs/netdata_parent.pem