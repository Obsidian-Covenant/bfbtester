
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

	$Id: bfbtester.c,v 1.4.2.2 2001/01/22 05:31:12 fesnel Exp $
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/param.h>
#include <dirent.h>
#if defined(SOLARIS) && defined(__sun)
#include <thread.h>     /* for thr_setconcurrency */
#endif

#include "datatypes.h"
#include "data_stuff.h"
#include "main.h"
#include "breaker.h"
#include "utils.h"
#include "tempfiles.h"

#define MAX_SIMULTANEOUS_EXECS	250

static void
usage(char *progname);

static void
run_breaker(void);

static void
load_execs(char **av, ExecEntryStatus status, int flags, char *rejects);

THREAD_DATA	*q;

FILE		*fp_msg = NULL;
int			debug_level;

extern int	optind;
extern char	*optarg;

int
main( int ac, char ** av )
{
	char *progname, *rejects = NULL;
	int t_on = FALSE;	/* tempfile monitoring */
	int	max_execs = MAX_SIMULTANEOUS_EXECS;
	int ch, flags = 0;

	if ((progname = strrchr(av[0], '/')) != NULL)
		progname++;
	else
		progname = av[0];

	while ((ch = getopt(ac, av, "ad:ehmo:r:stvx:")) != -1)
		switch (ch) {
		case 'a':
			flags |= FLAGS_ALL;
			break;
		case 'd':
			debug_level = atoi(optarg);
			break;
		case 'e':
			flags |= FLAGS_ENV_TEST;
			break;
		case 'h':
			usage(progname);
			break;
		case 'm':
			flags |= FLAGS_MULTIPLE_ARG_TEST;
			break;
		case 'o':
			if ((fp_msg = fopen(optarg, "w")) == NULL) {
				printf("Can't open output file: %s\n", optarg);
				return -1;
			}
			break;
		case 'r':
			rejects = optarg;
			break;
		case 's':
			flags |= FLAGS_SINGLE_ARG_TEST;
			break;
		case 't':
			t_on = TRUE;
			break;
		case 'v':
			printf("%s v. %s\n", IDSTR, VERSTR);
			return 0;
			/* NO FALL-THROUGH */
		case 'x':
			max_execs = atoi(optarg);
			break;
		default:
			usage(progname);
			/* NOTREACHED */
		}

	ac -= optind;
	av += optind;

	if (fp_msg == NULL)
		fp_msg = stdout;
	if (!flags || *av == NULL)
		usage(progname);

	q = xcalloc(1, sizeof(THREAD_DATA));
	q->max_execs = max_execs;
	q->mut = xmalloc(sizeof(pthread_mutex_t));

	if (pthread_mutex_init(q->mut, NULL) != 0)
		bfbt_message(MSG_TYPE_ERRNO_ABORT, "mutex init");

#if defined(SOLARIS) && defined(__sun)
    {
        /*
         * On Solaris 2.5, threads are not timesliced so we
         * must increase the concurrency.
         */
        int ret;

        if (t_on)
            ret = thr_setconcurrency(3);
        else
            ret = thr_setconcurrency(2);
        if (ret != 0)
            bfbt_message(MSG_TYPE_ERROR_ABORT, "Can't set thread concurrency");
    }
#endif /* SOLARIS && __sun */

	if (t_on)
		tempfile_start(q);

	load_execs(av, EXEC_ENTRY_STATUS_TEST, flags, rejects);
	run_breaker();

	return 0;

}

static void
usage(char *progname)
{
	printf("%s [-htv] [-d level ] [-r rejects] [-o out-file]\n"
		   "%*s [-x max-execs] -asme file [file2 file3 ...]\n"
		   "Must specify one or more of the following tests:\n"
		   "    -s	Single Argument Test\n"
		   "    -m	Multiple Argument Test\n"
		   "    -e	Environment Variable Test\n"
		   "    -a	Selects all tests\n"
		   "Options:\n"
		   "    -h  Print this message\n"
		   "    -t  Enable tempfile monitoring\n"
		   "    -v  Print version string\n"
		   "    -d level     Set debug level (default = 0, max = 2)\n"
		   "    -r rejects   Comma separated list of binaries to skip\n"
		   "    -o out-file  Output to out-file rather than stdout\n"
		   "    -x max-execs Set maximum executables to run in parallel (default = %d)\n"
		   "file    Specific binary or a directory of binaries to test\n",
		   progname, (int)strlen(progname), "", MAX_SIMULTANEOUS_EXECS);
	exit(2);
}

