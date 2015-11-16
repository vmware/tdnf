bin_PROGRAMS = tdnf

tdnf_CPPFLAGS = -I$(top_srcdir)/include
tdnf_SOURCES = \
    help.c \
    installcmd.c \
    main.c \
    options.c \
    output.c \
    parseargs.c \
    parsecleanargs.c \
    parselistargs.c \
    parserepolistargs.c \
    parseupdateinfo.c \
    updateinfocmd.c
tdnf_LDADD =  \
    $(top_builddir)/client/libtdnfclient.la
