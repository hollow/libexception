INCLUDES = -I$(srcdir)

noinst_HEADERS = debug.h list.h
include_HEADERS = exception.h

lib_LTLIBRARIES = libexception.la

libexception_la_SOURCES = exception.c tryenv.c
libexception_la_LIBADD = @PTHREAD_LIBS@
libexception_la_LDFLAGS = -version-info 0:0:0

# vim: ts=4 expandtab
