lib_LTLIBRARIES = libtdnfclient.la
libtdnfclient_la_CPPFLAGS = -I$(top_srcdir)/include

libtdnfclient_la_SOURCES = \
    api.c \
    clean.c \
    client.c \
    config.c \
    goal.c \
    gpgcheck.c \
    init.c \
    makecache.c \
    memory.c \
    packageutils.c \
    repo.c \
    remoterepo.c \
    repolist.c \
    repoutils.c \
    resolve.c \
    rpmtrans.c \
    strings.c \
    updateinfo.c \
    utils.c \
    search.c \
    validate.c

libtdnfclient_la_LIBADD =  \
    -lhawkey \
    -lrepo \
    -lrpm
