AC_INIT(tdnf, 1.1.0)
AC_MSG_NOTICE([tdnf configuration])

AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

AC_CONFIG_TESTDIR(tests)

AM_PROG_AR

AC_PROG_CC
#LT_INIT
AC_PROG_LIBTOOL

CPPFLAGS="$CPPFLAGS -D_REENTRANT -D_GNU_SOURCE -fPIC"

AM_CPPFLAGS="$AM_CPPFLAGS -I${top_srcdir}/include"
AM_CFLAGS="$AM_CFLAGS -Wall -Werror -fno-strict-aliasing"

CPPFLAGS="$CPPFLAGS -D_REENTRANT -D_GNU_SOURCE -fPIC"
AC_SUBST(AM_CPPFLAGS)
AC_SUBST(AM_CFLAGS)

#libsolv
echo "Looking for libsolv headers"
AC_CHECK_HEADERS(solv/pool.h)

echo "Looking for libsolv libs"
AC_CHECK_LIB(solv, pool_create)


#glib
PKG_CHECK_MODULES([GLIB], [glib-2.0], [have_libglib=yes], [have_libglib=no])
AM_CONDITIONAL([GLIB],  [test "$have_libglib" = "yes"])

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
                 solv/Makefile
                 common/Makefile
                 client/Makefile
                 tests/Makefile
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
