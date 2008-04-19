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
#include <setjmp.h>

#include <apr_pools.h>
#include <apr_strings.h>

#include "exception.h"

typedef struct tryenv_s {
	list_t list;
	jmp_buf env;
} tryenv_t;

jmp_buf __tryenv_buffer;

static tryenv_t *tryenv_stack = NULL;
static apr_pool_t *tryenv_pool = NULL;

static
int tryenv_abortfn(int retcode)
{
	write(2, "OOM in tryenv pool. Aborting.\n", 30);
	abort();
	return -1;
}

static
void tryenv_init(void)
{
	trace;

	if (tryenv_pool) {
		apr_pool_destroy(tryenv_pool);
		tryenv_pool = NULL;
	}

	apr_pool_create_ex(&tryenv_pool, NULL, &tryenv_abortfn, NULL);

	tryenv_stack = apr_pcalloc(tryenv_pool, sizeof(tryenv_t));
	INIT_LIST_HEAD(&(tryenv_stack->list));
}

static
bool tryenv_empty(void)
{
	trace;

	if (!tryenv_stack || !tryenv_stack->list.next)
		return true;

	trace;

	return list_empty(&(tryenv_stack->list));
}

void tryenv_push(void)
{
	trace;

	if (!tryenv_stack)
		tryenv_init();

	tryenv_t *new = apr_pcalloc(tryenv_pool, sizeof(tryenv_t));
	memcpy(new->env, __tryenv_buffer, sizeof(jmp_buf));
	list_add(&(new->list), &(tryenv_stack->list));
}

static
void tryenv_default_handler(void)
{
	trace;

	char *ebuf;

	if (exception_empty())
		ebuf = "internal error: tryenv_default_handler called with empty exception stack";
	else if (!(ebuf = exception_dump()))
		ebuf = "internal error: no dump for exception stack";

	write(STDOUT_FILENO, ebuf, strlen(ebuf));
	abort();
}

void tryenv_pop(void)
{
	trace;

	if (tryenv_empty())
		return;

	trace;

	tryenv_t *top;
	list_t *pos;

	pos = tryenv_stack->list.next;
	top = list_entry(pos, tryenv_t, list);

	list_del(pos);
}

void tryenv_jmp(void)
{
	trace;

	if (tryenv_empty())
		tryenv_default_handler();

	trace;

	tryenv_t *top;
	list_t *pos;

	pos = tryenv_stack->list.next;
	top = list_entry(pos, tryenv_t, list);

	siglongjmp(top->env, 1);
}
