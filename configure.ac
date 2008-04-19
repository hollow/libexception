dnl generic configuration
AC_PREREQ(2.59)
AC_INIT(libexception, 0.1, hollow@gentoo.org)

AC_GNU_SOURCE

AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE([1.9 foreign dist-bzip2])

dnl check for progs
AC_PROG_CC
AC_PROG_CPP
AC_PROG_LIBTOOL
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

dnl m4 macros from APR
sinclude(build/apr_common.m4)
sinclude(build/find_apr.m4)
sinclude(build/find_apu.m4)

APR_PARSE_ARGUMENTS
APR_CONFIG_NICE(config.nice)

dnl Check that mkdir -p works
APR_MKDIR_P_CHECK($top_srcdir/build/mkdir.sh)
AC_SUBST(mkdir_p)


nl='
'

echo $ac_n "${nl}Configuring Apache Portable Runtime library...${nl}"

APR_FIND_APR("$srcdir/apr", "./apr", 1, [0 1])

if test "$apr_found" = "no"; then
    AC_MSG_ERROR([APR not found.  Please read the documentation.])
fi

if test "$apr_found" = "reconfig"; then
    APR_SUBDIR_CONFIG(apr,
                      [--prefix=$prefix --exec-prefix=$exec_prefix --libdir=$libdir --includedir=$includedir --bindir=$bindir --datadir=$datadir --with-installbuilddir=$installbuilddir],
                      [--enable-layout=*|\'--enable-layout=*])
fi

APR_ADDTO(CFLAGS, `$apr_config --cflags`)
APR_ADDTO(CPPFLAGS, `$apr_config --cppflags`)
APR_ADDTO(LDFLAGS, `$apr_config --ldflags`)

APR_BINDIR=`$apr_config --bindir`
APR_INCLUDEDIR=`$apr_config --includedir`
APR_VERSION=`$apr_config --version`
APR_CONFIG="$apr_config"

AC_SUBST(APR_BINDIR)
AC_SUBST(APR_INCLUDEDIR)
AC_SUBST(APR_VERSION)
AC_SUBST(APR_CONFIG)


echo $ac_n "${nl}Configuring libexception...${nl}"

dnl check for debugging code
AC_ARG_ENABLE([debug],
              APR_HELP_STRING([--enable-debug], [enable debugging (default: disabled)]),
              [enable_debug=$enableval], [enable_debug=no])

if test "$enable_debug" = "yes"; then
    APR_ADDTO(CPPFLAGS, "-DCONFIG_DEBUG=1")
fi

dnl checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE

dnl compiler settings
APR_ADDTO(CFLAGS, [-std=gnu99 -pedantic -Wall])
APR_ADDTO(CFLAGS, [-Wpointer-arith -Winline])
APR_ADDTO(CFLAGS, [-Wredundant-decls -Wcast-align -Wno-unused-parameter])

APR_ADDTO(APR_LIBS, `$apr_config --link-libtool --libs`)
AC_SUBST(APR_LIBS)

echo $ac_n ""

# Final info page
AC_CONFIG_COMMANDS_PRE([SUMMARY="$PACKAGE_STRING configured successfully:

                       CC: $CC ($($CC --version | head -n1))
                 CPPFLAGS: '$CPPFLAGS'
                   CFLAGS: '$CFLAGS'
                    build: $build
                     host: $host
                   target: $target

                   prefix: $prefix
"])

dnl Makefile outputs
AC_CONFIG_FILES(Makefile src/Makefile test/Makefile)
AC_OUTPUT

AC_MSG_NOTICE([$SUMMARY])
