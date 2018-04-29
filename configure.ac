AC_INIT(tdnf, 2.0.0)
AC_MSG_NOTICE([tdnf configuration])

AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE([-Wall foreign])

AC_CONFIG_TESTDIR(tests)

AM_PROG_AR

AC_PROG_CC

#LT_INIT
AC_PROG_LIBTOOL

CPPFLAGS="$CPPFLAGS -D_REENTRANT -D_GNU_SOURCE -fPIC"

AM_CPPFLAGS="$AM_CPPFLAGS -I${top_srcdir}/include"
AM_CFLAGS="$AM_CFLAGS -std=gnu99 -Wall -fno-strict-aliasing"

CPPFLAGS="$CPPFLAGS -D_REENTRANT -D_GNU_SOURCE -fPIC"
AC_SUBST(AM_CPPFLAGS)
AC_SUBST(AM_CFLAGS)

#libsolv
PKG_CHECK_MODULES([LIBSOLV], [libsolv], [have_libsolv=yes], [have_libsolv=no])
PKG_CHECK_MODULES([LIBSOLVEXT], [libsolvext], [have_libsolvext=yes], [have_libsolvext=no])
AM_CONDITIONAL([LIBSOLV],  [test "$have_libsolv" = "yes"])
AM_CONDITIONAL([LIBSOLVEXT],  [test "$have_libsolvext" = "yes"])

#libcurl
PKG_CHECK_MODULES([LIBCURL], [libcurl], [have_libcurl=yes], [have_libcurl=no])
AM_CONDITIONAL([LIBCURL],  [test "$have_libcurl" = "yes"])

#rpm
PKG_CHECK_MODULES([RPM], [rpm], [have_rpm=yes], [have_rpm=no])
AM_CONDITIONAL([RPM], [test "$have_rpm" = "yes"])

# test to prove rpm.org rpm
echo "Looking for rpm.org librpm headers"
AC_CHECK_HEADERS(rpm/header.h)


#makefiles
AC_CONFIG_FILES([Makefile
                 solv/Makefile
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
