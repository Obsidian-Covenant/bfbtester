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

	$Id: data_stuff.h,v 1.2.2.2 2001/01/22 05:31:12 fesnel Exp $
*/

#ifndef __DATA_STUFF_H__
#define __DATA_STUFF_H__


ExecEntry *
ExecEntry_add(char *name, struct stat *sb, ExecEntryStatus status,
    int test_flags);

void
ExecEntry_subtract(ExecEntry *entry);

void
ExecEntry_remove(ExecEntry *entry);

ExecEntry *
ExecEntry_getnext(ExecEntry **entry);

ExecEntry *
ExecEntry_findbyname(char *name);

void
ExecEntry_release(ExecEntry *entry);

int
ExecEntry_isempty(void);

void
TestEntry_destroy(TestEntry *entry);

TestEntry *
TestEntry_create(ExecEntry *parent, char **args, char **env);

void
TestEntry_insert(TestEntry *entry);

void
TestEntry_remove(TestEntry *entry);

TestEntry *
TestEntry_getnext(TestEntry **entry);

CrashReport *
CrashReport_add(TestEntry *test_entry);

void
CrashReport_subtract(CrashReport *report);

void
CrashReport_log(CrashReport *report);


#endif /* __DATA_STUFF_H__ */
