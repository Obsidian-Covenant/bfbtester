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

	$Id: utils.c,v 1.3.2.1 2001/01/22 05:31:12 fesnel Exp $
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>

#include <sys/param.h>

#include "datatypes.h"
#include "utils.h"

extern FILE *fp_msg;
extern int debug_level;

void
__bfbt_message(MsgType type, const char *function, int line,
               const char *fmt, ...)
{
	va_list args;

	if (type == MSG_TYPE_DEBUG_L1 && debug_level < 1)	/* Level 1 */
		return;
	if (type == MSG_TYPE_DEBUG_L2 && debug_level < 2)	/* Level 2 */
		return;
	va_start(args, fmt);

	if (type != MSG_TYPE_MESSAGE)
		fprintf(fp_msg, "[%s:%d] ", function, line);
	vfprintf(fp_msg, fmt, args);
	if (type == MSG_TYPE_ERRNO_ABORT || type == MSG_TYPE_ERRNO_WARN)
		fprintf(fp_msg, ": %s", strerror(errno));
	fprintf(fp_msg, "\n");

	if (type == MSG_TYPE_ERROR_ABORT || type == MSG_TYPE_ERRNO_ABORT)
		exit(1);

	va_end(args);
}

/* Searches the PATH environment variable for an absolute
 * filename for name.
 * Result must be free()'d
 */
char *
find_exec_in_path(char *name)
{
	char *str, *buf, *last, *path = getenv("PATH");
	char filename[MAXPATHLEN];

	assert(name != NULL && name[0] != '\0');

	if (path) {
		if ((buf = strdup(path)) == NULL)
			bfbt_message(MSG_TYPE_ERROR_ABORT, "out of memory");
		str = strtok_r(buf, ":", &last);
		for (;str;str = strtok_r(NULL, ":", &last)) {
			if (snprintf(filename, sizeof(filename),
						 "%s%s%s", str, 
						 (str[strlen(str)-1] == '/' ? "" : "/"),
						 name) >= sizeof(filename))
				continue;
			if (access(filename, F_OK|X_OK) == 0) {
				free(buf);
				return strdup(filename);
			}
		}
		free(buf);
	}
	return NULL;
}

char *
args_to_str(char **args)
{
	char *str;
	char **temp;
	int i;
#define LONG_ARG_FMT		"[%5.5d]"
#define	LONG_ARG_LEN		(sizeof("[12345]")-1)
#define MAX_VALID_ARG_LEN	100

	if (*(args+1) == NULL)
		return NULL;

	for (i = 0, temp = args+1; *temp; temp++)
		i += (strlen(*temp) > MAX_VALID_ARG_LEN ? LONG_ARG_LEN 
			  : strlen(*temp)) + 1;
	str = xcalloc(1, i);
	for (temp = args+1; *temp; temp++){
		if (strlen(*temp) > MAX_VALID_ARG_LEN) {
			char long_str[LONG_ARG_LEN+1];
			snprintf(long_str, LONG_ARG_LEN+1,
					 LONG_ARG_FMT, (int)strlen(*temp));
			strcat(str, long_str);
		} else
			strcat(str, *temp);
		if (*(temp+1))
			strcat(str, " ");
	}
	return str;
}

char *
env_to_str(char **env)
{
	char **temp, *str;
	int i;
#define		LONG_ENV_FMT			"[%5.5d]"
#define		LONG_ENV_LEN			(sizeof("[12345]")-1)
#define		MAX_VALID_ENV_LEN		100
	
	for (i = 0, temp = env; *temp; temp++)
		i += (strlen(*temp) > MAX_VALID_ENV_LEN ? 
			  (strcspn(*temp, "=") + 1 + LONG_ENV_LEN) 
			  : strlen(*temp)) + 1;
	str = xcalloc(1, i);
	for (temp = env; *temp; temp++) {
		if (strlen(*temp) > MAX_VALID_ENV_LEN) {
			char long_env[LONG_ENV_LEN+1];
			i = strcspn(*temp, "=");
			strncat(str, *temp, i+1);
			snprintf(long_env, LONG_ENV_LEN+1, LONG_ENV_FMT,
					 (int)strlen(*temp+i+1));
			strcat(str, long_env);
		} else
			strcat(str, *temp);
		if (*(temp+1))
			strcat(str, " ");
	}
	return str;
}

static inline int
env_not_used(char **env, char *str)
{
	int len = strcspn(str, "=");
	char **temp;
	for (temp = env; *temp; temp++)
		if (strncmp(*temp, str, len) == 0)
			return FALSE;
	return TRUE;
}

extern char ** environ;

char **
env_create(char **env)
{
	int i, j;
	char **temp, **temp2;

	/* NOTE: To save a few CPU cycles we malloc the sum of all entries
	 * in env and environ, even if there may be duplicates in env and environ.
	 * This will create a temporary 'n * sizeof(char *)' memory bloat, however
	 * n will probably only be about 1 or 2.
	 */
	for (i = 0, temp = env; *temp; temp++, i++);
	for (temp = environ; *temp; temp++, i++);

	/* Merge env and environ */
	temp2 = xcalloc(1, (i+1) * sizeof(*temp2));
	for (j = 0, temp = env; *temp; j++, temp++)
		temp2[j] = *temp;
	for (temp = environ; *temp; temp++)
		if (env_not_used(env, *temp)) {
			temp2[j] = *temp;
			j++;
		}
	return temp2;
}
