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

#include <errno.h>
#include <stdbool.h>
#include <setjmp.h>

#include "list.h"

/*!
 * @defgroup exception exception stack
 *
 * The exception API provides a primitive stack interface to record an
 * exception and its trace back to the location where it will be handled.
 *
 * @{
 */

/*!
 * @brief the exception struct
 *
 * an object of type <tt>exception_t</tt> stores one exception and a
 * doubly-linked list to create the exception stack.
 */
typedef struct {
	list_t list;      /*!< exception stack list */
	const char *file; /*!< source file of this exception */
	const char *func; /*!< function of this exception */
	int line;         /*!< line in the source file of this exception */
	int errnum;       /*!< the errno value when this exception was thrown */
	char *msg;        /*!< the error message for this exception */
} exception_t;

/*!
 * @brief clear the exception stack
 *
 * <tt>exception_clear</tt> can be used in <tt>catch</tt> blocks to ignore the
 * current exception and clean up the exception stack. failure to clean these
 * stacks while ignoring the thrown exception will result in <b>undefined
 * behaviour.</b>
 */
void exception_clear(void);

/*!
 * @brief check if exceptions exist
 *
 * <tt>exception_empty</tt> checks whether the exception stack is empty.
 *
 * @note this function should not be used directly, <tt>try</tt>/<tt>catch</tt>
 * macros provide better semantics.
 *
 * @return <tt>true</tt> if the exception stack is empty, <tt>false</tt>
 *         otherwise.
 */
bool exception_empty(void);

/*!
 * @brief create new exception
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
 */
void exception_push(const char *file, int line, const char *func,
		int errnum, const char *fmt, ...);

/*!
 * @brief get last exception
 *
 * <tt>exception_pop</tt> deletes and returns the topmost exception object on
 * the stack.
 *
 * @return a pointer to the topmost exception object
 */
exception_t *exception_pop(void);

/*!
 * @brief get standard exception trace
 *
 * <tt>exception_dump</tt> returns an exception trace in a standard format.
 *
 * @note a call to this function clears the exception stack.
 *
 * @returns pointer to exception trace string
 */
char *exception_dump(void);

/*! @} exception */

/*!
 * @defgroup tryenv jump environment
 *
 * The tryenv API provides a primitive stack interface to record jump
 * environments needed by non-local goto (a.k.a <tt>siglongjmp</tt>) used by
 * <tt>throw</tt>.
 *
 * @{
 */

/*! @brief global environment buffer for the current <tt>try</tt> block */
extern jmp_buf __tryenv_buffer;

/*!
 * @brief create new jump environment
 *
 * <tt>tryenv_push</tt> creates a new <tt>tryenv_t</tt> object for the current
 * jump buffer and pushes it onto the environment stack.
 *
 * @note this function should not be used directly, <tt>try</tt> provides
 * better semantics.
 */
void tryenv_push(void);

/*!
 * @brief remove last jump environment
 *
 * <tt>tryenv_pop</tt> deletes the topmost environment on the environment
 * stack.
 *
 * @note this function should not be used directly, <tt>catch</tt> provides
 * better semantics.
 */
void tryenv_pop(void);

/*!
 * @brief jump to last environment
 *
 * <tt>tryenv_jmp</tt> jumps to the environment saved in the topmost
 * <tt>tryenv_t</tt> object on the environment stack.
 *
 * @note this function should not be used directly, <tt>throw</tt> provides
 * better semantics.
 */
void tryenv_jmp(void);

/*! @} tryenv */

/*!
 * @defgroup semantics try/catch semantics
 *
 * The following macros provide try/catch semantics ontop of the exception and
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

/*! @brief start new try block
 *
 * <tt>try</tt> records the current environment in the global buffer and pushes
 * it into the stack before executing the block. <b>calling <tt>try</tt>
 * without a corresponding <tt>catch</tt> will result in undefined
 * behaviour.</b>
 */
#define try \
	if (sigsetjmp(__tryenv_buffer, 1) == 0) { \
		tryenv_push(); \
	} \
	if (exception_empty())

/*! @brief catch exception
 *
 * <tt>catch</tt> removes the topmost jump environment and executes the catch
 * block if there are any exceptions. <b>calling <tt>catch</tt> without a
 * corresponding <tt>try</tt> will result in undefined behaviour.</b>
 */
#define catch \
	tryenv_pop(); \
	if (!exception_empty())

/*! @brief pass exception to callee
 *
 * <tt>pass</tt> passes the current exceptions to the calling function by
 * throwing a new exception without error message.
 */
#define pass \
	throw(errno, NULL)

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
