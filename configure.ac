dnl generic configuration
AC_PREREQ(2.59)
AC_INIT(libexception, 0.1)

AC_GNU_SOURCE

AC_CANONICAL_BUILD
AC_CANONICAL_HOST

AM_INIT_AUTOMAKE([1.9 foreign dist-bzip2])

dnl check for progs
AC_PROG_CC
AC_PROG_CPP
AC_PROG_LIBTOOL
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

dnl check for debugging code
AC_ARG_ENABLE([debug],
              AC_HELP_STRING([--enable-debug], [enable debugging (default: disabled)]),
              [enable_debug=$enableval], [enable_debug=no])

if test "$enable_debug" = "yes"; then
    CPPFLAGS="$CPPFLAGS -DCONFIG_DEBUG=1"
fi

dnl checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE

dnl compiler settings
CFLAGS="${CFLAGS} -std=gnu99 -pedantic -Wall"
CFLAGS="${CFLAGS} -Wpointer-arith -Wcast-qual -Winline"
CFLAGS="${CFLAGS} -Wredundant-decls -Wcast-align -Wno-unused-parameter"

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
AC_CONFIG_FILES(Makefile
                Doxyfile
                src/Makefile
                test/Makefile)
AC_OUTPUT

AC_MSG_NOTICE([$SUMMARY])

# vim: ts=4 expandtab
