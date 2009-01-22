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
