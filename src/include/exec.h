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

	$Id: exec.h,v 1.2.2.1 2001/01/22 05:31:12 fesnel Exp $
*/

#ifndef __EXEC_H__
#define	__EXEC_H__

int
execute_program		(ExecEntry *exec_entry, char ** args, char ** env);

void
exec_poll			(void);


#define	EXEC_TIMEOUT	5	/* seconds */

#endif /* __EXEC_H__ */
