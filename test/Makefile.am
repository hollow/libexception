INCLUDES = -I$(top_srcdir)/src/ \
           -I@APR_INCLUDEDIR@ \
           -I@APU_INCLUDEDIR@

check_PROGRAMS = test1 \
                 test2 \
                 test3 \
                 test4

TESTS = $(check_PROGRAMS)

test1_SOURCES = test1.c
test1_LDADD = $(top_builddir)/src/libexception.la

test2_SOURCES = test2.c
test2_LDADD = $(top_builddir)/src/libexception.la

test3_SOURCES = test3.c
test3_LDADD = $(top_builddir)/src/libexception.la

test4_SOURCES = test4.c
test4_LDADD = $(top_builddir)/src/libexception.la

# vim: ts=4 expandtab
