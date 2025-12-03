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

	$Id: breaker.c,v 1.9.2.1 2001/01/22 05:31:12 fesnel Exp $
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "datatypes.h"
#include "breaker_data.h"
#include "exec.h"
#include "utils.h"
#include "tempfiles.h"
#include "data_stuff.h"

extern THREAD_DATA 	*q;

static void
breaker_execute_command(ExecEntry *exec_entry, char ** args, char ** env)
{
	int		i;
#ifdef LINUX
	if (q->current_execs > (q->max_execs / 2))
		exec_poll();
#endif

	for(i=10000;  execute_program(exec_entry, args, env) != 0 ; i*=3) {
		usleep(i);
#ifdef LINUX
		exec_poll();
#endif
	}
}

static void
breaker_test_singlearg(ExecEntry *entry)
{
	char	*args[4]= { entry->fullname,
						NULL, NULL, NULL };
	char	*long_arg;
	int 	i;
	int		long_arg_len=50*1024+1, short_arg_len=5*1024+1;
	
	bfbt_message(MSG_TYPE_MESSAGE, "   * Single argument testing");
	long_arg = xcalloc(1, short_arg_len);
	memset(long_arg, 'A', short_arg_len-1);
	args[2]=long_arg;
	for(i=0; i < (sizeof(args_options)/sizeof(args_options[0])); i++){
		args[1]=(char *)args_options[i];
		breaker_execute_command(entry, args, NULL);
	}
	free(long_arg);
	long_arg = xcalloc(1, long_arg_len);
	memset(long_arg, 'A', long_arg_len-1);
	args[2]=long_arg;
	for(i=0; i < (sizeof(args_options)/sizeof(args_options[0])); i++){
		args[1]=(char *)args_options[i];
		breaker_execute_command(entry, args, NULL);
	}
	args[1]=long_arg;
	args[2]=NULL;
	breaker_execute_command(entry, args, NULL);
	free(long_arg);
	sleep(1);
	
}

static void
breaker_test_multiplearg(ExecEntry *entry)
{
	char	*args[7]={NULL};
	char	*long_arg;
	int		i,j;
	int		arg_len=10*1024+1; /* only one arg */
	args[0] = entry->fullname;
	
	bfbt_message(MSG_TYPE_MESSAGE, "   * Multiple arguments testing");
	long_arg = xcalloc(1, arg_len);
	memset(long_arg, 'A', arg_len-1);
	args[2] = long_arg; args[3] = long_arg; args[4] = NULL;
	for(i=0; i < (sizeof(args_options)/sizeof(args_options[0])); i++){
		args[1]=(char *)args_options[i];
		breaker_execute_command(entry, args, NULL);
	}
	args[2]=long_arg; args[4]=long_arg; args[5]=NULL;
	for(i=0; i < (sizeof(args_options)/sizeof(args_options[0])); i++){
		args[1] = (char *)args_options[i];
		for(j=0; j < (sizeof(args_options)/sizeof(args_options[0])); j++){
			args[3] = (char *)args_options[j];
			breaker_execute_command(entry, args, NULL);
		}
	}
	args[2]=long_arg; args[4]=long_arg; args[5]=long_arg; args[6]=NULL;
	for(i=0; i < (sizeof(args_options)/sizeof(args_options[0])); i++){
		args[1] = (char *)args_options[i];
		for(j=0; j < (sizeof(args_options)/sizeof(args_options[0])); j++){
			args[3] = (char *)args_options[j];
			breaker_execute_command(entry, args, NULL);
		}
	}
	args[1] = long_arg; args[2] = long_arg; args[3] = NULL;
	breaker_execute_command(entry, args, NULL);
	free(long_arg);
	sleep(1);
}

