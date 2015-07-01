AC_INIT(tdnf, 1.0.1)
AC_MSG_NOTICE([tdnf configuration])

AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

AM_PROG_AR

AC_PROG_CC
#LT_INIT
AC_PROG_LIBTOOL

AM_CPPFLAGS="$AM_CPPFLAGS -I${top_srcdir}/include"
AM_CFLAGS="$AM_CFLAGS -Wall -Werror -fno-strict-aliasing"

AC_SUBST(AM_CPPFLAGS)
AC_SUBST(AM_CFLAGS)

#hawkey lib
echo "Looking for hawkey headers"
AC_CHECK_HEADERS(hawkey/sack.h)

echo "Looking for hawkey libs"
AC_CHECK_LIB(hawkey, hy_sack_count)

CPPFLAGS="-I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include"
#glib
echo "Looking for glib headers"
AC_CHECK_HEADERS(glib.h)
AC_CHECK_HEADERS(glibconfig.h)

#librepo
echo "Looking for librepo headers"
AC_CHECK_HEADERS(librepo/librepo.h)

echo "Looking for librepo libs"
AC_CHECK_LIB(repo, lr_handle_init)

#rpm
echo "Looking for librpm headers"
AC_CHECK_HEADERS(rpm/rpmlib.h)

echo "Looking for librpm libs"
AC_CHECK_LIB(rpm, rpmtsCreate)

#makefiles
AC_CONFIG_FILES([Makefile
                 client/Makefile
                 tools/Makefile
                 tools/cli/Makefile
                ])

#tdnf conf dir
confdir=$sysconfdir/tdnf
AC_SUBST(confdir)

#yum repo dir
yumrepodir=$sysconfdir/yum.repos.d
AC_SUBST(yumrepodir)

#
AC_OUTPUT
