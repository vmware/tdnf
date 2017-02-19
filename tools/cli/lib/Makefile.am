lib_LTLIBRARIES = libtdnfcli.la
libtdnfcli_la_CPPFLAGS = -I$(top_srcdir)/include

libtdnfcli_la_SOURCES = \
    api.c \
    help.c \
    installcmd.c \
    options.c \
    output.c \
    parseargs.c \
    parsecleanargs.c \
    parselistargs.c \
    parserepolistargs.c \
    parseupdateinfo.c \
    setopt.c \
    updateinfocmd.c

libtdnfcli_la_LIBADD =  \
    $(top_builddir)/common/libcommon.la
