noinst_LTLIBRARIES = libtdnfsolv.la

libtdnfsolv_la_CPPFLAGS = -I$(top_srcdir)/include

libtdnfsolv_la_SOURCES = \
    tdnfpool.c \
    tdnfrepo.c \
    tdnfquery.c \
    tdnfpackage.c
    
libtdnfsolv_la_LDFLAGS =  \
    -static \
    @RPM_LIBS@ \
    @LIBSOLV_LIBS@
