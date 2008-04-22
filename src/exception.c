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
#include <stdio.h>
#include <string.h>
#include <libgen.h>

#include "debug.h"
#include "exception.h"
#include "list.h"

static exception_t *exception_stack = NULL;

static
void exception_init(void)
{
	trace;
	LIST_NODE_ALLOC(exception_stack);
	INIT_LIST_HEAD(&(exception_stack->list));
}

void exception_clear(void)
{
	trace;

	exception_t *e;
	while ((e = exception_pop())) {
		if (e->msg)
			free(e->msg);
		free(e);
	}

	free(exception_stack);
	exception_stack = NULL;
}

bool exception_empty(void)
{
	trace;
	return !exception_stack || list_empty(&(exception_stack->list));
}

int exception_errno(void)
{
	trace;

	if (exception_empty())
		return 0;

	list_t *pos;
	exception_t *top;

	pos = exception_stack->list.prev;
	top = list_entry(pos, exception_t, list);

	return top->errnum;
}

int exception_push(const char *file, int line, const char *func,
		int errnum, const char *fmt, ...)
{
	trace;

	if (!exception_stack)
		exception_init();

	exception_t *new;
	LIST_NODE_ALLOC(new);

	new->file   = file;
	new->func   = func;
	new->line   = line;
	new->errnum = errnum;

	if (fmt == NULL)
		new->msg = NULL;

	else {
		va_list ap;
		va_start(ap, fmt);
		vasprintf(&new->msg, fmt, ap);
		va_end(ap);
	}

	debug("%s:%d in %s(): errno = %d: %s", new->file, new->line,
			new->func, new->errnum, new->msg);

	list_add(&(new->list), &(exception_stack->list));
	return 0;
}

exception_t *exception_pop(void)
{
	trace;

	if (exception_empty())
		return NULL;

	list_t *pos;
	exception_t *top;

	pos = exception_stack->list.next;
	top = list_entry(pos, exception_t, list);

	list_del(pos);
	return top;
}

char *exception_print(exception_t *err)
{
	trace;
	char *buf;

	if (err->msg == NULL) {
		asprintf(&buf, "at %s:%d in %s():",
				err->file,
				err->line,
				err->func);
	} else {
		asprintf(&buf, "at %s:%d in %s(): %s (%d)",
				err->file,
				err->line,
				err->func,
				err->msg,
				err->errnum);
	}

	return buf;
}

char *exception_print_all(void)
{
	trace;

	if (exception_empty())
		return NULL;

	trace;

	list_t *head = &exception_stack->list;
	exception_t *e;

	int len = 0;
	char *buf = NULL;

	list_for_each_entry(e, head, list) {
		char *ebuf = exception_print(e);
		int elen = strlen(ebuf);

		buf = realloc(buf, len + elen + 2);
		strncpy(buf + len, ebuf, elen);
		len += elen;
		buf[len++] = '\n';

		free(ebuf);
	}

	buf[len] = '\0';
	return buf;
}
