/*
    BFBTester -- Brute Force Binary Tester
    Copyright (C) 2000 Mike Heffner <mheffner@vt.edu>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

	$Id: utils.h,v 1.6.2.1 2001/01/22 05:31:12 fesnel Exp $
*/

#ifndef	__UTILS_H__
#define __UTILS_H__



typedef enum _msg_type	MsgType;
enum _msg_type {
	MSG_TYPE_ERROR_ABORT,
	MSG_TYPE_ERROR_WARN,
	MSG_TYPE_ERRNO_ABORT,
	MSG_TYPE_ERRNO_WARN,
	MSG_TYPE_MESSAGE,		/* no function/line output */
	MSG_TYPE_DEBUG_L1,
	MSG_TYPE_DEBUG_L2
};

#ifdef __GNUC__

#define bfbt_message(type, args...) \
__bfbt_message(type, __PRETTY_FUNCTION__, __LINE__, args)

void
__bfbt_message(MsgType type, const char *function, int line,
               const char *fmt, ...) __attribute__ ((format (printf, 4, 5)));

#else

#define bfbt_message(type, fmt, args...) \
__bfbt_message(type, __FUNCTION__, __LINE__, fmt, ##args)

void
__bfbt_message(MsgType type, const char *function, int line,
               const char *fmt, ...);

#endif /* __GNUC__ */

/* Memory allocators */
#define xmalloc(a)      _xmalloc(a, __FUNCTION__, __LINE__)
static __inline__ void *
_xmalloc(size_t size, const char *function, int line)
{
    void *temp;
    temp = malloc(size);
    if (!temp)
        bfbt_message(MSG_TYPE_ERRNO_ABORT, "Malloc failed at [%s:%d]",
                     function, line);
    return temp;
}

#define xcalloc(a, b)   _xcalloc(a, b, __FUNCTION__, __LINE__)
static __inline__ void *
_xcalloc(size_t number, size_t size, const char *function, int line)
{
    void *temp;
    temp = calloc(number, size);
    if (!temp)
        bfbt_message(MSG_TYPE_ERRNO_ABORT, "Calloc failed at [%s:%d]",
                     function, line);
    return temp;
}

char *
find_exec_in_path(char *name);

char *
args_to_str(char **args);

char *
env_to_str(char **env);

char **
env_create(char **env);

#endif /* __UTILS_H__ */
