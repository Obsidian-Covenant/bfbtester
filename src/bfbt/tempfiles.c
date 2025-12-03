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

	$Id: tempfiles.c,v 1.4.2.1 2001/01/22 05:31:12 fesnel Exp $
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h> 

#include <sys/types.h>
#include <sys/time.h>
#include <dirent.h>


#include "datatypes.h"
#include "tempfiles.h"
#include "utils.h"


typedef struct _file_entry FileEntry;
struct _file_entry {

	char		*filename;	/* excludes leading directories */

	struct timeval	tm;

	int			statd;
#ifdef SOLARIS
	uint32_t	fileno;
#else
	u_int32_t	fileno;
#endif /* SOLARIS */
	struct stat	sb;

	FileEntry	*next;
	FileEntry	*prev;

};


typedef struct _tempfile_watch_request TempFileRequest;
struct _tempfile_watch_request {

	pthread_mutex_t	mutex;
	pthread_cond_t	cond;
	int	count;	/* Number of requests - normally 0 or 1 */
	int status; /* 0 - none, 1 - start up, 2 - running, 3 - shutting down */

	TempfileCB	create_cb;	/* creation call back */
	TempfileCB	remove_cb;	/* remove call back */
	void		*cb_data;	/* user data */
};

static int initialized = FALSE;	/* Set to true if we are using tempfile
								 * monitoring
								 */
static FileEntry *head = NULL;
static FileEntry *tail = NULL;
static TempFileRequest request = { PTHREAD_MUTEX_INITIALIZER, 
								   PTHREAD_COND_INITIALIZER,
								   0, 0, NULL, NULL, NULL };

static int
isNew(struct dirent *entry)
{
	FileEntry *entries = head;

#ifdef SOLARIS
	while(entries && entry->d_ino > entries->fileno)
#else
	while(entries && entry->d_fileno > entries->fileno)
#endif /* SOLARIS */
		entries = entries->next;

	if (entries &&
#ifdef SOLARIS
		entries->fileno == entry->d_ino
#else
		entries->fileno == entry->d_fileno
#endif
		)
		return FALSE;
	else
		return TRUE;
}

static void
addFile(struct dirent *entry, struct timeval tm)
{
	FileEntry *new;

	new = xmalloc(sizeof(FileEntry));
	
	new->filename = strdup(entry->d_name);
	new->tm = tm;
#ifdef SOLARIS
	new->fileno = entry->d_ino;
#else
	new->fileno = entry->d_fileno;
#endif /* SOLARIS */
	new->next = NULL;
	new->prev = NULL;

	if (head == NULL) /* tail also NULL */
		head = tail = new;
	else if (new->fileno < head->fileno) {
		head->prev = new;
		new->next = head;
		head = new;
	} else if (new->fileno > tail->fileno) {
		new->prev = tail;
		tail->next = new;
		tail = new;
	} else {
		FileEntry *temp = head;
		while (new->fileno > temp->fileno) temp = temp->next;
		temp->prev->next = new;
		new->prev = temp->prev;
		new->next = temp;
		temp->prev = new;
	}
	/* call creation callback */
	if (request.create_cb && request.count
		&& (request.status == 2 || request.status == 3))
		(*request.create_cb)(new->filename, &new->sb, request.cb_data);
}

static void
updateFile(struct dirent *entry, struct timeval tm)
{
	FileEntry *temp = head;

	/* At this point we know entry is in the list somewhere */
	while (temp->fileno !=
#ifdef SOLARIS
		   entry->d_ino
#else
		   entry->d_fileno
#endif
		) temp = temp->next;

	temp->tm = tm;
}

/* Unlinks from list -- does NOT free() node */
static void
removeFile(FileEntry *entry)
{
	if (entry->prev && entry->next) {
		entry->prev->next = entry->next;
		entry->next->prev = entry->prev;
	} else {
		if (head == entry) {
			if (entry->next)
				entry->next->prev = NULL;
			else /* one item list */
				tail = NULL;
			head = entry->next;
		} else { /* (tail == entry) */
			entry->prev->next = NULL;
			tail = entry->prev;
		}
	}
	/* call deletion callback */
	if (request.remove_cb && request.count
		&& (request.status == 2 || request.status == 3))
		(*request.remove_cb)(entry->filename, &entry->sb, request.cb_data);
}