static void
add_exec(char *filename, ExecEntryStatus status, int flags, char *rejects)
{
	struct stat sb;
	/* Test if exec is to be rejected */
	if (rejects) {
		char *ptr, *ptr2;

		if((ptr=strstr(rejects, filename)))
			if(ptr == rejects || *(ptr-1) == ' ' || *(ptr-1) == ',')
				if(*(ptr+strlen(filename)) == '\0' || *(ptr+strlen(filename)) == ' ' ||
				   *(ptr+strlen(filename)) == ',')
					return;
		if( (ptr2=strrchr(filename, '/')) && (ptr=strstr(rejects, ++ptr2)) )
			if(ptr == rejects || *(ptr-1) == ' ' || *(ptr-1) == ',')
				if(*(ptr+strlen(ptr2)) == '\0' || *(ptr+strlen(ptr2)) == ' ' ||
				   *(ptr+strlen(ptr2)) == ',')
					return;
	}
	/* Test that executable file is valid and accessible */
	if (stat(filename, &sb) == 0 && S_ISREG(sb.st_mode)
		&& access(filename, F_OK|X_OK) == 0)
		/* If we get to here we are safe to add it */
		ExecEntry_add(filename, &sb, status, flags);
}

/* XXX: This is somewhat of a mess */
static void
load_execs(char **av, ExecEntryStatus status, int flags, char *rejects)
{
	struct stat sb;
	char *filename;
	char *pwd = getenv("PWD");

	for (; *av; av++) {
		/* Executable selection is done in one of several ways:
		 *
		 * If the executable filename is specified with a leading
		 * slash (an absolute path), no selection is used and the
		 * supplied absolute filename is used.
		 *
		 * If there is no leading slash in the filename the selection
		 * is made in one of two ways (in this order):
		 *   1) Prepend file name with $PWD and test accesiblity
		 *   2) Search through $PATH and find first accessible executable
		 * The first one to succeed is the executable choosen.
		 *
		 * If the filename found is a directory, we walk the directory (one
		 * level deep) looking for executable binaries.
		 *
		 * Symbolic links are followed.
		 */

		/* already in full pathname form */
		if (*av[0] == '/')
			filename = strdup(*av);
		/* test whether the executable is in the current directory */
		else if (pwd
				 && (filename = xmalloc(strlen(pwd) + strlen(*av) + 2))
				 && sprintf(filename, "%s%s%s", pwd, pwd[strlen(pwd)-1] == '/' ? "" : "/", *av)
				 && access(filename, F_OK|X_OK) == 0)
			;
		/* or in the user's PATH */
		else
			filename = find_exec_in_path(*av);

		if (filename && stat(filename, &sb) == 0
			&& access(filename, F_OK|X_OK) == 0) {
			
			if (S_ISDIR(sb.st_mode)) {		/* Walk directory */
				DIR *dir;
				struct dirent *entry;
				char fname[MAXPATHLEN];
							
				if ((dir = opendir(filename)) == NULL) {
				    bfbt_message(MSG_TYPE_ERROR_WARN, "Can't open dir %s",
				                 filename);
				    free(filename);
				    continue;
				}
				
				while ((entry = readdir(dir)) != NULL) {
				    if (snprintf(fname, sizeof(fname), "%s%s%s", filename,
				                 filename[strlen(filename)-1] == '/' ?
				                 "" : "/", entry->d_name) < (int)sizeof(fname)) {
				        add_exec(fname, status, flags, rejects);
				    }
				}
				
				closedir(dir);
			} else							/* Treat as an executable */
				add_exec(filename, status, flags, rejects);
		} else {
			if (filename)
				bfbt_message(MSG_TYPE_ERROR_WARN, "Can't access file: %s (%s)",
							 filename, *av);
			else
				bfbt_message(MSG_TYPE_ERROR_WARN, "Can't find file: %s", *av);
		}
		if (filename)
			free(filename);
	}
}

static void
run_breaker(void)
{
	if (!ExecEntry_isempty()) {
		breaker_start();
		breaker_wait();
	}
}
