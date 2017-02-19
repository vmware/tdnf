lib_LTLIBRARIES = libtdnf.la
libtdnf_la_CPPFLAGS = -I$(top_srcdir)/include

AM_CFLAGS += $(LIBCURL_CFLAGS)

libtdnf_la_SOURCES = \
    api.c \
    clean.c \
    client.c \
    config.c \
    goal.c \
    gpgcheck.c \
    init.c \
    packageutils.c \
    repo.c \
    remoterepo.c \
    repolist.c \
    repoutils.c \
    resolve.c \
    rpmtrans.c \
    updateinfo.c \
    utils.c \
    search.c \
    validate.c

libtdnf_la_LIBADD =  \
    $(top_builddir)/common/libcommon.la \
    @LIBCURL_LIBS@ \
    -lhawkey \
    -lrpm
