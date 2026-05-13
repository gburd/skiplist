#                                               -*- Autoconf -*-
# configure.ac for skiplist -- header-only C splay-list with lock-free
# concurrency.
#
# This is intentionally a thin autoconf script.  Skiplist has no
# platform-specific code that requires feature detection; the
# implementation in include/sl.h already handles MSVC vs GNU and
# concurrent vs single-threaded via preprocessor macros.
#
# What ./configure does:
#   1. Validates that the C compiler accepts -std=c11.
#   2. Locates pthread support (required for concurrent tests).
#   3. Locates libm (required for the bench and splay rebalance).
#   4. Substitutes prefix/CC/CFLAGS/etc. into config.mk and skiplist.pc.
#
# The hand-written Makefile includes config.mk if present; plain
# `make` without configure still works because all variables have
# sensible defaults inside the Makefile.

AC_PREREQ([2.69])
AC_INIT([skiplist], [1.0.0], [greg@burd.me], [skiplist],
        [https://codeberg.org/gregburd/skiplist])

AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_SRCDIR([include/sl.h])

AC_PROG_CC
AC_PROG_INSTALL

# Allow --mandir override (autoconf provides the variable for free,
# we just substitute it into config.mk).
AC_SUBST([mandir])

# Validate -std=c11.
AC_MSG_CHECKING([whether $CC accepts -std=c11])
saved_CFLAGS=$CFLAGS
CFLAGS="$CFLAGS -std=c11"
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([], [])],
  [AC_MSG_RESULT([yes])],
  [AC_MSG_RESULT([no])
   AC_MSG_ERROR([a C11-capable compiler is required])])
CFLAGS=$saved_CFLAGS

# Pthreads -- search the conventional spellings.
AC_MSG_CHECKING([for pthread support])
PTHREAD_FLAGS=
AC_LINK_IFELSE(
  [AC_LANG_PROGRAM([[#include <pthread.h>]],
                   [[pthread_t t; pthread_create(&t, 0, 0, 0);]])],
  [AC_MSG_RESULT([none needed])],
  [saved_LIBS=$LIBS
   LIBS="$LIBS -lpthread"
   AC_LINK_IFELSE(
     [AC_LANG_PROGRAM([[#include <pthread.h>]],
                      [[pthread_t t; pthread_create(&t, 0, 0, 0);]])],
     [AC_MSG_RESULT([-lpthread])
      PTHREAD_FLAGS="-lpthread"],
     [LIBS=$saved_LIBS
      saved_CFLAGS=$CFLAGS
      CFLAGS="$CFLAGS -pthread"
      AC_LINK_IFELSE(
        [AC_LANG_PROGRAM([[#include <pthread.h>]],
                         [[pthread_t t; pthread_create(&t, 0, 0, 0);]])],
        [AC_MSG_RESULT([-pthread])
         PTHREAD_FLAGS="-pthread"],
        [AC_MSG_ERROR([cannot link pthread programs])])
      CFLAGS=$saved_CFLAGS])
   LIBS=$saved_LIBS])
AC_SUBST([PTHREAD_FLAGS])

# libm.
AC_CHECK_LIB([m], [pow], [LIBM="-lm"], [
  AC_MSG_ERROR([libm is required])
])
AC_SUBST([LIBM])

# Sanitizer toggle -- off by default.
AC_ARG_ENABLE([sanitizers],
  [AS_HELP_STRING([--enable-sanitizers],
                  [build with ASan + UBSan + LSan (default: no)])],
  [enable_sanitizers=$enableval], [enable_sanitizers=no])

EXTRA_CFLAGS=
EXTRA_LDFLAGS=
AS_IF([test "x$enable_sanitizers" = "xyes"], [
  EXTRA_CFLAGS="-fsanitize=address,leak,undefined -fno-omit-frame-pointer"
  EXTRA_LDFLAGS="-fsanitize=address,leak,undefined"
])

# Coverage toggle.
AC_ARG_ENABLE([coverage],
  [AS_HELP_STRING([--enable-coverage],
                  [build with gcov instrumentation (default: no)])],
  [enable_coverage=$enableval], [enable_coverage=no])

AS_IF([test "x$enable_coverage" = "xyes"], [
  EXTRA_CFLAGS="$EXTRA_CFLAGS -O0 -g --coverage"
  EXTRA_LDFLAGS="$EXTRA_LDFLAGS --coverage"
])

AC_SUBST([EXTRA_CFLAGS])
AC_SUBST([EXTRA_LDFLAGS])

AC_CONFIG_FILES([
  config.mk
])

AC_OUTPUT

cat <<EOF

skiplist $PACKAGE_VERSION configured.

  prefix:        $prefix
  CC:            $CC
  CFLAGS:        $CFLAGS $EXTRA_CFLAGS
  pthread:       $PTHREAD_FLAGS
  sanitizers:    $enable_sanitizers
  coverage:      $enable_coverage

Build with:    make
Run tests:     make test test_concurrent
Install with:  make install

EOF
