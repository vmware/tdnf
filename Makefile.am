ACLOCAL_AMFLAGS = -I m4

APP_NAME=tdnf
CLEANFILES =
EXTRA_DIST =
noinst_DATA =
conf_DATA = tdnf.conf
pkginclude_HEADERS = include/*.h
pkgconfigdir = $(libdir)/pkgconfig

SUBDIRS = \
    solv \
    common \
    client \
    tests \
    tools

if ENABLE_PYTHON
    SUBDIRS += python
endif

noinst_DATA += tdnf.spec
tdnf.spec: $(top_srcdir)/tdnf.spec.in
	./config.status --file=${subdir}/tdnf.spec:${subdir}/tdnf.spec.in

pkgconfig_DATA = tdnf.pc tdnf-cli-libs.pc

tdnf.pc: $(top_srcdir)/tdnf.pc.in
	./config.status --file=${subdir}/tdnf.pc:${subdir}/tdnf.pc.in

tdnf-cli-libs.pc: $(top_srcdir)/tdnf-cli-libs.pc.in
	./config.status --file=${subdir}/tdnf-cli-libs.pc:${subdir}/tdnf-cli-libs.pc.in

CLEANFILES += tdnf.pc
CLEANFILES += tdnf-cli-libs.pc

EXTRA_DIST += tdnf.pc.in
EXTRA_DIST += tdnf-cli-libs.pc.in

tar-src:
	git archive --format=tar.gz --prefix=$(APP_NAME)-$(VERSION)/ -o $(APP_NAME)-$(VERSION).tar.gz HEAD

rpm: tar-src tdnf.spec
	mkdir -p .rpm/SOURCES
	mkdir -p .rpm/BUILD
	mkdir -p .rpm/RPMS/x86_64
	-rm -rf .rpm/RPMS/$(APP_NAME)-*
	-rm -rf .rpm/SOURCES/$(APP_NAME)-*
	mv $(APP_NAME)-$(VERSION).tar.gz .rpm/SOURCES
	rpmbuild --quiet --nodeps --define="_topdir $(abs_top_builddir)/.rpm" \
	-bb $(APP_NAME).spec
	mkdir -p rpms
	mv .rpm/RPMS/x86_64/*$(APP_NAME)-*$(VERSION)-*.x86_64.rpm rpms

clean-local:
	-rm -rf .rpm
	-rm -rf rpms
