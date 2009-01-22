// Copyright (c) 2006-2009 Benedikt Böhm <bb@xnull.de>
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

/*! @defgroup exception exception stack
 *
 * The exception API provides a primitive stack interface to record an
 * exception and its trace back to the location where it will be handled.
 *
 * @{
 */

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
 * <tt>try</tt>/<tt>except</tt> provide better semantics.
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
 * @note this function should not be used directly, <tt>on</tt> provides better
 * semantics.
 *
 * @return errno value
 */
int exception_errno(void);

/*! @brief create new exception
 *
 * <tt>exception_push</tt> creates a new exception object and pushes it onto
 * the exception stack.
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
 * @param fd file descriptor
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
 * @note this function should not be used directly, <tt>throw</tt> provides
 * better semantics.
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

/*! @brief throw new exception
 *
 * <tt>throw</tt> creates a new exception object with <tt>exception_push</tt>
 * and jumps to the topmost environment on the stack.
 */
#define throw(...) do { \
	exception_push(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); \
	tryenv_jmp(); \
} while (0)

/* executes start before and end after the block */
#define __exception_block(start, end) \
	for (int __exception_block_pass = 1, start; \
	     __exception_block_pass; \
	     end, __exception_block_pass = 0)

/* executes end after the block */
#define __exception_end(end) \
	for (int __exception_block_pass = 1; \
	     __exception_block_pass; \
	     end, __exception_block_pass = 0)

/* push new environment on the stack */
#define __setjmp_push() do { \
	jmp_buf __tryenv_buffer; \
	int __tryenv_ret = setjmp(__tryenv_buffer); \
	tryenv_push(&__tryenv_buffer, __tryenv_ret); \
} while (0)

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

/* this cannot be a macro because __exception_block does not allow brace
 * expressions in for loops */
static inline
void __exception_rethrow(int handled)
{
	if (!handled)
		tryenv_jmp();
}

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
 * <tt>on</tt> handles the exception <tt>errnum</tt>, passes control to the
 * following block and clears the exception stack afterwards.
 */
#define on(errnum) \
	if (!__exception_handled && exception_errno() == errnum && (__exception_handled = 1)) \
		__exception_end(exception_clear())

/*! @brief handle unknown exceptions
 *
 * <tt>finally</tt> handles <em>all</em> exceptions, passes control to the
 * following block and clears the exception stack afterwards.
 */
#define finally \
	if (!__exception_handled && (__exception_handled = 1)) \
		__exception_end(exception_clear())

/*! @} semantics */

#endif