/* Free memory used by file */
static void
deleteFile(FileEntry *entry)
{
	if (entry->filename) free(entry->filename);
	free(entry);
}

static void
AddOrUpdate(struct dirent *entry, struct timeval tm)
{
	/* Add if entry is new or tag it as still available */
	if (isNew(entry))
		addFile(entry, tm);
	else
		updateFile(entry, tm);
}

static void
tempfile_scandir(DIR *dir)
{
    struct dirent *entry;
    struct timeval tm;
    FileEntry *temp, *dead;

    /* Timestamp for this scan: used to mark entries as "seen" */
    gettimeofday(&tm, NULL);

    /* First pass: add new files / update existing ones */
    while ((entry = readdir(dir)) != NULL) {
        AddOrUpdate(entry, tm);
    }

    /* Second pass: remove files that were not updated this round */
    temp = head;
    while (temp) {
        if (memcmp(&temp->tm, &tm, sizeof(struct timeval)) != 0) {
            dead = temp;
            temp = temp->next;
            removeFile(dead);
            deleteFile(dead);
        } else {
            temp = temp->next;
        }
    }

    /* Rewind for next time */
    rewinddir(dir);
}

static void
tempfile_request_startup(void)
{
	/* Already have mutex locked */
	request.status = 2;
	pthread_cond_signal(&request.cond);
}

static void
tempfile_request_shutdown(void)
{
	/* Already have mutex locked */
	request.status = 0;
	pthread_cond_signal(&request.cond);
}

static void *
tempfile_watch(void *data)
{

	char *dirname;
	DIR	*dir;
	int	lasttime = 0;

	if (!(dirname = getenv("BFBT_TMPDIR")))
		dirname = "/tmp";	
	
	initialized = TRUE;
	if ((dir = opendir(dirname)) == NULL)
		bfbt_message(MSG_TYPE_ERROR_ABORT, "opendir: %s failed", dirname);
	while(1){
		
		tempfile_scandir(dir);

		pthread_mutex_lock(&request.mutex);
		if (request.status == 1 || request.status == 3) {
			if (request.status == 1) {	/* starting a new request */
				if (lasttime) {
					tempfile_request_startup();
					lasttime = 0;
				} else
					lasttime = 1;
			} else /* request.status == 3 */ {	/* shutting down request */
				if (lasttime) {
					tempfile_request_shutdown();
					lasttime = 0;
				} else
					lasttime = 1;
			}
		} else
			while (request.status == 0)
				pthread_cond_wait(&request.cond, &request.mutex);
		pthread_mutex_unlock(&request.mutex);
		pthread_testcancel();
	}
	return NULL;
}

int
tempfile_request_add(TempfileCB	create, TempfileCB remove, void *cbdata)
{
	if (!initialized)
		return 0;
	pthread_mutex_lock(&request.mutex);
	if (request.count || request.status != 0)
		return -1;
	request.count = 1;
	request.status = 1;
	pthread_cond_signal(&request.cond);
	while (request.status == 1)
		pthread_cond_wait(&request.cond, &request.mutex);
	request.create_cb = create;
	request.remove_cb = remove;
	request.cb_data = cbdata;
	pthread_mutex_unlock(&request.mutex);
	return 0;
}

int
tempfile_request_subtract(void)
{
	if (!initialized)
		return 0;
	pthread_mutex_unlock(&request.mutex);
	if (!request.count || request.status != 2)
		return -1;
	request.status = 3;
	while (request.status == 3)
		pthread_cond_wait(&request.cond, &request.mutex);
	request.count--;
	request.create_cb = NULL;
	request.remove_cb = NULL;
	request.cb_data = NULL;
	pthread_mutex_unlock(&request.mutex);
	return 0;
}

void
tempfile_start(THREAD_DATA *data)
{
	if (pthread_create(&data->tempfile_thread, NULL, tempfile_watch, NULL) != 0)
		bfbt_message(MSG_TYPE_ERRNO_ABORT, "tempfile thread create");
}
