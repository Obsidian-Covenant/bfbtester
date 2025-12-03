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

	$Id: breaker_data.h,v 1.2.2.1 2001/01/22 05:31:12 fesnel Exp $
*/


#ifndef __BREAKER_DATA_H__
#define __BREAKER_DATA_H__

const char *args_options[] = { \
/* one characters */
"-0","-1","-2","-3","-4","-5",
"-6","-7","-8","-9","-@","-A",
"-B","-C","-D","-E","-F","-G",
"-H","-I","-J","-K","-L","-M",
"-N","-O","-P","-Q","-R","-S",
"-T","-U","-V","-W","-X","-Y",
"-Z","-a","-b","-c","-d","-e",
"-f","-g","-h","-i","-j","-k",
"-l","-m","-n","-o","-p","-q",
"-r","-s","-t","-u","-v","-w",
"-x","-y","-z"
};


/* various environment variables */
const char *envs[] = { \
					   "USER", "MAIL", "HOME", "ENV", "MORE", "TOP","CVSROOT",
					   "OLDPWD", "WMAKER_BIN_NAME", "PAGER", "RSTAR_NO_WARN",
					   "OPTIND", "LOGNAME", "WINDOWID", "MAILHOST","SHOPTYPE",
					   "TERM", "BLOCKSIZE", "DISPLAY","OPENSSL_CONF",
					   "PWD", "TERMCAP", "EDITOR", "POSIXLY_CORRECT",
					   "BC_ENV_ARGS", "BC_LINE_LENGTH", "GNUTARGET","GENOCIDED",
					   "COLUMNS", "EMULATION_ENVIRON", "TARGET_ENVIRON",
					   "LD_RUN_PATH", "LD_LIBRARY_PATH", "TMPDIR","KRB5_CONFIG",
					   "TMP", "TEMP", "CVS_SERVER", "CVS_CLIENT_PORT",
					   "CVS_CLIENT_LOG", "CVS_RSH", "IGNORE_ENV",
					   "CVS_PASSFILE", "CVS_PASSWORD","TMPDIR_ENV","SCSI_MODES",
					   "EDITOR1_ENV", "EDITOR2_ENV","EDITOR3_ENV","QUERY_STRING"
					   "CVSROOT_ENV", "CVSREAD_ENV", "CVSREADONLYFS_ENV",
					   "CVS_OPTIONS", "CVSUMASK_ENV", "CVS_SERVER_SLEEP",
					   "WRAPPER_ENV", "DIFF_OPTIONS", "DIFF","GETARGMANDOC",
					   "NAME, ENV_NAME", "LANG", "SUNPRO_DEPENDENCIES",
					   "DEPENDENCIES_OUTPUT","COLLECT_NO_DEMANGLE","SSL_CIPHER",
					   "COLLECT_GCC_OPTIONS", "COLLECT_GCC", "COMPILER_PATH",
					   "LIBRARY_PATH", "TLINK_VERBOSE", "SHELL","SSLEAY_CONF",
					   "PATH", "HISTSIZE", "GDBHISTFILE", "PS2", "EMACS",
					   "_POSIX_OPTION_ORDER", "TZ", "PATCH_GET", "AFSLIBPATH",
					   "SIMPLE_BACKUP_SUFFIX", "PATCH_VERSION_CONTROL","RMT",
					   "VERSION_CONTROL", "INFOPATH", "INFO_PRINT_COMMAND",
					   "LINES", "USERNAME", "AFS_SYSCALL",
					   "HZ", "GREP_OPTIONS", "SYSTEM", "MANROFFSEQ","KRBTKFILE",
					   "MANSECT", "MANLOCALES", "MANPATH", "PATCH_INDEX_FIRST",
					   "COMSPEC", "TAPE", "MODEMS", "MUSIC_CD", "CD_DRIVE",
					   "DISC", "CDPLAY", "CHANGER_ENV_VAR", "LC_CTYPE",
					   "LS_COLWIDTHS", "LC_TIME", "AWKPATH", "LOCALDEF",
					   "DSTKEYPATH", "NET_ORDER", "HES_DOMAIN", "IRPD_HOST_ENV",
					   "LOCALDOMAIN", "LCD_TYPE",
					   "CVS_LOCAL_BRANCH_NUM", "PROFDIR", "UDICONF", "KERB_DBG",
					   "GTAGSWARNING", "GTAGSDBPATH", "GTAGSROOT", "KRBCONFDIR",
					   "GTAGSLIBPATH", "BROWSER", "GTAGSCONF", "GTAGSLABEL",
					   "GROFF_COMMAND_PREFIX",
					   "GROFF_FONT_PATH", "GROFF_TYPESETTER", "KRB4PRINCIPAL",
					   "GROFF_TMPDIR_ENVVAR","TMPDIR_ENVVAR","REFER","RANDFILE",
					   "FONTPATH_ENV_VAR","KRB5CCNAME","BUFMOD_FIXED","EL_SIZE",
					   "ABCDE", "MOUSE_BUTTONS_123", "NCURSES_NO_PADDING",
					   "BAUDRATE", "CC", "TERMINFO", "TERMINFO_DIRS","FAITH_NS",
					   "TERMPATH", "NCURSES_NO_SETBUF", "KEYBOARD", "HACKDIR",
					   "LC_MESSAGES", "CSCOPE_DIRS", "NEXINIT", "EXINIT",
					   "TTYPROMPT", "PERL_BADFREE", "PERL_DESTRUCT_LEVEL",
					   "PERL5OPT", "PERL_DEBUG_MSTATS", "PERL5LIB", "PERLLIB",
					   "YYDEBUG", "TERMCAP_COLORS", "PERL5DB", "LANGUAGE",
					   "LC_ALL", "LC_COLLATE", "LC_NUMERIC", "PERL_BADLANG",
					   "PERL_DL_NONLAZY", "HOSTALIASES", "GAI", "USE_UNICODE",
					   "HACKPAGER", "MAGIC", "HACKOPTIONS", "INVENT", "VISUAL",
					   "MAILREADER", "ROGUEOPTS", "WOTD", "SAILNAME", "LIBDIR",
					   "DIALOGRC", "RCSLOCALID", "RCSINCEXC", "RCSINIT",
					   "LDSO", "OBJFORMAT", "RES_OPTIONS", "NLSPATH", "REMOTE",
					   "MALLOC_OPTIONS", "LIBC_R_DEBUG", "DEBUGTTY","MAKE",
					   "HTTP_PROXY", "FTP_TIMEOUT", "LD_BIND_NOW",
					   "LD_DEBUG", "LD_PRELOAD", "LD_TRACE_LOADED_OBJECTS",
					   "LD_TRACE_LOADED_OBJECTS_PROGNAME", "SYSINSTALL_DEBUG",
					   "LD_TRACE_LOADED_OBJECTS_FMT1", "VINUM_DATEFORMAT",
					   "LD_TRACE_LOADED_OBJECTS_FMT2", "VINUM_HISTORY",
					   "PROM_E_BOOTED_DEV", "PROM_E_BOOTED_OSFLAGS","PHONES",
					   "PROM_E_BOOTED_FILE", "kernelname",
					   "PROM_E_TTY_DEV", "rootdev", "currdev", "autoboot_delay",
					   "bootfile", "DOS_KERNEL", "MINUSKVAR", "FTP_PASSWORD",
					   "FTP_LOGIN", "FTP_PROXY", "HTTP_TIMEOUT",
					   "HTTP_AUTH", "HTTP_PROXY_AUTH", "FINGER", "REPLYTO",
					   "FTPSERVERPORT", "LOCATE_PATH", "MACHINE","MACHINE_ARCH",
					   "MAKEOBJDIRPREFIX", "MAKEOBJDIR", "MAKEFLAGS", 
					   "OBJFORMAT_PATH", "NLSPROVIDER", "V_TERMCAP", "PRINTER",
					   "PACKAGESITE", "_TOP", "PKG_DBDIR", "PKG_ADD_BASE",
					   "PKG_PATH", "PKG_TMPDIR", "CONNECT",
					   "EL_EDITOR", "PW_SCAN_BIG_IDS" };

#endif /* __BREAKER_DATA_H__ */
