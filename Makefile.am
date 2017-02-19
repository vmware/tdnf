ACLOCAL_AMFLAGS = -I m4

CLEANFILES =
EXTRA_DIST =
conf_DATA = tdnf.conf
pkginclude_HEADERS = include/*.h
pkgconfigdir = $(libdir)/pkgconfig

SUBDIRS = \
    common \
    client \
    tests \
    tools

pkgconfig_DATA = tdnf.pc tdnf-cli-libs.pc

tdnf.pc: $(top_srcdir)/tdnf.pc.in
	./config.status --file=${subdir}/tdnf.pc:${subdir}/tdnf.pc.in

tdnf-cli-libs.pc: $(top_srcdir)/tdnf-cli-libs.pc.in
	./config.status --file=${subdir}/tdnf-cli-libs.pc:${subdir}/tdnf-cli-libs.pc.in

CLEANFILES += tdnf.pc
CLEANFILES += tdnf-cli-libs.pc

EXTRA_DIST += tdnf.pc.in
EXTRA_DIST += tdnf-cli-libs.pc.in
