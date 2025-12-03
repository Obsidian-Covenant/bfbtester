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

	$Id: bfbt_queue.h,v 1.2.2.1 2001/01/22 05:31:12 fesnel Exp $
*/


#ifndef __BFBT_QUEUE_H__
#define __BFBT_QUEUE_H__

/* List macros derived from those in sys/queue.h */

/* DLIST: Double linked list with top pointer (thread protected) */

/* private: */
#define _DLIST_ISFIRST(elm, field)	((elm)->field.dle_prev == NULL)
#define _DLIST_ISLAST(elm, field)	((elm)->field.dle_next == NULL)
#define _DLIST_ISEMPTY(head)		((head)->dlh_first == NULL)

#define _DLIST_FIRST(head)			((head)->dlh_first)
#define	_DLIST_NEXT(elm, field)		((elm)->field.dle_next)
#define _DLIST_PREV(elm, field)		((elm)->field.dle_prev)

#define	_DLIST_INIT_ENTRY(elm, field)	do {							\
	pthread_mutex_init(&(elm)->field.dle_access, NULL);					\
	(elm)->field.dle_reader_count = 0;									\
	(elm)->field.dle_writer_count = 0;									\
	pthread_cond_init(&(elm)->field.dle_reader_wait, NULL);				\
	pthread_cond_init(&(elm)->field.dle_writer_wait, NULL);				\
} while (0)
#define _DLIST_DESTROY_ENTRY(elm, field) do {							\
	pthread_mutex_destroy(&(elm)->field.dle_access);					\
	pthread_cond_destroy(&(elm)->field.dle_reader_wait);				\
	pthread_cond_destroy(&(elm)->field.dle_writer_wait);				\
} while (0)

#define _DLIST_READ_ACCESS(elm, field) do {								\
	pthread_mutex_lock(&(elm)->field.dle_access);						\
	while ((elm)->field.dle_writer_count)								\
		pthread_cond_wait(&(elm)->field.dle_reader_wait, &(elm)->field.dle_access);\
	(elm)->field.dle_reader_count++;									\
	pthread_mutex_unlock(&(elm)->field.dle_access);						\
} while (0)
#define _DLIST_READ_DEACCESS(elm, field) do {							\
	pthread_mutex_lock(&(elm)->field.dle_access);						\
	(elm)->field.dle_reader_count--;									\
	if ((elm)->field.dle_writer_count && (elm)->field.dle_reader_count == 0)\
		pthread_cond_broadcast(&(elm)->field.dle_writer_wait);			\
	pthread_mutex_unlock(&(elm)->field.dle_access);						\
} while (0)

#define _DLIST_WRITE_ACCESS(elm, field) do {							\
	pthread_mutex_lock(&(elm)->field.dle_access);						\
	(elm)->field.dle_writer_count++;									\
	while ((elm)->field.dle_reader_count)								\
		pthread_cond_wait(&(elm)->field.dle_writer_wait, &(elm)->field.dle_access);\
	pthread_mutex_unlock(&(elm)->field.dle_access);						\
} while (0)
#define _DLIST_WRITE_DEACCESS(elm, field) do {							\
	pthread_mutex_lock(&(elm)->field.dle_access);						\
	(elm)->field.dle_writer_count--;									\
	if ((elm)->field.dle_reader_count /* && (elm)->field.dle_writer_count == 0 */)\
		pthread_cond_broadcast(&(elm)->field.dle_reader_wait);			\
	pthread_mutex_unlock(&(elm)->field.dle_access);						\
} while (0)

/* public: */
#define	DLIST_HEAD(name, type)											\
struct name {															\
	struct type 	*dlh_first;	/* first element */						\
	pthread_mutex_t	dlh_access;	/* protect head */						\
	pthread_mutex_t dlh_ops;	/* synchronize insertion/deletion ops */\
}


#define DLIST_HEAD_INITIALIZER(head)									\
{ NULL, PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER }

#define DLIST_ENTRY(type)												\
struct {																\
	struct type		*dle_next;	/* next element */						\
	struct type		*dle_prev;	/* previous element */					\
	pthread_mutex_t	dle_access;	/* protect entry */						\
	unsigned int	dle_reader_count;	/* readers of current entry */	\
	unsigned int	dle_writer_count;	/* writers of current entry */	\
	pthread_cond_t	dle_reader_wait;	/* wait if write is in progress */	\
	pthread_cond_t	dle_writer_wait;	/* wait if read is in progress */	\
}

