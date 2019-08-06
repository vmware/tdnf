FROM photon:latest

MAINTAINER csiddharth@vmware.com

RUN tdnf update  -q -y
RUN tdnf remove  -q -y toybox
RUN tdnf install -q -y build-essential autoconf automake make \
                       libtool sed curl-devel rpm-build popt-devel \
                       libsolv-devel createrepo_c glib libxml2 findutils

ENTRYPOINT ["./docker-entrypoint.sh"]
