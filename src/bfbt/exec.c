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

	$Id: exec.c,v 1.4.2.1 2001/01/22 05:31:12 fesnel Exp $
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <signal.h>

#include "datatypes.h"
#include "data_stuff.h"
#include "exec.h"
#include "utils.h"

extern	THREAD_DATA	*q;

typedef enum _exec_status ExecStatus;
enum _exec_status {
	EXEC_STATUS_DONE,
	EXEC_STATUS_STALE,
	EXEC_STATUS_GARBAGE,
	EXEC_STATUS_NOTDONE
};

int
execute_program(ExecEntry *exec_entry, char ** args, char ** env)
{
	TestEntry *test_entry;

	if( args == NULL )
		return -1;
	if( q->current_execs >= q->max_execs ){
		bfbt_message(MSG_TYPE_DEBUG_L1, "Hit maximum execs");
		return -1;
	}
	test_entry = TestEntry_create(exec_entry, args, env);

	switch( test_entry->pid = fork() ){
	case -1:

		bfbt_message(MSG_TYPE_ERROR_WARN, "Couldn't fork process");
		TestEntry_destroy(test_entry);
		return (1);

	case 0:
		
	{
		pid_t	pgid = getpid();
		if (setpgid(pgid, pgid) != 0)
			/* XXX: should fail completely */
			bfbt_message(MSG_TYPE_ERRNO_WARN, "Couldn't set process group");
	}
	
		close(0);
		close(1);
		close(2);

		execve(test_entry->progname, test_entry->args, test_entry->env);

		/* if we get here then the execve() didn't work 
		 * NOTE: The error message will not print if we are
		 * outputting to stdout/stderr
		 */
		bfbt_message(MSG_TYPE_ERRNO_WARN, "execve '%s'", test_entry->progname);
		_exit(2);

	default:

		gettimeofday(&(test_entry->tm), NULL);

		TestEntry_insert(test_entry);
		q->current_execs++;

	}
	return (0);
}

static void
exec_kill(pid_t pid)
{
	kill(-(pid), SIGKILL);
}

static ExecStatus
exec_GetStatus(TestEntry *entry)
{
	struct timeval	tm;
#define	EXEC_STALE		20	/* seconds */
#define EXEC_GARBAGE	(2 * EXEC_STALE)	/* Entry is garbage -- orphan it */

	if( waitpid(entry->pid, &(entry->status), WNOHANG) > 0 )
		return EXEC_STATUS_DONE;

	gettimeofday(&tm, NULL);
	if (tm.tv_sec - entry->tm.tv_sec > EXEC_TIMEOUT){
		exec_kill(entry->pid);
		if( waitpid(entry->pid, &(entry->status), WNOHANG) > 0 )
			return EXEC_STATUS_DONE;
	}
	if (tm.tv_sec - entry->tm.tv_sec > EXEC_GARBAGE)
		return EXEC_STATUS_GARBAGE;
	if (tm.tv_sec - entry->tm.tv_sec > EXEC_STALE)
		return EXEC_STATUS_STALE;

	return EXEC_STATUS_NOTDONE;
}

static void
exec_process_close(TestEntry *entry)
{
	q->current_execs--;
	if( WIFEXITED(entry->status) )
		; /* nothing bad happened to it */
	else if( WIFSIGNALED(entry->status) ){
		/* we ignore SIGKILL and SIGPIPE */
		if( WTERMSIG(entry->status) == SIGKILL
			|| WTERMSIG(entry->status) == SIGPIPE){
			return;
		}
		{
			/* we crashed -- record */
			CrashReport * report = CrashReport_add(entry);
			CrashReport_log(report);
		}
	} else
		bfbt_message(MSG_TYPE_ERROR_WARN, 
					 "%s ended in unknown condition, status=%d",
					 entry->progname, entry->status);
}


void
exec_poll(void)
{
	TestEntry *temp, *dead;

	temp = NULL;
	for (temp = TestEntry_getnext(&temp); temp;)
		switch (exec_GetStatus(temp)) {
		case EXEC_STATUS_NOTDONE:
			temp = TestEntry_getnext(&temp);
			break;
		case EXEC_STATUS_DONE:
			exec_process_close(temp);
			/* FALL THROUGH */
		case EXEC_STATUS_GARBAGE:
			dead = temp;
			temp = TestEntry_getnext(&temp);
			TestEntry_remove(dead);
			TestEntry_destroy(dead);
			break;
		case EXEC_STATUS_STALE:
			break;
		}
}

