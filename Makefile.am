ACLOCAL_AMFLAGS = -I m4

CLEANFILES =
EXTRA_DIST =
conf_DATA = tdnf.conf
pkginclude_HEADERS = include/tdnfclient.h include/tdnferror.h include/tdnftypes.h
pkgconfigdir = $(libdir)/pkgconfig

SUBDIRS = \
    solv \
    common \
    client \
    tests \
    tools

pkgconfig_DATA = tdnf.pc
tdnf.pc: $(top_srcdir)/tdnf.pc.in
	./config.status --file=${subdir}/tdnf.pc:${subdir}/tdnf.pc.in
CLEANFILES += tdnf.pc
EXTRA_DIST += tdnf.pc.in
