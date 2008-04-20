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

#ifndef _EXCEPTION_H
#define _EXCEPTION_H

#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include <setjmp.h>
#include <string.h>

/* from list.h */
typedef struct list_head {
	struct list_head *next, *prev;
} list_t;

/*! @defgroup exception exception stack
 *
 * The exception API provides a primitive stack interface to record an
 * exception and its trace back to the location where it will be handled.
 *
 * @{
 */

/*! @brief the exception struct
 *
 * an object of type <tt>exception_t</tt> stores one exception and a
 * doubly-linked list to create the exception stack.
 */
typedef struct {
	const char *file; /*!< source file of this exception */
	const char *func; /*!< function of this exception */
	int line;         /*!< line in the source file of this exception */
	int errnum;       /*!< the errno value when this exception was thrown */
	char *msg;        /*!< the error message for this exception */
	list_t list;      /*!< exception stack list */
} exception_t;

/*! @brief clear the exception stack
 *
 * <tt>exception_clear</tt> can be used in <tt>except</tt> blocks to ignore the
 * current exception and clean up the exception stack. failure to clean these
 * stacks while ignoring the thrown exception will result in <b>undefined
 * behaviour.</b>.
 *
 * @note this function should not be used directly.
 * <tt>on</tt>/<tt>finally</tt> provide better semantics.
 */
void exception_clear(void);

/*! @brief check if exceptions exist
 *
 * <tt>exception_empty</tt> checks whether the exception stack is empty.
 *
 * @note this function should not be used directly,
 * <tt>try</tt>/<tt>except</tt> macros provide better semantics.
 *
 * @return <tt>true</tt> if the exception stack is empty, <tt>false</tt>
 *         otherwise.
 */
bool exception_empty(void);

/*! @brief get exception errno
 *
 * <tt>exception_errno</tt> returns the original errno which caused this
 * exception.
 *
 * @return errno value
 */
int exception_errno(void);

/*! @brief create new exception
 *
 * <tt>exception_push</tt> creates a new object of type <tt>exception_t</tt>
 * and pushes it onto the exception stack.
 *
 * @note this function should not be used directly, <tt>throw</tt> provides
 * better semantics.
 *
 * @param file   source file of this exception
 * @param line   source line of this exception
 * @param func   function where exception was thrown
 * @param errnum <tt>errno</tt> value when this exception was thrown
 * @param fmt    <tt>printf</tt> compatible error message
 *
 * @returns zero
 */
int exception_push(const char *file, int line, const char *func,
		int errnum, const char *fmt, ...);

/*! @brief get last exception
 *
 * <tt>exception_pop</tt> deletes and returns the topmost exception object on
 * the stack.
 *
 * @return a pointer to the topmost exception object
 */
exception_t *exception_pop(void);

/*! @brief print exception
 *
 * <tt>exception_print</tt> returns an exception description in standard
 * format.
 *
 * @returns pointer to description string
 */
char *exception_print(exception_t *err);

/*! @brief print exception trace
 *
 * <tt>exception_print_all</tt> returns an exception trace in standard
 * format.
 *
 * @returns pointer to exception trace string
 */
char *exception_print_all(void);

/*! @brief dump exception trace to file
 *
 * <tt>exception_dump</tt> writes the output of <tt>exception_print_all</tt> to the given file descriptor.
 *
 * @params fd file descriptor
 */
#define exception_dump(fd) do { \
	char *buf = exception_print_all(); \
	write(fd, buf, strlen(buf)); \
	free(buf); \
} while (0)

/*! @} exception */

/*! @defgroup tryenv jump environment
 *
 * The tryenv API provides a primitive stack interface to record jump
 * environments needed by non-local goto (a.k.a <tt>longjmp</tt>) used by
 * <tt>throw</tt>.
 *
 * @{
 */

/*! @brief create new jump environment
 *
 * <tt>tryenv_push</tt> creates a new <tt>tryenv_t</tt> object for the current
 * jump buffer and pushes it onto the environment stack.
 *
 * @note this function should not be used directly, <tt>try</tt> provides
 * better semantics.
 *
 * @param env jump environment returned by <tt>setjmp</tt>
 * @param ret return code from <tt>setjmp</tt>
 */
void tryenv_push(jmp_buf *env, int ret);

/*! @brief remove last jump environment
 *
 * <tt>tryenv_pop</tt> deletes the topmost environment on the environment
 * stack.
 *
 * @note this function should not be used directly, <tt>except</tt> provides
 * better semantics.
 */
void tryenv_pop(void);

/*! @brief jump to last environment
 *
 * <tt>tryenv_jmp</tt> jumps to and deletes the topmost environment the stack.
 *
 * @note this function should not be used directly,
 * <tt>throw</tt>/<tt>rethrow</tt> provide better semantics.
 */
void tryenv_jmp(void);

/*! @} tryenv */

/*! @defgroup semantics try/except semantics
 *
 * The following macros provide try/except semantics ontop of the exception and
 * tryenv APIs.
 *
 * @{
 */

#define __exception_block(start, end) \
	for (int __exception_block_pass = 1, start; \
	     __exception_block_pass; \
	     end, __exception_block_pass = 0)

#define __exception_end(end) \
	for (int __exception_block_pass = 1; \
	     __exception_block_pass; \
	     end, __exception_block_pass = 0)

#define __setjmp_push() do { \
	jmp_buf __tryenv_buffer; \
	int __tryenv_ret = setjmp(__tryenv_buffer); \
	tryenv_push(&__tryenv_buffer, __tryenv_ret); \
} while (0)

/*! @brief throw new exception
 *
 * <tt>throw</tt> creates a new exception object with <tt>exception_push</tt>
 * and jumps to the topmost environment on the stack.
 */
#define throw(...) do { \
	exception_push(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); \
	tryenv_jmp(); \
} while (0)

static inline
void __exception_rethrow(int handled)
{
	if (!handled)
		tryenv_jmp();
}

/*! @brief start new try block
 *
 * <tt>try</tt> pushes the current environment onto the stack before executing
 * the block. <b>calling <tt>try</tt> without a corresponding <tt>except</tt>
 * will result in undefined behaviour.</b>
 */
#define try \
	__setjmp_push(); \
	if (exception_empty()) \
		__exception_end(tryenv_pop())

/*! @brief catch exception
 *
 * <tt>except</tt> catches an exception and executes the following block. if no
 * <tt>on</tt> block handled the exception control is given to the next
 * environment on the stack. <b>calling <tt>except</tt> without a corresponding
 * <tt>try</tt> will result in undefined behaviour.</b>
 */
#define except \
	else __exception_block(__exception_handled = exception_push(__FILE__, __LINE__, __FUNCTION__, 0, NULL), \
			__exception_rethrow(__exception_handled))

/*! @brief handle exception
 *
 * <tt>on</tt> handles the exception <tt>errnum</tt>.
 */
#define on(errnum) \
	if (!__exception_handled && exception_errno() == errnum && (__exception_handled = 1)) \
		__exception_end(exception_clear())

/*! @brief handle unknown errors */
#define finally \
	if (!__exception_handled && (__exception_handled = 1)) \
		__exception_end(exception_clear())

/*! @} semantics */

#ifdef CONFIG_DEBUG
#include <stdio.h>
#define debug(...) do { \
	printf("[debug] %s:%d in %s(): ", __FILE__, __LINE__, __FUNCTION__); \
	printf(__VA_ARGS__); \
	printf("\n"); \
} while (0)
#define trace printf("[trace] %s:%d in %s()\n", __FILE__, __LINE__, __FUNCTION__)
#else
#define debug(...)
#define trace
#endif

#endif