static void
breaker_test_env(ExecEntry *entry)
{
	char	*args_arr[]={entry->fullname, NULL, NULL};
	char	*envs_arr[]={NULL, NULL};
	char	*long_arg, *long_arg2;
	int		i, env_len;
	int		long_arg_len=10*1024 + 1;
	
	bfbt_message(MSG_TYPE_MESSAGE, "   * Environment variable testing");
	long_arg = xcalloc(1, long_arg_len);
	long_arg2 = xcalloc(1, long_arg_len);
	memset(long_arg2, 'A', long_arg_len-1);
	envs_arr[0]=long_arg;
	for(i=0; i < (sizeof(envs)/sizeof(envs[0])); i++){
		env_len = strlen(envs[i]);
		if(env_len+1 >= long_arg_len-1) continue; /*ahh!*/
		strcpy(long_arg, envs[i]);
		long_arg[env_len] = '=';
		memset(long_arg+env_len+1, 'A', long_arg_len - env_len -1 -1);
		
		breaker_execute_command(entry, args_arr, envs_arr); /* env no args */
		
		args_arr[1]=(char *)long_arg2;
		breaker_execute_command(entry, args_arr, envs_arr);
		args_arr[1]=NULL;
	}
	free(long_arg);
	free(long_arg2);
}

void
breaker_tempfile_createcb(char *filename, struct stat *sb, void *data)
{
	ExecEntry *entry = (ExecEntry *)data;

	bfbt_message(MSG_TYPE_MESSAGE, "[Tempfile: %s]+ \"%s\"",
				 entry->fullname, filename);
}

void
breaker_tempfile_removecb(char *filename, struct stat *sb, void *data)
{
	ExecEntry *entry = (ExecEntry *)data;

	bfbt_message(MSG_TYPE_MESSAGE, "[Tempfile: %s]- \"%s\"",
				 entry->fullname, filename);
}

static void
breaker_runtests(ExecEntry *entry)
{
	int test_flags = entry->test_flags;

	tempfile_request_add(breaker_tempfile_createcb,
						 breaker_tempfile_removecb, (void *)entry);

	bfbt_message(MSG_TYPE_MESSAGE, "=> %s", entry->fullname);
	if (entry->filestat.st_mode & S_ISUID)
		bfbt_message(MSG_TYPE_MESSAGE, " (setuid: %d)",
					 (int) entry->filestat.st_uid);
	if (entry->filestat.st_mode & S_ISGID)
		bfbt_message(MSG_TYPE_MESSAGE, " (setgid: %d)",
					 (int) entry->filestat.st_gid);
	
	/* Single arg test */
	if (test_flags & FLAGS_SINGLE_ARG_TEST)
		breaker_test_singlearg(entry);
	
	/* Multiple args test */
	if (test_flags & FLAGS_MULTIPLE_ARG_TEST)
		breaker_test_multiplearg(entry);
	
	/* Environment variable test */
	if (test_flags & FLAGS_ENV_TEST)
		breaker_test_env(entry);

	tempfile_request_subtract();
}

/* Cleanup routine
 * Runs exec_poll() twice after 5 second delay
 */
static void
breaker_cleanup(void * arg)
{
	bfbt_message(MSG_TYPE_MESSAGE, "Cleaning up...might take a few seconds");
	sleep(EXEC_TIMEOUT+1);
	exec_poll();
	exec_poll();
}

void *
breaker_run(void *data)
{
	ExecEntry *entry=NULL;

	entry = ExecEntry_getnext(&entry);
	while(entry) {
		if (entry->status == EXEC_ENTRY_STATUS_TEST){
			gettimeofday(&entry->testtime, NULL);
			breaker_runtests(entry);
			entry->status = EXEC_ENTRY_STATUS_TESTED;
		}
		entry = ExecEntry_getnext(&entry);
	}

#ifdef LINUX
	breaker_cleanup(NULL);
#endif

	return NULL;
}

#ifndef LINUX
static void *
poll_execs(void *data)
{

	pthread_cleanup_push(breaker_cleanup, NULL);
	for(;;){
		exec_poll();
		usleep(100000);
		pthread_testcancel();
	}
	pthread_cleanup_pop(0);
	return NULL;
}
#endif /* LINUX */

void
breaker_start(void)
{
	if( pthread_create(&(q->run_thread), NULL, breaker_run, NULL) != 0 )
		bfbt_message(MSG_TYPE_ERRNO_ABORT, "Can't create run thread");
#ifndef LINUX
	if( pthread_create(&(q->poll_thread), NULL, poll_execs, NULL) != 0 )
		bfbt_message(MSG_TYPE_ERRNO_ABORT, "Can't create poll thread");
#endif
}

void
breaker_wait(void)
{
	pthread_join(q->run_thread, NULL);
#ifndef LINUX
	pthread_cancel(q->poll_thread);
	pthread_join(q->poll_thread, NULL);
#endif
}
