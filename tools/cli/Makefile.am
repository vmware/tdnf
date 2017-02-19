bin_PROGRAMS = tdnf

tdnf_CPPFLAGS = -I$(top_srcdir)/include
tdnf_SOURCES = \
    main.c

SUBDIRS = \
    lib

tdnf_LDADD =  \
    $(top_builddir)/common/libcommon.la \
    $(top_builddir)/client/libtdnf.la \
    $(top_builddir)/tools/cli/lib/libtdnfcli.la
