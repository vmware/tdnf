pkgpyexec_LTLIBRARIES = _tdnf.la
pkgpyexec_DATA = tdnf/__init__.py

CLEANFILES =

_tdnf_la_CPPFLAGS = \
    -I$(top_srcdir)/include \
    @PYTHON_CFLAGS@

_tdnf_la_SOURCES = \
    tdnfbase.c \
    tdnfpycommands.c \
    tdnfmodule.c \
    tdnfpyrepodata.c \
    utils.c

_tdnf_la_LDFLAGS= -module -avoid-version

_tdnf_la_LIBADD = \
    $(top_builddir)/common/libcommon.la \
    $(top_builddir)/client/libtdnf.la \
    @PYTHON_LIBS@

CLEANFILES += tdnf/__pycache__
CLEANFILES += build
