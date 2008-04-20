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
#include <string.h>
#include <setjmp.h>

#include "exception.h"
#include "list.h"

typedef struct {
	list_t list;
	jmp_buf env;
} tryenv_t;

static tryenv_t *tryenv_stack = NULL;

static
void tryenv_init(void)
{
	trace;
	LIST_NODE_ALLOC(tryenv_stack);
	INIT_LIST_HEAD(&(tryenv_stack->list));
}

static
bool tryenv_empty(void)
{
	trace;
	return !tryenv_stack || list_empty(&(tryenv_stack->list));
}

void tryenv_push(jmp_buf *env, int ret)
{
	trace;

	if (ret != 0)
		return;

	trace;

	if (!tryenv_stack)
		tryenv_init();

	tryenv_t *new;
	LIST_NODE_ALLOC(new);
	memcpy(&new->env, env, sizeof(jmp_buf));
	list_add(&(new->list), &(tryenv_stack->list));
}

static
void tryenv_default_handler(void)
{
	trace;

	char *ebuf;

	if (exception_empty()) {
		ebuf = "internal error: tryenv_default_handler called with empty exception stack";
		write(STDOUT_FILENO, ebuf, strlen(ebuf));
	}

	else {
		exception_t *e;
		while ((e = exception_pop())) {
			ebuf = exception_print(e);
			write(STDOUT_FILENO, ebuf, strlen(ebuf));
			free(ebuf);
		}
	}

	abort();
}

void tryenv_pop(void)
{
	trace;

	if (tryenv_empty())
		return;

	trace;

	list_t *pos;
	tryenv_t *top;

	pos = tryenv_stack->list.next;
	top = list_entry(pos, tryenv_t, list);

	list_del(pos);
	free(top);

	if (tryenv_stack && tryenv_empty()) {
		free(tryenv_stack);
		tryenv_stack = NULL;
	}
}

void tryenv_jmp(void)
{
	trace;

	if (tryenv_empty())
		tryenv_default_handler();

	trace;

	list_t *pos;
	tryenv_t *top;

	pos = tryenv_stack->list.next;
	top = list_entry(pos, tryenv_t, list);

	jmp_buf env;
	memcpy(&env, &top->env, sizeof(jmp_buf));

	list_del(pos);
	free(top);

	longjmp(env, 1);
}
