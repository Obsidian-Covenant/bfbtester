dnl Process this file with autoconf to produce a configure script.

dnl Package name + version
AC_INIT([BFBTester], [2.1.0])

dnl A source file that must exist, used to sanity-check the build dir
AC_CONFIG_SRCDIR([src/bfbt/bfbtester.c])

dnl Generate src/config.h from src/config.h.in
AC_CONFIG_HEADERS([src/config.h])

dnl Custom config.h entries (used to be in acconfig.h)
AC_DEFINE([SOLARIS], [1], [Define if building on Solaris])
AC_DEFINE([LINUX],   [1], [Define if building on Linux])
AC_DEFINE([__USE_GNU], [1], [Define for GNU extensions, if needed])

dnl Canonicalize system type (same intent as before)
AC_CANONICAL_HOST

dnl Tell automake about the package
AM_INIT_AUTOMAKE([foreign])

dnl Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL

dnl Checks for libraries.
AC_CHECK_LIB(c_r, pthread_create, [LIBS="$LIBS -pthread"])
AC_CHECK_LIB(pthread, pthread_create)

dnl Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS(fcntl.h sys/time.h unistd.h pthread.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_STRUCT_TM

dnl Checks for library functions.
AC_FUNC_VPRINTF
AC_CHECK_FUNCS(gettimeofday strsignal strcspn strdup strerror strstr readdir_r, ,
				AC_MSG_ERROR(Can't find needed function: $ac_func))

case $build_os in
    solaris*)
        AC_DEFINE([SOLARIS], [1], [Define if building on Solaris])
        CFLAGS="$CFLAGS -D_REENTRANT -D_POSIX_PTHREAD_SEMANTICS"
    ;;
    linux*)
        AC_DEFINE([LINUX], [1], [Define if building on Linux])
    ;;
esac

AC_CONFIG_FILES([
    Makefile
    src/Makefile
    src/bfbt/Makefile
])

AC_OUTPUT
