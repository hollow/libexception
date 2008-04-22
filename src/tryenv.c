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
#include <string.h>
#include <setjmp.h>

#include "debug.h"
#include "exception.h"
#include "list.h"

typedef struct {
	list_t list;
	jmp_buf env;
} tryenv_t;

static tryenv_t tryenv_stack;
static list_t *tryenv_head = NULL;

static inline
void tryenv_init(void)
{
	if (!tryenv_head) {
		tryenv_head = &tryenv_stack.list;
		INIT_LIST_HEAD(tryenv_head);
	}
}

static inline
bool tryenv_empty(void)
{
	tryenv_init();
	return list_empty(tryenv_head);
}

void tryenv_push(jmp_buf *env, int ret)
{
	if (ret != 0)
		return;

	tryenv_init();

	tryenv_t *new;
	LIST_NODE_ALLOC(new);
	memcpy(&new->env, env, sizeof(jmp_buf));

	list_add(&new->list, tryenv_head);
}

static
void tryenv_default_handler(void)
{
	char *ebuf = "FATAL: uncaught exception\n";
	write(STDERR_FILENO, ebuf, strlen(ebuf));

	if (exception_empty())
		ebuf = "internal error: tryenv_default_handler called with empty exception stack";
	else
		ebuf = exception_print_all();

	write(STDERR_FILENO, ebuf, strlen(ebuf));
	abort();
}

void tryenv_pop(void)
{
	if (tryenv_empty())
		return;

	list_t *pos = tryenv_head->next;
	tryenv_t *env = list_entry(pos, tryenv_t, list);

	list_del(pos);
	free(env);
}

void tryenv_jmp(void)
{
	if (tryenv_empty())
		tryenv_default_handler();

	list_t *pos = tryenv_head->next;
	tryenv_t *env = list_entry(pos, tryenv_t, list);

	jmp_buf buf;
	memcpy(&buf, &env->env, sizeof(jmp_buf));

	list_del(pos);
	free(env);

	longjmp(buf, 1);
}
