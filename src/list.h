// Copyright (c) 2007-2008 Benedikt Böhm <hollow@gentoo.org>
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

/*!
 * @defgroup list Simple doubly linked lists
 *
 * The simplest kind of linked list is a singly-linked list, which has one link
 * per node. This link points to the next node in the list, or to a null value
 * or empty list if it is the final node; e.g. 12 -> 99 -> 37 -> NULL.
 *
 * A more sophisticated kind of linked list is a doubly-linked list. Each node
 * has two links: one points to the previous node, or points to a null value or
 * empty list if it is the first node; and one points to the next, or points to
 * a null value or empty list if it is the final node; e.g.
 * NULL <- 26 <-> 56 <-> 46 -> NULL.
 *
 * The list family of functions and macros provide routines to create a list,
 * add, move or remove elements and iterate over the list.
 *
 * @{
 */

#ifndef _LIST_H
#define _LIST_H

#include <stddef.h>
#include <stdlib.h>

#include "exception.h"

/*! @brief get container of list head */
#define container_of(ptr, type, member) \
	((type *)((char *)(ptr) - offsetof(type, member)))

/*
 * Simple doubly linked list implementation.
 *
 * Some of the internal functions ("__xxx") are useful when
 * manipulating whole lists rather than single entries, as
 * sometimes we already know the next/prev entries and we can
 * generate better code by using them directly rather than
 * using the generic single-entry routines.
 */

/*! @brief list head */
typedef struct list_head {
	struct list_head *next, *prev;
} list_t;

#define LIST_NODE_ALLOC(NAME) \
	NAME = calloc(1, sizeof(*NAME))

static inline void INIT_LIST_HEAD(list_t *list)
{
	list->next = list;
	list->prev = list;
}

/*
 * Insert a new entry between two known consecutive entries.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline
void __list_add(list_t *new,
                list_t *prev,
                list_t *next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

/*!
 * @brief add a new entry
 *
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 *
 * @param new  new entry to be added
 * @param head list head to add it after
 */
static inline
void list_add(list_t *new, list_t *head)
{
	__list_add(new, head, head->next);
}

/*!
 * @brief add a new entry
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 *
 * @param new  new entry to be added
 * @param head list head to add it before
 */
static inline
void list_add_tail(list_t *new, list_t *head)
{
	__list_add(new, head->prev, head);
}

/*
 * Delete a list entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline
void __list_del(list_t * prev, list_t * next)
{
	next->prev = prev;
	prev->next = next;
}

/*!
 * @brief deletes entry from list
 *
 * @param entry the element to delete from the list
 *
 * @note list_empty on entry does not return true after this, the entry is
 *       in an undefined state
 */
static inline
void list_del(list_t *entry)
{
	__list_del(entry->prev, entry->next);
	entry->next = (void *) 0;
	entry->prev = (void *) 0;
}

/*!
 * @brief deletes entry from list and reinitialize it
 *
 * @param entry the element to delete from the list
 */
static inline
void list_del_init(list_t *entry)
{
	__list_del(entry->prev, entry->next);
	INIT_LIST_HEAD(entry);
}

/*!
 * @brief delete from one list and add as another's head
 *
 * @param list the entry to move
 * @param head the head that will precede our entry
 */
static inline
void list_move(list_t *list, list_t *head)
{
	__list_del(list->prev, list->next);
	list_add(list, head);
}

/*!
 * @brief delete from one list and add as another's tail
 *
 * @param list the entry to move
 * @param head the head that will follow our entry
 */
static inline
void list_move_tail(list_t *list, list_t *head)
{
	__list_del(list->prev, list->next);
	list_add_tail(list, head);
}

/*!
 * @brief tests whether a list is empty
 *
 * @param head the list to test
 */
static inline
int list_empty(const list_t *head)
{
	return head->next == head;
}

static inline
void __list_splice(list_t *list, list_t *head)
{
	list_t *first = list->next;
	list_t *last = list->prev;
	list_t *at = head->next;

	first->prev = head;
	head->next = first;

	last->next = at;
	at->prev = last;
}

/*!
 * @brief join two lists
 *
 * @param list the new list to add
 * @param head the place to add it in the first list
 */
static inline
void list_splice(list_t *list, list_t *head)
{
	if (!list_empty(list))
		__list_splice(list, head);
}

/*!
 * @brief join two lists and reinitialise the emptied list
 *
 * @param list the new list to add
 * @param head the place to add it in the first list
 */
static inline
void list_splice_init(list_t *list, list_t *head)
{
	if (!list_empty(list)) {
		__list_splice(list, head);
		INIT_LIST_HEAD(list);
	}
}

/*!
 * @brief get the struct for this entry
 *
 * @param ptr    the &list_t pointer
 * @param type   the type of the struct this is embedded in
 * @param member the name of the list_struct within the struct
 */
#define list_entry(ptr, type, member) \
	container_of(ptr, type, member)

/*!
 * @brief iterate over a list
 *
 * @param pos  the &list_t to use as a loop counter
 * @param head the head for your list
 */
#define list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

/*!
 * @brief iterate over a list backwards
 *
 * @param pos  the &list_t to use as a loop counter
 * @param head the head for your list
 */
#define list_for_each_prev(pos, head) \
	for (pos = (head)->prev; pos != (head); pos = pos->prev)

/*!
 * @brief iterate over a list safe against removal of list entry
 *
 * @param pos   the &list_t to use as a loop counter
 * @param n     another &list_t to use as temporary storage
 * @param head  the head for your list
 */
#define list_for_each_safe(pos, n, head) \
	for (pos = (head)->next, n = pos->next; pos != (head); \
		pos = n, n = pos->next)

/*!
 * @brief iterate over list of given type
 *
 * @param pos    the type * to use as a loop counter
 * @param head:  the head for your list
 * @param member the name of the list_struct within the struct
 */
#define list_for_each_entry(pos, head, member) \
	for (pos = list_entry((head)->next, typeof(*pos), member); \
	     &pos->member != (head); \
	     pos = list_entry(pos->member.next, typeof(*pos), member))

/*!
 * @brief iterate backwards over list of given type.
 *
 * @param pos    the type * to use as a loop counter
 * @param head   the head for your list
 * @param member the name of the list_struct within the struct
 */
#define list_for_each_entry_reverse(pos, head, member) \
	for (pos = list_entry((head)->prev, typeof(*pos), member); \
	     &pos->member != (head); \
	     pos = list_entry(pos->member.prev, typeof(*pos), member))

/*!
 * @brief iterate over list of given type safe against removal of list entry
 *
 * @param pos    the type * to use as a loop counter
 * @param n      another type * to use as temporary storage
 * @param head   the head for your list
 * @param member the name of the list_struct within the struct
 */
#define list_for_each_entry_safe(pos, n, head, member) \
	for (pos = list_entry((head)->next, typeof(*pos), member), \
	     n = list_entry(pos->member.next, typeof(*pos), member); \
	     &pos->member != (head); \
	     pos = n, n = list_entry(n->member.next, typeof(*n), member))

/*!
 * @brief iterate backwards over list of given type safe against removal of list entry
 *
 * @param pos    the type * to use as a loop counter
 * @param n      another type * to use as temporary storage
 * @param head   the head for your list
 * @param member the name of the list_struct within the struct
 */
#define list_for_each_entry_safe_reverse(pos, n, head, member) \
	for (pos = list_entry((head)->prev, typeof(*pos), member), \
	     n = list_entry(pos->member.prev, typeof(*pos), member); \
	     &pos->member != (head); \
	     pos = n, n = list_entry(n->member.prev, typeof(*n), member))

#endif

/*! @} list */
