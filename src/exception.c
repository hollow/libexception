#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

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

static pthread_key_t exception_head_key;
static pthread_once_t exception_head_once = PTHREAD_ONCE_INIT;

static
void exception_key_init(void)
{
	pthread_key_create(&exception_head_key, NULL);
}

static
void exception_init(void)
{
	pthread_once(&exception_head_once, exception_key_init);
	list_t *head = pthread_getspecific(exception_head_key);

	if (!head) {
		exception_t *new;
		LIST_NODE_ALLOC(new);
		INIT_LIST_HEAD(&(new->list));
		pthread_setspecific(exception_head_key, &(new->list));
	}
}

void exception_clear(void)
{
	list_t *head = pthread_getspecific(exception_head_key);
	list_t *pos, *tmp;

	if (!head)
		return;

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
	list_t *head = pthread_getspecific(exception_head_key);
	return list_empty(head);
}

int exception_errno(void)
{
	if (exception_empty())
		return 0;

	list_t *head = pthread_getspecific(exception_head_key);
	list_t *pos = head->prev;
	exception_t *e = list_entry(pos, exception_t, list);

	return e->errnum;
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

	list_t *head = pthread_getspecific(exception_head_key);
	list_add(&new->list, head);
	return 0;
}

static
char *exception_print(exception_t *e)
{
	char *buf;

	if (e->msg == NULL) {
		asprintf(&buf, "at %s:%d in %s():\n",
				e->file,
				e->line,
				e->func);
	} else {
		asprintf(&buf, "at %s:%d in %s(): %s (%d)\n",
				e->file,
				e->line,
				e->func,
				e->msg,
				e->errnum);
	}

	return buf;
}

char *exception_print_all(void)
{
	if (exception_empty())
		return NULL;

	list_t *head = pthread_getspecific(exception_head_key);
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
