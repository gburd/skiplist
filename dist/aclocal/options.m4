# $Id: options.ac 7 2007-02-03 13:34:17Z gburd $

# Process user-specified options.
AC_DEFUN(AM_OPTIONS_SET, [


AC_MSG_CHECKING(if --disable-statistics option specified)
AC_ARG_ENABLE(statistics,
	AS_HELP_STRING([--disable-statistics],
	    [Do not build statistics support.]),, enableval="yes")
db_cv_build_statistics="$enableval"
case "$enableval" in
 no) AC_MSG_RESULT(yes);;
yes) AC_MSG_RESULT(no);;
esac

AC_MSG_CHECKING(if --enable-posixmutexes option specified)
AC_ARG_ENABLE(posixmutexes,
	[AS_HELP_STRING([--enable-posixmutexes],
			[Force use of POSIX standard mutexes.])],
	[db_cv_posixmutexes="$enable_posixmutexes"], [db_cv_posixmutexes="no"])
AC_MSG_RESULT($db_cv_posixmutexes)

AC_MSG_CHECKING(if --enable-debug option specified)
AC_ARG_ENABLE(debug,
	[AS_HELP_STRING([--enable-debug],
			[Build a debugging version.])],
	[db_cv_debug="$enable_debug"], [db_cv_debug="no"])
AC_MSG_RESULT($db_cv_debug)

AC_MSG_CHECKING(if --enable-diagnostic option specified)
AC_ARG_ENABLE(diagnostic,
	[AS_HELP_STRING([--enable-diagnostic],
			[Build a version with run-time diagnostics.])],
	[db_cv_diagnostic="$enable_diagnostic"], [db_cv_diagnostic="no"])
AC_MSG_RESULT($db_cv_diagnostic)

AC_MSG_CHECKING(if --enable-test option specified)
AC_ARG_ENABLE(test,
	[AS_HELP_STRING([--enable-test],
			[Configure to run the test suite.])],
	[db_cv_test="$enable_test"], [db_cv_test="no"])
AC_MSG_RESULT($db_cv_test)

])