#define DLIST_INIT(head) do {											\
	(head)->dlh_first = NULL;											\
	pthread_mutex_init(&(head)->dlh_access, NULL);						\
	pthread_mutex_init(&(head)->dlh_ops, NULL);							\
} while (0)

#define DLIST_DESTROY(head) do {										\
	pthread_mutex_destroy(&(head)->dlh_ops);							\
	pthread_mutex_destroy(&(head)->dlh_access);							\
} while(0)

#define DLIST_INSERT_HEAD(head, elm, field) do {						\
	_DLIST_INIT_ENTRY((elm), field);									\
	pthread_mutex_lock(&(head)->dlh_ops);								\
	pthread_mutex_lock(&(head)->dlh_access);							\
	_DLIST_WRITE_ACCESS((elm), field);									\
	if (! _DLIST_ISEMPTY((head))) {										\
		_DLIST_WRITE_ACCESS(_DLIST_FIRST((head)), field);				\
		_DLIST_PREV(_DLIST_FIRST((head)), field) = (elm);				\
		_DLIST_WRITE_DEACCESS(_DLIST_FIRST((head)), field);				\
	}																	\
	_DLIST_NEXT((elm), field) = _DLIST_FIRST((head));					\
	_DLIST_PREV((elm), field) = NULL;									\
	_DLIST_FIRST((head)) = (elm);										\
	_DLIST_WRITE_DEACCESS((elm), field);								\
	pthread_mutex_unlock(&(head)->dlh_access);							\
	pthread_mutex_unlock(&(head)->dlh_ops);								\
} while (0)


#define DLIST_REMOVE(head, elm, field) do {								\
	/* Caller must assure no one will remove							\
	 * this element before we do										\
	 */																	\
	pthread_mutex_lock(&(head)->dlh_ops);								\
	if (_DLIST_ISFIRST((elm), field)) {									\
		pthread_mutex_lock(&(head)->dlh_access);						\
		_DLIST_FIRST((head)) = _DLIST_NEXT((elm), field);				\
	} else {															\
		_DLIST_WRITE_ACCESS(_DLIST_PREV((elm), field), field);			\
		_DLIST_NEXT(_DLIST_PREV((elm), field), field) = _DLIST_NEXT((elm), field);\
	}																	\
	_DLIST_WRITE_ACCESS((elm), field);									\
	if (! _DLIST_ISLAST((elm), field)) {								\
		/* XXX: We don't write-lock next element, however we don't		\
		 * go backwards in queue so it isn't too bad					\
		 */																\
		/*_DLIST_WRITE_ACCESS(_DLIST_NEXT((elm), field), field);*/		\
		_DLIST_PREV(_DLIST_NEXT((elm), field), field) = _DLIST_PREV((elm), field);\
		/*_DLIST_WRITE_DEACCESS(_DLIST_NEXT((elm), field), field);*/	\
	}																	\
	_DLIST_WRITE_DEACCESS((elm), field);								\
	if (_DLIST_ISFIRST((elm), field))									\
		pthread_mutex_unlock(&(head)->dlh_access);						\
	else																\
		_DLIST_WRITE_DEACCESS(_DLIST_PREV((elm), field), field);		\
	pthread_mutex_unlock(&(head)->dlh_ops);								\
	_DLIST_DESTROY_ENTRY((elm), field);									\
} while (0)

#define DLIST_GETNEXT(head, p_elm, field) do {							\
	if (*(p_elm) == NULL) {	/* start at top */							\
		pthread_mutex_lock(&(head)->dlh_access);						\
		*(p_elm) = _DLIST_FIRST((head));								\
		if (*(p_elm) != NULL)											\
			_DLIST_READ_ACCESS(*(p_elm), field);						\
		pthread_mutex_unlock(&(head)->dlh_access);						\
	} else if (_DLIST_ISLAST(*(p_elm), field)) {						\
		_DLIST_READ_DEACCESS(*(p_elm), field);							\
		*(p_elm) = NULL;												\
	} else {															\
		*(p_elm) = _DLIST_NEXT(*(p_elm), field);						\
		_DLIST_READ_ACCESS(*(p_elm), field);							\
		_DLIST_READ_DEACCESS(_DLIST_PREV(*(p_elm), field), field);		\
	}																	\
} while (0)

#define DLIST_RELEASE(elm, field)	_DLIST_READ_DEACCESS((elm), field)

/* This macro shouldn't be used for any purpose other than to test
 * the instantaneous `emptyness' of a list
 */
#define DLIST_ISEMPTY(head)	_DLIST_ISEMPTY((head))

#endif /* __BFBT_QUEUE_H__ */
