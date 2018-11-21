AC_INIT(tdnf, 1.2)
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

AC_SUBST(AM_CPPFLAGS)
AC_SUBST(AM_CFLAGS)

#hawkey lib
echo "Looking for hawkey headers"
AC_CHECK_HEADERS(hawkey/sack.h)

echo "Looking for hawkey libs"
AC_CHECK_LIB(hawkey, hy_sack_count)

#licurl
PKG_CHECK_MODULES([LIBCURL], [libcurl], [have_libcurl=yes], [have_libcurl=no])
AM_CONDITIONAL([LIBCURL],  [test "$have_libcurl" = "yes"])

#rpm
echo "Looking for librpm headers"
AC_CHECK_HEADERS(rpm/rpmlib.h)

echo "Looking for librpm libs"
AC_CHECK_LIB(rpm, rpmtsCreate)

#makefiles
AC_CONFIG_FILES([Makefile
                 common/Makefile
                 client/Makefile
                 tests/Makefile
                 tools/Makefile
                 tools/cli/Makefile
                 tools/cli/lib/Makefile
                ])

#tdnf conf dir
confdir=$sysconfdir/tdnf
AC_SUBST(confdir)

#yum repo dir
yumrepodir=$sysconfdir/yum.repos.d
AC_SUBST(yumrepodir)

#
AC_OUTPUT
