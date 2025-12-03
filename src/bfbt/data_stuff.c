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

	$Id: data_stuff.c,v 1.4.2.2 2001/01/22 05:31:12 fesnel Exp $
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef LINUX
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/wait.h>


#include "datatypes.h"
#include "data_stuff.h"
#include "utils.h"

#include "bfbt_queue.h"

/* E X E C   E N T R Y  */

DLIST_HEAD(exec_entry_head, _exec_entry) ExecEntry_Head;

static void
ExecEntry_constructor(void)				__attribute__ ((constructor));

static void
ExecEntry_constructor(void)
{
	DLIST_INIT(&ExecEntry_Head);
}

static inline void
ExecEntry_init(ExecEntry *entry, struct stat *sb, char *fullname, 
			   ExecEntryStatus status, int test_flags)
{
	entry->fullname = strdup(fullname);
	memcpy(&entry->filestat, sb, sizeof(entry->filestat));
	entry->status = status;
	entry->test_flags = test_flags;
	entry->testtime.tv_sec = 0;
	entry->testtime.tv_usec = 0;
}

static void
ExecEntry_destroy(ExecEntry *entry)
{
	assert(entry != NULL);
	if (entry->fullname)	free(entry->fullname);
	free(entry);
}

static inline int
ExecEntry_compare(ExecEntry *e1, ExecEntry *e2)
{
	if (e1->filestat.st_ino == e2->filestat.st_ino)
		return TRUE;
	else
		return FALSE;
}

ExecEntry *
ExecEntry_add(char *name, struct stat *sb, ExecEntryStatus status,
	    int test_flags)
{
	ExecEntry *new;

	assert(name != NULL && sb != NULL);

	new = xmalloc(sizeof(ExecEntry));
	ExecEntry_init(new, sb, name, status, test_flags);
	DLIST_INSERT_HEAD(&ExecEntry_Head, new, link);
	return new;
}

void
ExecEntry_subtract(ExecEntry *entry)
{
	assert(entry != NULL);
	DLIST_REMOVE(&ExecEntry_Head, entry, link);
	ExecEntry_destroy(entry);
}

ExecEntry *
ExecEntry_getnext(ExecEntry **entry)
{
	DLIST_GETNEXT(&ExecEntry_Head, entry, link);
	return *entry;
}

ExecEntry *
ExecEntry_findbyname(char *name)
{
	ExecEntry *entry = NULL;
	
	entry = ExecEntry_getnext(&entry);
	while (entry && strcmp(entry->fullname, name) != 0)
		entry = ExecEntry_getnext(&entry);
	return entry;
}

void
ExecEntry_release(ExecEntry *entry)
{
	DLIST_RELEASE(entry, link);
}

int
ExecEntry_isempty(void)
{
	return DLIST_ISEMPTY(&ExecEntry_Head);
}

/* T E S T   E N T R Y */

DLIST_HEAD(test_entry_head, _test_entry) TestEntry_Head;

static void
TestEntry_constructor(void)			__attribute__  ((constructor));

static void
TestEntry_constructor(void)
{
	DLIST_INIT(&TestEntry_Head);
}

extern char ** environ;

static inline void
TestEntry_init(TestEntry *entry, ExecEntry *parent, char **args, char **env)
{
	assert(entry != NULL);
	entry->pid = -1;
	entry->status = 0;
	if ((entry->progname = strdup(parent->fullname)) == NULL)
		bfbt_message(MSG_TYPE_ERRNO_ABORT, "Out of memory");
	entry->args = args;
	entry->args_str = args_to_str(entry->args);
	if (env) {
		entry->env = env_create(env);
		entry->env_flag = FALSE;
		entry->env_str = env_to_str(env);
	} else {
		entry->env = environ;
		entry->env_flag = TRUE;
		entry->env_str = NULL;
	}
}

void
TestEntry_destroy(TestEntry *entry)
{
	assert(entry != NULL);
	if (entry->progname)	free(entry->progname);
	if (entry->args_str)	free(entry->args_str);
	if (entry->env && entry->env_flag == FALSE)	free(entry->env);
	if (entry->env_str)		free(entry->env_str);
	free(entry);
}

TestEntry *
TestEntry_create(ExecEntry *parent, char **args, char **env)
{
	TestEntry *new;

	new = xmalloc(sizeof(TestEntry));
	TestEntry_init(new, parent, args, env);

	return new;
}

void
TestEntry_insert(TestEntry *entry)
{
	assert(entry != NULL && entry->pid != -1);
	DLIST_INSERT_HEAD(&TestEntry_Head, entry, link);
}

void
TestEntry_remove(TestEntry *entry)
{
	assert(entry != NULL && entry->pid != -1);

	DLIST_REMOVE(&TestEntry_Head, entry, link);
}

TestEntry *
TestEntry_getnext(TestEntry **entry)
{
	DLIST_GETNEXT(&TestEntry_Head, entry, link);
	return *entry;
}

/* C R A S H   R E P O R T S */

DLIST_HEAD(crashreport_slist_head, _crash_report) CrashReport_Head;

static void
CrashReport_constructor(void)				__attribute__ ((constructor));

static void
CrashReport_constructor(void)
{
	DLIST_INIT(&CrashReport_Head);
}

static void
CrashReport_init(CrashReport *report, TestEntry *test_entry)
{
	report->progname = strdup(test_entry->progname);
	report->args_str = (test_entry->args_str ? strdup(test_entry->args_str)
						: NULL);
	report->env_str = (test_entry->env_str ? strdup(test_entry->env_str)
					   : NULL);
	report->signal	= (int)WTERMSIG(test_entry->status);
	report->didCore = (int)WCOREDUMP(test_entry->status);
}

static void
CrashReport_destroy(CrashReport *report)
{
	if (report->progname)	free(report->progname);
	if (report->args_str)	free(report->args_str);
	if (report->env_str)	free(report->env_str);
	free(report);
}

CrashReport *
CrashReport_add(TestEntry *test_entry)
{
	CrashReport	*new;

	assert(test_entry != NULL);
	new = xmalloc(sizeof(CrashReport));
	CrashReport_init(new, test_entry);
	DLIST_INSERT_HEAD(&CrashReport_Head, new, link);
	return new;
}

void
CrashReport_subtract(CrashReport *report)
{
	assert(report != NULL);
	DLIST_REMOVE(&CrashReport_Head, report, link);
	CrashReport_destroy(report);
}

void
CrashReport_log(CrashReport *report)
{
	bfbt_message(MSG_TYPE_MESSAGE,
				 "*** Crash <%s> ***\n"
				 "args:		%s\n"
				 "envs:		%s\n"
				 "Signal:		%d ( %s )\n"
				 "Core?		%s\n",
				 report->progname,
				 report->args_str ? report->args_str : "",
				 report->env_str ? report->env_str : "",
				 report->signal, strsignal(report->signal),
				 report->didCore ? "Yes" : "No");
}





