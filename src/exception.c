// Copyright (c) 2008 Benedikt Böhm <hollow@gentoo.org>
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. The name of the author may not be used to endorse or promote products
//    derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <unistd.h>
#include <stdarg.h>
#include <libgen.h>

#include <apr_pools.h>
#include <apr_strings.h>

#include "exception.h"

static exception_t *exception_stack = NULL;
static apr_pool_t *exception_pool = NULL;

static
int exception_abortfn(int retcode)
{
	write(2, "OOM in exception pool. Aborting.\n", 33);
	abort();
	return -1;
}

static
void exception_init(void)
{
	trace;

	if (exception_pool) {
		apr_pool_destroy(exception_pool);
		exception_pool = NULL;
	}

	apr_pool_create_ex(&exception_pool, NULL, &exception_abortfn, NULL);

	exception_stack = apr_pcalloc(exception_pool, sizeof(exception_t));
	INIT_LIST_HEAD(&(exception_stack->list));
}

void exception_clear(void)
{
	trace;

	if (exception_pool)
		apr_pool_destroy(exception_pool);

	exception_pool  = NULL;
	exception_stack = NULL;
}

bool exception_empty(void)
{
	trace;

	if (!exception_stack || !exception_stack->list.next)
		return true;

	return list_empty(&(exception_stack->list));
}

void exception_push(const char *file, int line, const char *func,
		int errnum, const char *fmt, ...)
{
	trace;

	if (!exception_stack)
		exception_init();

	exception_t *new = apr_pcalloc(exception_pool, sizeof(exception_t));

	new->file   = file;
	new->func   = func;
	new->line   = line;
	new->errnum = errnum;

	if (fmt == NULL)
		new->msg = NULL;

	else {
		va_list ap;
		va_start(ap, fmt);
		new->msg = apr_pvsprintf(exception_pool, fmt, ap);
		va_end(ap);
	}

	debug("%s:%d in %s(): errno = %d: %s", new->file, new->line, new->func, new->errnum, new->msg);

	list_add(&(new->list), &(exception_stack->list));
}

exception_t *exception_pop(void)
{
	trace;

	if (exception_empty())
		return NULL;

	exception_t *top;
	list_t *pos;

	pos = exception_stack->list.next;
	top = list_entry(pos, exception_t, list);

	list_del(pos);
	return top;
}

static
char *exception_describe(exception_t *err, const char *prefix)
{
	trace;

	apr_pool_t *p = exception_pool;

	char *errmsg = NULL, *errstr = NULL;
	char *file   = basename(apr_pstrdup(p, err->file));

	if (err->msg)
		errmsg = apr_psprintf(p, "\n%10s%s", "", err->msg);

	if (err->errnum > 0)
		errstr = apr_psprintf(p, "\n%10serrno = %d: %s",
				"", err->errnum, strerror(err->errnum));

	return apr_psprintf(p, "%s %s (%s:%d):%s%s",
			prefix, err->func, file, err->line,
			errmsg ? errmsg : "",
			errstr ? errstr : "");
}

char *exception_dump(void)
{
	trace;

	apr_pool_t *p = exception_pool;
	exception_t *next = NULL, *cur = exception_pop();

	if (!cur)
		return NULL;

	char *dump = apr_pstrcat(p,
			exception_describe(cur, "in  "), "\n", NULL);

	if (!(cur = exception_pop()))
		return dump;

	do {
		if (!(next = exception_pop()))
			break;

		dump = apr_pstrcat(p, dump,
				exception_describe(cur, "from"), "\n", NULL);

		cur = next;
	} while (true);

	dump = apr_pstrcat(p, dump,
			exception_describe(cur, "by  "), "\n", NULL);

	return dump;
}
