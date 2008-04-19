INCLUDES = -I$(srcdir) \
           -I@APR_INCLUDEDIR@ \
           -I@APU_INCLUDEDIR@

noinst_HEADERS = list.h
include_HEADERS = exception.h

lib_LTLIBRARIES = libexception.la

libexception_la_SOURCES = exception.c tryenv.c
libexception_la_LIBADD = @APR_LIBS@
libexception_la_LDFLAGS = -version-info 0:0:0

# vim: ts=4 expandtab
