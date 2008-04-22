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
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "debug.h"
#include "exception.h"
#include "list.h"

typedef struct {
	list_t list;
	const char *file;
	const char *func;
	int line;
	int errnum;
	char *msg;
} exception_t;

static exception_t exception_stack;
static list_t *exception_head = NULL;

static inline
void exception_init(void)
{
	if (!exception_head) {
		exception_head = &exception_stack.list;
		INIT_LIST_HEAD(exception_head);
	}
}

void exception_clear(void)
{
	list_t *head = exception_head;
	list_t *pos, *tmp;

	list_for_each_safe(pos, tmp, head) {
		list_del(pos);
		exception_t *e = list_entry(pos, exception_t, list);

		if (e->msg)
			free(e->msg);
		free(e);
	}
}

bool exception_empty(void)
{
	exception_init();
	return list_empty(exception_head);
}

int exception_errno(void)
{
	if (exception_empty())
		return 0;

	list_t *pos = exception_head->prev;
	exception_t *top = list_entry(pos, exception_t, list);

	return top->errnum;
}

int exception_push(const char *file, int line, const char *func,
		int errnum, const char *fmt, ...)
{
	exception_init();

	exception_t *new;
	LIST_NODE_ALLOC(new);

	new->file   = file;
	new->func   = func;
	new->line   = line;
	new->errnum = errnum;

	if (fmt == NULL) {
		new->msg = NULL;
	} else {
		va_list ap;
		va_start(ap, fmt);
		vasprintf(&new->msg, fmt, ap);
		va_end(ap);
	}

	debug("%s:%d in %s(): errno = %d: %s", new->file, new->line,
			new->func, new->errnum, new->msg);

	list_add(&new->list, exception_head);
	return 0;
}

static
char *exception_print(exception_t *err)
{
	char *buf;

	if (err->msg == NULL) {
		asprintf(&buf, "at %s:%d in %s():\n",
				err->file,
				err->line,
				err->func);
	} else {
		asprintf(&buf, "at %s:%d in %s(): %s (%d)\n",
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
	if (exception_empty())
		return NULL;

	list_t *head = exception_head;
	exception_t *e;

	char *buf = NULL;
	int len = 0;

	list_for_each_entry(e, head, list) {
		char *ebuf = exception_print(e);
		int elen = strlen(ebuf);

		buf = realloc(buf, len + elen + 1);
		strncpy(buf + len, ebuf, elen);
		len += elen;

		free(ebuf);
	}

	buf[len] = '\0';
	return buf;
}
