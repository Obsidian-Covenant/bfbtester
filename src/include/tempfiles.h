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

	$Id: tempfiles.h,v 1.2.2.1 2001/01/22 05:31:12 fesnel Exp $
*/

#ifndef __TEMPFILES_H__
#define __TEMPFILES_H__

typedef void (*TempfileCB)(char *, struct stat *, void *data);

int
tempfile_request_add(TempfileCB	create, TempfileCB remove, void *cbdata);

int
tempfile_request_subtract(void);

void
tempfile_start(THREAD_DATA *data);

#endif /* __TEMPFILES_H__ */
