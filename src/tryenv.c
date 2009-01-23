#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <pthread.h>

#include "debug.h"
#include "exception.h"
#include "list.h"

typedef struct {
	list_t list;
	jmp_buf env;
} tryenv_t;

static pthread_key_t tryenv_head_key;
static pthread_once_t tryenv_head_once = PTHREAD_ONCE_INIT;

static
void tryenv_init_key(void)
{
	pthread_key_create(&tryenv_head_key, NULL);
}

static
void tryenv_init(void)
{
	pthread_once(&tryenv_head_once, tryenv_init_key);
	list_t *head = pthread_getspecific(tryenv_head_key);

	if (!head) {
		tryenv_t *new;
		LIST_NODE_ALLOC(new);
		INIT_LIST_HEAD(&(new->list));
		pthread_setspecific(tryenv_head_key, &(new->list));
	}
}

static
bool tryenv_empty(void)
{
	tryenv_init();
	list_t *head = pthread_getspecific(tryenv_head_key);
	return list_empty(head);
}

void tryenv_push(jmp_buf *env, int ret)
{
	if (ret != 0)
		return;

	tryenv_init();

	tryenv_t *new;
	LIST_NODE_ALLOC(new);
	memcpy(&new->env, env, sizeof(jmp_buf));

	list_t *head = pthread_getspecific(tryenv_head_key);
	list_add(&new->list, head);
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

	list_t *head = pthread_getspecific(tryenv_head_key);
	list_t *pos = head->next;
	tryenv_t *env = list_entry(pos, tryenv_t, list);

	list_del(pos);
	free(env);
}

void tryenv_jmp(void)
{
	if (tryenv_empty())
		tryenv_default_handler();

	list_t *head = pthread_getspecific(tryenv_head_key);
	list_t *pos = head->next;
	tryenv_t *env = list_entry(pos, tryenv_t, list);

	jmp_buf buf;
	memcpy(&buf, &env->env, sizeof(jmp_buf));

	list_del(pos);
	free(env);

	longjmp(buf, 1);
}
