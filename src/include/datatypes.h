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

	$Id: datatypes.h,v 1.2.2.1 2001/01/22 05:31:12 fesnel Exp $
*/

#ifndef __DATATYPES_H__
#define __DATATYPES_H__

#include <sys/time.h>
#include <sys/stat.h>
#include "bfbt_queue.h"

#include <pthread.h>

typedef struct _crash_report	CrashReport;
struct _crash_report {
	char 		*progname;
	char		*args_str;
	char		*env_str;
	int			signal;
	int			didCore;

	DLIST_ENTRY(_crash_report) link;
};

typedef enum _exec_entry_status	ExecEntryStatus;
enum _exec_entry_status {
	EXEC_ENTRY_STATUS_NOTHING,	/* Exec has been added but doesn't
								 * need/want testing (mainly for GUI)
								 */
	EXEC_ENTRY_STATUS_TEST,		/* Exec needs testing on next pass */
	EXEC_ENTRY_STATUS_TESTED	/* Test has been run at specified time */
};

typedef struct _exec_entry	ExecEntry;
struct _exec_entry {

	char			*fullname;		/* absolute file path */
	ExecEntryStatus	status;

	int				test_flags;
	
	struct stat		filestat;
	struct timeval	testtime;
	DLIST_ENTRY(_exec_entry) link;

};


typedef struct _test_entry	TestEntry;
struct _test_entry {

	pid_t		pid;
	int			status;		/* as used with wait() */

	char		*progname;
	char		**args;
	char		*args_str;	/* string version */
	char		**env;
	char		*env_str;	/* string version of extra args (ie. overrides) */
	int		   	env_flag;	/* if (TRUE) - using environ and 'env' above
							 * shouldn't be free()'d
							 */
	struct timeval tm;	/* start time */
	
	DLIST_ENTRY(_test_entry) link;
};

typedef struct _thread_data	THREAD_DATA;
struct _thread_data {
	pthread_mutex_t		*mut;
	pthread_t			run_thread;
	pthread_t			poll_thread;
	pthread_t			tempfile_thread;
	int					current_execs;
	int					max_execs;
};


#ifdef	TRUE
#undef	TRUE
#undef	FALSE
#endif
#define	FALSE	(1 == 0)
#define TRUE	(!FALSE)

#ifndef MAXPATHLEN
#define MAXPATHLEN	1024
#endif

#define FLAGS_SINGLE_ARG_TEST	(1<<0)
#define FLAGS_MULTIPLE_ARG_TEST	(1<<1)
#define FLAGS_ENV_TEST			(1<<2)
#define FLAGS_ALL (FLAGS_SINGLE_ARG_TEST|FLAGS_MULTIPLE_ARG_TEST|FLAGS_ENV_TEST)


#endif /* __DATATYPES_H__ */
