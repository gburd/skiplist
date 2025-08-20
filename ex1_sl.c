/* Copyright (C) 1991-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* This header is separate from features.h so that the compiler can
   include it implicitly at the start of every compilation.  It must
   not itself include <features.h> or any other header that includes
   <features.h> because the implicit include comes before any feature
   test macros that may be defined in a source file before it first
   explicitly includes a system header.  GCC knows the name of this
   header in order to preinclude it.  */

/* glibc's intent is to support the IEC 559 math functionality, real
   and complex.  If the GCC (4.9 and later) predefined macros
   specifying compiler intent are available, use them to determine
   whether the overall intent is to support these features; otherwise,
   presume an older compiler has intent to support these features and
   define these macros by default.  */
/* wchar_t uses Unicode 10.0.0.  Version 10.0 of the Unicode Standard is
   synchronized with ISO/IEC 10646:2017, fifth edition, plus
   the following additions from Amendment 1 to the fifth edition:
   - 56 emoji characters
   - 285 hentaigana
   - 3 additional Zanabazar Square characters */
#pragma GCC push_options
#pragma GCC optimize("O0")

// OPTIONS to set before including sl.h
// ---------------------------------------------------------------------------

// Include our monolithic ADT, the Skiplist!
// ---------------------------------------------------------------------------
/*
 * Copyright (c) 2024
 *	Gregory Burd <greg@burd.me>.  All rights reserved.
 *
 * ISC License Permission to use, copy, modify, and/or distribute this software
 * for any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * I'd like to thank others for thoughtfully licensing their work, the
 * community of software engineers succeeds when we work together.
 *
 * Portions of this code are derived from other copyrighted works:
 *
 *  - MIT License
 *    - https://github.com/greensky00/skiplist
 *      2017-2024 Jung-Sang Ahn <jungsang.ahn@gmail.com>
 *    - https://github.com/paulross/skiplist
 *      Copyright (c) 2017-2023 Paul Ross <paulross@uky.edu>
 *    - https://github.com/JP-Ellis/rust-skiplist
 *      Copyright (c) 2015 Joshua Ellis <github@jpellis.me>
 *  - Public Domain
 *    - https://gist.github.com/zhpengg/2873424
 *      Zhipeng Li <zhpeng.is@gmail.com>
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"

/* Copyright (C) 1991-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/*
 *	ISO C99 Standard: 7.2 Diagnostics	<assert.h>
 */
/* Copyright (C) 1991-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* These are defined by the user (or the compiler)
   to specify the desired environment:

   __STRICT_ANSI__	ISO Standard C.
   _ISOC99_SOURCE	Extensions to ISO C89 from ISO C99.
   _ISOC11_SOURCE	Extensions to ISO C99 from ISO C11.
   _ISOC2X_SOURCE	Extensions to ISO C99 from ISO C2X.
   __STDC_WANT_LIB_EXT2__
            Extensions to ISO C99 from TR 27431-2:2010.
   __STDC_WANT_IEC_60559_BFP_EXT__
            Extensions to ISO C11 from TS 18661-1:2014.
   __STDC_WANT_IEC_60559_FUNCS_EXT__
            Extensions to ISO C11 from TS 18661-4:2015.
   __STDC_WANT_IEC_60559_TYPES_EXT__
            Extensions to ISO C11 from TS 18661-3:2015.
   __STDC_WANT_IEC_60559_EXT__
            ISO C2X interfaces defined only in Annex F.

   _POSIX_SOURCE	IEEE Std 1003.1.
   _POSIX_C_SOURCE	If ==1, like _POSIX_SOURCE; if >=2 add IEEE Std 1003.2;
            if >=199309L, add IEEE Std 1003.1b-1993;
            if >=199506L, add IEEE Std 1003.1c-1995;
            if >=200112L, all of IEEE 1003.1-2004
            if >=200809L, all of IEEE 1003.1-2008
   _XOPEN_SOURCE	Includes POSIX and XPG things.  Set to 500 if
            Single Unix conformance is wanted, to 600 for the
            sixth revision, to 700 for the seventh revision.
   _XOPEN_SOURCE_EXTENDED XPG things and X/Open Unix extensions.
   _LARGEFILE_SOURCE	Some more functions for correct standard I/O.
   _LARGEFILE64_SOURCE	Additional functionality from LFS for large files.
   _FILE_OFFSET_BITS=N	Select default filesystem interface.
   _ATFILE_SOURCE	Additional *at interfaces.
   _DYNAMIC_STACK_SIZE_SOURCE Select correct (but non compile-time constant)
            MINSIGSTKSZ, SIGSTKSZ and PTHREAD_STACK_MIN.
   _GNU_SOURCE		All of the above, plus GNU extensions.
   _DEFAULT_SOURCE	The default set of features (taking precedence over
            __STRICT_ANSI__).

   _FORTIFY_SOURCE	Add security hardening to many library functions.
            Set to 1, 2 or 3; 3 performs stricter checks than 2, which
            performs stricter checks than 1.

   _REENTRANT, _THREAD_SAFE
            Obsolete; equivalent to _POSIX_C_SOURCE=199506L.

   The `-ansi' switch to the GNU C compiler, and standards conformance
   options such as `-std=c99', define __STRICT_ANSI__.  If none of
   these are defined, or if _DEFAULT_SOURCE is defined, the default is
   to have _POSIX_SOURCE set to one and _POSIX_C_SOURCE set to
   200809L, as well as enabling miscellaneous functions from BSD and
   SVID.  If more than one of these are defined, they accumulate.  For
   example __STRICT_ANSI__, _POSIX_SOURCE and _POSIX_C_SOURCE together
   give you ISO C, 1003.1, and 1003.2, but nothing else.

   These are defined by this file and are used by the
   header files to decide what to declare or define:

   __GLIBC_USE (F)	Define things from feature set F.  This is defined
            to 1 or 0; the subsequent macros are either defined
            or undefined, and those tests should be moved to
            __GLIBC_USE.
   __USE_ISOC11		Define ISO C11 things.
   __USE_ISOC99		Define ISO C99 things.
   __USE_ISOC95		Define ISO C90 AMD1 (C95) things.
   __USE_ISOCXX11	Define ISO C++11 things.
   __USE_POSIX		Define IEEE Std 1003.1 things.
   __USE_POSIX2		Define IEEE Std 1003.2 things.
   __USE_POSIX199309	Define IEEE Std 1003.1, and .1b things.
   __USE_POSIX199506	Define IEEE Std 1003.1, .1b, .1c and .1i things.
   __USE_XOPEN		Define XPG things.
   __USE_XOPEN_EXTENDED	Define X/Open Unix things.
   __USE_UNIX98		Define Single Unix V2 things.
   __USE_XOPEN2K        Define XPG6 things.
   __USE_XOPEN2KXSI     Define XPG6 XSI things.
   __USE_XOPEN2K8       Define XPG7 things.
   __USE_XOPEN2K8XSI    Define XPG7 XSI things.
   __USE_LARGEFILE	Define correct standard I/O things.
   __USE_LARGEFILE64	Define LFS things with separate names.
   __USE_FILE_OFFSET64	Define 64bit interface as default.
   __USE_MISC		Define things from 4.3BSD or System V Unix.
   __USE_ATFILE		Define *at interfaces and AT_* constants for them.
   __USE_DYNAMIC_STACK_SIZE Define correct (but non compile-time constant)
            MINSIGSTKSZ, SIGSTKSZ and PTHREAD_STACK_MIN.
   __USE_GNU		Define GNU extensions.
   __USE_FORTIFY_LEVEL	Additional security measures used, according to level.

   The macros `__GNU_LIBRARY__', `__GLIBC__', and `__GLIBC_MINOR__' are
   defined by this file unconditionally.  `__GNU_LIBRARY__' is provided
   only for compatibility.  All new code should use the other symbols
   to test for features.

   All macros listed above as possibly being defined by this file are
   explicitly undefined if they are not explicitly defined.
   Feature-test macros that are not defined by the user or compiler
   but are implied by the other feature-test macros defined (or by the
   lack of any definitions) are defined by the file.

   ISO C feature test macros depend on the definition of the macro
   when an affected header is included, not when the first system
   header is included, and so they are handled in
   <bits/libc-header-start.h>, which does not have a multiple include
   guard.  Feature test macros that can be handled from the first
   system header included are handled here.  */

/* Undefine everything, so we get a clean slate.  */
/* Suppress kernel-name space pollution unless user expressedly asks
   for it.  */

/* Convenience macro to test the version of gcc.
   Use like this:
   #if __GNUC_PREREQ (2,8)
   ... code requiring gcc 2.8 or later ...
   #endif
   Note: only works for GCC 2.0 and later, because __GNUC_MINOR__ was
   added in 2.0.  */

/* Similarly for clang.  Features added to GCC after version 4.2 may
   or may not also be available in clang, and clang's definitions of
   __GNUC(_MINOR)__ are fixed at 4 and 2 respectively.  Not all such
   features can be queried via __has_extension/__has_feature.  */

/* Whether to use feature set F.  */

/* _BSD_SOURCE and _SVID_SOURCE are deprecated aliases for
   _DEFAULT_SOURCE.  If _DEFAULT_SOURCE is present we do not
   issue a warning; the expectation is that the source is being
   transitioned to use the new macro.  */

/* If _GNU_SOURCE was defined by the user, turn on all the other features.  */
/* If nothing (other than _GNU_SOURCE and _DEFAULT_SOURCE) is defined,
   define _DEFAULT_SOURCE.  */
/* This is to enable the ISO C2X extension.  */

/* This is to enable the ISO C11 extension.  */

/* This is to enable the ISO C99 extension.  */

/* This is to enable the ISO C90 Amendment 1:1995 extension.  */
/* If none of the ANSI/POSIX macros are defined, or if _DEFAULT_SOURCE
   is defined, use POSIX.1-2008 (or another version depending on
   _XOPEN_SOURCE).  */
/* Some C libraries once required _REENTRANT and/or _THREAD_SAFE to be
   defined in all multithreaded code.  GNU libc has not required this
   for many years.  We now treat them as compatibility synonyms for
   _POSIX_C_SOURCE=199506L, which is the earliest level of POSIX with
   comprehensive support for multithreaded code.  Using them never
   lowers the selected level of POSIX conformance, only raises it.  */
/* Features part to handle 64-bit time_t support.
   Copyright (C) 2021-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* We need to know the word size in order to check the time size.  */
/* Determine the wordsize from the preprocessor defines.  */
/* Both x86-64 and x32 use the 64-bit system call interface.  */
/* Bit size of the time_t type at glibc build time, x86-64 and x32 case.
   Copyright (C) 2018-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* Determine the wordsize from the preprocessor defines.  */
/* Both x86-64 and x32 use the 64-bit system call interface.  */

/* For others, time size is word size.  */
/* The function 'gets' existed in C89, but is impossible to use
   safely.  It has been removed from ISO C11 and ISO C++14.  Note: for
   compatibility with various implementations of <cstdio>, this test
   must consider only the value of __cplusplus when compiling C++.  */

/* GNU formerly extended the scanf functions with modified format
   specifiers %as, %aS, and %a[...] that allocate a buffer for the
   input using malloc.  This extension conflicts with ISO C99, which
   defines %a as a standalone format specifier that reads a floating-
   point number; moreover, POSIX.1-2008 provides the same feature
   using the modifier letter 'm' instead (%ms, %mS, %m[...]).

   We now follow C99 unless GNU extensions are active and the compiler
   is specifically in C89 or C++98 mode (strict or not).  For
   instance, with GCC, -std=gnu11 will have C99-compliant scanf with
   or without -D_GNU_SOURCE, but -std=c89 -D_GNU_SOURCE will have the
   old extension.  */
/* ISO C2X added support for a 0b or 0B prefix on binary constants as
   inputs to strtol-family functions (base 0 or 2).  This macro is
   used to condition redirection in headers to allow that redirection
   to be disabled when building those functions, despite _GNU_SOURCE
   being defined.  */

/* Get definitions of __STDC_* predefined macros, if the compiler has
   not preincluded this header automatically.  */
/* Copyright (C) 1991-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* This macro indicates that the installed library is the GNU C Library.
   For historic reasons the value now is 6 and this will stay from now
   on.  The use of this variable is deprecated.  Use __GLIBC__ and
   __GLIBC_MINOR__ now (see below) when you want to test for a specific
   GNU C library version and use the values in <gnu/lib-names.h> to get
   the sonames of the shared libraries.  */

/* Major and minor version number of the GNU C library package.  Use
   these macros to test for features in specific releases.  */

/* This is here only because every header file already includes this one.  */

/* Copyright (C) 1992-2023 Free Software Foundation, Inc.
   Copyright The GNU Toolchain Authors.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* We are almost always included from features.h. */

/* The GNU libc does not support any K&R compilers or the traditional mode
   of ISO C compilers anymore.  Check for some of the combinations not
   supported anymore.  */

/* Some user header file might have defined this before.  */

/* Compilers that lack __has_attribute may object to
       #if defined __has_attribute && __has_attribute (...)
   even though they do not need to evaluate the right-hand side of the &&.
   Similarly for __has_builtin, etc.  */
/* All functions, except those with callbacks or those that
   synchronize memory, are leaf functions.  */
/* GCC can always grok prototypes.  For C++ programs we add throw()
   to help it optimize the function calls.  But this only works with
   gcc 2.8.x and egcs.  For gcc 3.4 and up we even mark C functions
   as non-throwing using a function attribute since programs can use
   the -fexceptions options for C code as well.  */
/* These two macros are not used in glibc anymore.  They are kept here
   only because some other projects expect the macros to be defined.  */

/* For these things, GCC behaves the ANSI way normally,
   and the non-ANSI way under -traditional.  */

/* This is not a typedef so `const __ptr_t' does the right thing.  */

/* C++ needs to know that types and declarations are C, not C++.  */
/* Fortify support.  */

/* Use __builtin_dynamic_object_size at _FORTIFY_SOURCE=3 when available.  */
/* Compile time conditions to choose between the regular, _chk and _chk_warn
   variants.  These conditions should get evaluated to constant and optimized
   away.  */

/* Length is known to be safe at compile time if the __L * __S <= __OBJSZ
   condition can be folded to a constant and if it is true, or unknown (-1) */

/* Conversely, we know at compile time that the length is unsafe if the
   __L * __S <= __OBJSZ condition can be folded to a constant and if it is
   false.  */

/* Fortify function f.  __f_alias, __f_chk and __f_chk_warn must be
   declared.  */
/* Fortify function f, where object size argument passed to f is the number of
   elements and not total size.  */
/* Support for flexible arrays.
   Headers that should use flexible arrays only if they're "real"
   (e.g. only if they won't affect sizeof()) should test
   #if __glibc_c99_flexarr_available.  */
/* __asm__ ("xyz") is used throughout the headers to rename functions
   at the assembly language level.  This is wrapped by the __REDIRECT
   macro, in order to support compilers that can do this some other
   way.  When compilers don't support asm-names at all, we have to do
   preprocessor tricks instead (which don't have exactly the right
   semantics, but it's the best we can do).

   Example:
   int __REDIRECT(setpgrp, (__pid_t pid, __pid_t pgrp), setpgid); */
/*
#elif __SOME_OTHER_COMPILER__

# define __REDIRECT(name, proto, alias) name proto; 	_Pragma("let " #name " = " #alias)
)
*/

/* GCC and clang have various useful declarations that can be made with
   the '__attribute__' syntax.  All of the ways we use this do fine if
   they are omitted for compilers that don't understand it.  */

/* At some point during the gcc 2.96 development the `malloc' attribute
   for functions was introduced.  We don't want to use it unconditionally
   (although this would be possible) since it generates warnings.  */

/* Tell the compiler which arguments to an allocation function
   indicate the size of the allocation.  */

/* Tell the compiler which argument to an allocation function
   indicates the alignment of the allocation.  */

/* At some point during the gcc 2.96 development the `pure' attribute
   for functions was introduced.  We don't want to use it unconditionally
   (although this would be possible) since it generates warnings.  */

/* This declaration tells the compiler that the value is constant.  */
/* At some point during the gcc 3.1 development the `used' attribute
   for functions was introduced.  We don't want to use it unconditionally
   (although this would be possible) since it generates warnings.  */
/* Since version 3.2, gcc allows marking deprecated functions.  */

/* Since version 4.5, gcc also allows one to specify the message printed
   when a deprecated function is used.  clang claims to be gcc 4.2, but
   may also support this feature.  */
/* At some point during the gcc 2.8 development the `format_arg' attribute
   for functions was introduced.  We don't want to use it unconditionally
   (although this would be possible) since it generates warnings.
   If several `format_arg' attributes are given for the same function, in
   gcc-3.0 and older, all but the last one are ignored.  In newer gccs,
   all designated arguments are considered.  */

/* At some point during the gcc 2.97 development the `strfmon' format
   attribute for functions was introduced.  We don't want to use it
   unconditionally (although this would be possible) since it
   generates warnings.  */

/* The nonnull function attribute marks pointer parameters that
   must not be NULL.  This has the name __nonnull in glibc,
   and __attribute_nonnull__ in files shared with Gnulib to avoid
   collision with a different __nonnull in DragonFlyBSD 5.9.  */
/* The returns_nonnull function attribute marks the return type of the function
   as always being non-null.  */
/* If fortification mode, we warn about unused results of certain
   function calls which can lead to problems.  */
/* Forces a function to be always inlined.  */

/* The Linux kernel defines __always_inline in stddef.h (283d7573), and
   it conflicts with this definition.  Therefore undefine it first to
   allow either header to be included first.  */

/* Associate error messages with the source location of the call site rather
   than with the source location inside the function.  */

/* GCC 4.3 and above with -std=c99 or -std=gnu99 implements ISO C99
   inline semantics, unless -fgnu89-inline is used.  Using __GNUC_STDC_INLINE__
   or __GNUC_GNU_INLINE is not a good enough check for gcc because gcc versions
   older than 4.3 may define these macros and still not guarantee GNU inlining
   semantics.

   clang++ identifies itself as gcc-4.2, but has support for GNU inlining
   semantics, that can be checked for by using the __GNUC_STDC_INLINE_ and
   __GNUC_GNU_INLINE__ macro definitions.  */
/* GCC 4.3 and above allow passing all anonymous arguments of an
   __extern_always_inline function to some other vararg function.  */

/* It is possible to compile containing GCC extensions even if GCC is
   run in pedantic mode if the uses are carefully marked using the
   `__extension__' keyword.  But this is not generally available before
   version 2.8.  */

/* __restrict is known in EGCS 1.2 and above, and in clang.
   It works also in C++ mode (outside of arrays), but only when spelled
   as '__restrict', not 'restrict'.  */
/* ISO C99 also allows to declare arrays as non-overlapping.  The syntax is
     array_name[restrict]
   GCC 3.1 and clang support this.
   This syntax is not usable in C++ mode.  */
/* Describes a char array whose address can safely be passed as the first
   argument to strncpy and strncat, as the char array is not necessarily
   a NUL-terminated string.  */

/* Undefine (also defined in libc-symbols.h).  */

/* Copies attributes from the declaration or type referenced by
   the argument.  */
/* Gnulib avoids including these, as they don't work on non-glibc or
   older glibc platforms.  */

/* Determine the wordsize from the preprocessor defines.  */
/* Both x86-64 and x32 use the 64-bit system call interface.  */
/* Properties of long double type.  ldbl-96 version.
   Copyright (C) 2016-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License  published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* long double is distinct from double, so there is nothing to
   define here.  */
/* __glibc_macro_warning (MESSAGE) issues warning MESSAGE.  This is
   intended for use in preprocessor macros.

   Note: MESSAGE must be a _single_ string; concatenation of string
   literals is not supported.  */
/* Generic selection (ISO C11) is a C-only feature, available in GCC
   since version 4.9.  Previous versions do not provide generic
   selection, even though they might set __STDC_VERSION__ to 201112L,
   when in -std=c11 mode.  Thus, we must check for !defined __GNUC__
   when testing __STDC_VERSION__ for generic selection support.
   On the other hand, Clang also defines __GNUC__, so a clang-specific
   check is required to enable the use of generic selection.  */
/* Designates a 1-based positional argument ref-index of pointer type
   that can be used to access size-index elements of the pointed-to
   array according to access mode, or at least one element when
   size-index is not provided:
     access (access-mode, <ref-index> [, <size-index>])  */

/* For _FORTIFY_SOURCE == 3 we use __builtin_dynamic_object_size, which may
   use the access attribute to get object sizes from function definition
   arguments, so we can't use them on functions we fortify.  Drop the object
   size hints for such functions.  */
/* Designates dealloc as a function to call to deallocate objects
   allocated by the declared function.  */
/* Specify that a function such as setjmp or vfork may return
   twice.  */

/* If we don't have __REDIRECT, prototypes will be missing if
   __USE_FILE_OFFSET64 but not __USE_LARGEFILE[64]. */

/* Decide whether we can define 'extern inline' functions in headers.  */

/* This is here only because every header file already includes this one.
   Get the definitions of all the appropriate `__stub_FUNCTION' symbols.
   <gnu/stubs.h> contains `#define __stub_FUNCTION' when FUNCTION is a stub
   that will always return failure (and set errno to ENOSYS).  */
/* This file is automatically generated.
   This file selects the right generated file of `__stub_FUNCTION' macros
   based on the architecture being compiled for.  */

/* This file is automatically generated.
   It defines a symbol `__stub_FUNCTION' for each function
   in the C library which is a stub, meaning it will fail
   every time called, usually setting errno to ENOSYS.  */

/* void assert (int expression);

   If NDEBUG is defined, do nothing.
   If not, and EXPRESSION is zero, print an error message and abort.  */

/* This prints an "Assertion failed" message and aborts.  */
extern void __assert_fail(const char *__assertion, const char *__file, unsigned int __line, const char *__function) __attribute__((__nothrow__, __leaf__))
__attribute__((__noreturn__));

/* Likewise, but prints the error text for ERRNUM.  */
extern void __assert_perror_fail(int __errnum, const char *__file, unsigned int __line, const char *__function) __attribute__((__nothrow__, __leaf__))
__attribute__((__noreturn__));

/* The following is not at all used here but needed for standard
   compliance.  */
extern void __assert(const char *__assertion, const char *__file, int __line) __attribute__((__nothrow__, __leaf__)) __attribute__((__noreturn__));

/* When possible, define assert so that it does not add extra
   parentheses around EXPR.  Otherwise, those added parentheses would
   suppress warnings we'd expect to be detected by gcc's -Wparentheses.  */
/* Version 2.4 and later of GCC define a magical variable `__PRETTY_FUNCTION__'
   which contains the name of the function currently being defined.
   This is broken in G++ before version 2.6.
   C9x has a similar variable called __func__, but prefer the GCC one since
   it demangles C++ function names.  */
/* Copyright (C) 1991-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/*
 *	ISO C99 Standard: 7.5 Errors	<errno.h>
 */

/* Copyright (C) 1991-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* The system-specific definitions of the E* constants, as macros.  */
/* Error constants.  Linux specific version.
   Copyright (C) 1996-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */
/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */

/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */

/*
 * This error code is special: arch syscall entry code will return
 * -ENOSYS if users try to call a syscall that doesn't exist.  To keep
 * failures of syscalls that really do exist distinguishable from
 * failures due to attempts to use a nonexistent syscall, syscall
 * implementations should refrain from returning -ENOSYS.
 */
/* for robust mutexes */

/* Older Linux headers do not define these constants.  */

/* When included from assembly language, this header only provides the
   E* constants.  */

/* The error code set by various library functions.  */
extern int *__errno_location(void) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* The full and simple forms of the name with which the program was
   invoked.  These variables are set up automatically at startup based on
   the value of argv[0].  */
extern char *program_invocation_name;
extern char *program_invocation_short_name;

/* Define error_t.
   Copyright (C) 1991-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

typedef int error_t;

/* Declarations for math functions.
   Copyright (C) 1991-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/*
 *	ISO C99 Standard: 7.12 Mathematics	<math.h>
 */

/* Handle feature test macros at the start of a header.
   Copyright (C) 2016-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* This header is internal to glibc and should not be included outside
   of glibc headers.  Headers including it must define
   __GLIBC_INTERNAL_STARTING_HEADER_IMPLEMENTATION first.  This header
   cannot have multiple include guards because ISO C feature test
   macros depend on the definition of the macro when an affected
   header is included, not when the first system header is
   included.  */

/* Copyright (C) 1991-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* ISO/IEC TR 24731-2:2010 defines the __STDC_WANT_LIB_EXT2__
   macro.  */
/* ISO/IEC TS 18661-1:2014 defines the __STDC_WANT_IEC_60559_BFP_EXT__
   macro.  Most but not all symbols enabled by that macro in TS
   18661-1 are enabled unconditionally in C2X.  In C2X, the symbols in
   Annex F still require a new feature test macro
   __STDC_WANT_IEC_60559_EXT__ instead (C2X does not define
   __STDC_WANT_IEC_60559_BFP_EXT__), while a few features from TS
   18661-1 are not included in C2X (and thus should depend on
   __STDC_WANT_IEC_60559_BFP_EXT__ even when C2X features are
   enabled).

   __GLIBC_USE (IEC_60559_BFP_EXT) controls those features from TS
   18661-1 not included in C2X.

   __GLIBC_USE (IEC_60559_BFP_EXT_C2X) controls those features from TS
   18661-1 that are also included in C2X (with no feature test macro
   required in C2X).

   __GLIBC_USE (IEC_60559_EXT) controls those features from TS 18661-1
   that are included in C2X but conditional on
   __STDC_WANT_IEC_60559_EXT__.  (There are currently no features
   conditional on __STDC_WANT_IEC_60559_EXT__ that are not in TS
   18661-1.)  */
/* ISO/IEC TS 18661-4:2015 defines the
   __STDC_WANT_IEC_60559_FUNCS_EXT__ macro.  Other than the reduction
   functions, the symbols from this TS are enabled unconditionally in
   C2X.  */
/* ISO/IEC TS 18661-3:2015 defines the
   __STDC_WANT_IEC_60559_TYPES_EXT__ macro.  */

/* Get definitions of __intmax_t and __uintmax_t.  */
/* bits/types.h -- definitions of __*_t types underlying *_t types.
   Copyright (C) 2002-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/*
 * Never include this file directly; use <sys/types.h> instead.
 */

/* Copyright (C) 1991-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */
/* Determine the wordsize from the preprocessor defines.  */
/* Both x86-64 and x32 use the 64-bit system call interface.  */
/* Bit size of the time_t type at glibc build time, x86-64 and x32 case.
   Copyright (C) 2018-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* Determine the wordsize from the preprocessor defines.  */
/* Both x86-64 and x32 use the 64-bit system call interface.  */

/* For others, time size is word size.  */

/* Convenience types.  */
typedef unsigned char __u_char;
typedef unsigned short int __u_short;
typedef unsigned int __u_int;
typedef unsigned long int __u_long;

/* Fixed-size types, underlying types depend on word size and compiler.  */
typedef signed char __int8_t;
typedef unsigned char __uint8_t;
typedef signed short int __int16_t;
typedef unsigned short int __uint16_t;
typedef signed int __int32_t;
typedef unsigned int __uint32_t;

typedef signed long int __int64_t;
typedef unsigned long int __uint64_t;

/* Smallest types with at least a given width.  */
typedef __int8_t __int_least8_t;
typedef __uint8_t __uint_least8_t;
typedef __int16_t __int_least16_t;
typedef __uint16_t __uint_least16_t;
typedef __int32_t __int_least32_t;
typedef __uint32_t __uint_least32_t;
typedef __int64_t __int_least64_t;
typedef __uint64_t __uint_least64_t;

/* quad_t is also 64 bits.  */

typedef long int __quad_t;
typedef unsigned long int __u_quad_t;

/* Largest integral types.  */

typedef long int __intmax_t;
typedef unsigned long int __uintmax_t;

/* The machine-dependent file <bits/typesizes.h> defines __*_T_TYPE
   macros for each of the OS types we define below.  The definitions
   of those macros must use the following macros for underlying types.
   We define __S<SIZE>_TYPE and __U<SIZE>_TYPE for the signed and unsigned
   variants of each of the following integer types on this machine.

    16		-- "natural" 16-bit type (always short)
    32		-- "natural" 32-bit type (always int)
    64		-- "natural" 64-bit type (long or long long)
    LONG32		-- 32-bit type, traditionally long
    QUAD		-- 64-bit type, traditionally long long
    WORD		-- natural type of __WORDSIZE bits (int or long)
    LONGWORD	-- type of __WORDSIZE bits, traditionally long

   We distinguish WORD/LONGWORD, 32/LONG32, and 64/QUAD so that the
   conventional uses of `long' or `long long' type modifiers match the
   types we define, even when a less-adorned type would be the same size.
   This matters for (somewhat) portably writing printf/scanf formats for
   these types, where using the appropriate l or ll format modifiers can
   make the typedefs and the formats match up across all GNU platforms.  If
   we used `long' when it's 64 bits where `long long' is expected, then the
   compiler would warn about the formats not matching the argument types,
   and the programmer changing them to shut up the compiler would break the
   program's portability.

   Here we assume what is presently the case in all the GCC configurations
   we support: long long is always 64 bits, long is always word/address size,
   and int is always 32 bits.  */
/* No need to mark the typedef with __extension__.   */

/* bits/typesizes.h -- underlying types for *_t.  Linux/x86-64 version.
   Copyright (C) 2012-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */
/* See <bits/types.h> for the meaning of these macros.  This file exists so
   that <bits/types.h> need not vary across different GNU platforms.  */

/* X32 kernel interface is 64-bit.  */
/* Tell the libc code that off_t and off64_t are actually the same type
   for all ABI purposes, even if possibly expressed as different base types
   for C type-checking purposes.  */

/* Same for ino_t and ino64_t.  */

/* And for __rlim_t and __rlim64_t.  */

/* And for fsblkcnt_t, fsblkcnt64_t, fsfilcnt_t and fsfilcnt64_t.  */

/* And for getitimer, setitimer and rusage  */
/* Number of descriptors that can fit in an `fd_set'.  */
/* bits/time64.h -- underlying types for __time64_t.  Generic version.
   Copyright (C) 2018-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */
/* Define __TIME64_T_TYPE so that it is always a 64-bit type.  */

/* If we already have 64-bit time type then use it.  */

typedef unsigned long int __dev_t;   /* Type of device numbers.  */
typedef unsigned int __uid_t;        /* Type of user identifications.  */
typedef unsigned int __gid_t;        /* Type of group identifications.  */
typedef unsigned long int __ino_t;   /* Type of file serial numbers.  */
typedef unsigned long int __ino64_t; /* Type of file serial numbers (LFS).*/
typedef unsigned int __mode_t;       /* Type of file attribute bitmasks.  */
typedef unsigned long int __nlink_t; /* Type of file link counts.  */
typedef long int __off_t;            /* Type of file sizes and offsets.  */
typedef long int __off64_t;          /* Type of file sizes and offsets (LFS).  */
typedef int __pid_t;                 /* Type of process identifications.  */
typedef struct {
    int __val[2];
} __fsid_t;                           /* Type of file system IDs.  */
typedef long int __clock_t;           /* Type of CPU usage counts.  */
typedef unsigned long int __rlim_t;   /* Type for resource measurement.  */
typedef unsigned long int __rlim64_t; /* Type for resource measurement (LFS).  */
typedef unsigned int __id_t;          /* General type for IDs.  */
typedef long int __time_t;            /* Seconds since the Epoch.  */
typedef unsigned int __useconds_t;    /* Count of microseconds.  */
typedef long int __suseconds_t;       /* Signed count of microseconds.  */
typedef long int __suseconds64_t;

typedef int __daddr_t; /* The type of a disk address.  */
typedef int __key_t;   /* Type of an IPC key.  */

/* Clock ID used in clock and timer functions.  */
typedef int __clockid_t;

/* Timer ID returned by `timer_create'.  */
typedef void *__timer_t;

/* Type to represent block size.  */
typedef long int __blksize_t;

/* Types from the Large File Support interface.  */

/* Type to count number of disk blocks.  */
typedef long int __blkcnt_t;
typedef long int __blkcnt64_t;

/* Type to count file system blocks.  */
typedef unsigned long int __fsblkcnt_t;
typedef unsigned long int __fsblkcnt64_t;

/* Type to count file system nodes.  */
typedef unsigned long int __fsfilcnt_t;
typedef unsigned long int __fsfilcnt64_t;

/* Type of miscellaneous file system fields.  */
typedef long int __fsword_t;

typedef long int __ssize_t; /* Type of a byte count, or error.  */

/* Signed long type used in system calls.  */
typedef long int __syscall_slong_t;
/* Unsigned long type used in system calls.  */
typedef unsigned long int __syscall_ulong_t;

/* These few don't really vary by system, they always correspond
   to one of the other defined types.  */
typedef __off64_t __loff_t; /* Type of file sizes and offsets (LFS).  */
typedef char *__caddr_t;

/* Duplicates info from stdint.h but this is used in unistd.h.  */
typedef long int __intptr_t;

/* Duplicate info from sys/socket.h.  */
typedef unsigned int __socklen_t;

/* C99: An integer type that can be accessed as an atomic entity,
   even in the presence of asynchronous interrupts.
   It is not currently necessary for this to be machine-specific.  */
typedef int __sig_atomic_t;

/* Seconds since the Epoch, visible to user code when time_t is too
   narrow only for consistency with the old way of widening too-narrow
   types.  User code should never use __time64_t.  */

/* Get machine-dependent vector math functions declarations.  */
/* Platform-specific SIMD declarations of math functions.
   Copyright (C) 2014-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* Get default empty definitions for simd declarations.  */
/* Empty definitions required for __MATHCALL_VEC unfolding in mathcalls.h.
   Copyright (C) 2014-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* Needed definitions could be generated with:
   for func in $(grep __MATHCALL_VEC math/bits/mathcalls.h |		 sed -r "s|__MATHCALL_VEC.?\(||; s|,.*||"); do
o
     echo "#define __DECL_SIMD_${func}";
     echo "#define __DECL_SIMD_${func}f";
     echo "#define __DECL_SIMD_${func}l";
   done
 */

/* Gather machine dependent type support.  */
/* Macros to control TS 18661-3 glibc features on x86.
   Copyright (C) 2017-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* Copyright (C) 1991-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* Defined to 1 if the current compiler invocation provides a
   floating-point type with the IEEE 754 binary128 format, and this
   glibc includes corresponding *f128 interfaces for it.  The required
   libgcc support was added some time after the basic compiler
   support, for x86_64 and x86.  */
/* Defined to 1 if __HAVE_FLOAT128 is 1 and the type is ABI-distinct
   from the default float, double and long double types in this glibc.  */

/* Defined to 1 if the current compiler invocation provides a
   floating-point type with the right format for _Float64x, and this
   glibc includes corresponding *f64x interfaces for it.  */

/* Defined to 1 if __HAVE_FLOAT64X is 1 and _Float64x has the format
   of long double.  Otherwise, if __HAVE_FLOAT64X is 1, _Float64x has
   the format of _Float128, which must be different from that of long
   double.  */

/* Defined to concatenate the literal suffix to be used with _Float128
   types, if __HAVE_FLOAT128 is 1. */
/* Defined to a complex binary128 type if __HAVE_FLOAT128 is 1.  */
/* The remaining of this file provides support for older compilers.  */

/* The type _Float128 exists only since GCC 7.0.  */

/* __builtin_huge_valf128 doesn't exist before GCC 7.0.  */

/* Older GCC has only a subset of built-in functions for _Float128 on
   x86, and __builtin_infq is not usable in static initializers.
   Converting a narrower sNaN to _Float128 produces a quiet NaN, so
   attempts to use _Float128 sNaNs will not work properly with older
   compilers.  */
/* In math/math.h, __MATH_TG will expand signbit to __builtin_signbit*,
   e.g.: __builtin_signbitf128, before GCC 6.  However, there has never
   been a __builtin_signbitf128 in GCC and the type-generic builtin is
   only available since GCC 6.  */
/* Macros to control TS 18661-3 glibc features where the same
   definitions are appropriate for all platforms.
   Copyright (C) 2017-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* Copyright (C) 1991-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */
/* Properties of long double type.  ldbl-96 version.
   Copyright (C) 2016-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License  published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* long double is distinct from double, so there is nothing to
   define here.  */

/* This header should be included at the bottom of each bits/floatn.h.
   It defines the following macros for each _FloatN and _FloatNx type,
   where the same definitions, or definitions based only on the macros
   in bits/floatn.h, are appropriate for all glibc configurations.  */

/* Defined to 1 if the current compiler invocation provides a
   floating-point type with the right format for this type, and this
   glibc includes corresponding *fN or *fNx interfaces for it.  */

/* Defined to 1 if the corresponding __HAVE_<type> macro is 1 and the
   type is the first with its format in the sequence of (the default
   choices for) float, double, long double, _Float16, _Float32,
   _Float64, _Float128, _Float32x, _Float64x, _Float128x for this
   glibc; that is, if functions present once per floating-point format
   rather than once per type are present for this type.

   All configurations supported by glibc have _Float32 the same format
   as float, _Float64 and _Float32x the same format as double, the
   _Float64x the same format as either long double or _Float128.  No
   configurations support _Float128x or, as of GCC 7, have compiler
   support for a type meeting the requirements for _Float128x.  */

/* Defined to 1 if the corresponding _FloatN type is not binary compatible
   with the corresponding ISO C type in the current compilation unit as
   opposed to __HAVE_DISTINCT_FLOATN, which indicates the default types built
   in glibc.  */

/* Defined to 1 if any _FloatN or _FloatNx types that are not
   ABI-distinct are however distinct types at the C language level (so
   for the purposes of __builtin_types_compatible_p and _Generic).  */
/* Defined to concatenate the literal suffix to be used with _FloatN
   or _FloatNx types, if __HAVE_<type> is 1.  The corresponding
   literal suffixes exist since GCC 7, for C only.  */
/* Defined to a complex type if __HAVE_<type> is 1.  */
/* The remaining of this file provides support for older compilers.  */
/* If double, long double and _Float64 all have the same set of
   values, TS 18661-3 requires the usual arithmetic conversions on
   long double and _Float64 to produce _Float64.  For this to be the
   case when building with a compiler without a distinct _Float64
   type, _Float64 must be a typedef for long double, not for
   double.  */

/* Value returned on overflow.  With IEEE 754 floating point, this is
   +Infinity, otherwise the largest representable positive value.  */
/* IEEE positive infinity.  */

/* IEEE Not A Number.  */
/* Signaling NaN macros, if supported.  */
/* Get __GLIBC_FLT_EVAL_METHOD.  */
/* Define __GLIBC_FLT_EVAL_METHOD.  x86 version.
   Copyright (C) 2016-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* Define the following typedefs.

    float_t	floating-point type at least as wide as `float' used
        to evaluate `float' expressions
    double_t	floating-point type at least as wide as `double' used
        to evaluate `double' expressions
*/

typedef float float_t;
typedef double double_t;
/* Define macros for the return values of ilogb and llogb, based on
   __FP_LOGB0_IS_MIN and __FP_LOGBNAN_IS_MIN.

    FP_ILOGB0	Expands to a value returned by `ilogb (0.0)'.
    FP_ILOGBNAN	Expands to a value returned by `ilogb (NAN)'.
    FP_LLOGB0	Expands to a value returned by `llogb (0.0)'.
    FP_LLOGBNAN	Expands to a value returned by `llogb (NAN)'.

*/

/* Define __FP_LOGB0_IS_MIN and __FP_LOGBNAN_IS_MIN.  x86 version.
   Copyright (C) 2016-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */
/* Get the architecture specific values describing the floating-point
   evaluation.  The following symbols will get defined:

    FP_FAST_FMA
    FP_FAST_FMAF
    FP_FAST_FMAL
        If defined it indicates that the `fma' function
        generally executes about as fast as a multiply and an add.
        This macro is defined only iff the `fma' function is
        implemented directly with a hardware multiply-add instructions.
*/

/* Define FP_FAST_* macros.
   Copyright (C) 2016-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* The GCC 4.6 compiler will define __FP_FAST_FMA{,F,L} if the fma{,f,l}
   builtins are supported.  */

/* Rounding direction macros for fromfp functions.  */
enum {
    FP_INT_UPWARD =

        0,
    FP_INT_DOWNWARD =

        1,
    FP_INT_TOWARDZERO =

        2,
    FP_INT_TONEARESTFROMZERO =

        3,
    FP_INT_TONEAREST =

        4,
};

/* The file <bits/mathcalls.h> contains the prototypes for all the
   actual math functions.  These macros are used for those prototypes,
   so we can easily declare each function as both `name' and `__name',
   and can declare the float versions `namef' and `__namef'.  */
/* Ignore the alias by default.  The alias is only useful with
   redirections.  */
/* Prototype declarations for math classification macros helpers.
   Copyright (C) 2017-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* Classify given number.  */
extern int __fpclassify(double __value) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Test for negative number.  */
extern int __signbit(double __value) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return 0 if VALUE is finite or NaN, +1 if it
   is +Infinity, -1 if it is -Infinity.  */
extern int __isinf(double __value) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return nonzero if VALUE is finite and not NaN.  Used by isfinite macro.  */
extern int __finite(double __value) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return nonzero if VALUE is not a number.  */
extern int __isnan(double __value) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Test equality.  */
extern int __iseqsig(double __x, double __y) __attribute__((__nothrow__, __leaf__));

/* Test for signaling NaN.  */
extern int __issignaling(double __value) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
/* Prototype declarations for math functions; helper file for <math.h>.
   Copyright (C) 1996-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* NOTE: Because of the special way this file is used by <math.h>, this
   file must NOT be protected from multiple inclusion as header files
   usually are.

   This file provides prototype declarations for the math functions.
   Most functions are declared using the macro:

   __MATHCALL (NAME,[_r], (ARGS...));

   This means there is a function `NAME' returning `double' and a function
   `NAMEf' returning `float'.  Each place `_Mdouble_' appears in the
   prototype, that is actually `double' in the prototype for `NAME' and
   `float' in the prototype for `NAMEf'.  Reentrant variant functions are
   called `NAME_r' and `NAMEf_r'.

   Functions returning other types like `int' are declared using the macro:

   __MATHDECL (TYPE, NAME,[_r], (ARGS...));

   This is just like __MATHCALL but for a function returning `TYPE'
   instead of `_Mdouble_'.  In all of these cases, there is still
   both a `NAME' and a `NAMEf' that takes `float' arguments.

   Note that there must be no whitespace before the argument passed for
   NAME, to make token pasting work with -traditional.  */

/* Trigonometric functions.  */

/* Arc cosine of X.  */
extern double acos(double __x) __attribute__((__nothrow__, __leaf__));
extern double __acos(double __x) __attribute__((__nothrow__, __leaf__));
/* Arc sine of X.  */
extern double asin(double __x) __attribute__((__nothrow__, __leaf__));
extern double __asin(double __x) __attribute__((__nothrow__, __leaf__));
/* Arc tangent of X.  */
extern double atan(double __x) __attribute__((__nothrow__, __leaf__));
extern double __atan(double __x) __attribute__((__nothrow__, __leaf__));
/* Arc tangent of Y/X.  */
extern double atan2(double __y, double __x) __attribute__((__nothrow__, __leaf__));
extern double __atan2(double __y, double __x) __attribute__((__nothrow__, __leaf__));

/* Cosine of X.  */
extern double cos(double __x) __attribute__((__nothrow__, __leaf__));
extern double __cos(double __x) __attribute__((__nothrow__, __leaf__));
/* Sine of X.  */
extern double sin(double __x) __attribute__((__nothrow__, __leaf__));
extern double __sin(double __x) __attribute__((__nothrow__, __leaf__));
/* Tangent of X.  */
extern double tan(double __x) __attribute__((__nothrow__, __leaf__));
extern double __tan(double __x) __attribute__((__nothrow__, __leaf__));

/* Hyperbolic functions.  */

/* Hyperbolic cosine of X.  */
extern double cosh(double __x) __attribute__((__nothrow__, __leaf__));
extern double __cosh(double __x) __attribute__((__nothrow__, __leaf__));
/* Hyperbolic sine of X.  */
extern double sinh(double __x) __attribute__((__nothrow__, __leaf__));
extern double __sinh(double __x) __attribute__((__nothrow__, __leaf__));
/* Hyperbolic tangent of X.  */
extern double tanh(double __x) __attribute__((__nothrow__, __leaf__));
extern double __tanh(double __x) __attribute__((__nothrow__, __leaf__));

/* Cosine and sine of X.  */
extern void sincos(double __x, double *__sinx, double *__cosx) __attribute__((__nothrow__, __leaf__));
extern void __sincos(double __x, double *__sinx, double *__cosx) __attribute__((__nothrow__, __leaf__));

/* Hyperbolic arc cosine of X.  */
extern double acosh(double __x) __attribute__((__nothrow__, __leaf__));
extern double __acosh(double __x) __attribute__((__nothrow__, __leaf__));
/* Hyperbolic arc sine of X.  */
extern double asinh(double __x) __attribute__((__nothrow__, __leaf__));
extern double __asinh(double __x) __attribute__((__nothrow__, __leaf__));
/* Hyperbolic arc tangent of X.  */
extern double atanh(double __x) __attribute__((__nothrow__, __leaf__));
extern double __atanh(double __x) __attribute__((__nothrow__, __leaf__));

/* Exponential and logarithmic functions.  */

/* Exponential function of X.  */
extern double exp(double __x) __attribute__((__nothrow__, __leaf__));
extern double __exp(double __x) __attribute__((__nothrow__, __leaf__));

/* Break VALUE into a normalized fraction and an integral power of 2.  */
extern double frexp(double __x, int *__exponent) __attribute__((__nothrow__, __leaf__));
extern double __frexp(double __x, int *__exponent) __attribute__((__nothrow__, __leaf__));

/* X times (two to the EXP power).  */
extern double ldexp(double __x, int __exponent) __attribute__((__nothrow__, __leaf__));
extern double __ldexp(double __x, int __exponent) __attribute__((__nothrow__, __leaf__));

/* Natural logarithm of X.  */
extern double log(double __x) __attribute__((__nothrow__, __leaf__));
extern double __log(double __x) __attribute__((__nothrow__, __leaf__));

/* Base-ten logarithm of X.  */
extern double log10(double __x) __attribute__((__nothrow__, __leaf__));
extern double __log10(double __x) __attribute__((__nothrow__, __leaf__));

/* Break VALUE into integral and fractional parts.  */
extern double modf(double __x, double *__iptr) __attribute__((__nothrow__, __leaf__));
extern double __modf(double __x, double *__iptr) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(2)));

/* Compute exponent to base ten.  */
extern double exp10(double __x) __attribute__((__nothrow__, __leaf__));
extern double __exp10(double __x) __attribute__((__nothrow__, __leaf__));

/* Return exp(X) - 1.  */
extern double expm1(double __x) __attribute__((__nothrow__, __leaf__));
extern double __expm1(double __x) __attribute__((__nothrow__, __leaf__));

/* Return log(1 + X).  */
extern double log1p(double __x) __attribute__((__nothrow__, __leaf__));
extern double __log1p(double __x) __attribute__((__nothrow__, __leaf__));

/* Return the base 2 signed integral exponent of X.  */
extern double logb(double __x) __attribute__((__nothrow__, __leaf__));
extern double __logb(double __x) __attribute__((__nothrow__, __leaf__));

/* Compute base-2 exponential of X.  */
extern double exp2(double __x) __attribute__((__nothrow__, __leaf__));
extern double __exp2(double __x) __attribute__((__nothrow__, __leaf__));

/* Compute base-2 logarithm of X.  */
extern double log2(double __x) __attribute__((__nothrow__, __leaf__));
extern double __log2(double __x) __attribute__((__nothrow__, __leaf__));

/* Power functions.  */

/* Return X to the Y power.  */
extern double pow(double __x, double __y) __attribute__((__nothrow__, __leaf__));
extern double __pow(double __x, double __y) __attribute__((__nothrow__, __leaf__));

/* Return the square root of X.  */
extern double sqrt(double __x) __attribute__((__nothrow__, __leaf__));
extern double __sqrt(double __x) __attribute__((__nothrow__, __leaf__));

/* Return `sqrt(X*X + Y*Y)'.  */
extern double hypot(double __x, double __y) __attribute__((__nothrow__, __leaf__));
extern double __hypot(double __x, double __y) __attribute__((__nothrow__, __leaf__));

/* Return the cube root of X.  */
extern double cbrt(double __x) __attribute__((__nothrow__, __leaf__));
extern double __cbrt(double __x) __attribute__((__nothrow__, __leaf__));

/* Nearest integer, absolute value, and remainder functions.  */

/* Smallest integral value not less than X.  */
extern double ceil(double __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern double __ceil(double __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Absolute value of X.  */
extern double fabs(double __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern double __fabs(double __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Largest integer not greater than X.  */
extern double floor(double __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern double __floor(double __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Floating-point modulo remainder of X/Y.  */
extern double fmod(double __x, double __y) __attribute__((__nothrow__, __leaf__));
extern double __fmod(double __x, double __y) __attribute__((__nothrow__, __leaf__));

/* Return 0 if VALUE is finite or NaN, +1 if it
   is +Infinity, -1 if it is -Infinity.  */
extern int isinf(double __value) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return nonzero if VALUE is finite and not NaN.  */
extern int finite(double __value) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return the remainder of X/Y.  */
extern double drem(double __x, double __y) __attribute__((__nothrow__, __leaf__));
extern double __drem(double __x, double __y) __attribute__((__nothrow__, __leaf__));

/* Return the fractional part of X after dividing out `ilogb (X)'.  */
extern double significand(double __x) __attribute__((__nothrow__, __leaf__));
extern double __significand(double __x) __attribute__((__nothrow__, __leaf__));

/* Return X with its signed changed to Y's.  */
extern double copysign(double __x, double __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern double __copysign(double __x, double __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return representation of qNaN for double type.  */
extern double nan(const char *__tagb) __attribute__((__nothrow__, __leaf__));
extern double __nan(const char *__tagb) __attribute__((__nothrow__, __leaf__));
/* Return nonzero if VALUE is not a number.  */
extern int isnan(double __value) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Bessel functions.  */
extern double j0(double) __attribute__((__nothrow__, __leaf__));
extern double __j0(double) __attribute__((__nothrow__, __leaf__));
extern double j1(double) __attribute__((__nothrow__, __leaf__));
extern double __j1(double) __attribute__((__nothrow__, __leaf__));
extern double jn(int, double) __attribute__((__nothrow__, __leaf__));
extern double __jn(int, double) __attribute__((__nothrow__, __leaf__));
extern double y0(double) __attribute__((__nothrow__, __leaf__));
extern double __y0(double) __attribute__((__nothrow__, __leaf__));
extern double y1(double) __attribute__((__nothrow__, __leaf__));
extern double __y1(double) __attribute__((__nothrow__, __leaf__));
extern double yn(int, double) __attribute__((__nothrow__, __leaf__));
extern double __yn(int, double) __attribute__((__nothrow__, __leaf__));

/* Error and gamma functions.  */
extern double erf(double) __attribute__((__nothrow__, __leaf__));
extern double __erf(double) __attribute__((__nothrow__, __leaf__));
extern double erfc(double) __attribute__((__nothrow__, __leaf__));
extern double __erfc(double) __attribute__((__nothrow__, __leaf__));
extern double lgamma(double) __attribute__((__nothrow__, __leaf__));
extern double __lgamma(double) __attribute__((__nothrow__, __leaf__));

/* True gamma function.  */
extern double tgamma(double) __attribute__((__nothrow__, __leaf__));
extern double __tgamma(double) __attribute__((__nothrow__, __leaf__));

/* Obsolete alias for `lgamma'.  */
extern double gamma(double) __attribute__((__nothrow__, __leaf__));
extern double __gamma(double) __attribute__((__nothrow__, __leaf__));

/* Reentrant version of lgamma.  This function uses the global variable
   `signgam'.  The reentrant version instead takes a pointer and stores
   the value through it.  */
extern double lgamma_r(double, int *__signgamp) __attribute__((__nothrow__, __leaf__));
extern double __lgamma_r(double, int *__signgamp) __attribute__((__nothrow__, __leaf__));

/* Return the integer nearest X in the direction of the
   prevailing rounding mode.  */
extern double rint(double __x) __attribute__((__nothrow__, __leaf__));
extern double __rint(double __x) __attribute__((__nothrow__, __leaf__));

/* Return X + epsilon if X < Y, X - epsilon if X > Y.  */
extern double nextafter(double __x, double __y) __attribute__((__nothrow__, __leaf__));
extern double __nextafter(double __x, double __y) __attribute__((__nothrow__, __leaf__));

extern double nexttoward(double __x, long double __y) __attribute__((__nothrow__, __leaf__));
extern double __nexttoward(double __x, long double __y) __attribute__((__nothrow__, __leaf__));

/* Return X - epsilon.  */
extern double nextdown(double __x) __attribute__((__nothrow__, __leaf__));
extern double __nextdown(double __x) __attribute__((__nothrow__, __leaf__));
/* Return X + epsilon.  */
extern double nextup(double __x) __attribute__((__nothrow__, __leaf__));
extern double __nextup(double __x) __attribute__((__nothrow__, __leaf__));

/* Return the remainder of integer division X / Y with infinite precision.  */
extern double remainder(double __x, double __y) __attribute__((__nothrow__, __leaf__));
extern double __remainder(double __x, double __y) __attribute__((__nothrow__, __leaf__));

/* Return X times (2 to the Nth power).  */
extern double scalbn(double __x, int __n) __attribute__((__nothrow__, __leaf__));
extern double __scalbn(double __x, int __n) __attribute__((__nothrow__, __leaf__));

/* Return the binary exponent of X, which must be nonzero.  */
extern int ilogb(double __x) __attribute__((__nothrow__, __leaf__));
extern int __ilogb(double __x) __attribute__((__nothrow__, __leaf__));

/* Like ilogb, but returning long int.  */
extern long int llogb(double __x) __attribute__((__nothrow__, __leaf__));
extern long int __llogb(double __x) __attribute__((__nothrow__, __leaf__));

/* Return X times (2 to the Nth power).  */
extern double scalbln(double __x, long int __n) __attribute__((__nothrow__, __leaf__));
extern double __scalbln(double __x, long int __n) __attribute__((__nothrow__, __leaf__));

/* Round X to integral value in floating-point format using current
   rounding direction, but do not raise inexact exception.  */
extern double nearbyint(double __x) __attribute__((__nothrow__, __leaf__));
extern double __nearbyint(double __x) __attribute__((__nothrow__, __leaf__));

/* Round X to nearest integral value, rounding halfway cases away from
   zero.  */
extern double round(double __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern double __round(double __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Round X to the integral value in floating-point format nearest but
   not larger in magnitude.  */
extern double trunc(double __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern double __trunc(double __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Compute remainder of X and Y and put in *QUO a value with sign of x/y
   and magnitude congruent `mod 2^n' to the magnitude of the integral
   quotient x/y, with n >= 3.  */
extern double remquo(double __x, double __y, int *__quo) __attribute__((__nothrow__, __leaf__));
extern double __remquo(double __x, double __y, int *__quo) __attribute__((__nothrow__, __leaf__));

/* Conversion functions.  */

/* Round X to nearest integral value according to current rounding
   direction.  */
extern long int lrint(double __x) __attribute__((__nothrow__, __leaf__));
extern long int __lrint(double __x) __attribute__((__nothrow__, __leaf__));
__extension__ extern long long int llrint(double __x) __attribute__((__nothrow__, __leaf__));
extern long long int __llrint(double __x) __attribute__((__nothrow__, __leaf__));

/* Round X to nearest integral value, rounding halfway cases away from
   zero.  */
extern long int lround(double __x) __attribute__((__nothrow__, __leaf__));
extern long int __lround(double __x) __attribute__((__nothrow__, __leaf__));
__extension__ extern long long int llround(double __x) __attribute__((__nothrow__, __leaf__));
extern long long int __llround(double __x) __attribute__((__nothrow__, __leaf__));

/* Return positive difference between X and Y.  */
extern double fdim(double __x, double __y) __attribute__((__nothrow__, __leaf__));
extern double __fdim(double __x, double __y) __attribute__((__nothrow__, __leaf__));

/* Return maximum numeric value from X and Y.  */
extern double fmax(double __x, double __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern double __fmax(double __x, double __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return minimum numeric value from X and Y.  */
extern double fmin(double __x, double __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern double __fmin(double __x, double __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Multiply-add function computed as a ternary operation.  */
extern double fma(double __x, double __y, double __z) __attribute__((__nothrow__, __leaf__));
extern double __fma(double __x, double __y, double __z) __attribute__((__nothrow__, __leaf__));

/* Round X to nearest integer value, rounding halfway cases to even.  */
extern double roundeven(double __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern double __roundeven(double __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Round X to nearest signed integer value, not raising inexact, with
   control of rounding direction and width of result.  */
extern __intmax_t fromfp(double __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));
extern __intmax_t __fromfp(double __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));

/* Round X to nearest unsigned integer value, not raising inexact,
   with control of rounding direction and width of result.  */
extern __uintmax_t ufromfp(double __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));
extern __uintmax_t __ufromfp(double __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));

/* Round X to nearest signed integer value, raising inexact for
   non-integers, with control of rounding direction and width of
   result.  */
extern __intmax_t fromfpx(double __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));
extern __intmax_t __fromfpx(double __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));

/* Round X to nearest unsigned integer value, raising inexact for
   non-integers, with control of rounding direction and width of
   result.  */
extern __uintmax_t ufromfpx(double __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));
extern __uintmax_t __ufromfpx(double __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));

/* Canonicalize floating-point representation.  */
extern int canonicalize(double *__cx, const double *__x) __attribute__((__nothrow__, __leaf__));

/* Return value with maximum magnitude.  */
extern double fmaxmag(double __x, double __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern double __fmaxmag(double __x, double __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return value with minimum magnitude.  */
extern double fminmag(double __x, double __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern double __fminmag(double __x, double __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return maximum value from X and Y.  */
extern double fmaximum(double __x, double __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern double __fmaximum(double __x, double __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return minimum value from X and Y.  */
extern double fminimum(double __x, double __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern double __fminimum(double __x, double __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return maximum numeric value from X and Y.  */
extern double fmaximum_num(double __x, double __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern double __fmaximum_num(double __x, double __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return minimum numeric value from X and Y.  */
extern double fminimum_num(double __x, double __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern double __fminimum_num(double __x, double __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return value with maximum magnitude.  */
extern double fmaximum_mag(double __x, double __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern double __fmaximum_mag(double __x, double __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return value with minimum magnitude.  */
extern double fminimum_mag(double __x, double __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern double __fminimum_mag(double __x, double __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return numeric value with maximum magnitude.  */
extern double fmaximum_mag_num(double __x, double __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern double __fmaximum_mag_num(double __x, double __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return numeric value with minimum magnitude.  */
extern double fminimum_mag_num(double __x, double __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern double __fminimum_mag_num(double __x, double __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Total order operation.  */
extern int totalorder(const double *__x, const double *__y) __attribute__((__nothrow__, __leaf__))

__attribute__((__pure__));

/* Total order operation on absolute values.  */
extern int totalordermag(const double *__x, const double *__y) __attribute__((__nothrow__, __leaf__))

__attribute__((__pure__));

/* Get NaN payload.  */
extern double getpayload(const double *__x) __attribute__((__nothrow__, __leaf__));
extern double __getpayload(const double *__x) __attribute__((__nothrow__, __leaf__));

/* Set quiet NaN payload.  */
extern int setpayload(double *__x, double __payload) __attribute__((__nothrow__, __leaf__));

/* Set signaling NaN payload.  */
extern int setpayloadsig(double *__x, double __payload) __attribute__((__nothrow__, __leaf__));

/* Return X times (2 to the Nth power).  */
extern double scalb(double __x, double __n) __attribute__((__nothrow__, __leaf__));
extern double __scalb(double __x, double __n) __attribute__((__nothrow__, __leaf__));
/* Include the file of declarations again, this time using `float'
   instead of `double' and appending f to each function name.  */

/* Prototype declarations for math classification macros helpers.
   Copyright (C) 2017-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* Classify given number.  */
extern int __fpclassifyf(float __value) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Test for negative number.  */
extern int __signbitf(float __value) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return 0 if VALUE is finite or NaN, +1 if it
   is +Infinity, -1 if it is -Infinity.  */
extern int __isinff(float __value) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return nonzero if VALUE is finite and not NaN.  Used by isfinite macro.  */
extern int __finitef(float __value) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return nonzero if VALUE is not a number.  */
extern int __isnanf(float __value) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Test equality.  */
extern int __iseqsigf(float __x, float __y) __attribute__((__nothrow__, __leaf__));

/* Test for signaling NaN.  */
extern int __issignalingf(float __value) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
/* Prototype declarations for math functions; helper file for <math.h>.
   Copyright (C) 1996-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* NOTE: Because of the special way this file is used by <math.h>, this
   file must NOT be protected from multiple inclusion as header files
   usually are.

   This file provides prototype declarations for the math functions.
   Most functions are declared using the macro:

   __MATHCALL (NAME,[_r], (ARGS...));

   This means there is a function `NAME' returning `double' and a function
   `NAMEf' returning `float'.  Each place `_Mdouble_' appears in the
   prototype, that is actually `double' in the prototype for `NAME' and
   `float' in the prototype for `NAMEf'.  Reentrant variant functions are
   called `NAME_r' and `NAMEf_r'.

   Functions returning other types like `int' are declared using the macro:

   __MATHDECL (TYPE, NAME,[_r], (ARGS...));

   This is just like __MATHCALL but for a function returning `TYPE'
   instead of `_Mdouble_'.  In all of these cases, there is still
   both a `NAME' and a `NAMEf' that takes `float' arguments.

   Note that there must be no whitespace before the argument passed for
   NAME, to make token pasting work with -traditional.  */

/* Trigonometric functions.  */

/* Arc cosine of X.  */
extern float acosf(float __x) __attribute__((__nothrow__, __leaf__));
extern float __acosf(float __x) __attribute__((__nothrow__, __leaf__));
/* Arc sine of X.  */
extern float asinf(float __x) __attribute__((__nothrow__, __leaf__));
extern float __asinf(float __x) __attribute__((__nothrow__, __leaf__));
/* Arc tangent of X.  */
extern float atanf(float __x) __attribute__((__nothrow__, __leaf__));
extern float __atanf(float __x) __attribute__((__nothrow__, __leaf__));
/* Arc tangent of Y/X.  */
extern float atan2f(float __y, float __x) __attribute__((__nothrow__, __leaf__));
extern float __atan2f(float __y, float __x) __attribute__((__nothrow__, __leaf__));

/* Cosine of X.  */
extern float cosf(float __x) __attribute__((__nothrow__, __leaf__));
extern float __cosf(float __x) __attribute__((__nothrow__, __leaf__));
/* Sine of X.  */
extern float sinf(float __x) __attribute__((__nothrow__, __leaf__));
extern float __sinf(float __x) __attribute__((__nothrow__, __leaf__));
/* Tangent of X.  */
extern float tanf(float __x) __attribute__((__nothrow__, __leaf__));
extern float __tanf(float __x) __attribute__((__nothrow__, __leaf__));

/* Hyperbolic functions.  */

/* Hyperbolic cosine of X.  */
extern float coshf(float __x) __attribute__((__nothrow__, __leaf__));
extern float __coshf(float __x) __attribute__((__nothrow__, __leaf__));
/* Hyperbolic sine of X.  */
extern float sinhf(float __x) __attribute__((__nothrow__, __leaf__));
extern float __sinhf(float __x) __attribute__((__nothrow__, __leaf__));
/* Hyperbolic tangent of X.  */
extern float tanhf(float __x) __attribute__((__nothrow__, __leaf__));
extern float __tanhf(float __x) __attribute__((__nothrow__, __leaf__));

/* Cosine and sine of X.  */
extern void sincosf(float __x, float *__sinx, float *__cosx) __attribute__((__nothrow__, __leaf__));
extern void __sincosf(float __x, float *__sinx, float *__cosx) __attribute__((__nothrow__, __leaf__));

/* Hyperbolic arc cosine of X.  */
extern float acoshf(float __x) __attribute__((__nothrow__, __leaf__));
extern float __acoshf(float __x) __attribute__((__nothrow__, __leaf__));
/* Hyperbolic arc sine of X.  */
extern float asinhf(float __x) __attribute__((__nothrow__, __leaf__));
extern float __asinhf(float __x) __attribute__((__nothrow__, __leaf__));
/* Hyperbolic arc tangent of X.  */
extern float atanhf(float __x) __attribute__((__nothrow__, __leaf__));
extern float __atanhf(float __x) __attribute__((__nothrow__, __leaf__));

/* Exponential and logarithmic functions.  */

/* Exponential function of X.  */
extern float expf(float __x) __attribute__((__nothrow__, __leaf__));
extern float __expf(float __x) __attribute__((__nothrow__, __leaf__));

/* Break VALUE into a normalized fraction and an integral power of 2.  */
extern float frexpf(float __x, int *__exponent) __attribute__((__nothrow__, __leaf__));
extern float __frexpf(float __x, int *__exponent) __attribute__((__nothrow__, __leaf__));

/* X times (two to the EXP power).  */
extern float ldexpf(float __x, int __exponent) __attribute__((__nothrow__, __leaf__));
extern float __ldexpf(float __x, int __exponent) __attribute__((__nothrow__, __leaf__));

/* Natural logarithm of X.  */
extern float logf(float __x) __attribute__((__nothrow__, __leaf__));
extern float __logf(float __x) __attribute__((__nothrow__, __leaf__));

/* Base-ten logarithm of X.  */
extern float log10f(float __x) __attribute__((__nothrow__, __leaf__));
extern float __log10f(float __x) __attribute__((__nothrow__, __leaf__));

/* Break VALUE into integral and fractional parts.  */
extern float modff(float __x, float *__iptr) __attribute__((__nothrow__, __leaf__));
extern float __modff(float __x, float *__iptr) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(2)));

/* Compute exponent to base ten.  */
extern float exp10f(float __x) __attribute__((__nothrow__, __leaf__));
extern float __exp10f(float __x) __attribute__((__nothrow__, __leaf__));

/* Return exp(X) - 1.  */
extern float expm1f(float __x) __attribute__((__nothrow__, __leaf__));
extern float __expm1f(float __x) __attribute__((__nothrow__, __leaf__));

/* Return log(1 + X).  */
extern float log1pf(float __x) __attribute__((__nothrow__, __leaf__));
extern float __log1pf(float __x) __attribute__((__nothrow__, __leaf__));

/* Return the base 2 signed integral exponent of X.  */
extern float logbf(float __x) __attribute__((__nothrow__, __leaf__));
extern float __logbf(float __x) __attribute__((__nothrow__, __leaf__));

/* Compute base-2 exponential of X.  */
extern float exp2f(float __x) __attribute__((__nothrow__, __leaf__));
extern float __exp2f(float __x) __attribute__((__nothrow__, __leaf__));

/* Compute base-2 logarithm of X.  */
extern float log2f(float __x) __attribute__((__nothrow__, __leaf__));
extern float __log2f(float __x) __attribute__((__nothrow__, __leaf__));

/* Power functions.  */

/* Return X to the Y power.  */
extern float powf(float __x, float __y) __attribute__((__nothrow__, __leaf__));
extern float __powf(float __x, float __y) __attribute__((__nothrow__, __leaf__));

/* Return the square root of X.  */
extern float sqrtf(float __x) __attribute__((__nothrow__, __leaf__));
extern float __sqrtf(float __x) __attribute__((__nothrow__, __leaf__));

/* Return `sqrt(X*X + Y*Y)'.  */
extern float hypotf(float __x, float __y) __attribute__((__nothrow__, __leaf__));
extern float __hypotf(float __x, float __y) __attribute__((__nothrow__, __leaf__));

/* Return the cube root of X.  */
extern float cbrtf(float __x) __attribute__((__nothrow__, __leaf__));
extern float __cbrtf(float __x) __attribute__((__nothrow__, __leaf__));

/* Nearest integer, absolute value, and remainder functions.  */

/* Smallest integral value not less than X.  */
extern float ceilf(float __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern float __ceilf(float __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Absolute value of X.  */
extern float fabsf(float __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern float __fabsf(float __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Largest integer not greater than X.  */
extern float floorf(float __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern float __floorf(float __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Floating-point modulo remainder of X/Y.  */
extern float fmodf(float __x, float __y) __attribute__((__nothrow__, __leaf__));
extern float __fmodf(float __x, float __y) __attribute__((__nothrow__, __leaf__));

/* Return 0 if VALUE is finite or NaN, +1 if it
   is +Infinity, -1 if it is -Infinity.  */
extern int isinff(float __value) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return nonzero if VALUE is finite and not NaN.  */
extern int finitef(float __value) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return the remainder of X/Y.  */
extern float dremf(float __x, float __y) __attribute__((__nothrow__, __leaf__));
extern float __dremf(float __x, float __y) __attribute__((__nothrow__, __leaf__));

/* Return the fractional part of X after dividing out `ilogb (X)'.  */
extern float significandf(float __x) __attribute__((__nothrow__, __leaf__));
extern float __significandf(float __x) __attribute__((__nothrow__, __leaf__));

/* Return X with its signed changed to Y's.  */
extern float copysignf(float __x, float __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern float __copysignf(float __x, float __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return representation of qNaN for double type.  */
extern float nanf(const char *__tagb) __attribute__((__nothrow__, __leaf__));
extern float __nanf(const char *__tagb) __attribute__((__nothrow__, __leaf__));
/* Return nonzero if VALUE is not a number.  */
extern int isnanf(float __value) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Bessel functions.  */
extern float j0f(float) __attribute__((__nothrow__, __leaf__));
extern float __j0f(float) __attribute__((__nothrow__, __leaf__));
extern float j1f(float) __attribute__((__nothrow__, __leaf__));
extern float __j1f(float) __attribute__((__nothrow__, __leaf__));
extern float jnf(int, float) __attribute__((__nothrow__, __leaf__));
extern float __jnf(int, float) __attribute__((__nothrow__, __leaf__));
extern float y0f(float) __attribute__((__nothrow__, __leaf__));
extern float __y0f(float) __attribute__((__nothrow__, __leaf__));
extern float y1f(float) __attribute__((__nothrow__, __leaf__));
extern float __y1f(float) __attribute__((__nothrow__, __leaf__));
extern float ynf(int, float) __attribute__((__nothrow__, __leaf__));
extern float __ynf(int, float) __attribute__((__nothrow__, __leaf__));

/* Error and gamma functions.  */
extern float erff(float) __attribute__((__nothrow__, __leaf__));
extern float __erff(float) __attribute__((__nothrow__, __leaf__));
extern float erfcf(float) __attribute__((__nothrow__, __leaf__));
extern float __erfcf(float) __attribute__((__nothrow__, __leaf__));
extern float lgammaf(float) __attribute__((__nothrow__, __leaf__));
extern float __lgammaf(float) __attribute__((__nothrow__, __leaf__));

/* True gamma function.  */
extern float tgammaf(float) __attribute__((__nothrow__, __leaf__));
extern float __tgammaf(float) __attribute__((__nothrow__, __leaf__));

/* Obsolete alias for `lgamma'.  */
extern float gammaf(float) __attribute__((__nothrow__, __leaf__));
extern float __gammaf(float) __attribute__((__nothrow__, __leaf__));

/* Reentrant version of lgamma.  This function uses the global variable
   `signgam'.  The reentrant version instead takes a pointer and stores
   the value through it.  */
extern float lgammaf_r(float, int *__signgamp) __attribute__((__nothrow__, __leaf__));
extern float __lgammaf_r(float, int *__signgamp) __attribute__((__nothrow__, __leaf__));

/* Return the integer nearest X in the direction of the
   prevailing rounding mode.  */
extern float rintf(float __x) __attribute__((__nothrow__, __leaf__));
extern float __rintf(float __x) __attribute__((__nothrow__, __leaf__));

/* Return X + epsilon if X < Y, X - epsilon if X > Y.  */
extern float nextafterf(float __x, float __y) __attribute__((__nothrow__, __leaf__));
extern float __nextafterf(float __x, float __y) __attribute__((__nothrow__, __leaf__));

extern float nexttowardf(float __x, long double __y) __attribute__((__nothrow__, __leaf__));
extern float __nexttowardf(float __x, long double __y) __attribute__((__nothrow__, __leaf__));

/* Return X - epsilon.  */
extern float nextdownf(float __x) __attribute__((__nothrow__, __leaf__));
extern float __nextdownf(float __x) __attribute__((__nothrow__, __leaf__));
/* Return X + epsilon.  */
extern float nextupf(float __x) __attribute__((__nothrow__, __leaf__));
extern float __nextupf(float __x) __attribute__((__nothrow__, __leaf__));

/* Return the remainder of integer division X / Y with infinite precision.  */
extern float remainderf(float __x, float __y) __attribute__((__nothrow__, __leaf__));
extern float __remainderf(float __x, float __y) __attribute__((__nothrow__, __leaf__));

/* Return X times (2 to the Nth power).  */
extern float scalbnf(float __x, int __n) __attribute__((__nothrow__, __leaf__));
extern float __scalbnf(float __x, int __n) __attribute__((__nothrow__, __leaf__));

/* Return the binary exponent of X, which must be nonzero.  */
extern int ilogbf(float __x) __attribute__((__nothrow__, __leaf__));
extern int __ilogbf(float __x) __attribute__((__nothrow__, __leaf__));

/* Like ilogb, but returning long int.  */
extern long int llogbf(float __x) __attribute__((__nothrow__, __leaf__));
extern long int __llogbf(float __x) __attribute__((__nothrow__, __leaf__));

/* Return X times (2 to the Nth power).  */
extern float scalblnf(float __x, long int __n) __attribute__((__nothrow__, __leaf__));
extern float __scalblnf(float __x, long int __n) __attribute__((__nothrow__, __leaf__));

/* Round X to integral value in floating-point format using current
   rounding direction, but do not raise inexact exception.  */
extern float nearbyintf(float __x) __attribute__((__nothrow__, __leaf__));
extern float __nearbyintf(float __x) __attribute__((__nothrow__, __leaf__));

/* Round X to nearest integral value, rounding halfway cases away from
   zero.  */
extern float roundf(float __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern float __roundf(float __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Round X to the integral value in floating-point format nearest but
   not larger in magnitude.  */
extern float truncf(float __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern float __truncf(float __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Compute remainder of X and Y and put in *QUO a value with sign of x/y
   and magnitude congruent `mod 2^n' to the magnitude of the integral
   quotient x/y, with n >= 3.  */
extern float remquof(float __x, float __y, int *__quo) __attribute__((__nothrow__, __leaf__));
extern float __remquof(float __x, float __y, int *__quo) __attribute__((__nothrow__, __leaf__));

/* Conversion functions.  */

/* Round X to nearest integral value according to current rounding
   direction.  */
extern long int lrintf(float __x) __attribute__((__nothrow__, __leaf__));
extern long int __lrintf(float __x) __attribute__((__nothrow__, __leaf__));
__extension__ extern long long int llrintf(float __x) __attribute__((__nothrow__, __leaf__));
extern long long int __llrintf(float __x) __attribute__((__nothrow__, __leaf__));

/* Round X to nearest integral value, rounding halfway cases away from
   zero.  */
extern long int lroundf(float __x) __attribute__((__nothrow__, __leaf__));
extern long int __lroundf(float __x) __attribute__((__nothrow__, __leaf__));
__extension__ extern long long int llroundf(float __x) __attribute__((__nothrow__, __leaf__));
extern long long int __llroundf(float __x) __attribute__((__nothrow__, __leaf__));

/* Return positive difference between X and Y.  */
extern float fdimf(float __x, float __y) __attribute__((__nothrow__, __leaf__));
extern float __fdimf(float __x, float __y) __attribute__((__nothrow__, __leaf__));

/* Return maximum numeric value from X and Y.  */
extern float fmaxf(float __x, float __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern float __fmaxf(float __x, float __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return minimum numeric value from X and Y.  */
extern float fminf(float __x, float __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern float __fminf(float __x, float __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Multiply-add function computed as a ternary operation.  */
extern float fmaf(float __x, float __y, float __z) __attribute__((__nothrow__, __leaf__));
extern float __fmaf(float __x, float __y, float __z) __attribute__((__nothrow__, __leaf__));

/* Round X to nearest integer value, rounding halfway cases to even.  */
extern float roundevenf(float __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern float __roundevenf(float __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Round X to nearest signed integer value, not raising inexact, with
   control of rounding direction and width of result.  */
extern __intmax_t fromfpf(float __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));
extern __intmax_t __fromfpf(float __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));

/* Round X to nearest unsigned integer value, not raising inexact,
   with control of rounding direction and width of result.  */
extern __uintmax_t ufromfpf(float __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));
extern __uintmax_t __ufromfpf(float __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));

/* Round X to nearest signed integer value, raising inexact for
   non-integers, with control of rounding direction and width of
   result.  */
extern __intmax_t fromfpxf(float __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));
extern __intmax_t __fromfpxf(float __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));

/* Round X to nearest unsigned integer value, raising inexact for
   non-integers, with control of rounding direction and width of
   result.  */
extern __uintmax_t ufromfpxf(float __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));
extern __uintmax_t __ufromfpxf(float __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));

/* Canonicalize floating-point representation.  */
extern int canonicalizef(float *__cx, const float *__x) __attribute__((__nothrow__, __leaf__));

/* Return value with maximum magnitude.  */
extern float fmaxmagf(float __x, float __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern float __fmaxmagf(float __x, float __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return value with minimum magnitude.  */
extern float fminmagf(float __x, float __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern float __fminmagf(float __x, float __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return maximum value from X and Y.  */
extern float fmaximumf(float __x, float __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern float __fmaximumf(float __x, float __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return minimum value from X and Y.  */
extern float fminimumf(float __x, float __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern float __fminimumf(float __x, float __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return maximum numeric value from X and Y.  */
extern float fmaximum_numf(float __x, float __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern float __fmaximum_numf(float __x, float __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return minimum numeric value from X and Y.  */
extern float fminimum_numf(float __x, float __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern float __fminimum_numf(float __x, float __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return value with maximum magnitude.  */
extern float fmaximum_magf(float __x, float __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern float __fmaximum_magf(float __x, float __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return value with minimum magnitude.  */
extern float fminimum_magf(float __x, float __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern float __fminimum_magf(float __x, float __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return numeric value with maximum magnitude.  */
extern float fmaximum_mag_numf(float __x, float __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern float __fmaximum_mag_numf(float __x, float __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return numeric value with minimum magnitude.  */
extern float fminimum_mag_numf(float __x, float __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern float __fminimum_mag_numf(float __x, float __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Total order operation.  */
extern int totalorderf(const float *__x, const float *__y) __attribute__((__nothrow__, __leaf__))

__attribute__((__pure__));

/* Total order operation on absolute values.  */
extern int totalordermagf(const float *__x, const float *__y) __attribute__((__nothrow__, __leaf__))

__attribute__((__pure__));

/* Get NaN payload.  */
extern float getpayloadf(const float *__x) __attribute__((__nothrow__, __leaf__));
extern float __getpayloadf(const float *__x) __attribute__((__nothrow__, __leaf__));

/* Set quiet NaN payload.  */
extern int setpayloadf(float *__x, float __payload) __attribute__((__nothrow__, __leaf__));

/* Set signaling NaN payload.  */
extern int setpayloadsigf(float *__x, float __payload) __attribute__((__nothrow__, __leaf__));

/* Return X times (2 to the Nth power).  */
extern float scalbf(float __x, float __n) __attribute__((__nothrow__, __leaf__));
extern float __scalbf(float __x, float __n) __attribute__((__nothrow__, __leaf__));
/* Include the file of declarations again, this time using `long double'
   instead of `double' and appending l to each function name.  */

/* Prototype declarations for math classification macros helpers.
   Copyright (C) 2017-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* Classify given number.  */
extern int __fpclassifyl(long double __value) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Test for negative number.  */
extern int __signbitl(long double __value) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return 0 if VALUE is finite or NaN, +1 if it
   is +Infinity, -1 if it is -Infinity.  */
extern int __isinfl(long double __value) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return nonzero if VALUE is finite and not NaN.  Used by isfinite macro.  */
extern int __finitel(long double __value) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return nonzero if VALUE is not a number.  */
extern int __isnanl(long double __value) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Test equality.  */
extern int __iseqsigl(long double __x, long double __y) __attribute__((__nothrow__, __leaf__));

/* Test for signaling NaN.  */
extern int __issignalingl(long double __value) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
/* Prototype declarations for math functions; helper file for <math.h>.
   Copyright (C) 1996-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* NOTE: Because of the special way this file is used by <math.h>, this
   file must NOT be protected from multiple inclusion as header files
   usually are.

   This file provides prototype declarations for the math functions.
   Most functions are declared using the macro:

   __MATHCALL (NAME,[_r], (ARGS...));

   This means there is a function `NAME' returning `double' and a function
   `NAMEf' returning `float'.  Each place `_Mdouble_' appears in the
   prototype, that is actually `double' in the prototype for `NAME' and
   `float' in the prototype for `NAMEf'.  Reentrant variant functions are
   called `NAME_r' and `NAMEf_r'.

   Functions returning other types like `int' are declared using the macro:

   __MATHDECL (TYPE, NAME,[_r], (ARGS...));

   This is just like __MATHCALL but for a function returning `TYPE'
   instead of `_Mdouble_'.  In all of these cases, there is still
   both a `NAME' and a `NAMEf' that takes `float' arguments.

   Note that there must be no whitespace before the argument passed for
   NAME, to make token pasting work with -traditional.  */

/* Trigonometric functions.  */

/* Arc cosine of X.  */
extern long double acosl(long double __x) __attribute__((__nothrow__, __leaf__));
extern long double __acosl(long double __x) __attribute__((__nothrow__, __leaf__));
/* Arc sine of X.  */
extern long double asinl(long double __x) __attribute__((__nothrow__, __leaf__));
extern long double __asinl(long double __x) __attribute__((__nothrow__, __leaf__));
/* Arc tangent of X.  */
extern long double atanl(long double __x) __attribute__((__nothrow__, __leaf__));
extern long double __atanl(long double __x) __attribute__((__nothrow__, __leaf__));
/* Arc tangent of Y/X.  */
extern long double atan2l(long double __y, long double __x) __attribute__((__nothrow__, __leaf__));
extern long double __atan2l(long double __y, long double __x) __attribute__((__nothrow__, __leaf__));

/* Cosine of X.  */
extern long double cosl(long double __x) __attribute__((__nothrow__, __leaf__));
extern long double __cosl(long double __x) __attribute__((__nothrow__, __leaf__));
/* Sine of X.  */
extern long double sinl(long double __x) __attribute__((__nothrow__, __leaf__));
extern long double __sinl(long double __x) __attribute__((__nothrow__, __leaf__));
/* Tangent of X.  */
extern long double tanl(long double __x) __attribute__((__nothrow__, __leaf__));
extern long double __tanl(long double __x) __attribute__((__nothrow__, __leaf__));

/* Hyperbolic functions.  */

/* Hyperbolic cosine of X.  */
extern long double coshl(long double __x) __attribute__((__nothrow__, __leaf__));
extern long double __coshl(long double __x) __attribute__((__nothrow__, __leaf__));
/* Hyperbolic sine of X.  */
extern long double sinhl(long double __x) __attribute__((__nothrow__, __leaf__));
extern long double __sinhl(long double __x) __attribute__((__nothrow__, __leaf__));
/* Hyperbolic tangent of X.  */
extern long double tanhl(long double __x) __attribute__((__nothrow__, __leaf__));
extern long double __tanhl(long double __x) __attribute__((__nothrow__, __leaf__));

/* Cosine and sine of X.  */
extern void sincosl(long double __x, long double *__sinx, long double *__cosx) __attribute__((__nothrow__, __leaf__));
extern void __sincosl(long double __x, long double *__sinx, long double *__cosx) __attribute__((__nothrow__, __leaf__));

/* Hyperbolic arc cosine of X.  */
extern long double acoshl(long double __x) __attribute__((__nothrow__, __leaf__));
extern long double __acoshl(long double __x) __attribute__((__nothrow__, __leaf__));
/* Hyperbolic arc sine of X.  */
extern long double asinhl(long double __x) __attribute__((__nothrow__, __leaf__));
extern long double __asinhl(long double __x) __attribute__((__nothrow__, __leaf__));
/* Hyperbolic arc tangent of X.  */
extern long double atanhl(long double __x) __attribute__((__nothrow__, __leaf__));
extern long double __atanhl(long double __x) __attribute__((__nothrow__, __leaf__));

/* Exponential and logarithmic functions.  */

/* Exponential function of X.  */
extern long double expl(long double __x) __attribute__((__nothrow__, __leaf__));
extern long double __expl(long double __x) __attribute__((__nothrow__, __leaf__));

/* Break VALUE into a normalized fraction and an integral power of 2.  */
extern long double frexpl(long double __x, int *__exponent) __attribute__((__nothrow__, __leaf__));
extern long double __frexpl(long double __x, int *__exponent) __attribute__((__nothrow__, __leaf__));

/* X times (two to the EXP power).  */
extern long double ldexpl(long double __x, int __exponent) __attribute__((__nothrow__, __leaf__));
extern long double __ldexpl(long double __x, int __exponent) __attribute__((__nothrow__, __leaf__));

/* Natural logarithm of X.  */
extern long double logl(long double __x) __attribute__((__nothrow__, __leaf__));
extern long double __logl(long double __x) __attribute__((__nothrow__, __leaf__));

/* Base-ten logarithm of X.  */
extern long double log10l(long double __x) __attribute__((__nothrow__, __leaf__));
extern long double __log10l(long double __x) __attribute__((__nothrow__, __leaf__));

/* Break VALUE into integral and fractional parts.  */
extern long double modfl(long double __x, long double *__iptr) __attribute__((__nothrow__, __leaf__));
extern long double __modfl(long double __x, long double *__iptr) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(2)));

/* Compute exponent to base ten.  */
extern long double exp10l(long double __x) __attribute__((__nothrow__, __leaf__));
extern long double __exp10l(long double __x) __attribute__((__nothrow__, __leaf__));

/* Return exp(X) - 1.  */
extern long double expm1l(long double __x) __attribute__((__nothrow__, __leaf__));
extern long double __expm1l(long double __x) __attribute__((__nothrow__, __leaf__));

/* Return log(1 + X).  */
extern long double log1pl(long double __x) __attribute__((__nothrow__, __leaf__));
extern long double __log1pl(long double __x) __attribute__((__nothrow__, __leaf__));

/* Return the base 2 signed integral exponent of X.  */
extern long double logbl(long double __x) __attribute__((__nothrow__, __leaf__));
extern long double __logbl(long double __x) __attribute__((__nothrow__, __leaf__));

/* Compute base-2 exponential of X.  */
extern long double exp2l(long double __x) __attribute__((__nothrow__, __leaf__));
extern long double __exp2l(long double __x) __attribute__((__nothrow__, __leaf__));

/* Compute base-2 logarithm of X.  */
extern long double log2l(long double __x) __attribute__((__nothrow__, __leaf__));
extern long double __log2l(long double __x) __attribute__((__nothrow__, __leaf__));

/* Power functions.  */

/* Return X to the Y power.  */
extern long double powl(long double __x, long double __y) __attribute__((__nothrow__, __leaf__));
extern long double __powl(long double __x, long double __y) __attribute__((__nothrow__, __leaf__));

/* Return the square root of X.  */
extern long double sqrtl(long double __x) __attribute__((__nothrow__, __leaf__));
extern long double __sqrtl(long double __x) __attribute__((__nothrow__, __leaf__));

/* Return `sqrt(X*X + Y*Y)'.  */
extern long double hypotl(long double __x, long double __y) __attribute__((__nothrow__, __leaf__));
extern long double __hypotl(long double __x, long double __y) __attribute__((__nothrow__, __leaf__));

/* Return the cube root of X.  */
extern long double cbrtl(long double __x) __attribute__((__nothrow__, __leaf__));
extern long double __cbrtl(long double __x) __attribute__((__nothrow__, __leaf__));

/* Nearest integer, absolute value, and remainder functions.  */

/* Smallest integral value not less than X.  */
extern long double ceill(long double __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern long double __ceill(long double __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Absolute value of X.  */
extern long double fabsl(long double __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern long double __fabsl(long double __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Largest integer not greater than X.  */
extern long double floorl(long double __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern long double __floorl(long double __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Floating-point modulo remainder of X/Y.  */
extern long double fmodl(long double __x, long double __y) __attribute__((__nothrow__, __leaf__));
extern long double __fmodl(long double __x, long double __y) __attribute__((__nothrow__, __leaf__));

/* Return 0 if VALUE is finite or NaN, +1 if it
   is +Infinity, -1 if it is -Infinity.  */
extern int isinfl(long double __value) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return nonzero if VALUE is finite and not NaN.  */
extern int finitel(long double __value) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return the remainder of X/Y.  */
extern long double dreml(long double __x, long double __y) __attribute__((__nothrow__, __leaf__));
extern long double __dreml(long double __x, long double __y) __attribute__((__nothrow__, __leaf__));

/* Return the fractional part of X after dividing out `ilogb (X)'.  */
extern long double significandl(long double __x) __attribute__((__nothrow__, __leaf__));
extern long double __significandl(long double __x) __attribute__((__nothrow__, __leaf__));

/* Return X with its signed changed to Y's.  */
extern long double copysignl(long double __x, long double __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern long double __copysignl(long double __x, long double __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return representation of qNaN for double type.  */
extern long double nanl(const char *__tagb) __attribute__((__nothrow__, __leaf__));
extern long double __nanl(const char *__tagb) __attribute__((__nothrow__, __leaf__));
/* Return nonzero if VALUE is not a number.  */
extern int isnanl(long double __value) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Bessel functions.  */
extern long double j0l(long double) __attribute__((__nothrow__, __leaf__));
extern long double __j0l(long double) __attribute__((__nothrow__, __leaf__));
extern long double j1l(long double) __attribute__((__nothrow__, __leaf__));
extern long double __j1l(long double) __attribute__((__nothrow__, __leaf__));
extern long double jnl(int, long double) __attribute__((__nothrow__, __leaf__));
extern long double __jnl(int, long double) __attribute__((__nothrow__, __leaf__));
extern long double y0l(long double) __attribute__((__nothrow__, __leaf__));
extern long double __y0l(long double) __attribute__((__nothrow__, __leaf__));
extern long double y1l(long double) __attribute__((__nothrow__, __leaf__));
extern long double __y1l(long double) __attribute__((__nothrow__, __leaf__));
extern long double ynl(int, long double) __attribute__((__nothrow__, __leaf__));
extern long double __ynl(int, long double) __attribute__((__nothrow__, __leaf__));

/* Error and gamma functions.  */
extern long double erfl(long double) __attribute__((__nothrow__, __leaf__));
extern long double __erfl(long double) __attribute__((__nothrow__, __leaf__));
extern long double erfcl(long double) __attribute__((__nothrow__, __leaf__));
extern long double __erfcl(long double) __attribute__((__nothrow__, __leaf__));
extern long double lgammal(long double) __attribute__((__nothrow__, __leaf__));
extern long double __lgammal(long double) __attribute__((__nothrow__, __leaf__));

/* True gamma function.  */
extern long double tgammal(long double) __attribute__((__nothrow__, __leaf__));
extern long double __tgammal(long double) __attribute__((__nothrow__, __leaf__));

/* Obsolete alias for `lgamma'.  */
extern long double gammal(long double) __attribute__((__nothrow__, __leaf__));
extern long double __gammal(long double) __attribute__((__nothrow__, __leaf__));

/* Reentrant version of lgamma.  This function uses the global variable
   `signgam'.  The reentrant version instead takes a pointer and stores
   the value through it.  */
extern long double lgammal_r(long double, int *__signgamp) __attribute__((__nothrow__, __leaf__));
extern long double __lgammal_r(long double, int *__signgamp) __attribute__((__nothrow__, __leaf__));

/* Return the integer nearest X in the direction of the
   prevailing rounding mode.  */
extern long double rintl(long double __x) __attribute__((__nothrow__, __leaf__));
extern long double __rintl(long double __x) __attribute__((__nothrow__, __leaf__));

/* Return X + epsilon if X < Y, X - epsilon if X > Y.  */
extern long double nextafterl(long double __x, long double __y) __attribute__((__nothrow__, __leaf__));
extern long double __nextafterl(long double __x, long double __y) __attribute__((__nothrow__, __leaf__));

extern long double nexttowardl(long double __x, long double __y) __attribute__((__nothrow__, __leaf__));
extern long double __nexttowardl(long double __x, long double __y) __attribute__((__nothrow__, __leaf__));

/* Return X - epsilon.  */
extern long double nextdownl(long double __x) __attribute__((__nothrow__, __leaf__));
extern long double __nextdownl(long double __x) __attribute__((__nothrow__, __leaf__));
/* Return X + epsilon.  */
extern long double nextupl(long double __x) __attribute__((__nothrow__, __leaf__));
extern long double __nextupl(long double __x) __attribute__((__nothrow__, __leaf__));

/* Return the remainder of integer division X / Y with infinite precision.  */
extern long double remainderl(long double __x, long double __y) __attribute__((__nothrow__, __leaf__));
extern long double __remainderl(long double __x, long double __y) __attribute__((__nothrow__, __leaf__));

/* Return X times (2 to the Nth power).  */
extern long double scalbnl(long double __x, int __n) __attribute__((__nothrow__, __leaf__));
extern long double __scalbnl(long double __x, int __n) __attribute__((__nothrow__, __leaf__));

/* Return the binary exponent of X, which must be nonzero.  */
extern int ilogbl(long double __x) __attribute__((__nothrow__, __leaf__));
extern int __ilogbl(long double __x) __attribute__((__nothrow__, __leaf__));

/* Like ilogb, but returning long int.  */
extern long int llogbl(long double __x) __attribute__((__nothrow__, __leaf__));
extern long int __llogbl(long double __x) __attribute__((__nothrow__, __leaf__));

/* Return X times (2 to the Nth power).  */
extern long double scalblnl(long double __x, long int __n) __attribute__((__nothrow__, __leaf__));
extern long double __scalblnl(long double __x, long int __n) __attribute__((__nothrow__, __leaf__));

/* Round X to integral value in floating-point format using current
   rounding direction, but do not raise inexact exception.  */
extern long double nearbyintl(long double __x) __attribute__((__nothrow__, __leaf__));
extern long double __nearbyintl(long double __x) __attribute__((__nothrow__, __leaf__));

/* Round X to nearest integral value, rounding halfway cases away from
   zero.  */
extern long double roundl(long double __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern long double __roundl(long double __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Round X to the integral value in floating-point format nearest but
   not larger in magnitude.  */
extern long double truncl(long double __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern long double __truncl(long double __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Compute remainder of X and Y and put in *QUO a value with sign of x/y
   and magnitude congruent `mod 2^n' to the magnitude of the integral
   quotient x/y, with n >= 3.  */
extern long double remquol(long double __x, long double __y, int *__quo) __attribute__((__nothrow__, __leaf__));
extern long double __remquol(long double __x, long double __y, int *__quo) __attribute__((__nothrow__, __leaf__));

/* Conversion functions.  */

/* Round X to nearest integral value according to current rounding
   direction.  */
extern long int lrintl(long double __x) __attribute__((__nothrow__, __leaf__));
extern long int __lrintl(long double __x) __attribute__((__nothrow__, __leaf__));
__extension__ extern long long int llrintl(long double __x) __attribute__((__nothrow__, __leaf__));
extern long long int __llrintl(long double __x) __attribute__((__nothrow__, __leaf__));

/* Round X to nearest integral value, rounding halfway cases away from
   zero.  */
extern long int lroundl(long double __x) __attribute__((__nothrow__, __leaf__));
extern long int __lroundl(long double __x) __attribute__((__nothrow__, __leaf__));
__extension__ extern long long int llroundl(long double __x) __attribute__((__nothrow__, __leaf__));
extern long long int __llroundl(long double __x) __attribute__((__nothrow__, __leaf__));

/* Return positive difference between X and Y.  */
extern long double fdiml(long double __x, long double __y) __attribute__((__nothrow__, __leaf__));
extern long double __fdiml(long double __x, long double __y) __attribute__((__nothrow__, __leaf__));

/* Return maximum numeric value from X and Y.  */
extern long double fmaxl(long double __x, long double __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern long double __fmaxl(long double __x, long double __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return minimum numeric value from X and Y.  */
extern long double fminl(long double __x, long double __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern long double __fminl(long double __x, long double __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Multiply-add function computed as a ternary operation.  */
extern long double fmal(long double __x, long double __y, long double __z) __attribute__((__nothrow__, __leaf__));
extern long double __fmal(long double __x, long double __y, long double __z) __attribute__((__nothrow__, __leaf__));

/* Round X to nearest integer value, rounding halfway cases to even.  */
extern long double roundevenl(long double __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern long double __roundevenl(long double __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Round X to nearest signed integer value, not raising inexact, with
   control of rounding direction and width of result.  */
extern __intmax_t fromfpl(long double __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));
extern __intmax_t __fromfpl(long double __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));

/* Round X to nearest unsigned integer value, not raising inexact,
   with control of rounding direction and width of result.  */
extern __uintmax_t ufromfpl(long double __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));
extern __uintmax_t __ufromfpl(long double __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));

/* Round X to nearest signed integer value, raising inexact for
   non-integers, with control of rounding direction and width of
   result.  */
extern __intmax_t fromfpxl(long double __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));
extern __intmax_t __fromfpxl(long double __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));

/* Round X to nearest unsigned integer value, raising inexact for
   non-integers, with control of rounding direction and width of
   result.  */
extern __uintmax_t ufromfpxl(long double __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));
extern __uintmax_t __ufromfpxl(long double __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));

/* Canonicalize floating-point representation.  */
extern int canonicalizel(long double *__cx, const long double *__x) __attribute__((__nothrow__, __leaf__));

/* Return value with maximum magnitude.  */
extern long double fmaxmagl(long double __x, long double __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern long double __fmaxmagl(long double __x, long double __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return value with minimum magnitude.  */
extern long double fminmagl(long double __x, long double __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern long double __fminmagl(long double __x, long double __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return maximum value from X and Y.  */
extern long double fmaximuml(long double __x, long double __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern long double __fmaximuml(long double __x, long double __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return minimum value from X and Y.  */
extern long double fminimuml(long double __x, long double __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern long double __fminimuml(long double __x, long double __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return maximum numeric value from X and Y.  */
extern long double fmaximum_numl(long double __x, long double __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern long double __fmaximum_numl(long double __x, long double __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return minimum numeric value from X and Y.  */
extern long double fminimum_numl(long double __x, long double __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern long double __fminimum_numl(long double __x, long double __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return value with maximum magnitude.  */
extern long double fmaximum_magl(long double __x, long double __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern long double __fmaximum_magl(long double __x, long double __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return value with minimum magnitude.  */
extern long double fminimum_magl(long double __x, long double __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern long double __fminimum_magl(long double __x, long double __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return numeric value with maximum magnitude.  */
extern long double fmaximum_mag_numl(long double __x, long double __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern long double __fmaximum_mag_numl(long double __x, long double __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return numeric value with minimum magnitude.  */
extern long double fminimum_mag_numl(long double __x, long double __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern long double __fminimum_mag_numl(long double __x, long double __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Total order operation.  */
extern int totalorderl(const long double *__x, const long double *__y) __attribute__((__nothrow__, __leaf__))

__attribute__((__pure__));

/* Total order operation on absolute values.  */
extern int totalordermagl(const long double *__x, const long double *__y) __attribute__((__nothrow__, __leaf__))

__attribute__((__pure__));

/* Get NaN payload.  */
extern long double getpayloadl(const long double *__x) __attribute__((__nothrow__, __leaf__));
extern long double __getpayloadl(const long double *__x) __attribute__((__nothrow__, __leaf__));

/* Set quiet NaN payload.  */
extern int setpayloadl(long double *__x, long double __payload) __attribute__((__nothrow__, __leaf__));

/* Set signaling NaN payload.  */
extern int setpayloadsigl(long double *__x, long double __payload) __attribute__((__nothrow__, __leaf__));

/* Return X times (2 to the Nth power).  */
extern long double scalbl(long double __x, long double __n) __attribute__((__nothrow__, __leaf__));
extern long double __scalbl(long double __x, long double __n) __attribute__((__nothrow__, __leaf__));
/* Include the file of declarations for _FloatN and _FloatNx
   types.  */
/* Prototype declarations for math functions; helper file for <math.h>.
   Copyright (C) 1996-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* NOTE: Because of the special way this file is used by <math.h>, this
   file must NOT be protected from multiple inclusion as header files
   usually are.

   This file provides prototype declarations for the math functions.
   Most functions are declared using the macro:

   __MATHCALL (NAME,[_r], (ARGS...));

   This means there is a function `NAME' returning `double' and a function
   `NAMEf' returning `float'.  Each place `_Mdouble_' appears in the
   prototype, that is actually `double' in the prototype for `NAME' and
   `float' in the prototype for `NAMEf'.  Reentrant variant functions are
   called `NAME_r' and `NAMEf_r'.

   Functions returning other types like `int' are declared using the macro:

   __MATHDECL (TYPE, NAME,[_r], (ARGS...));

   This is just like __MATHCALL but for a function returning `TYPE'
   instead of `_Mdouble_'.  In all of these cases, there is still
   both a `NAME' and a `NAMEf' that takes `float' arguments.

   Note that there must be no whitespace before the argument passed for
   NAME, to make token pasting work with -traditional.  */

/* Trigonometric functions.  */

/* Arc cosine of X.  */
extern _Float32 acosf32(_Float32 __x) __attribute__((__nothrow__, __leaf__));
extern _Float32 __acosf32(_Float32 __x) __attribute__((__nothrow__, __leaf__));
/* Arc sine of X.  */
extern _Float32 asinf32(_Float32 __x) __attribute__((__nothrow__, __leaf__));
extern _Float32 __asinf32(_Float32 __x) __attribute__((__nothrow__, __leaf__));
/* Arc tangent of X.  */
extern _Float32 atanf32(_Float32 __x) __attribute__((__nothrow__, __leaf__));
extern _Float32 __atanf32(_Float32 __x) __attribute__((__nothrow__, __leaf__));
/* Arc tangent of Y/X.  */
extern _Float32 atan2f32(_Float32 __y, _Float32 __x) __attribute__((__nothrow__, __leaf__));
extern _Float32 __atan2f32(_Float32 __y, _Float32 __x) __attribute__((__nothrow__, __leaf__));

/* Cosine of X.  */
extern _Float32 cosf32(_Float32 __x) __attribute__((__nothrow__, __leaf__));
extern _Float32 __cosf32(_Float32 __x) __attribute__((__nothrow__, __leaf__));
/* Sine of X.  */
extern _Float32 sinf32(_Float32 __x) __attribute__((__nothrow__, __leaf__));
extern _Float32 __sinf32(_Float32 __x) __attribute__((__nothrow__, __leaf__));
/* Tangent of X.  */
extern _Float32 tanf32(_Float32 __x) __attribute__((__nothrow__, __leaf__));
extern _Float32 __tanf32(_Float32 __x) __attribute__((__nothrow__, __leaf__));

/* Hyperbolic functions.  */

/* Hyperbolic cosine of X.  */
extern _Float32 coshf32(_Float32 __x) __attribute__((__nothrow__, __leaf__));
extern _Float32 __coshf32(_Float32 __x) __attribute__((__nothrow__, __leaf__));
/* Hyperbolic sine of X.  */
extern _Float32 sinhf32(_Float32 __x) __attribute__((__nothrow__, __leaf__));
extern _Float32 __sinhf32(_Float32 __x) __attribute__((__nothrow__, __leaf__));
/* Hyperbolic tangent of X.  */
extern _Float32 tanhf32(_Float32 __x) __attribute__((__nothrow__, __leaf__));
extern _Float32 __tanhf32(_Float32 __x) __attribute__((__nothrow__, __leaf__));

/* Cosine and sine of X.  */
extern void sincosf32(_Float32 __x, _Float32 *__sinx, _Float32 *__cosx) __attribute__((__nothrow__, __leaf__));
extern void __sincosf32(_Float32 __x, _Float32 *__sinx, _Float32 *__cosx) __attribute__((__nothrow__, __leaf__));

/* Hyperbolic arc cosine of X.  */
extern _Float32 acoshf32(_Float32 __x) __attribute__((__nothrow__, __leaf__));
extern _Float32 __acoshf32(_Float32 __x) __attribute__((__nothrow__, __leaf__));
/* Hyperbolic arc sine of X.  */
extern _Float32 asinhf32(_Float32 __x) __attribute__((__nothrow__, __leaf__));
extern _Float32 __asinhf32(_Float32 __x) __attribute__((__nothrow__, __leaf__));
/* Hyperbolic arc tangent of X.  */
extern _Float32 atanhf32(_Float32 __x) __attribute__((__nothrow__, __leaf__));
extern _Float32 __atanhf32(_Float32 __x) __attribute__((__nothrow__, __leaf__));

/* Exponential and logarithmic functions.  */

/* Exponential function of X.  */
extern _Float32 expf32(_Float32 __x) __attribute__((__nothrow__, __leaf__));
extern _Float32 __expf32(_Float32 __x) __attribute__((__nothrow__, __leaf__));

/* Break VALUE into a normalized fraction and an integral power of 2.  */
extern _Float32 frexpf32(_Float32 __x, int *__exponent) __attribute__((__nothrow__, __leaf__));
extern _Float32 __frexpf32(_Float32 __x, int *__exponent) __attribute__((__nothrow__, __leaf__));

/* X times (two to the EXP power).  */
extern _Float32 ldexpf32(_Float32 __x, int __exponent) __attribute__((__nothrow__, __leaf__));
extern _Float32 __ldexpf32(_Float32 __x, int __exponent) __attribute__((__nothrow__, __leaf__));

/* Natural logarithm of X.  */
extern _Float32 logf32(_Float32 __x) __attribute__((__nothrow__, __leaf__));
extern _Float32 __logf32(_Float32 __x) __attribute__((__nothrow__, __leaf__));

/* Base-ten logarithm of X.  */
extern _Float32 log10f32(_Float32 __x) __attribute__((__nothrow__, __leaf__));
extern _Float32 __log10f32(_Float32 __x) __attribute__((__nothrow__, __leaf__));

/* Break VALUE into integral and fractional parts.  */
extern _Float32 modff32(_Float32 __x, _Float32 *__iptr) __attribute__((__nothrow__, __leaf__));
extern _Float32 __modff32(_Float32 __x, _Float32 *__iptr) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(2)));

/* Compute exponent to base ten.  */
extern _Float32 exp10f32(_Float32 __x) __attribute__((__nothrow__, __leaf__));
extern _Float32 __exp10f32(_Float32 __x) __attribute__((__nothrow__, __leaf__));

/* Return exp(X) - 1.  */
extern _Float32 expm1f32(_Float32 __x) __attribute__((__nothrow__, __leaf__));
extern _Float32 __expm1f32(_Float32 __x) __attribute__((__nothrow__, __leaf__));

/* Return log(1 + X).  */
extern _Float32 log1pf32(_Float32 __x) __attribute__((__nothrow__, __leaf__));
extern _Float32 __log1pf32(_Float32 __x) __attribute__((__nothrow__, __leaf__));

/* Return the base 2 signed integral exponent of X.  */
extern _Float32 logbf32(_Float32 __x) __attribute__((__nothrow__, __leaf__));
extern _Float32 __logbf32(_Float32 __x) __attribute__((__nothrow__, __leaf__));

/* Compute base-2 exponential of X.  */
extern _Float32 exp2f32(_Float32 __x) __attribute__((__nothrow__, __leaf__));
extern _Float32 __exp2f32(_Float32 __x) __attribute__((__nothrow__, __leaf__));

/* Compute base-2 logarithm of X.  */
extern _Float32 log2f32(_Float32 __x) __attribute__((__nothrow__, __leaf__));
extern _Float32 __log2f32(_Float32 __x) __attribute__((__nothrow__, __leaf__));

/* Power functions.  */

/* Return X to the Y power.  */
extern _Float32 powf32(_Float32 __x, _Float32 __y) __attribute__((__nothrow__, __leaf__));
extern _Float32 __powf32(_Float32 __x, _Float32 __y) __attribute__((__nothrow__, __leaf__));

/* Return the square root of X.  */
extern _Float32 sqrtf32(_Float32 __x) __attribute__((__nothrow__, __leaf__));
extern _Float32 __sqrtf32(_Float32 __x) __attribute__((__nothrow__, __leaf__));

/* Return `sqrt(X*X + Y*Y)'.  */
extern _Float32 hypotf32(_Float32 __x, _Float32 __y) __attribute__((__nothrow__, __leaf__));
extern _Float32 __hypotf32(_Float32 __x, _Float32 __y) __attribute__((__nothrow__, __leaf__));

/* Return the cube root of X.  */
extern _Float32 cbrtf32(_Float32 __x) __attribute__((__nothrow__, __leaf__));
extern _Float32 __cbrtf32(_Float32 __x) __attribute__((__nothrow__, __leaf__));

/* Nearest integer, absolute value, and remainder functions.  */

/* Smallest integral value not less than X.  */
extern _Float32 ceilf32(_Float32 __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float32 __ceilf32(_Float32 __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Absolute value of X.  */
extern _Float32 fabsf32(_Float32 __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float32 __fabsf32(_Float32 __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Largest integer not greater than X.  */
extern _Float32 floorf32(_Float32 __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float32 __floorf32(_Float32 __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Floating-point modulo remainder of X/Y.  */
extern _Float32 fmodf32(_Float32 __x, _Float32 __y) __attribute__((__nothrow__, __leaf__));
extern _Float32 __fmodf32(_Float32 __x, _Float32 __y) __attribute__((__nothrow__, __leaf__));
/* Return X with its signed changed to Y's.  */
extern _Float32 copysignf32(_Float32 __x, _Float32 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float32 __copysignf32(_Float32 __x, _Float32 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return representation of qNaN for double type.  */
extern _Float32 nanf32(const char *__tagb) __attribute__((__nothrow__, __leaf__));
extern _Float32 __nanf32(const char *__tagb) __attribute__((__nothrow__, __leaf__));
/* Bessel functions.  */
extern _Float32 j0f32(_Float32) __attribute__((__nothrow__, __leaf__));
extern _Float32 __j0f32(_Float32) __attribute__((__nothrow__, __leaf__));
extern _Float32 j1f32(_Float32) __attribute__((__nothrow__, __leaf__));
extern _Float32 __j1f32(_Float32) __attribute__((__nothrow__, __leaf__));
extern _Float32 jnf32(int, _Float32) __attribute__((__nothrow__, __leaf__));
extern _Float32 __jnf32(int, _Float32) __attribute__((__nothrow__, __leaf__));
extern _Float32 y0f32(_Float32) __attribute__((__nothrow__, __leaf__));
extern _Float32 __y0f32(_Float32) __attribute__((__nothrow__, __leaf__));
extern _Float32 y1f32(_Float32) __attribute__((__nothrow__, __leaf__));
extern _Float32 __y1f32(_Float32) __attribute__((__nothrow__, __leaf__));
extern _Float32 ynf32(int, _Float32) __attribute__((__nothrow__, __leaf__));
extern _Float32 __ynf32(int, _Float32) __attribute__((__nothrow__, __leaf__));

/* Error and gamma functions.  */
extern _Float32 erff32(_Float32) __attribute__((__nothrow__, __leaf__));
extern _Float32 __erff32(_Float32) __attribute__((__nothrow__, __leaf__));
extern _Float32 erfcf32(_Float32) __attribute__((__nothrow__, __leaf__));
extern _Float32 __erfcf32(_Float32) __attribute__((__nothrow__, __leaf__));
extern _Float32 lgammaf32(_Float32) __attribute__((__nothrow__, __leaf__));
extern _Float32 __lgammaf32(_Float32) __attribute__((__nothrow__, __leaf__));

/* True gamma function.  */
extern _Float32 tgammaf32(_Float32) __attribute__((__nothrow__, __leaf__));
extern _Float32 __tgammaf32(_Float32) __attribute__((__nothrow__, __leaf__));
/* Reentrant version of lgamma.  This function uses the global variable
   `signgam'.  The reentrant version instead takes a pointer and stores
   the value through it.  */
extern _Float32 lgammaf32_r(_Float32, int *__signgamp) __attribute__((__nothrow__, __leaf__));
extern _Float32 __lgammaf32_r(_Float32, int *__signgamp) __attribute__((__nothrow__, __leaf__));

/* Return the integer nearest X in the direction of the
   prevailing rounding mode.  */
extern _Float32 rintf32(_Float32 __x) __attribute__((__nothrow__, __leaf__));
extern _Float32 __rintf32(_Float32 __x) __attribute__((__nothrow__, __leaf__));

/* Return X + epsilon if X < Y, X - epsilon if X > Y.  */
extern _Float32 nextafterf32(_Float32 __x, _Float32 __y) __attribute__((__nothrow__, __leaf__));
extern _Float32 __nextafterf32(_Float32 __x, _Float32 __y) __attribute__((__nothrow__, __leaf__));

/* Return X - epsilon.  */
extern _Float32 nextdownf32(_Float32 __x) __attribute__((__nothrow__, __leaf__));
extern _Float32 __nextdownf32(_Float32 __x) __attribute__((__nothrow__, __leaf__));
/* Return X + epsilon.  */
extern _Float32 nextupf32(_Float32 __x) __attribute__((__nothrow__, __leaf__));
extern _Float32 __nextupf32(_Float32 __x) __attribute__((__nothrow__, __leaf__));

/* Return the remainder of integer division X / Y with infinite precision.  */
extern _Float32 remainderf32(_Float32 __x, _Float32 __y) __attribute__((__nothrow__, __leaf__));
extern _Float32 __remainderf32(_Float32 __x, _Float32 __y) __attribute__((__nothrow__, __leaf__));

/* Return X times (2 to the Nth power).  */
extern _Float32 scalbnf32(_Float32 __x, int __n) __attribute__((__nothrow__, __leaf__));
extern _Float32 __scalbnf32(_Float32 __x, int __n) __attribute__((__nothrow__, __leaf__));

/* Return the binary exponent of X, which must be nonzero.  */
extern int ilogbf32(_Float32 __x) __attribute__((__nothrow__, __leaf__));
extern int __ilogbf32(_Float32 __x) __attribute__((__nothrow__, __leaf__));

/* Like ilogb, but returning long int.  */
extern long int llogbf32(_Float32 __x) __attribute__((__nothrow__, __leaf__));
extern long int __llogbf32(_Float32 __x) __attribute__((__nothrow__, __leaf__));

/* Return X times (2 to the Nth power).  */
extern _Float32 scalblnf32(_Float32 __x, long int __n) __attribute__((__nothrow__, __leaf__));
extern _Float32 __scalblnf32(_Float32 __x, long int __n) __attribute__((__nothrow__, __leaf__));

/* Round X to integral value in floating-point format using current
   rounding direction, but do not raise inexact exception.  */
extern _Float32 nearbyintf32(_Float32 __x) __attribute__((__nothrow__, __leaf__));
extern _Float32 __nearbyintf32(_Float32 __x) __attribute__((__nothrow__, __leaf__));

/* Round X to nearest integral value, rounding halfway cases away from
   zero.  */
extern _Float32 roundf32(_Float32 __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float32 __roundf32(_Float32 __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Round X to the integral value in floating-point format nearest but
   not larger in magnitude.  */
extern _Float32 truncf32(_Float32 __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float32 __truncf32(_Float32 __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Compute remainder of X and Y and put in *QUO a value with sign of x/y
   and magnitude congruent `mod 2^n' to the magnitude of the integral
   quotient x/y, with n >= 3.  */
extern _Float32 remquof32(_Float32 __x, _Float32 __y, int *__quo) __attribute__((__nothrow__, __leaf__));
extern _Float32 __remquof32(_Float32 __x, _Float32 __y, int *__quo) __attribute__((__nothrow__, __leaf__));

/* Conversion functions.  */

/* Round X to nearest integral value according to current rounding
   direction.  */
extern long int lrintf32(_Float32 __x) __attribute__((__nothrow__, __leaf__));
extern long int __lrintf32(_Float32 __x) __attribute__((__nothrow__, __leaf__));
__extension__ extern long long int llrintf32(_Float32 __x) __attribute__((__nothrow__, __leaf__));
extern long long int __llrintf32(_Float32 __x) __attribute__((__nothrow__, __leaf__));

/* Round X to nearest integral value, rounding halfway cases away from
   zero.  */
extern long int lroundf32(_Float32 __x) __attribute__((__nothrow__, __leaf__));
extern long int __lroundf32(_Float32 __x) __attribute__((__nothrow__, __leaf__));
__extension__ extern long long int llroundf32(_Float32 __x) __attribute__((__nothrow__, __leaf__));
extern long long int __llroundf32(_Float32 __x) __attribute__((__nothrow__, __leaf__));

/* Return positive difference between X and Y.  */
extern _Float32 fdimf32(_Float32 __x, _Float32 __y) __attribute__((__nothrow__, __leaf__));
extern _Float32 __fdimf32(_Float32 __x, _Float32 __y) __attribute__((__nothrow__, __leaf__));

/* Return maximum numeric value from X and Y.  */
extern _Float32 fmaxf32(_Float32 __x, _Float32 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float32 __fmaxf32(_Float32 __x, _Float32 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return minimum numeric value from X and Y.  */
extern _Float32 fminf32(_Float32 __x, _Float32 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float32 __fminf32(_Float32 __x, _Float32 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Multiply-add function computed as a ternary operation.  */
extern _Float32 fmaf32(_Float32 __x, _Float32 __y, _Float32 __z) __attribute__((__nothrow__, __leaf__));
extern _Float32 __fmaf32(_Float32 __x, _Float32 __y, _Float32 __z) __attribute__((__nothrow__, __leaf__));

/* Round X to nearest integer value, rounding halfway cases to even.  */
extern _Float32 roundevenf32(_Float32 __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float32 __roundevenf32(_Float32 __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Round X to nearest signed integer value, not raising inexact, with
   control of rounding direction and width of result.  */
extern __intmax_t fromfpf32(_Float32 __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));
extern __intmax_t __fromfpf32(_Float32 __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));

/* Round X to nearest unsigned integer value, not raising inexact,
   with control of rounding direction and width of result.  */
extern __uintmax_t ufromfpf32(_Float32 __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));
extern __uintmax_t __ufromfpf32(_Float32 __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));

/* Round X to nearest signed integer value, raising inexact for
   non-integers, with control of rounding direction and width of
   result.  */
extern __intmax_t fromfpxf32(_Float32 __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));
extern __intmax_t __fromfpxf32(_Float32 __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));

/* Round X to nearest unsigned integer value, raising inexact for
   non-integers, with control of rounding direction and width of
   result.  */
extern __uintmax_t ufromfpxf32(_Float32 __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));
extern __uintmax_t __ufromfpxf32(_Float32 __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));

/* Canonicalize floating-point representation.  */
extern int canonicalizef32(_Float32 *__cx, const _Float32 *__x) __attribute__((__nothrow__, __leaf__));

/* Return value with maximum magnitude.  */
extern _Float32 fmaxmagf32(_Float32 __x, _Float32 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float32 __fmaxmagf32(_Float32 __x, _Float32 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return value with minimum magnitude.  */
extern _Float32 fminmagf32(_Float32 __x, _Float32 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float32 __fminmagf32(_Float32 __x, _Float32 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return maximum value from X and Y.  */
extern _Float32 fmaximumf32(_Float32 __x, _Float32 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float32 __fmaximumf32(_Float32 __x, _Float32 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return minimum value from X and Y.  */
extern _Float32 fminimumf32(_Float32 __x, _Float32 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float32 __fminimumf32(_Float32 __x, _Float32 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return maximum numeric value from X and Y.  */
extern _Float32 fmaximum_numf32(_Float32 __x, _Float32 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float32 __fmaximum_numf32(_Float32 __x, _Float32 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return minimum numeric value from X and Y.  */
extern _Float32 fminimum_numf32(_Float32 __x, _Float32 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float32 __fminimum_numf32(_Float32 __x, _Float32 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return value with maximum magnitude.  */
extern _Float32 fmaximum_magf32(_Float32 __x, _Float32 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float32 __fmaximum_magf32(_Float32 __x, _Float32 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return value with minimum magnitude.  */
extern _Float32 fminimum_magf32(_Float32 __x, _Float32 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float32 __fminimum_magf32(_Float32 __x, _Float32 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return numeric value with maximum magnitude.  */
extern _Float32 fmaximum_mag_numf32(_Float32 __x, _Float32 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float32 __fmaximum_mag_numf32(_Float32 __x, _Float32 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return numeric value with minimum magnitude.  */
extern _Float32 fminimum_mag_numf32(_Float32 __x, _Float32 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float32 __fminimum_mag_numf32(_Float32 __x, _Float32 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Total order operation.  */
extern int totalorderf32(const _Float32 *__x, const _Float32 *__y) __attribute__((__nothrow__, __leaf__))

__attribute__((__pure__));

/* Total order operation on absolute values.  */
extern int totalordermagf32(const _Float32 *__x, const _Float32 *__y) __attribute__((__nothrow__, __leaf__))

__attribute__((__pure__));

/* Get NaN payload.  */
extern _Float32 getpayloadf32(const _Float32 *__x) __attribute__((__nothrow__, __leaf__));
extern _Float32 __getpayloadf32(const _Float32 *__x) __attribute__((__nothrow__, __leaf__));

/* Set quiet NaN payload.  */
extern int setpayloadf32(_Float32 *__x, _Float32 __payload) __attribute__((__nothrow__, __leaf__));

/* Set signaling NaN payload.  */
extern int setpayloadsigf32(_Float32 *__x, _Float32 __payload) __attribute__((__nothrow__, __leaf__));
/* Prototype declarations for math functions; helper file for <math.h>.
   Copyright (C) 1996-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* NOTE: Because of the special way this file is used by <math.h>, this
   file must NOT be protected from multiple inclusion as header files
   usually are.

   This file provides prototype declarations for the math functions.
   Most functions are declared using the macro:

   __MATHCALL (NAME,[_r], (ARGS...));

   This means there is a function `NAME' returning `double' and a function
   `NAMEf' returning `float'.  Each place `_Mdouble_' appears in the
   prototype, that is actually `double' in the prototype for `NAME' and
   `float' in the prototype for `NAMEf'.  Reentrant variant functions are
   called `NAME_r' and `NAMEf_r'.

   Functions returning other types like `int' are declared using the macro:

   __MATHDECL (TYPE, NAME,[_r], (ARGS...));

   This is just like __MATHCALL but for a function returning `TYPE'
   instead of `_Mdouble_'.  In all of these cases, there is still
   both a `NAME' and a `NAMEf' that takes `float' arguments.

   Note that there must be no whitespace before the argument passed for
   NAME, to make token pasting work with -traditional.  */

/* Trigonometric functions.  */

/* Arc cosine of X.  */
extern _Float64 acosf64(_Float64 __x) __attribute__((__nothrow__, __leaf__));
extern _Float64 __acosf64(_Float64 __x) __attribute__((__nothrow__, __leaf__));
/* Arc sine of X.  */
extern _Float64 asinf64(_Float64 __x) __attribute__((__nothrow__, __leaf__));
extern _Float64 __asinf64(_Float64 __x) __attribute__((__nothrow__, __leaf__));
/* Arc tangent of X.  */
extern _Float64 atanf64(_Float64 __x) __attribute__((__nothrow__, __leaf__));
extern _Float64 __atanf64(_Float64 __x) __attribute__((__nothrow__, __leaf__));
/* Arc tangent of Y/X.  */
extern _Float64 atan2f64(_Float64 __y, _Float64 __x) __attribute__((__nothrow__, __leaf__));
extern _Float64 __atan2f64(_Float64 __y, _Float64 __x) __attribute__((__nothrow__, __leaf__));

/* Cosine of X.  */
extern _Float64 cosf64(_Float64 __x) __attribute__((__nothrow__, __leaf__));
extern _Float64 __cosf64(_Float64 __x) __attribute__((__nothrow__, __leaf__));
/* Sine of X.  */
extern _Float64 sinf64(_Float64 __x) __attribute__((__nothrow__, __leaf__));
extern _Float64 __sinf64(_Float64 __x) __attribute__((__nothrow__, __leaf__));
/* Tangent of X.  */
extern _Float64 tanf64(_Float64 __x) __attribute__((__nothrow__, __leaf__));
extern _Float64 __tanf64(_Float64 __x) __attribute__((__nothrow__, __leaf__));

/* Hyperbolic functions.  */

/* Hyperbolic cosine of X.  */
extern _Float64 coshf64(_Float64 __x) __attribute__((__nothrow__, __leaf__));
extern _Float64 __coshf64(_Float64 __x) __attribute__((__nothrow__, __leaf__));
/* Hyperbolic sine of X.  */
extern _Float64 sinhf64(_Float64 __x) __attribute__((__nothrow__, __leaf__));
extern _Float64 __sinhf64(_Float64 __x) __attribute__((__nothrow__, __leaf__));
/* Hyperbolic tangent of X.  */
extern _Float64 tanhf64(_Float64 __x) __attribute__((__nothrow__, __leaf__));
extern _Float64 __tanhf64(_Float64 __x) __attribute__((__nothrow__, __leaf__));

/* Cosine and sine of X.  */
extern void sincosf64(_Float64 __x, _Float64 *__sinx, _Float64 *__cosx) __attribute__((__nothrow__, __leaf__));
extern void __sincosf64(_Float64 __x, _Float64 *__sinx, _Float64 *__cosx) __attribute__((__nothrow__, __leaf__));

/* Hyperbolic arc cosine of X.  */
extern _Float64 acoshf64(_Float64 __x) __attribute__((__nothrow__, __leaf__));
extern _Float64 __acoshf64(_Float64 __x) __attribute__((__nothrow__, __leaf__));
/* Hyperbolic arc sine of X.  */
extern _Float64 asinhf64(_Float64 __x) __attribute__((__nothrow__, __leaf__));
extern _Float64 __asinhf64(_Float64 __x) __attribute__((__nothrow__, __leaf__));
/* Hyperbolic arc tangent of X.  */
extern _Float64 atanhf64(_Float64 __x) __attribute__((__nothrow__, __leaf__));
extern _Float64 __atanhf64(_Float64 __x) __attribute__((__nothrow__, __leaf__));

/* Exponential and logarithmic functions.  */

/* Exponential function of X.  */
extern _Float64 expf64(_Float64 __x) __attribute__((__nothrow__, __leaf__));
extern _Float64 __expf64(_Float64 __x) __attribute__((__nothrow__, __leaf__));

/* Break VALUE into a normalized fraction and an integral power of 2.  */
extern _Float64 frexpf64(_Float64 __x, int *__exponent) __attribute__((__nothrow__, __leaf__));
extern _Float64 __frexpf64(_Float64 __x, int *__exponent) __attribute__((__nothrow__, __leaf__));

/* X times (two to the EXP power).  */
extern _Float64 ldexpf64(_Float64 __x, int __exponent) __attribute__((__nothrow__, __leaf__));
extern _Float64 __ldexpf64(_Float64 __x, int __exponent) __attribute__((__nothrow__, __leaf__));

/* Natural logarithm of X.  */
extern _Float64 logf64(_Float64 __x) __attribute__((__nothrow__, __leaf__));
extern _Float64 __logf64(_Float64 __x) __attribute__((__nothrow__, __leaf__));

/* Base-ten logarithm of X.  */
extern _Float64 log10f64(_Float64 __x) __attribute__((__nothrow__, __leaf__));
extern _Float64 __log10f64(_Float64 __x) __attribute__((__nothrow__, __leaf__));

/* Break VALUE into integral and fractional parts.  */
extern _Float64 modff64(_Float64 __x, _Float64 *__iptr) __attribute__((__nothrow__, __leaf__));
extern _Float64 __modff64(_Float64 __x, _Float64 *__iptr) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(2)));

/* Compute exponent to base ten.  */
extern _Float64 exp10f64(_Float64 __x) __attribute__((__nothrow__, __leaf__));
extern _Float64 __exp10f64(_Float64 __x) __attribute__((__nothrow__, __leaf__));

/* Return exp(X) - 1.  */
extern _Float64 expm1f64(_Float64 __x) __attribute__((__nothrow__, __leaf__));
extern _Float64 __expm1f64(_Float64 __x) __attribute__((__nothrow__, __leaf__));

/* Return log(1 + X).  */
extern _Float64 log1pf64(_Float64 __x) __attribute__((__nothrow__, __leaf__));
extern _Float64 __log1pf64(_Float64 __x) __attribute__((__nothrow__, __leaf__));

/* Return the base 2 signed integral exponent of X.  */
extern _Float64 logbf64(_Float64 __x) __attribute__((__nothrow__, __leaf__));
extern _Float64 __logbf64(_Float64 __x) __attribute__((__nothrow__, __leaf__));

/* Compute base-2 exponential of X.  */
extern _Float64 exp2f64(_Float64 __x) __attribute__((__nothrow__, __leaf__));
extern _Float64 __exp2f64(_Float64 __x) __attribute__((__nothrow__, __leaf__));

/* Compute base-2 logarithm of X.  */
extern _Float64 log2f64(_Float64 __x) __attribute__((__nothrow__, __leaf__));
extern _Float64 __log2f64(_Float64 __x) __attribute__((__nothrow__, __leaf__));

/* Power functions.  */

/* Return X to the Y power.  */
extern _Float64 powf64(_Float64 __x, _Float64 __y) __attribute__((__nothrow__, __leaf__));
extern _Float64 __powf64(_Float64 __x, _Float64 __y) __attribute__((__nothrow__, __leaf__));

/* Return the square root of X.  */
extern _Float64 sqrtf64(_Float64 __x) __attribute__((__nothrow__, __leaf__));
extern _Float64 __sqrtf64(_Float64 __x) __attribute__((__nothrow__, __leaf__));

/* Return `sqrt(X*X + Y*Y)'.  */
extern _Float64 hypotf64(_Float64 __x, _Float64 __y) __attribute__((__nothrow__, __leaf__));
extern _Float64 __hypotf64(_Float64 __x, _Float64 __y) __attribute__((__nothrow__, __leaf__));

/* Return the cube root of X.  */
extern _Float64 cbrtf64(_Float64 __x) __attribute__((__nothrow__, __leaf__));
extern _Float64 __cbrtf64(_Float64 __x) __attribute__((__nothrow__, __leaf__));

/* Nearest integer, absolute value, and remainder functions.  */

/* Smallest integral value not less than X.  */
extern _Float64 ceilf64(_Float64 __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float64 __ceilf64(_Float64 __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Absolute value of X.  */
extern _Float64 fabsf64(_Float64 __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float64 __fabsf64(_Float64 __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Largest integer not greater than X.  */
extern _Float64 floorf64(_Float64 __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float64 __floorf64(_Float64 __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Floating-point modulo remainder of X/Y.  */
extern _Float64 fmodf64(_Float64 __x, _Float64 __y) __attribute__((__nothrow__, __leaf__));
extern _Float64 __fmodf64(_Float64 __x, _Float64 __y) __attribute__((__nothrow__, __leaf__));
/* Return X with its signed changed to Y's.  */
extern _Float64 copysignf64(_Float64 __x, _Float64 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float64 __copysignf64(_Float64 __x, _Float64 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return representation of qNaN for double type.  */
extern _Float64 nanf64(const char *__tagb) __attribute__((__nothrow__, __leaf__));
extern _Float64 __nanf64(const char *__tagb) __attribute__((__nothrow__, __leaf__));
/* Bessel functions.  */
extern _Float64 j0f64(_Float64) __attribute__((__nothrow__, __leaf__));
extern _Float64 __j0f64(_Float64) __attribute__((__nothrow__, __leaf__));
extern _Float64 j1f64(_Float64) __attribute__((__nothrow__, __leaf__));
extern _Float64 __j1f64(_Float64) __attribute__((__nothrow__, __leaf__));
extern _Float64 jnf64(int, _Float64) __attribute__((__nothrow__, __leaf__));
extern _Float64 __jnf64(int, _Float64) __attribute__((__nothrow__, __leaf__));
extern _Float64 y0f64(_Float64) __attribute__((__nothrow__, __leaf__));
extern _Float64 __y0f64(_Float64) __attribute__((__nothrow__, __leaf__));
extern _Float64 y1f64(_Float64) __attribute__((__nothrow__, __leaf__));
extern _Float64 __y1f64(_Float64) __attribute__((__nothrow__, __leaf__));
extern _Float64 ynf64(int, _Float64) __attribute__((__nothrow__, __leaf__));
extern _Float64 __ynf64(int, _Float64) __attribute__((__nothrow__, __leaf__));

/* Error and gamma functions.  */
extern _Float64 erff64(_Float64) __attribute__((__nothrow__, __leaf__));
extern _Float64 __erff64(_Float64) __attribute__((__nothrow__, __leaf__));
extern _Float64 erfcf64(_Float64) __attribute__((__nothrow__, __leaf__));
extern _Float64 __erfcf64(_Float64) __attribute__((__nothrow__, __leaf__));
extern _Float64 lgammaf64(_Float64) __attribute__((__nothrow__, __leaf__));
extern _Float64 __lgammaf64(_Float64) __attribute__((__nothrow__, __leaf__));

/* True gamma function.  */
extern _Float64 tgammaf64(_Float64) __attribute__((__nothrow__, __leaf__));
extern _Float64 __tgammaf64(_Float64) __attribute__((__nothrow__, __leaf__));
/* Reentrant version of lgamma.  This function uses the global variable
   `signgam'.  The reentrant version instead takes a pointer and stores
   the value through it.  */
extern _Float64 lgammaf64_r(_Float64, int *__signgamp) __attribute__((__nothrow__, __leaf__));
extern _Float64 __lgammaf64_r(_Float64, int *__signgamp) __attribute__((__nothrow__, __leaf__));

/* Return the integer nearest X in the direction of the
   prevailing rounding mode.  */
extern _Float64 rintf64(_Float64 __x) __attribute__((__nothrow__, __leaf__));
extern _Float64 __rintf64(_Float64 __x) __attribute__((__nothrow__, __leaf__));

/* Return X + epsilon if X < Y, X - epsilon if X > Y.  */
extern _Float64 nextafterf64(_Float64 __x, _Float64 __y) __attribute__((__nothrow__, __leaf__));
extern _Float64 __nextafterf64(_Float64 __x, _Float64 __y) __attribute__((__nothrow__, __leaf__));

/* Return X - epsilon.  */
extern _Float64 nextdownf64(_Float64 __x) __attribute__((__nothrow__, __leaf__));
extern _Float64 __nextdownf64(_Float64 __x) __attribute__((__nothrow__, __leaf__));
/* Return X + epsilon.  */
extern _Float64 nextupf64(_Float64 __x) __attribute__((__nothrow__, __leaf__));
extern _Float64 __nextupf64(_Float64 __x) __attribute__((__nothrow__, __leaf__));

/* Return the remainder of integer division X / Y with infinite precision.  */
extern _Float64 remainderf64(_Float64 __x, _Float64 __y) __attribute__((__nothrow__, __leaf__));
extern _Float64 __remainderf64(_Float64 __x, _Float64 __y) __attribute__((__nothrow__, __leaf__));

/* Return X times (2 to the Nth power).  */
extern _Float64 scalbnf64(_Float64 __x, int __n) __attribute__((__nothrow__, __leaf__));
extern _Float64 __scalbnf64(_Float64 __x, int __n) __attribute__((__nothrow__, __leaf__));

/* Return the binary exponent of X, which must be nonzero.  */
extern int ilogbf64(_Float64 __x) __attribute__((__nothrow__, __leaf__));
extern int __ilogbf64(_Float64 __x) __attribute__((__nothrow__, __leaf__));

/* Like ilogb, but returning long int.  */
extern long int llogbf64(_Float64 __x) __attribute__((__nothrow__, __leaf__));
extern long int __llogbf64(_Float64 __x) __attribute__((__nothrow__, __leaf__));

/* Return X times (2 to the Nth power).  */
extern _Float64 scalblnf64(_Float64 __x, long int __n) __attribute__((__nothrow__, __leaf__));
extern _Float64 __scalblnf64(_Float64 __x, long int __n) __attribute__((__nothrow__, __leaf__));

/* Round X to integral value in floating-point format using current
   rounding direction, but do not raise inexact exception.  */
extern _Float64 nearbyintf64(_Float64 __x) __attribute__((__nothrow__, __leaf__));
extern _Float64 __nearbyintf64(_Float64 __x) __attribute__((__nothrow__, __leaf__));

/* Round X to nearest integral value, rounding halfway cases away from
   zero.  */
extern _Float64 roundf64(_Float64 __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float64 __roundf64(_Float64 __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Round X to the integral value in floating-point format nearest but
   not larger in magnitude.  */
extern _Float64 truncf64(_Float64 __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float64 __truncf64(_Float64 __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Compute remainder of X and Y and put in *QUO a value with sign of x/y
   and magnitude congruent `mod 2^n' to the magnitude of the integral
   quotient x/y, with n >= 3.  */
extern _Float64 remquof64(_Float64 __x, _Float64 __y, int *__quo) __attribute__((__nothrow__, __leaf__));
extern _Float64 __remquof64(_Float64 __x, _Float64 __y, int *__quo) __attribute__((__nothrow__, __leaf__));

/* Conversion functions.  */

/* Round X to nearest integral value according to current rounding
   direction.  */
extern long int lrintf64(_Float64 __x) __attribute__((__nothrow__, __leaf__));
extern long int __lrintf64(_Float64 __x) __attribute__((__nothrow__, __leaf__));
__extension__ extern long long int llrintf64(_Float64 __x) __attribute__((__nothrow__, __leaf__));
extern long long int __llrintf64(_Float64 __x) __attribute__((__nothrow__, __leaf__));

/* Round X to nearest integral value, rounding halfway cases away from
   zero.  */
extern long int lroundf64(_Float64 __x) __attribute__((__nothrow__, __leaf__));
extern long int __lroundf64(_Float64 __x) __attribute__((__nothrow__, __leaf__));
__extension__ extern long long int llroundf64(_Float64 __x) __attribute__((__nothrow__, __leaf__));
extern long long int __llroundf64(_Float64 __x) __attribute__((__nothrow__, __leaf__));

/* Return positive difference between X and Y.  */
extern _Float64 fdimf64(_Float64 __x, _Float64 __y) __attribute__((__nothrow__, __leaf__));
extern _Float64 __fdimf64(_Float64 __x, _Float64 __y) __attribute__((__nothrow__, __leaf__));

/* Return maximum numeric value from X and Y.  */
extern _Float64 fmaxf64(_Float64 __x, _Float64 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float64 __fmaxf64(_Float64 __x, _Float64 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return minimum numeric value from X and Y.  */
extern _Float64 fminf64(_Float64 __x, _Float64 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float64 __fminf64(_Float64 __x, _Float64 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Multiply-add function computed as a ternary operation.  */
extern _Float64 fmaf64(_Float64 __x, _Float64 __y, _Float64 __z) __attribute__((__nothrow__, __leaf__));
extern _Float64 __fmaf64(_Float64 __x, _Float64 __y, _Float64 __z) __attribute__((__nothrow__, __leaf__));

/* Round X to nearest integer value, rounding halfway cases to even.  */
extern _Float64 roundevenf64(_Float64 __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float64 __roundevenf64(_Float64 __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Round X to nearest signed integer value, not raising inexact, with
   control of rounding direction and width of result.  */
extern __intmax_t fromfpf64(_Float64 __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));
extern __intmax_t __fromfpf64(_Float64 __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));

/* Round X to nearest unsigned integer value, not raising inexact,
   with control of rounding direction and width of result.  */
extern __uintmax_t ufromfpf64(_Float64 __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));
extern __uintmax_t __ufromfpf64(_Float64 __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));

/* Round X to nearest signed integer value, raising inexact for
   non-integers, with control of rounding direction and width of
   result.  */
extern __intmax_t fromfpxf64(_Float64 __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));
extern __intmax_t __fromfpxf64(_Float64 __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));

/* Round X to nearest unsigned integer value, raising inexact for
   non-integers, with control of rounding direction and width of
   result.  */
extern __uintmax_t ufromfpxf64(_Float64 __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));
extern __uintmax_t __ufromfpxf64(_Float64 __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));

/* Canonicalize floating-point representation.  */
extern int canonicalizef64(_Float64 *__cx, const _Float64 *__x) __attribute__((__nothrow__, __leaf__));

/* Return value with maximum magnitude.  */
extern _Float64 fmaxmagf64(_Float64 __x, _Float64 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float64 __fmaxmagf64(_Float64 __x, _Float64 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return value with minimum magnitude.  */
extern _Float64 fminmagf64(_Float64 __x, _Float64 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float64 __fminmagf64(_Float64 __x, _Float64 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return maximum value from X and Y.  */
extern _Float64 fmaximumf64(_Float64 __x, _Float64 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float64 __fmaximumf64(_Float64 __x, _Float64 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return minimum value from X and Y.  */
extern _Float64 fminimumf64(_Float64 __x, _Float64 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float64 __fminimumf64(_Float64 __x, _Float64 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return maximum numeric value from X and Y.  */
extern _Float64 fmaximum_numf64(_Float64 __x, _Float64 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float64 __fmaximum_numf64(_Float64 __x, _Float64 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return minimum numeric value from X and Y.  */
extern _Float64 fminimum_numf64(_Float64 __x, _Float64 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float64 __fminimum_numf64(_Float64 __x, _Float64 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return value with maximum magnitude.  */
extern _Float64 fmaximum_magf64(_Float64 __x, _Float64 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float64 __fmaximum_magf64(_Float64 __x, _Float64 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return value with minimum magnitude.  */
extern _Float64 fminimum_magf64(_Float64 __x, _Float64 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float64 __fminimum_magf64(_Float64 __x, _Float64 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return numeric value with maximum magnitude.  */
extern _Float64 fmaximum_mag_numf64(_Float64 __x, _Float64 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float64 __fmaximum_mag_numf64(_Float64 __x, _Float64 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return numeric value with minimum magnitude.  */
extern _Float64 fminimum_mag_numf64(_Float64 __x, _Float64 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float64 __fminimum_mag_numf64(_Float64 __x, _Float64 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Total order operation.  */
extern int totalorderf64(const _Float64 *__x, const _Float64 *__y) __attribute__((__nothrow__, __leaf__))

__attribute__((__pure__));

/* Total order operation on absolute values.  */
extern int totalordermagf64(const _Float64 *__x, const _Float64 *__y) __attribute__((__nothrow__, __leaf__))

__attribute__((__pure__));

/* Get NaN payload.  */
extern _Float64 getpayloadf64(const _Float64 *__x) __attribute__((__nothrow__, __leaf__));
extern _Float64 __getpayloadf64(const _Float64 *__x) __attribute__((__nothrow__, __leaf__));

/* Set quiet NaN payload.  */
extern int setpayloadf64(_Float64 *__x, _Float64 __payload) __attribute__((__nothrow__, __leaf__));

/* Set signaling NaN payload.  */
extern int setpayloadsigf64(_Float64 *__x, _Float64 __payload) __attribute__((__nothrow__, __leaf__));
/* Prototype declarations for math classification macros helpers.
   Copyright (C) 2017-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* Classify given number.  */
extern int __fpclassifyf128(_Float128 __value) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Test for negative number.  */
extern int __signbitf128(_Float128 __value) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return 0 if VALUE is finite or NaN, +1 if it
   is +Infinity, -1 if it is -Infinity.  */
extern int __isinff128(_Float128 __value) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return nonzero if VALUE is finite and not NaN.  Used by isfinite macro.  */
extern int __finitef128(_Float128 __value) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return nonzero if VALUE is not a number.  */
extern int __isnanf128(_Float128 __value) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Test equality.  */
extern int __iseqsigf128(_Float128 __x, _Float128 __y) __attribute__((__nothrow__, __leaf__));

/* Test for signaling NaN.  */
extern int __issignalingf128(_Float128 __value) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Prototype declarations for math functions; helper file for <math.h>.
   Copyright (C) 1996-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* NOTE: Because of the special way this file is used by <math.h>, this
   file must NOT be protected from multiple inclusion as header files
   usually are.

   This file provides prototype declarations for the math functions.
   Most functions are declared using the macro:

   __MATHCALL (NAME,[_r], (ARGS...));

   This means there is a function `NAME' returning `double' and a function
   `NAMEf' returning `float'.  Each place `_Mdouble_' appears in the
   prototype, that is actually `double' in the prototype for `NAME' and
   `float' in the prototype for `NAMEf'.  Reentrant variant functions are
   called `NAME_r' and `NAMEf_r'.

   Functions returning other types like `int' are declared using the macro:

   __MATHDECL (TYPE, NAME,[_r], (ARGS...));

   This is just like __MATHCALL but for a function returning `TYPE'
   instead of `_Mdouble_'.  In all of these cases, there is still
   both a `NAME' and a `NAMEf' that takes `float' arguments.

   Note that there must be no whitespace before the argument passed for
   NAME, to make token pasting work with -traditional.  */

/* Trigonometric functions.  */

/* Arc cosine of X.  */
extern _Float128 acosf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));
extern _Float128 __acosf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));
/* Arc sine of X.  */
extern _Float128 asinf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));
extern _Float128 __asinf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));
/* Arc tangent of X.  */
extern _Float128 atanf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));
extern _Float128 __atanf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));
/* Arc tangent of Y/X.  */
extern _Float128 atan2f128(_Float128 __y, _Float128 __x) __attribute__((__nothrow__, __leaf__));
extern _Float128 __atan2f128(_Float128 __y, _Float128 __x) __attribute__((__nothrow__, __leaf__));

/* Cosine of X.  */
extern _Float128 cosf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));
extern _Float128 __cosf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));
/* Sine of X.  */
extern _Float128 sinf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));
extern _Float128 __sinf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));
/* Tangent of X.  */
extern _Float128 tanf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));
extern _Float128 __tanf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));

/* Hyperbolic functions.  */

/* Hyperbolic cosine of X.  */
extern _Float128 coshf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));
extern _Float128 __coshf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));
/* Hyperbolic sine of X.  */
extern _Float128 sinhf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));
extern _Float128 __sinhf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));
/* Hyperbolic tangent of X.  */
extern _Float128 tanhf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));
extern _Float128 __tanhf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));

/* Cosine and sine of X.  */
extern void sincosf128(_Float128 __x, _Float128 *__sinx, _Float128 *__cosx) __attribute__((__nothrow__, __leaf__));
extern void __sincosf128(_Float128 __x, _Float128 *__sinx, _Float128 *__cosx) __attribute__((__nothrow__, __leaf__));

/* Hyperbolic arc cosine of X.  */
extern _Float128 acoshf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));
extern _Float128 __acoshf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));
/* Hyperbolic arc sine of X.  */
extern _Float128 asinhf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));
extern _Float128 __asinhf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));
/* Hyperbolic arc tangent of X.  */
extern _Float128 atanhf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));
extern _Float128 __atanhf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));

/* Exponential and logarithmic functions.  */

/* Exponential function of X.  */
extern _Float128 expf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));
extern _Float128 __expf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));

/* Break VALUE into a normalized fraction and an integral power of 2.  */
extern _Float128 frexpf128(_Float128 __x, int *__exponent) __attribute__((__nothrow__, __leaf__));
extern _Float128 __frexpf128(_Float128 __x, int *__exponent) __attribute__((__nothrow__, __leaf__));

/* X times (two to the EXP power).  */
extern _Float128 ldexpf128(_Float128 __x, int __exponent) __attribute__((__nothrow__, __leaf__));
extern _Float128 __ldexpf128(_Float128 __x, int __exponent) __attribute__((__nothrow__, __leaf__));

/* Natural logarithm of X.  */
extern _Float128 logf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));
extern _Float128 __logf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));

/* Base-ten logarithm of X.  */
extern _Float128 log10f128(_Float128 __x) __attribute__((__nothrow__, __leaf__));
extern _Float128 __log10f128(_Float128 __x) __attribute__((__nothrow__, __leaf__));

/* Break VALUE into integral and fractional parts.  */
extern _Float128 modff128(_Float128 __x, _Float128 *__iptr) __attribute__((__nothrow__, __leaf__));
extern _Float128 __modff128(_Float128 __x, _Float128 *__iptr) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(2)));

/* Compute exponent to base ten.  */
extern _Float128 exp10f128(_Float128 __x) __attribute__((__nothrow__, __leaf__));
extern _Float128 __exp10f128(_Float128 __x) __attribute__((__nothrow__, __leaf__));

/* Return exp(X) - 1.  */
extern _Float128 expm1f128(_Float128 __x) __attribute__((__nothrow__, __leaf__));
extern _Float128 __expm1f128(_Float128 __x) __attribute__((__nothrow__, __leaf__));

/* Return log(1 + X).  */
extern _Float128 log1pf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));
extern _Float128 __log1pf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));

/* Return the base 2 signed integral exponent of X.  */
extern _Float128 logbf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));
extern _Float128 __logbf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));

/* Compute base-2 exponential of X.  */
extern _Float128 exp2f128(_Float128 __x) __attribute__((__nothrow__, __leaf__));
extern _Float128 __exp2f128(_Float128 __x) __attribute__((__nothrow__, __leaf__));

/* Compute base-2 logarithm of X.  */
extern _Float128 log2f128(_Float128 __x) __attribute__((__nothrow__, __leaf__));
extern _Float128 __log2f128(_Float128 __x) __attribute__((__nothrow__, __leaf__));

/* Power functions.  */

/* Return X to the Y power.  */
extern _Float128 powf128(_Float128 __x, _Float128 __y) __attribute__((__nothrow__, __leaf__));
extern _Float128 __powf128(_Float128 __x, _Float128 __y) __attribute__((__nothrow__, __leaf__));

/* Return the square root of X.  */
extern _Float128 sqrtf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));
extern _Float128 __sqrtf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));

/* Return `sqrt(X*X + Y*Y)'.  */
extern _Float128 hypotf128(_Float128 __x, _Float128 __y) __attribute__((__nothrow__, __leaf__));
extern _Float128 __hypotf128(_Float128 __x, _Float128 __y) __attribute__((__nothrow__, __leaf__));

/* Return the cube root of X.  */
extern _Float128 cbrtf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));
extern _Float128 __cbrtf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));

/* Nearest integer, absolute value, and remainder functions.  */

/* Smallest integral value not less than X.  */
extern _Float128 ceilf128(_Float128 __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float128 __ceilf128(_Float128 __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Absolute value of X.  */
extern _Float128 fabsf128(_Float128 __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float128 __fabsf128(_Float128 __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Largest integer not greater than X.  */
extern _Float128 floorf128(_Float128 __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float128 __floorf128(_Float128 __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Floating-point modulo remainder of X/Y.  */
extern _Float128 fmodf128(_Float128 __x, _Float128 __y) __attribute__((__nothrow__, __leaf__));
extern _Float128 __fmodf128(_Float128 __x, _Float128 __y) __attribute__((__nothrow__, __leaf__));
/* Return X with its signed changed to Y's.  */
extern _Float128 copysignf128(_Float128 __x, _Float128 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float128 __copysignf128(_Float128 __x, _Float128 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return representation of qNaN for double type.  */
extern _Float128 nanf128(const char *__tagb) __attribute__((__nothrow__, __leaf__));
extern _Float128 __nanf128(const char *__tagb) __attribute__((__nothrow__, __leaf__));
/* Bessel functions.  */
extern _Float128 j0f128(_Float128) __attribute__((__nothrow__, __leaf__));
extern _Float128 __j0f128(_Float128) __attribute__((__nothrow__, __leaf__));
extern _Float128 j1f128(_Float128) __attribute__((__nothrow__, __leaf__));
extern _Float128 __j1f128(_Float128) __attribute__((__nothrow__, __leaf__));
extern _Float128 jnf128(int, _Float128) __attribute__((__nothrow__, __leaf__));
extern _Float128 __jnf128(int, _Float128) __attribute__((__nothrow__, __leaf__));
extern _Float128 y0f128(_Float128) __attribute__((__nothrow__, __leaf__));
extern _Float128 __y0f128(_Float128) __attribute__((__nothrow__, __leaf__));
extern _Float128 y1f128(_Float128) __attribute__((__nothrow__, __leaf__));
extern _Float128 __y1f128(_Float128) __attribute__((__nothrow__, __leaf__));
extern _Float128 ynf128(int, _Float128) __attribute__((__nothrow__, __leaf__));
extern _Float128 __ynf128(int, _Float128) __attribute__((__nothrow__, __leaf__));

/* Error and gamma functions.  */
extern _Float128 erff128(_Float128) __attribute__((__nothrow__, __leaf__));
extern _Float128 __erff128(_Float128) __attribute__((__nothrow__, __leaf__));
extern _Float128 erfcf128(_Float128) __attribute__((__nothrow__, __leaf__));
extern _Float128 __erfcf128(_Float128) __attribute__((__nothrow__, __leaf__));
extern _Float128 lgammaf128(_Float128) __attribute__((__nothrow__, __leaf__));
extern _Float128 __lgammaf128(_Float128) __attribute__((__nothrow__, __leaf__));

/* True gamma function.  */
extern _Float128 tgammaf128(_Float128) __attribute__((__nothrow__, __leaf__));
extern _Float128 __tgammaf128(_Float128) __attribute__((__nothrow__, __leaf__));
/* Reentrant version of lgamma.  This function uses the global variable
   `signgam'.  The reentrant version instead takes a pointer and stores
   the value through it.  */
extern _Float128 lgammaf128_r(_Float128, int *__signgamp) __attribute__((__nothrow__, __leaf__));
extern _Float128 __lgammaf128_r(_Float128, int *__signgamp) __attribute__((__nothrow__, __leaf__));

/* Return the integer nearest X in the direction of the
   prevailing rounding mode.  */
extern _Float128 rintf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));
extern _Float128 __rintf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));

/* Return X + epsilon if X < Y, X - epsilon if X > Y.  */
extern _Float128 nextafterf128(_Float128 __x, _Float128 __y) __attribute__((__nothrow__, __leaf__));
extern _Float128 __nextafterf128(_Float128 __x, _Float128 __y) __attribute__((__nothrow__, __leaf__));

/* Return X - epsilon.  */
extern _Float128 nextdownf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));
extern _Float128 __nextdownf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));
/* Return X + epsilon.  */
extern _Float128 nextupf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));
extern _Float128 __nextupf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));

/* Return the remainder of integer division X / Y with infinite precision.  */
extern _Float128 remainderf128(_Float128 __x, _Float128 __y) __attribute__((__nothrow__, __leaf__));
extern _Float128 __remainderf128(_Float128 __x, _Float128 __y) __attribute__((__nothrow__, __leaf__));

/* Return X times (2 to the Nth power).  */
extern _Float128 scalbnf128(_Float128 __x, int __n) __attribute__((__nothrow__, __leaf__));
extern _Float128 __scalbnf128(_Float128 __x, int __n) __attribute__((__nothrow__, __leaf__));

/* Return the binary exponent of X, which must be nonzero.  */
extern int ilogbf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));
extern int __ilogbf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));

/* Like ilogb, but returning long int.  */
extern long int llogbf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));
extern long int __llogbf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));

/* Return X times (2 to the Nth power).  */
extern _Float128 scalblnf128(_Float128 __x, long int __n) __attribute__((__nothrow__, __leaf__));
extern _Float128 __scalblnf128(_Float128 __x, long int __n) __attribute__((__nothrow__, __leaf__));

/* Round X to integral value in floating-point format using current
   rounding direction, but do not raise inexact exception.  */
extern _Float128 nearbyintf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));
extern _Float128 __nearbyintf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));

/* Round X to nearest integral value, rounding halfway cases away from
   zero.  */
extern _Float128 roundf128(_Float128 __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float128 __roundf128(_Float128 __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Round X to the integral value in floating-point format nearest but
   not larger in magnitude.  */
extern _Float128 truncf128(_Float128 __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float128 __truncf128(_Float128 __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Compute remainder of X and Y and put in *QUO a value with sign of x/y
   and magnitude congruent `mod 2^n' to the magnitude of the integral
   quotient x/y, with n >= 3.  */
extern _Float128 remquof128(_Float128 __x, _Float128 __y, int *__quo) __attribute__((__nothrow__, __leaf__));
extern _Float128 __remquof128(_Float128 __x, _Float128 __y, int *__quo) __attribute__((__nothrow__, __leaf__));

/* Conversion functions.  */

/* Round X to nearest integral value according to current rounding
   direction.  */
extern long int lrintf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));
extern long int __lrintf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));
__extension__ extern long long int llrintf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));
extern long long int __llrintf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));

/* Round X to nearest integral value, rounding halfway cases away from
   zero.  */
extern long int lroundf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));
extern long int __lroundf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));
__extension__ extern long long int llroundf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));
extern long long int __llroundf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));

/* Return positive difference between X and Y.  */
extern _Float128 fdimf128(_Float128 __x, _Float128 __y) __attribute__((__nothrow__, __leaf__));
extern _Float128 __fdimf128(_Float128 __x, _Float128 __y) __attribute__((__nothrow__, __leaf__));

/* Return maximum numeric value from X and Y.  */
extern _Float128 fmaxf128(_Float128 __x, _Float128 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float128 __fmaxf128(_Float128 __x, _Float128 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return minimum numeric value from X and Y.  */
extern _Float128 fminf128(_Float128 __x, _Float128 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float128 __fminf128(_Float128 __x, _Float128 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Multiply-add function computed as a ternary operation.  */
extern _Float128 fmaf128(_Float128 __x, _Float128 __y, _Float128 __z) __attribute__((__nothrow__, __leaf__));
extern _Float128 __fmaf128(_Float128 __x, _Float128 __y, _Float128 __z) __attribute__((__nothrow__, __leaf__));

/* Round X to nearest integer value, rounding halfway cases to even.  */
extern _Float128 roundevenf128(_Float128 __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float128 __roundevenf128(_Float128 __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Round X to nearest signed integer value, not raising inexact, with
   control of rounding direction and width of result.  */
extern __intmax_t fromfpf128(_Float128 __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));
extern __intmax_t __fromfpf128(_Float128 __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));

/* Round X to nearest unsigned integer value, not raising inexact,
   with control of rounding direction and width of result.  */
extern __uintmax_t ufromfpf128(_Float128 __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));
extern __uintmax_t __ufromfpf128(_Float128 __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));

/* Round X to nearest signed integer value, raising inexact for
   non-integers, with control of rounding direction and width of
   result.  */
extern __intmax_t fromfpxf128(_Float128 __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));
extern __intmax_t __fromfpxf128(_Float128 __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));

/* Round X to nearest unsigned integer value, raising inexact for
   non-integers, with control of rounding direction and width of
   result.  */
extern __uintmax_t ufromfpxf128(_Float128 __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));
extern __uintmax_t __ufromfpxf128(_Float128 __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));

/* Canonicalize floating-point representation.  */
extern int canonicalizef128(_Float128 *__cx, const _Float128 *__x) __attribute__((__nothrow__, __leaf__));

/* Return value with maximum magnitude.  */
extern _Float128 fmaxmagf128(_Float128 __x, _Float128 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float128 __fmaxmagf128(_Float128 __x, _Float128 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return value with minimum magnitude.  */
extern _Float128 fminmagf128(_Float128 __x, _Float128 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float128 __fminmagf128(_Float128 __x, _Float128 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return maximum value from X and Y.  */
extern _Float128 fmaximumf128(_Float128 __x, _Float128 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float128 __fmaximumf128(_Float128 __x, _Float128 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return minimum value from X and Y.  */
extern _Float128 fminimumf128(_Float128 __x, _Float128 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float128 __fminimumf128(_Float128 __x, _Float128 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return maximum numeric value from X and Y.  */
extern _Float128 fmaximum_numf128(_Float128 __x, _Float128 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float128 __fmaximum_numf128(_Float128 __x, _Float128 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return minimum numeric value from X and Y.  */
extern _Float128 fminimum_numf128(_Float128 __x, _Float128 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float128 __fminimum_numf128(_Float128 __x, _Float128 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return value with maximum magnitude.  */
extern _Float128 fmaximum_magf128(_Float128 __x, _Float128 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float128 __fmaximum_magf128(_Float128 __x, _Float128 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return value with minimum magnitude.  */
extern _Float128 fminimum_magf128(_Float128 __x, _Float128 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float128 __fminimum_magf128(_Float128 __x, _Float128 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return numeric value with maximum magnitude.  */
extern _Float128 fmaximum_mag_numf128(_Float128 __x, _Float128 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float128 __fmaximum_mag_numf128(_Float128 __x, _Float128 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return numeric value with minimum magnitude.  */
extern _Float128 fminimum_mag_numf128(_Float128 __x, _Float128 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float128 __fminimum_mag_numf128(_Float128 __x, _Float128 __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Total order operation.  */
extern int totalorderf128(const _Float128 *__x, const _Float128 *__y) __attribute__((__nothrow__, __leaf__))

__attribute__((__pure__));

/* Total order operation on absolute values.  */
extern int totalordermagf128(const _Float128 *__x, const _Float128 *__y) __attribute__((__nothrow__, __leaf__))

__attribute__((__pure__));

/* Get NaN payload.  */
extern _Float128 getpayloadf128(const _Float128 *__x) __attribute__((__nothrow__, __leaf__));
extern _Float128 __getpayloadf128(const _Float128 *__x) __attribute__((__nothrow__, __leaf__));

/* Set quiet NaN payload.  */
extern int setpayloadf128(_Float128 *__x, _Float128 __payload) __attribute__((__nothrow__, __leaf__));

/* Set signaling NaN payload.  */
extern int setpayloadsigf128(_Float128 *__x, _Float128 __payload) __attribute__((__nothrow__, __leaf__));
/* Prototype declarations for math functions; helper file for <math.h>.
   Copyright (C) 1996-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* NOTE: Because of the special way this file is used by <math.h>, this
   file must NOT be protected from multiple inclusion as header files
   usually are.

   This file provides prototype declarations for the math functions.
   Most functions are declared using the macro:

   __MATHCALL (NAME,[_r], (ARGS...));

   This means there is a function `NAME' returning `double' and a function
   `NAMEf' returning `float'.  Each place `_Mdouble_' appears in the
   prototype, that is actually `double' in the prototype for `NAME' and
   `float' in the prototype for `NAMEf'.  Reentrant variant functions are
   called `NAME_r' and `NAMEf_r'.

   Functions returning other types like `int' are declared using the macro:

   __MATHDECL (TYPE, NAME,[_r], (ARGS...));

   This is just like __MATHCALL but for a function returning `TYPE'
   instead of `_Mdouble_'.  In all of these cases, there is still
   both a `NAME' and a `NAMEf' that takes `float' arguments.

   Note that there must be no whitespace before the argument passed for
   NAME, to make token pasting work with -traditional.  */

/* Trigonometric functions.  */

/* Arc cosine of X.  */
extern _Float32x acosf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));
extern _Float32x __acosf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));
/* Arc sine of X.  */
extern _Float32x asinf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));
extern _Float32x __asinf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));
/* Arc tangent of X.  */
extern _Float32x atanf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));
extern _Float32x __atanf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));
/* Arc tangent of Y/X.  */
extern _Float32x atan2f32x(_Float32x __y, _Float32x __x) __attribute__((__nothrow__, __leaf__));
extern _Float32x __atan2f32x(_Float32x __y, _Float32x __x) __attribute__((__nothrow__, __leaf__));

/* Cosine of X.  */
extern _Float32x cosf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));
extern _Float32x __cosf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));
/* Sine of X.  */
extern _Float32x sinf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));
extern _Float32x __sinf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));
/* Tangent of X.  */
extern _Float32x tanf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));
extern _Float32x __tanf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));

/* Hyperbolic functions.  */

/* Hyperbolic cosine of X.  */
extern _Float32x coshf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));
extern _Float32x __coshf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));
/* Hyperbolic sine of X.  */
extern _Float32x sinhf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));
extern _Float32x __sinhf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));
/* Hyperbolic tangent of X.  */
extern _Float32x tanhf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));
extern _Float32x __tanhf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));

/* Cosine and sine of X.  */
extern void sincosf32x(_Float32x __x, _Float32x *__sinx, _Float32x *__cosx) __attribute__((__nothrow__, __leaf__));
extern void __sincosf32x(_Float32x __x, _Float32x *__sinx, _Float32x *__cosx) __attribute__((__nothrow__, __leaf__));

/* Hyperbolic arc cosine of X.  */
extern _Float32x acoshf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));
extern _Float32x __acoshf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));
/* Hyperbolic arc sine of X.  */
extern _Float32x asinhf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));
extern _Float32x __asinhf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));
/* Hyperbolic arc tangent of X.  */
extern _Float32x atanhf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));
extern _Float32x __atanhf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));

/* Exponential and logarithmic functions.  */

/* Exponential function of X.  */
extern _Float32x expf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));
extern _Float32x __expf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));

/* Break VALUE into a normalized fraction and an integral power of 2.  */
extern _Float32x frexpf32x(_Float32x __x, int *__exponent) __attribute__((__nothrow__, __leaf__));
extern _Float32x __frexpf32x(_Float32x __x, int *__exponent) __attribute__((__nothrow__, __leaf__));

/* X times (two to the EXP power).  */
extern _Float32x ldexpf32x(_Float32x __x, int __exponent) __attribute__((__nothrow__, __leaf__));
extern _Float32x __ldexpf32x(_Float32x __x, int __exponent) __attribute__((__nothrow__, __leaf__));

/* Natural logarithm of X.  */
extern _Float32x logf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));
extern _Float32x __logf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));

/* Base-ten logarithm of X.  */
extern _Float32x log10f32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));
extern _Float32x __log10f32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));

/* Break VALUE into integral and fractional parts.  */
extern _Float32x modff32x(_Float32x __x, _Float32x *__iptr) __attribute__((__nothrow__, __leaf__));
extern _Float32x __modff32x(_Float32x __x, _Float32x *__iptr) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(2)));

/* Compute exponent to base ten.  */
extern _Float32x exp10f32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));
extern _Float32x __exp10f32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));

/* Return exp(X) - 1.  */
extern _Float32x expm1f32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));
extern _Float32x __expm1f32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));

/* Return log(1 + X).  */
extern _Float32x log1pf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));
extern _Float32x __log1pf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));

/* Return the base 2 signed integral exponent of X.  */
extern _Float32x logbf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));
extern _Float32x __logbf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));

/* Compute base-2 exponential of X.  */
extern _Float32x exp2f32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));
extern _Float32x __exp2f32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));

/* Compute base-2 logarithm of X.  */
extern _Float32x log2f32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));
extern _Float32x __log2f32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));

/* Power functions.  */

/* Return X to the Y power.  */
extern _Float32x powf32x(_Float32x __x, _Float32x __y) __attribute__((__nothrow__, __leaf__));
extern _Float32x __powf32x(_Float32x __x, _Float32x __y) __attribute__((__nothrow__, __leaf__));

/* Return the square root of X.  */
extern _Float32x sqrtf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));
extern _Float32x __sqrtf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));

/* Return `sqrt(X*X + Y*Y)'.  */
extern _Float32x hypotf32x(_Float32x __x, _Float32x __y) __attribute__((__nothrow__, __leaf__));
extern _Float32x __hypotf32x(_Float32x __x, _Float32x __y) __attribute__((__nothrow__, __leaf__));

/* Return the cube root of X.  */
extern _Float32x cbrtf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));
extern _Float32x __cbrtf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));

/* Nearest integer, absolute value, and remainder functions.  */

/* Smallest integral value not less than X.  */
extern _Float32x ceilf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float32x __ceilf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Absolute value of X.  */
extern _Float32x fabsf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float32x __fabsf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Largest integer not greater than X.  */
extern _Float32x floorf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float32x __floorf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Floating-point modulo remainder of X/Y.  */
extern _Float32x fmodf32x(_Float32x __x, _Float32x __y) __attribute__((__nothrow__, __leaf__));
extern _Float32x __fmodf32x(_Float32x __x, _Float32x __y) __attribute__((__nothrow__, __leaf__));
/* Return X with its signed changed to Y's.  */
extern _Float32x copysignf32x(_Float32x __x, _Float32x __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float32x __copysignf32x(_Float32x __x, _Float32x __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return representation of qNaN for double type.  */
extern _Float32x nanf32x(const char *__tagb) __attribute__((__nothrow__, __leaf__));
extern _Float32x __nanf32x(const char *__tagb) __attribute__((__nothrow__, __leaf__));
/* Bessel functions.  */
extern _Float32x j0f32x(_Float32x) __attribute__((__nothrow__, __leaf__));
extern _Float32x __j0f32x(_Float32x) __attribute__((__nothrow__, __leaf__));
extern _Float32x j1f32x(_Float32x) __attribute__((__nothrow__, __leaf__));
extern _Float32x __j1f32x(_Float32x) __attribute__((__nothrow__, __leaf__));
extern _Float32x jnf32x(int, _Float32x) __attribute__((__nothrow__, __leaf__));
extern _Float32x __jnf32x(int, _Float32x) __attribute__((__nothrow__, __leaf__));
extern _Float32x y0f32x(_Float32x) __attribute__((__nothrow__, __leaf__));
extern _Float32x __y0f32x(_Float32x) __attribute__((__nothrow__, __leaf__));
extern _Float32x y1f32x(_Float32x) __attribute__((__nothrow__, __leaf__));
extern _Float32x __y1f32x(_Float32x) __attribute__((__nothrow__, __leaf__));
extern _Float32x ynf32x(int, _Float32x) __attribute__((__nothrow__, __leaf__));
extern _Float32x __ynf32x(int, _Float32x) __attribute__((__nothrow__, __leaf__));

/* Error and gamma functions.  */
extern _Float32x erff32x(_Float32x) __attribute__((__nothrow__, __leaf__));
extern _Float32x __erff32x(_Float32x) __attribute__((__nothrow__, __leaf__));
extern _Float32x erfcf32x(_Float32x) __attribute__((__nothrow__, __leaf__));
extern _Float32x __erfcf32x(_Float32x) __attribute__((__nothrow__, __leaf__));
extern _Float32x lgammaf32x(_Float32x) __attribute__((__nothrow__, __leaf__));
extern _Float32x __lgammaf32x(_Float32x) __attribute__((__nothrow__, __leaf__));

/* True gamma function.  */
extern _Float32x tgammaf32x(_Float32x) __attribute__((__nothrow__, __leaf__));
extern _Float32x __tgammaf32x(_Float32x) __attribute__((__nothrow__, __leaf__));
/* Reentrant version of lgamma.  This function uses the global variable
   `signgam'.  The reentrant version instead takes a pointer and stores
   the value through it.  */
extern _Float32x lgammaf32x_r(_Float32x, int *__signgamp) __attribute__((__nothrow__, __leaf__));
extern _Float32x __lgammaf32x_r(_Float32x, int *__signgamp) __attribute__((__nothrow__, __leaf__));

/* Return the integer nearest X in the direction of the
   prevailing rounding mode.  */
extern _Float32x rintf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));
extern _Float32x __rintf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));

/* Return X + epsilon if X < Y, X - epsilon if X > Y.  */
extern _Float32x nextafterf32x(_Float32x __x, _Float32x __y) __attribute__((__nothrow__, __leaf__));
extern _Float32x __nextafterf32x(_Float32x __x, _Float32x __y) __attribute__((__nothrow__, __leaf__));

/* Return X - epsilon.  */
extern _Float32x nextdownf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));
extern _Float32x __nextdownf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));
/* Return X + epsilon.  */
extern _Float32x nextupf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));
extern _Float32x __nextupf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));

/* Return the remainder of integer division X / Y with infinite precision.  */
extern _Float32x remainderf32x(_Float32x __x, _Float32x __y) __attribute__((__nothrow__, __leaf__));
extern _Float32x __remainderf32x(_Float32x __x, _Float32x __y) __attribute__((__nothrow__, __leaf__));

/* Return X times (2 to the Nth power).  */
extern _Float32x scalbnf32x(_Float32x __x, int __n) __attribute__((__nothrow__, __leaf__));
extern _Float32x __scalbnf32x(_Float32x __x, int __n) __attribute__((__nothrow__, __leaf__));

/* Return the binary exponent of X, which must be nonzero.  */
extern int ilogbf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));
extern int __ilogbf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));

/* Like ilogb, but returning long int.  */
extern long int llogbf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));
extern long int __llogbf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));

/* Return X times (2 to the Nth power).  */
extern _Float32x scalblnf32x(_Float32x __x, long int __n) __attribute__((__nothrow__, __leaf__));
extern _Float32x __scalblnf32x(_Float32x __x, long int __n) __attribute__((__nothrow__, __leaf__));

/* Round X to integral value in floating-point format using current
   rounding direction, but do not raise inexact exception.  */
extern _Float32x nearbyintf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));
extern _Float32x __nearbyintf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));

/* Round X to nearest integral value, rounding halfway cases away from
   zero.  */
extern _Float32x roundf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float32x __roundf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Round X to the integral value in floating-point format nearest but
   not larger in magnitude.  */
extern _Float32x truncf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float32x __truncf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Compute remainder of X and Y and put in *QUO a value with sign of x/y
   and magnitude congruent `mod 2^n' to the magnitude of the integral
   quotient x/y, with n >= 3.  */
extern _Float32x remquof32x(_Float32x __x, _Float32x __y, int *__quo) __attribute__((__nothrow__, __leaf__));
extern _Float32x __remquof32x(_Float32x __x, _Float32x __y, int *__quo) __attribute__((__nothrow__, __leaf__));

/* Conversion functions.  */

/* Round X to nearest integral value according to current rounding
   direction.  */
extern long int lrintf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));
extern long int __lrintf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));
__extension__ extern long long int llrintf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));
extern long long int __llrintf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));

/* Round X to nearest integral value, rounding halfway cases away from
   zero.  */
extern long int lroundf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));
extern long int __lroundf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));
__extension__ extern long long int llroundf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));
extern long long int __llroundf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));

/* Return positive difference between X and Y.  */
extern _Float32x fdimf32x(_Float32x __x, _Float32x __y) __attribute__((__nothrow__, __leaf__));
extern _Float32x __fdimf32x(_Float32x __x, _Float32x __y) __attribute__((__nothrow__, __leaf__));

/* Return maximum numeric value from X and Y.  */
extern _Float32x fmaxf32x(_Float32x __x, _Float32x __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float32x __fmaxf32x(_Float32x __x, _Float32x __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return minimum numeric value from X and Y.  */
extern _Float32x fminf32x(_Float32x __x, _Float32x __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float32x __fminf32x(_Float32x __x, _Float32x __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Multiply-add function computed as a ternary operation.  */
extern _Float32x fmaf32x(_Float32x __x, _Float32x __y, _Float32x __z) __attribute__((__nothrow__, __leaf__));
extern _Float32x __fmaf32x(_Float32x __x, _Float32x __y, _Float32x __z) __attribute__((__nothrow__, __leaf__));

/* Round X to nearest integer value, rounding halfway cases to even.  */
extern _Float32x roundevenf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float32x __roundevenf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Round X to nearest signed integer value, not raising inexact, with
   control of rounding direction and width of result.  */
extern __intmax_t fromfpf32x(_Float32x __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));
extern __intmax_t __fromfpf32x(_Float32x __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));

/* Round X to nearest unsigned integer value, not raising inexact,
   with control of rounding direction and width of result.  */
extern __uintmax_t ufromfpf32x(_Float32x __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));
extern __uintmax_t __ufromfpf32x(_Float32x __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));

/* Round X to nearest signed integer value, raising inexact for
   non-integers, with control of rounding direction and width of
   result.  */
extern __intmax_t fromfpxf32x(_Float32x __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));
extern __intmax_t __fromfpxf32x(_Float32x __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));

/* Round X to nearest unsigned integer value, raising inexact for
   non-integers, with control of rounding direction and width of
   result.  */
extern __uintmax_t ufromfpxf32x(_Float32x __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));
extern __uintmax_t __ufromfpxf32x(_Float32x __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));

/* Canonicalize floating-point representation.  */
extern int canonicalizef32x(_Float32x *__cx, const _Float32x *__x) __attribute__((__nothrow__, __leaf__));

/* Return value with maximum magnitude.  */
extern _Float32x fmaxmagf32x(_Float32x __x, _Float32x __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float32x __fmaxmagf32x(_Float32x __x, _Float32x __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return value with minimum magnitude.  */
extern _Float32x fminmagf32x(_Float32x __x, _Float32x __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float32x __fminmagf32x(_Float32x __x, _Float32x __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return maximum value from X and Y.  */
extern _Float32x fmaximumf32x(_Float32x __x, _Float32x __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float32x __fmaximumf32x(_Float32x __x, _Float32x __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return minimum value from X and Y.  */
extern _Float32x fminimumf32x(_Float32x __x, _Float32x __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float32x __fminimumf32x(_Float32x __x, _Float32x __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return maximum numeric value from X and Y.  */
extern _Float32x fmaximum_numf32x(_Float32x __x, _Float32x __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float32x __fmaximum_numf32x(_Float32x __x, _Float32x __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return minimum numeric value from X and Y.  */
extern _Float32x fminimum_numf32x(_Float32x __x, _Float32x __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float32x __fminimum_numf32x(_Float32x __x, _Float32x __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return value with maximum magnitude.  */
extern _Float32x fmaximum_magf32x(_Float32x __x, _Float32x __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float32x __fmaximum_magf32x(_Float32x __x, _Float32x __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return value with minimum magnitude.  */
extern _Float32x fminimum_magf32x(_Float32x __x, _Float32x __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float32x __fminimum_magf32x(_Float32x __x, _Float32x __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return numeric value with maximum magnitude.  */
extern _Float32x fmaximum_mag_numf32x(_Float32x __x, _Float32x __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float32x __fmaximum_mag_numf32x(_Float32x __x, _Float32x __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return numeric value with minimum magnitude.  */
extern _Float32x fminimum_mag_numf32x(_Float32x __x, _Float32x __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float32x __fminimum_mag_numf32x(_Float32x __x, _Float32x __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Total order operation.  */
extern int totalorderf32x(const _Float32x *__x, const _Float32x *__y) __attribute__((__nothrow__, __leaf__))

__attribute__((__pure__));

/* Total order operation on absolute values.  */
extern int totalordermagf32x(const _Float32x *__x, const _Float32x *__y) __attribute__((__nothrow__, __leaf__))

__attribute__((__pure__));

/* Get NaN payload.  */
extern _Float32x getpayloadf32x(const _Float32x *__x) __attribute__((__nothrow__, __leaf__));
extern _Float32x __getpayloadf32x(const _Float32x *__x) __attribute__((__nothrow__, __leaf__));

/* Set quiet NaN payload.  */
extern int setpayloadf32x(_Float32x *__x, _Float32x __payload) __attribute__((__nothrow__, __leaf__));

/* Set signaling NaN payload.  */
extern int setpayloadsigf32x(_Float32x *__x, _Float32x __payload) __attribute__((__nothrow__, __leaf__));
/* Prototype declarations for math functions; helper file for <math.h>.
   Copyright (C) 1996-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* NOTE: Because of the special way this file is used by <math.h>, this
   file must NOT be protected from multiple inclusion as header files
   usually are.

   This file provides prototype declarations for the math functions.
   Most functions are declared using the macro:

   __MATHCALL (NAME,[_r], (ARGS...));

   This means there is a function `NAME' returning `double' and a function
   `NAMEf' returning `float'.  Each place `_Mdouble_' appears in the
   prototype, that is actually `double' in the prototype for `NAME' and
   `float' in the prototype for `NAMEf'.  Reentrant variant functions are
   called `NAME_r' and `NAMEf_r'.

   Functions returning other types like `int' are declared using the macro:

   __MATHDECL (TYPE, NAME,[_r], (ARGS...));

   This is just like __MATHCALL but for a function returning `TYPE'
   instead of `_Mdouble_'.  In all of these cases, there is still
   both a `NAME' and a `NAMEf' that takes `float' arguments.

   Note that there must be no whitespace before the argument passed for
   NAME, to make token pasting work with -traditional.  */

/* Trigonometric functions.  */

/* Arc cosine of X.  */
extern _Float64x acosf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));
extern _Float64x __acosf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));
/* Arc sine of X.  */
extern _Float64x asinf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));
extern _Float64x __asinf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));
/* Arc tangent of X.  */
extern _Float64x atanf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));
extern _Float64x __atanf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));
/* Arc tangent of Y/X.  */
extern _Float64x atan2f64x(_Float64x __y, _Float64x __x) __attribute__((__nothrow__, __leaf__));
extern _Float64x __atan2f64x(_Float64x __y, _Float64x __x) __attribute__((__nothrow__, __leaf__));

/* Cosine of X.  */
extern _Float64x cosf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));
extern _Float64x __cosf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));
/* Sine of X.  */
extern _Float64x sinf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));
extern _Float64x __sinf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));
/* Tangent of X.  */
extern _Float64x tanf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));
extern _Float64x __tanf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));

/* Hyperbolic functions.  */

/* Hyperbolic cosine of X.  */
extern _Float64x coshf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));
extern _Float64x __coshf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));
/* Hyperbolic sine of X.  */
extern _Float64x sinhf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));
extern _Float64x __sinhf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));
/* Hyperbolic tangent of X.  */
extern _Float64x tanhf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));
extern _Float64x __tanhf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));

/* Cosine and sine of X.  */
extern void sincosf64x(_Float64x __x, _Float64x *__sinx, _Float64x *__cosx) __attribute__((__nothrow__, __leaf__));
extern void __sincosf64x(_Float64x __x, _Float64x *__sinx, _Float64x *__cosx) __attribute__((__nothrow__, __leaf__));

/* Hyperbolic arc cosine of X.  */
extern _Float64x acoshf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));
extern _Float64x __acoshf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));
/* Hyperbolic arc sine of X.  */
extern _Float64x asinhf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));
extern _Float64x __asinhf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));
/* Hyperbolic arc tangent of X.  */
extern _Float64x atanhf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));
extern _Float64x __atanhf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));

/* Exponential and logarithmic functions.  */

/* Exponential function of X.  */
extern _Float64x expf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));
extern _Float64x __expf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));

/* Break VALUE into a normalized fraction and an integral power of 2.  */
extern _Float64x frexpf64x(_Float64x __x, int *__exponent) __attribute__((__nothrow__, __leaf__));
extern _Float64x __frexpf64x(_Float64x __x, int *__exponent) __attribute__((__nothrow__, __leaf__));

/* X times (two to the EXP power).  */
extern _Float64x ldexpf64x(_Float64x __x, int __exponent) __attribute__((__nothrow__, __leaf__));
extern _Float64x __ldexpf64x(_Float64x __x, int __exponent) __attribute__((__nothrow__, __leaf__));

/* Natural logarithm of X.  */
extern _Float64x logf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));
extern _Float64x __logf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));

/* Base-ten logarithm of X.  */
extern _Float64x log10f64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));
extern _Float64x __log10f64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));

/* Break VALUE into integral and fractional parts.  */
extern _Float64x modff64x(_Float64x __x, _Float64x *__iptr) __attribute__((__nothrow__, __leaf__));
extern _Float64x __modff64x(_Float64x __x, _Float64x *__iptr) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(2)));

/* Compute exponent to base ten.  */
extern _Float64x exp10f64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));
extern _Float64x __exp10f64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));

/* Return exp(X) - 1.  */
extern _Float64x expm1f64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));
extern _Float64x __expm1f64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));

/* Return log(1 + X).  */
extern _Float64x log1pf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));
extern _Float64x __log1pf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));

/* Return the base 2 signed integral exponent of X.  */
extern _Float64x logbf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));
extern _Float64x __logbf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));

/* Compute base-2 exponential of X.  */
extern _Float64x exp2f64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));
extern _Float64x __exp2f64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));

/* Compute base-2 logarithm of X.  */
extern _Float64x log2f64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));
extern _Float64x __log2f64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));

/* Power functions.  */

/* Return X to the Y power.  */
extern _Float64x powf64x(_Float64x __x, _Float64x __y) __attribute__((__nothrow__, __leaf__));
extern _Float64x __powf64x(_Float64x __x, _Float64x __y) __attribute__((__nothrow__, __leaf__));

/* Return the square root of X.  */
extern _Float64x sqrtf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));
extern _Float64x __sqrtf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));

/* Return `sqrt(X*X + Y*Y)'.  */
extern _Float64x hypotf64x(_Float64x __x, _Float64x __y) __attribute__((__nothrow__, __leaf__));
extern _Float64x __hypotf64x(_Float64x __x, _Float64x __y) __attribute__((__nothrow__, __leaf__));

/* Return the cube root of X.  */
extern _Float64x cbrtf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));
extern _Float64x __cbrtf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));

/* Nearest integer, absolute value, and remainder functions.  */

/* Smallest integral value not less than X.  */
extern _Float64x ceilf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float64x __ceilf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Absolute value of X.  */
extern _Float64x fabsf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float64x __fabsf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Largest integer not greater than X.  */
extern _Float64x floorf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float64x __floorf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Floating-point modulo remainder of X/Y.  */
extern _Float64x fmodf64x(_Float64x __x, _Float64x __y) __attribute__((__nothrow__, __leaf__));
extern _Float64x __fmodf64x(_Float64x __x, _Float64x __y) __attribute__((__nothrow__, __leaf__));
/* Return X with its signed changed to Y's.  */
extern _Float64x copysignf64x(_Float64x __x, _Float64x __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float64x __copysignf64x(_Float64x __x, _Float64x __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return representation of qNaN for double type.  */
extern _Float64x nanf64x(const char *__tagb) __attribute__((__nothrow__, __leaf__));
extern _Float64x __nanf64x(const char *__tagb) __attribute__((__nothrow__, __leaf__));
/* Bessel functions.  */
extern _Float64x j0f64x(_Float64x) __attribute__((__nothrow__, __leaf__));
extern _Float64x __j0f64x(_Float64x) __attribute__((__nothrow__, __leaf__));
extern _Float64x j1f64x(_Float64x) __attribute__((__nothrow__, __leaf__));
extern _Float64x __j1f64x(_Float64x) __attribute__((__nothrow__, __leaf__));
extern _Float64x jnf64x(int, _Float64x) __attribute__((__nothrow__, __leaf__));
extern _Float64x __jnf64x(int, _Float64x) __attribute__((__nothrow__, __leaf__));
extern _Float64x y0f64x(_Float64x) __attribute__((__nothrow__, __leaf__));
extern _Float64x __y0f64x(_Float64x) __attribute__((__nothrow__, __leaf__));
extern _Float64x y1f64x(_Float64x) __attribute__((__nothrow__, __leaf__));
extern _Float64x __y1f64x(_Float64x) __attribute__((__nothrow__, __leaf__));
extern _Float64x ynf64x(int, _Float64x) __attribute__((__nothrow__, __leaf__));
extern _Float64x __ynf64x(int, _Float64x) __attribute__((__nothrow__, __leaf__));

/* Error and gamma functions.  */
extern _Float64x erff64x(_Float64x) __attribute__((__nothrow__, __leaf__));
extern _Float64x __erff64x(_Float64x) __attribute__((__nothrow__, __leaf__));
extern _Float64x erfcf64x(_Float64x) __attribute__((__nothrow__, __leaf__));
extern _Float64x __erfcf64x(_Float64x) __attribute__((__nothrow__, __leaf__));
extern _Float64x lgammaf64x(_Float64x) __attribute__((__nothrow__, __leaf__));
extern _Float64x __lgammaf64x(_Float64x) __attribute__((__nothrow__, __leaf__));

/* True gamma function.  */
extern _Float64x tgammaf64x(_Float64x) __attribute__((__nothrow__, __leaf__));
extern _Float64x __tgammaf64x(_Float64x) __attribute__((__nothrow__, __leaf__));
/* Reentrant version of lgamma.  This function uses the global variable
   `signgam'.  The reentrant version instead takes a pointer and stores
   the value through it.  */
extern _Float64x lgammaf64x_r(_Float64x, int *__signgamp) __attribute__((__nothrow__, __leaf__));
extern _Float64x __lgammaf64x_r(_Float64x, int *__signgamp) __attribute__((__nothrow__, __leaf__));

/* Return the integer nearest X in the direction of the
   prevailing rounding mode.  */
extern _Float64x rintf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));
extern _Float64x __rintf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));

/* Return X + epsilon if X < Y, X - epsilon if X > Y.  */
extern _Float64x nextafterf64x(_Float64x __x, _Float64x __y) __attribute__((__nothrow__, __leaf__));
extern _Float64x __nextafterf64x(_Float64x __x, _Float64x __y) __attribute__((__nothrow__, __leaf__));

/* Return X - epsilon.  */
extern _Float64x nextdownf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));
extern _Float64x __nextdownf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));
/* Return X + epsilon.  */
extern _Float64x nextupf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));
extern _Float64x __nextupf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));

/* Return the remainder of integer division X / Y with infinite precision.  */
extern _Float64x remainderf64x(_Float64x __x, _Float64x __y) __attribute__((__nothrow__, __leaf__));
extern _Float64x __remainderf64x(_Float64x __x, _Float64x __y) __attribute__((__nothrow__, __leaf__));

/* Return X times (2 to the Nth power).  */
extern _Float64x scalbnf64x(_Float64x __x, int __n) __attribute__((__nothrow__, __leaf__));
extern _Float64x __scalbnf64x(_Float64x __x, int __n) __attribute__((__nothrow__, __leaf__));

/* Return the binary exponent of X, which must be nonzero.  */
extern int ilogbf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));
extern int __ilogbf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));

/* Like ilogb, but returning long int.  */
extern long int llogbf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));
extern long int __llogbf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));

/* Return X times (2 to the Nth power).  */
extern _Float64x scalblnf64x(_Float64x __x, long int __n) __attribute__((__nothrow__, __leaf__));
extern _Float64x __scalblnf64x(_Float64x __x, long int __n) __attribute__((__nothrow__, __leaf__));

/* Round X to integral value in floating-point format using current
   rounding direction, but do not raise inexact exception.  */
extern _Float64x nearbyintf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));
extern _Float64x __nearbyintf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));

/* Round X to nearest integral value, rounding halfway cases away from
   zero.  */
extern _Float64x roundf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float64x __roundf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Round X to the integral value in floating-point format nearest but
   not larger in magnitude.  */
extern _Float64x truncf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float64x __truncf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Compute remainder of X and Y and put in *QUO a value with sign of x/y
   and magnitude congruent `mod 2^n' to the magnitude of the integral
   quotient x/y, with n >= 3.  */
extern _Float64x remquof64x(_Float64x __x, _Float64x __y, int *__quo) __attribute__((__nothrow__, __leaf__));
extern _Float64x __remquof64x(_Float64x __x, _Float64x __y, int *__quo) __attribute__((__nothrow__, __leaf__));

/* Conversion functions.  */

/* Round X to nearest integral value according to current rounding
   direction.  */
extern long int lrintf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));
extern long int __lrintf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));
__extension__ extern long long int llrintf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));
extern long long int __llrintf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));

/* Round X to nearest integral value, rounding halfway cases away from
   zero.  */
extern long int lroundf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));
extern long int __lroundf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));
__extension__ extern long long int llroundf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));
extern long long int __llroundf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));

/* Return positive difference between X and Y.  */
extern _Float64x fdimf64x(_Float64x __x, _Float64x __y) __attribute__((__nothrow__, __leaf__));
extern _Float64x __fdimf64x(_Float64x __x, _Float64x __y) __attribute__((__nothrow__, __leaf__));

/* Return maximum numeric value from X and Y.  */
extern _Float64x fmaxf64x(_Float64x __x, _Float64x __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float64x __fmaxf64x(_Float64x __x, _Float64x __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return minimum numeric value from X and Y.  */
extern _Float64x fminf64x(_Float64x __x, _Float64x __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float64x __fminf64x(_Float64x __x, _Float64x __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Multiply-add function computed as a ternary operation.  */
extern _Float64x fmaf64x(_Float64x __x, _Float64x __y, _Float64x __z) __attribute__((__nothrow__, __leaf__));
extern _Float64x __fmaf64x(_Float64x __x, _Float64x __y, _Float64x __z) __attribute__((__nothrow__, __leaf__));

/* Round X to nearest integer value, rounding halfway cases to even.  */
extern _Float64x roundevenf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float64x __roundevenf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Round X to nearest signed integer value, not raising inexact, with
   control of rounding direction and width of result.  */
extern __intmax_t fromfpf64x(_Float64x __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));
extern __intmax_t __fromfpf64x(_Float64x __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));

/* Round X to nearest unsigned integer value, not raising inexact,
   with control of rounding direction and width of result.  */
extern __uintmax_t ufromfpf64x(_Float64x __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));
extern __uintmax_t __ufromfpf64x(_Float64x __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));

/* Round X to nearest signed integer value, raising inexact for
   non-integers, with control of rounding direction and width of
   result.  */
extern __intmax_t fromfpxf64x(_Float64x __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));
extern __intmax_t __fromfpxf64x(_Float64x __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));

/* Round X to nearest unsigned integer value, raising inexact for
   non-integers, with control of rounding direction and width of
   result.  */
extern __uintmax_t ufromfpxf64x(_Float64x __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));
extern __uintmax_t __ufromfpxf64x(_Float64x __x, int __round, unsigned int __width) __attribute__((__nothrow__, __leaf__));

/* Canonicalize floating-point representation.  */
extern int canonicalizef64x(_Float64x *__cx, const _Float64x *__x) __attribute__((__nothrow__, __leaf__));

/* Return value with maximum magnitude.  */
extern _Float64x fmaxmagf64x(_Float64x __x, _Float64x __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float64x __fmaxmagf64x(_Float64x __x, _Float64x __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return value with minimum magnitude.  */
extern _Float64x fminmagf64x(_Float64x __x, _Float64x __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float64x __fminmagf64x(_Float64x __x, _Float64x __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return maximum value from X and Y.  */
extern _Float64x fmaximumf64x(_Float64x __x, _Float64x __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float64x __fmaximumf64x(_Float64x __x, _Float64x __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return minimum value from X and Y.  */
extern _Float64x fminimumf64x(_Float64x __x, _Float64x __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float64x __fminimumf64x(_Float64x __x, _Float64x __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return maximum numeric value from X and Y.  */
extern _Float64x fmaximum_numf64x(_Float64x __x, _Float64x __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float64x __fmaximum_numf64x(_Float64x __x, _Float64x __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return minimum numeric value from X and Y.  */
extern _Float64x fminimum_numf64x(_Float64x __x, _Float64x __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float64x __fminimum_numf64x(_Float64x __x, _Float64x __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return value with maximum magnitude.  */
extern _Float64x fmaximum_magf64x(_Float64x __x, _Float64x __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float64x __fmaximum_magf64x(_Float64x __x, _Float64x __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return value with minimum magnitude.  */
extern _Float64x fminimum_magf64x(_Float64x __x, _Float64x __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float64x __fminimum_magf64x(_Float64x __x, _Float64x __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return numeric value with maximum magnitude.  */
extern _Float64x fmaximum_mag_numf64x(_Float64x __x, _Float64x __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float64x __fmaximum_mag_numf64x(_Float64x __x, _Float64x __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return numeric value with minimum magnitude.  */
extern _Float64x fminimum_mag_numf64x(_Float64x __x, _Float64x __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern _Float64x __fminimum_mag_numf64x(_Float64x __x, _Float64x __y) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Total order operation.  */
extern int totalorderf64x(const _Float64x *__x, const _Float64x *__y) __attribute__((__nothrow__, __leaf__))

__attribute__((__pure__));

/* Total order operation on absolute values.  */
extern int totalordermagf64x(const _Float64x *__x, const _Float64x *__y) __attribute__((__nothrow__, __leaf__))

__attribute__((__pure__));

/* Get NaN payload.  */
extern _Float64x getpayloadf64x(const _Float64x *__x) __attribute__((__nothrow__, __leaf__));
extern _Float64x __getpayloadf64x(const _Float64x *__x) __attribute__((__nothrow__, __leaf__));

/* Set quiet NaN payload.  */
extern int setpayloadf64x(_Float64x *__x, _Float64x __payload) __attribute__((__nothrow__, __leaf__));

/* Set signaling NaN payload.  */
extern int setpayloadsigf64x(_Float64x *__x, _Float64x __payload) __attribute__((__nothrow__, __leaf__));
/* Declare functions returning a narrower type.  */
/* Declare functions returning a narrower type.
   Copyright (C) 2018-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* Add.  */
extern float fadd(double __x, double __y) __attribute__((__nothrow__, __leaf__));

/* Divide.  */
extern float fdiv(double __x, double __y) __attribute__((__nothrow__, __leaf__));

/* Fused multiply-add.  */
extern float ffma(double __x, double __y, double __z) __attribute__((__nothrow__, __leaf__));

/* Multiply.  */
extern float fmul(double __x, double __y) __attribute__((__nothrow__, __leaf__));

/* Square root.  */
extern float fsqrt(double __x) __attribute__((__nothrow__, __leaf__));

/* Subtract.  */
extern float fsub(double __x, double __y) __attribute__((__nothrow__, __leaf__));
/* Declare functions returning a narrower type.
   Copyright (C) 2018-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* Add.  */
extern float faddl(long double __x, long double __y) __attribute__((__nothrow__, __leaf__));

/* Divide.  */
extern float fdivl(long double __x, long double __y) __attribute__((__nothrow__, __leaf__));

/* Fused multiply-add.  */
extern float ffmal(long double __x, long double __y, long double __z) __attribute__((__nothrow__, __leaf__));

/* Multiply.  */
extern float fmull(long double __x, long double __y) __attribute__((__nothrow__, __leaf__));

/* Square root.  */
extern float fsqrtl(long double __x) __attribute__((__nothrow__, __leaf__));

/* Subtract.  */
extern float fsubl(long double __x, long double __y) __attribute__((__nothrow__, __leaf__));
/* Declare functions returning a narrower type.
   Copyright (C) 2018-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* Add.  */
extern double daddl(long double __x, long double __y) __attribute__((__nothrow__, __leaf__));

/* Divide.  */
extern double ddivl(long double __x, long double __y) __attribute__((__nothrow__, __leaf__));

/* Fused multiply-add.  */
extern double dfmal(long double __x, long double __y, long double __z) __attribute__((__nothrow__, __leaf__));

/* Multiply.  */
extern double dmull(long double __x, long double __y) __attribute__((__nothrow__, __leaf__));

/* Square root.  */
extern double dsqrtl(long double __x) __attribute__((__nothrow__, __leaf__));

/* Subtract.  */
extern double dsubl(long double __x, long double __y) __attribute__((__nothrow__, __leaf__));
/* Declare functions returning a narrower type.
   Copyright (C) 2018-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* Add.  */
extern _Float32 f32addf32x(_Float32x __x, _Float32x __y) __attribute__((__nothrow__, __leaf__));

/* Divide.  */
extern _Float32 f32divf32x(_Float32x __x, _Float32x __y) __attribute__((__nothrow__, __leaf__));

/* Fused multiply-add.  */
extern _Float32 f32fmaf32x(_Float32x __x, _Float32x __y, _Float32x __z) __attribute__((__nothrow__, __leaf__));

/* Multiply.  */
extern _Float32 f32mulf32x(_Float32x __x, _Float32x __y) __attribute__((__nothrow__, __leaf__));

/* Square root.  */
extern _Float32 f32sqrtf32x(_Float32x __x) __attribute__((__nothrow__, __leaf__));

/* Subtract.  */
extern _Float32 f32subf32x(_Float32x __x, _Float32x __y) __attribute__((__nothrow__, __leaf__));
/* Declare functions returning a narrower type.
   Copyright (C) 2018-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* Add.  */
extern _Float32 f32addf64(_Float64 __x, _Float64 __y) __attribute__((__nothrow__, __leaf__));

/* Divide.  */
extern _Float32 f32divf64(_Float64 __x, _Float64 __y) __attribute__((__nothrow__, __leaf__));

/* Fused multiply-add.  */
extern _Float32 f32fmaf64(_Float64 __x, _Float64 __y, _Float64 __z) __attribute__((__nothrow__, __leaf__));

/* Multiply.  */
extern _Float32 f32mulf64(_Float64 __x, _Float64 __y) __attribute__((__nothrow__, __leaf__));

/* Square root.  */
extern _Float32 f32sqrtf64(_Float64 __x) __attribute__((__nothrow__, __leaf__));

/* Subtract.  */
extern _Float32 f32subf64(_Float64 __x, _Float64 __y) __attribute__((__nothrow__, __leaf__));
/* Declare functions returning a narrower type.
   Copyright (C) 2018-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* Add.  */
extern _Float32 f32addf64x(_Float64x __x, _Float64x __y) __attribute__((__nothrow__, __leaf__));

/* Divide.  */
extern _Float32 f32divf64x(_Float64x __x, _Float64x __y) __attribute__((__nothrow__, __leaf__));

/* Fused multiply-add.  */
extern _Float32 f32fmaf64x(_Float64x __x, _Float64x __y, _Float64x __z) __attribute__((__nothrow__, __leaf__));

/* Multiply.  */
extern _Float32 f32mulf64x(_Float64x __x, _Float64x __y) __attribute__((__nothrow__, __leaf__));

/* Square root.  */
extern _Float32 f32sqrtf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));

/* Subtract.  */
extern _Float32 f32subf64x(_Float64x __x, _Float64x __y) __attribute__((__nothrow__, __leaf__));
/* Declare functions returning a narrower type.
   Copyright (C) 2018-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* Add.  */
extern _Float32 f32addf128(_Float128 __x, _Float128 __y) __attribute__((__nothrow__, __leaf__));

/* Divide.  */
extern _Float32 f32divf128(_Float128 __x, _Float128 __y) __attribute__((__nothrow__, __leaf__));

/* Fused multiply-add.  */
extern _Float32 f32fmaf128(_Float128 __x, _Float128 __y, _Float128 __z) __attribute__((__nothrow__, __leaf__));

/* Multiply.  */
extern _Float32 f32mulf128(_Float128 __x, _Float128 __y) __attribute__((__nothrow__, __leaf__));

/* Square root.  */
extern _Float32 f32sqrtf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));

/* Subtract.  */
extern _Float32 f32subf128(_Float128 __x, _Float128 __y) __attribute__((__nothrow__, __leaf__));
/* Declare functions returning a narrower type.
   Copyright (C) 2018-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* Add.  */
extern _Float32x f32xaddf64(_Float64 __x, _Float64 __y) __attribute__((__nothrow__, __leaf__));

/* Divide.  */
extern _Float32x f32xdivf64(_Float64 __x, _Float64 __y) __attribute__((__nothrow__, __leaf__));

/* Fused multiply-add.  */
extern _Float32x f32xfmaf64(_Float64 __x, _Float64 __y, _Float64 __z) __attribute__((__nothrow__, __leaf__));

/* Multiply.  */
extern _Float32x f32xmulf64(_Float64 __x, _Float64 __y) __attribute__((__nothrow__, __leaf__));

/* Square root.  */
extern _Float32x f32xsqrtf64(_Float64 __x) __attribute__((__nothrow__, __leaf__));

/* Subtract.  */
extern _Float32x f32xsubf64(_Float64 __x, _Float64 __y) __attribute__((__nothrow__, __leaf__));
/* Declare functions returning a narrower type.
   Copyright (C) 2018-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* Add.  */
extern _Float32x f32xaddf64x(_Float64x __x, _Float64x __y) __attribute__((__nothrow__, __leaf__));

/* Divide.  */
extern _Float32x f32xdivf64x(_Float64x __x, _Float64x __y) __attribute__((__nothrow__, __leaf__));

/* Fused multiply-add.  */
extern _Float32x f32xfmaf64x(_Float64x __x, _Float64x __y, _Float64x __z) __attribute__((__nothrow__, __leaf__));

/* Multiply.  */
extern _Float32x f32xmulf64x(_Float64x __x, _Float64x __y) __attribute__((__nothrow__, __leaf__));

/* Square root.  */
extern _Float32x f32xsqrtf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));

/* Subtract.  */
extern _Float32x f32xsubf64x(_Float64x __x, _Float64x __y) __attribute__((__nothrow__, __leaf__));
/* Declare functions returning a narrower type.
   Copyright (C) 2018-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* Add.  */
extern _Float32x f32xaddf128(_Float128 __x, _Float128 __y) __attribute__((__nothrow__, __leaf__));

/* Divide.  */
extern _Float32x f32xdivf128(_Float128 __x, _Float128 __y) __attribute__((__nothrow__, __leaf__));

/* Fused multiply-add.  */
extern _Float32x f32xfmaf128(_Float128 __x, _Float128 __y, _Float128 __z) __attribute__((__nothrow__, __leaf__));

/* Multiply.  */
extern _Float32x f32xmulf128(_Float128 __x, _Float128 __y) __attribute__((__nothrow__, __leaf__));

/* Square root.  */
extern _Float32x f32xsqrtf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));

/* Subtract.  */
extern _Float32x f32xsubf128(_Float128 __x, _Float128 __y) __attribute__((__nothrow__, __leaf__));
/* Declare functions returning a narrower type.
   Copyright (C) 2018-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* Add.  */
extern _Float64 f64addf64x(_Float64x __x, _Float64x __y) __attribute__((__nothrow__, __leaf__));

/* Divide.  */
extern _Float64 f64divf64x(_Float64x __x, _Float64x __y) __attribute__((__nothrow__, __leaf__));

/* Fused multiply-add.  */
extern _Float64 f64fmaf64x(_Float64x __x, _Float64x __y, _Float64x __z) __attribute__((__nothrow__, __leaf__));

/* Multiply.  */
extern _Float64 f64mulf64x(_Float64x __x, _Float64x __y) __attribute__((__nothrow__, __leaf__));

/* Square root.  */
extern _Float64 f64sqrtf64x(_Float64x __x) __attribute__((__nothrow__, __leaf__));

/* Subtract.  */
extern _Float64 f64subf64x(_Float64x __x, _Float64x __y) __attribute__((__nothrow__, __leaf__));
/* Declare functions returning a narrower type.
   Copyright (C) 2018-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* Add.  */
extern _Float64 f64addf128(_Float128 __x, _Float128 __y) __attribute__((__nothrow__, __leaf__));

/* Divide.  */
extern _Float64 f64divf128(_Float128 __x, _Float128 __y) __attribute__((__nothrow__, __leaf__));

/* Fused multiply-add.  */
extern _Float64 f64fmaf128(_Float128 __x, _Float128 __y, _Float128 __z) __attribute__((__nothrow__, __leaf__));

/* Multiply.  */
extern _Float64 f64mulf128(_Float128 __x, _Float128 __y) __attribute__((__nothrow__, __leaf__));

/* Square root.  */
extern _Float64 f64sqrtf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));

/* Subtract.  */
extern _Float64 f64subf128(_Float128 __x, _Float128 __y) __attribute__((__nothrow__, __leaf__));
/* Declare functions returning a narrower type.
   Copyright (C) 2018-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* Add.  */
extern _Float64x f64xaddf128(_Float128 __x, _Float128 __y) __attribute__((__nothrow__, __leaf__));

/* Divide.  */
extern _Float64x f64xdivf128(_Float128 __x, _Float128 __y) __attribute__((__nothrow__, __leaf__));

/* Fused multiply-add.  */
extern _Float64x f64xfmaf128(_Float128 __x, _Float128 __y, _Float128 __z) __attribute__((__nothrow__, __leaf__));

/* Multiply.  */
extern _Float64x f64xmulf128(_Float128 __x, _Float128 __y) __attribute__((__nothrow__, __leaf__));

/* Square root.  */
extern _Float64x f64xsqrtf128(_Float128 __x) __attribute__((__nothrow__, __leaf__));

/* Subtract.  */
extern _Float64x f64xsubf128(_Float128 __x, _Float128 __y) __attribute__((__nothrow__, __leaf__));
/* This variable is used by `gamma' and `lgamma'.  */
extern int signgam;
/* Depending on the type of TG_ARG, call an appropriately suffixed
   version of FUNC with arguments (including parentheses) ARGS.
   Suffixed functions may not exist for long double if it has the same
   format as double, or for other types with the same format as float,
   double or long double.  The behavior is undefined if the argument
   does not have a real floating type.  The definition may use a
   conditional expression, so all suffixed versions of FUNC must
   return the same type (FUNC may include a cast if necessary rather
   than being a single identifier).  */
/* ISO C99 defines some generic macros which work on any data type.  */

/* All floating-point numbers can be put in one of these categories.  */
enum {
    FP_NAN =

        0,
    FP_INFINITE =

        1,
    FP_ZERO =

        2,
    FP_SUBNORMAL =

        3,
    FP_NORMAL =

        4
};

/* GCC bug 66462 means we cannot use the math builtins with -fsignaling-nan,
   so disable builtins if this is enabled.  When fixed in a newer GCC,
   the __SUPPORT_SNAN__ check may be skipped for those versions.  */

/* Return number of classification appropriate for X.  */

/* The check for __cplusplus allows the use of the builtin, even
when optimization for size is on.  This is provided for
libstdc++, only to let its configure test work when it is built
with -Os.  No further use of this definition of fpclassify is
expected in C++ mode, since libstdc++ provides its own version
of fpclassify in cmath (which undefines fpclassify).  */

/* Return nonzero value if sign of X is negative.  */
/* Return nonzero value if X is not +-Inf or NaN.  */

/* Return nonzero value if X is neither zero, subnormal, Inf, nor NaN.  */

/* Return nonzero value if X is a NaN.  We could use `fpclassify' but
   we already have this functions `__isnan' and it is faster.  */

/* Return nonzero value if X is positive or negative infinity.  */
/* Bitmasks for the math_errhandling macro.  */

/* By default all math functions support both errno and exception handling
   (except for soft floating point implementations which may only support
   errno handling).  If errno handling is disabled, exceptions are still
   supported by GLIBC.  Set math_errhandling to 0 with -ffast-math (this is
   nonconforming but it is more useful than leaving it undefined).  */
/* Define iscanonical macro.  ldbl-96 version.
   Copyright (C) 2016-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

extern int __iscanonicall(long double __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return nonzero value if X is canonical.  In IEEE interchange binary
   formats, all values are canonical, but the argument must still be
   converted to its semantic type for any exceptions arising from the
   conversion, before being discarded; in extended precision, there
   are encodings that are not consistently handled as corresponding to
   any particular value of the type, and we return 0 for those.  */

/* Return nonzero value if X is a signaling NaN.  */
/* Return nonzero value if X is subnormal.  */

/* Return nonzero value if X is zero.  */
/* X/Open wants another strange constant.  */

/* Some useful constants.  */
/* GNU extension to provide float constants with similar names.  */
/* The above constants are not adequate for computation using `long double's.
   Therefore we provide as an extension constants with similar names as a
   GNU extension.  Provide enough digits for the 128-bit IEEE quad.  */
/* ISO C99 defines some macros to compare number while taking care for
   unordered numbers.  Many FPUs provide special instructions to support
   these operations.  Generic support in GCC for these as builtins went
   in 2.97, but not all cpus added their patterns until 3.1.  Therefore
   we enable the builtins from 3.1 onwards and use a generic implementation
   otherwise.  */
/* An expression whose type has the widest of the evaluation formats
   of X and Y (which are of floating-point types).  */
/* Return X == Y but raising "invalid" and setting errno if X or Y is
   a NaN.  */

/* Copyright (C) 1989-2022 Free Software Foundation, Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3, or (at your option)
any later version.

GCC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

Under Section 7 of GPL version 3, you are granted additional
permissions described in the GCC Runtime Library Exception, version
3.1, as published by the Free Software Foundation.

You should have received a copy of the GNU General Public License and
a copy of the GCC Runtime Library Exception along with this program;
see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
<http://www.gnu.org/licenses/>.  */

/*
 * ISO C Standard:  7.15  Variable arguments  <stdarg.h>
 */
/* Define __gnuc_va_list.  */

typedef __builtin_va_list __gnuc_va_list;

/* Define the standard macros for the user,
   if this invocation was from the user program.  */
/* Define va_list, if desired, from __gnuc_va_list. */
/* We deliberately do not define va_list when called from
   stdio.h, because ANSI C says that stdio.h is not supposed to define
   va_list.  stdio.h needs to have access to that data type,
   but must not use that name.  It should use the name __gnuc_va_list,
   which is safe because it is reserved for the implementation.  */
/* The macro _VA_LIST_ is the same thing used by this file in Ultrix.
   But on BSD NET2 we must not test or define or undef it.
   (Note that the comments in NET 2's ansi.h
   are incorrect for _VA_LIST_--see stdio.h!)  */

/* The macro _VA_LIST_DEFINED is used in Windows NT 3.5  */

/* The macro _VA_LIST is used in SCO Unix 3.2.  */

/* The macro _VA_LIST_T_H is used in the Bull dpx2  */

/* The macro __va_list__ is used by BeOS.  */

typedef __gnuc_va_list va_list;
/* Copyright (C) 1989-2022 Free Software Foundation, Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3, or (at your option)
any later version.

GCC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

Under Section 7 of GPL version 3, you are granted additional
permissions described in the GCC Runtime Library Exception, version
3.1, as published by the Free Software Foundation.

You should have received a copy of the GNU General Public License and
a copy of the GCC Runtime Library Exception along with this program;
see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
<http://www.gnu.org/licenses/>.  */

/*
 * ISO C Standard:  7.17  Common definitions  <stddef.h>
 */

/* Any one of these symbols __need_* means that GNU libc
   wants us just to define one data type.  So don't define
   the symbols that indicate this file's entire job has been done.  */

/* snaroff@next.com says the NeXT needs this.  */

/* This avoids lossage on SunOS but only if stdtypes.h comes first.
   There's no way to win with the other order!  Sun lossage.  */
/* Sequent's header files use _PTRDIFF_T_ in some conflicting way.
   Just ignore it.  */

/* On VxWorks, <type/vxTypesBase.h> may have defined macros like
   _TYPE_size_t which will typedef size_t.  fixincludes patched the
   vxTypesBase.h so that this macro is only defined if _GCC_SIZE_T is
   not defined, and so that defining this macro defines _GCC_SIZE_T.
   If we find that the macros are still defined at this point, we must
   invoke them so that the type is defined as expected.  */
/* In case nobody has defined these types, but we aren't running under
   GCC 2.00, make sure that __PTRDIFF_TYPE__, __SIZE_TYPE__, and
   __WCHAR_TYPE__ have reasonable values.  This can happen if the
   parts of GCC is compiled by an older compiler, that actually
   include gstddef.h, such as collect2.  */

/* Signed type of difference of two pointers.  */

/* Define this type if we are doing the whole job,
   or if we want this type in particular.  */
typedef long int ptrdiff_t;
/* If this symbol has done its job, get rid of it.  */

/* Unsigned type of `sizeof' something.  */

/* Define this type if we are doing the whole job,
   or if we want this type in particular.  */
typedef long unsigned int size_t;
/* Wide character type.
   Locale-writers should change this as necessary to
   be big enough to hold unique values not between 0 and 127,
   and not (wchar_t) -1, for each defined multibyte character.  */

/* Define this type if we are doing the whole job,
   or if we want this type in particular.  */
/* On BSD/386 1.1, at least, machine/ansi.h defines _BSD_WCHAR_T_
   instead of _WCHAR_T_, and _BSD_RUNE_T_ (which, unlike the other
   symbols in the _FOO_T_ family, stays defined even after its
   corresponding type is defined).  If we define wchar_t, then we
   must undef _WCHAR_T_; for BSD/386 1.1 (and perhaps others), if
   we undef _WCHAR_T_, then we must also define rune_t, since
   headers like runetype.h assume that if machine/ansi.h is included,
   and _BSD_WCHAR_T_ is not defined, then rune_t is available.
   machine/ansi.h says, "Note that _WCHAR_T_ and _RUNE_T_ must be of
   the same type." */
/* FreeBSD 5 can't be handled well using "traditional" logic above
   since it no longer defines _BSD_RUNE_T_ yet still desires to export
   rune_t in some cases... */
typedef int wchar_t;
/* A null pointer constant.  */
/* Offset of member MEMBER in a struct of type TYPE. */
/* Copyright (C) 1997-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/*
 *	ISO C99: 7.18 Integer types <stdint.h>
 */

/* Handle feature test macros at the start of a header.
   Copyright (C) 2016-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* This header is internal to glibc and should not be included outside
   of glibc headers.  Headers including it must define
   __GLIBC_INTERNAL_STARTING_HEADER_IMPLEMENTATION first.  This header
   cannot have multiple include guards because ISO C feature test
   macros depend on the definition of the macro when an affected
   header is included, not when the first system header is
   included.  */

/* Copyright (C) 1991-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* ISO/IEC TR 24731-2:2010 defines the __STDC_WANT_LIB_EXT2__
   macro.  */
/* ISO/IEC TS 18661-1:2014 defines the __STDC_WANT_IEC_60559_BFP_EXT__
   macro.  Most but not all symbols enabled by that macro in TS
   18661-1 are enabled unconditionally in C2X.  In C2X, the symbols in
   Annex F still require a new feature test macro
   __STDC_WANT_IEC_60559_EXT__ instead (C2X does not define
   __STDC_WANT_IEC_60559_BFP_EXT__), while a few features from TS
   18661-1 are not included in C2X (and thus should depend on
   __STDC_WANT_IEC_60559_BFP_EXT__ even when C2X features are
   enabled).

   __GLIBC_USE (IEC_60559_BFP_EXT) controls those features from TS
   18661-1 not included in C2X.

   __GLIBC_USE (IEC_60559_BFP_EXT_C2X) controls those features from TS
   18661-1 that are also included in C2X (with no feature test macro
   required in C2X).

   __GLIBC_USE (IEC_60559_EXT) controls those features from TS 18661-1
   that are included in C2X but conditional on
   __STDC_WANT_IEC_60559_EXT__.  (There are currently no features
   conditional on __STDC_WANT_IEC_60559_EXT__ that are not in TS
   18661-1.)  */
/* ISO/IEC TS 18661-4:2015 defines the
   __STDC_WANT_IEC_60559_FUNCS_EXT__ macro.  Other than the reduction
   functions, the symbols from this TS are enabled unconditionally in
   C2X.  */
/* ISO/IEC TS 18661-3:2015 defines the
   __STDC_WANT_IEC_60559_TYPES_EXT__ macro.  */
/* bits/types.h -- definitions of __*_t types underlying *_t types.
   Copyright (C) 2002-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/*
 * Never include this file directly; use <sys/types.h> instead.
 */
/* wchar_t type related definitions.
   Copyright (C) 2000-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* The fallback definitions, for when __WCHAR_MAX__ or __WCHAR_MIN__
   are not defined, give the right value and type as long as both int
   and wchar_t are 32-bit types.  Adding L'\0' to a constant value
   ensures that the type is correct; it is necessary to use (L'\0' +
   0) rather than just L'\0' so that the type in C++ is the promoted
   version of wchar_t rather than the distinct wchar_t type itself.
   Because wchar_t in preprocessor #if expressions is treated as
   intmax_t or uintmax_t, the expression (L'\0' - 1) would have the
   wrong value for WCHAR_MAX in such expressions and so cannot be used
   to define __WCHAR_MAX in the unsigned case.  */
/* Determine the wordsize from the preprocessor defines.  */
/* Both x86-64 and x32 use the 64-bit system call interface.  */

/* Exact integral types.  */

/* Signed.  */
/* Define intN_t types.
   Copyright (C) 2017-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* bits/types.h -- definitions of __*_t types underlying *_t types.
   Copyright (C) 2002-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/*
 * Never include this file directly; use <sys/types.h> instead.
 */

typedef __int8_t int8_t;
typedef __int16_t int16_t;
typedef __int32_t int32_t;
typedef __int64_t int64_t;

/* Unsigned.  */
/* Define uintN_t types.
   Copyright (C) 2017-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* bits/types.h -- definitions of __*_t types underlying *_t types.
   Copyright (C) 2002-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/*
 * Never include this file directly; use <sys/types.h> instead.
 */

typedef __uint8_t uint8_t;
typedef __uint16_t uint16_t;
typedef __uint32_t uint32_t;
typedef __uint64_t uint64_t;

/* Small types.  */

/* Signed.  */
typedef __int_least8_t int_least8_t;
typedef __int_least16_t int_least16_t;
typedef __int_least32_t int_least32_t;
typedef __int_least64_t int_least64_t;

/* Unsigned.  */
typedef __uint_least8_t uint_least8_t;
typedef __uint_least16_t uint_least16_t;
typedef __uint_least32_t uint_least32_t;
typedef __uint_least64_t uint_least64_t;

/* Fast types.  */

/* Signed.  */
typedef signed char int_fast8_t;

typedef long int int_fast16_t;
typedef long int int_fast32_t;
typedef long int int_fast64_t;

/* Unsigned.  */
typedef unsigned char uint_fast8_t;

typedef unsigned long int uint_fast16_t;
typedef unsigned long int uint_fast32_t;
typedef unsigned long int uint_fast64_t;
/* Types for `void *' pointers.  */

typedef long int intptr_t;

typedef unsigned long int uintptr_t;
/* Largest integral types.  */
typedef __intmax_t intmax_t;
typedef __uintmax_t uintmax_t;
/* Limits of integral types.  */

/* Minimum of signed integral types.  */

/* Maximum of signed integral types.  */

/* Maximum of unsigned integral types.  */

/* Minimum of signed integral types having a minimum size.  */

/* Maximum of signed integral types having a minimum size.  */

/* Maximum of unsigned integral types having a minimum size.  */

/* Minimum of fast signed integral types having a minimum size.  */
/* Maximum of fast signed integral types having a minimum size.  */
/* Maximum of fast unsigned integral types having a minimum size.  */
/* Values to test for integral types holding `void *' pointer.  */
/* Minimum for largest signed integral type.  */

/* Maximum for largest signed integral type.  */

/* Maximum for largest unsigned integral type.  */

/* Limits of other integer types.  */

/* Limits of `ptrdiff_t' type.  */
/* Limits of `sig_atomic_t'.  */

/* Limit of `size_t' type.  */
/* Limits of `wchar_t'.  */

/* These constants might also be defined in <wchar.h>.  */

/* Limits of `wint_t'.  */

/* Signed.  */
/* Unsigned.  */
/* Maximal type.  */
/* Define ISO C stdio on top of C++ iostreams.
   Copyright (C) 1991-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/*
 *	ISO C99 Standard: 7.19 Input/output	<stdio.h>
 */

/* Handle feature test macros at the start of a header.
   Copyright (C) 2016-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* This header is internal to glibc and should not be included outside
   of glibc headers.  Headers including it must define
   __GLIBC_INTERNAL_STARTING_HEADER_IMPLEMENTATION first.  This header
   cannot have multiple include guards because ISO C feature test
   macros depend on the definition of the macro when an affected
   header is included, not when the first system header is
   included.  */

/* Copyright (C) 1991-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* ISO/IEC TR 24731-2:2010 defines the __STDC_WANT_LIB_EXT2__
   macro.  */
/* ISO/IEC TS 18661-1:2014 defines the __STDC_WANT_IEC_60559_BFP_EXT__
   macro.  Most but not all symbols enabled by that macro in TS
   18661-1 are enabled unconditionally in C2X.  In C2X, the symbols in
   Annex F still require a new feature test macro
   __STDC_WANT_IEC_60559_EXT__ instead (C2X does not define
   __STDC_WANT_IEC_60559_BFP_EXT__), while a few features from TS
   18661-1 are not included in C2X (and thus should depend on
   __STDC_WANT_IEC_60559_BFP_EXT__ even when C2X features are
   enabled).

   __GLIBC_USE (IEC_60559_BFP_EXT) controls those features from TS
   18661-1 not included in C2X.

   __GLIBC_USE (IEC_60559_BFP_EXT_C2X) controls those features from TS
   18661-1 that are also included in C2X (with no feature test macro
   required in C2X).

   __GLIBC_USE (IEC_60559_EXT) controls those features from TS 18661-1
   that are included in C2X but conditional on
   __STDC_WANT_IEC_60559_EXT__.  (There are currently no features
   conditional on __STDC_WANT_IEC_60559_EXT__ that are not in TS
   18661-1.)  */
/* ISO/IEC TS 18661-4:2015 defines the
   __STDC_WANT_IEC_60559_FUNCS_EXT__ macro.  Other than the reduction
   functions, the symbols from this TS are enabled unconditionally in
   C2X.  */
/* ISO/IEC TS 18661-3:2015 defines the
   __STDC_WANT_IEC_60559_TYPES_EXT__ macro.  */

/* Copyright (C) 1989-2022 Free Software Foundation, Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3, or (at your option)
any later version.

GCC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

Under Section 7 of GPL version 3, you are granted additional
permissions described in the GCC Runtime Library Exception, version
3.1, as published by the Free Software Foundation.

You should have received a copy of the GNU General Public License and
a copy of the GCC Runtime Library Exception along with this program;
see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
<http://www.gnu.org/licenses/>.  */

/*
 * ISO C Standard:  7.17  Common definitions  <stddef.h>
 */

/* Any one of these symbols __need_* means that GNU libc
   wants us just to define one data type.  So don't define
   the symbols that indicate this file's entire job has been done.  */
/* This avoids lossage on SunOS but only if stdtypes.h comes first.
   There's no way to win with the other order!  Sun lossage.  */
/* Sequent's header files use _PTRDIFF_T_ in some conflicting way.
   Just ignore it.  */

/* On VxWorks, <type/vxTypesBase.h> may have defined macros like
   _TYPE_size_t which will typedef size_t.  fixincludes patched the
   vxTypesBase.h so that this macro is only defined if _GCC_SIZE_T is
   not defined, and so that defining this macro defines _GCC_SIZE_T.
   If we find that the macros are still defined at this point, we must
   invoke them so that the type is defined as expected.  */
/* In case nobody has defined these types, but we aren't running under
   GCC 2.00, make sure that __PTRDIFF_TYPE__, __SIZE_TYPE__, and
   __WCHAR_TYPE__ have reasonable values.  This can happen if the
   parts of GCC is compiled by an older compiler, that actually
   include gstddef.h, such as collect2.  */

/* Signed type of difference of two pointers.  */

/* Define this type if we are doing the whole job,
   or if we want this type in particular.  */
/* If this symbol has done its job, get rid of it.  */

/* Unsigned type of `sizeof' something.  */

/* Define this type if we are doing the whole job,
   or if we want this type in particular.  */
/* Wide character type.
   Locale-writers should change this as necessary to
   be big enough to hold unique values not between 0 and 127,
   and not (wchar_t) -1, for each defined multibyte character.  */

/* Define this type if we are doing the whole job,
   or if we want this type in particular.  */
/* A null pointer constant.  */
/* Offset of member MEMBER in a struct of type TYPE. */

/* Copyright (C) 1989-2022 Free Software Foundation, Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3, or (at your option)
any later version.

GCC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

Under Section 7 of GPL version 3, you are granted additional
permissions described in the GCC Runtime Library Exception, version
3.1, as published by the Free Software Foundation.

You should have received a copy of the GNU General Public License and
a copy of the GCC Runtime Library Exception along with this program;
see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
<http://www.gnu.org/licenses/>.  */

/*
 * ISO C Standard:  7.15  Variable arguments  <stdarg.h>
 */

/* bits/types.h -- definitions of __*_t types underlying *_t types.
   Copyright (C) 2002-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/*
 * Never include this file directly; use <sys/types.h> instead.
 */

/* bits/types.h -- definitions of __*_t types underlying *_t types.
   Copyright (C) 2002-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/*
 * Never include this file directly; use <sys/types.h> instead.
 */

/* Integral type unchanged by default argument promotions that can
   hold any value corresponding to members of the extended character
   set, as well as at least one value that does not correspond to any
   member of the extended character set.  */

/* Conversion state information.  */
typedef struct {
    int __count;
    union {
        unsigned int __wch;
        char __wchb[4];
    } __value; /* Value so far.  */
} __mbstate_t;

/* The tag name of this struct is _G_fpos_t to preserve historic
   C++ mangled names for functions taking fpos_t arguments.
   That name should not be used in new code.  */
typedef struct _G_fpos_t {
    __off_t __pos;
    __mbstate_t __state;
} __fpos_t;

/* bits/types.h -- definitions of __*_t types underlying *_t types.
   Copyright (C) 2002-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/*
 * Never include this file directly; use <sys/types.h> instead.
 */

/* The tag name of this struct is _G_fpos64_t to preserve historic
   C++ mangled names for functions taking fpos_t and/or fpos64_t
   arguments.  That name should not be used in new code.  */
typedef struct _G_fpos64_t {
    __off64_t __pos;
    __mbstate_t __state;
} __fpos64_t;

struct _IO_FILE;
typedef struct _IO_FILE __FILE;

struct _IO_FILE;

/* The opaque type of streams.  This is the definition used elsewhere.  */
typedef struct _IO_FILE FILE;
/* Copyright (C) 1991-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* Caution: The contents of this file are not part of the official
   stdio.h API.  However, much of it is part of the official *binary*
   interface, and therefore cannot be changed.  */
/* bits/types.h -- definitions of __*_t types underlying *_t types.
   Copyright (C) 2002-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/*
 * Never include this file directly; use <sys/types.h> instead.
 */

struct _IO_FILE;
struct _IO_marker;
struct _IO_codecvt;
struct _IO_wide_data;

/* During the build of glibc itself, _IO_lock_t will already have been
   defined by internal headers.  */

typedef void _IO_lock_t;

/* The tag name of this struct is _IO_FILE to preserve historic
   C++ mangled names for functions taking FILE* arguments.
   That name should not be used in new code.  */
struct _IO_FILE {
    int _flags; /* High-order word is _IO_MAGIC; rest is flags. */

    /* The following pointers correspond to the C++ streambuf protocol. */
    char *_IO_read_ptr;   /* Current read pointer */
    char *_IO_read_end;   /* End of get area. */
    char *_IO_read_base;  /* Start of putback+get area. */
    char *_IO_write_base; /* Start of put area. */
    char *_IO_write_ptr;  /* Current put pointer. */
    char *_IO_write_end;  /* End of put area. */
    char *_IO_buf_base;   /* Start of reserve area. */
    char *_IO_buf_end;    /* End of reserve area. */

    /* The following fields are used to support backing up and undo. */
    char *_IO_save_base;   /* Pointer to start of non-current get area. */
    char *_IO_backup_base; /* Pointer to first valid character of backup area */
    char *_IO_save_end;    /* Pointer to end of non-current get area. */

    struct _IO_marker *_markers;

    struct _IO_FILE *_chain;

    int _fileno;
    int _flags2;
    __off_t _old_offset; /* This used to be _offset but it's too small.  */

    /* 1+column number of pbase(); 0 is unknown. */
    unsigned short _cur_column;
    signed char _vtable_offset;
    char _shortbuf[1];

    _IO_lock_t *_lock;

    __off64_t _offset;
    /* Wide character stream stuff.  */
    struct _IO_codecvt *_codecvt;
    struct _IO_wide_data *_wide_data;
    struct _IO_FILE *_freeres_list;
    void *_freeres_buf;
    size_t __pad5;
    int _mode;
    /* Make sure we don't get into trouble again.  */
    char _unused2[15 * sizeof(int) - 4 * sizeof(void *) - sizeof(size_t)];
};

/* These macros are used by bits/stdio.h and internal headers.  */
/* Many more flag bits are defined internally.  */

/* Copyright (C) 1991-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* bits/types.h -- definitions of __*_t types underlying *_t types.
   Copyright (C) 2002-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/*
 * Never include this file directly; use <sys/types.h> instead.
 */

/* Functions to do I/O and file management for a stream.  */

/* Read NBYTES bytes from COOKIE into a buffer pointed to by BUF.
   Return number of bytes read.  */
typedef __ssize_t cookie_read_function_t(void *__cookie, char *__buf, size_t __nbytes);

/* Write NBYTES bytes pointed to by BUF to COOKIE.  Write all NBYTES bytes
   unless there is an error.  Return number of bytes written.  If
   there is an error, return 0 and do not write anything.  If the file
   has been opened for append (__mode.__append set), then set the file
   pointer to the end of the file and then do the write; if not, just
   write at the current file pointer.  */
typedef __ssize_t cookie_write_function_t(void *__cookie, const char *__buf, size_t __nbytes);

/* Move COOKIE's file position to *POS bytes from the
   beginning of the file (if W is SEEK_SET),
   the current position (if W is SEEK_CUR),
   or the end of the file (if W is SEEK_END).
   Set *POS to the new file position.
   Returns zero if successful, nonzero if not.  */
typedef int cookie_seek_function_t(void *__cookie, __off64_t *__pos, int __w);

/* Close COOKIE.  */
typedef int cookie_close_function_t(void *__cookie);

/* The structure with the cookie function pointers.
   The tag name of this struct is _IO_cookie_io_functions_t to
   preserve historic C++ mangled names for functions taking
   cookie_io_functions_t arguments.  That name should not be used in
   new code.  */
typedef struct _IO_cookie_io_functions_t {
    cookie_read_function_t *read;   /* Read bytes.  */
    cookie_write_function_t *write; /* Write bytes.  */
    cookie_seek_function_t *seek;   /* Seek/tell file position.  */
    cookie_close_function_t *close; /* Close file.  */
} cookie_io_functions_t;
typedef __off_t off_t;

typedef __off64_t off64_t;

typedef __ssize_t ssize_t;

/* The type of the second argument to `fgetpos' and `fsetpos'.  */

typedef __fpos_t fpos_t;

typedef __fpos64_t fpos64_t;

/* The possibilities for the third argument to `setvbuf'.  */

/* Default buffer size.  */

/* The value returned by fgetc and similar functions to indicate the
   end of the file.  */

/* The possibilities for the third argument to `fseek'.
   These values should not be changed.  */
/* Default path prefix for `tempnam' and `tmpnam'.  */

/* Get the values:
   FILENAME_MAX	Maximum length of a filename.  */
/* System specific stdio.h definitions.  Linux version.
   Copyright (C) 2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */
/* Maximum length of printf output for a NaN.  */

/* Standard streams.  */
extern FILE *stdin;  /* Standard input stream.  */
extern FILE *stdout; /* Standard output stream.  */
extern FILE *stderr; /* Standard error output stream.  */
/* C89/C99 say they're macros.  Make them happy.  */

/* Remove file FILENAME.  */
extern int remove(const char *__filename) __attribute__((__nothrow__, __leaf__));
/* Rename file OLD to NEW.  */
extern int rename(const char *__old, const char *__new) __attribute__((__nothrow__, __leaf__));

/* Rename file OLD relative to OLDFD to NEW relative to NEWFD.  */
extern int renameat(int __oldfd, const char *__old, int __newfd, const char *__new) __attribute__((__nothrow__, __leaf__));

/* Flags for renameat2.  */

/* Rename file OLD relative to OLDFD to NEW relative to NEWFD, with
   additional flags.  */
extern int renameat2(int __oldfd, const char *__old, int __newfd, const char *__new, unsigned int __flags) __attribute__((__nothrow__, __leaf__));

/* Close STREAM.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern int fclose(FILE *__stream) __attribute__((__nonnull__(1)));

/* Create a temporary file and open it read/write.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */

extern FILE *tmpfile(void) __attribute__((__malloc__)) __attribute__((__malloc__(fclose, 1))) __attribute__((__warn_unused_result__));
extern FILE *tmpfile64(void) __attribute__((__malloc__)) __attribute__((__malloc__(fclose, 1))) __attribute__((__warn_unused_result__));

/* Generate a temporary filename.  */
extern char *tmpnam(char[20]) __attribute__((__nothrow__, __leaf__)) __attribute__((__warn_unused_result__));

/* This is the reentrant variant of `tmpnam'.  The only difference is
   that it does not allow S to be NULL.  */
extern char *tmpnam_r(char __s[20]) __attribute__((__nothrow__, __leaf__)) __attribute__((__warn_unused_result__));

/* Generate a unique temporary filename using up to five characters of PFX
   if it is not NULL.  The directory to put this file in is searched for
   as follows: First the environment variable "TMPDIR" is checked.
   If it contains the name of a writable directory, that directory is used.
   If not and if DIR is not NULL, that value is checked.  If that fails,
   P_tmpdir is tried and finally "/tmp".  The storage for the filename
   is allocated by `malloc'.  */
extern char *tempnam(const char *__dir, const char *__pfx) __attribute__((__nothrow__, __leaf__)) __attribute__((__malloc__))
__attribute__((__warn_unused_result__)) __attribute__((__malloc__(__builtin_free, 1)));

/* Flush STREAM, or all streams if STREAM is NULL.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern int fflush(FILE *__stream);

/* Faster versions when locking is not required.

   This function is not part of POSIX and therefore no official
   cancellation point.  But due to similarity with an POSIX interface
   or due to the implementation it is a cancellation point and
   therefore not marked with __THROW.  */
extern int fflush_unlocked(FILE *__stream);

/* Close all streams.

   This function is not part of POSIX and therefore no official
   cancellation point.  But due to similarity with an POSIX interface
   or due to the implementation it is a cancellation point and
   therefore not marked with __THROW.  */
extern int fcloseall(void);

/* Open a file and create a new stream for it.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern FILE *fopen(const char *__restrict __filename, const char *__restrict __modes) __attribute__((__malloc__)) __attribute__((__malloc__(fclose, 1)))
__attribute__((__warn_unused_result__));
/* Open a file, replacing an existing stream with it.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern FILE *freopen(const char *__restrict __filename, const char *__restrict __modes, FILE *__restrict __stream) __attribute__((__warn_unused_result__))
__attribute__((__nonnull__(3)));
extern FILE *fopen64(const char *__restrict __filename, const char *__restrict __modes) __attribute__((__malloc__)) __attribute__((__malloc__(fclose, 1)))
__attribute__((__warn_unused_result__));
extern FILE *freopen64(const char *__restrict __filename, const char *__restrict __modes, FILE *__restrict __stream) __attribute__((__warn_unused_result__))
__attribute__((__nonnull__(3)));

/* Create a new stream that refers to an existing system file descriptor.  */
extern FILE *fdopen(int __fd, const char *__modes) __attribute__((__nothrow__, __leaf__)) __attribute__((__malloc__)) __attribute__((__malloc__(fclose, 1)))
__attribute__((__warn_unused_result__));

/* Create a new stream that refers to the given magic cookie,
   and uses the given functions for input and output.  */
extern FILE *fopencookie(void *__restrict __magic_cookie, const char *__restrict __modes, cookie_io_functions_t __io_funcs)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__malloc__)) __attribute__((__malloc__(fclose, 1))) __attribute__((__warn_unused_result__));

/* Create a new stream that refers to a memory buffer.  */
extern FILE *fmemopen(void *__s, size_t __len, const char *__modes) __attribute__((__nothrow__, __leaf__)) __attribute__((__malloc__))
__attribute__((__malloc__(fclose, 1))) __attribute__((__warn_unused_result__));

/* Open a stream that writes into a malloc'd buffer that is expanded as
   necessary.  *BUFLOC and *SIZELOC are updated with the buffer's location
   and the number of characters written on fflush or fclose.  */
extern FILE *open_memstream(char **__bufloc, size_t *__sizeloc) __attribute__((__nothrow__, __leaf__)) __attribute__((__malloc__))
__attribute__((__malloc__(fclose, 1))) __attribute__((__warn_unused_result__));
/* If BUF is NULL, make STREAM unbuffered.
   Else make it use buffer BUF, of size BUFSIZ.  */
extern void setbuf(FILE *__restrict __stream, char *__restrict __buf) __attribute__((__nothrow__, __leaf__));
/* Make STREAM use buffering mode MODE.
   If BUF is not NULL, use N bytes of it for buffering;
   else allocate an internal buffer N bytes long.  */
extern int setvbuf(FILE *__restrict __stream, char *__restrict __buf, int __modes, size_t __n) __attribute__((__nothrow__, __leaf__));

/* If BUF is NULL, make STREAM unbuffered.
   Else make it use SIZE bytes of BUF for buffering.  */
extern void setbuffer(FILE *__restrict __stream, char *__restrict __buf, size_t __size) __attribute__((__nothrow__, __leaf__));

/* Make STREAM line-buffered.  */
extern void setlinebuf(FILE *__stream) __attribute__((__nothrow__, __leaf__));

/* Write formatted output to STREAM.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern int fprintf(FILE *__restrict __stream, const char *__restrict __format, ...);
/* Write formatted output to stdout.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern int printf(const char *__restrict __format, ...);
/* Write formatted output to S.  */
extern int sprintf(char *__restrict __s, const char *__restrict __format, ...) __attribute__((__nothrow__));

/* Write formatted output to S from argument list ARG.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern int vfprintf(FILE *__restrict __s, const char *__restrict __format, __gnuc_va_list __arg);
/* Write formatted output to stdout from argument list ARG.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern int vprintf(const char *__restrict __format, __gnuc_va_list __arg);
/* Write formatted output to S from argument list ARG.  */
extern int vsprintf(char *__restrict __s, const char *__restrict __format, __gnuc_va_list __arg) __attribute__((__nothrow__));

/* Maximum chars of output to write in MAXLEN.  */
extern int snprintf(char *__restrict __s, size_t __maxlen, const char *__restrict __format, ...) __attribute__((__nothrow__))
__attribute__((__format__(__printf__, 3, 4)));

extern int vsnprintf(char *__restrict __s, size_t __maxlen, const char *__restrict __format, __gnuc_va_list __arg) __attribute__((__nothrow__))
__attribute__((__format__(__printf__, 3, 0)));

/* Write formatted output to a string dynamically allocated with `malloc'.
   Store the address of the string in *PTR.  */
extern int vasprintf(char **__restrict __ptr, const char *__restrict __f, __gnuc_va_list __arg) __attribute__((__nothrow__))
__attribute__((__format__(__printf__, 2, 0))) __attribute__((__warn_unused_result__));
extern int __asprintf(char **__restrict __ptr, const char *__restrict __fmt, ...) __attribute__((__nothrow__)) __attribute__((__format__(__printf__, 2, 3)))
__attribute__((__warn_unused_result__));
extern int asprintf(char **__restrict __ptr, const char *__restrict __fmt, ...) __attribute__((__nothrow__)) __attribute__((__format__(__printf__, 2, 3)))
__attribute__((__warn_unused_result__));

/* Write formatted output to a file descriptor.  */
extern int vdprintf(int __fd, const char *__restrict __fmt, __gnuc_va_list __arg) __attribute__((__format__(__printf__, 2, 0)));
extern int dprintf(int __fd, const char *__restrict __fmt, ...) __attribute__((__format__(__printf__, 2, 3)));

/* Read formatted input from STREAM.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern int fscanf(FILE *__restrict __stream, const char *__restrict __format, ...) __attribute__((__warn_unused_result__));
/* Read formatted input from stdin.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern int scanf(const char *__restrict __format, ...) __attribute__((__warn_unused_result__));
/* Read formatted input from S.  */
extern int sscanf(const char *__restrict __s, const char *__restrict __format, ...) __attribute__((__nothrow__, __leaf__));

/* For historical reasons, the C99-compliant versions of the scanf
   functions are at alternative names.  When __LDBL_COMPAT or
   __LDOUBLE_REDIRECTS_TO_FLOAT128_ABI are in effect, this is handled in
   bits/stdio-ldbl.h.  */
/* Macros to control TS 18661-3 glibc features on x86.
   Copyright (C) 2017-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

extern int fscanf(FILE *__restrict __stream, const char *__restrict __format, ...) __asm__(""
                                                                                           "__isoc23_fscanf")

    __attribute__((__warn_unused_result__));
extern int scanf(const char *__restrict __format, ...) __asm__(""
                                                               "__isoc23_scanf") __attribute__((__warn_unused_result__));
extern int sscanf(const char *__restrict __s, const char *__restrict __format, ...) __asm__(""
                                                                                            "__isoc23_sscanf") __attribute__((__nothrow__, __leaf__))

;
/* Read formatted input from S into argument list ARG.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern int vfscanf(FILE *__restrict __s, const char *__restrict __format, __gnuc_va_list __arg) __attribute__((__format__(__scanf__, 2, 0)))
__attribute__((__warn_unused_result__));

/* Read formatted input from stdin into argument list ARG.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern int vscanf(const char *__restrict __format, __gnuc_va_list __arg) __attribute__((__format__(__scanf__, 1, 0))) __attribute__((__warn_unused_result__));

/* Read formatted input from S into argument list ARG.  */
extern int vsscanf(const char *__restrict __s, const char *__restrict __format, __gnuc_va_list __arg) __attribute__((__nothrow__, __leaf__))
__attribute__((__format__(__scanf__, 2, 0)));

/* Same redirection as above for the v*scanf family.  */

extern int vfscanf(FILE *__restrict __s, const char *__restrict __format, __gnuc_va_list __arg) __asm__(""
                                                                                                        "__isoc23_vfscanf")

    __attribute__((__format__(__scanf__, 2, 0))) __attribute__((__warn_unused_result__));
extern int vscanf(const char *__restrict __format, __gnuc_va_list __arg) __asm__(""
                                                                                 "__isoc23_vscanf")

    __attribute__((__format__(__scanf__, 1, 0))) __attribute__((__warn_unused_result__));
extern int vsscanf(const char *__restrict __s, const char *__restrict __format, __gnuc_va_list __arg) __asm__(""
                                                                                                              "__isoc23_vsscanf")
    __attribute__((__nothrow__, __leaf__))

    __attribute__((__format__(__scanf__, 2, 0)));
/* Read a character from STREAM.

   These functions are possible cancellation points and therefore not
   marked with __THROW.  */
extern int fgetc(FILE *__stream);
extern int getc(FILE *__stream);

/* Read a character from stdin.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern int getchar(void);

/* These are defined in POSIX.1:1996.

   These functions are possible cancellation points and therefore not
   marked with __THROW.  */
extern int getc_unlocked(FILE *__stream);
extern int getchar_unlocked(void);

/* Faster version when locking is not necessary.

   This function is not part of POSIX and therefore no official
   cancellation point.  But due to similarity with an POSIX interface
   or due to the implementation it is a cancellation point and
   therefore not marked with __THROW.  */
extern int fgetc_unlocked(FILE *__stream);

/* Write a character to STREAM.

   These functions are possible cancellation points and therefore not
   marked with __THROW.

   These functions is a possible cancellation point and therefore not
   marked with __THROW.  */
extern int fputc(int __c, FILE *__stream);
extern int putc(int __c, FILE *__stream);

/* Write a character to stdout.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern int putchar(int __c);

/* Faster version when locking is not necessary.

   This function is not part of POSIX and therefore no official
   cancellation point.  But due to similarity with an POSIX interface
   or due to the implementation it is a cancellation point and
   therefore not marked with __THROW.  */
extern int fputc_unlocked(int __c, FILE *__stream);

/* These are defined in POSIX.1:1996.

   These functions are possible cancellation points and therefore not
   marked with __THROW.  */
extern int putc_unlocked(int __c, FILE *__stream);
extern int putchar_unlocked(int __c);

/* Get a word (int) from STREAM.  */
extern int getw(FILE *__stream);

/* Write a word (int) to STREAM.  */
extern int putw(int __w, FILE *__stream);

/* Get a newline-terminated string of finite length from STREAM.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern char *fgets(char *__restrict __s, int __n, FILE *__restrict __stream) __attribute__((__warn_unused_result__))
__attribute__((__access__(__write_only__, 1)));
/* This function does the same as `fgets' but does not lock the stream.

   This function is not part of POSIX and therefore no official
   cancellation point.  But due to similarity with an POSIX interface
   or due to the implementation it is a cancellation point and
   therefore not marked with __THROW.  */
extern char *fgets_unlocked(char *__restrict __s, int __n, FILE *__restrict __stream) __attribute__((__warn_unused_result__))
__attribute__((__access__(__write_only__, 1)));

/* Read up to (and including) a DELIMITER from STREAM into *LINEPTR
   (and null-terminate it). *LINEPTR is a pointer returned from malloc (or
   NULL), pointing to *N characters of space.  It is realloc'd as
   necessary.  Returns the number of characters read (not including the
   null terminator), or -1 on error or EOF.

   These functions are not part of POSIX and therefore no official
   cancellation point.  But due to similarity with an POSIX interface
   or due to the implementation they are cancellation points and
   therefore not marked with __THROW.  */
extern __ssize_t __getdelim(char **__restrict __lineptr, size_t *__restrict __n, int __delimiter, FILE *__restrict __stream)
    __attribute__((__warn_unused_result__));
extern __ssize_t getdelim(char **__restrict __lineptr, size_t *__restrict __n, int __delimiter, FILE *__restrict __stream)
    __attribute__((__warn_unused_result__));

/* Like `getdelim', but reads up to a newline.

   This function is not part of POSIX and therefore no official
   cancellation point.  But due to similarity with an POSIX interface
   or due to the implementation it is a cancellation point and
   therefore not marked with __THROW.  */
extern __ssize_t getline(char **__restrict __lineptr, size_t *__restrict __n, FILE *__restrict __stream) __attribute__((__warn_unused_result__));

/* Write a string to STREAM.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern int fputs(const char *__restrict __s, FILE *__restrict __stream);

/* Write a string, followed by a newline, to stdout.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern int puts(const char *__s);

/* Push a character back onto the input buffer of STREAM.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern int ungetc(int __c, FILE *__stream);

/* Read chunks of generic data from STREAM.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern size_t fread(void *__restrict __ptr, size_t __size, size_t __n, FILE *__restrict __stream) __attribute__((__warn_unused_result__));
/* Write chunks of generic data to STREAM.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern size_t fwrite(const void *__restrict __ptr, size_t __size, size_t __n, FILE *__restrict __s);

/* This function does the same as `fputs' but does not lock the stream.

   This function is not part of POSIX and therefore no official
   cancellation point.  But due to similarity with an POSIX interface
   or due to the implementation it is a cancellation point and
   therefore not marked with __THROW.  */
extern int fputs_unlocked(const char *__restrict __s, FILE *__restrict __stream);

/* Faster versions when locking is not necessary.

   These functions are not part of POSIX and therefore no official
   cancellation point.  But due to similarity with an POSIX interface
   or due to the implementation they are cancellation points and
   therefore not marked with __THROW.  */
extern size_t fread_unlocked(void *__restrict __ptr, size_t __size, size_t __n, FILE *__restrict __stream) __attribute__((__warn_unused_result__));
extern size_t fwrite_unlocked(const void *__restrict __ptr, size_t __size, size_t __n, FILE *__restrict __stream);

/* Seek to a certain position on STREAM.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern int fseek(FILE *__stream, long int __off, int __whence);
/* Return the current position of STREAM.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern long int ftell(FILE *__stream) __attribute__((__warn_unused_result__));
/* Rewind to the beginning of STREAM.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern void rewind(FILE *__stream);

/* The Single Unix Specification, Version 2, specifies an alternative,
   more adequate interface for the two functions above which deal with
   file offset.  `long int' is not the right type.  These definitions
   are originally defined in the Large File Support API.  */

/* Seek to a certain position on STREAM.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern int fseeko(FILE *__stream, __off_t __off, int __whence);
/* Return the current position of STREAM.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern __off_t ftello(FILE *__stream) __attribute__((__warn_unused_result__));
/* Get STREAM's position.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern int fgetpos(FILE *__restrict __stream, fpos_t *__restrict __pos);
/* Set STREAM's position.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern int fsetpos(FILE *__stream, const fpos_t *__pos);
extern int fseeko64(FILE *__stream, __off64_t __off, int __whence);
extern __off64_t ftello64(FILE *__stream) __attribute__((__warn_unused_result__));
extern int fgetpos64(FILE *__restrict __stream, fpos64_t *__restrict __pos);
extern int fsetpos64(FILE *__stream, const fpos64_t *__pos);

/* Clear the error and EOF indicators for STREAM.  */
extern void clearerr(FILE *__stream) __attribute__((__nothrow__, __leaf__));
/* Return the EOF indicator for STREAM.  */
extern int feof(FILE *__stream) __attribute__((__nothrow__, __leaf__)) __attribute__((__warn_unused_result__));
/* Return the error indicator for STREAM.  */
extern int ferror(FILE *__stream) __attribute__((__nothrow__, __leaf__)) __attribute__((__warn_unused_result__));

/* Faster versions when locking is not required.  */
extern void clearerr_unlocked(FILE *__stream) __attribute__((__nothrow__, __leaf__));
extern int feof_unlocked(FILE *__stream) __attribute__((__nothrow__, __leaf__)) __attribute__((__warn_unused_result__));
extern int ferror_unlocked(FILE *__stream) __attribute__((__nothrow__, __leaf__)) __attribute__((__warn_unused_result__));

/* Print a message describing the meaning of the value of errno.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern void perror(const char *__s) __attribute__((__cold__));

/* Return the system file descriptor for STREAM.  */
extern int fileno(FILE *__stream) __attribute__((__nothrow__, __leaf__)) __attribute__((__warn_unused_result__));

/* Faster version when locking is not required.  */
extern int fileno_unlocked(FILE *__stream) __attribute__((__nothrow__, __leaf__)) __attribute__((__warn_unused_result__));

/* Close a stream opened by popen and return the status of its child.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern int pclose(FILE *__stream);

/* Create a new stream connected to a pipe running the given command.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern FILE *popen(const char *__command, const char *__modes) __attribute__((__malloc__)) __attribute__((__malloc__(pclose, 1)))
__attribute__((__warn_unused_result__));

/* Return the name of the controlling terminal.  */
extern char *ctermid(char *__s) __attribute__((__nothrow__, __leaf__)) __attribute__((__access__(__write_only__, 1)));

/* Return the name of the current user.  */
extern char *cuserid(char *__s) __attribute__((__access__(__write_only__, 1)));

struct obstack; /* See <obstack.h>.  */

/* Write formatted output to an obstack.  */
extern int obstack_printf(struct obstack *__restrict __obstack, const char *__restrict __format, ...) __attribute__((__nothrow__))
__attribute__((__format__(__printf__, 2, 3)));
extern int obstack_vprintf(struct obstack *__restrict __obstack, const char *__restrict __format, __gnuc_va_list __args) __attribute__((__nothrow__))
__attribute__((__format__(__printf__, 2, 0)));

/* These are defined in POSIX.1:1996.  */

/* Acquire ownership of STREAM.  */
extern void flockfile(FILE *__stream) __attribute__((__nothrow__, __leaf__));

/* Try to acquire ownership of STREAM but do not block if it is not
   possible.  */
extern int ftrylockfile(FILE *__stream) __attribute__((__nothrow__, __leaf__)) __attribute__((__warn_unused_result__));

/* Relinquish the ownership granted for STREAM.  */
extern void funlockfile(FILE *__stream) __attribute__((__nothrow__, __leaf__));
/* Slow-path routines used by the optimized inline functions in
   bits/stdio.h.  */
extern int __uflow(FILE *);
extern int __overflow(FILE *, int);

/* Declare all functions from bits/stdio2-decl.h first.  */
/* Checking macros for stdio functions. Declarations only.
   Copyright (C) 2004-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */
extern int __sprintf_chk(char *__restrict __s, int __flag, size_t __slen, const char *__restrict __format, ...) __attribute__((__nothrow__, __leaf__))
__attribute__((__access__(__write_only__, 1, 3)));
extern int __vsprintf_chk(char *__restrict __s, int __flag, size_t __slen, const char *__restrict __format, __gnuc_va_list __ap)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__access__(__write_only__, 1, 3)));

extern int __snprintf_chk(char *__restrict __s, size_t __n, int __flag, size_t __slen, const char *__restrict __format, ...)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__access__(__write_only__, 1, 2)));
extern int __vsnprintf_chk(char *__restrict __s, size_t __n, int __flag, size_t __slen, const char *__restrict __format, __gnuc_va_list __ap)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__access__(__write_only__, 1, 2)));

extern int __fprintf_chk(FILE *__restrict __stream, int __flag, const char *__restrict __format, ...);
extern int __printf_chk(int __flag, const char *__restrict __format, ...);
extern int __vfprintf_chk(FILE *__restrict __stream, int __flag, const char *__restrict __format, __gnuc_va_list __ap);
extern int __vprintf_chk(int __flag, const char *__restrict __format, __gnuc_va_list __ap);

extern int __dprintf_chk(int __fd, int __flag, const char *__restrict __fmt, ...) __attribute__((__format__(__printf__, 3, 4)));
extern int __vdprintf_chk(int __fd, int __flag, const char *__restrict __fmt, __gnuc_va_list __arg) __attribute__((__format__(__printf__, 3, 0)));

extern int __asprintf_chk(char **__restrict __ptr, int __flag, const char *__restrict __fmt, ...) __attribute__((__nothrow__, __leaf__))
__attribute__((__format__(__printf__, 3, 4))) __attribute__((__warn_unused_result__));
extern int __vasprintf_chk(char **__restrict __ptr, int __flag, const char *__restrict __fmt, __gnuc_va_list __arg) __attribute__((__nothrow__, __leaf__))
__attribute__((__format__(__printf__, 3, 0))) __attribute__((__warn_unused_result__));
extern int __obstack_printf_chk(struct obstack *__restrict __obstack, int __flag, const char *__restrict __format, ...) __attribute__((__nothrow__, __leaf__))
__attribute__((__format__(__printf__, 3, 4)));
extern int __obstack_vprintf_chk(struct obstack *__restrict __obstack, int __flag, const char *__restrict __format, __gnuc_va_list __args)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__format__(__printf__, 3, 0)));
extern char *__fgets_alias(char *__restrict __s, int __n, FILE *__restrict __stream) __asm__(""
                                                                                             "fgets")

    __attribute__((__warn_unused_result__)) __attribute__((__access__(__write_only__, 1, 2)));
extern char *__fgets_chk_warn(char *__restrict __s, size_t __size, int __n, FILE *__restrict __stream) __asm__(""
                                                                                                               "__fgets_chk")

    __attribute__((__warn_unused_result__)) __attribute__((__warning__("fgets called with bigger size than length "
                                                                       "of destination buffer")));

extern char *__fgets_chk(char *__restrict __s, size_t __size, int __n, FILE *__restrict __stream) __attribute__((__warn_unused_result__))
__attribute__((__access__(__write_only__, 1, 3)));

extern size_t __fread_alias(void *__restrict __ptr, size_t __size, size_t __n, FILE *__restrict __stream) __asm__(""
                                                                                                                  "fread")

    __attribute__((__warn_unused_result__));
extern size_t __fread_chk_warn(void *__restrict __ptr, size_t __ptrlen, size_t __size, size_t __n, FILE *__restrict __stream) __asm__(""
                                                                                                                                      "__fread_chk")

    __attribute__((__warn_unused_result__)) __attribute__((__warning__("fread called with bigger size * nmemb than length "
                                                                       "of destination buffer")));

extern size_t __fread_chk(void *__restrict __ptr, size_t __ptrlen, size_t __size, size_t __n, FILE *__restrict __stream)
    __attribute__((__warn_unused_result__));

extern char *__fgets_unlocked_alias(char *__restrict __s, int __n, FILE *__restrict __stream) __asm__(""
                                                                                                      "fgets_unlocked")

    __attribute__((__warn_unused_result__)) __attribute__((__access__(__write_only__, 1, 2)));
extern char *__fgets_unlocked_chk_warn(char *__restrict __s, size_t __size, int __n, FILE *__restrict __stream) __asm__(""
                                                                                                                        "__fgets_unlocked_chk")

    __attribute__((__warn_unused_result__)) __attribute__((__warning__("fgets_unlocked called with bigger size than length "
                                                                       "of destination buffer")));

extern char *__fgets_unlocked_chk(char *__restrict __s, size_t __size, int __n, FILE *__restrict __stream) __attribute__((__warn_unused_result__))
__attribute__((__access__(__write_only__, 1, 3)));

extern size_t __fread_unlocked_alias(void *__restrict __ptr, size_t __size, size_t __n, FILE *__restrict __stream) __asm__(""
                                                                                                                           "fread_unlocked")

    __attribute__((__warn_unused_result__));
extern size_t __fread_unlocked_chk_warn(void *__restrict __ptr, size_t __ptrlen, size_t __size, size_t __n, FILE *__restrict __stream) __asm__(
    ""
    "__fread_unlocked_chk")

    __attribute__((__warn_unused_result__)) __attribute__((__warning__("fread_unlocked called with bigger size * nmemb than "
                                                                       "length of destination buffer")));

extern size_t __fread_unlocked_chk(void *__restrict __ptr, size_t __ptrlen, size_t __size, size_t __n, FILE *__restrict __stream)
    __attribute__((__warn_unused_result__));

/* The following headers provide asm redirections.  These redirections must
   appear before the first usage of these functions, e.g. in bits/stdio.h.  */

/* If we are compiling with optimizing read this file.  It contains
   several optimizing inline functions and macros.  */

/* Optimizing macros and inline functions for stdio functions.
   Copyright (C) 1998-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */
/* For -D_FORTIFY_SOURCE{,=2,=3} bits/stdio2.h will define a different
   inline.  */
/* Read a character from stdin.  */
extern __inline __attribute__((__gnu_inline__)) int
getchar(void)
{
    return getc(stdin);
}

/* Faster version when locking is not necessary.  */
extern __inline __attribute__((__gnu_inline__)) int
fgetc_unlocked(FILE *__fp)
{
    return (__builtin_expect(((__fp)->_IO_read_ptr >= (__fp)->_IO_read_end), 0) ? __uflow(__fp) : *(unsigned char *)(__fp)->_IO_read_ptr++);
}

/* This is defined in POSIX.1:1996.  */
extern __inline __attribute__((__gnu_inline__)) int
getc_unlocked(FILE *__fp)
{
    return (__builtin_expect(((__fp)->_IO_read_ptr >= (__fp)->_IO_read_end), 0) ? __uflow(__fp) : *(unsigned char *)(__fp)->_IO_read_ptr++);
}

/* This is defined in POSIX.1:1996.  */
extern __inline __attribute__((__gnu_inline__)) int
getchar_unlocked(void)
{
    return (__builtin_expect(((stdin)->_IO_read_ptr >= (stdin)->_IO_read_end), 0) ? __uflow(stdin) : *(unsigned char *)(stdin)->_IO_read_ptr++);
}

/* Write a character to stdout.  */
extern __inline __attribute__((__gnu_inline__)) int
putchar(int __c)
{
    return putc(__c, stdout);
}

/* Faster version when locking is not necessary.  */
extern __inline __attribute__((__gnu_inline__)) int
fputc_unlocked(int __c, FILE *__stream)
{
    return (__builtin_expect(((__stream)->_IO_write_ptr >= (__stream)->_IO_write_end), 0) ? __overflow(__stream, (unsigned char)(__c)) :
                                                                                            (unsigned char)(*(__stream)->_IO_write_ptr++ = (__c)));
}

/* This is defined in POSIX.1:1996.  */
extern __inline __attribute__((__gnu_inline__)) int
putc_unlocked(int __c, FILE *__stream)
{
    return (__builtin_expect(((__stream)->_IO_write_ptr >= (__stream)->_IO_write_end), 0) ? __overflow(__stream, (unsigned char)(__c)) :
                                                                                            (unsigned char)(*(__stream)->_IO_write_ptr++ = (__c)));
}

/* This is defined in POSIX.1:1996.  */
extern __inline __attribute__((__gnu_inline__)) int
putchar_unlocked(int __c)
{
    return (__builtin_expect(((stdout)->_IO_write_ptr >= (stdout)->_IO_write_end), 0) ? __overflow(stdout, (unsigned char)(__c)) :
                                                                                        (unsigned char)(*(stdout)->_IO_write_ptr++ = (__c)));
}

/* Like `getdelim', but reads up to a newline.  */
extern __inline __attribute__((__gnu_inline__)) __ssize_t
getline(char **__lineptr, size_t *__n, FILE *__stream)
{
    return __getdelim(__lineptr, __n, '\n', __stream);
}

/* Faster versions when locking is not required.  */
extern __inline __attribute__((__gnu_inline__)) int __attribute__((__nothrow__, __leaf__)) feof_unlocked(FILE *__stream)
{
    return (((__stream)->_flags & 0x0010) != 0);
}

/* Faster versions when locking is not required.  */
extern __inline __attribute__((__gnu_inline__)) int __attribute__((__nothrow__, __leaf__)) ferror_unlocked(FILE *__stream)
{
    return (((__stream)->_flags & 0x0020) != 0);
}

/* Perform some simple optimizations.  */
/* Define helper macro.  */

/* Now include the function definitions and redirects too.  */
/* Checking macros for stdio functions.
   Copyright (C) 2004-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */
extern __inline __attribute__((__always_inline__)) __attribute__((__gnu_inline__)) __attribute__((__artificial__)) int __attribute__((__nothrow__, __leaf__))
sprintf(char *__restrict __s, const char *__restrict __fmt, ...)
{
    return __builtin___sprintf_chk(__s, 3 - 1, __builtin_dynamic_object_size(__s, 1), __fmt, __builtin_va_arg_pack());
}

extern __inline __attribute__((__always_inline__)) __attribute__((__gnu_inline__)) __attribute__((__artificial__)) int __attribute__((__nothrow__, __leaf__))
vsprintf(char *__restrict __s, const char *__restrict __fmt, __gnuc_va_list __ap)

{
    return __builtin___vsprintf_chk(__s, 3 - 1, __builtin_dynamic_object_size(__s, 1), __fmt, __ap);
}

extern __inline __attribute__((__always_inline__)) __attribute__((__gnu_inline__)) __attribute__((__artificial__)) int __attribute__((__nothrow__, __leaf__))
snprintf(char *__restrict __s, size_t __n, const char *__restrict __fmt, ...)

{
    return __builtin___snprintf_chk(__s, __n, 3 - 1, __builtin_dynamic_object_size(__s, 1), __fmt, __builtin_va_arg_pack());
}

extern __inline __attribute__((__always_inline__)) __attribute__((__gnu_inline__)) __attribute__((__artificial__)) int __attribute__((__nothrow__, __leaf__))
vsnprintf(char *__restrict __s, size_t __n, const char *__restrict __fmt, __gnuc_va_list __ap)

{
    return __builtin___vsnprintf_chk(__s, __n, 3 - 1, __builtin_dynamic_object_size(__s, 1), __fmt, __ap);
}

extern __inline __attribute__((__always_inline__)) __attribute__((__gnu_inline__)) __attribute__((__artificial__)) int
fprintf(FILE *__restrict __stream, const char *__restrict __fmt, ...)
{
    return __fprintf_chk(__stream, 3 - 1, __fmt, __builtin_va_arg_pack());
}

extern __inline __attribute__((__always_inline__)) __attribute__((__gnu_inline__)) __attribute__((__artificial__)) int
printf(const char *__restrict __fmt, ...)
{
    return __printf_chk(3 - 1, __fmt, __builtin_va_arg_pack());
}

extern __inline __attribute__((__always_inline__)) __attribute__((__gnu_inline__)) __attribute__((__artificial__)) int
vprintf(const char *__restrict __fmt, __gnuc_va_list __ap)
{

    return __vfprintf_chk(stdout, 3 - 1, __fmt, __ap);
}

extern __inline __attribute__((__always_inline__)) __attribute__((__gnu_inline__)) __attribute__((__artificial__)) int
vfprintf(FILE *__restrict __stream, const char *__restrict __fmt, __gnuc_va_list __ap)
{
    return __vfprintf_chk(__stream, 3 - 1, __fmt, __ap);
}

extern __inline __attribute__((__always_inline__)) __attribute__((__gnu_inline__)) __attribute__((__artificial__)) int
dprintf(int __fd, const char *__restrict __fmt, ...)
{
    return __dprintf_chk(__fd, 3 - 1, __fmt, __builtin_va_arg_pack());
}

extern __inline __attribute__((__always_inline__)) __attribute__((__gnu_inline__)) __attribute__((__artificial__)) int
vdprintf(int __fd, const char *__restrict __fmt, __gnuc_va_list __ap)
{
    return __vdprintf_chk(__fd, 3 - 1, __fmt, __ap);
}

extern __inline __attribute__((__always_inline__)) __attribute__((__gnu_inline__)) __attribute__((__artificial__)) int __attribute__((__nothrow__, __leaf__))
asprintf(char **__restrict __ptr, const char *__restrict __fmt, ...)
{
    return __asprintf_chk(__ptr, 3 - 1, __fmt, __builtin_va_arg_pack());
}

extern __inline __attribute__((__always_inline__)) __attribute__((__gnu_inline__)) __attribute__((__artificial__)) int __attribute__((__nothrow__, __leaf__))
__asprintf(char **__restrict __ptr, const char *__restrict __fmt, ...)

{
    return __asprintf_chk(__ptr, 3 - 1, __fmt, __builtin_va_arg_pack());
}

extern __inline __attribute__((__always_inline__)) __attribute__((__gnu_inline__)) __attribute__((__artificial__)) int __attribute__((__nothrow__, __leaf__))
obstack_printf(struct obstack *__restrict __obstack, const char *__restrict __fmt, ...)

{
    return __obstack_printf_chk(__obstack, 3 - 1, __fmt, __builtin_va_arg_pack());
}
extern __inline __attribute__((__always_inline__)) __attribute__((__gnu_inline__)) __attribute__((__artificial__)) int __attribute__((__nothrow__, __leaf__))
vasprintf(char **__restrict __ptr, const char *__restrict __fmt, __gnuc_va_list __ap)

{
    return __vasprintf_chk(__ptr, 3 - 1, __fmt, __ap);
}

extern __inline __attribute__((__always_inline__)) __attribute__((__gnu_inline__)) __attribute__((__artificial__)) int __attribute__((__nothrow__, __leaf__))
obstack_vprintf(struct obstack *__restrict __obstack, const char *__restrict __fmt, __gnuc_va_list __ap)

{
    return __obstack_vprintf_chk(__obstack, 3 - 1, __fmt, __ap);
}
extern __inline __attribute__((__always_inline__)) __attribute__((__gnu_inline__)) __attribute__((__artificial__)) __attribute__((__warn_unused_result__))
__attribute__((__access__(__write_only__, 1))) char *
fgets(char *__restrict __s, int __n, FILE *__restrict __stream)
{
    size_t sz = __builtin_dynamic_object_size(__s, 1);
    if (((__builtin_constant_p(sz) && (sz) == (long unsigned int)-1) ||
            (((__typeof(__n))0 < (__typeof(__n))-1 || (__builtin_constant_p(__n) && (__n) > 0)) &&
                __builtin_constant_p((((long unsigned int)(__n)) <= ((sz)) / ((sizeof(char))))) && (((long unsigned int)(__n)) <= ((sz)) / ((sizeof(char)))))))
        return __fgets_alias(__s, __n, __stream);
    if ((((__typeof(__n))0 < (__typeof(__n))-1 || (__builtin_constant_p(__n) && (__n) > 0)) &&
            __builtin_constant_p((((long unsigned int)(__n)) <= (sz) / (sizeof(char)))) && !(((long unsigned int)(__n)) <= (sz) / (sizeof(char)))))
        return __fgets_chk_warn(__s, sz, __n, __stream);
    return __fgets_chk(__s, sz, __n, __stream);
}

extern __inline __attribute__((__always_inline__)) __attribute__((__gnu_inline__)) __attribute__((__artificial__)) __attribute__((__warn_unused_result__))
size_t
fread(void *__restrict __ptr, size_t __size, size_t __n, FILE *__restrict __stream)
{
    size_t sz = __builtin_dynamic_object_size(__ptr, 0);
    if (((__builtin_constant_p(sz) && (sz) == (long unsigned int)-1) ||
            (((__typeof(__n))0 < (__typeof(__n))-1 || (__builtin_constant_p(__n) && (__n) > 0)) &&
                __builtin_constant_p((((long unsigned int)(__n)) <= ((sz)) / ((__size)))) && (((long unsigned int)(__n)) <= ((sz)) / ((__size))))))
        return __fread_alias(__ptr, __size, __n, __stream);
    if ((((__typeof(__n))0 < (__typeof(__n))-1 || (__builtin_constant_p(__n) && (__n) > 0)) &&
            __builtin_constant_p((((long unsigned int)(__n)) <= (sz) / (__size))) && !(((long unsigned int)(__n)) <= (sz) / (__size))))
        return __fread_chk_warn(__ptr, sz, __size, __n, __stream);
    return __fread_chk(__ptr, sz, __size, __n, __stream);
}

extern __inline __attribute__((__always_inline__)) __attribute__((__gnu_inline__)) __attribute__((__artificial__)) __attribute__((__warn_unused_result__))
__attribute__((__access__(__write_only__, 1))) char *
fgets_unlocked(char *__restrict __s, int __n, FILE *__restrict __stream)
{
    size_t sz = __builtin_dynamic_object_size(__s, 1);
    if (((__builtin_constant_p(sz) && (sz) == (long unsigned int)-1) ||
            (((__typeof(__n))0 < (__typeof(__n))-1 || (__builtin_constant_p(__n) && (__n) > 0)) &&
                __builtin_constant_p((((long unsigned int)(__n)) <= ((sz)) / ((sizeof(char))))) && (((long unsigned int)(__n)) <= ((sz)) / ((sizeof(char)))))))
        return __fgets_unlocked_alias(__s, __n, __stream);
    if ((((__typeof(__n))0 < (__typeof(__n))-1 || (__builtin_constant_p(__n) && (__n) > 0)) &&
            __builtin_constant_p((((long unsigned int)(__n)) <= (sz) / (sizeof(char)))) && !(((long unsigned int)(__n)) <= (sz) / (sizeof(char)))))
        return __fgets_unlocked_chk_warn(__s, sz, __n, __stream);
    return __fgets_unlocked_chk(__s, sz, __n, __stream);
}

extern __inline __attribute__((__always_inline__)) __attribute__((__gnu_inline__)) __attribute__((__artificial__)) __attribute__((__warn_unused_result__))
size_t
fread_unlocked(void *__restrict __ptr, size_t __size, size_t __n, FILE *__restrict __stream)
{
    size_t sz = __builtin_dynamic_object_size(__ptr, 0);
    if (((__builtin_constant_p(sz) && (sz) == (long unsigned int)-1) ||
            (((__typeof(__n))0 < (__typeof(__n))-1 || (__builtin_constant_p(__n) && (__n) > 0)) &&
                __builtin_constant_p((((long unsigned int)(__n)) <= ((sz)) / ((__size)))) && (((long unsigned int)(__n)) <= ((sz)) / ((__size)))))) {

        if (__builtin_constant_p(__size) && __builtin_constant_p(__n) && (__size | __n) < (((size_t)1) << (8 * sizeof(size_t) / 2)) && __size * __n <= 8) {
            size_t __cnt = __size * __n;
            char *__cptr = (char *)__ptr;
            if (__cnt == 0)
                return 0;

            for (; __cnt > 0; --__cnt) {
                int __c = getc_unlocked(__stream);
                if (__c == (-1))
                    break;
                *__cptr++ = __c;
            }
            return (__cptr - (char *)__ptr) / __size;
        }

        return __fread_unlocked_alias(__ptr, __size, __n, __stream);
    }
    if ((((__typeof(__n))0 < (__typeof(__n))-1 || (__builtin_constant_p(__n) && (__n) > 0)) &&
            __builtin_constant_p((((long unsigned int)(__n)) <= (sz) / (__size))) && !(((long unsigned int)(__n)) <= (sz) / (__size))))
        return __fread_unlocked_chk_warn(__ptr, sz, __size, __n, __stream);
    return __fread_unlocked_chk(__ptr, sz, __size, __n, __stream);
}

/* Copyright (C) 1991-2023 Free Software Foundation, Inc.
   Copyright The GNU Toolchain Authors.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/*
 *	ISO C99 Standard: 7.20 General utilities	<stdlib.h>
 */

/* Handle feature test macros at the start of a header.
   Copyright (C) 2016-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* This header is internal to glibc and should not be included outside
   of glibc headers.  Headers including it must define
   __GLIBC_INTERNAL_STARTING_HEADER_IMPLEMENTATION first.  This header
   cannot have multiple include guards because ISO C feature test
   macros depend on the definition of the macro when an affected
   header is included, not when the first system header is
   included.  */

/* Copyright (C) 1991-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* ISO/IEC TR 24731-2:2010 defines the __STDC_WANT_LIB_EXT2__
   macro.  */
/* ISO/IEC TS 18661-1:2014 defines the __STDC_WANT_IEC_60559_BFP_EXT__
   macro.  Most but not all symbols enabled by that macro in TS
   18661-1 are enabled unconditionally in C2X.  In C2X, the symbols in
   Annex F still require a new feature test macro
   __STDC_WANT_IEC_60559_EXT__ instead (C2X does not define
   __STDC_WANT_IEC_60559_BFP_EXT__), while a few features from TS
   18661-1 are not included in C2X (and thus should depend on
   __STDC_WANT_IEC_60559_BFP_EXT__ even when C2X features are
   enabled).

   __GLIBC_USE (IEC_60559_BFP_EXT) controls those features from TS
   18661-1 not included in C2X.

   __GLIBC_USE (IEC_60559_BFP_EXT_C2X) controls those features from TS
   18661-1 that are also included in C2X (with no feature test macro
   required in C2X).

   __GLIBC_USE (IEC_60559_EXT) controls those features from TS 18661-1
   that are included in C2X but conditional on
   __STDC_WANT_IEC_60559_EXT__.  (There are currently no features
   conditional on __STDC_WANT_IEC_60559_EXT__ that are not in TS
   18661-1.)  */
/* ISO/IEC TS 18661-4:2015 defines the
   __STDC_WANT_IEC_60559_FUNCS_EXT__ macro.  Other than the reduction
   functions, the symbols from this TS are enabled unconditionally in
   C2X.  */
/* ISO/IEC TS 18661-3:2015 defines the
   __STDC_WANT_IEC_60559_TYPES_EXT__ macro.  */

/* Get size_t, wchar_t and NULL from <stddef.h>.  */

/* Copyright (C) 1989-2022 Free Software Foundation, Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3, or (at your option)
any later version.

GCC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

Under Section 7 of GPL version 3, you are granted additional
permissions described in the GCC Runtime Library Exception, version
3.1, as published by the Free Software Foundation.

You should have received a copy of the GNU General Public License and
a copy of the GCC Runtime Library Exception along with this program;
see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
<http://www.gnu.org/licenses/>.  */

/*
 * ISO C Standard:  7.17  Common definitions  <stddef.h>
 */

/* Any one of these symbols __need_* means that GNU libc
   wants us just to define one data type.  So don't define
   the symbols that indicate this file's entire job has been done.  */
/* This avoids lossage on SunOS but only if stdtypes.h comes first.
   There's no way to win with the other order!  Sun lossage.  */
/* Sequent's header files use _PTRDIFF_T_ in some conflicting way.
   Just ignore it.  */

/* On VxWorks, <type/vxTypesBase.h> may have defined macros like
   _TYPE_size_t which will typedef size_t.  fixincludes patched the
   vxTypesBase.h so that this macro is only defined if _GCC_SIZE_T is
   not defined, and so that defining this macro defines _GCC_SIZE_T.
   If we find that the macros are still defined at this point, we must
   invoke them so that the type is defined as expected.  */
/* In case nobody has defined these types, but we aren't running under
   GCC 2.00, make sure that __PTRDIFF_TYPE__, __SIZE_TYPE__, and
   __WCHAR_TYPE__ have reasonable values.  This can happen if the
   parts of GCC is compiled by an older compiler, that actually
   include gstddef.h, such as collect2.  */

/* Signed type of difference of two pointers.  */

/* Define this type if we are doing the whole job,
   or if we want this type in particular.  */
/* If this symbol has done its job, get rid of it.  */

/* Unsigned type of `sizeof' something.  */

/* Define this type if we are doing the whole job,
   or if we want this type in particular.  */
/* Wide character type.
   Locale-writers should change this as necessary to
   be big enough to hold unique values not between 0 and 127,
   and not (wchar_t) -1, for each defined multibyte character.  */

/* Define this type if we are doing the whole job,
   or if we want this type in particular.  */
/* A null pointer constant.  */
/* Offset of member MEMBER in a struct of type TYPE. */

/* XPG requires a few symbols from <sys/wait.h> being defined.  */
/* Definitions of flag bits for `waitpid' et al.
   Copyright (C) 1992-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* Bits in the third argument to `waitpid'.  */

/* Bits in the fourth argument to `waitid'.  */
/* Definitions of status bits for `wait' et al.
   Copyright (C) 1992-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* Everything extant so far uses these same bits.  */

/* If WIFEXITED(STATUS), the low-order 8 bits of the status.  */

/* If WIFSIGNALED(STATUS), the terminating signal.  */

/* If WIFSTOPPED(STATUS), the signal that stopped the child.  */

/* Nonzero if STATUS indicates normal termination.  */

/* Nonzero if STATUS indicates termination by a signal.  */

/* Nonzero if STATUS indicates the child is stopped.  */

/* Nonzero if STATUS indicates the child continued after a stop.  We only
   define this if <bits/waitflags.h> provides the WCONTINUED flag bit.  */

/* Nonzero if STATUS indicates the child dumped core.  */

/* Macros for constructing status values.  */

/* Define the macros <sys/wait.h> also would define this way.  */
/* _FloatN API tests for enablement.  */
/* Macros to control TS 18661-3 glibc features on x86.
   Copyright (C) 2017-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* Returned by `div'.  */
typedef struct {
    int quot; /* Quotient.  */
    int rem;  /* Remainder.  */
} div_t;

/* Returned by `ldiv'.  */

typedef struct {
    long int quot; /* Quotient.  */
    long int rem;  /* Remainder.  */
} ldiv_t;

/* Returned by `lldiv'.  */
__extension__ typedef struct {
    long long int quot; /* Quotient.  */
    long long int rem;  /* Remainder.  */
} lldiv_t;

/* The largest number rand will return (same as INT_MAX).  */

/* We define these the same for all machines.
   Changes from this to the outside world should be done in `_exit'.  */

/* Maximum length of a multibyte character in the current locale.  */

extern size_t __ctype_get_mb_cur_max(void) __attribute__((__nothrow__, __leaf__)) __attribute__((__warn_unused_result__));

/* Convert a string to a floating-point number.  */
extern double atof(const char *__nptr) __attribute__((__nothrow__, __leaf__)) __attribute__((__pure__)) __attribute__((__nonnull__(1)))
__attribute__((__warn_unused_result__));
/* Convert a string to an integer.  */
extern int atoi(const char *__nptr) __attribute__((__nothrow__, __leaf__)) __attribute__((__pure__)) __attribute__((__nonnull__(1)))
__attribute__((__warn_unused_result__));
/* Convert a string to a long integer.  */
extern long int atol(const char *__nptr) __attribute__((__nothrow__, __leaf__)) __attribute__((__pure__)) __attribute__((__nonnull__(1)))
__attribute__((__warn_unused_result__));

/* Convert a string to a long long integer.  */
__extension__ extern long long int atoll(const char *__nptr) __attribute__((__nothrow__, __leaf__)) __attribute__((__pure__)) __attribute__((__nonnull__(1)))
__attribute__((__warn_unused_result__));

/* Convert a string to a floating-point number.  */
extern double strtod(const char *__restrict __nptr, char **__restrict __endptr) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));

/* Likewise for `float' and `long double' sizes of floating-point numbers.  */
extern float strtof(const char *__restrict __nptr, char **__restrict __endptr) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));

extern long double strtold(const char *__restrict __nptr, char **__restrict __endptr) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));

/* Likewise for '_FloatN' and '_FloatNx'.  */
extern _Float32 strtof32(const char *__restrict __nptr, char **__restrict __endptr) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));

extern _Float64 strtof64(const char *__restrict __nptr, char **__restrict __endptr) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));

extern _Float128 strtof128(const char *__restrict __nptr, char **__restrict __endptr) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));

extern _Float32x strtof32x(const char *__restrict __nptr, char **__restrict __endptr) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));

extern _Float64x strtof64x(const char *__restrict __nptr, char **__restrict __endptr) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));
/* Convert a string to a long integer.  */
extern long int strtol(const char *__restrict __nptr, char **__restrict __endptr, int __base) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(1)));
/* Convert a string to an unsigned long integer.  */
extern unsigned long int strtoul(const char *__restrict __nptr, char **__restrict __endptr, int __base) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(1)));

/* Convert a string to a quadword integer.  */
__extension__ extern long long int strtoq(const char *__restrict __nptr, char **__restrict __endptr, int __base) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(1)));
/* Convert a string to an unsigned quadword integer.  */
__extension__ extern unsigned long long int strtouq(const char *__restrict __nptr, char **__restrict __endptr, int __base)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));

/* Convert a string to a quadword integer.  */
__extension__ extern long long int strtoll(const char *__restrict __nptr, char **__restrict __endptr, int __base) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(1)));
/* Convert a string to an unsigned quadword integer.  */
__extension__ extern unsigned long long int strtoull(const char *__restrict __nptr, char **__restrict __endptr, int __base)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));

/* Versions of the above functions that handle '0b' and '0B' prefixes
   in base 0 or 2.  */

extern long int strtol(const char *__restrict __nptr, char **__restrict __endptr, int __base) __asm__(""
                                                                                                      "__isoc23_strtol") __attribute__((__nothrow__, __leaf__))

__attribute__((__nonnull__(1)));
extern unsigned long int strtoul(const char *__restrict __nptr, char **__restrict __endptr, int __base) __asm__(""
                                                                                                                "__isoc23_strtoul")
    __attribute__((__nothrow__, __leaf__))

    __attribute__((__nonnull__(1)));

__extension__ extern long long int strtoq(const char *__restrict __nptr, char **__restrict __endptr, int __base) __asm__(""
                                                                                                                         "__isoc23_strtoll")
    __attribute__((__nothrow__, __leaf__))

    __attribute__((__nonnull__(1)));
__extension__ extern unsigned long long int strtouq(const char *__restrict __nptr, char **__restrict __endptr, int __base) __asm__(""
                                                                                                                                   "__isoc23_strtoull")
    __attribute__((__nothrow__, __leaf__))

    __attribute__((__nonnull__(1)));

__extension__ extern long long int strtoll(const char *__restrict __nptr, char **__restrict __endptr, int __base) __asm__(""
                                                                                                                          "__isoc23_strtoll")
    __attribute__((__nothrow__, __leaf__))

    __attribute__((__nonnull__(1)));
__extension__ extern unsigned long long int strtoull(const char *__restrict __nptr, char **__restrict __endptr, int __base) __asm__(""
                                                                                                                                    "__isoc23_strtoull")
    __attribute__((__nothrow__, __leaf__))

    __attribute__((__nonnull__(1)));
/* Convert a floating-point number to a string.  */

extern int strfromd(char *__dest, size_t __size, const char *__format, double __f) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(3)));

extern int strfromf(char *__dest, size_t __size, const char *__format, float __f) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(3)));

extern int strfroml(char *__dest, size_t __size, const char *__format, long double __f) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(3)));
extern int strfromf32(char *__dest, size_t __size, const char *__format, _Float32 __f) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(3)));

extern int strfromf64(char *__dest, size_t __size, const char *__format, _Float64 __f) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(3)));

extern int strfromf128(char *__dest, size_t __size, const char *__format, _Float128 __f) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(3)));

extern int strfromf32x(char *__dest, size_t __size, const char *__format, _Float32x __f) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(3)));

extern int strfromf64x(char *__dest, size_t __size, const char *__format, _Float64x __f) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(3)));
/* Parallel versions of the functions above which take the locale to
   use as an additional parameter.  These are GNU extensions inspired
   by the POSIX.1-2008 extended locale API.  */
/* Definition of locale_t.
   Copyright (C) 2017-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* Definition of struct __locale_struct and __locale_t.
   Copyright (C) 1997-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* POSIX.1-2008: the locale_t type, representing a locale context
   (implementation-namespace version).  This type should be treated
   as opaque by applications; some details are exposed for the sake of
   efficiency in e.g. ctype functions.  */

struct __locale_struct {
    /* Note: LC_ALL is not a valid index into this array.  */
    struct __locale_data *__locales[13]; /* 13 = __LC_LAST. */

    /* To increase the speed of this solution we add some special members.  */
    const unsigned short int *__ctype_b;
    const int *__ctype_tolower;
    const int *__ctype_toupper;

    /* Note: LC_ALL is not a valid index into this array.  */
    const char *__names[13];
};

typedef struct __locale_struct *__locale_t;

typedef __locale_t locale_t;

extern long int strtol_l(const char *__restrict __nptr, char **__restrict __endptr, int __base, locale_t __loc) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(1, 4)));

extern unsigned long int strtoul_l(const char *__restrict __nptr, char **__restrict __endptr, int __base, locale_t __loc) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(1, 4)));

__extension__ extern long long int strtoll_l(const char *__restrict __nptr, char **__restrict __endptr, int __base, locale_t __loc)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1, 4)));

__extension__ extern unsigned long long int strtoull_l(const char *__restrict __nptr, char **__restrict __endptr, int __base, locale_t __loc)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1, 4)));

/* Versions of the above functions that handle '0b' and '0B' prefixes
   in base 0 or 2.  */

extern long int strtol_l(const char *__restrict __nptr, char **__restrict __endptr, int __base, locale_t __loc) __asm__(""
                                                                                                                        "__isoc23_strtol_l")
    __attribute__((__nothrow__, __leaf__))

    __attribute__((__nonnull__(1, 4)));
extern unsigned long int strtoul_l(const char *__restrict __nptr, char **__restrict __endptr, int __base, locale_t __loc) __asm__(""
                                                                                                                                  "__isoc23_strtoul_l")
    __attribute__((__nothrow__, __leaf__))

    __attribute__((__nonnull__(1, 4)));
__extension__ extern long long int strtoll_l(const char *__restrict __nptr, char **__restrict __endptr, int __base, locale_t __loc) __asm__(
    ""
    "__isoc23_strtoll_l") __attribute__((__nothrow__, __leaf__))

__attribute__((__nonnull__(1, 4)));
__extension__ extern unsigned long long int strtoull_l(const char *__restrict __nptr, char **__restrict __endptr, int __base, locale_t __loc) __asm__(
    ""
    "__isoc23_strtoull_l") __attribute__((__nothrow__, __leaf__))

__attribute__((__nonnull__(1, 4)));
extern double strtod_l(const char *__restrict __nptr, char **__restrict __endptr, locale_t __loc) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(1, 3)));

extern float strtof_l(const char *__restrict __nptr, char **__restrict __endptr, locale_t __loc) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(1, 3)));

extern long double strtold_l(const char *__restrict __nptr, char **__restrict __endptr, locale_t __loc) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(1, 3)));
extern _Float32 strtof32_l(const char *__restrict __nptr, char **__restrict __endptr, locale_t __loc) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(1, 3)));

extern _Float64 strtof64_l(const char *__restrict __nptr, char **__restrict __endptr, locale_t __loc) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(1, 3)));

extern _Float128 strtof128_l(const char *__restrict __nptr, char **__restrict __endptr, locale_t __loc) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(1, 3)));

extern _Float32x strtof32x_l(const char *__restrict __nptr, char **__restrict __endptr, locale_t __loc) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(1, 3)));

extern _Float64x strtof64x_l(const char *__restrict __nptr, char **__restrict __endptr, locale_t __loc) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(1, 3)));
extern __inline __attribute__((__gnu_inline__)) int __attribute__((__nothrow__, __leaf__)) atoi(const char *__nptr)
{
    return (int)strtol(__nptr, (char **)((void *)0), 10);
}
extern __inline __attribute__((__gnu_inline__)) long int __attribute__((__nothrow__, __leaf__)) atol(const char *__nptr)
{
    return strtol(__nptr, (char **)((void *)0), 10);
}

__extension__ extern __inline __attribute__((__gnu_inline__)) long long int __attribute__((__nothrow__, __leaf__)) atoll(const char *__nptr)
{
    return strtoll(__nptr, (char **)((void *)0), 10);
}

/* Convert N to base 64 using the digits "./0-9A-Za-z", least-significant
   digit first.  Returns a pointer to static storage overwritten by the
   next call.  */
extern char *l64a(long int __n) __attribute__((__nothrow__, __leaf__)) __attribute__((__warn_unused_result__));

/* Read a number from a string S in base 64 as above.  */
extern long int a64l(const char *__s) __attribute__((__nothrow__, __leaf__)) __attribute__((__pure__)) __attribute__((__nonnull__(1)))
__attribute__((__warn_unused_result__));

/* Copyright (C) 1991-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/*
 *	POSIX Standard: 2.6 Primitive System Data Types	<sys/types.h>
 */

/* Copyright (C) 1991-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* bits/types.h -- definitions of __*_t types underlying *_t types.
   Copyright (C) 2002-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/*
 * Never include this file directly; use <sys/types.h> instead.
 */

typedef __u_char u_char;
typedef __u_short u_short;
typedef __u_int u_int;
typedef __u_long u_long;
typedef __quad_t quad_t;
typedef __u_quad_t u_quad_t;
typedef __fsid_t fsid_t;

typedef __loff_t loff_t;

typedef __ino_t ino_t;

typedef __ino64_t ino64_t;

typedef __dev_t dev_t;

typedef __gid_t gid_t;

typedef __mode_t mode_t;

typedef __nlink_t nlink_t;

typedef __uid_t uid_t;
typedef __pid_t pid_t;

typedef __id_t id_t;
typedef __daddr_t daddr_t;
typedef __caddr_t caddr_t;

typedef __key_t key_t;

/* bits/types.h -- definitions of __*_t types underlying *_t types.
   Copyright (C) 2002-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/*
 * Never include this file directly; use <sys/types.h> instead.
 */

/* Returned by `clock'.  */
typedef __clock_t clock_t;

/* bits/types.h -- definitions of __*_t types underlying *_t types.
   Copyright (C) 2002-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/*
 * Never include this file directly; use <sys/types.h> instead.
 */

/* Clock ID used in clock and timer functions.  */
typedef __clockid_t clockid_t;

/* bits/types.h -- definitions of __*_t types underlying *_t types.
   Copyright (C) 2002-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/*
 * Never include this file directly; use <sys/types.h> instead.
 */

/* Returned by `time'.  */

typedef __time_t time_t;

/* bits/types.h -- definitions of __*_t types underlying *_t types.
   Copyright (C) 2002-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/*
 * Never include this file directly; use <sys/types.h> instead.
 */

/* Timer ID returned by `timer_create'.  */
typedef __timer_t timer_t;

typedef __useconds_t useconds_t;

typedef __suseconds_t suseconds_t;

/* Copyright (C) 1989-2022 Free Software Foundation, Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3, or (at your option)
any later version.

GCC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

Under Section 7 of GPL version 3, you are granted additional
permissions described in the GCC Runtime Library Exception, version
3.1, as published by the Free Software Foundation.

You should have received a copy of the GNU General Public License and
a copy of the GCC Runtime Library Exception along with this program;
see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
<http://www.gnu.org/licenses/>.  */

/*
 * ISO C Standard:  7.17  Common definitions  <stddef.h>
 */

/* Any one of these symbols __need_* means that GNU libc
   wants us just to define one data type.  So don't define
   the symbols that indicate this file's entire job has been done.  */
/* This avoids lossage on SunOS but only if stdtypes.h comes first.
   There's no way to win with the other order!  Sun lossage.  */
/* Sequent's header files use _PTRDIFF_T_ in some conflicting way.
   Just ignore it.  */

/* On VxWorks, <type/vxTypesBase.h> may have defined macros like
   _TYPE_size_t which will typedef size_t.  fixincludes patched the
   vxTypesBase.h so that this macro is only defined if _GCC_SIZE_T is
   not defined, and so that defining this macro defines _GCC_SIZE_T.
   If we find that the macros are still defined at this point, we must
   invoke them so that the type is defined as expected.  */
/* In case nobody has defined these types, but we aren't running under
   GCC 2.00, make sure that __PTRDIFF_TYPE__, __SIZE_TYPE__, and
   __WCHAR_TYPE__ have reasonable values.  This can happen if the
   parts of GCC is compiled by an older compiler, that actually
   include gstddef.h, such as collect2.  */

/* Signed type of difference of two pointers.  */

/* Define this type if we are doing the whole job,
   or if we want this type in particular.  */
/* If this symbol has done its job, get rid of it.  */

/* Unsigned type of `sizeof' something.  */

/* Define this type if we are doing the whole job,
   or if we want this type in particular.  */
/* Wide character type.
   Locale-writers should change this as necessary to
   be big enough to hold unique values not between 0 and 127,
   and not (wchar_t) -1, for each defined multibyte character.  */

/* Define this type if we are doing the whole job,
   or if we want this type in particular.  */
/* A null pointer constant.  */
/* Offset of member MEMBER in a struct of type TYPE. */

/* Old compatibility names for C types.  */
typedef unsigned long int ulong;
typedef unsigned short int ushort;
typedef unsigned int uint;

/* These size-specific names are used by some of the inet code.  */

/* Define intN_t types.
   Copyright (C) 2017-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* These were defined by ISO C without the first `_'.  */
typedef __uint8_t u_int8_t;
typedef __uint16_t u_int16_t;
typedef __uint32_t u_int32_t;
typedef __uint64_t u_int64_t;

typedef int register_t __attribute__((__mode__(__word__)));

/* Some code from BIND tests this macro to see if the types above are
   defined.  */

/* In BSD <sys/types.h> is expected to define BYTE_ORDER.  */
/* Copyright (C) 1992-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* Copyright (C) 1991-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* Get the definitions of __*_ENDIAN, __BYTE_ORDER, and __FLOAT_WORD_ORDER.  */
/* Endian macros for string.h functions
   Copyright (C) 1992-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

/* Definitions for byte order, according to significance of bytes,
   from low addresses to high addresses.  The value is what you get by
   putting '4' in the most significant byte, '3' in the second most
   significant byte, '2' in the second least significant byte, and '1'
   in the least significant byte, and then writing down one digit for
   each byte, starting with the byte at the lowest address at the left,
   and proceeding to the byte with the highest address at the right.  */

/* This file defines `__BYTE_ORDER' for the particular machine.  */

/* i386/x86_64 are little-endian.  */

/* Some machines may need to use a different endianness for floating point
   values.  */
/* Conversion interfaces.  */
/* Macros and inline functions to swap the order of bytes in integer values.
   Copyright (C) 1997-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */
/* Copyright (C) 1991-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */
/* bits/types.h -- definitions of __*_t types underlying *_t types.
   Copyright (C) 2002-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/*
 * Never include this file directly; use <sys/types.h> instead.
 */

/* Swap bytes in 16-bit value.  */

static __inline __uint16_t
__bswap_16(__uint16_t __bsx)
{

    return __builtin_bswap16(__bsx);
}

/* Swap bytes in 32-bit value.  */

static __inline __uint32_t
__bswap_32(__uint32_t __bsx)
{

    return __builtin_bswap32(__bsx);
}

/* Swap bytes in 64-bit value.  */
__extension__ static __inline __uint64_t
__bswap_64(__uint64_t __bsx)
{

    return __builtin_bswap64(__bsx);
}
/* Inline functions to return unsigned integer values unchanged.
   Copyright (C) 2017-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */
/* bits/types.h -- definitions of __*_t types underlying *_t types.
   Copyright (C) 2002-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/*
 * Never include this file directly; use <sys/types.h> instead.
 */

/* These inline functions are to ensure the appropriate type
   conversions and associated diagnostics from macros that convert to
   a given endianness.  */

static __inline __uint16_t
__uint16_identity(__uint16_t __x)
{
    return __x;
}

static __inline __uint32_t
__uint32_identity(__uint32_t __x)
{
    return __x;
}

static __inline __uint64_t
__uint64_identity(__uint64_t __x)
{
    return __x;
}

/* It also defines `fd_set' and the FD_* macros for `select'.  */
/* `fd_set' type and related macros, and `select'/`pselect' declarations.
   Copyright (C) 1996-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/*	POSIX 1003.1g: 6.2 Select from File Descriptor Sets <sys/select.h>  */

/* Copyright (C) 1991-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* Get definition of needed basic types.  */
/* bits/types.h -- definitions of __*_t types underlying *_t types.
   Copyright (C) 2002-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/*
 * Never include this file directly; use <sys/types.h> instead.
 */

/* Get __FD_* definitions.  */
/* Copyright (C) 1997-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* We don't use `memset' because this would require a prototype and
   the array isn't too big.  */

/* Get sigset_t.  */

typedef struct {
    unsigned long int __val[(1024 / (8 * sizeof(unsigned long int)))];
} __sigset_t;

/* A set of signals to be blocked, unblocked, or waited for.  */
typedef __sigset_t sigset_t;

/* Get definition of timer specification structures.  */

/* bits/types.h -- definitions of __*_t types underlying *_t types.
   Copyright (C) 2002-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/*
 * Never include this file directly; use <sys/types.h> instead.
 */

/* A time value that is accurate to the nearest
   microsecond but also has a range of years.  */
struct timeval {

    __time_t tv_sec;       /* Seconds.  */
    __suseconds_t tv_usec; /* Microseconds.  */
};

/* NB: Include guard matches what <linux/time.h> uses.  */

/* bits/types.h -- definitions of __*_t types underlying *_t types.
   Copyright (C) 2002-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/*
 * Never include this file directly; use <sys/types.h> instead.
 */
/* Endian macros for string.h functions
   Copyright (C) 1992-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

/* POSIX.1b structure for a time value.  This is like a `struct timeval' but
   has nanoseconds instead of microseconds.  */
struct timespec {

    __time_t tv_sec; /* Seconds.  */

    __syscall_slong_t tv_nsec; /* Nanoseconds.  */
};
/* The fd_set member is required to be an array of longs.  */
typedef long int __fd_mask;

/* Some versions of <linux/posix_types.h> define this macros.  */

/* It's easier to assume 8-bit bytes than to get CHAR_BIT.  */

/* fd_set for select and pselect.  */
typedef struct {
    /* XPG4.2 requires this member name.  Otherwise avoid the name
       from the global namespace.  */

    __fd_mask fds_bits[1024 / (8 * (int)sizeof(__fd_mask))];

} fd_set;

/* Maximum number of file descriptors in `fd_set'.  */

/* Sometimes the fd_set member is assumed to have this type.  */
typedef __fd_mask fd_mask;

/* Number of bits per word of `fd_set' (some code assumes this is 32).  */

/* Access macros for `fd_set'.  */

/* Check the first NFDS descriptors each in READFDS (if not NULL) for read
   readiness, in WRITEFDS (if not NULL) for write readiness, and in EXCEPTFDS
   (if not NULL) for exceptional conditions.  If TIMEOUT is not NULL, time out
   after waiting the interval specified therein.  Returns the number of ready
   descriptors, or -1 for errors.

   This function is a cancellation point and therefore not marked with
   __THROW.  */

extern int select(int __nfds, fd_set *__restrict __readfds, fd_set *__restrict __writefds, fd_set *__restrict __exceptfds,
    struct timeval *__restrict __timeout);
/* Same as above only that the TIMEOUT value is given with higher
   resolution and a sigmask which is been set temporarily.  This version
   should be used.

   This function is a cancellation point and therefore not marked with
   __THROW.  */

extern int pselect(int __nfds, fd_set *__restrict __readfds, fd_set *__restrict __writefds, fd_set *__restrict __exceptfds,
    const struct timespec *__restrict __timeout, const __sigset_t *__restrict __sigmask);
/* Define some inlines helping to catch common problems.  */

/* Checking macros for select functions.
   Copyright (C) 2011-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* Checking routines for select functions. Declaration only.
   Copyright (C) 2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */
/* Helper functions to issue warnings and errors when needed.  */
extern long int __fdelt_chk(long int __d);
extern long int __fdelt_warn(long int __d) __attribute__((__warning__("bit outside of fd_set selected")));

typedef __blksize_t blksize_t;

/* Types from the Large File Support interface.  */

typedef __blkcnt_t blkcnt_t; /* Type to count number of disk blocks.  */

typedef __fsblkcnt_t fsblkcnt_t; /* Type to count file system blocks.  */

typedef __fsfilcnt_t fsfilcnt_t;     /* Type to count file system inodes.  */
typedef __blkcnt64_t blkcnt64_t;     /* Type to count number of disk blocks. */
typedef __fsblkcnt64_t fsblkcnt64_t; /* Type to count file system blocks.  */
typedef __fsfilcnt64_t fsfilcnt64_t; /* Type to count file system inodes.  */

/* Now add the thread types.  */

/* Declaration of common pthread types for all architectures.
   Copyright (C) 2017-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* For internal mutex and condition variable definitions.  */
/* Common threading primitives definitions for both POSIX and C11.
   Copyright (C) 2017-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* Arch-specific definitions.  Each architecture must define the following
   macros to define the expected sizes of pthread data types:

   __SIZEOF_PTHREAD_ATTR_T        - size of pthread_attr_t.
   __SIZEOF_PTHREAD_MUTEX_T       - size of pthread_mutex_t.
   __SIZEOF_PTHREAD_MUTEXATTR_T   - size of pthread_mutexattr_t.
   __SIZEOF_PTHREAD_COND_T        - size of pthread_cond_t.
   __SIZEOF_PTHREAD_CONDATTR_T    - size of pthread_condattr_t.
   __SIZEOF_PTHREAD_RWLOCK_T      - size of pthread_rwlock_t.
   __SIZEOF_PTHREAD_RWLOCKATTR_T  - size of pthread_rwlockattr_t.
   __SIZEOF_PTHREAD_BARRIER_T     - size of pthread_barrier_t.
   __SIZEOF_PTHREAD_BARRIERATTR_T - size of pthread_barrierattr_t.

   The additional macro defines any constraint for the lock alignment
   inside the thread structures:

   __LOCK_ALIGNMENT - for internal lock/futex usage.

   Same idea but for the once locking primitive:

   __ONCE_ALIGNMENT - for pthread_once_t/once_flag definition.  */

/* Copyright (C) 2002-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* Determine the wordsize from the preprocessor defines.  */
/* Both x86-64 and x32 use the 64-bit system call interface.  */

/* Monotonically increasing wide counters (at least 62 bits).
   Copyright (C) 2016-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* Counter that is monotonically increasing (by less than 2**31 per
   increment), with a single writer, and an arbitrary number of
   readers.  */
typedef union {
    __extension__ unsigned long long int __value64;
    struct {
        unsigned int __low;
        unsigned int __high;
    } __value32;
} __atomic_wide_counter;

/* Common definition of pthread_mutex_t. */

typedef struct __pthread_internal_list {
    struct __pthread_internal_list *__prev;
    struct __pthread_internal_list *__next;
} __pthread_list_t;

typedef struct __pthread_internal_slist {
    struct __pthread_internal_slist *__next;
} __pthread_slist_t;

/* Arch-specific mutex definitions.  A generic implementation is provided
   by sysdeps/nptl/bits/struct_mutex.h.  If required, an architecture
   can override it by defining:

   1. struct __pthread_mutex_s (used on both pthread_mutex_t and mtx_t
      definition).  It should contains at least the internal members
      defined in the generic version.

   2. __LOCK_ALIGNMENT for any extra attribute for internal lock used with
      atomic operations.

   3. The macro __PTHREAD_MUTEX_INITIALIZER used for static initialization.
      It should initialize the mutex internal flag.  */

/* x86 internal mutex struct definitions.
   Copyright (C) 2019-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

struct __pthread_mutex_s {
    int __lock;
    unsigned int __count;
    int __owner;

    unsigned int __nusers;

    /* KIND must stay at this position in the structure to maintain
       binary compatibility with static initializers.  */
    int __kind;

    short __spins;
    short __elision;
    __pthread_list_t __list;
};

/* Arch-sepecific read-write lock definitions.  A generic implementation is
   provided by struct_rwlock.h.  If required, an architecture can override it
   by defining:

   1. struct __pthread_rwlock_arch_t (used on pthread_rwlock_t definition).
      It should contain at least the internal members defined in the
      generic version.

   2. The macro __PTHREAD_RWLOCK_INITIALIZER used for static initialization.
      It should initialize the rwlock internal type.  */

/* x86 internal rwlock struct definitions.
   Copyright (C) 2019-2023 Free Software Foundation, Inc.

   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

struct __pthread_rwlock_arch_t {
    unsigned int __readers;
    unsigned int __writers;
    unsigned int __wrphase_futex;
    unsigned int __writers_futex;
    unsigned int __pad3;
    unsigned int __pad4;

    int __cur_writer;
    int __shared;
    signed char __rwelision;

    unsigned char __pad1[7];

    unsigned long int __pad2;
    /* FLAGS must stay at this position in the structure to maintain
       binary compatibility.  */
    unsigned int __flags;
};

/* Common definition of pthread_cond_t. */

struct __pthread_cond_s {
    __atomic_wide_counter __wseq;
    __atomic_wide_counter __g1_start;
    unsigned int __g_refs[2];
    unsigned int __g_size[2];
    unsigned int __g1_orig_size;
    unsigned int __wrefs;
    unsigned int __g_signals[2];
};

typedef unsigned int __tss_t;
typedef unsigned long int __thrd_t;

typedef struct {
    int __data;
} __once_flag;

/* Thread identifiers.  The structure of the attribute type is not
   exposed on purpose.  */
typedef unsigned long int pthread_t;

/* Data structures for mutex handling.  The structure of the attribute
   type is not exposed on purpose.  */
typedef union {
    char __size[4];
    int __align;
} pthread_mutexattr_t;

/* Data structure for condition variable handling.  The structure of
   the attribute type is not exposed on purpose.  */
typedef union {
    char __size[4];
    int __align;
} pthread_condattr_t;

/* Keys for thread-specific data */
typedef unsigned int pthread_key_t;

/* Once-only execution */
typedef int pthread_once_t;

union pthread_attr_t {
    char __size[56];
    long int __align;
};

typedef union pthread_attr_t pthread_attr_t;

typedef union {
    struct __pthread_mutex_s __data;
    char __size[40];
    long int __align;
} pthread_mutex_t;

typedef union {
    struct __pthread_cond_s __data;
    char __size[48];
    __extension__ long long int __align;
} pthread_cond_t;

/* Data structure for reader-writer lock variable handling.  The
   structure of the attribute type is deliberately not exposed.  */
typedef union {
    struct __pthread_rwlock_arch_t __data;
    char __size[56];
    long int __align;
} pthread_rwlock_t;

typedef union {
    char __size[8];
    long int __align;
} pthread_rwlockattr_t;

/* POSIX spinlock data type.  */
typedef volatile int pthread_spinlock_t;

/* POSIX barriers data type.  The structure of the type is
   deliberately not exposed.  */
typedef union {
    char __size[32];
    long int __align;
} pthread_barrier_t;

typedef union {
    char __size[4];
    int __align;
} pthread_barrierattr_t;

/* These are the functions that actually do things.  The `random', `srandom',
   `initstate' and `setstate' functions are those from BSD Unices.
   The `rand' and `srand' functions are required by the ANSI standard.
   We provide both interfaces to the same random number generator.  */
/* Return a random long integer between 0 and 2^31-1 inclusive.  */
extern long int random(void) __attribute__((__nothrow__, __leaf__));

/* Seed the random number generator with the given number.  */
extern void srandom(unsigned int __seed) __attribute__((__nothrow__, __leaf__));

/* Initialize the random number generator to use state buffer STATEBUF,
   of length STATELEN, and seed it with SEED.  Optimal lengths are 8, 16,
   32, 64, 128 and 256, the bigger the better; values less than 8 will
   cause an error and values greater than 256 will be rounded down.  */
extern char *initstate(unsigned int __seed, char *__statebuf, size_t __statelen) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(2)));

/* Switch the random number generator to state buffer STATEBUF,
   which should have been previously initialized by `initstate'.  */
extern char *setstate(char *__statebuf) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));

/* Reentrant versions of the `random' family of functions.
   These functions all use the following data structure to contain
   state, rather than global state variables.  */

struct random_data {
    int32_t *fptr;    /* Front pointer.  */
    int32_t *rptr;    /* Rear pointer.  */
    int32_t *state;   /* Array of state values.  */
    int rand_type;    /* Type of random number generator.  */
    int rand_deg;     /* Degree of random number generator.  */
    int rand_sep;     /* Distance between front and rear.  */
    int32_t *end_ptr; /* Pointer behind state table.  */
};

extern int random_r(struct random_data *__restrict __buf, int32_t *__restrict __result) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(1, 2)));

extern int srandom_r(unsigned int __seed, struct random_data *__buf) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(2)));

extern int initstate_r(unsigned int __seed, char *__restrict __statebuf, size_t __statelen, struct random_data *__restrict __buf)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(2, 4)));

extern int setstate_r(char *__restrict __statebuf, struct random_data *__restrict __buf) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(1, 2)));

/* Return a random integer between 0 and RAND_MAX inclusive.  */
extern int rand(void) __attribute__((__nothrow__, __leaf__));
/* Seed the random number generator with the given number.  */
extern void srand(unsigned int __seed) __attribute__((__nothrow__, __leaf__));

/* Reentrant interface according to POSIX.1.  */
extern int rand_r(unsigned int *__seed) __attribute__((__nothrow__, __leaf__));

/* System V style 48-bit random number generator functions.  */

/* Return non-negative, double-precision floating-point value in [0.0,1.0).  */
extern double drand48(void) __attribute__((__nothrow__, __leaf__));
extern double erand48(unsigned short int __xsubi[3]) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));

/* Return non-negative, long integer in [0,2^31).  */
extern long int lrand48(void) __attribute__((__nothrow__, __leaf__));
extern long int nrand48(unsigned short int __xsubi[3]) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));

/* Return signed, long integers in [-2^31,2^31).  */
extern long int mrand48(void) __attribute__((__nothrow__, __leaf__));
extern long int jrand48(unsigned short int __xsubi[3]) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));

/* Seed random number generator.  */
extern void srand48(long int __seedval) __attribute__((__nothrow__, __leaf__));
extern unsigned short int *seed48(unsigned short int __seed16v[3]) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));
extern void lcong48(unsigned short int __param[7]) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));

/* Data structure for communication with thread safe versions.  This
   type is to be regarded as opaque.  It's only exported because users
   have to allocate objects of this type.  */
struct drand48_data {
    unsigned short int __x[3];                /* Current state.  */
    unsigned short int __old_x[3];            /* Old state.  */
    unsigned short int __c;                   /* Additive const. in congruential formula.  */
    unsigned short int __init;                /* Flag for initializing.  */
    __extension__ unsigned long long int __a; /* Factor in congruential
                           formula.  */
};

/* Return non-negative, double-precision floating-point value in [0.0,1.0).  */
extern int drand48_r(struct drand48_data *__restrict __buffer, double *__restrict __result) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(1, 2)));
extern int erand48_r(unsigned short int __xsubi[3], struct drand48_data *__restrict __buffer, double *__restrict __result)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1, 2)));

/* Return non-negative, long integer in [0,2^31).  */
extern int lrand48_r(struct drand48_data *__restrict __buffer, long int *__restrict __result) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(1, 2)));
extern int nrand48_r(unsigned short int __xsubi[3], struct drand48_data *__restrict __buffer, long int *__restrict __result)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1, 2)));

/* Return signed, long integers in [-2^31,2^31).  */
extern int mrand48_r(struct drand48_data *__restrict __buffer, long int *__restrict __result) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(1, 2)));
extern int jrand48_r(unsigned short int __xsubi[3], struct drand48_data *__restrict __buffer, long int *__restrict __result)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1, 2)));

/* Seed random number generator.  */
extern int srand48_r(long int __seedval, struct drand48_data *__buffer) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(2)));

extern int seed48_r(unsigned short int __seed16v[3], struct drand48_data *__buffer) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1, 2)));

extern int lcong48_r(unsigned short int __param[7], struct drand48_data *__buffer) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1, 2)));

/* Return a random integer between zero and 2**32-1 (inclusive).  */
extern __uint32_t arc4random(void) __attribute__((__nothrow__, __leaf__)) __attribute__((__warn_unused_result__));

/* Fill the buffer with random data.  */
extern void arc4random_buf(void *__buf, size_t __size) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));

/* Return a random number between zero (inclusive) and the specified
   limit (exclusive).  */
extern __uint32_t arc4random_uniform(__uint32_t __upper_bound) __attribute__((__nothrow__, __leaf__)) __attribute__((__warn_unused_result__));

/* Allocate SIZE bytes of memory.  */
extern void *malloc(size_t __size) __attribute__((__nothrow__, __leaf__)) __attribute__((__malloc__)) __attribute__((__alloc_size__(1)))
__attribute__((__warn_unused_result__));
/* Allocate NMEMB elements of SIZE bytes each, all initialized to 0.  */
extern void *calloc(size_t __nmemb, size_t __size) __attribute__((__nothrow__, __leaf__)) __attribute__((__malloc__)) __attribute__((__alloc_size__(1, 2)))
__attribute__((__warn_unused_result__));

/* Re-allocate the previously allocated block
   in PTR, making the new block SIZE bytes long.  */
/* __attribute_malloc__ is not used, because if realloc returns
   the same pointer that was passed to it, aliasing needs to be allowed
   between objects pointed by the old and new pointers.  */
extern void *realloc(void *__ptr, size_t __size) __attribute__((__nothrow__, __leaf__)) __attribute__((__warn_unused_result__))
__attribute__((__alloc_size__(2)));

/* Free a block allocated by `malloc', `realloc' or `calloc'.  */
extern void free(void *__ptr) __attribute__((__nothrow__, __leaf__));

/* Re-allocate the previously allocated block in PTR, making the new
   block large enough for NMEMB elements of SIZE bytes each.  */
/* __attribute_malloc__ is not used, because if reallocarray returns
   the same pointer that was passed to it, aliasing needs to be allowed
   between objects pointed by the old and new pointers.  */
extern void *reallocarray(void *__ptr, size_t __nmemb, size_t __size) __attribute__((__nothrow__, __leaf__)) __attribute__((__warn_unused_result__))
__attribute__((__alloc_size__(2, 3))) __attribute__((__malloc__(__builtin_free, 1)));

/* Add reallocarray as its own deallocator.  */
extern void *reallocarray(void *__ptr, size_t __nmemb, size_t __size) __attribute__((__nothrow__, __leaf__)) __attribute__((__malloc__(reallocarray, 1)));

/* Copyright (C) 1992-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* Copyright (C) 1991-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* Copyright (C) 1989-2022 Free Software Foundation, Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3, or (at your option)
any later version.

GCC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

Under Section 7 of GPL version 3, you are granted additional
permissions described in the GCC Runtime Library Exception, version
3.1, as published by the Free Software Foundation.

You should have received a copy of the GNU General Public License and
a copy of the GCC Runtime Library Exception along with this program;
see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
<http://www.gnu.org/licenses/>.  */

/*
 * ISO C Standard:  7.17  Common definitions  <stddef.h>
 */

/* Any one of these symbols __need_* means that GNU libc
   wants us just to define one data type.  So don't define
   the symbols that indicate this file's entire job has been done.  */
/* This avoids lossage on SunOS but only if stdtypes.h comes first.
   There's no way to win with the other order!  Sun lossage.  */
/* Sequent's header files use _PTRDIFF_T_ in some conflicting way.
   Just ignore it.  */

/* On VxWorks, <type/vxTypesBase.h> may have defined macros like
   _TYPE_size_t which will typedef size_t.  fixincludes patched the
   vxTypesBase.h so that this macro is only defined if _GCC_SIZE_T is
   not defined, and so that defining this macro defines _GCC_SIZE_T.
   If we find that the macros are still defined at this point, we must
   invoke them so that the type is defined as expected.  */
/* In case nobody has defined these types, but we aren't running under
   GCC 2.00, make sure that __PTRDIFF_TYPE__, __SIZE_TYPE__, and
   __WCHAR_TYPE__ have reasonable values.  This can happen if the
   parts of GCC is compiled by an older compiler, that actually
   include gstddef.h, such as collect2.  */

/* Signed type of difference of two pointers.  */

/* Define this type if we are doing the whole job,
   or if we want this type in particular.  */
/* If this symbol has done its job, get rid of it.  */

/* Unsigned type of `sizeof' something.  */

/* Define this type if we are doing the whole job,
   or if we want this type in particular.  */
/* Wide character type.
   Locale-writers should change this as necessary to
   be big enough to hold unique values not between 0 and 127,
   and not (wchar_t) -1, for each defined multibyte character.  */

/* Define this type if we are doing the whole job,
   or if we want this type in particular.  */
/* A null pointer constant.  */
/* Offset of member MEMBER in a struct of type TYPE. */

/* Remove any previous definition.  */

/* Allocate a block that will be freed when the calling function exits.  */
extern void *alloca(size_t __size) __attribute__((__nothrow__, __leaf__));

/* Allocate SIZE bytes on a page boundary.  The storage cannot be freed.  */
extern void *valloc(size_t __size) __attribute__((__nothrow__, __leaf__)) __attribute__((__malloc__)) __attribute__((__alloc_size__(1)))
__attribute__((__warn_unused_result__));

/* Allocate memory of SIZE bytes with an alignment of ALIGNMENT.  */
extern int posix_memalign(void **__memptr, size_t __alignment, size_t __size) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)))
__attribute__((__warn_unused_result__));

/* ISO C variant of aligned allocation.  */
extern void *aligned_alloc(size_t __alignment, size_t __size) __attribute__((__nothrow__, __leaf__)) __attribute__((__malloc__))
__attribute__((__alloc_align__(1))) __attribute__((__alloc_size__(2))) __attribute__((__warn_unused_result__));

/* Abort execution and generate a core-dump.  */
extern void abort(void) __attribute__((__nothrow__, __leaf__)) __attribute__((__noreturn__));

/* Register a function to be called when `exit' is called.  */
extern int atexit(void (*__func)(void)) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));

/* Register a function to be called when `quick_exit' is called.  */

extern int at_quick_exit(void (*__func)(void)) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));

/* Register a function to be called with the status
   given to `exit' and the given argument.  */
extern int on_exit(void (*__func)(int __status, void *__arg), void *__arg) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));

/* Call all functions registered with `atexit' and `on_exit',
   in the reverse of the order in which they were registered,
   perform stdio cleanup, and terminate program execution with STATUS.  */
extern void exit(int __status) __attribute__((__nothrow__, __leaf__)) __attribute__((__noreturn__));

/* Call all functions registered with `at_quick_exit' in the reverse
   of the order in which they were registered and terminate program
   execution with STATUS.  */
extern void quick_exit(int __status) __attribute__((__nothrow__, __leaf__)) __attribute__((__noreturn__));

/* Terminate the program with STATUS without calling any of the
   functions registered with `atexit' or `on_exit'.  */
extern void _Exit(int __status) __attribute__((__nothrow__, __leaf__)) __attribute__((__noreturn__));

/* Return the value of envariable NAME, or NULL if it doesn't exist.  */
extern char *getenv(const char *__name) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1))) __attribute__((__warn_unused_result__));

/* This function is similar to the above but returns NULL if the
   programs is running with SUID or SGID enabled.  */
extern char *secure_getenv(const char *__name) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1))) __attribute__((__warn_unused_result__));

/* The SVID says this is in <stdio.h>, but this seems a better place.	*/
/* Put STRING, which is of the form "NAME=VALUE", in the environment.
   If there is no `=', remove NAME from the environment.  */
extern int putenv(char *__string) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));

/* Set NAME to VALUE in the environment.
   If REPLACE is nonzero, overwrite an existing value.  */
extern int setenv(const char *__name, const char *__value, int __replace) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(2)));

/* Remove the variable NAME from the environment.  */
extern int unsetenv(const char *__name) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));

/* The `clearenv' was planned to be added to POSIX.1 but probably
   never made it.  Nevertheless the POSIX.9 standard (POSIX bindings
   for Fortran 77) requires this function.  */
extern int clearenv(void) __attribute__((__nothrow__, __leaf__));

/* Generate a unique temporary file name from TEMPLATE.
   The last six characters of TEMPLATE must be "XXXXXX";
   they are replaced with a string that makes the file name unique.
   Always returns TEMPLATE, it's either a temporary file name or a null
   string if it cannot get a unique file name.  */
extern char *mktemp(char *__template) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));

/* Generate a unique temporary file name from TEMPLATE.
   The last six characters of TEMPLATE must be "XXXXXX";
   they are replaced with a string that makes the filename unique.
   Returns a file descriptor open on the file for reading and writing,
   or -1 if it cannot create a uniquely-named file.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */

extern int mkstemp(char *__template) __attribute__((__nonnull__(1))) __attribute__((__warn_unused_result__));
extern int mkstemp64(char *__template) __attribute__((__nonnull__(1))) __attribute__((__warn_unused_result__));

/* Similar to mkstemp, but the template can have a suffix after the
   XXXXXX.  The length of the suffix is specified in the second
   parameter.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */

extern int mkstemps(char *__template, int __suffixlen) __attribute__((__nonnull__(1))) __attribute__((__warn_unused_result__));
extern int mkstemps64(char *__template, int __suffixlen) __attribute__((__nonnull__(1))) __attribute__((__warn_unused_result__));

/* Create a unique temporary directory from TEMPLATE.
   The last six characters of TEMPLATE must be "XXXXXX";
   they are replaced with a string that makes the directory name unique.
   Returns TEMPLATE, or a null pointer if it cannot get a unique name.
   The directory is created mode 700.  */
extern char *mkdtemp(char *__template) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1))) __attribute__((__warn_unused_result__));

/* Generate a unique temporary file name from TEMPLATE similar to
   mkstemp.  But allow the caller to pass additional flags which are
   used in the open call to create the file..

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */

extern int mkostemp(char *__template, int __flags) __attribute__((__nonnull__(1))) __attribute__((__warn_unused_result__));
extern int mkostemp64(char *__template, int __flags) __attribute__((__nonnull__(1))) __attribute__((__warn_unused_result__));

/* Similar to mkostemp, but the template can have a suffix after the
   XXXXXX.  The length of the suffix is specified in the second
   parameter.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */

extern int mkostemps(char *__template, int __suffixlen, int __flags) __attribute__((__nonnull__(1))) __attribute__((__warn_unused_result__));
extern int mkostemps64(char *__template, int __suffixlen, int __flags) __attribute__((__nonnull__(1))) __attribute__((__warn_unused_result__));

/* Execute the given line as a shell command.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
extern int system(const char *__command) __attribute__((__warn_unused_result__));

/* Return a malloc'd string containing the canonical absolute name of the
   existing named file.  */
extern char *canonicalize_file_name(const char *__name) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1))) __attribute__((__malloc__))
__attribute__((__malloc__(__builtin_free, 1))) __attribute__((__warn_unused_result__));

/* Return the canonical absolute name of file NAME.  If RESOLVED is
   null, the result is malloc'd; otherwise, if the canonical name is
   PATH_MAX chars or more, returns null with `errno' set to
   ENAMETOOLONG; if the name fits in fewer than PATH_MAX chars,
   returns the name in RESOLVED.  */
extern char *realpath(const char *__restrict __name, char *__restrict __resolved) __attribute__((__nothrow__, __leaf__))
__attribute__((__warn_unused_result__));

/* Shorthand for type of comparison functions.  */

typedef int (*__compar_fn_t)(const void *, const void *);

typedef __compar_fn_t comparison_fn_t;

typedef int (*__compar_d_fn_t)(const void *, const void *, void *);

/* Do a binary search for KEY in BASE, which consists of NMEMB elements
   of SIZE bytes each, using COMPAR to perform the comparisons.  */
extern void *bsearch(const void *__key, const void *__base, size_t __nmemb, size_t __size, __compar_fn_t __compar) __attribute__((__nonnull__(1, 2, 5)))
__attribute__((__warn_unused_result__));

/* Perform binary search - inline version.
   Copyright (C) 1991-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

extern __inline __attribute__((__gnu_inline__)) void *
bsearch(const void *__key, const void *__base, size_t __nmemb, size_t __size, __compar_fn_t __compar)
{
    size_t __l, __u, __idx;
    const void *__p;
    int __comparison;

    __l = 0;
    __u = __nmemb;
    while (__l < __u) {
        __idx = (__l + __u) / 2;
        __p = (const void *)(((const char *)__base) + (__idx * __size));
        __comparison = (*__compar)(__key, __p);
        if (__comparison < 0)
            __u = __idx;
        else if (__comparison > 0)
            __l = __idx + 1;
        else {

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"

            return (void *)__p;

#pragma GCC diagnostic pop
        }
    }

    return ((void *)0);
}

/* Sort NMEMB elements of BASE, of SIZE bytes each,
   using COMPAR to perform the comparisons.  */
extern void qsort(void *__base, size_t __nmemb, size_t __size, __compar_fn_t __compar) __attribute__((__nonnull__(1, 4)));

extern void qsort_r(void *__base, size_t __nmemb, size_t __size, __compar_d_fn_t __compar, void *__arg) __attribute__((__nonnull__(1, 4)));

/* Return the absolute value of X.  */
extern int abs(int __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__)) __attribute__((__warn_unused_result__));
extern long int labs(long int __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__)) __attribute__((__warn_unused_result__));

__extension__ extern long long int llabs(long long int __x) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__))
__attribute__((__warn_unused_result__));

/* Return the `div_t', `ldiv_t' or `lldiv_t' representation
   of the value of NUMER over DENOM. */
/* GCC may have built-ins for these someday.  */
extern div_t div(int __numer, int __denom) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__)) __attribute__((__warn_unused_result__));
extern ldiv_t ldiv(long int __numer, long int __denom) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__))
__attribute__((__warn_unused_result__));

__extension__ extern lldiv_t lldiv(long long int __numer, long long int __denom) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__))
__attribute__((__warn_unused_result__));

/* Convert floating point numbers to strings.  The returned values are
   valid only until another call to the same function.  */

/* Convert VALUE to a string with NDIGIT digits and return a pointer to
   this.  Set *DECPT with the position of the decimal character and *SIGN
   with the sign of the number.  */
extern char *ecvt(double __value, int __ndigit, int *__restrict __decpt, int *__restrict __sign) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(3, 4))) __attribute__((__warn_unused_result__));

/* Convert VALUE to a string rounded to NDIGIT decimal digits.  Set *DECPT
   with the position of the decimal character and *SIGN with the sign of
   the number.  */
extern char *fcvt(double __value, int __ndigit, int *__restrict __decpt, int *__restrict __sign) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(3, 4))) __attribute__((__warn_unused_result__));

/* If possible convert VALUE to a string with NDIGIT significant digits.
   Otherwise use exponential representation.  The resulting string will
   be written to BUF.  */
extern char *gcvt(double __value, int __ndigit, char *__buf) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(3)))
__attribute__((__warn_unused_result__));

/* Long double versions of above functions.  */
extern char *qecvt(long double __value, int __ndigit, int *__restrict __decpt, int *__restrict __sign) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(3, 4))) __attribute__((__warn_unused_result__));
extern char *qfcvt(long double __value, int __ndigit, int *__restrict __decpt, int *__restrict __sign) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(3, 4))) __attribute__((__warn_unused_result__));
extern char *qgcvt(long double __value, int __ndigit, char *__buf) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(3)))
__attribute__((__warn_unused_result__));

/* Reentrant version of the functions above which provide their own
   buffers.  */
extern int ecvt_r(double __value, int __ndigit, int *__restrict __decpt, int *__restrict __sign, char *__restrict __buf, size_t __len)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(3, 4, 5)));
extern int fcvt_r(double __value, int __ndigit, int *__restrict __decpt, int *__restrict __sign, char *__restrict __buf, size_t __len)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(3, 4, 5)));

extern int qecvt_r(long double __value, int __ndigit, int *__restrict __decpt, int *__restrict __sign, char *__restrict __buf, size_t __len)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(3, 4, 5)));
extern int qfcvt_r(long double __value, int __ndigit, int *__restrict __decpt, int *__restrict __sign, char *__restrict __buf, size_t __len)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(3, 4, 5)));

/* Return the length of the multibyte character
   in S, which is no longer than N.  */
extern int mblen(const char *__s, size_t __n) __attribute__((__nothrow__, __leaf__));
/* Return the length of the given multibyte character,
   putting its `wchar_t' representation in *PWC.  */
extern int mbtowc(wchar_t *__restrict __pwc, const char *__restrict __s, size_t __n) __attribute__((__nothrow__, __leaf__));
/* Put the multibyte character represented
   by WCHAR in S, returning its length.  */
extern int wctomb(char *__s, wchar_t __wchar) __attribute__((__nothrow__, __leaf__));

/* Convert a multibyte string to a wide char string.  */
extern size_t mbstowcs(wchar_t *__restrict __pwcs, const char *__restrict __s, size_t __n) __attribute__((__nothrow__, __leaf__))
__attribute__((__access__(__read_only__, 2)));
/* Convert a wide char string to multibyte string.  */
extern size_t wcstombs(char *__restrict __s, const wchar_t *__restrict __pwcs, size_t __n) __attribute__((__nothrow__, __leaf__))
__attribute__((__access__(__write_only__, 1))) __attribute__((__access__(__read_only__, 2)));

/* Determine whether the string value of RESPONSE matches the affirmation
   or negative response expression as specified by the LC_MESSAGES category
   in the program's current locale.  Returns 1 if affirmative, 0 if
   negative, and -1 if not matching.  */
extern int rpmatch(const char *__response) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1))) __attribute__((__warn_unused_result__));

/* Parse comma separated suboption from *OPTIONP and match against
   strings in TOKENS.  If found return index and set *VALUEP to
   optional value introduced by an equal sign.  If the suboption is
   not part of TOKENS return in *VALUEP beginning of unknown
   suboption.  On exit *OPTIONP is set to the beginning of the next
   token or at the terminating NUL character.  */
extern int getsubopt(char **__restrict __optionp, char *const *__restrict __tokens, char **__restrict __valuep) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(1, 2, 3))) __attribute__((__warn_unused_result__));

/* X/Open pseudo terminal handling.  */

/* Return a master pseudo-terminal handle.  */
extern int posix_openpt(int __oflag) __attribute__((__warn_unused_result__));

/* The next four functions all take a master pseudo-tty fd and
   perform an operation on the associated slave:  */

/* Chown the slave to the calling user.  */
extern int grantpt(int __fd) __attribute__((__nothrow__, __leaf__));

/* Release an internal lock so the slave can be opened.
   Call after grantpt().  */
extern int unlockpt(int __fd) __attribute__((__nothrow__, __leaf__));

/* Return the pathname of the pseudo terminal slave associated with
   the master FD is open on, or NULL on errors.
   The returned storage is good until the next call to this function.  */
extern char *ptsname(int __fd) __attribute__((__nothrow__, __leaf__)) __attribute__((__warn_unused_result__));

/* Store at most BUFLEN characters of the pathname of the slave pseudo
   terminal associated with the master FD is open on in BUF.
   Return 0 on success, otherwise an error number.  */
extern int ptsname_r(int __fd, char *__buf, size_t __buflen) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(2)))
__attribute__((__access__(__write_only__, 2)));

/* Open a master pseudo terminal and return its file descriptor.  */
extern int getpt(void);

/* Put the 1 minute, 5 minute and 15 minute load averages into the first
   NELEM elements of LOADAVG.  Return the number written (never more than
   three, but may be less than NELEM), or -1 if an error occurred.  */
extern int getloadavg(double __loadavg[], int __nelem) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));
/* Floating-point inline functions for stdlib.h.
   Copyright (C) 2012-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

extern __inline __attribute__((__gnu_inline__)) double __attribute__((__nothrow__, __leaf__)) atof(const char *__nptr)
{
    return strtod(__nptr, (char **)((void *)0));
}

/* Define some macros helping to catch buffer overflows.  */

/* Checking macros for stdlib functions.
   Copyright (C) 2005-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

extern char *__realpath_chk(const char *__restrict __name, char *__restrict __resolved, size_t __resolvedlen) __attribute__((__nothrow__, __leaf__))
__attribute__((__warn_unused_result__));
extern char *__realpath_alias(const char *__restrict __name, char *__restrict __resolved) __asm__(""
                                                                                                  "realpath") __attribute__((__nothrow__, __leaf__))

__attribute__((__warn_unused_result__));
extern char *__realpath_chk_warn(const char *__restrict __name, char *__restrict __resolved, size_t __resolvedlen) __asm__(""
                                                                                                                           "__realpath_chk")
    __attribute__((__nothrow__, __leaf__))

    __attribute__((__warn_unused_result__)) __attribute__((__warning__("second argument of realpath must be either NULL or at "
                                                                       "least PATH_MAX bytes long buffer")));

extern __inline __attribute__((__always_inline__)) __attribute__((__gnu_inline__)) __attribute__((__artificial__))
__attribute__((__warn_unused_result__)) char *__attribute__((__nothrow__, __leaf__)) realpath(const char *__restrict __name, char *__restrict __resolved)
{
    size_t sz = __builtin_dynamic_object_size(__resolved, 1);

    if (sz == (size_t)-1)
        return __realpath_alias(__name, __resolved);

    return __realpath_chk(__name, __resolved, sz);
}

extern int __ptsname_r_chk(int __fd, char *__buf, size_t __buflen, size_t __nreal) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(2)))
__attribute__((__access__(__write_only__, 2, 3)));
extern int __ptsname_r_alias(int __fd, char *__buf, size_t __buflen) __asm__(""
                                                                             "ptsname_r") __attribute__((__nothrow__, __leaf__))

__attribute__((__nonnull__(2))) __attribute__((__access__(__write_only__, 2, 3)));
extern int __ptsname_r_chk_warn(int __fd, char *__buf, size_t __buflen, size_t __nreal) __asm__(""
                                                                                                "__ptsname_r_chk") __attribute__((__nothrow__, __leaf__))

__attribute__((__nonnull__(2))) __attribute__((__warning__("ptsname_r called with buflen bigger than "
                                                           "size of buf")));

extern __inline __attribute__((__always_inline__)) __attribute__((__gnu_inline__)) __attribute__((__artificial__)) int __attribute__((__nothrow__, __leaf__))
ptsname_r(int __fd, char *__buf, size_t __buflen)
{
    return (((__builtin_constant_p(__builtin_dynamic_object_size(__buf, 1)) && (__builtin_dynamic_object_size(__buf, 1)) == (long unsigned int)-1) ||
                (((__typeof(__buflen))0 < (__typeof(__buflen))-1 || (__builtin_constant_p(__buflen) && (__buflen) > 0)) &&
                    __builtin_constant_p((((long unsigned int)(__buflen)) <= ((__builtin_dynamic_object_size(__buf, 1))) / ((sizeof(char))))) &&
                    (((long unsigned int)(__buflen)) <= ((__builtin_dynamic_object_size(__buf, 1))) / ((sizeof(char)))))) ?
            __ptsname_r_alias(__fd, __buf, __buflen) :
            ((((__typeof(__buflen))0 < (__typeof(__buflen))-1 || (__builtin_constant_p(__buflen) && (__buflen) > 0)) &&
                 __builtin_constant_p((((long unsigned int)(__buflen)) <= (__builtin_dynamic_object_size(__buf, 1)) / (sizeof(char)))) &&
                 !(((long unsigned int)(__buflen)) <= (__builtin_dynamic_object_size(__buf, 1)) / (sizeof(char)))) ?
                    __ptsname_r_chk_warn(__fd, __buf, __buflen, __builtin_dynamic_object_size(__buf, 1)) :
                    __ptsname_r_chk(__fd, __buf, __buflen, __builtin_dynamic_object_size(__buf, 1))))

        ;
}

extern int __wctomb_chk(char *__s, wchar_t __wchar, size_t __buflen) __attribute__((__nothrow__, __leaf__)) __attribute__((__warn_unused_result__));
extern int __wctomb_alias(char *__s, wchar_t __wchar) __asm__(""
                                                              "wctomb") __attribute__((__nothrow__, __leaf__)) __attribute__((__warn_unused_result__));

extern __inline __attribute__((__always_inline__)) __attribute__((__gnu_inline__)) __attribute__((__artificial__)) __attribute__((__warn_unused_result__)) int
    __attribute__((__nothrow__, __leaf__)) wctomb(char *__s, wchar_t __wchar)
{
    /* We would have to include <limits.h> to get a definition of MB_LEN_MAX.
       But this would only disturb the namespace.  So we define our own
       version here.  */

    if (__builtin_dynamic_object_size(__s, 1) != (size_t)-1 && 16 > __builtin_dynamic_object_size(__s, 1))
        return __wctomb_chk(__s, __wchar, __builtin_dynamic_object_size(__s, 1));
    return __wctomb_alias(__s, __wchar);
}

extern size_t __mbstowcs_chk(wchar_t *__restrict __dst, const char *__restrict __src, size_t __len, size_t __dstlen) __attribute__((__nothrow__, __leaf__))
__attribute__((__access__(__write_only__, 1, 3))) __attribute__((__access__(__read_only__, 2)));
extern size_t __mbstowcs_nulldst(wchar_t *__restrict __dst, const char *__restrict __src, size_t __len) __asm__(""
                                                                                                                "mbstowcs")
    __attribute__((__nothrow__, __leaf__))

    __attribute__((__access__(__read_only__, 2)));
extern size_t __mbstowcs_alias(wchar_t *__restrict __dst, const char *__restrict __src, size_t __len) __asm__(""
                                                                                                              "mbstowcs") __attribute__((__nothrow__, __leaf__))

__attribute__((__access__(__write_only__, 1, 3))) __attribute__((__access__(__read_only__, 2)));
extern size_t __mbstowcs_chk_warn(wchar_t *__restrict __dst, const char *__restrict __src, size_t __len, size_t __dstlen) __asm__(""
                                                                                                                                  "__mbstowcs_chk")
    __attribute__((__nothrow__, __leaf__))

    __attribute__((__warning__("mbstowcs called with dst buffer smaller than len "
                               "* sizeof (wchar_t)")));

extern __inline __attribute__((__always_inline__)) __attribute__((__gnu_inline__)) __attribute__((__artificial__)) size_t __attribute__((__nothrow__, __leaf__))
mbstowcs(wchar_t *__restrict __dst, const char *__restrict __src, size_t __len)

{
    if (__builtin_constant_p(__dst == ((void *)0)) && __dst == ((void *)0))
        return __mbstowcs_nulldst(__dst, __src, __len);
    else
        return (((__builtin_constant_p(__builtin_dynamic_object_size(__dst, 1)) && (__builtin_dynamic_object_size(__dst, 1)) == (long unsigned int)-1) ||
                    (((__typeof(__len))0 < (__typeof(__len))-1 || (__builtin_constant_p(__len) && (__len) > 0)) &&
                        __builtin_constant_p((((long unsigned int)(__len)) <= ((__builtin_dynamic_object_size(__dst, 1))) / ((sizeof(wchar_t))))) &&
                        (((long unsigned int)(__len)) <= ((__builtin_dynamic_object_size(__dst, 1))) / ((sizeof(wchar_t)))))) ?
                __mbstowcs_alias(__dst, __src, __len) :
                ((((__typeof(__len))0 < (__typeof(__len))-1 || (__builtin_constant_p(__len) && (__len) > 0)) &&
                     __builtin_constant_p((((long unsigned int)(__len)) <= (__builtin_dynamic_object_size(__dst, 1)) / (sizeof(wchar_t)))) &&
                     !(((long unsigned int)(__len)) <= (__builtin_dynamic_object_size(__dst, 1)) / (sizeof(wchar_t)))) ?
                        __mbstowcs_chk_warn(__dst, __src, __len, (__builtin_dynamic_object_size(__dst, 1)) / (sizeof(wchar_t))) :
                        __mbstowcs_chk(__dst, __src, __len, (__builtin_dynamic_object_size(__dst, 1)) / (sizeof(wchar_t)))));
}

extern size_t __wcstombs_chk(char *__restrict __dst, const wchar_t *__restrict __src, size_t __len, size_t __dstlen) __attribute__((__nothrow__, __leaf__))
__attribute__((__access__(__write_only__, 1, 3))) __attribute__((__access__(__read_only__, 2)));
extern size_t __wcstombs_alias(char *__restrict __dst, const wchar_t *__restrict __src, size_t __len) __asm__(""
                                                                                                              "wcstombs") __attribute__((__nothrow__, __leaf__))

__attribute__((__access__(__write_only__, 1, 3))) __attribute__((__access__(__read_only__, 2)));
extern size_t __wcstombs_chk_warn(char *__restrict __dst, const wchar_t *__restrict __src, size_t __len, size_t __dstlen) __asm__(""
                                                                                                                                  "__wcstombs_chk")
    __attribute__((__nothrow__, __leaf__))

    __attribute__((__warning__("wcstombs called with dst buffer smaller than len")));

extern __inline __attribute__((__always_inline__)) __attribute__((__gnu_inline__)) __attribute__((__artificial__)) size_t __attribute__((__nothrow__, __leaf__))
wcstombs(char *__restrict __dst, const wchar_t *__restrict __src, size_t __len)

{
    return (((__builtin_constant_p(__builtin_dynamic_object_size(__dst, 1)) && (__builtin_dynamic_object_size(__dst, 1)) == (long unsigned int)-1) ||
                (((__typeof(__len))0 < (__typeof(__len))-1 || (__builtin_constant_p(__len) && (__len) > 0)) &&
                    __builtin_constant_p((((long unsigned int)(__len)) <= ((__builtin_dynamic_object_size(__dst, 1))) / ((sizeof(char))))) &&
                    (((long unsigned int)(__len)) <= ((__builtin_dynamic_object_size(__dst, 1))) / ((sizeof(char)))))) ?
            __wcstombs_alias(__dst, __src, __len) :
            ((((__typeof(__len))0 < (__typeof(__len))-1 || (__builtin_constant_p(__len) && (__len) > 0)) &&
                 __builtin_constant_p((((long unsigned int)(__len)) <= (__builtin_dynamic_object_size(__dst, 1)) / (sizeof(char)))) &&
                 !(((long unsigned int)(__len)) <= (__builtin_dynamic_object_size(__dst, 1)) / (sizeof(char)))) ?
                    __wcstombs_chk_warn(__dst, __src, __len, __builtin_dynamic_object_size(__dst, 1)) :
                    __wcstombs_chk(__dst, __src, __len, __builtin_dynamic_object_size(__dst, 1))))

        ;
}

/* Macros to control TS 18661-3 glibc features on x86.
   Copyright (C) 2017-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* Copyright (C) 1991-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/*
 *	ISO C99 Standard: 7.21 String handling	<string.h>
 */

/* Handle feature test macros at the start of a header.
   Copyright (C) 2016-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* This header is internal to glibc and should not be included outside
   of glibc headers.  Headers including it must define
   __GLIBC_INTERNAL_STARTING_HEADER_IMPLEMENTATION first.  This header
   cannot have multiple include guards because ISO C feature test
   macros depend on the definition of the macro when an affected
   header is included, not when the first system header is
   included.  */

/* Copyright (C) 1991-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* ISO/IEC TR 24731-2:2010 defines the __STDC_WANT_LIB_EXT2__
   macro.  */
/* ISO/IEC TS 18661-1:2014 defines the __STDC_WANT_IEC_60559_BFP_EXT__
   macro.  Most but not all symbols enabled by that macro in TS
   18661-1 are enabled unconditionally in C2X.  In C2X, the symbols in
   Annex F still require a new feature test macro
   __STDC_WANT_IEC_60559_EXT__ instead (C2X does not define
   __STDC_WANT_IEC_60559_BFP_EXT__), while a few features from TS
   18661-1 are not included in C2X (and thus should depend on
   __STDC_WANT_IEC_60559_BFP_EXT__ even when C2X features are
   enabled).

   __GLIBC_USE (IEC_60559_BFP_EXT) controls those features from TS
   18661-1 not included in C2X.

   __GLIBC_USE (IEC_60559_BFP_EXT_C2X) controls those features from TS
   18661-1 that are also included in C2X (with no feature test macro
   required in C2X).

   __GLIBC_USE (IEC_60559_EXT) controls those features from TS 18661-1
   that are included in C2X but conditional on
   __STDC_WANT_IEC_60559_EXT__.  (There are currently no features
   conditional on __STDC_WANT_IEC_60559_EXT__ that are not in TS
   18661-1.)  */
/* ISO/IEC TS 18661-4:2015 defines the
   __STDC_WANT_IEC_60559_FUNCS_EXT__ macro.  Other than the reduction
   functions, the symbols from this TS are enabled unconditionally in
   C2X.  */
/* ISO/IEC TS 18661-3:2015 defines the
   __STDC_WANT_IEC_60559_TYPES_EXT__ macro.  */

/* Get size_t and NULL from <stddef.h>.  */

/* Copyright (C) 1989-2022 Free Software Foundation, Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3, or (at your option)
any later version.

GCC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

Under Section 7 of GPL version 3, you are granted additional
permissions described in the GCC Runtime Library Exception, version
3.1, as published by the Free Software Foundation.

You should have received a copy of the GNU General Public License and
a copy of the GCC Runtime Library Exception along with this program;
see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
<http://www.gnu.org/licenses/>.  */

/*
 * ISO C Standard:  7.17  Common definitions  <stddef.h>
 */

/* Any one of these symbols __need_* means that GNU libc
   wants us just to define one data type.  So don't define
   the symbols that indicate this file's entire job has been done.  */
/* This avoids lossage on SunOS but only if stdtypes.h comes first.
   There's no way to win with the other order!  Sun lossage.  */
/* Sequent's header files use _PTRDIFF_T_ in some conflicting way.
   Just ignore it.  */

/* On VxWorks, <type/vxTypesBase.h> may have defined macros like
   _TYPE_size_t which will typedef size_t.  fixincludes patched the
   vxTypesBase.h so that this macro is only defined if _GCC_SIZE_T is
   not defined, and so that defining this macro defines _GCC_SIZE_T.
   If we find that the macros are still defined at this point, we must
   invoke them so that the type is defined as expected.  */
/* In case nobody has defined these types, but we aren't running under
   GCC 2.00, make sure that __PTRDIFF_TYPE__, __SIZE_TYPE__, and
   __WCHAR_TYPE__ have reasonable values.  This can happen if the
   parts of GCC is compiled by an older compiler, that actually
   include gstddef.h, such as collect2.  */

/* Signed type of difference of two pointers.  */

/* Define this type if we are doing the whole job,
   or if we want this type in particular.  */
/* If this symbol has done its job, get rid of it.  */

/* Unsigned type of `sizeof' something.  */

/* Define this type if we are doing the whole job,
   or if we want this type in particular.  */
/* Wide character type.
   Locale-writers should change this as necessary to
   be big enough to hold unique values not between 0 and 127,
   and not (wchar_t) -1, for each defined multibyte character.  */

/* Define this type if we are doing the whole job,
   or if we want this type in particular.  */
/* A null pointer constant.  */
/* Offset of member MEMBER in a struct of type TYPE. */

/* Tell the caller that we provide correct C++ prototypes.  */

/* Copy N bytes of SRC to DEST.  */
extern void *memcpy(void *__restrict __dest, const void *__restrict __src, size_t __n) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(1, 2)));
/* Copy N bytes of SRC to DEST, guaranteeing
   correct behavior for overlapping strings.  */
extern void *memmove(void *__dest, const void *__src, size_t __n) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1, 2)));

/* Copy no more than N bytes of SRC to DEST, stopping when C is found.
   Return the position in DEST one byte past where C was copied,
   or NULL if C was not found in the first N bytes of SRC.  */

extern void *memccpy(void *__restrict __dest, const void *__restrict __src, int __c, size_t __n) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(1, 2))) __attribute__((__access__(__write_only__, 1, 4)));

/* Set N bytes of S to C.  */
extern void *memset(void *__s, int __c, size_t __n) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));

/* Compare N bytes of S1 and S2.  */
extern int memcmp(const void *__s1, const void *__s2, size_t __n) __attribute__((__nothrow__, __leaf__)) __attribute__((__pure__))
__attribute__((__nonnull__(1, 2)));

/* Compare N bytes of S1 and S2.  Return zero if S1 and S2 are equal.
   Return some non-zero value otherwise.

   Essentially __memcmpeq has the exact same semantics as memcmp
   except the return value is less constrained.  memcmp is always a
   correct implementation of __memcmpeq.  As well !!memcmp, -memcmp,
   or bcmp are correct implementations.

   __memcmpeq is meant to be used by compilers when memcmp return is
   only used for its boolean value.

   __memcmpeq is declared only for use by compilers.  Programs should
   continue to use memcmp.  */
extern int __memcmpeq(const void *__s1, const void *__s2, size_t __n) __attribute__((__nothrow__, __leaf__)) __attribute__((__pure__))
__attribute__((__nonnull__(1, 2)));

/* Search N bytes of S for C.  */
extern void *memchr(const void *__s, int __c, size_t __n) __attribute__((__nothrow__, __leaf__)) __attribute__((__pure__)) __attribute__((__nonnull__(1)));

/* Search in S for C.  This is similar to `memchr' but there is no
   length limit.  */

extern void *rawmemchr(const void *__s, int __c) __attribute__((__nothrow__, __leaf__)) __attribute__((__pure__)) __attribute__((__nonnull__(1)));

/* Search N bytes of S for the final occurrence of C.  */
extern void *memrchr(const void *__s, int __c, size_t __n) __attribute__((__nothrow__, __leaf__)) __attribute__((__pure__)) __attribute__((__nonnull__(1)))
__attribute__((__access__(__read_only__, 1, 3)));

/* Copy SRC to DEST.  */
extern char *strcpy(char *__restrict __dest, const char *__restrict __src) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1, 2)));
/* Copy no more than N characters of SRC to DEST.  */
extern char *strncpy(char *__restrict __dest, const char *__restrict __src, size_t __n) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(1, 2)));

/* Append SRC onto DEST.  */
extern char *strcat(char *__restrict __dest, const char *__restrict __src) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1, 2)));
/* Append no more than N characters from SRC onto DEST.  */
extern char *strncat(char *__restrict __dest, const char *__restrict __src, size_t __n) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(1, 2)));

/* Compare S1 and S2.  */
extern int strcmp(const char *__s1, const char *__s2) __attribute__((__nothrow__, __leaf__)) __attribute__((__pure__)) __attribute__((__nonnull__(1, 2)));
/* Compare N characters of S1 and S2.  */
extern int strncmp(const char *__s1, const char *__s2, size_t __n) __attribute__((__nothrow__, __leaf__)) __attribute__((__pure__))
__attribute__((__nonnull__(1, 2)));

/* Compare the collated forms of S1 and S2.  */
extern int strcoll(const char *__s1, const char *__s2) __attribute__((__nothrow__, __leaf__)) __attribute__((__pure__)) __attribute__((__nonnull__(1, 2)));
/* Put a transformation of SRC into no more than N bytes of DEST.  */
extern size_t strxfrm(char *__restrict __dest, const char *__restrict __src, size_t __n) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(2)))
__attribute__((__access__(__write_only__, 1, 3)));

/* POSIX.1-2008 extended locale interface (see locale.h).  */
/* Definition of locale_t.
   Copyright (C) 2017-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* Compare the collated forms of S1 and S2, using sorting rules from L.  */
extern int strcoll_l(const char *__s1, const char *__s2, locale_t __l) __attribute__((__nothrow__, __leaf__)) __attribute__((__pure__))
__attribute__((__nonnull__(1, 2, 3)));
/* Put a transformation of SRC into no more than N bytes of DEST,
   using sorting rules from L.  */
extern size_t strxfrm_l(char *__dest, const char *__src, size_t __n, locale_t __l) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(2, 4)))
__attribute__((__access__(__write_only__, 1, 3)));

/* Duplicate S, returning an identical malloc'd string.  */
extern char *strdup(const char *__s) __attribute__((__nothrow__, __leaf__)) __attribute__((__malloc__)) __attribute__((__nonnull__(1)));

/* Return a malloc'd copy of at most N bytes of STRING.  The
   resultant string is terminated even if no null terminator
   appears before STRING[N].  */

extern char *strndup(const char *__string, size_t __n) __attribute__((__nothrow__, __leaf__)) __attribute__((__malloc__)) __attribute__((__nonnull__(1)));

/* Duplicate S, returning an identical alloca'd string.  */
/* Return an alloca'd copy of at most N bytes of string.  */
/* Find the first occurrence of C in S.  */
extern char *strchr(const char *__s, int __c) __attribute__((__nothrow__, __leaf__)) __attribute__((__pure__)) __attribute__((__nonnull__(1)));

/* Find the last occurrence of C in S.  */
extern char *strrchr(const char *__s, int __c) __attribute__((__nothrow__, __leaf__)) __attribute__((__pure__)) __attribute__((__nonnull__(1)));

/* This function is similar to `strchr'.  But it returns a pointer to
   the closing NUL byte in case C is not found in S.  */

extern char *strchrnul(const char *__s, int __c) __attribute__((__nothrow__, __leaf__)) __attribute__((__pure__)) __attribute__((__nonnull__(1)));

/* Return the length of the initial segment of S which
   consists entirely of characters not in REJECT.  */
extern size_t strcspn(const char *__s, const char *__reject) __attribute__((__nothrow__, __leaf__)) __attribute__((__pure__))
__attribute__((__nonnull__(1, 2)));
/* Return the length of the initial segment of S which
   consists entirely of characters in ACCEPT.  */
extern size_t strspn(const char *__s, const char *__accept) __attribute__((__nothrow__, __leaf__)) __attribute__((__pure__)) __attribute__((__nonnull__(1, 2)));
/* Find the first occurrence in S of any character in ACCEPT.  */
extern char *strpbrk(const char *__s, const char *__accept) __attribute__((__nothrow__, __leaf__)) __attribute__((__pure__)) __attribute__((__nonnull__(1, 2)));

/* Find the first occurrence of NEEDLE in HAYSTACK.  */
extern char *strstr(const char *__haystack, const char *__needle) __attribute__((__nothrow__, __leaf__)) __attribute__((__pure__))
__attribute__((__nonnull__(1, 2)));

/* Divide S into tokens separated by characters in DELIM.  */
extern char *strtok(char *__restrict __s, const char *__restrict __delim) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(2)));

/* Divide S into tokens separated by characters in DELIM.  Information
   passed between calls are stored in SAVE_PTR.  */
extern char *__strtok_r(char *__restrict __s, const char *__restrict __delim, char **__restrict __save_ptr) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(2, 3)));

extern char *strtok_r(char *__restrict __s, const char *__restrict __delim, char **__restrict __save_ptr) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(2, 3)));

/* Similar to `strstr' but this function ignores the case of both strings.  */

extern char *strcasestr(const char *__haystack, const char *__needle) __attribute__((__nothrow__, __leaf__)) __attribute__((__pure__))
__attribute__((__nonnull__(1, 2)));

/* Find the first occurrence of NEEDLE in HAYSTACK.
   NEEDLE is NEEDLELEN bytes long;
   HAYSTACK is HAYSTACKLEN bytes long.  */
extern void *memmem(const void *__haystack, size_t __haystacklen, const void *__needle, size_t __needlelen) __attribute__((__nothrow__, __leaf__))
__attribute__((__pure__)) __attribute__((__nonnull__(1, 3))) __attribute__((__access__(__read_only__, 1, 2))) __attribute__((__access__(__read_only__, 3, 4)));

/* Copy N bytes of SRC to DEST, return pointer to bytes after the
   last written byte.  */
extern void *__mempcpy(void *__restrict __dest, const void *__restrict __src, size_t __n) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(1, 2)));
extern void *mempcpy(void *__restrict __dest, const void *__restrict __src, size_t __n) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(1, 2)));

/* Return the length of S.  */
extern size_t strlen(const char *__s) __attribute__((__nothrow__, __leaf__)) __attribute__((__pure__)) __attribute__((__nonnull__(1)));

/* Find the length of STRING, but scan at most MAXLEN characters.
   If no '\0' terminator is found in that many characters, return MAXLEN.  */
extern size_t strnlen(const char *__string, size_t __maxlen) __attribute__((__nothrow__, __leaf__)) __attribute__((__pure__)) __attribute__((__nonnull__(1)));

/* Return a string describing the meaning of the `errno' code in ERRNUM.  */
extern char *strerror(int __errnum) __attribute__((__nothrow__, __leaf__));

/* Reentrant version of `strerror'.
   There are 2 flavors of `strerror_r', GNU which returns the string
   and may or may not use the supplied temporary buffer and POSIX one
   which fills the string into the buffer.
   To use the POSIX version, -D_XOPEN_SOURCE=600 or -D_POSIX_C_SOURCE=200112L
   without -D_GNU_SOURCE is needed, otherwise the GNU version is
   preferred.  */
/* If a temporary buffer is required, at most BUFLEN bytes of BUF will be
   used.  */
extern char *strerror_r(int __errnum, char *__buf, size_t __buflen) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(2)))
__attribute__((__warn_unused_result__)) __attribute__((__access__(__write_only__, 2, 3)));

/* Return a string describing the meaning of tthe error in ERR.  */
extern const char *strerrordesc_np(int __err) __attribute__((__nothrow__, __leaf__));
/* Return a string with the error name in ERR.  */
extern const char *strerrorname_np(int __err) __attribute__((__nothrow__, __leaf__));

/* Translate error number to string according to the locale L.  */
extern char *strerror_l(int __errnum, locale_t __l) __attribute__((__nothrow__, __leaf__));

/* Copyright (C) 1991-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* Copyright (C) 1991-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* Copyright (C) 1989-2022 Free Software Foundation, Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3, or (at your option)
any later version.

GCC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

Under Section 7 of GPL version 3, you are granted additional
permissions described in the GCC Runtime Library Exception, version
3.1, as published by the Free Software Foundation.

You should have received a copy of the GNU General Public License and
a copy of the GCC Runtime Library Exception along with this program;
see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
<http://www.gnu.org/licenses/>.  */

/*
 * ISO C Standard:  7.17  Common definitions  <stddef.h>
 */

/* Any one of these symbols __need_* means that GNU libc
   wants us just to define one data type.  So don't define
   the symbols that indicate this file's entire job has been done.  */
/* This avoids lossage on SunOS but only if stdtypes.h comes first.
   There's no way to win with the other order!  Sun lossage.  */
/* Sequent's header files use _PTRDIFF_T_ in some conflicting way.
   Just ignore it.  */

/* On VxWorks, <type/vxTypesBase.h> may have defined macros like
   _TYPE_size_t which will typedef size_t.  fixincludes patched the
   vxTypesBase.h so that this macro is only defined if _GCC_SIZE_T is
   not defined, and so that defining this macro defines _GCC_SIZE_T.
   If we find that the macros are still defined at this point, we must
   invoke them so that the type is defined as expected.  */
/* In case nobody has defined these types, but we aren't running under
   GCC 2.00, make sure that __PTRDIFF_TYPE__, __SIZE_TYPE__, and
   __WCHAR_TYPE__ have reasonable values.  This can happen if the
   parts of GCC is compiled by an older compiler, that actually
   include gstddef.h, such as collect2.  */

/* Signed type of difference of two pointers.  */

/* Define this type if we are doing the whole job,
   or if we want this type in particular.  */
/* If this symbol has done its job, get rid of it.  */

/* Unsigned type of `sizeof' something.  */

/* Define this type if we are doing the whole job,
   or if we want this type in particular.  */
/* Wide character type.
   Locale-writers should change this as necessary to
   be big enough to hold unique values not between 0 and 127,
   and not (wchar_t) -1, for each defined multibyte character.  */

/* Define this type if we are doing the whole job,
   or if we want this type in particular.  */
/* A null pointer constant.  */
/* Offset of member MEMBER in a struct of type TYPE. */

/* Tell the caller that we provide correct C++ prototypes.  */

/* Compare N bytes of S1 and S2 (same as memcmp).  */
extern int bcmp(const void *__s1, const void *__s2, size_t __n) __attribute__((__nothrow__, __leaf__)) __attribute__((__pure__))
__attribute__((__nonnull__(1, 2)));

/* Copy N bytes of SRC to DEST (like memmove, but args reversed).  */
extern void bcopy(const void *__src, void *__dest, size_t __n) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1, 2)));

/* Set N bytes of S to 0.  */
extern void bzero(void *__s, size_t __n) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));

/* Find the first occurrence of C in S (same as strchr).  */
extern char *index(const char *__s, int __c) __attribute__((__nothrow__, __leaf__)) __attribute__((__pure__)) __attribute__((__nonnull__(1)));

/* Find the last occurrence of C in S (same as strrchr).  */
extern char *rindex(const char *__s, int __c) __attribute__((__nothrow__, __leaf__)) __attribute__((__pure__)) __attribute__((__nonnull__(1)));

/* Return the position of the first bit set in I, or 0 if none are set.
   The least-significant bit is position 1, the most-significant 32.  */
extern int ffs(int __i) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* The following two functions are non-standard but necessary for non-32 bit
   platforms.  */

extern int ffsl(long int __l) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
__extension__ extern int ffsll(long long int __ll) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Compare S1 and S2, ignoring case.  */
extern int strcasecmp(const char *__s1, const char *__s2) __attribute__((__nothrow__, __leaf__)) __attribute__((__pure__)) __attribute__((__nonnull__(1, 2)));

/* Compare no more than N chars of S1 and S2, ignoring case.  */
extern int strncasecmp(const char *__s1, const char *__s2, size_t __n) __attribute__((__nothrow__, __leaf__)) __attribute__((__pure__))
__attribute__((__nonnull__(1, 2)));

/* POSIX.1-2008 extended locale interface (see locale.h).  */
/* Definition of locale_t.
   Copyright (C) 2017-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* Compare S1 and S2, ignoring case, using collation rules from LOC.  */
extern int strcasecmp_l(const char *__s1, const char *__s2, locale_t __loc) __attribute__((__nothrow__, __leaf__)) __attribute__((__pure__))
__attribute__((__nonnull__(1, 2, 3)));

/* Compare no more than N chars of S1 and S2, ignoring case, using
   collation rules from LOC.  */
extern int strncasecmp_l(const char *__s1, const char *__s2, size_t __n, locale_t __loc) __attribute__((__nothrow__, __leaf__)) __attribute__((__pure__))
__attribute__((__nonnull__(1, 2, 4)));

/* Functions with security checks.  */

/* Fortify macros for strings.h functions.
   Copyright (C) 2017-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

extern __inline __attribute__((__always_inline__)) __attribute__((__gnu_inline__)) __attribute__((__artificial__)) void __attribute__((__nothrow__, __leaf__))
bcopy(const void *__src, void *__dest, size_t __len)
{
    (void)__builtin___memmove_chk(__dest, __src, __len, __builtin_dynamic_object_size(__dest, 0));
}

extern __inline __attribute__((__always_inline__)) __attribute__((__gnu_inline__)) __attribute__((__artificial__)) void __attribute__((__nothrow__, __leaf__))
bzero(void *__dest, size_t __len)
{
    (void)__builtin___memset_chk(__dest, '\0', __len, __builtin_dynamic_object_size(__dest, 0));
}

/* Set N bytes of S to 0.  The compiler will not delete a call to this
   function, even if S is dead after the call.  */
extern void explicit_bzero(void *__s, size_t __n) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)))
__attribute__((__access__(__write_only__, 1)));

/* Return the next DELIM-delimited token from *STRINGP,
   terminating it with a '\0', and update *STRINGP to point past it.  */
extern char *strsep(char **__restrict __stringp, const char *__restrict __delim) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1, 2)));

/* Return a string describing the meaning of the signal number in SIG.  */
extern char *strsignal(int __sig) __attribute__((__nothrow__, __leaf__));

/* Return an abbreviation string for the signal number SIG.  */
extern const char *sigabbrev_np(int __sig) __attribute__((__nothrow__, __leaf__));
/* Return a string describing the meaning of the signal number in SIG,
   the result is not translated.  */
extern const char *sigdescr_np(int __sig) __attribute__((__nothrow__, __leaf__));

/* Copy SRC to DEST, returning the address of the terminating '\0' in DEST.  */
extern char *__stpcpy(char *__restrict __dest, const char *__restrict __src) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1, 2)));
extern char *stpcpy(char *__restrict __dest, const char *__restrict __src) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1, 2)));

/* Copy no more than N characters of SRC to DEST, returning the address of
   the last character written into DEST.  */
extern char *__stpncpy(char *__restrict __dest, const char *__restrict __src, size_t __n) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(1, 2)));
extern char *stpncpy(char *__restrict __dest, const char *__restrict __src, size_t __n) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(1, 2)));

/* Copy at most N - 1 characters from SRC to DEST.  */
extern size_t strlcpy(char *__restrict __dest, const char *__restrict __src, size_t __n) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(1, 2))) __attribute__((__access__(__write_only__, 1, 3)));

/* Append SRC to DEST, possibly with truncation to keep the total size
   below N.  */
extern size_t strlcat(char *__restrict __dest, const char *__restrict __src, size_t __n) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(1, 2))) __attribute__((__access__(__read_write__, 1, 3)));

/* Compare S1 and S2 as strings holding name & indices/version numbers.  */
extern int strverscmp(const char *__s1, const char *__s2) __attribute__((__nothrow__, __leaf__)) __attribute__((__pure__)) __attribute__((__nonnull__(1, 2)));

/* Sautee STRING briskly.  */
extern char *strfry(char *__string) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));

/* Frobnicate N bytes of S.  */
extern void *memfrob(void *__s, size_t __n) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)))
__attribute__((__access__(__read_write__, 1, 2)));

/* Return the file name within directory of FILENAME.  We don't
   declare the function if the `basename' macro is available (defined
   in <libgen.h>) which makes the XPG version of this function
   available.  */

extern char *basename(const char *__filename) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));

/* Functions with security checks.  */
/* Copyright (C) 2004-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */
extern __inline __attribute__((__always_inline__)) __attribute__((__gnu_inline__)) __attribute__((__artificial__)) void *__attribute__((__nothrow__, __leaf__))
memcpy(void *__restrict __dest, const void *__restrict __src, size_t __len)

{
    return __builtin___memcpy_chk(__dest, __src, __len, __builtin_dynamic_object_size(__dest, 0));
}

extern __inline __attribute__((__always_inline__)) __attribute__((__gnu_inline__)) __attribute__((__artificial__)) void *__attribute__((__nothrow__, __leaf__))
memmove(void *__dest, const void *__src, size_t __len)
{
    return __builtin___memmove_chk(__dest, __src, __len, __builtin_dynamic_object_size(__dest, 0));
}

extern __inline __attribute__((__always_inline__)) __attribute__((__gnu_inline__)) __attribute__((__artificial__)) void *__attribute__((__nothrow__, __leaf__))
mempcpy(void *__restrict __dest, const void *__restrict __src, size_t __len)

{
    return __builtin___mempcpy_chk(__dest, __src, __len, __builtin_dynamic_object_size(__dest, 0));
}

/* The first two tests here help to catch a somewhat common problem
   where the second and third parameter are transposed.  This is
   especially problematic if the intended fill value is zero.  In this
   case no work is done at all.  We detect these problems by referring
   non-existing functions.  */
extern __inline __attribute__((__always_inline__)) __attribute__((__gnu_inline__)) __attribute__((__artificial__)) void *__attribute__((__nothrow__, __leaf__))
memset(void *__dest, int __ch, size_t __len)
{
    return __builtin___memset_chk(__dest, __ch, __len, __builtin_dynamic_object_size(__dest, 0));
}

/* Fortify macros for strings.h functions.
   Copyright (C) 2017-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

void __explicit_bzero_chk(void *__dest, size_t __len, size_t __destlen) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)))
__attribute__((__access__(__write_only__, 1)));

extern __inline __attribute__((__always_inline__)) __attribute__((__gnu_inline__)) __attribute__((__artificial__)) void __attribute__((__nothrow__, __leaf__))
explicit_bzero(void *__dest, size_t __len)
{
    __explicit_bzero_chk(__dest, __len, __builtin_dynamic_object_size(__dest, 0));
}

extern __inline __attribute__((__always_inline__)) __attribute__((__gnu_inline__)) __attribute__((__artificial__)) char *__attribute__((__nothrow__, __leaf__))
strcpy(char *__restrict __dest, const char *__restrict __src)
{
    return __builtin___strcpy_chk(__dest, __src, __builtin_dynamic_object_size(__dest, 1));
}

extern __inline __attribute__((__always_inline__)) __attribute__((__gnu_inline__)) __attribute__((__artificial__)) char *__attribute__((__nothrow__, __leaf__))
stpcpy(char *__restrict __dest, const char *__restrict __src)
{
    return __builtin___stpcpy_chk(__dest, __src, __builtin_dynamic_object_size(__dest, 1));
}

extern __inline __attribute__((__always_inline__)) __attribute__((__gnu_inline__)) __attribute__((__artificial__)) char *__attribute__((__nothrow__, __leaf__))
strncpy(char *__restrict __dest, const char *__restrict __src, size_t __len)

{
    return __builtin___strncpy_chk(__dest, __src, __len, __builtin_dynamic_object_size(__dest, 1));
}

extern __inline __attribute__((__always_inline__)) __attribute__((__gnu_inline__)) __attribute__((__artificial__)) char *__attribute__((__nothrow__, __leaf__))
stpncpy(char *__dest, const char *__src, size_t __n)
{
    return __builtin___stpncpy_chk(__dest, __src, __n, __builtin_dynamic_object_size(__dest, 1));
}
extern __inline __attribute__((__always_inline__)) __attribute__((__gnu_inline__)) __attribute__((__artificial__)) char *__attribute__((__nothrow__, __leaf__))
strcat(char *__restrict __dest, const char *__restrict __src)
{
    return __builtin___strcat_chk(__dest, __src, __builtin_dynamic_object_size(__dest, 1));
}

extern __inline __attribute__((__always_inline__)) __attribute__((__gnu_inline__)) __attribute__((__artificial__)) char *__attribute__((__nothrow__, __leaf__))
strncat(char *__restrict __dest, const char *__restrict __src, size_t __len)

{
    return __builtin___strncat_chk(__dest, __src, __len, __builtin_dynamic_object_size(__dest, 1));
}

extern size_t __strlcpy_chk(char *__dest, const char *__src, size_t __n, size_t __destlen) __attribute__((__nothrow__, __leaf__));
extern size_t __strlcpy_alias(char *__dest, const char *__src, size_t __n) __asm__(""
                                                                                   "strlcpy") __attribute__((__nothrow__, __leaf__))

;

extern __inline __attribute__((__always_inline__)) __attribute__((__gnu_inline__)) __attribute__((__artificial__)) size_t __attribute__((__nothrow__, __leaf__))
strlcpy(char *__restrict __dest, const char *__restrict __src, size_t __n)

{
    if (__builtin_dynamic_object_size(__dest, 1) != (size_t)-1 &&
        (!__builtin_constant_p(__n > __builtin_dynamic_object_size(__dest, 1)) || __n > __builtin_dynamic_object_size(__dest, 1)))
        return __strlcpy_chk(__dest, __src, __n, __builtin_dynamic_object_size(__dest, 1));
    return __strlcpy_alias(__dest, __src, __n);
}

extern size_t __strlcat_chk(char *__dest, const char *__src, size_t __n, size_t __destlen) __attribute__((__nothrow__, __leaf__));
extern size_t __strlcat_alias(char *__dest, const char *__src, size_t __n) __asm__(""
                                                                                   "strlcat") __attribute__((__nothrow__, __leaf__))

;

extern __inline __attribute__((__always_inline__)) __attribute__((__gnu_inline__)) __attribute__((__artificial__)) size_t __attribute__((__nothrow__, __leaf__))
strlcat(char *__restrict __dest, const char *__restrict __src, size_t __n)

{
    if (__builtin_dynamic_object_size(__dest, 1) != (size_t)-1 &&
        (!__builtin_constant_p(__n > __builtin_dynamic_object_size(__dest, 1)) || __n > __builtin_dynamic_object_size(__dest, 1)))
        return __strlcat_chk(__dest, __src, __n, __builtin_dynamic_object_size(__dest, 1));
    return __strlcat_alias(__dest, __src, __n);
}

/* Copyright (C) 1991-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/*
 *	ISO C99 Standard: 7.23 Date and time	<time.h>
 */

/* Copyright (C) 1991-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* Copyright (C) 1989-2022 Free Software Foundation, Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3, or (at your option)
any later version.

GCC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

Under Section 7 of GPL version 3, you are granted additional
permissions described in the GCC Runtime Library Exception, version
3.1, as published by the Free Software Foundation.

You should have received a copy of the GNU General Public License and
a copy of the GCC Runtime Library Exception along with this program;
see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
<http://www.gnu.org/licenses/>.  */

/*
 * ISO C Standard:  7.17  Common definitions  <stddef.h>
 */

/* Any one of these symbols __need_* means that GNU libc
   wants us just to define one data type.  So don't define
   the symbols that indicate this file's entire job has been done.  */
/* This avoids lossage on SunOS but only if stdtypes.h comes first.
   There's no way to win with the other order!  Sun lossage.  */
/* Sequent's header files use _PTRDIFF_T_ in some conflicting way.
   Just ignore it.  */

/* On VxWorks, <type/vxTypesBase.h> may have defined macros like
   _TYPE_size_t which will typedef size_t.  fixincludes patched the
   vxTypesBase.h so that this macro is only defined if _GCC_SIZE_T is
   not defined, and so that defining this macro defines _GCC_SIZE_T.
   If we find that the macros are still defined at this point, we must
   invoke them so that the type is defined as expected.  */
/* In case nobody has defined these types, but we aren't running under
   GCC 2.00, make sure that __PTRDIFF_TYPE__, __SIZE_TYPE__, and
   __WCHAR_TYPE__ have reasonable values.  This can happen if the
   parts of GCC is compiled by an older compiler, that actually
   include gstddef.h, such as collect2.  */

/* Signed type of difference of two pointers.  */

/* Define this type if we are doing the whole job,
   or if we want this type in particular.  */
/* If this symbol has done its job, get rid of it.  */

/* Unsigned type of `sizeof' something.  */

/* Define this type if we are doing the whole job,
   or if we want this type in particular.  */
/* Wide character type.
   Locale-writers should change this as necessary to
   be big enough to hold unique values not between 0 and 127,
   and not (wchar_t) -1, for each defined multibyte character.  */

/* Define this type if we are doing the whole job,
   or if we want this type in particular.  */
/* A null pointer constant.  */
/* Offset of member MEMBER in a struct of type TYPE. */

/* This defines CLOCKS_PER_SEC, which is the number of processor clock
   ticks per second, and possibly a number of other constants.   */
/* System-dependent timing definitions.  Linux version.
   Copyright (C) 1996-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/*
 * Never include this file directly; use <time.h> instead.
 */

/* bits/types.h -- definitions of __*_t types underlying *_t types.
   Copyright (C) 2002-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/*
 * Never include this file directly; use <sys/types.h> instead.
 */

/* ISO/IEC 9899:1999 7.23.1: Components of time
   The macro `CLOCKS_PER_SEC' is an expression with type `clock_t' that is
   the number per second of the value returned by the `clock' function.  */
/* CAE XSH, Issue 4, Version 2: <time.h>
   The value of CLOCKS_PER_SEC is required to be 1 million on all
   XSI-conformant systems. */
/* Identifier for system-wide realtime clock.  */

/* Monotonic system-wide clock.  */

/* High-resolution timer from the CPU.  */

/* Thread-specific CPU-time clock.  */

/* Monotonic system-wide clock, not adjusted for frequency scaling.  */

/* Identifier for system-wide realtime clock, updated only on ticks.  */

/* Monotonic system-wide clock, updated only on ticks.  */

/* Monotonic system-wide clock that includes time spent in suspension.  */

/* Like CLOCK_REALTIME but also wakes suspended system.  */

/* Like CLOCK_BOOTTIME but also wakes suspended system.  */

/* Like CLOCK_REALTIME but in International Atomic Time.  */

/* Flag to indicate time is absolute.  */

/* Copyright (C) 1995-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* bits/types.h -- definitions of __*_t types underlying *_t types.
   Copyright (C) 2002-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/*
 * Never include this file directly; use <sys/types.h> instead.
 */

/* These definitions from linux/timex.h as of 3.18.  */

struct timex {
    unsigned int modes;          /* mode selector */
    __syscall_slong_t offset;    /* time offset (usec) */
    __syscall_slong_t freq;      /* frequency offset (scaled ppm) */
    __syscall_slong_t maxerror;  /* maximum error (usec) */
    __syscall_slong_t esterror;  /* estimated error (usec) */
    int status;                  /* clock command/status */
    __syscall_slong_t constant;  /* pll time constant */
    __syscall_slong_t precision; /* clock precision (usec) (ro) */
    __syscall_slong_t tolerance; /* clock frequency tolerance (ppm) (ro) */
    struct timeval time;         /* (read only, except for ADJ_SETOFFSET) */
    __syscall_slong_t tick;      /* (modified) usecs between clock ticks */
    __syscall_slong_t ppsfreq;   /* pps frequency (scaled ppm) (ro) */
    __syscall_slong_t jitter;    /* pps jitter (us) (ro) */
    int shift;                   /* interval duration (s) (shift) (ro) */
    __syscall_slong_t stabil;    /* pps stability (scaled ppm) (ro) */
    __syscall_slong_t jitcnt;    /* jitter limit exceeded (ro) */
    __syscall_slong_t calcnt;    /* calibration intervals (ro) */
    __syscall_slong_t errcnt;    /* calibration errors (ro) */
    __syscall_slong_t stbcnt;    /* stability limit exceeded (ro) */

    int tai; /* TAI offset (ro) */

    /* ??? */
    int : 32;
    int : 32;
    int : 32;
    int : 32;
    int : 32;
    int : 32;
    int : 32;
    int : 32;
    int : 32;
    int : 32;
    int : 32;
};

/* Mode codes (timex.mode) */
/* xntp 3.4 compatibility names */
/* Status codes (timex.status) */
/* Read-only bits */

/* Tune a POSIX clock.  */
extern int clock_adjtime(__clockid_t __clock_id, struct timex *__utx) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(2)));

/* Many of the typedefs and structs whose official home is this header
   may also need to be defined by other headers.  */

/* bits/types.h -- definitions of __*_t types underlying *_t types.
   Copyright (C) 2002-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/*
 * Never include this file directly; use <sys/types.h> instead.
 */

/* ISO C `broken-down time' structure.  */
struct tm {
    int tm_sec;   /* Seconds.	[0-60] (1 leap second) */
    int tm_min;   /* Minutes.	[0-59] */
    int tm_hour;  /* Hours.	[0-23] */
    int tm_mday;  /* Day.		[1-31] */
    int tm_mon;   /* Month.	[0-11] */
    int tm_year;  /* Year	- 1900.  */
    int tm_wday;  /* Day of week.	[0-6] */
    int tm_yday;  /* Days in year.[0-365]	*/
    int tm_isdst; /* DST.		[-1/0/1]*/

    long int tm_gmtoff;  /* Seconds east of UTC.  */
    const char *tm_zone; /* Timezone abbreviation.  */
};

/* NB: Include guard matches what <linux/time.h> uses.  */

/* bits/types.h -- definitions of __*_t types underlying *_t types.
   Copyright (C) 2002-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/*
 * Never include this file directly; use <sys/types.h> instead.
 */
/* NB: Include guard matches what <linux/time.h> uses.  */

/* POSIX.1b structure for timer start values and intervals.  */
struct itimerspec {
    struct timespec it_interval;
    struct timespec it_value;
};
struct sigevent;
/* Definition of locale_t.
   Copyright (C) 2017-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* Time base values for timespec_get.  */

/* Time used by the program so far (user time + system time).
   The result / CLOCKS_PER_SEC is program time in seconds.  */
extern clock_t clock(void) __attribute__((__nothrow__, __leaf__));

/* Return the current time and put it in *TIMER if TIMER is not NULL.  */
extern time_t time(time_t *__timer) __attribute__((__nothrow__, __leaf__));

/* Return the difference between TIME1 and TIME0.  */
extern double difftime(time_t __time1, time_t __time0) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return the `time_t' representation of TP and normalize TP.  */
extern time_t mktime(struct tm *__tp) __attribute__((__nothrow__, __leaf__));
/* Format TP into S according to FORMAT.
   Write no more than MAXSIZE characters and return the number
   of characters written, or 0 if it would exceed MAXSIZE.  */
extern size_t strftime(char *__restrict __s, size_t __maxsize, const char *__restrict __format, const struct tm *__restrict __tp)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1, 3, 4)));

/* Parse S according to FORMAT and store binary time information in TP.
   The return value is a pointer to the first unparsed character in S.  */
extern char *strptime(const char *__restrict __s, const char *__restrict __fmt, struct tm *__tp) __attribute__((__nothrow__, __leaf__));

/* Similar to the two functions above but take the information from
   the provided locale and not the global locale.  */

extern size_t strftime_l(char *__restrict __s, size_t __maxsize, const char *__restrict __format, const struct tm *__restrict __tp, locale_t __loc)
    __attribute__((__nothrow__, __leaf__));

extern char *strptime_l(const char *__restrict __s, const char *__restrict __fmt, struct tm *__tp, locale_t __loc) __attribute__((__nothrow__, __leaf__));

/* Return the `struct tm' representation of *TIMER
   in Universal Coordinated Time (aka Greenwich Mean Time).  */
extern struct tm *gmtime(const time_t *__timer) __attribute__((__nothrow__, __leaf__));

/* Return the `struct tm' representation
   of *TIMER in the local timezone.  */
extern struct tm *localtime(const time_t *__timer) __attribute__((__nothrow__, __leaf__));
/* Return the `struct tm' representation of *TIMER in UTC,
   using *TP to store the result.  */
extern struct tm *gmtime_r(const time_t *__restrict __timer, struct tm *__restrict __tp) __attribute__((__nothrow__, __leaf__));

/* Return the `struct tm' representation of *TIMER in local time,
   using *TP to store the result.  */
extern struct tm *localtime_r(const time_t *__restrict __timer, struct tm *__restrict __tp) __attribute__((__nothrow__, __leaf__));
/* Return a string of the form "Day Mon dd hh:mm:ss yyyy\n"
   that is the representation of TP in this format.  */
extern char *asctime(const struct tm *__tp) __attribute__((__nothrow__, __leaf__));

/* Equivalent to `asctime (localtime (timer))'.  */

extern char *ctime(const time_t *__timer) __attribute__((__nothrow__, __leaf__));
/* Reentrant versions of the above functions.  */

/* Return in BUF a string of the form "Day Mon dd hh:mm:ss yyyy\n"
   that is the representation of TP in this format.  */
extern char *asctime_r(const struct tm *__restrict __tp, char *__restrict __buf) __attribute__((__nothrow__, __leaf__));

/* Equivalent to `asctime_r (localtime_r (timer, *TMP*), buf)'.  */

extern char *ctime_r(const time_t *__restrict __timer, char *__restrict __buf) __attribute__((__nothrow__, __leaf__));
/* Defined in localtime.c.  */
extern char *__tzname[2];   /* Current time zone abbreviations.  */
extern int __daylight;      /* If daylight-saving time is ever in use.  */
extern long int __timezone; /* Seconds west of UTC.  */

/* Same as above.  */
extern char *tzname[2];

/* Set time conversion information from the TZ environment variable.
   If TZ is not defined, a locale-dependent default is used.  */
extern void tzset(void) __attribute__((__nothrow__, __leaf__));

extern int daylight;
extern long int timezone;

/* Nonzero if YEAR is a leap year (every 4 years,
   except every 100th isn't, and every 400th is).  */

/* Like `mktime', but for TP represents Universal Time, not local time.  */
extern time_t timegm(struct tm *__tp) __attribute__((__nothrow__, __leaf__));
/* Miscellaneous functions many Unices inherited from the public domain
   localtime package.  These are included only for compatibility.  */

/* Another name for `mktime'.  */
extern time_t timelocal(struct tm *__tp) __attribute__((__nothrow__, __leaf__));

/* Return the number of days in YEAR.  */
extern int dysize(int __year) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Pause execution for a number of nanoseconds.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
extern int nanosleep(const struct timespec *__requested_time, struct timespec *__remaining);

/* Get resolution of clock CLOCK_ID.  */
extern int clock_getres(clockid_t __clock_id, struct timespec *__res) __attribute__((__nothrow__, __leaf__));

/* Get current value of clock CLOCK_ID and store it in TP.  */
extern int clock_gettime(clockid_t __clock_id, struct timespec *__tp) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(2)));

/* Set clock CLOCK_ID to value TP.  */
extern int clock_settime(clockid_t __clock_id, const struct timespec *__tp) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(2)));
/* High-resolution sleep with the specified clock.

   This function is a cancellation point and therefore not marked with
   __THROW.  */

extern int clock_nanosleep(clockid_t __clock_id, int __flags, const struct timespec *__req, struct timespec *__rem);
/* Return clock ID for CPU-time clock.  */
extern int clock_getcpuclockid(pid_t __pid, clockid_t *__clock_id) __attribute__((__nothrow__, __leaf__));

/* Create new per-process timer using CLOCK_ID.  */
extern int timer_create(clockid_t __clock_id, struct sigevent *__restrict __evp, timer_t *__restrict __timerid) __attribute__((__nothrow__, __leaf__));

/* Delete timer TIMERID.  */
extern int timer_delete(timer_t __timerid) __attribute__((__nothrow__, __leaf__));

/* Set timer TIMERID to VALUE, returning old value in OVALUE.  */

extern int timer_settime(timer_t __timerid, int __flags, const struct itimerspec *__restrict __value, struct itimerspec *__restrict __ovalue)
    __attribute__((__nothrow__, __leaf__));

/* Get current value of timer TIMERID and store it in VALUE.  */
extern int timer_gettime(timer_t __timerid, struct itimerspec *__value) __attribute__((__nothrow__, __leaf__));
/* Get expiration overrun for timer TIMERID.  */
extern int timer_getoverrun(timer_t __timerid) __attribute__((__nothrow__, __leaf__));

/* Set TS to calendar time based in time base BASE.  */
extern int timespec_get(struct timespec *__ts, int __base) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));
/* Set TS to resolution of time base BASE.  */
extern int timespec_getres(struct timespec *__ts, int __base) __attribute__((__nothrow__, __leaf__));
/* Set to one of the following values to indicate an error.
     1  the DATEMSK environment variable is null or undefined,
     2  the template file cannot be opened for reading,
     3  failed to get file status information,
     4  the template file is not a regular file,
     5  an error is encountered while reading the template file,
     6  memory allication failed (not enough memory available),
     7  there is no line in the template that matches the input,
     8  invalid input specification Example: February 31 or a time is
    specified that can not be represented in a time_t (representing
    the time in seconds since 00:00:00 UTC, January 1, 1970) */
extern int getdate_err;

/* Parse the given string as a date specification and return a value
   representing the value.  The templates from the file identified by
   the environment variable DATEMSK are used.  In case of an error
   `getdate_err' is set.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern struct tm *getdate(const char *__string);

/* Since `getdate' is not reentrant because of the use of `getdate_err'
   and the static buffer to return the result in, we provide a thread-safe
   variant.  The functionality is the same.  The result is returned in
   the buffer pointed to by RESBUFP and in case of an error the return
   value is != 0 with the same values as given above for `getdate_err'.

   This function is not part of POSIX and therefore no official
   cancellation point.  But due to similarity with an POSIX interface
   or due to the implementation it is a cancellation point and
   therefore not marked with __THROW.  */
extern int getdate_r(const char *__restrict __string, struct tm *__restrict __resbufp);

/* Copyright (C) 1991-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/*
 *	POSIX Standard: 2.10 Symbolic Constants		<unistd.h>
 */

/* Copyright (C) 1991-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* These may be used to determine what facilities are present at compile time.
   Their values can be obtained at run time from `sysconf'.  */

/* POSIX Standard approved as ISO/IEC 9945-1 as of September 2008.  */
/* These are not #ifdef __USE_POSIX2 because they are
   in the theoretically application-owned namespace.  */

/* The utilities on GNU systems also correspond to this version.  */
/* The utilities on GNU systems also correspond to this version.  */

/* This symbol was required until the 2001 edition of POSIX.  */

/* If defined, the implementation supports the
   C Language Bindings Option.  */

/* If defined, the implementation supports the
   C Language Development Utilities Option.  */

/* If defined, the implementation supports the
   Software Development Utilities Option.  */

/* If defined, the implementation supports the
   creation of locales with the localedef utility.  */

/* X/Open version number to which the library conforms.  It is selectable.  */
/* Commands and utilities from XPG4 are available.  */

/* We are compatible with the old published standards as well.  */

/* The X/Open Unix extensions are available.  */

/* The enhanced internationalization capabilities according to XPG4.2
   are present.  */

/* The legacy interfaces are also available.  */

/* Get values of POSIX options:

   If these symbols are defined, the corresponding features are
   always available.  If not, they may be available sometimes.
   The current values can be obtained with `sysconf'.

   _POSIX_JOB_CONTROL		Job control is supported.
   _POSIX_SAVED_IDS		Processes have a saved set-user-ID
                and a saved set-group-ID.
   _POSIX_REALTIME_SIGNALS	Real-time, queued signals are supported.
   _POSIX_PRIORITY_SCHEDULING	Priority scheduling is supported.
   _POSIX_TIMERS		POSIX.4 clocks and timers are supported.
   _POSIX_ASYNCHRONOUS_IO	Asynchronous I/O is supported.
   _POSIX_PRIORITIZED_IO	Prioritized asynchronous I/O is supported.
   _POSIX_SYNCHRONIZED_IO	Synchronizing file data is supported.
   _POSIX_FSYNC			The fsync function is present.
   _POSIX_MAPPED_FILES		Mapping of files to memory is supported.
   _POSIX_MEMLOCK		Locking of all memory is supported.
   _POSIX_MEMLOCK_RANGE		Locking of ranges of memory is supported.
   _POSIX_MEMORY_PROTECTION	Setting of memory protections is supported.
   _POSIX_MESSAGE_PASSING	POSIX.4 message queues are supported.
   _POSIX_SEMAPHORES		POSIX.4 counting semaphores are supported.
   _POSIX_SHARED_MEMORY_OBJECTS	POSIX.4 shared memory objects are supported.
   _POSIX_THREADS		POSIX.1c pthreads are supported.
   _POSIX_THREAD_ATTR_STACKADDR	Thread stack address attribute option supported.
   _POSIX_THREAD_ATTR_STACKSIZE	Thread stack size attribute option supported.
   _POSIX_THREAD_SAFE_FUNCTIONS	Thread-safe functions are supported.
   _POSIX_THREAD_PRIORITY_SCHEDULING
                POSIX.1c thread execution scheduling supported.
   _POSIX_THREAD_PRIO_INHERIT	Thread priority inheritance option supported.
   _POSIX_THREAD_PRIO_PROTECT	Thread priority protection option supported.
   _POSIX_THREAD_PROCESS_SHARED	Process-shared synchronization supported.
   _POSIX_PII			Protocol-independent interfaces are supported.
   _POSIX_PII_XTI		XTI protocol-indep. interfaces are supported.
   _POSIX_PII_SOCKET		Socket protocol-indep. interfaces are supported.
   _POSIX_PII_INTERNET		Internet family of protocols supported.
   _POSIX_PII_INTERNET_STREAM	Connection-mode Internet protocol supported.
   _POSIX_PII_INTERNET_DGRAM	Connectionless Internet protocol supported.
   _POSIX_PII_OSI		ISO/OSI family of protocols supported.
   _POSIX_PII_OSI_COTS		Connection-mode ISO/OSI service supported.
   _POSIX_PII_OSI_CLTS		Connectionless ISO/OSI service supported.
   _POSIX_POLL			Implementation supports `poll' function.
   _POSIX_SELECT		Implementation supports `select' and `pselect'.

   _XOPEN_REALTIME		X/Open realtime support is available.
   _XOPEN_REALTIME_THREADS	X/Open realtime thread support is available.
   _XOPEN_SHM			Shared memory interface according to XPG4.2.

   _XBS5_ILP32_OFF32		Implementation provides environment with 32-bit
                int, long, pointer, and off_t types.
   _XBS5_ILP32_OFFBIG		Implementation provides environment with 32-bit
                int, long, and pointer and off_t with at least
                64 bits.
   _XBS5_LP64_OFF64		Implementation provides environment with 32-bit
                int, and 64-bit long, pointer, and off_t types.
   _XBS5_LPBIG_OFFBIG		Implementation provides environment with at
                least 32 bits int and long, pointer, and off_t
                with at least 64 bits.

   If any of these symbols is defined as -1, the corresponding option is not
   true for any file.  If any is defined as other than -1, the corresponding
   option is true for all files.  If a symbol is not defined at all, the value
   for a specific file can be obtained from `pathconf' and `fpathconf'.

   _POSIX_CHOWN_RESTRICTED	Only the super user can use `chown' to change
                the owner of a file.  `chown' can only be used
                to change the group ID of a file to a group of
                which the calling process is a member.
   _POSIX_NO_TRUNC		Pathname components longer than
                NAME_MAX generate an error.
   _POSIX_VDISABLE		If defined, if the value of an element of the
                `c_cc' member of `struct termios' is
                _POSIX_VDISABLE, no character will have the
                effect associated with that element.
   _POSIX_SYNC_IO		Synchronous I/O may be performed.
   _POSIX_ASYNC_IO		Asynchronous I/O may be performed.
   _POSIX_PRIO_IO		Prioritized Asynchronous I/O may be performed.

   Support for the Large File Support interface is not generally available.
   If it is available the following constants are defined to one.
   _LFS64_LARGEFILE		Low-level I/O supports large files.
   _LFS64_STDIO			Standard I/O supports large files.
   */

/* Define POSIX options for Linux.
   Copyright (C) 1996-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <https://www.gnu.org/licenses/>.  */

/* Job control is supported.  */

/* Processes have a saved set-user-ID and a saved set-group-ID.  */

/* Priority scheduling is not supported with the correct semantics,
   but GNU/Linux applications expect that the corresponding interfaces
   are available, even though the semantics do not meet the POSIX
   requirements.  See glibc bug 14829.  */

/* Synchronizing file data is supported.  */

/* The fsync function is present.  */

/* Mapping of files to memory is supported.  */

/* Locking of all memory is supported.  */

/* Locking of ranges of memory is supported.  */

/* Setting of memory protections is supported.  */

/* Some filesystems allow all users to change file ownership.  */

/* `c_cc' member of 'struct termios' structure can be disabled by
   using the value _POSIX_VDISABLE.  */

/* Filenames are not silently truncated.  */

/* X/Open realtime support is available.  */

/* X/Open thread realtime support is available.  */

/* XPG4.2 shared memory is supported.  */

/* Tell we have POSIX threads.  */

/* We have the reentrant functions described in POSIX.  */

/* We provide priority scheduling for threads.  */

/* We support user-defined stack sizes.  */

/* We support user-defined stacks.  */

/* We support priority inheritance.  */

/* We support priority protection, though only for non-robust
   mutexes.  */

/* We support priority inheritance for robust mutexes.  */

/* We do not support priority protection for robust mutexes.  */

/* We support POSIX.1b semaphores.  */

/* Real-time signals are supported.  */

/* We support asynchronous I/O.  */

/* Alternative name for Unix98.  */

/* Support for prioritization is also available.  */

/* The LFS support in asynchronous I/O is also available.  */

/* The rest of the LFS is also available.  */

/* POSIX shared memory objects are implemented.  */

/* CPU-time clocks support needs to be checked at runtime.  */

/* Clock support in threads must be also checked at runtime.  */

/* GNU libc provides regular expression handling.  */

/* Reader/Writer locks are available.  */

/* We have a POSIX shell.  */

/* We support the Timeouts option.  */

/* We support spinlocks.  */

/* The `spawn' function family is supported.  */

/* We have POSIX timers.  */

/* The barrier functions are available.  */

/* POSIX message queues are available.  */

/* Thread process-shared synchronization is supported.  */

/* The monotonic clock might be available.  */

/* The clock selection interfaces are available.  */

/* Advisory information interfaces are available.  */

/* IPv6 support is available.  */

/* Raw socket support is available.  */

/* We have at least one terminal.  */

/* Neither process nor thread sporadic server interfaces is available.  */

/* trace.h is not available.  */

/* Typed memory objects are not available.  */

/* Get the environment definitions from Unix98.  */

/* Copyright (C) 1999-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* Determine the wordsize from the preprocessor defines.  */
/* Both x86-64 and x32 use the 64-bit system call interface.  */

/* This header should define the following symbols under the described
   situations.  A value `1' means that the model is always supported,
   `-1' means it is never supported.  Undefined means it cannot be
   statically decided.

   _POSIX_V7_ILP32_OFF32   32bit int, long, pointers, and off_t type
   _POSIX_V7_ILP32_OFFBIG  32bit int, long, and pointers and larger off_t type

   _POSIX_V7_LP64_OFF32	   64bit long and pointers and 32bit off_t type
   _POSIX_V7_LPBIG_OFFBIG  64bit long and pointers and large off_t type

   The macros _POSIX_V6_ILP32_OFF32, _POSIX_V6_ILP32_OFFBIG,
   _POSIX_V6_LP64_OFF32, _POSIX_V6_LPBIG_OFFBIG, _XBS5_ILP32_OFF32,
   _XBS5_ILP32_OFFBIG, _XBS5_LP64_OFF32, and _XBS5_LPBIG_OFFBIG were
   used in previous versions of the Unix standard and are available
   only for compatibility.
*/

/* Environments with 32-bit wide pointers are optionally provided.
   Therefore following macros aren't defined:
   # undef _POSIX_V7_ILP32_OFF32
   # undef _POSIX_V7_ILP32_OFFBIG
   # undef _POSIX_V6_ILP32_OFF32
   # undef _POSIX_V6_ILP32_OFFBIG
   # undef _XBS5_ILP32_OFF32
   # undef _XBS5_ILP32_OFFBIG
   and users need to check at runtime.  */

/* We also have no use (for now) for an environment with bigger pointers
   and offsets.  */

/* By default we have 64-bit wide `long int', pointers and `off_t'.  */

/* Standard file descriptors.  */

/* All functions that are not declared anywhere else.  */

/* bits/types.h -- definitions of __*_t types underlying *_t types.
   Copyright (C) 2002-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/*
 * Never include this file directly; use <sys/types.h> instead.
 */
/* Copyright (C) 1989-2022 Free Software Foundation, Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3, or (at your option)
any later version.

GCC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

Under Section 7 of GPL version 3, you are granted additional
permissions described in the GCC Runtime Library Exception, version
3.1, as published by the Free Software Foundation.

You should have received a copy of the GNU General Public License and
a copy of the GCC Runtime Library Exception along with this program;
see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
<http://www.gnu.org/licenses/>.  */

/*
 * ISO C Standard:  7.17  Common definitions  <stddef.h>
 */

/* Any one of these symbols __need_* means that GNU libc
   wants us just to define one data type.  So don't define
   the symbols that indicate this file's entire job has been done.  */
/* This avoids lossage on SunOS but only if stdtypes.h comes first.
   There's no way to win with the other order!  Sun lossage.  */
/* Sequent's header files use _PTRDIFF_T_ in some conflicting way.
   Just ignore it.  */

/* On VxWorks, <type/vxTypesBase.h> may have defined macros like
   _TYPE_size_t which will typedef size_t.  fixincludes patched the
   vxTypesBase.h so that this macro is only defined if _GCC_SIZE_T is
   not defined, and so that defining this macro defines _GCC_SIZE_T.
   If we find that the macros are still defined at this point, we must
   invoke them so that the type is defined as expected.  */
/* In case nobody has defined these types, but we aren't running under
   GCC 2.00, make sure that __PTRDIFF_TYPE__, __SIZE_TYPE__, and
   __WCHAR_TYPE__ have reasonable values.  This can happen if the
   parts of GCC is compiled by an older compiler, that actually
   include gstddef.h, such as collect2.  */

/* Signed type of difference of two pointers.  */

/* Define this type if we are doing the whole job,
   or if we want this type in particular.  */
/* If this symbol has done its job, get rid of it.  */

/* Unsigned type of `sizeof' something.  */

/* Define this type if we are doing the whole job,
   or if we want this type in particular.  */
/* Wide character type.
   Locale-writers should change this as necessary to
   be big enough to hold unique values not between 0 and 127,
   and not (wchar_t) -1, for each defined multibyte character.  */

/* Define this type if we are doing the whole job,
   or if we want this type in particular.  */
/* A null pointer constant.  */
/* Offset of member MEMBER in a struct of type TYPE. */

/* The Single Unix specification says that some more types are
   available here.  */
typedef __socklen_t socklen_t;

/* Values for the second argument to access.
   These may be OR'd together.  */

/* Test for access to NAME using the real UID and real GID.  */
extern int access(const char *__name, int __type) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));

/* Test for access to NAME using the effective UID and GID
   (as normal file operations use).  */
extern int euidaccess(const char *__name, int __type) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));

/* An alias for `euidaccess', used by some other systems.  */
extern int eaccess(const char *__name, int __type) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));

/* Execute program relative to a directory file descriptor.  */
extern int execveat(int __fd, const char *__path, char *const __argv[], char *const __envp[], int __flags) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(2, 3)));

/* Test for access to FILE relative to the directory FD is open on.
   If AT_EACCESS is set in FLAG, then use effective IDs like `eaccess',
   otherwise use real IDs like `access'.  */
extern int faccessat(int __fd, const char *__file, int __type, int __flag) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(2)))
__attribute__((__warn_unused_result__));

/* Values for the WHENCE argument to lseek.  */
/* Old BSD names for the same constants; just for compatibility.  */

/* Move FD's file position to OFFSET bytes from the
   beginning of the file (if WHENCE is SEEK_SET),
   the current position (if WHENCE is SEEK_CUR),
   or the end of the file (if WHENCE is SEEK_END).
   Return the new file position.  */

extern __off_t lseek(int __fd, __off_t __offset, int __whence) __attribute__((__nothrow__, __leaf__));
extern __off64_t lseek64(int __fd, __off64_t __offset, int __whence) __attribute__((__nothrow__, __leaf__));

/* Close the file descriptor FD.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
extern int close(int __fd);

/* Close all open file descriptors greater than or equal to LOWFD.
   Negative LOWFD is clamped to 0.  */
extern void closefrom(int __lowfd) __attribute__((__nothrow__, __leaf__));

/* Read NBYTES into BUF from FD.  Return the
   number read, -1 for errors or 0 for EOF.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
extern ssize_t read(int __fd, void *__buf, size_t __nbytes) __attribute__((__warn_unused_result__)) __attribute__((__access__(__write_only__, 2)));

/* Write N bytes of BUF to FD.  Return the number written, or -1.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
extern ssize_t write(int __fd, const void *__buf, size_t __n) __attribute__((__warn_unused_result__)) __attribute__((__access__(__read_only__, 2, 3)));

/* Read NBYTES into BUF from FD at the given position OFFSET without
   changing the file pointer.  Return the number read, -1 for errors
   or 0 for EOF.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
extern ssize_t pread(int __fd, void *__buf, size_t __nbytes, __off_t __offset) __attribute__((__warn_unused_result__))
__attribute__((__access__(__write_only__, 2)));

/* Write N bytes of BUF to FD at the given position OFFSET without
   changing the file pointer.  Return the number written, or -1.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
extern ssize_t pwrite(int __fd, const void *__buf, size_t __n, __off_t __offset) __attribute__((__warn_unused_result__))
__attribute__((__access__(__read_only__, 2, 3)));
/* Read NBYTES into BUF from FD at the given position OFFSET without
   changing the file pointer.  Return the number read, -1 for errors
   or 0 for EOF.  */
extern ssize_t pread64(int __fd, void *__buf, size_t __nbytes, __off64_t __offset) __attribute__((__warn_unused_result__))
__attribute__((__access__(__write_only__, 2)));
/* Write N bytes of BUF to FD at the given position OFFSET without
   changing the file pointer.  Return the number written, or -1.  */
extern ssize_t pwrite64(int __fd, const void *__buf, size_t __n, __off64_t __offset) __attribute__((__warn_unused_result__))
__attribute__((__access__(__read_only__, 2, 3)));

/* Create a one-way communication channel (pipe).
   If successful, two file descriptors are stored in PIPEDES;
   bytes written on PIPEDES[1] can be read from PIPEDES[0].
   Returns 0 if successful, -1 if not.  */
extern int pipe(int __pipedes[2]) __attribute__((__nothrow__, __leaf__)) __attribute__((__warn_unused_result__));

/* Same as pipe but apply flags passed in FLAGS to the new file
   descriptors.  */
extern int pipe2(int __pipedes[2], int __flags) __attribute__((__nothrow__, __leaf__)) __attribute__((__warn_unused_result__));

/* Schedule an alarm.  In SECONDS seconds, the process will get a SIGALRM.
   If SECONDS is zero, any currently scheduled alarm will be cancelled.
   The function returns the number of seconds remaining until the last
   alarm scheduled would have signaled, or zero if there wasn't one.
   There is no return value to indicate an error, but you can set `errno'
   to 0 and check its value after calling `alarm', and this might tell you.
   The signal may come late due to processor scheduling.  */
extern unsigned int alarm(unsigned int __seconds) __attribute__((__nothrow__, __leaf__));

/* Make the process sleep for SECONDS seconds, or until a signal arrives
   and is not ignored.  The function returns the number of seconds less
   than SECONDS which it actually slept (thus zero if it slept the full time).
   If a signal handler does a `longjmp' or modifies the handling of the
   SIGALRM signal while inside `sleep' call, the handling of the SIGALRM
   signal afterwards is undefined.  There is no return value to indicate
   error, but if `sleep' returns SECONDS, it probably didn't work.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
extern unsigned int sleep(unsigned int __seconds);

/* Set an alarm to go off (generating a SIGALRM signal) in VALUE
   microseconds.  If INTERVAL is nonzero, when the alarm goes off, the
   timer is reset to go off every INTERVAL microseconds thereafter.
   Returns the number of microseconds remaining before the alarm.  */
extern __useconds_t ualarm(__useconds_t __value, __useconds_t __interval) __attribute__((__nothrow__, __leaf__));

/* Sleep USECONDS microseconds, or until a signal arrives that is not blocked
   or ignored.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
extern int usleep(__useconds_t __useconds);

/* Suspend the process until a signal arrives.
   This always returns -1 and sets `errno' to EINTR.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
extern int pause(void);

/* Change the owner and group of FILE.  */
extern int chown(const char *__file, __uid_t __owner, __gid_t __group) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)))
__attribute__((__warn_unused_result__));

/* Change the owner and group of the file that FD is open on.  */
extern int fchown(int __fd, __uid_t __owner, __gid_t __group) __attribute__((__nothrow__, __leaf__)) __attribute__((__warn_unused_result__));

/* Change owner and group of FILE, if it is a symbolic
   link the ownership of the symbolic link is changed.  */
extern int lchown(const char *__file, __uid_t __owner, __gid_t __group) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)))
__attribute__((__warn_unused_result__));

/* Change the owner and group of FILE relative to the directory FD is open
   on.  */
extern int fchownat(int __fd, const char *__file, __uid_t __owner, __gid_t __group, int __flag) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(2))) __attribute__((__warn_unused_result__));

/* Change the process's working directory to PATH.  */
extern int chdir(const char *__path) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1))) __attribute__((__warn_unused_result__));

/* Change the process's working directory to the one FD is open on.  */
extern int fchdir(int __fd) __attribute__((__nothrow__, __leaf__)) __attribute__((__warn_unused_result__));

/* Get the pathname of the current working directory,
   and put it in SIZE bytes of BUF.  Returns NULL if the
   directory couldn't be determined or SIZE was too small.
   If successful, returns BUF.  In GNU, if BUF is NULL,
   an array is allocated with `malloc'; the array is SIZE
   bytes long, unless SIZE == 0, in which case it is as
   big as necessary.  */
extern char *getcwd(char *__buf, size_t __size) __attribute__((__nothrow__, __leaf__)) __attribute__((__warn_unused_result__));

/* Return a malloc'd string containing the current directory name.
   If the environment variable `PWD' is set, and its value is correct,
   that value is used.  */
extern char *get_current_dir_name(void) __attribute__((__nothrow__, __leaf__));

/* Put the absolute pathname of the current working directory in BUF.
   If successful, return BUF.  If not, put an error message in
   BUF and return NULL.  BUF should be at least PATH_MAX bytes long.  */
extern char *getwd(char *__buf) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1))) __attribute__((__deprecated__))
__attribute__((__warn_unused_result__)) __attribute__((__access__(__write_only__, 1)));

/* Duplicate FD, returning a new file descriptor on the same file.  */
extern int dup(int __fd) __attribute__((__nothrow__, __leaf__)) __attribute__((__warn_unused_result__));

/* Duplicate FD to FD2, closing FD2 and making it open on the same file.  */
extern int dup2(int __fd, int __fd2) __attribute__((__nothrow__, __leaf__));

/* Duplicate FD to FD2, closing FD2 and making it open on the same
   file while setting flags according to FLAGS.  */
extern int dup3(int __fd, int __fd2, int __flags) __attribute__((__nothrow__, __leaf__));

/* NULL-terminated array of "NAME=VALUE" environment variables.  */
extern char **__environ;

extern char **environ;

/* Replace the current process, executing PATH with arguments ARGV and
   environment ENVP.  ARGV and ENVP are terminated by NULL pointers.  */
extern int execve(const char *__path, char *const __argv[], char *const __envp[]) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1, 2)));

/* Execute the file FD refers to, overlaying the running program image.
   ARGV and ENVP are passed to the new program, as for `execve'.  */
extern int fexecve(int __fd, char *const __argv[], char *const __envp[]) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(2)));

/* Execute PATH with arguments ARGV and environment from `environ'.  */
extern int execv(const char *__path, char *const __argv[]) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1, 2)));

/* Execute PATH with all arguments after PATH until a NULL pointer,
   and the argument after that for environment.  */
extern int execle(const char *__path, const char *__arg, ...) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1, 2)));

/* Execute PATH with all arguments after PATH until
   a NULL pointer and environment from `environ'.  */
extern int execl(const char *__path, const char *__arg, ...) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1, 2)));

/* Execute FILE, searching in the `PATH' environment variable if it contains
   no slashes, with arguments ARGV and environment from `environ'.  */
extern int execvp(const char *__file, char *const __argv[]) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1, 2)));

/* Execute FILE, searching in the `PATH' environment variable if
   it contains no slashes, with all arguments after FILE until a
   NULL pointer and environment from `environ'.  */
extern int execlp(const char *__file, const char *__arg, ...) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1, 2)));

/* Execute FILE, searching in the `PATH' environment variable if it contains
   no slashes, with arguments ARGV and environment from `environ'.  */
extern int execvpe(const char *__file, char *const __argv[], char *const __envp[]) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1, 2)));

/* Add INC to priority of the current process.  */
extern int nice(int __inc) __attribute__((__nothrow__, __leaf__)) __attribute__((__warn_unused_result__));

/* Terminate program execution with the low-order 8 bits of STATUS.  */
extern void _exit(int __status) __attribute__((__noreturn__));

/* Get the `_PC_*' symbols for the NAME argument to `pathconf' and `fpathconf';
   the `_SC_*' symbols for the NAME argument to `sysconf';
   and the `_CS_*' symbols for the NAME argument to `confstr'.  */
/* `sysconf', `pathconf', and `confstr' NAME values.  Generic version.
   Copyright (C) 1993-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* Values for the NAME argument to `pathconf' and `fpathconf'.  */
enum {
    _PC_LINK_MAX,

    _PC_MAX_CANON,

    _PC_MAX_INPUT,

    _PC_NAME_MAX,

    _PC_PATH_MAX,

    _PC_PIPE_BUF,

    _PC_CHOWN_RESTRICTED,

    _PC_NO_TRUNC,

    _PC_VDISABLE,

    _PC_SYNC_IO,

    _PC_ASYNC_IO,

    _PC_PRIO_IO,

    _PC_SOCK_MAXBUF,

    _PC_FILESIZEBITS,

    _PC_REC_INCR_XFER_SIZE,

    _PC_REC_MAX_XFER_SIZE,

    _PC_REC_MIN_XFER_SIZE,

    _PC_REC_XFER_ALIGN,

    _PC_ALLOC_SIZE_MIN,

    _PC_SYMLINK_MAX,

    _PC_2_SYMLINKS

};

/* Values for the argument to `sysconf'.  */
enum {
    _SC_ARG_MAX,

    _SC_CHILD_MAX,

    _SC_CLK_TCK,

    _SC_NGROUPS_MAX,

    _SC_OPEN_MAX,

    _SC_STREAM_MAX,

    _SC_TZNAME_MAX,

    _SC_JOB_CONTROL,

    _SC_SAVED_IDS,

    _SC_REALTIME_SIGNALS,

    _SC_PRIORITY_SCHEDULING,

    _SC_TIMERS,

    _SC_ASYNCHRONOUS_IO,

    _SC_PRIORITIZED_IO,

    _SC_SYNCHRONIZED_IO,

    _SC_FSYNC,

    _SC_MAPPED_FILES,

    _SC_MEMLOCK,

    _SC_MEMLOCK_RANGE,

    _SC_MEMORY_PROTECTION,

    _SC_MESSAGE_PASSING,

    _SC_SEMAPHORES,

    _SC_SHARED_MEMORY_OBJECTS,

    _SC_AIO_LISTIO_MAX,

    _SC_AIO_MAX,

    _SC_AIO_PRIO_DELTA_MAX,

    _SC_DELAYTIMER_MAX,

    _SC_MQ_OPEN_MAX,

    _SC_MQ_PRIO_MAX,

    _SC_VERSION,

    _SC_PAGESIZE,

    _SC_RTSIG_MAX,

    _SC_SEM_NSEMS_MAX,

    _SC_SEM_VALUE_MAX,

    _SC_SIGQUEUE_MAX,

    _SC_TIMER_MAX,

    /* Values for the argument to `sysconf'
       corresponding to _POSIX2_* symbols.  */
    _SC_BC_BASE_MAX,

    _SC_BC_DIM_MAX,

    _SC_BC_SCALE_MAX,

    _SC_BC_STRING_MAX,

    _SC_COLL_WEIGHTS_MAX,

    _SC_EQUIV_CLASS_MAX,

    _SC_EXPR_NEST_MAX,

    _SC_LINE_MAX,

    _SC_RE_DUP_MAX,

    _SC_CHARCLASS_NAME_MAX,

    _SC_2_VERSION,

    _SC_2_C_BIND,

    _SC_2_C_DEV,

    _SC_2_FORT_DEV,

    _SC_2_FORT_RUN,

    _SC_2_SW_DEV,

    _SC_2_LOCALEDEF,

    _SC_PII,

    _SC_PII_XTI,

    _SC_PII_SOCKET,

    _SC_PII_INTERNET,

    _SC_PII_OSI,

    _SC_POLL,

    _SC_SELECT,

    _SC_UIO_MAXIOV,

    _SC_IOV_MAX = _SC_UIO_MAXIOV,

    _SC_PII_INTERNET_STREAM,

    _SC_PII_INTERNET_DGRAM,

    _SC_PII_OSI_COTS,

    _SC_PII_OSI_CLTS,

    _SC_PII_OSI_M,

    _SC_T_IOV_MAX,

    /* Values according to POSIX 1003.1c (POSIX threads).  */
    _SC_THREADS,

    _SC_THREAD_SAFE_FUNCTIONS,

    _SC_GETGR_R_SIZE_MAX,

    _SC_GETPW_R_SIZE_MAX,

    _SC_LOGIN_NAME_MAX,

    _SC_TTY_NAME_MAX,

    _SC_THREAD_DESTRUCTOR_ITERATIONS,

    _SC_THREAD_KEYS_MAX,

    _SC_THREAD_STACK_MIN,

    _SC_THREAD_THREADS_MAX,

    _SC_THREAD_ATTR_STACKADDR,

    _SC_THREAD_ATTR_STACKSIZE,

    _SC_THREAD_PRIORITY_SCHEDULING,

    _SC_THREAD_PRIO_INHERIT,

    _SC_THREAD_PRIO_PROTECT,

    _SC_THREAD_PROCESS_SHARED,

    _SC_NPROCESSORS_CONF,

    _SC_NPROCESSORS_ONLN,

    _SC_PHYS_PAGES,

    _SC_AVPHYS_PAGES,

    _SC_ATEXIT_MAX,

    _SC_PASS_MAX,

    _SC_XOPEN_VERSION,

    _SC_XOPEN_XCU_VERSION,

    _SC_XOPEN_UNIX,

    _SC_XOPEN_CRYPT,

    _SC_XOPEN_ENH_I18N,

    _SC_XOPEN_SHM,

    _SC_2_CHAR_TERM,

    _SC_2_C_VERSION,

    _SC_2_UPE,

    _SC_XOPEN_XPG2,

    _SC_XOPEN_XPG3,

    _SC_XOPEN_XPG4,

    _SC_CHAR_BIT,

    _SC_CHAR_MAX,

    _SC_CHAR_MIN,

    _SC_INT_MAX,

    _SC_INT_MIN,

    _SC_LONG_BIT,

    _SC_WORD_BIT,

    _SC_MB_LEN_MAX,

    _SC_NZERO,

    _SC_SSIZE_MAX,

    _SC_SCHAR_MAX,

    _SC_SCHAR_MIN,

    _SC_SHRT_MAX,

    _SC_SHRT_MIN,

    _SC_UCHAR_MAX,

    _SC_UINT_MAX,

    _SC_ULONG_MAX,

    _SC_USHRT_MAX,

    _SC_NL_ARGMAX,

    _SC_NL_LANGMAX,

    _SC_NL_MSGMAX,

    _SC_NL_NMAX,

    _SC_NL_SETMAX,

    _SC_NL_TEXTMAX,

    _SC_XBS5_ILP32_OFF32,

    _SC_XBS5_ILP32_OFFBIG,

    _SC_XBS5_LP64_OFF64,

    _SC_XBS5_LPBIG_OFFBIG,

    _SC_XOPEN_LEGACY,

    _SC_XOPEN_REALTIME,

    _SC_XOPEN_REALTIME_THREADS,

    _SC_ADVISORY_INFO,

    _SC_BARRIERS,

    _SC_BASE,

    _SC_C_LANG_SUPPORT,

    _SC_C_LANG_SUPPORT_R,

    _SC_CLOCK_SELECTION,

    _SC_CPUTIME,

    _SC_THREAD_CPUTIME,

    _SC_DEVICE_IO,

    _SC_DEVICE_SPECIFIC,

    _SC_DEVICE_SPECIFIC_R,

    _SC_FD_MGMT,

    _SC_FIFO,

    _SC_PIPE,

    _SC_FILE_ATTRIBUTES,

    _SC_FILE_LOCKING,

    _SC_FILE_SYSTEM,

    _SC_MONOTONIC_CLOCK,

    _SC_MULTI_PROCESS,

    _SC_SINGLE_PROCESS,

    _SC_NETWORKING,

    _SC_READER_WRITER_LOCKS,

    _SC_SPIN_LOCKS,

    _SC_REGEXP,

    _SC_REGEX_VERSION,

    _SC_SHELL,

    _SC_SIGNALS,

    _SC_SPAWN,

    _SC_SPORADIC_SERVER,

    _SC_THREAD_SPORADIC_SERVER,

    _SC_SYSTEM_DATABASE,

    _SC_SYSTEM_DATABASE_R,

    _SC_TIMEOUTS,

    _SC_TYPED_MEMORY_OBJECTS,

    _SC_USER_GROUPS,

    _SC_USER_GROUPS_R,

    _SC_2_PBS,

    _SC_2_PBS_ACCOUNTING,

    _SC_2_PBS_LOCATE,

    _SC_2_PBS_MESSAGE,

    _SC_2_PBS_TRACK,

    _SC_SYMLOOP_MAX,

    _SC_STREAMS,

    _SC_2_PBS_CHECKPOINT,

    _SC_V6_ILP32_OFF32,

    _SC_V6_ILP32_OFFBIG,

    _SC_V6_LP64_OFF64,

    _SC_V6_LPBIG_OFFBIG,

    _SC_HOST_NAME_MAX,

    _SC_TRACE,

    _SC_TRACE_EVENT_FILTER,

    _SC_TRACE_INHERIT,

    _SC_TRACE_LOG,

    _SC_LEVEL1_ICACHE_SIZE,

    _SC_LEVEL1_ICACHE_ASSOC,

    _SC_LEVEL1_ICACHE_LINESIZE,

    _SC_LEVEL1_DCACHE_SIZE,

    _SC_LEVEL1_DCACHE_ASSOC,

    _SC_LEVEL1_DCACHE_LINESIZE,

    _SC_LEVEL2_CACHE_SIZE,

    _SC_LEVEL2_CACHE_ASSOC,

    _SC_LEVEL2_CACHE_LINESIZE,

    _SC_LEVEL3_CACHE_SIZE,

    _SC_LEVEL3_CACHE_ASSOC,

    _SC_LEVEL3_CACHE_LINESIZE,

    _SC_LEVEL4_CACHE_SIZE,

    _SC_LEVEL4_CACHE_ASSOC,

    _SC_LEVEL4_CACHE_LINESIZE,

    /* Leave room here, maybe we need a few more cache levels some day.  */

    _SC_IPV6 = _SC_LEVEL1_ICACHE_SIZE + 50,

    _SC_RAW_SOCKETS,

    _SC_V7_ILP32_OFF32,

    _SC_V7_ILP32_OFFBIG,

    _SC_V7_LP64_OFF64,

    _SC_V7_LPBIG_OFFBIG,

    _SC_SS_REPL_MAX,

    _SC_TRACE_EVENT_NAME_MAX,

    _SC_TRACE_NAME_MAX,

    _SC_TRACE_SYS_MAX,

    _SC_TRACE_USER_EVENT_MAX,

    _SC_XOPEN_STREAMS,

    _SC_THREAD_ROBUST_PRIO_INHERIT,

    _SC_THREAD_ROBUST_PRIO_PROTECT,

    _SC_MINSIGSTKSZ,

    _SC_SIGSTKSZ

};

/* Values for the NAME argument to `confstr'.  */
enum {
    _CS_PATH, /* The default search path.  */

    _CS_V6_WIDTH_RESTRICTED_ENVS,

    _CS_GNU_LIBC_VERSION,

    _CS_GNU_LIBPTHREAD_VERSION,

    _CS_V5_WIDTH_RESTRICTED_ENVS,

    _CS_V7_WIDTH_RESTRICTED_ENVS,

    _CS_LFS_CFLAGS = 1000,

    _CS_LFS_LDFLAGS,

    _CS_LFS_LIBS,

    _CS_LFS_LINTFLAGS,

    _CS_LFS64_CFLAGS,

    _CS_LFS64_LDFLAGS,

    _CS_LFS64_LIBS,

    _CS_LFS64_LINTFLAGS,

    _CS_XBS5_ILP32_OFF32_CFLAGS = 1100,

    _CS_XBS5_ILP32_OFF32_LDFLAGS,

    _CS_XBS5_ILP32_OFF32_LIBS,

    _CS_XBS5_ILP32_OFF32_LINTFLAGS,

    _CS_XBS5_ILP32_OFFBIG_CFLAGS,

    _CS_XBS5_ILP32_OFFBIG_LDFLAGS,

    _CS_XBS5_ILP32_OFFBIG_LIBS,

    _CS_XBS5_ILP32_OFFBIG_LINTFLAGS,

    _CS_XBS5_LP64_OFF64_CFLAGS,

    _CS_XBS5_LP64_OFF64_LDFLAGS,

    _CS_XBS5_LP64_OFF64_LIBS,

    _CS_XBS5_LP64_OFF64_LINTFLAGS,

    _CS_XBS5_LPBIG_OFFBIG_CFLAGS,

    _CS_XBS5_LPBIG_OFFBIG_LDFLAGS,

    _CS_XBS5_LPBIG_OFFBIG_LIBS,

    _CS_XBS5_LPBIG_OFFBIG_LINTFLAGS,

    _CS_POSIX_V6_ILP32_OFF32_CFLAGS,

    _CS_POSIX_V6_ILP32_OFF32_LDFLAGS,

    _CS_POSIX_V6_ILP32_OFF32_LIBS,

    _CS_POSIX_V6_ILP32_OFF32_LINTFLAGS,

    _CS_POSIX_V6_ILP32_OFFBIG_CFLAGS,

    _CS_POSIX_V6_ILP32_OFFBIG_LDFLAGS,

    _CS_POSIX_V6_ILP32_OFFBIG_LIBS,

    _CS_POSIX_V6_ILP32_OFFBIG_LINTFLAGS,

    _CS_POSIX_V6_LP64_OFF64_CFLAGS,

    _CS_POSIX_V6_LP64_OFF64_LDFLAGS,

    _CS_POSIX_V6_LP64_OFF64_LIBS,

    _CS_POSIX_V6_LP64_OFF64_LINTFLAGS,

    _CS_POSIX_V6_LPBIG_OFFBIG_CFLAGS,

    _CS_POSIX_V6_LPBIG_OFFBIG_LDFLAGS,

    _CS_POSIX_V6_LPBIG_OFFBIG_LIBS,

    _CS_POSIX_V6_LPBIG_OFFBIG_LINTFLAGS,

    _CS_POSIX_V7_ILP32_OFF32_CFLAGS,

    _CS_POSIX_V7_ILP32_OFF32_LDFLAGS,

    _CS_POSIX_V7_ILP32_OFF32_LIBS,

    _CS_POSIX_V7_ILP32_OFF32_LINTFLAGS,

    _CS_POSIX_V7_ILP32_OFFBIG_CFLAGS,

    _CS_POSIX_V7_ILP32_OFFBIG_LDFLAGS,

    _CS_POSIX_V7_ILP32_OFFBIG_LIBS,

    _CS_POSIX_V7_ILP32_OFFBIG_LINTFLAGS,

    _CS_POSIX_V7_LP64_OFF64_CFLAGS,

    _CS_POSIX_V7_LP64_OFF64_LDFLAGS,

    _CS_POSIX_V7_LP64_OFF64_LIBS,

    _CS_POSIX_V7_LP64_OFF64_LINTFLAGS,

    _CS_POSIX_V7_LPBIG_OFFBIG_CFLAGS,

    _CS_POSIX_V7_LPBIG_OFFBIG_LDFLAGS,

    _CS_POSIX_V7_LPBIG_OFFBIG_LIBS,

    _CS_POSIX_V7_LPBIG_OFFBIG_LINTFLAGS,

    _CS_V6_ENV,

    _CS_V7_ENV

};

/* Get file-specific configuration information about PATH.  */
extern long int pathconf(const char *__path, int __name) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));

/* Get file-specific configuration about descriptor FD.  */
extern long int fpathconf(int __fd, int __name) __attribute__((__nothrow__, __leaf__));

/* Get the value of the system variable NAME.  */
extern long int sysconf(int __name) __attribute__((__nothrow__, __leaf__));

/* Get the value of the string-valued system variable NAME.  */
extern size_t confstr(int __name, char *__buf, size_t __len) __attribute__((__nothrow__, __leaf__)) __attribute__((__access__(__write_only__, 2)));

/* Get the process ID of the calling process.  */
extern __pid_t getpid(void) __attribute__((__nothrow__, __leaf__));

/* Get the process ID of the calling process's parent.  */
extern __pid_t getppid(void) __attribute__((__nothrow__, __leaf__));

/* Get the process group ID of the calling process.  */
extern __pid_t getpgrp(void) __attribute__((__nothrow__, __leaf__));

/* Get the process group ID of process PID.  */
extern __pid_t __getpgid(__pid_t __pid) __attribute__((__nothrow__, __leaf__));

extern __pid_t getpgid(__pid_t __pid) __attribute__((__nothrow__, __leaf__));

/* Set the process group ID of the process matching PID to PGID.
   If PID is zero, the current process's process group ID is set.
   If PGID is zero, the process ID of the process is used.  */
extern int setpgid(__pid_t __pid, __pid_t __pgid) __attribute__((__nothrow__, __leaf__));

/* Both System V and BSD have `setpgrp' functions, but with different
   calling conventions.  The BSD function is the same as POSIX.1 `setpgid'
   (above).  The System V function takes no arguments and puts the calling
   process in its on group like `setpgid (0, 0)'.

   New programs should always use `setpgid' instead.

   GNU provides the POSIX.1 function.  */

/* Set the process group ID of the calling process to its own PID.
   This is exactly the same as `setpgid (0, 0)'.  */
extern int setpgrp(void) __attribute__((__nothrow__, __leaf__));

/* Create a new session with the calling process as its leader.
   The process group IDs of the session and the calling process
   are set to the process ID of the calling process, which is returned.  */
extern __pid_t setsid(void) __attribute__((__nothrow__, __leaf__));

/* Return the session ID of the given process.  */
extern __pid_t getsid(__pid_t __pid) __attribute__((__nothrow__, __leaf__));

/* Get the real user ID of the calling process.  */
extern __uid_t getuid(void) __attribute__((__nothrow__, __leaf__));

/* Get the effective user ID of the calling process.  */
extern __uid_t geteuid(void) __attribute__((__nothrow__, __leaf__));

/* Get the real group ID of the calling process.  */
extern __gid_t getgid(void) __attribute__((__nothrow__, __leaf__));

/* Get the effective group ID of the calling process.  */
extern __gid_t getegid(void) __attribute__((__nothrow__, __leaf__));

/* If SIZE is zero, return the number of supplementary groups
   the calling process is in.  Otherwise, fill in the group IDs
   of its supplementary groups in LIST and return the number written.  */
extern int getgroups(int __size, __gid_t __list[]) __attribute__((__nothrow__, __leaf__)) __attribute__((__warn_unused_result__))
__attribute__((__access__(__write_only__, 2)));

/* Return nonzero iff the calling process is in group GID.  */
extern int group_member(__gid_t __gid) __attribute__((__nothrow__, __leaf__));

/* Set the user ID of the calling process to UID.
   If the calling process is the super-user, set the real
   and effective user IDs, and the saved set-user-ID to UID;
   if not, the effective user ID is set to UID.  */
extern int setuid(__uid_t __uid) __attribute__((__nothrow__, __leaf__)) __attribute__((__warn_unused_result__));

/* Set the real user ID of the calling process to RUID,
   and the effective user ID of the calling process to EUID.  */
extern int setreuid(__uid_t __ruid, __uid_t __euid) __attribute__((__nothrow__, __leaf__)) __attribute__((__warn_unused_result__));

/* Set the effective user ID of the calling process to UID.  */
extern int seteuid(__uid_t __uid) __attribute__((__nothrow__, __leaf__)) __attribute__((__warn_unused_result__));

/* Set the group ID of the calling process to GID.
   If the calling process is the super-user, set the real
   and effective group IDs, and the saved set-group-ID to GID;
   if not, the effective group ID is set to GID.  */
extern int setgid(__gid_t __gid) __attribute__((__nothrow__, __leaf__)) __attribute__((__warn_unused_result__));

/* Set the real group ID of the calling process to RGID,
   and the effective group ID of the calling process to EGID.  */
extern int setregid(__gid_t __rgid, __gid_t __egid) __attribute__((__nothrow__, __leaf__)) __attribute__((__warn_unused_result__));

/* Set the effective group ID of the calling process to GID.  */
extern int setegid(__gid_t __gid) __attribute__((__nothrow__, __leaf__)) __attribute__((__warn_unused_result__));

/* Fetch the real user ID, effective user ID, and saved-set user ID,
   of the calling process.  */
extern int getresuid(__uid_t *__ruid, __uid_t *__euid, __uid_t *__suid) __attribute__((__nothrow__, __leaf__));

/* Fetch the real group ID, effective group ID, and saved-set group ID,
   of the calling process.  */
extern int getresgid(__gid_t *__rgid, __gid_t *__egid, __gid_t *__sgid) __attribute__((__nothrow__, __leaf__));

/* Set the real user ID, effective user ID, and saved-set user ID,
   of the calling process to RUID, EUID, and SUID, respectively.  */
extern int setresuid(__uid_t __ruid, __uid_t __euid, __uid_t __suid) __attribute__((__nothrow__, __leaf__)) __attribute__((__warn_unused_result__));

/* Set the real group ID, effective group ID, and saved-set group ID,
   of the calling process to RGID, EGID, and SGID, respectively.  */
extern int setresgid(__gid_t __rgid, __gid_t __egid, __gid_t __sgid) __attribute__((__nothrow__, __leaf__)) __attribute__((__warn_unused_result__));

/* Clone the calling process, creating an exact copy.
   Return -1 for errors, 0 to the new process,
   and the process ID of the new process to the old process.  */
extern __pid_t fork(void) __attribute__((__nothrow__));

/* Clone the calling process, but without copying the whole address space.
   The calling process is suspended until the new process exits or is
   replaced by a call to `execve'.  Return -1 for errors, 0 to the new process,
   and the process ID of the new process to the old process.  */
extern __pid_t vfork(void) __attribute__((__nothrow__, __leaf__));

/* This is similar to fork, however it does not run the atfork handlers
   neither reinitialize any internal locks in multithread case.
   Different than fork, _Fork is async-signal-safe.  */
extern __pid_t _Fork(void) __attribute__((__nothrow__, __leaf__));

/* Return the pathname of the terminal FD is open on, or NULL on errors.
   The returned storage is good only until the next call to this function.  */
extern char *ttyname(int __fd) __attribute__((__nothrow__, __leaf__));

/* Store at most BUFLEN characters of the pathname of the terminal FD is
   open on in BUF.  Return 0 on success, otherwise an error number.  */
extern int ttyname_r(int __fd, char *__buf, size_t __buflen) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(2)))
__attribute__((__warn_unused_result__)) __attribute__((__access__(__write_only__, 2)));

/* Return 1 if FD is a valid descriptor associated
   with a terminal, zero if not.  */
extern int isatty(int __fd) __attribute__((__nothrow__, __leaf__));

/* Return the index into the active-logins file (utmp) for
   the controlling terminal.  */
extern int ttyslot(void) __attribute__((__nothrow__, __leaf__));

/* Make a link to FROM named TO.  */
extern int link(const char *__from, const char *__to) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1, 2)))
__attribute__((__warn_unused_result__));

/* Like link but relative paths in TO and FROM are interpreted relative
   to FROMFD and TOFD respectively.  */
extern int linkat(int __fromfd, const char *__from, int __tofd, const char *__to, int __flags) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(2, 4))) __attribute__((__warn_unused_result__));

/* Make a symbolic link to FROM named TO.  */
extern int symlink(const char *__from, const char *__to) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1, 2)))
__attribute__((__warn_unused_result__));

/* Read the contents of the symbolic link PATH into no more than
   LEN bytes of BUF.  The contents are not null-terminated.
   Returns the number of characters read, or -1 for errors.  */
extern ssize_t readlink(const char *__restrict __path, char *__restrict __buf, size_t __len) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(1, 2))) __attribute__((__warn_unused_result__)) __attribute__((__access__(__write_only__, 2)));

/* Like symlink but a relative path in TO is interpreted relative to TOFD.  */
extern int symlinkat(const char *__from, int __tofd, const char *__to) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1, 3)))
__attribute__((__warn_unused_result__));

/* Like readlink but a relative PATH is interpreted relative to FD.  */
extern ssize_t readlinkat(int __fd, const char *__restrict __path, char *__restrict __buf, size_t __len) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(2, 3))) __attribute__((__warn_unused_result__)) __attribute__((__access__(__write_only__, 3)));

/* Remove the link NAME.  */
extern int unlink(const char *__name) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));

/* Remove the link NAME relative to FD.  */
extern int unlinkat(int __fd, const char *__name, int __flag) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(2)));

/* Remove the directory PATH.  */
extern int rmdir(const char *__path) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));

/* Return the foreground process group ID of FD.  */
extern __pid_t tcgetpgrp(int __fd) __attribute__((__nothrow__, __leaf__));

/* Set the foreground process group ID of FD set PGRP_ID.  */
extern int tcsetpgrp(int __fd, __pid_t __pgrp_id) __attribute__((__nothrow__, __leaf__));

/* Return the login name of the user.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern char *getlogin(void);

/* Return at most NAME_LEN characters of the login name of the user in NAME.
   If it cannot be determined or some other error occurred, return the error
   code.  Otherwise return 0.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern int getlogin_r(char *__name, size_t __name_len) __attribute__((__nonnull__(1))) __attribute__((__access__(__write_only__, 1)));

/* Set the login name returned by `getlogin'.  */
extern int setlogin(const char *__name) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));

/* Get definitions and prototypes for functions to process the
   arguments in ARGV (ARGC of them, minus the program name) for
   options given in OPTS.  */
/* Declarations for getopt (POSIX compatibility shim).
   Copyright (C) 1989-2023 Free Software Foundation, Inc.
   Unlike the bulk of the getopt implementation, this file is NOT part
   of gnulib.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */
/* Declarations for getopt (basic, portable features only).
   Copyright (C) 1989-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library and is also part of gnulib.
   Patches to this file should be submitted to both projects.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* This header should not be used directly; include getopt.h or
   unistd.h instead.  Unlike most bits headers, it does not have
   a protective #error, because the guard macro for getopt.h in
   gnulib is not fixed.  */

/* For communication from 'getopt' to the caller.
   When 'getopt' finds an option that takes an argument,
   the argument value is returned here.
   Also, when 'ordering' is RETURN_IN_ORDER,
   each non-option ARGV-element is returned here.  */

extern char *optarg;

/* Index in ARGV of the next element to be scanned.
   This is used for communication to and from the caller
   and for communication between successive calls to 'getopt'.

   On entry to 'getopt', zero means this is the first call; initialize.

   When 'getopt' returns -1, this is the index of the first of the
   non-option elements that the caller should itself scan.

   Otherwise, 'optind' communicates from one call to the next
   how much of ARGV has been scanned so far.  */

extern int optind;

/* Callers store zero here to inhibit the error message 'getopt' prints
   for unrecognized options.  */

extern int opterr;

/* Set to an option character which was unrecognized.  */

extern int optopt;

/* Get definitions and prototypes for functions to process the
   arguments in ARGV (ARGC of them, minus the program name) for
   options given in OPTS.

   Return the option character from OPTS just read.  Return -1 when
   there are no more options.  For unrecognized options, or options
   missing arguments, 'optopt' is set to the option letter, and '?' is
   returned.

   The OPTS string is a list of characters which are recognized option
   letters, optionally followed by colons, specifying that that letter
   takes an argument, to be placed in 'optarg'.

   If a letter in OPTS is followed by two colons, its argument is
   optional.  This behavior is specific to the GNU 'getopt'.

   The argument '--' causes premature termination of argument
   scanning, explicitly telling 'getopt' that there are no more
   options.

   If OPTS begins with '-', then non-option arguments are treated as
   arguments to the option '\1'.  This behavior is specific to the GNU
   'getopt'.  If OPTS begins with '+', or POSIXLY_CORRECT is set in
   the environment, then do not permute arguments.

   For standards compliance, the 'argv' argument has the type
   char *const *, but this is inaccurate; if argument permutation is
   enabled, the argv array (not the strings it points to) must be
   writable.  */

extern int getopt(int ___argc, char *const *___argv, const char *__shortopts) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(2, 3)));

/* Put the name of the current host in no more than LEN bytes of NAME.
   The result is null-terminated if LEN is large enough for the full
   name and the terminator.  */
extern int gethostname(char *__name, size_t __len) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)))
__attribute__((__access__(__write_only__, 1)));

/* Set the name of the current host to NAME, which is LEN bytes long.
   This call is restricted to the super-user.  */
extern int sethostname(const char *__name, size_t __len) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)))
__attribute__((__warn_unused_result__)) __attribute__((__access__(__read_only__, 1, 2)));

/* Set the current machine's Internet number to ID.
   This call is restricted to the super-user.  */
extern int sethostid(long int __id) __attribute__((__nothrow__, __leaf__)) __attribute__((__warn_unused_result__));

/* Get and set the NIS (aka YP) domain name, if any.
   Called just like `gethostname' and `sethostname'.
   The NIS domain name is usually the empty string when not using NIS.  */
extern int getdomainname(char *__name, size_t __len) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)))
__attribute__((__warn_unused_result__)) __attribute__((__access__(__write_only__, 1)));
extern int setdomainname(const char *__name, size_t __len) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)))
__attribute__((__warn_unused_result__)) __attribute__((__access__(__read_only__, 1, 2)));

/* Revoke access permissions to all processes currently communicating
   with the control terminal, and then send a SIGHUP signal to the process
   group of the control terminal.  */
extern int vhangup(void) __attribute__((__nothrow__, __leaf__));

/* Revoke the access of all descriptors currently open on FILE.  */
extern int revoke(const char *__file) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1))) __attribute__((__warn_unused_result__));

/* Enable statistical profiling, writing samples of the PC into at most
   SIZE bytes of SAMPLE_BUFFER; every processor clock tick while profiling
   is enabled, the system examines the user PC and increments
   SAMPLE_BUFFER[((PC - OFFSET) / 2) * SCALE / 65536].  If SCALE is zero,
   disable profiling.  Returns zero on success, -1 on error.  */
extern int profil(unsigned short int *__sample_buffer, size_t __size, size_t __offset, unsigned int __scale) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(1)));

/* Turn accounting on if NAME is an existing file.  The system will then write
   a record for each process as it terminates, to this file.  If NAME is NULL,
   turn accounting off.  This call is restricted to the super-user.  */
extern int acct(const char *__name) __attribute__((__nothrow__, __leaf__));

/* Successive calls return the shells listed in `/etc/shells'.  */
extern char *getusershell(void) __attribute__((__nothrow__, __leaf__));
extern void endusershell(void) __attribute__((__nothrow__, __leaf__)); /* Discard cached info.  */
extern void setusershell(void) __attribute__((__nothrow__, __leaf__)); /* Rewind and re-read the file.  */

/* Put the program in the background, and dissociate from the controlling
   terminal.  If NOCHDIR is zero, do `chdir ("/")'.  If NOCLOSE is zero,
   redirects stdin, stdout, and stderr to /dev/null.  */
extern int daemon(int __nochdir, int __noclose) __attribute__((__nothrow__, __leaf__)) __attribute__((__warn_unused_result__));

/* Make PATH be the root directory (the starting point for absolute paths).
   This call is restricted to the super-user.  */
extern int chroot(const char *__path) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1))) __attribute__((__warn_unused_result__));

/* Prompt with PROMPT and read a string from the terminal without echoing.
   Uses /dev/tty if possible; otherwise stderr and stdin.  */
extern char *getpass(const char *__prompt) __attribute__((__nonnull__(1)));

/* Make all changes done to FD actually appear on disk.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
extern int fsync(int __fd);

/* Make all changes done to all files on the file system associated
   with FD actually appear on disk.  */
extern int syncfs(int __fd) __attribute__((__nothrow__, __leaf__));

/* Return identifier for the current host.  */
extern long int gethostid(void);

/* Make all changes done to all files actually appear on disk.  */
extern void sync(void) __attribute__((__nothrow__, __leaf__));

/* Return the number of bytes in a page.  This is the system's page size,
   which is not necessarily the same as the hardware page size.  */
extern int getpagesize(void) __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

/* Return the maximum number of file descriptors
   the current process could possibly have.  */
extern int getdtablesize(void) __attribute__((__nothrow__, __leaf__));

/* Truncate FILE to LENGTH bytes.  */

extern int truncate(const char *__file, __off_t __length) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)))
__attribute__((__warn_unused_result__));
extern int truncate64(const char *__file, __off64_t __length) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)))
__attribute__((__warn_unused_result__));

/* Truncate the file FD is open on to LENGTH bytes.  */

extern int ftruncate(int __fd, __off_t __length) __attribute__((__nothrow__, __leaf__)) __attribute__((__warn_unused_result__));
extern int ftruncate64(int __fd, __off64_t __length) __attribute__((__nothrow__, __leaf__)) __attribute__((__warn_unused_result__));
/* Set the end of accessible data space (aka "the break") to ADDR.
   Returns zero on success and -1 for errors (with errno set).  */
extern int brk(void *__addr) __attribute__((__nothrow__, __leaf__)) __attribute__((__warn_unused_result__));

/* Increase or decrease the end of accessible data space by DELTA bytes.
   If successful, returns the address the previous end of data space
   (i.e. the beginning of the new space, if DELTA > 0);
   returns (void *) -1 for errors (with errno set).  */
extern void *sbrk(intptr_t __delta) __attribute__((__nothrow__, __leaf__));

/* Invoke `system call' number SYSNO, passing it the remaining arguments.
   This is completely system-dependent, and not often useful.

   In Unix, `syscall' sets `errno' for all errors and most calls return -1
   for errors; in many systems you cannot pass arguments or get return
   values for all system calls (`pipe', `fork', and `getppid' typically
   among them).

   In Mach, all system calls take normal arguments and always return an
   error code (zero for success).  */
extern long int syscall(long int __sysno, ...) __attribute__((__nothrow__, __leaf__));

/* NOTE: These declarations also appear in <fcntl.h>; be sure to keep both
   files consistent.  Some systems have them there and some here, and some
   software depends on the macros being defined without including both.  */

/* `lockf' is a simpler interface to the locking facilities of `fcntl'.
   LEN is always relative to the current file position.
   The CMD argument is one of the following.

   This function is a cancellation point and therefore not marked with
   __THROW.  */

extern int lockf(int __fd, int __cmd, __off_t __len) __attribute__((__warn_unused_result__));
extern int lockf64(int __fd, int __cmd, __off64_t __len) __attribute__((__warn_unused_result__));

/* Evaluate EXPRESSION, and repeat as long as it returns -1 with `errno'
   set to EINTR.  */
/* Copy LENGTH bytes from INFD to OUTFD.  */
ssize_t copy_file_range(int __infd, __off64_t *__pinoff, int __outfd, __off64_t *__poutoff, size_t __length, unsigned int __flags);

/* Synchronize at least the data part of a file with the underlying
   media.  */
extern int fdatasync(int __fildes);

/* One-way hash PHRASE, returning a string suitable for storage in the
   user database.  SALT selects the one-way function to use, and
   ensures that no two users' hashes are the same, even if they use
   the same passphrase.  The return value points to static storage
   which will be overwritten by the next call to crypt.  */
extern char *crypt(const char *__key, const char *__salt) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1, 2)));

/* Swab pairs bytes in the first N bytes of the area pointed to by
   FROM and copy the result to TO.  The value of TO must not be in the
   range [FROM - N + 1, FROM - 1].  If N is odd the first byte in FROM
   is without partner.  */
extern void swab(const void *__restrict __from, void *__restrict __to, ssize_t __n) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1, 2)))
__attribute__((__access__(__read_only__, 1, 3))) __attribute__((__access__(__write_only__, 2, 3)));

/* Prior to Issue 6, the Single Unix Specification required these
   prototypes to appear in this header.  They are also found in
   <stdio.h>.  */
/* Unix98 requires this function to be declared here.  In other
   standards it is in <pthread.h>.  */

/* Write LENGTH bytes of randomness starting at BUFFER.  Return 0 on
   success or -1 on error.  */
int getentropy(void *__buffer, size_t __length) __attribute__((__warn_unused_result__)) __attribute__((__access__(__write_only__, 1, 2)));

/* Close all file descriptors in the range FD up to MAX_FD.  The flag FLAGS
   are define by the CLOSE_RANGE prefix.  This function behaves like close
   on the range and gaps where the file descriptor is invalid or errors
   encountered while closing file descriptors are ignored.   Returns 0 on
   successor or -1 for failure (and sets errno accordingly).  */
extern int close_range(unsigned int __fd, unsigned int __max_fd, int __flags) __attribute__((__nothrow__, __leaf__));

/* Define some macros helping to catch buffer overflows.  */

/* Checking macros for unistd functions.
   Copyright (C) 2005-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* Checking routines for unistd functions. Declaration only.
   Copyright (C) 2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */
extern ssize_t __read_chk(int __fd, void *__buf, size_t __nbytes, size_t __buflen) __attribute__((__warn_unused_result__))
__attribute__((__access__(__write_only__, 2, 3)));
extern ssize_t __read_alias(int __fd, void *__buf, size_t __nbytes) __asm__(""
                                                                            "read")

    __attribute__((__warn_unused_result__)) __attribute__((__access__(__write_only__, 2, 3)));
extern ssize_t __read_chk_warn(int __fd, void *__buf, size_t __nbytes, size_t __buflen) __asm__(""
                                                                                                "__read_chk")

    __attribute__((__warn_unused_result__)) __attribute__((__warning__("read called with bigger length than size of "
                                                                       "the destination buffer")));

extern ssize_t __pread_chk(int __fd, void *__buf, size_t __nbytes, __off_t __offset, size_t __bufsize) __attribute__((__warn_unused_result__))
__attribute__((__access__(__write_only__, 2, 3)));
extern ssize_t __pread64_chk(int __fd, void *__buf, size_t __nbytes, __off64_t __offset, size_t __bufsize) __attribute__((__warn_unused_result__))
__attribute__((__access__(__write_only__, 2, 3)));
extern ssize_t __pread_alias(int __fd, void *__buf, size_t __nbytes, __off_t __offset) __asm__(""
                                                                                               "pread")

    __attribute__((__warn_unused_result__)) __attribute__((__access__(__write_only__, 2, 3)));
extern ssize_t __pread64_alias(int __fd, void *__buf, size_t __nbytes, __off64_t __offset) __asm__(""
                                                                                                   "pread64")

    __attribute__((__warn_unused_result__)) __attribute__((__access__(__write_only__, 2, 3)));
extern ssize_t __pread_chk_warn(int __fd, void *__buf, size_t __nbytes, __off_t __offset, size_t __bufsize) __asm__(""
                                                                                                                    "__pread_chk")

    __attribute__((__warn_unused_result__)) __attribute__((__warning__("pread called with bigger length than size of "
                                                                       "the destination buffer")));
extern ssize_t __pread64_chk_warn(int __fd, void *__buf, size_t __nbytes, __off64_t __offset, size_t __bufsize) __asm__(""
                                                                                                                        "__pread64_chk")

    __attribute__((__warn_unused_result__)) __attribute__((__warning__("pread64 called with bigger length than size of "
                                                                       "the destination buffer")));

extern ssize_t __readlink_chk(const char *__restrict __path, char *__restrict __buf, size_t __len, size_t __buflen) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(1, 2))) __attribute__((__warn_unused_result__)) __attribute__((__access__(__write_only__, 2, 3)));
extern ssize_t __readlink_alias(const char *__restrict __path, char *__restrict __buf, size_t __len) __asm__(""
                                                                                                             "readlink") __attribute__((__nothrow__, __leaf__))

__attribute__((__nonnull__(1, 2))) __attribute__((__warn_unused_result__)) __attribute__((__access__(__write_only__, 2, 3)));
extern ssize_t __readlink_chk_warn(const char *__restrict __path, char *__restrict __buf, size_t __len, size_t __buflen) __asm__(""
                                                                                                                                 "__readlink_chk")
    __attribute__((__nothrow__, __leaf__))

    __attribute__((__nonnull__(1, 2))) __attribute__((__warn_unused_result__)) __attribute__((__warning__("readlink called with bigger length "
                                                                                                          "than size of destination buffer")));

extern ssize_t __readlinkat_chk(int __fd, const char *__restrict __path, char *__restrict __buf, size_t __len, size_t __buflen)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(2, 3))) __attribute__((__warn_unused_result__))
    __attribute__((__access__(__write_only__, 3, 4)));
extern ssize_t __readlinkat_alias(int __fd, const char *__restrict __path, char *__restrict __buf, size_t __len) __asm__(""
                                                                                                                         "readlinkat")
    __attribute__((__nothrow__, __leaf__))

    __attribute__((__nonnull__(2, 3))) __attribute__((__warn_unused_result__)) __attribute__((__access__(__write_only__, 3, 4)));
extern ssize_t __readlinkat_chk_warn(int __fd, const char *__restrict __path, char *__restrict __buf, size_t __len, size_t __buflen) __asm__(""
                                                                                                                                             "__readlinkat_chk")
    __attribute__((__nothrow__, __leaf__))

    __attribute__((__nonnull__(2, 3))) __attribute__((__warn_unused_result__)) __attribute__((__warning__("readlinkat called with bigger "
                                                                                                          "length than size of destination "
                                                                                                          "buffer")))

    ;

extern char *__getcwd_chk(char *__buf, size_t __size, size_t __buflen) __attribute__((__nothrow__, __leaf__)) __attribute__((__warn_unused_result__));
extern char *__getcwd_alias(char *__buf, size_t __size) __asm__(""
                                                                "getcwd") __attribute__((__nothrow__, __leaf__)) __attribute__((__warn_unused_result__));
extern char *__getcwd_chk_warn(char *__buf, size_t __size, size_t __buflen) __asm__(""
                                                                                    "__getcwd_chk") __attribute__((__nothrow__, __leaf__))

__attribute__((__warn_unused_result__)) __attribute__((__warning__("getcwd caller with bigger length than size of "
                                                                   "destination buffer")));

extern char *__getwd_chk(char *__buf, size_t buflen) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)))
__attribute__((__warn_unused_result__)) __attribute__((__access__(__write_only__, 1, 2)));
extern char *__getwd_warn(char *__buf) __asm__(""
                                               "getwd") __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)))
__attribute__((__warn_unused_result__)) __attribute__((__warning__("please use getcwd instead, as getwd "
                                                                   "doesn't specify buffer size")));

extern size_t __confstr_chk(int __name, char *__buf, size_t __len, size_t __buflen) __attribute__((__nothrow__, __leaf__))
__attribute__((__access__(__write_only__, 2, 3)));
extern size_t __confstr_alias(int __name, char *__buf, size_t __len) __asm__(""
                                                                             "confstr") __attribute__((__nothrow__, __leaf__))

__attribute__((__access__(__write_only__, 2, 3)));
extern size_t __confstr_chk_warn(int __name, char *__buf, size_t __len, size_t __buflen) __asm__(""
                                                                                                 "__confstr_chk") __attribute__((__nothrow__, __leaf__))

__attribute__((__warning__("confstr called with bigger length than size of destination "
                           "buffer")));

extern int __getgroups_chk(int __size, __gid_t __list[], size_t __listlen) __attribute__((__nothrow__, __leaf__)) __attribute__((__warn_unused_result__))
__attribute__((__access__(__write_only__, 2, 1)));
extern int __getgroups_alias(int __size, __gid_t __list[]) __asm__(""
                                                                   "getgroups") __attribute__((__nothrow__, __leaf__)) __attribute__((__warn_unused_result__))
__attribute__((__access__(__write_only__, 2, 1)));
extern int __getgroups_chk_warn(int __size, __gid_t __list[], size_t __listlen) __asm__(""
                                                                                        "__getgroups_chk") __attribute__((__nothrow__, __leaf__))

__attribute__((__warn_unused_result__)) __attribute__((__warning__("getgroups called with bigger group count than what "
                                                                   "can fit into destination buffer")));

extern int __ttyname_r_chk(int __fd, char *__buf, size_t __buflen, size_t __nreal) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(2)))
__attribute__((__access__(__write_only__, 2, 3)));
extern int __ttyname_r_alias(int __fd, char *__buf, size_t __buflen) __asm__(""
                                                                             "ttyname_r") __attribute__((__nothrow__, __leaf__))

__attribute__((__nonnull__(2)));
extern int __ttyname_r_chk_warn(int __fd, char *__buf, size_t __buflen, size_t __nreal) __asm__(""
                                                                                                "__ttyname_r_chk") __attribute__((__nothrow__, __leaf__))

__attribute__((__nonnull__(2))) __attribute__((__warning__("ttyname_r called with bigger buflen than "
                                                           "size of destination buffer")));

extern int __getlogin_r_chk(char *__buf, size_t __buflen, size_t __nreal) __attribute__((__nonnull__(1))) __attribute__((__access__(__write_only__, 1, 2)));
extern int __getlogin_r_alias(char *__buf, size_t __buflen) __asm__(""
                                                                    "getlogin_r") __attribute__((__nonnull__(1)));
extern int __getlogin_r_chk_warn(char *__buf, size_t __buflen, size_t __nreal) __asm__(""
                                                                                       "__getlogin_r_chk")

    __attribute__((__nonnull__(1))) __attribute__((__warning__("getlogin_r called with bigger buflen than "
                                                               "size of destination buffer")));

extern int __gethostname_chk(char *__buf, size_t __buflen, size_t __nreal) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)))
__attribute__((__access__(__write_only__, 1, 2)));
extern int __gethostname_alias(char *__buf, size_t __buflen) __asm__(""
                                                                     "gethostname") __attribute__((__nothrow__, __leaf__))

__attribute__((__nonnull__(1))) __attribute__((__access__(__write_only__, 1, 2)));
extern int __gethostname_chk_warn(char *__buf, size_t __buflen, size_t __nreal) __asm__(""
                                                                                        "__gethostname_chk") __attribute__((__nothrow__, __leaf__))

__attribute__((__nonnull__(1))) __attribute__((__warning__("gethostname called with bigger buflen than "
                                                           "size of destination buffer")));

extern int __getdomainname_chk(char *__buf, size_t __buflen, size_t __nreal) __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)))
__attribute__((__warn_unused_result__)) __attribute__((__access__(__write_only__, 1, 2)));
extern int __getdomainname_alias(char *__buf, size_t __buflen) __asm__(""
                                                                       "getdomainname") __attribute__((__nothrow__, __leaf__))

__attribute__((__nonnull__(1))) __attribute__((__warn_unused_result__)) __attribute__((__access__(__write_only__, 1, 2)));
extern int __getdomainname_chk_warn(char *__buf, size_t __buflen, size_t __nreal) __asm__(""
                                                                                          "__getdomainname_chk") __attribute__((__nothrow__, __leaf__))

__attribute__((__nonnull__(1))) __attribute__((__warn_unused_result__)) __attribute__((__warning__("getdomainname called with bigger "
                                                                                                   "buflen than size of destination "
                                                                                                   "buffer")))

;

extern __inline __attribute__((__always_inline__)) __attribute__((__gnu_inline__)) __attribute__((__artificial__)) __attribute__((__warn_unused_result__))
ssize_t
read(int __fd, void *__buf, size_t __nbytes)
{
    return (((__builtin_constant_p(__builtin_dynamic_object_size(__buf, 0)) && (__builtin_dynamic_object_size(__buf, 0)) == (long unsigned int)-1) ||
                (((__typeof(__nbytes))0 < (__typeof(__nbytes))-1 || (__builtin_constant_p(__nbytes) && (__nbytes) > 0)) &&
                    __builtin_constant_p((((long unsigned int)(__nbytes)) <= ((__builtin_dynamic_object_size(__buf, 0))) / ((sizeof(char))))) &&
                    (((long unsigned int)(__nbytes)) <= ((__builtin_dynamic_object_size(__buf, 0))) / ((sizeof(char)))))) ?
            __read_alias(__fd, __buf, __nbytes) :
            ((((__typeof(__nbytes))0 < (__typeof(__nbytes))-1 || (__builtin_constant_p(__nbytes) && (__nbytes) > 0)) &&
                 __builtin_constant_p((((long unsigned int)(__nbytes)) <= (__builtin_dynamic_object_size(__buf, 0)) / (sizeof(char)))) &&
                 !(((long unsigned int)(__nbytes)) <= (__builtin_dynamic_object_size(__buf, 0)) / (sizeof(char)))) ?
                    __read_chk_warn(__fd, __buf, __nbytes, __builtin_dynamic_object_size(__buf, 0)) :
                    __read_chk(__fd, __buf, __nbytes, __builtin_dynamic_object_size(__buf, 0))))

        ;
}

extern __inline __attribute__((__always_inline__)) __attribute__((__gnu_inline__)) __attribute__((__artificial__)) __attribute__((__warn_unused_result__))
ssize_t
pread(int __fd, void *__buf, size_t __nbytes, __off_t __offset)
{
    return (((__builtin_constant_p(__builtin_dynamic_object_size(__buf, 0)) && (__builtin_dynamic_object_size(__buf, 0)) == (long unsigned int)-1) ||
                (((__typeof(__nbytes))0 < (__typeof(__nbytes))-1 || (__builtin_constant_p(__nbytes) && (__nbytes) > 0)) &&
                    __builtin_constant_p((((long unsigned int)(__nbytes)) <= ((__builtin_dynamic_object_size(__buf, 0))) / ((sizeof(char))))) &&
                    (((long unsigned int)(__nbytes)) <= ((__builtin_dynamic_object_size(__buf, 0))) / ((sizeof(char)))))) ?
            __pread_alias(__fd, __buf, __nbytes, __offset) :
            ((((__typeof(__nbytes))0 < (__typeof(__nbytes))-1 || (__builtin_constant_p(__nbytes) && (__nbytes) > 0)) &&
                 __builtin_constant_p((((long unsigned int)(__nbytes)) <= (__builtin_dynamic_object_size(__buf, 0)) / (sizeof(char)))) &&
                 !(((long unsigned int)(__nbytes)) <= (__builtin_dynamic_object_size(__buf, 0)) / (sizeof(char)))) ?
                    __pread_chk_warn(__fd, __buf, __nbytes, __offset, __builtin_dynamic_object_size(__buf, 0)) :
                    __pread_chk(__fd, __buf, __nbytes, __offset, __builtin_dynamic_object_size(__buf, 0))))

        ;
}
extern __inline __attribute__((__always_inline__)) __attribute__((__gnu_inline__)) __attribute__((__artificial__)) __attribute__((__warn_unused_result__))
ssize_t
pread64(int __fd, void *__buf, size_t __nbytes, __off64_t __offset)
{
    return (((__builtin_constant_p(__builtin_dynamic_object_size(__buf, 0)) && (__builtin_dynamic_object_size(__buf, 0)) == (long unsigned int)-1) ||
                (((__typeof(__nbytes))0 < (__typeof(__nbytes))-1 || (__builtin_constant_p(__nbytes) && (__nbytes) > 0)) &&
                    __builtin_constant_p((((long unsigned int)(__nbytes)) <= ((__builtin_dynamic_object_size(__buf, 0))) / ((sizeof(char))))) &&
                    (((long unsigned int)(__nbytes)) <= ((__builtin_dynamic_object_size(__buf, 0))) / ((sizeof(char)))))) ?
            __pread64_alias(__fd, __buf, __nbytes, __offset) :
            ((((__typeof(__nbytes))0 < (__typeof(__nbytes))-1 || (__builtin_constant_p(__nbytes) && (__nbytes) > 0)) &&
                 __builtin_constant_p((((long unsigned int)(__nbytes)) <= (__builtin_dynamic_object_size(__buf, 0)) / (sizeof(char)))) &&
                 !(((long unsigned int)(__nbytes)) <= (__builtin_dynamic_object_size(__buf, 0)) / (sizeof(char)))) ?
                    __pread64_chk_warn(__fd, __buf, __nbytes, __offset, __builtin_dynamic_object_size(__buf, 0)) :
                    __pread64_chk(__fd, __buf, __nbytes, __offset, __builtin_dynamic_object_size(__buf, 0))))

        ;
}

extern __inline __attribute__((__always_inline__)) __attribute__((__gnu_inline__)) __attribute__((__artificial__)) __attribute__((__nonnull__(1, 2)))
__attribute__((__warn_unused_result__)) ssize_t __attribute__((__nothrow__, __leaf__))
readlink(const char *__restrict __path, char *__restrict __buf, size_t __len)

{
    return (((__builtin_constant_p(__builtin_dynamic_object_size(__buf, 1)) && (__builtin_dynamic_object_size(__buf, 1)) == (long unsigned int)-1) ||
                (((__typeof(__len))0 < (__typeof(__len))-1 || (__builtin_constant_p(__len) && (__len) > 0)) &&
                    __builtin_constant_p((((long unsigned int)(__len)) <= ((__builtin_dynamic_object_size(__buf, 1))) / ((sizeof(char))))) &&
                    (((long unsigned int)(__len)) <= ((__builtin_dynamic_object_size(__buf, 1))) / ((sizeof(char)))))) ?
            __readlink_alias(__path, __buf, __len) :
            ((((__typeof(__len))0 < (__typeof(__len))-1 || (__builtin_constant_p(__len) && (__len) > 0)) &&
                 __builtin_constant_p((((long unsigned int)(__len)) <= (__builtin_dynamic_object_size(__buf, 1)) / (sizeof(char)))) &&
                 !(((long unsigned int)(__len)) <= (__builtin_dynamic_object_size(__buf, 1)) / (sizeof(char)))) ?
                    __readlink_chk_warn(__path, __buf, __len, __builtin_dynamic_object_size(__buf, 1)) :
                    __readlink_chk(__path, __buf, __len, __builtin_dynamic_object_size(__buf, 1))))

        ;
}

extern __inline __attribute__((__always_inline__)) __attribute__((__gnu_inline__)) __attribute__((__artificial__)) __attribute__((__nonnull__(2, 3)))
__attribute__((__warn_unused_result__)) ssize_t __attribute__((__nothrow__, __leaf__))
readlinkat(int __fd, const char *__restrict __path, char *__restrict __buf, size_t __len)

{
    return (((__builtin_constant_p(__builtin_dynamic_object_size(__buf, 1)) && (__builtin_dynamic_object_size(__buf, 1)) == (long unsigned int)-1) ||
                (((__typeof(__len))0 < (__typeof(__len))-1 || (__builtin_constant_p(__len) && (__len) > 0)) &&
                    __builtin_constant_p((((long unsigned int)(__len)) <= ((__builtin_dynamic_object_size(__buf, 1))) / ((sizeof(char))))) &&
                    (((long unsigned int)(__len)) <= ((__builtin_dynamic_object_size(__buf, 1))) / ((sizeof(char)))))) ?
            __readlinkat_alias(__fd, __path, __buf, __len) :
            ((((__typeof(__len))0 < (__typeof(__len))-1 || (__builtin_constant_p(__len) && (__len) > 0)) &&
                 __builtin_constant_p((((long unsigned int)(__len)) <= (__builtin_dynamic_object_size(__buf, 1)) / (sizeof(char)))) &&
                 !(((long unsigned int)(__len)) <= (__builtin_dynamic_object_size(__buf, 1)) / (sizeof(char)))) ?
                    __readlinkat_chk_warn(__fd, __path, __buf, __len, __builtin_dynamic_object_size(__buf, 1)) :
                    __readlinkat_chk(__fd, __path, __buf, __len, __builtin_dynamic_object_size(__buf, 1))))

        ;
}

extern __inline __attribute__((__always_inline__)) __attribute__((__gnu_inline__)) __attribute__((__artificial__))
__attribute__((__warn_unused_result__)) char *__attribute__((__nothrow__, __leaf__)) getcwd(char *__buf, size_t __size)
{
    return (((__builtin_constant_p(__builtin_dynamic_object_size(__buf, 1)) && (__builtin_dynamic_object_size(__buf, 1)) == (long unsigned int)-1) ||
                (((__typeof(__size))0 < (__typeof(__size))-1 || (__builtin_constant_p(__size) && (__size) > 0)) &&
                    __builtin_constant_p((((long unsigned int)(__size)) <= ((__builtin_dynamic_object_size(__buf, 1))) / ((sizeof(char))))) &&
                    (((long unsigned int)(__size)) <= ((__builtin_dynamic_object_size(__buf, 1))) / ((sizeof(char)))))) ?
            __getcwd_alias(__buf, __size) :
            ((((__typeof(__size))0 < (__typeof(__size))-1 || (__builtin_constant_p(__size) && (__size) > 0)) &&
                 __builtin_constant_p((((long unsigned int)(__size)) <= (__builtin_dynamic_object_size(__buf, 1)) / (sizeof(char)))) &&
                 !(((long unsigned int)(__size)) <= (__builtin_dynamic_object_size(__buf, 1)) / (sizeof(char)))) ?
                    __getcwd_chk_warn(__buf, __size, __builtin_dynamic_object_size(__buf, 1)) :
                    __getcwd_chk(__buf, __size, __builtin_dynamic_object_size(__buf, 1))))

        ;
}

extern __inline __attribute__((__always_inline__)) __attribute__((__gnu_inline__)) __attribute__((__artificial__)) __attribute__((__nonnull__(1)))
__attribute__((__deprecated__)) __attribute__((__warn_unused_result__)) char *__attribute__((__nothrow__, __leaf__)) getwd(char *__buf)
{
    if (__builtin_dynamic_object_size(__buf, 1) != (size_t)-1)
        return __getwd_chk(__buf, __builtin_dynamic_object_size(__buf, 1));
    return __getwd_warn(__buf);
}

extern __inline __attribute__((__always_inline__)) __attribute__((__gnu_inline__)) __attribute__((__artificial__)) size_t __attribute__((__nothrow__, __leaf__))
confstr(int __name, char *__buf, size_t __len)
{
    return (((__builtin_constant_p(__builtin_dynamic_object_size(__buf, 1)) && (__builtin_dynamic_object_size(__buf, 1)) == (long unsigned int)-1) ||
                (((__typeof(__len))0 < (__typeof(__len))-1 || (__builtin_constant_p(__len) && (__len) > 0)) &&
                    __builtin_constant_p((((long unsigned int)(__len)) <= ((__builtin_dynamic_object_size(__buf, 1))) / ((sizeof(char))))) &&
                    (((long unsigned int)(__len)) <= ((__builtin_dynamic_object_size(__buf, 1))) / ((sizeof(char)))))) ?
            __confstr_alias(__name, __buf, __len) :
            ((((__typeof(__len))0 < (__typeof(__len))-1 || (__builtin_constant_p(__len) && (__len) > 0)) &&
                 __builtin_constant_p((((long unsigned int)(__len)) <= (__builtin_dynamic_object_size(__buf, 1)) / (sizeof(char)))) &&
                 !(((long unsigned int)(__len)) <= (__builtin_dynamic_object_size(__buf, 1)) / (sizeof(char)))) ?
                    __confstr_chk_warn(__name, __buf, __len, __builtin_dynamic_object_size(__buf, 1)) :
                    __confstr_chk(__name, __buf, __len, __builtin_dynamic_object_size(__buf, 1))))

        ;
}

extern __inline __attribute__((__always_inline__)) __attribute__((__gnu_inline__)) __attribute__((__artificial__)) int __attribute__((__nothrow__, __leaf__))
getgroups(int __size, __gid_t __list[])
{
    return (((__builtin_constant_p(__builtin_dynamic_object_size(__list, 1)) && (__builtin_dynamic_object_size(__list, 1)) == (long unsigned int)-1) ||
                (((__typeof(__size))0 < (__typeof(__size))-1 || (__builtin_constant_p(__size) && (__size) > 0)) &&
                    __builtin_constant_p((((long unsigned int)(__size)) <= ((__builtin_dynamic_object_size(__list, 1))) / ((sizeof(__gid_t))))) &&
                    (((long unsigned int)(__size)) <= ((__builtin_dynamic_object_size(__list, 1))) / ((sizeof(__gid_t)))))) ?
            __getgroups_alias(__size, __list) :
            ((((__typeof(__size))0 < (__typeof(__size))-1 || (__builtin_constant_p(__size) && (__size) > 0)) &&
                 __builtin_constant_p((((long unsigned int)(__size)) <= (__builtin_dynamic_object_size(__list, 1)) / (sizeof(__gid_t)))) &&
                 !(((long unsigned int)(__size)) <= (__builtin_dynamic_object_size(__list, 1)) / (sizeof(__gid_t)))) ?
                    __getgroups_chk_warn(__size, __list, __builtin_dynamic_object_size(__list, 1)) :
                    __getgroups_chk(__size, __list, __builtin_dynamic_object_size(__list, 1))))

        ;
}

extern __inline __attribute__((__always_inline__)) __attribute__((__gnu_inline__)) __attribute__((__artificial__)) int __attribute__((__nothrow__, __leaf__))
ttyname_r(int __fd, char *__buf, size_t __buflen)
{
    return (((__builtin_constant_p(__builtin_dynamic_object_size(__buf, 1)) && (__builtin_dynamic_object_size(__buf, 1)) == (long unsigned int)-1) ||
                (((__typeof(__buflen))0 < (__typeof(__buflen))-1 || (__builtin_constant_p(__buflen) && (__buflen) > 0)) &&
                    __builtin_constant_p((((long unsigned int)(__buflen)) <= ((__builtin_dynamic_object_size(__buf, 1))) / ((sizeof(char))))) &&
                    (((long unsigned int)(__buflen)) <= ((__builtin_dynamic_object_size(__buf, 1))) / ((sizeof(char)))))) ?
            __ttyname_r_alias(__fd, __buf, __buflen) :
            ((((__typeof(__buflen))0 < (__typeof(__buflen))-1 || (__builtin_constant_p(__buflen) && (__buflen) > 0)) &&
                 __builtin_constant_p((((long unsigned int)(__buflen)) <= (__builtin_dynamic_object_size(__buf, 1)) / (sizeof(char)))) &&
                 !(((long unsigned int)(__buflen)) <= (__builtin_dynamic_object_size(__buf, 1)) / (sizeof(char)))) ?
                    __ttyname_r_chk_warn(__fd, __buf, __buflen, __builtin_dynamic_object_size(__buf, 1)) :
                    __ttyname_r_chk(__fd, __buf, __buflen, __builtin_dynamic_object_size(__buf, 1))))

        ;
}

extern __inline __attribute__((__always_inline__)) __attribute__((__gnu_inline__)) __attribute__((__artificial__)) int
getlogin_r(char *__buf, size_t __buflen)
{
    return (((__builtin_constant_p(__builtin_dynamic_object_size(__buf, 1)) && (__builtin_dynamic_object_size(__buf, 1)) == (long unsigned int)-1) ||
                (((__typeof(__buflen))0 < (__typeof(__buflen))-1 || (__builtin_constant_p(__buflen) && (__buflen) > 0)) &&
                    __builtin_constant_p((((long unsigned int)(__buflen)) <= ((__builtin_dynamic_object_size(__buf, 1))) / ((sizeof(char))))) &&
                    (((long unsigned int)(__buflen)) <= ((__builtin_dynamic_object_size(__buf, 1))) / ((sizeof(char)))))) ?
            __getlogin_r_alias(__buf, __buflen) :
            ((((__typeof(__buflen))0 < (__typeof(__buflen))-1 || (__builtin_constant_p(__buflen) && (__buflen) > 0)) &&
                 __builtin_constant_p((((long unsigned int)(__buflen)) <= (__builtin_dynamic_object_size(__buf, 1)) / (sizeof(char)))) &&
                 !(((long unsigned int)(__buflen)) <= (__builtin_dynamic_object_size(__buf, 1)) / (sizeof(char)))) ?
                    __getlogin_r_chk_warn(__buf, __buflen, __builtin_dynamic_object_size(__buf, 1)) :
                    __getlogin_r_chk(__buf, __buflen, __builtin_dynamic_object_size(__buf, 1))))

        ;
}

extern __inline __attribute__((__always_inline__)) __attribute__((__gnu_inline__)) __attribute__((__artificial__)) int __attribute__((__nothrow__, __leaf__))
gethostname(char *__buf, size_t __buflen)
{
    return (((__builtin_constant_p(__builtin_dynamic_object_size(__buf, 1)) && (__builtin_dynamic_object_size(__buf, 1)) == (long unsigned int)-1) ||
                (((__typeof(__buflen))0 < (__typeof(__buflen))-1 || (__builtin_constant_p(__buflen) && (__buflen) > 0)) &&
                    __builtin_constant_p((((long unsigned int)(__buflen)) <= ((__builtin_dynamic_object_size(__buf, 1))) / ((sizeof(char))))) &&
                    (((long unsigned int)(__buflen)) <= ((__builtin_dynamic_object_size(__buf, 1))) / ((sizeof(char)))))) ?
            __gethostname_alias(__buf, __buflen) :
            ((((__typeof(__buflen))0 < (__typeof(__buflen))-1 || (__builtin_constant_p(__buflen) && (__buflen) > 0)) &&
                 __builtin_constant_p((((long unsigned int)(__buflen)) <= (__builtin_dynamic_object_size(__buf, 1)) / (sizeof(char)))) &&
                 !(((long unsigned int)(__buflen)) <= (__builtin_dynamic_object_size(__buf, 1)) / (sizeof(char)))) ?
                    __gethostname_chk_warn(__buf, __buflen, __builtin_dynamic_object_size(__buf, 1)) :
                    __gethostname_chk(__buf, __buflen, __builtin_dynamic_object_size(__buf, 1))))

        ;
}

extern __inline __attribute__((__always_inline__)) __attribute__((__gnu_inline__)) __attribute__((__artificial__)) int __attribute__((__nothrow__, __leaf__))
getdomainname(char *__buf, size_t __buflen)
{
    return (((__builtin_constant_p(__builtin_dynamic_object_size(__buf, 1)) && (__builtin_dynamic_object_size(__buf, 1)) == (long unsigned int)-1) ||
                (((__typeof(__buflen))0 < (__typeof(__buflen))-1 || (__builtin_constant_p(__buflen) && (__buflen) > 0)) &&
                    __builtin_constant_p((((long unsigned int)(__buflen)) <= ((__builtin_dynamic_object_size(__buf, 1))) / ((sizeof(char))))) &&
                    (((long unsigned int)(__buflen)) <= ((__builtin_dynamic_object_size(__buf, 1))) / ((sizeof(char)))))) ?
            __getdomainname_alias(__buf, __buflen) :
            ((((__typeof(__buflen))0 < (__typeof(__buflen))-1 || (__builtin_constant_p(__buflen) && (__buflen) > 0)) &&
                 __builtin_constant_p((((long unsigned int)(__buflen)) <= (__builtin_dynamic_object_size(__buf, 1)) / (sizeof(char)))) &&
                 !(((long unsigned int)(__buflen)) <= (__builtin_dynamic_object_size(__buf, 1)) / (sizeof(char)))) ?
                    __getdomainname_chk_warn(__buf, __buflen, __builtin_dynamic_object_size(__buf, 1)) :
                    __getdomainname_chk(__buf, __buflen, __builtin_dynamic_object_size(__buf, 1))))

        ;
}

/* System-specific extensions.  */
/* System-specific extensions of <unistd.h>, Linux version.
   Copyright (C) 2019-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* Return the kernel thread ID (TID) of the current thread.  The
   returned value is not subject to caching.  Most Linux system calls
   accept a TID in place of a PID.  Using the TID to change properties
   of a thread that has been created using pthread_create can lead to
   undefined behavior (comparable to manipulating file descriptors
   directly that have not been created explicitly).  Note that a TID
   uniquely identifies a thread only while this thread is running; a
   TID can be reused once a thread has exited, even if the thread is
   not detached and has not been joined.  */
extern __pid_t gettid(void) __attribute__((__nothrow__, __leaf__));

/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */

/* Unshare the file descriptor table before closing file descriptors. */

/* Set the FD_CLOEXEC bit instead of closing the file descriptor. */

/* Unshare the file descriptor table before closing file descriptors.  */

/* Set the FD_CLOEXEC bit instead of closing the file descriptor.  */

#pragma GCC diagnostic pop

/*
 * This file defines a skip-list data structure written in C.  Implemented as
 * using macros this code provides a way to essentially "template" (as in C++)
 * and emit code with types and functions specific to your use case.  You can
 * apply these macros multiple times safely, once for each list type you need.
 *
 * A skip-list is a sorted list with O(log(n)) on average for most operations.
 * It is a probabilistic data structure, meaning that it does not guarantee
 * O(log(n)), but it has been shown to approximate it over time.  This
 * implementation includes the re-balancing techniques that improve on that
 * approximation using an adaptive technique called "splay-list". It is similar
 * to a standard skip-list, with the key distinction that the height of each
 * element adapts dynamically to its access rate: popular elements increase in
 * height, whereas rarely-accessed elements decrease in height. See below for
 * the link to the research behind this adaptive technique.
 *
 * Conceptually, at a high level, a skip-list is arranged as follows:
 *
 * <head> ----------> [2] --------------------------------------------------> [9] ---------->
 * <head> ----------> [2] ------------------------------------[7] ----------> [9] ---------->
 * <head> ----------> [2] ----------> [4] ------------------> [7] ----------> [9] --> [10] ->
 * <head> --> [1] --> [2] --> [3] --> [4] --> [5] --> [6] --> [7] --> [8] --> [9] --> [10] ->
 *
 * Each node contains at the very least a link to the next element in the list
 * (corresponding to the lowest level in the above diagram), but it can randomly
 * contain more links which skip further down the list (the towers in the above
 * diagram). This allows for the algorithm to move down the list faster than
 * having to visit every element.
 *
 * A skip-list can be thought of as a stack of linked lists. At the very bottom
 * is a linked list with every element, and each layer above corresponds to a
 * linked list containing a random subset of the elements from the layer
 * immediately below it. The probability distribution that determines this
 * random subset can be customized, but typically a layer will contain half the
 * nodes from the layer below.
 *
 * This implementation maintains a doubly-linked list at the bottom layer to
 * support efficient iteration in either direction.  There is also a guard node
 * at the tail rather than simply pointing to NULL.
 *
 * <head> <-> [1] <-> [2] <-> [3] <-> [4] <-> [5] <-> [6] <-> [7] <-> <tail>
 *
 * Safety:
 *
 * The ordered skip-list relies on a well-behaved comparison
 * function. Specifically, given some ordering function f(a, b), it must satisfy
 * the following properties:
 *
 * 1) Be well defined: f(a, b) should always return the same value
 * 2) Be anti-symmetric: f(a, b) == Greater if and only if f(b, a) == Less, and
 *    f(a, b) == Equal == f(b, a).
 * 3) Be transitive: If f(a, b) == Greater and f(b, c) == Greater then f(a, c)
 *    == Greater.
 *
 * Failure to satisfy these properties can result in unexpected behavior at
 * best, and at worst will cause a segfault, null deref, or some other bad
 * behavior.
 *
 * References for this implementation include, but are not limited to:
 *
 *  - Skip lists: a probabilistic alternative to balanced trees
 *    @article{10.1145/78973.78977,
 *      author = {Pugh, William},
 *      title = {Skip lists: a probabilistic alternative to balanced trees},
 *      year = {1990}, issue_date = {June 1990},
 *      publisher = {Association for Computing Machinery},
 *      address = {New York, NY, USA},
 *      volume = {33}, number = {6}, issn = {0001-0782},
 *      url = {https://doi.org/10.1145/78973.78977},
 *      doi = {10.1145/78973.78977},
 *      journal = {Commun. ACM}, month = {jun}, pages = {668-676}, numpages = {9},
 *      keywords = {trees, searching, data structures},
 *      download = {https://www.cl.cam.ac.uk/teaching/2005/Algorithms/skiplists.pdf}
 *    }
 *
 *  - Tutorial: The Ubiquitous Skiplist, its Variants, and Applications in Modern Big Data Systems
 *    @article{Vadrevu2023TutorialTU,
 *      title={Tutorial: The Ubiquitous Skiplist, its Variants, and Applications in Modern Big Data Systems},
 *      author={Venkata Sai Pavan Kumar Vadrevu and Lu Xing and Walid G. Aref},
 *      journal={ArXiv},
 *      year={2023},
 *      volume={abs/2304.09983},
 *      url={https://api.semanticscholar.org/CorpusID:258236678},
 *      download={https://arxiv.org/pdf/2304.09983.pdf}
 *    }
 *
 *  - The Splay-List: A Distribution-Adaptive Concurrent Skip-List
 *    @misc{aksenov2020splaylist,
 *      title={The Splay-List: A Distribution-Adaptive Concurrent Skip-List},
 *      author={Vitaly Aksenov and Dan Alistarh and Alexandra Drozdova and Amirkeivan Mohtashami},
 *      year={2020},
 *      eprint={2008.01009},
 *      archivePrefix={arXiv},
 *      primaryClass={cs.DC},
 *      download={https://arxiv.org/pdf/2008.01009.pdf}
 *    }
 *
 *  - JellyFish: A Fast Skip List with MVCC},
 *    @article{Yeon2020JellyFishAF,
 *      title={JellyFish: A Fast Skip List with MVCC},
 *      author={Jeseong Yeon and Leeju Kim and Youil Han and Hyeon Gyu Lee and Eunji Lee and Bryan Suk Joon Kim},
 *      journal={Proceedings of the 21st International Middleware Conference},
 *      year={2020},
 *      url={https://api.semanticscholar.org/CorpusID:228086012}
 *    }
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wvariadic-macros"

#pragma GCC diagnostic pop

/**                                                                                                                                                      * --
 * __skip_diag__                                                                                                                                      * * Write
 * debug message to stderr with origin of message.                                                                                                 */

void __attribute__((format(printf, 4, 5))) __skip_diag_(const char *file, int line, const char *func, const char *format, ...)
{
    va_list args;

    __builtin_va_start(args, format);
    fprintf(stderr, "%s:%d:%s(): ", file, line, func);
    vfprintf(stderr, format, args);

    __builtin_va_end(args);
}
/*
 * Skiplist declarations.
 */

/*
 * Every Skiplist node has to have an additional section of data used to manage
 * nodes in the list. The rest of the data structure is defined by the use case.
 * This housekeeping portion is the SKIPLIST_ENTRY, see below.  It maintains the
 * array of forward pointers to nodes and has a height, this height is a
 * zero-based count of levels, so a height of `0` means one (1) level and a
 * height of `4` means five (5) forward pointers (levels) in the node, [0-4).
 */
/* Iterate from tail to head over the nodes. */

/* Iterate over the next pointers in a node from bottom to top (B2T) or top to bottom (T2B). */
/* Iterate over the left (v) subtree or right (u) subtree or "CHu" and "CHv". */

/* Iterate over a subtree starting at provided path element, u = path.in */

/*
 * Skiplist declarations and access methods.
 */

// Local demo application OPTIONS:
// ---------------------------------------------------------------------------

// define SNAPSHOTS
// define TODO_RESTORE_SNAPSHOTS

size_t gen = 0;
FILE *of = 0;

// ---------------------------------------------------------------------------

/*
 * SKIPLIST EXAMPLE:
 *
 * This example creates an "ex" Skiplist where keys are integers, values are
 * strings containing the roman numeral for the key allocated on the heap.
 */

/*
 * To start, you must create a type node that will contain the
 * fields you'd like to maintain in your Skiplist.  In this case
 * we map int -> char [] on the heap, but what you put here is up
 * to you.  You don't even need a "key", just a way to compare one
 * node against another, logic you'll provide in SKIP_DECL as a
 * block below.
 */
struct ex_node {
    int key;
    char *value;
    struct __skiplist_ex_entry {
        size_t sle_era;
        size_t sle_height;
        struct ex_node *sle_prev;
        struct __skiplist_ex_level {
            struct ex_node *next;
            size_t hits;
        } *sle_levels;
    } entries;
};

/*
 * Generate all the access functions for our type of Skiplist.
 */
typedef enum { SKIP_EQ = 0, SKIP_LTE = -1, SKIP_LT = -2, SKIP_GTE = 1, SKIP_GT = 2 } skip_pos_decl_t;
typedef struct ex_node ex_node_t;
typedef struct ex ex_t;
struct ex {
    size_t slh_length;
    void *slh_aux;
    int slh_prng_state;
    ex_node_t *slh_head;
    ex_node_t *slh_tail;
    struct {
        void (*free_entry)(ex_node_t *);
        int (*update_entry)(ex_node_t *, void *);
        int (*archive_entry)(ex_node_t *, const ex_node_t *);
        size_t (*sizeof_entry)(ex_node_t *);
        int (*compare_entries)(ex_t *, ex_node_t *, ex_node_t *, void *);
        int (*snapshot_preserve_node)(ex_t *slist, const ex_node_t *src, ex_node_t **preserved);
        void (*snapshot_release)(ex_t *);
    } slh_fns;
    struct {
        size_t cur_era;
        size_t pres_era;
        ex_node_t *pres;
    } slh_snap;
};
typedef struct __skiplist_path_ex {
    ex_node_t *node;
    size_t in;
    size_t pu;
} __skiplist_path_ex_t;
static uint32_t
__ex_xorshift32(int *state)
{
    uint32_t x = *state;
    if (x == 0)
        x = 123456789;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
}
static int
__skip_compare_entries_fn_ex(ex_t *list, ex_node_t *a, ex_node_t *b, void *aux)
{ /* compare entries: list, a, b, aux */
    {
        (void)list;
        (void)aux;
        if (a->key < b->key)
            return -1;
        if (a->key > b->key)
            return 1;
        return 0;
    };
}
static void
__skip_free_entry_fn_ex(ex_node_t *node)
{ /* free entry: node */
    {
        free(node->value);
        node->value = ((void *)0);
    };
}
static int
__skip_update_entry_fn_ex(ex_node_t *node, void *value)
{
    int rc = 0; /* update entry: rc, node, value */
    {
        char *numeral = (char *)value;
        if (node->value)
            free(node->value);
        node->value = numeral;
    };
    return rc;
}
static int
__skip_archive_entry_fn_ex(ex_node_t *dest, const ex_node_t *src)
{
    int rc = 0; /* archive an entry: rc, src, dest */
    {
        dest->key = src->key;
        char *nv = calloc(strlen(src->value) + 1, sizeof(char));
        if (nv == ((void *)0))
            rc = 12;
        else {
            strncpy(nv, src->value, strlen(src->value));
            dest->value = nv;
        }
    };
    return rc;
}
static size_t
__skip_sizeof_entry_fn_ex(ex_node_t *node)
{
    size_t bytes = 0; /* size in bytes of the content stored in an entry: bytes */
    {
        bytes = strlen(node->value) + 1;
    };
    return bytes;
}
static int
__skip_compare_nodes_ex(ex_t *slist, ex_node_t *a, ex_node_t *b, void *aux)
{
    if (a == b)
        return 0;
    if (a == ((void *)0))
        return -1;
    if (b == ((void *)0))
        return 1;
    if (a == slist->slh_head || b == slist->slh_tail)
        return -1;
    if (a == slist->slh_tail || b == slist->slh_head)
        return 1;
    return slist->slh_fns.compare_entries(slist, a, b, aux);
}
static int
__skip_toss_ex(ex_t *slist, size_t max)
{
    size_t level = 0;
    double probability = 0.5;
    double random_value = (double)__ex_xorshift32(&slist->slh_prng_state) / 2147483647;
    while (random_value < probability && level < max) {
        level++;
        probability *= 0.5;
    }
    return level;
}
int
api_skip_alloc_node_ex(ex_node_t **node)
{
    ex_node_t *n;
    size_t sle_arr_sz = sizeof(struct __skiplist_ex_level) * 64;
    n = (ex_node_t *)calloc(1, sizeof(ex_node_t) + sle_arr_sz);
    if (n == ((void *)0))
        return 12;
    n->entries.sle_height = 0;
    n->entries.sle_levels = (struct __skiplist_ex_level *)((uintptr_t)n + sizeof(ex_node_t));
    *node = n;
    return 0;
}
int
api_skip_init_ex(ex_t *slist)
{
    int rc = 0;
    size_t i;
    slist->slh_length = 0;
    slist->slh_snap.cur_era = 0;
    slist->slh_snap.pres_era = 0;
    slist->slh_snap.pres = 0;
    slist->slh_fns.free_entry = __skip_free_entry_fn_ex;
    slist->slh_fns.update_entry = __skip_update_entry_fn_ex;
    slist->slh_fns.archive_entry = __skip_archive_entry_fn_ex;
    slist->slh_fns.sizeof_entry = __skip_sizeof_entry_fn_ex;
    slist->slh_fns.compare_entries = __skip_compare_entries_fn_ex;
    rc = api_skip_alloc_node_ex(&slist->slh_head);
    if (rc)
        goto fail;
    rc = api_skip_alloc_node_ex(&slist->slh_tail);
    if (rc)
        goto fail;
    slist->slh_head->entries.sle_height = 1;
    for (i = 0; i < slist->slh_head->entries.sle_height + 1; i++)
        slist->slh_head->entries.sle_levels[i].next = slist->slh_tail;
    slist->slh_head->entries.sle_prev = ((void *)0);
    slist->slh_tail->entries.sle_height = slist->slh_head->entries.sle_height;
    for (i = 0; i < slist->slh_head->entries.sle_height + 1; i++)
        slist->slh_tail->entries.sle_levels[i].next = ((void *)0);
    slist->slh_tail->entries.sle_prev = slist->slh_head;
    slist->slh_prng_state = ((unsigned int)time(((void *)0)) ^ getpid());
fail:;
    return rc;
}
void
api_skip_free_node_ex(ex_t *slist, ex_node_t *node)
{
    slist->slh_fns.free_entry(node);
    free(node);
}
size_t
api_skip_length_ex(ex_t *slist)
{
    return slist->slh_length;
}
int
api_skip_is_empty_ex(ex_t *slist)
{
    return slist->slh_length == 0;
}
ex_node_t *
api_skip_head_ex(ex_t *slist)
{
    return slist->slh_head->entries.sle_levels[0].next == slist->slh_tail ? ((void *)0) : slist->slh_head->entries.sle_levels[0].next;
}
ex_node_t *
api_skip_tail_ex(ex_t *slist)
{
    if (slist == ((void *)0))
        return ((void *)0);
    return slist->slh_tail->entries.sle_prev == slist->slh_head->entries.sle_levels[0].next ? ((void *)0) : slist->slh_tail->entries.sle_prev;
}
ex_node_t *
api_skip_next_node_ex(ex_t *slist, ex_node_t *n)
{
    if (slist == ((void *)0) || n == ((void *)0))
        return ((void *)0);
    if (n->entries.sle_levels[0].next == slist->slh_tail)
        return ((void *)0);
    return n->entries.sle_levels[0].next;
}
ex_node_t *
api_skip_prev_node_ex(ex_t *slist, ex_node_t *n)
{
    if (slist == ((void *)0) || n == ((void *)0))
        return ((void *)0);
    if (n->entries.sle_prev == slist->slh_head)
        return ((void *)0);
    return n->entries.sle_prev;
}
void
api_skip_release_ex(ex_t *slist)
{
    ex_node_t *node, *next;
    if (slist == ((void *)0))
        return;
    if (api_skip_is_empty_ex(slist))
        return;
    node = slist->slh_head->entries.sle_levels[0].next;
    while (node != slist->slh_tail) {
        next = node->entries.sle_levels[0].next;
        api_skip_free_node_ex(slist, node);
        node = next;
    }
    if (slist->slh_snap.pres_era > 0)
        slist->slh_snap.cur_era++;
    return;
}
ex_node_t **
api_skip_to_array_ex(ex_t *slist)
{
    size_t nth, len = api_skip_length_ex(slist);
    ex_node_t *node, **nodes = ((void *)0);
    nodes = (ex_node_t **)calloc(sizeof(ex_node_t *), len + 1);
    if (nodes != ((void *)0)) {
        nodes[0] = (ex_node_t *)(uintptr_t)len;
        nodes++;
        for (nth = 0, (node) = (slist)->slh_head; ((node) = (node)->entries.sle_levels[0].next) != (slist)->slh_tail; nth++) {
            nodes[nth] = node;
        }
    }
    return nodes;
}
static void
__skip_adjust_hit_counts_ex(ex_t *slist)
{
    ((void)slist);
}
static void
__skip_rebalance_ex(ex_t *slist, size_t len, __skiplist_path_ex_t path[])
{
    size_t i, lvl, u_hits, hits_CHu = 0, hits_CHv = 0, delta_height;
    size_t k_threshold, m_total_hits, expected_height;
    double asc_cond, dsc_cond;
    __skiplist_path_ex_t *p, path_u, path_v;
    ex_node_t *node;
    m_total_hits = slist->slh_head->entries.sle_levels[slist->slh_head->entries.sle_height].hits;
    if (m_total_hits > 0) {
        expected_height = floor(log(m_total_hits) / log(2));
        if (expected_height > slist->slh_head->entries.sle_height && expected_height < 64 - 1) {
            slist->slh_head->entries.sle_height++;
            slist->slh_head->entries.sle_levels[slist->slh_head->entries.sle_height].next = slist->slh_tail;
            slist->slh_head->entries.sle_levels[slist->slh_head->entries.sle_height].hits =
                slist->slh_head->entries.sle_levels[slist->slh_head->entries.sle_height - 1].hits;
            slist->slh_head->entries.sle_levels[slist->slh_head->entries.sle_height - 1].hits = 0;
            slist->slh_tail->entries.sle_height = slist->slh_head->entries.sle_height;
        }
    }
    k_threshold = slist->slh_head->entries.sle_height;
    p = &path[0];
    for (i = 0; i < len; i++, p++) {
        hits_CHu = hits_CHv = 0;
        path_u = *p;
        if (path_u.node == slist->slh_head || path_u.node == slist->slh_tail)
            continue;
        path_v = *(p + 1);
        for (ex_node_t *elm = path_u.node->entries.sle_levels[path_u.in].next->entries.sle_prev; elm != path_u.node->entries.sle_prev;
             elm = elm->entries.sle_prev) {
            hits_CHu += elm->entries.sle_levels[0].hits;
        }
        for (ex_node_t *elm = (*(p + 1)).node->entries.sle_levels[path_u.in].next->entries.sle_levels[path_u.in].next->entries.sle_prev;
             elm != (*(p + 1)).node->entries.sle_levels[path_u.in].next->entries.sle_prev; elm = elm->entries.sle_prev) {
            hits_CHv += elm->entries.sle_levels[0].hits;
        }
        delta_height = k_threshold - path_u.node->entries.sle_height;
        dsc_cond = m_total_hits / pow(2.0, delta_height);
        u_hits = hits_CHu + hits_CHv;
        if (u_hits <= dsc_cond && path_u.node->entries.sle_height > 0) {
            lvl = path_u.node->entries.sle_height;
            node = path_u.node;
            do {
                node = node->entries.sle_prev;
                if (node->entries.sle_height >= lvl && node->entries.sle_levels[lvl].next == path_u.node) {
                    node->entries.sle_levels[lvl].next = path_u.node->entries.sle_levels[lvl].next;
                    node->entries.sle_levels[lvl].hits += 1;
                    break;
                }
            } while (node != slist->slh_head);
            path_u.node->entries.sle_height--;
        }
        asc_cond = m_total_hits / pow(2.0, delta_height - 1);
        if (path_u.pu > asc_cond && path_u.node->entries.sle_height < 64 - 1) {
            if (path_u.node->entries.sle_height + 1 > path_v.node->entries.sle_height) {
                continue;
            }
            lvl = ++path_u.node->entries.sle_height;
            if (path_v.node != slist->slh_head) {
                path_u.node->entries.sle_levels[lvl].hits = path_v.node->entries.sle_levels[lvl].hits;
                path_v.node->entries.sle_levels[lvl].hits = 0;
            }
            path_u.node->entries.sle_levels[lvl].next = path_v.node->entries.sle_levels[lvl].next;
            path_v.node->entries.sle_levels[lvl].next = path_u.node;
        }
    }
}
static size_t
__skip_locate_ex(ex_t *slist, ex_node_t *n, __skiplist_path_ex_t path[])
{
    unsigned int i;
    size_t len = 0;
    ex_node_t *elm;
    __skiplist_path_ex_t *cur;
    if (slist == ((void *)0) || n == ((void *)0))
        return 0;
    elm = slist->slh_head;
    i = slist->slh_head->entries.sle_height - 1;
    do {
        cur = &path[i + 1];
        cur->pu = 0;
        while (elm != slist->slh_tail && __skip_compare_nodes_ex(slist, elm->entries.sle_levels[i].next, n, slist->slh_aux) < 0) {
            cur->in = i;
            elm = elm->entries.sle_levels[i].next;
        }
        cur->node = elm;
        cur->node->entries.sle_levels[cur->in].hits += i ? 1 : 0;
        cur->pu += cur->node->entries.sle_levels[0].hits;
        len++;
    } while (i--);
    elm = elm->entries.sle_levels[0].next;
    if (__skip_compare_nodes_ex(slist, elm, n, slist->slh_aux) == 0) {
        path[0].node = elm;
        path[0].node->entries.sle_levels[0].hits++;
        cur->pu += path[0].node->entries.sle_levels[0].hits;
        slist->slh_head->entries.sle_levels[slist->slh_head->entries.sle_height].hits++;
        __skip_rebalance_ex(slist, len, path);
    }
    return len;
}
static int
__skip_insert_ex(ex_t *slist, ex_node_t *new, int flags)
{
    static __skiplist_path_ex_t apath[64 + 1];
    int rc = 0;
    size_t i, len, loc = 0, current_height, new_height;
    ex_node_t *node;
    __skiplist_path_ex_t *path = apath;
    if (slist == ((void *)0) || new == ((void *)0))
        return 2;
    memset(path, 0, sizeof(__skiplist_path_ex_t) * 64 + 1);
    len = __skip_locate_ex(slist, new, path);
    node = path[0].node;
    if (len > 0) {
        if ((node != ((void *)0)) && (flags == 0)) {
            return -1;
        }
        current_height = slist->slh_head->entries.sle_height - 1;
        new_height = __skip_toss_ex(slist, current_height);
        new->entries.sle_height = new_height;
        for (i = current_height + 1; i <= new_height; i++) {
            path[i + 1].node = slist->slh_tail;
        }
        for (size_t lvl = 0; lvl < slist->slh_head->entries.sle_height - 1; lvl++) {
            new->entries.sle_levels[lvl].next = slist->slh_tail;
        }
        for (i = 0; i <= new_height; i++) {
            if (path[i + 1].node != slist->slh_tail) {
                new->entries.sle_levels[i].next = path[i + 1].node->entries.sle_levels[i].next;
                path[i + 1].node->entries.sle_levels[i].next = new;
                loc = path[i + 1].node == slist->slh_head ? i : loc;
            } else {
                new->entries.sle_levels[i].next = slist->slh_tail;
            }
        }
        new->entries.sle_prev = path[1].node;
        new->entries.sle_levels[0].next->entries.sle_prev = new;
        if (new->entries.sle_levels[0].next == slist->slh_tail)
            slist->slh_tail->entries.sle_prev = new;
        if (slist->slh_snap.pres_era > 0) {
            new->entries.sle_era = slist->slh_snap.cur_era++;
        }
        new->entries.sle_levels[new_height].hits = 1;
        slist->slh_length++;
        if (64 == 1)
            free(path);
    }
    return rc;
}
int
api_skip_insert_ex(ex_t *slist, ex_node_t *n)
{
    return __skip_insert_ex(slist, n, 0);
}
int
api_skip_insert_dup_ex(ex_t *slist, ex_node_t *n)
{
    return __skip_insert_ex(slist, n, 1);
}
ex_node_t *
api_skip_position_eq_ex(ex_t *slist, ex_node_t *query)
{
    static __skiplist_path_ex_t apath[64 + 1];
    ex_node_t *node = ((void *)0);
    __skiplist_path_ex_t *path = apath;
    memset(path, 0, sizeof(__skiplist_path_ex_t) * 64 + 1);
    __skip_locate_ex(slist, query, path);
    node = path[0].node;
    if (64 == 1)
        free(path);
    return node;
}
ex_node_t *
api_skip_position_gte_ex(ex_t *slist, ex_node_t *query)
{
    static __skiplist_path_ex_t apath[64 + 1];
    int cmp;
    ex_node_t *node;
    __skiplist_path_ex_t *path = apath;
    memset(path, 0, sizeof(__skiplist_path_ex_t) * 64 + 1);
    __skip_locate_ex(slist, query, path);
    node = path[1].node;
    do {
        node = node->entries.sle_levels[0].next;
        cmp = __skip_compare_nodes_ex(slist, node, query, slist->slh_aux);
    } while (cmp < 0);
    if (64 == 1)
        free(path);
    return node;
}
ex_node_t *
api_skip_position_gt_ex(ex_t *slist, ex_node_t *query)
{
    static __skiplist_path_ex_t apath[64 + 1];
    int cmp;
    ex_node_t *node;
    __skiplist_path_ex_t *path = apath;
    memset(path, 0, sizeof(__skiplist_path_ex_t) * 64 + 1);
    __skip_locate_ex(slist, query, path);
    node = path[1].node;
    if (node == slist->slh_tail)
        goto done;
    do {
        node = node->entries.sle_levels[0].next;
        cmp = __skip_compare_nodes_ex(slist, node, query, slist->slh_aux);
    } while (cmp <= 0 && node != slist->slh_tail);
done:;
    if (64 == 1)
        free(path);
    return node;
}
ex_node_t *
api_skip_position_lte_ex(ex_t *slist, ex_node_t *query)
{
    static __skiplist_path_ex_t apath[64 + 1];
    ex_node_t *node;
    __skiplist_path_ex_t *path = apath;
    memset(path, 0, sizeof(__skiplist_path_ex_t) * 64 + 1);
    __skip_locate_ex(slist, query, path);
    node = path[0].node;
    if (node)
        goto done;
    node = path[1].node;
done:;
    if (64 == 1)
        free(path);
    return node;
}
ex_node_t *
api_skip_position_lt_ex(ex_t *slist, ex_node_t *query)
{
    static __skiplist_path_ex_t apath[64 + 1];
    ex_node_t *node;
    __skiplist_path_ex_t *path = apath;
    memset(path, 0, sizeof(__skiplist_path_ex_t) * 64 + 1);
    __skip_locate_ex(slist, query, path);
    node = path[1].node;
    if (64 == 1)
        free(path);
    return node;
}
ex_node_t *
api_skip_position_ex(ex_t *slist, skip_pos_decl_t op, ex_node_t *query)
{
    ex_node_t *node;
    switch (op) {
    case (SKIP_LT):
        node = api_skip_position_lt_ex(slist, query);
        break;
    case (SKIP_LTE):
        node = api_skip_position_lte_ex(slist, query);
        break;
    case (SKIP_GTE):
        node = api_skip_position_gte_ex(slist, query);
        break;
    case (SKIP_GT):
        node = api_skip_position_gt_ex(slist, query);
        break;
    default:
    case (SKIP_EQ):
        node = api_skip_position_eq_ex(slist, query);
        break;
    }
    return node;
}
int
api_skip_update_ex(ex_t *slist, ex_node_t *query, void *value)
{
    static __skiplist_path_ex_t apath[64 + 1];
    int rc = 0, np;
    ex_node_t *node;
    __skiplist_path_ex_t *path = apath;
    if (slist == ((void *)0))
        return -1;
    memset(path, 0, sizeof(__skiplist_path_ex_t) * 64 + 1);
    __skip_locate_ex(slist, query, path);
    node = path[0].node;
    if (64 == 1)
        free(path);
    if (node == ((void *)0))
        return -1;
    if (slist->slh_snap.pres_era > 0) {
        np = slist->slh_fns.snapshot_preserve_node(slist, node, ((void *)0));
        if (np > 0)
            return np;
        slist->slh_snap.cur_era++;
    }
    slist->slh_fns.update_entry(node, value);
    return rc;
}
int
api_skip_remove_node_ex(ex_t *slist, ex_node_t *query)
{
    static __skiplist_path_ex_t apath[64 + 1];
    int np = 0;
    size_t i, len, height;
    ex_node_t *node;
    __skiplist_path_ex_t *path = apath;
    if (slist == ((void *)0) || query == ((void *)0))
        return -1;
    if (slist->slh_length == 0)
        return 0;
    memset(path, 0, sizeof(__skiplist_path_ex_t) * 64 + 1);
    len = __skip_locate_ex(slist, query, path);
    node = path[0].node;
    if (node) {
        if (slist->slh_snap.pres_era > 0) {
            np = slist->slh_fns.snapshot_preserve_node(slist, node, ((void *)0));
            if (np > 0)
                return np;
            slist->slh_snap.cur_era++;
        }
        node->entries.sle_levels[0].next->entries.sle_prev = node->entries.sle_prev;
        for (i = 0; i < len; i++) {
            if (path[i + 1].node->entries.sle_levels[i].next != node)
                break;
            path[i + 1].node->entries.sle_levels[i].next = node->entries.sle_levels[i].next;
            if (path[i + 1].node->entries.sle_levels[i].next == slist->slh_tail) {
                height = path[i + 1].node->entries.sle_height;
                path[i + 1].node->entries.sle_height = height - 1;
            }
        }
        if (node->entries.sle_levels[0].next == slist->slh_tail) {
            slist->slh_tail->entries.sle_prev = query->entries.sle_prev;
        }
        if (64 == 1)
            free(path);
        slist->slh_fns.free_entry(node);
        free(node);
        for (i = 0; slist->slh_head->entries.sle_levels[i].next != slist->slh_tail && i < 64; i++)
            ;
        slist->slh_head->entries.sle_levels[i].hits = slist->slh_head->entries.sle_levels[slist->slh_head->entries.sle_height].hits;
        slist->slh_head->entries.sle_height = slist->slh_tail->entries.sle_height = i;
        slist->slh_length--;
        __skip_adjust_hit_counts_ex(slist);
    }
    return 0;
}
void
api_skip_free_ex(ex_t *slist)
{
    if (slist == ((void *)0))
        return;
    if (slist->slh_snap.pres_era > 0 && slist->slh_fns.snapshot_release)
        slist->slh_fns.snapshot_release(slist);
    api_skip_release_ex(slist);
    free(slist->slh_head);
    free(slist->slh_tail);
}
/*
 * Skiplists are ordered, we need a way to compare entries.
 * Let's create a function with four arguments:
 *   - a reference to the Skiplist, `slist`
 *   - the two nodes to compare, `a` and `b`
 *   - and `aux`, which you can use to pass into this function
 *     any additional information required to compare objects.
 *     `aux` is passed from the value in the Skiplist, you can
 *     modify that value at any time to suit your needs.
 *
 * Your function should result in a return statement:
 *   a  < b : return -1
 *   a == b : return 0
 *   a  > b : return 1
 *
 * This result provides the ordering within the Skiplist.  Sometimes
 * your function will not be used when comparing nodes.  This will
 * happen when `a` or `b` are references to the head or tail of the
 * list or when `a == b`.  In those cases the comparison function
 * returns before using the code in your block, don't panic. :)
 */
int
__ex_key_compare(ex_t *list, ex_node_t *a, ex_node_t *b, void *aux)
{
    (void)list;
    (void)aux;
    if (a->key < b->key)
        return -1;
    if (a->key > b->key)
        return 1;
    return 0;
}

/*
 * Optional: Getters and Setters
 *  - get, put, dup(put), del, etc. functions
 *
 * It can be useful to have simple get/put-style API, but to
 * do that you'll have to supply some blocks of code used to
 * extract data from within your nodes.
 */
char *
api_skip_get_ex(ex_t *slist, int key)
{
    ex_node_t *node, query; /* query blk */
    {
        query.key = key;
    };
    node = api_skip_position_eq_ex(slist, &query);
    if (node) { /* return blk */
        {
            return node->value;
        };
    }
    return (char *)0;
}
int
api_skip_contains_ex(ex_t *slist, int key)
{
    ex_node_t *node, query; /* query blk */
    {
        query.key = key;
    };
    node = api_skip_position_eq_ex(slist, &query);
    if (node)
        return 1;
    return 0;
}
ex_node_t *
api_skip_pos_ex(ex_t *slist, skip_pos_decl_t op, int key)
{
    ex_node_t *node, query; /* query blk */
    {
        query.key = key;
    };
    node = api_skip_position_ex(slist, op, &query);
    if (node != slist->slh_head && node != slist->slh_tail)
        return node;
    return ((void *)0);
}
int
api_skip_put_ex(ex_t *slist, int key, char *value)
{
    int rc;
    ex_node_t *node;
    rc = api_skip_alloc_node_ex(&node);
    if (rc)
        return rc;
    node->key = key;
    node->value = value;
    rc = api_skip_insert_ex(slist, node);
    if (rc)
        api_skip_free_node_ex(slist, node);
    return rc;
}
int
api_skip_dup_ex(ex_t *slist, int key, char *value)
{
    int rc;
    ex_node_t *node;
    rc = api_skip_alloc_node_ex(&node);
    if (rc)
        return rc;
    node->key = key;
    node->value = value;
    rc = api_skip_insert_dup_ex(slist, node);
    if (rc)
        api_skip_free_node_ex(slist, node);
    return rc;
}
int
api_skip_set_ex(ex_t *slist, int key, char *value)
{
    ex_node_t node;
    node.key = key;
    return api_skip_update_ex(slist, &node, (void *)value);
}
int
api_skip_del_ex(ex_t *slist, int key)
{
    ex_node_t node;
    node.key = key;
    return api_skip_remove_node_ex(slist, &node);
}

/*
 * Optional: Snapshots
 *
 * Enable functions that enable returning to an earlier point in
 * time when a snapshot was created.
 */
uint64_t
api_skip_snapshot_ex(ex_t *slist)
{
    if (slist == ((void *)0))
        return 0;
    return ++slist->slh_snap.pres_era;
}
void
api_skip_release_snapshots_ex(ex_t *slist)
{
    ex_node_t *node, *next;
    if (slist == ((void *)0))
        return;
    if (slist->slh_snap.pres_era == 0)
        return;
    node = slist->slh_snap.pres;
    while (node) {
        next = node->entries.sle_levels[0].next;
        api_skip_free_node_ex(slist, node);
        node = next;
    }
    slist->slh_snap.pres = ((void *)0);
    slist->slh_snap.pres_era = 0;
}
static int
__skip_preserve_node_ex(ex_t *slist, const ex_node_t *src, ex_node_t **preserved)
{
    int rc = 0;
    ex_node_t *dest, *is_dup = 0;
    if (slist == ((void *)0) || src == ((void *)0))
        return 0;
    if (src == slist->slh_head || src == slist->slh_tail)
        return 0;
    if (src->entries.sle_era > slist->slh_snap.pres_era)
        return 0;
    size_t sle_arr_sz = sizeof(struct __skiplist_ex_level) * slist->slh_head->entries.sle_height - 1;
    rc = api_skip_alloc_node_ex(&dest);
    if (rc)
        return rc;
    memcpy(dest, src, sizeof(ex_node_t) + sle_arr_sz);
    dest->entries.sle_levels = (struct __skiplist_ex_level *)((uintptr_t)dest + sizeof(ex_node_t));
    slist->slh_fns.archive_entry(dest, src);
    if (rc) {
        api_skip_free_node_ex(slist, dest);
        return rc;
    }
    if (__skip_compare_nodes_ex(slist, dest, dest->entries.sle_levels[0].next, slist->slh_aux) == 0 ||
        __skip_compare_nodes_ex(slist, dest, dest->entries.sle_prev, slist->slh_aux) == 0)
        is_dup = (ex_node_t *)0x1;
    dest->entries.sle_prev = ((void *)0);
    for (size_t lvl = 0; lvl < slist->slh_head->entries.sle_height - 1; lvl++) {
        dest->entries.sle_levels[lvl].next = ((void *)0);
    }
    dest->entries.sle_levels[1].next = is_dup;
    if (slist->slh_snap.pres == ((void *)0)) {
        dest->entries.sle_levels[0].next = ((void *)0);
        slist->slh_snap.pres = dest;
    } else {
        dest->entries.sle_levels[0].next = slist->slh_snap.pres;
        slist->slh_snap.pres = dest;
    }
    if (preserved)
        *preserved = dest;
    rc = 1;
    return -rc;
}
ex_t *
api_skip_restore_snapshot_ex(ex_t *slist, size_t era)
{
    size_t i, cur_era;
    ex_node_t *node, *prev;
    if (slist == ((void *)0))
        return ((void *)0);
    if (slist->slh_snap.pres_era == 0)
        return ((void *)0);
    if (era >= slist->slh_snap.cur_era || slist->slh_snap.pres == ((void *)0))
        return slist;
    cur_era = slist->slh_snap.cur_era;
    for (i = 0, (node) = (slist)->slh_head; ((node) = (node)->entries.sle_levels[0].next) != (slist)->slh_tail; i++) {
        ((void)i);
        if (node->entries.sle_era > era) {
            api_skip_remove_node_ex(slist, node);
        }
    }
    prev = ((void *)0);
    node = slist->slh_snap.pres;
    while (node) {
        if (node->entries.sle_era > era) {
            if (slist->slh_snap.pres == node)
                slist->slh_snap.pres = node->entries.sle_levels[0].next;
            else {
                if (node->entries.sle_levels[0].next == ((void *)0))
                    if (node == slist->slh_snap.pres)
                        slist->slh_snap.pres = ((void *)0);
                    else
                        prev->entries.sle_levels[0].next = ((void *)0);
                else
                    prev->entries.sle_levels[0].next = node->entries.sle_levels[0].next;
            }
            api_skip_free_node_ex(slist, node);
        }
        prev = ((void *)0);
        if (node->entries.sle_era == era) {
            if (slist->slh_snap.pres == node)
                slist->slh_snap.pres = node->entries.sle_levels[0].next;
            else {
                if (node->entries.sle_levels[0].next == ((void *)0))
                    if (node == slist->slh_snap.pres)
                        slist->slh_snap.pres = ((void *)0);
                    else
                        prev->entries.sle_levels[0].next = ((void *)0);
                else
                    prev->entries.sle_levels[0].next = node->entries.sle_levels[0].next;
            }
            node->entries.sle_prev = ((void *)0);
            if (node->entries.sle_levels[1].next != 0) {
                node->entries.sle_levels[1].next = ((void *)0);
                api_skip_insert_dup_ex(slist, node);
            } else {
                api_skip_insert_ex(slist, node);
            }
        }
        prev = node;
        node = node->entries.sle_levels[0].next;
    }
    slist->slh_snap.pres_era = slist->slh_snap.pres == ((void *)0) ? 0 : cur_era;
    return slist;
}
void
api_skip_snapshots_init_ex(ex_t *slist)
{
    if (slist != ((void *)0)) {
        slist->slh_fns.snapshot_preserve_node = __skip_preserve_node_ex;
        slist->slh_fns.snapshot_release = api_skip_release_snapshots_ex;
    }
}

/*
 * Optional: Archive to/from bytes
 *
 * Enable functions that can write/read the content of your Skiplist
 * out/in to/from an array of bytes.
 */
// TODO SKIPLIST_DECL_ARCHIVE(ex, api_, entries)

/*
 * Optional: As Hashtable
 *
 * Turn your Skiplist into a hash table.
 */
// TODO SKIPLIST_DECL_HASHTABLE(ex, api_, entries, snaps)

/*
 * Optional: Check Skiplists at runtime
 *
 * Create a functions that validate the integrity of a Skiplist.
 */
static void __attribute__((format(printf, 1, 2))) __skip_integrity_failure_ex(const char *format, ...)
{
    va_list args;
    __builtin_va_start(args, format);
    vfprintf(stderr, format, args);
    __builtin_va_end(args);
}
static int
__skip_integrity_check_ex(ex_t *slist, int flags)
{
    size_t n = 0;
    unsigned long nth, n_err = 0;
    ex_node_t *node, *prev, *next;
    struct __skiplist_ex_entry *this;
    if (slist == ((void *)0)) {
        __skip_integrity_failure_ex("slist was NULL, nothing to check\n");
        n_err++;
        return n_err;
    }
    if (slist->slh_head == ((void *)0)) {
        __skip_integrity_failure_ex("skiplist slh_head is NULL\n");
        n_err++;
        return n_err;
    }
    if (slist->slh_tail == ((void *)0)) {
        __skip_integrity_failure_ex("skiplist slh_tail is NULL\n");
        n_err++;
        return n_err;
    }
    if (slist->slh_fns.free_entry == ((void *)0)) {
        __skip_integrity_failure_ex("skiplist free_entry fn is NULL\n");
        n_err++;
        return n_err;
    }
    if (slist->slh_fns.update_entry == ((void *)0)) {
        __skip_integrity_failure_ex("skiplist update_entry fn is NULL\n");
        n_err++;
        return n_err;
    }
    if (slist->slh_fns.archive_entry == ((void *)0)) {
        __skip_integrity_failure_ex("skiplist archive_entry fn is NULL\n");
        n_err++;
        return n_err;
    }
    if (slist->slh_fns.sizeof_entry == ((void *)0)) {
        __skip_integrity_failure_ex("skiplist sizeof_entry fn is NULL\n");
        n_err++;
        return n_err;
    }
    if (slist->slh_fns.compare_entries == ((void *)0)) {
        __skip_integrity_failure_ex("skiplist compare_entries fn is NULL\n");
        n_err++;
        return n_err;
    }
    if (slist->slh_head->entries.sle_height > 64) {
        __skip_integrity_failure_ex("skiplist head height > SKIPLIST_MAX_HEIGHT\n");
        n_err++;
        if (flags)
            return n_err;
    }
    if (slist->slh_tail->entries.sle_height > 64) {
        __skip_integrity_failure_ex("skiplist tail height > SKIPLIST_MAX_HEIGHT\n");
        n_err++;
        if (flags)
            return n_err;
    }
    if (slist->slh_head->entries.sle_height != slist->slh_tail->entries.sle_height) {
        __skip_integrity_failure_ex("skiplist head & tail height are not equal\n");
        n_err++;
        if (flags)
            return n_err;
    }
    if (64 < 1) {
        __skip_integrity_failure_ex("SKIPLIST_MAX_HEIGHT cannot be less than 1\n");
        n_err++;
        if (flags)
            return n_err;
    }
    node = slist->slh_head;
    for (size_t lvl = 0; lvl <= node->entries.sle_height; lvl++) {
        if (node->entries.sle_levels[lvl].next == ((void *)0)) {
            __skip_integrity_failure_ex("the head's %lu next node should not be NULL\n", lvl);
            n_err++;
            if (flags)
                return n_err;
        }
        n = lvl;
        if (node->entries.sle_levels[lvl].next == slist->slh_tail)
            break;
    }
    n++;
    for (size_t lvl = n; lvl <= node->entries.sle_height; lvl++) {
        if (node->entries.sle_levels[lvl].next == ((void *)0)) {
            __skip_integrity_failure_ex("the head's %lu next node should not be NULL\n", lvl);
            n_err++;
            if (flags)
                return n_err;
        }
    }
    if (slist->slh_length > 0 && slist->slh_tail->entries.sle_prev == slist->slh_head) {
        __skip_integrity_failure_ex("slist->slh_length is 0, but tail->prev == head, not an internal node\n");
        n_err++;
        if (flags)
            return n_err;
    }
    for (nth = 0, (node) = (slist)->slh_head; ((node) = (node)->entries.sle_levels[0].next) != (slist)->slh_tail; nth++) {
        this = &node->entries;
        if (this->sle_height > slist->slh_head->entries.sle_height) {
            __skip_integrity_failure_ex("the %lu node's [%p] height %lu is > head %lu\n", nth, (void *)node, this->sle_height,
                slist->slh_head->entries.sle_height);
            n_err++;
            if (flags)
                return n_err;
        }
        if (this->sle_levels == ((void *)0)) {
            __skip_integrity_failure_ex("the %lu node's [%p] next field should never be NULL\n", nth, (void *)node);
            n_err++;
            if (flags)
                return n_err;
        }
        if (this->sle_prev == ((void *)0)) {
            __skip_integrity_failure_ex("the %lu node [%p] prev field should never be NULL\n", nth, (void *)node);
            n_err++;
            if (flags)
                return n_err;
        }
        for (size_t lvl = 0; lvl <= node->entries.sle_height; lvl++) {
            if (this->sle_levels[lvl].next == ((void *)0)) {
                __skip_integrity_failure_ex("the %lu node's next[%lu] should not be NULL\n", nth, lvl);
                n_err++;
                if (flags)
                    return n_err;
            }
            n = lvl;
            if (this->sle_levels[lvl].next == slist->slh_tail)
                break;
        }
        n++;
        for (size_t lvl = n; lvl <= node->entries.sle_height; lvl++) {
            if (this->sle_levels[lvl].next == ((void *)0)) {
                __skip_integrity_failure_ex("after the %lunth the %lu node's next[%lu] should not be NULL\n", n, nth, lvl);
                n_err++;
                if (flags)
                    return n_err;
            } else if (this->sle_levels[lvl].next != slist->slh_tail) {
                __skip_integrity_failure_ex("after the %lunth the %lu node's next[%lu] should point to the tail\n", n, nth, lvl);
                n_err++;
                if (flags)
                    return n_err;
            }
        }
        ex_node_t *a = (ex_node_t *)(uintptr_t)this->sle_levels;
        ex_node_t *b = (ex_node_t *)(intptr_t)((uintptr_t)node + sizeof(ex_node_t));
        if (a != b) {
            __skip_integrity_failure_ex("the %lu node's [%p] next field isn't at the proper offset relative to the node\n", nth, (void *)node);
            n_err++;
            if (flags)
                return n_err;
        }
        next = this->sle_levels[0].next;
        prev = this->sle_prev;
        if (__skip_compare_nodes_ex(slist, node, node, slist->slh_aux) != 0) {
            __skip_integrity_failure_ex("the %lu node [%p] is not equal to itself\n", nth, (void *)node);
            n_err++;
            if (flags)
                return n_err;
        }
        if (__skip_compare_nodes_ex(slist, node, prev, slist->slh_aux) < 0) {
            __skip_integrity_failure_ex("the %lu node [%p] is not greater than the prev node [%p]\n", nth, (void *)node, (void *)prev);
            n_err++;
            if (flags)
                return n_err;
        }
        if (__skip_compare_nodes_ex(slist, node, next, slist->slh_aux) > 0) {
            __skip_integrity_failure_ex("the %lu node [%p] is not less than the next node [%p]\n", nth, (void *)node, (void *)next);
            n_err++;
            if (flags)
                return n_err;
        }
        if (__skip_compare_nodes_ex(slist, prev, node, slist->slh_aux) > 0) {
            __skip_integrity_failure_ex("the prev node [%p] is not less than the %lu node [%p]\n", (void *)prev, nth, (void *)node);
            n_err++;
            if (flags)
                return n_err;
        }
        if (__skip_compare_nodes_ex(slist, next, node, slist->slh_aux) < 0) {
            __skip_integrity_failure_ex("the next node [%p] is not greater than the %lu node [%p]\n", (void *)next, nth, (void *)node);
            n_err++;
            if (flags)
                return n_err;
        }
    }
    if (slist->slh_length != nth) {
        __skip_integrity_failure_ex("slist->slh_length (%lu) doesn't match the count (%lu) of nodes between the head and tail\n", slist->slh_length, nth);
        n_err++;
        if (flags)
            return n_err;
    }
    return 0;
}

/* Optional: Visualize your Skiplist using DOT/Graphviz in PDF
 *
 * Create the functions used to annotate a visualization of a Skiplist.
 */
typedef void (*skip_sprintf_node_ex_t)(ex_node_t *, char *);
static size_t
__skip_dot_width_ex(ex_t *slist, ex_node_t *from, ex_node_t *to)
{
    size_t w = 1;
    ex_node_t *n = to;
    if (from == ((void *)0) || to == ((void *)0))
        return 0;
    while (n->entries.sle_prev != from) {
        w++;
        n = api_skip_prev_node_ex(slist, n);
    }
    return w;
}
static inline void
__skip_dot_write_node_ex(FILE *os, size_t nsg, ex_node_t *node)
{
    if (node)
        fprintf(os, "\"node%lu %p\"", nsg, (void *)node);
    else
        fprintf(os, "\"node%lu NULL\"", nsg);
}
static void
__skip_dot_node_ex(FILE *os, ex_t *slist, ex_node_t *node, size_t nsg, skip_sprintf_node_ex_t fn)
{
    char buf[2048];
    ex_node_t *next;
    __skip_dot_write_node_ex(os, nsg, node);
    fprintf(os, " [label = \"");
    fflush(os);
    for (size_t lvl = node->entries.sle_height; lvl != (size_t)-1; lvl--) {
        next = (node->entries.sle_levels[lvl].next == slist->slh_tail) ? ((void *)0) : node->entries.sle_levels[lvl].next;
        (void)__skip_dot_width_ex;
        fprintf(os, " { <w%lu> %lu | <f%lu> ", lvl, node->entries.sle_levels[lvl].hits, lvl);
        if (next)
            fprintf(os, "%p } |", (void *)next);
        else
            fprintf(os, "0x0 } |");
        fflush(os);
    }
    if (fn) {
        fn(node, buf);
        fprintf(os, " <f0> \u219F %lu \u226B %s \"\n", node->entries.sle_height, buf);
    } else {
        fprintf(os, " <f0> \u219F %lu \"\n", node->entries.sle_height);
    }
    fprintf(os, "shape = \"record\"\n");
    fprintf(os, "];\n");
    fflush(os);
    for (size_t lvl = 0; lvl <= node->entries.sle_height; lvl++) {
        next = (node->entries.sle_levels[lvl].next == slist->slh_tail) ? ((void *)0) : node->entries.sle_levels[lvl].next;
        __skip_dot_write_node_ex(os, nsg, node);
        fprintf(os, ":f%lu -> ", lvl);
        __skip_dot_write_node_ex(os, nsg, next);
        fprintf(os, ":w%lu [];\n", lvl);
        fflush(os);
    }
}
void
api_skip_dot_end_ex(FILE *os, size_t nsg)
{
    size_t i;
    if (nsg > 0) {
        fprintf(os, "node0 [shape=record, label = \"");
        for (i = 0; i < nsg; ++i) {
            fprintf(os, "<f%lu> | ", i);
        }
        fprintf(os, "\", style=invis, width=0.01];\n");
        for (i = 0; i < nsg; ++i) {
            fprintf(os, "node0:f%lu -> HeadNode%lu [style=invis];\n", i, i);
        }
        nsg = 0;
    }
    fprintf(os, "}\n");
}
int
api_skip_dot_ex(FILE *os, ex_t *slist, size_t nsg, char *msg, skip_sprintf_node_ex_t fn)
{
    int letitgo = 0;
    size_t i;
    ex_node_t *node, *next;
    if (slist == ((void *)0) || fn == ((void *)0))
        return nsg;
    if (nsg == 0) {
        fprintf(os, "digraph Skiplist {\n");
        fprintf(os, "label = \"Skiplist.\"\n");
        fprintf(os, "graph [rankdir = \"LR\"];\n");
        fprintf(os, "node [fontsize = \"12\" shape = \"ellipse\"];\n");
        fprintf(os, "edge [];\n\n");
    }
    fprintf(os, "subgraph cluster%lu {\n", nsg);
    fprintf(os, "style=dashed\n");
    fprintf(os, "label=\"Skiplist [%lu]", nsg);
    if (msg)
        fprintf(os, " %s", msg);
    fprintf(os, "\"\n\n");
    fprintf(os, "\"HeadNode%lu\" [\n", nsg);
    fprintf(os, "label = \"");
    if (slist->slh_head->entries.sle_height || slist->slh_head->entries.sle_levels[0].next != slist->slh_tail)
        letitgo = 1;
    node = slist->slh_head;
    if (letitgo) {
        for (size_t lvl = node->entries.sle_height; lvl != (size_t)-1; lvl--) {
            next = (node->entries.sle_levels[lvl].next == slist->slh_tail) ? ((void *)0) : node->entries.sle_levels[lvl].next;
            fprintf(os, "{ %lu | <f%lu> ", node->entries.sle_levels[lvl].hits, lvl);
            if (next)
                fprintf(os, "%p }", (void *)next);
            else
                fprintf(os, "0x0 }");
            if (lvl == 0)
                continue;
            fprintf(os, " | ");
        }
    } else {
        fprintf(os, "Empty HeadNode");
    }
    fprintf(os, "\"\n");
    fprintf(os, "shape = \"record\"\n");
    fprintf(os, "];\n");
    fflush(os);
    node = slist->slh_head;
    if (letitgo) {
        node = slist->slh_head;
        for (size_t lvl = 0; lvl <= node->entries.sle_height; lvl++) {
            next = (node->entries.sle_levels[lvl].next == slist->slh_tail) ? ((void *)0) : node->entries.sle_levels[lvl].next;
            fprintf(os, "\"HeadNode%lu\":f%lu -> ", nsg, lvl);
            __skip_dot_write_node_ex(os, nsg, next);
            fprintf(os, ":w%lu [];\n", lvl);
        }
        fprintf(os, "\n");
    }
    fflush(os);
    node = slist->slh_head;
    if (letitgo) {
        for (i = 0, (next) = (slist)->slh_head; ((next) = (next)->entries.sle_levels[0].next) != (slist)->slh_tail; i++) {
            ((void)i);
            __skip_dot_node_ex(os, slist, next, nsg, fn);
            fflush(os);
        }
        fprintf(os, "\n");
    }
    fflush(os);
    if (letitgo) {
        __skip_dot_write_node_ex(os, nsg, ((void *)0));
        fprintf(os, " [label = \"");
        node = slist->slh_tail;
        size_t th = slist->slh_head->entries.sle_height;
        for (size_t lvl = th; lvl != (size_t)-1; lvl--) {
            next = (node->entries.sle_levels[lvl].next == slist->slh_tail) ? ((void *)0) : node->entries.sle_levels[lvl].next;
            fprintf(os, "<w%lu> 0x0", lvl);
            if (lvl == 0)
                continue;
            fprintf(os, " | ");
        }
        fprintf(os, "\" shape = \"record\"];\n");
    }
    fprintf(os, "}\n\n");
    nsg += 1;
    fflush(os);
    return nsg;
}

static void
sprintf_ex_node(ex_node_t *node, char *buf)
{
    sprintf(buf, "%d:%s", node->key, node->value);
}

// Function for this demo application.
// ---------------------------------------------------------------------------

/* convert a number into the Roman numeral equivalent, allocates a string caller must free */
static char *
int_to_roman_numeral(int num)
{
    int del[] = { 1000, 900, 500, 400, 100, 90, 50, 40, 10, 9, 5, 4, 1 };                    // Key value in Roman counting
    char *sym[] = { "M", "CM", "D", "CD", "C", "XC", "L", "XL", "X", "IX", "V", "IV", "I" }; // Symbols for key values
    // The maximum length of the Roman numeral representation for the maximum signed 64-bit integer would be approximately 19 * 3 = 57 characters, assuming
    // every digit is represented by its Roman numeral equivalent up to 3 repetitions.  Therefore, 64 should be more than enough.
    char *res = (char *)calloc(4096, sizeof(char));
    int i = 0;
    if (num < 0) {
        res[0] = '-';
        i++;
        num = -num;
    }
    if (num == 0) {
        res[0] = '0';
        return res;
    }
    if (num > 10000) {
        sprintf(res, "The person you were looking for is not here, their mailbox is full, good bye.");
        return res;
    }
    while (num) {                // while input number is not zero
        while (num / del[i]) {   // while a number contains the largest key value possible
            strcat(res, sym[i]); // append the symbol for this key value to res string
            num -= del[i];       // subtract the key value from number
        }
        i++; // proceed to the next key value
    }
    return res;
}

/* calculate the floor of the log base 2 of a number m (⌊log2(m)⌋) */
static int
floor_log2(unsigned int m)
{
    return (int)floor(log(m) / log(2));
}
// ---------------------------------------------------------------------------

/* The head node's height is always 1 more than the tallest node, that location
   is where we store the total hits, or "m". */

int
main(int argc, char *argv[], char *envp[])
{
    (argc) = (1);
    (argv) = (((void *)0));
    (envp) = (((void *)0));
    int rc;
    char *numeral;

    char msg[1024] = { 0 };

    of = fopen("/tmp/ex1.dot", "w");
    if (!of) {
        perror("Failed to open file /tmp/ex1.dot");
        return 1;
    }

    /* Allocate and initialize a Skiplist. */
    ex_t *list = (ex_t *)malloc(sizeof(ex_t));
    if (list == ((void *)0)) {
        return 12;
    }

    rc = api_skip_init_ex(list);
    if (rc) {
        return rc;
    }

    /* Set the PRNG state to a known constant for reproducible generation, easing debugging. */
    list->slh_prng_state = 12;

    api_skip_dot_ex(of, list, gen++, "init", sprintf_ex_node);

    /* This example mirrors the example given in the paper about splay-lists
       to test implementation against research. */

    for (int i = 1; i < 8; i++) {
        numeral = int_to_roman_numeral(i);
        if ((rc = api_skip_put_ex(list, i, numeral))) {
            perror("put failed");
        }

        sprintf(msg, "put key: %d value: %s", i, numeral);
        api_skip_dot_ex(of, list, gen++, msg, sprintf_ex_node);
    }
    __skip_integrity_check_ex(list, 0);

    /* Now we're going to poke around the internals a bit to set things up.
       This first time around we're going to build the list by hand, later
       we'll ensure that we can build this shape using only API calls. */

    ex_node_t *head = list->slh_head;
    ex_node_t *tail = list->slh_tail;
    ex_node_t *node_1 = head->entries.sle_levels[0].next;
    ex_node_t *node_2 = node_1->entries.sle_levels[0].next;
    ex_node_t *node_3 = node_2->entries.sle_levels[0].next;
    ex_node_t *node_4 = node_3->entries.sle_levels[0].next;
    ex_node_t *node_5 = node_4->entries.sle_levels[0].next;
    ex_node_t *node_6 = node_5->entries.sle_levels[0].next;

    // Head/Tail-nodes are height 3, ...
    head->entries.sle_height = tail->entries.sle_height = 3;

    // Head-node
    head->entries.sle_levels[3].hits = 10;
    head->entries.sle_levels[2].hits = 5;
    head->entries.sle_levels[1].hits = 1;
    head->entries.sle_levels[0].hits = 1;
    head->entries.sle_levels[1].next = node_2;
    head->entries.sle_levels[2].next = node_6;
    head->entries.sle_levels[3].next = tail;

    // Tail-node
    tail->entries.sle_levels[3].hits = 0;
    tail->entries.sle_levels[2].hits = 0;
    tail->entries.sle_levels[1].hits = 0;
    tail->entries.sle_levels[0].hits = 1;
    tail->entries.sle_levels[1].next = tail;
    tail->entries.sle_levels[2].next = tail;
    tail->entries.sle_levels[3].next = tail;

    // First node has key "1", height "0", hits(0) = 1
    node_1->entries.sle_height = 0;
    node_1->entries.sle_levels[0].hits = 1;

    // Second node has key "2", height "1", hits(0) = 1, hits(1) = 0
    node_2->entries.sle_height = 1;
    node_2->entries.sle_levels[0].hits = 1;
    node_2->entries.sle_levels[1].hits = 0;
    node_2->entries.sle_levels[1].next = node_3;

    // Third node has key "3", height "1", hits(0) = 1, hits(1) = 2
    node_3->entries.sle_height = 1;
    node_3->entries.sle_levels[0].hits = 1;
    node_3->entries.sle_levels[1].hits = 2;
    node_3->entries.sle_levels[1].next = node_6;

    // Fourth node has key "4", height "0", hits(0) = 1
    node_4->entries.sle_height = 0;
    node_4->entries.sle_levels[0].hits = 1;

    // Fifth node has key "5", height "0", hits(0) = 1
    node_5->entries.sle_height = 0;
    node_5->entries.sle_levels[0].hits = 1;

    // Sixth node has key "6", height "2", hits(0) = 5, hits(1) = 0, hits(2) = 0
    node_6->entries.sle_height = 2;
    node_6->entries.sle_levels[0].hits = 5;
    node_6->entries.sle_levels[1].hits = 0;
    node_6->entries.sle_levels[2].hits = 0;
    node_6->entries.sle_levels[1].next = tail;
    node_6->entries.sle_levels[2].next = tail;

    __skip_integrity_check_ex(list, 0);

    sprintf(msg, "manually adjusted");
    api_skip_dot_ex(of, list, gen++, msg, sprintf_ex_node);

    printf("m = %ld; ", list->slh_head->entries.sle_levels[list->slh_head->entries.sle_height].hits);
    printf("(⌊log2(m)⌋) = %d\n", floor_log2(list->slh_head->entries.sle_levels[list->slh_head->entries.sle_height].hits));

    if (!(rc = api_skip_contains_ex(list, 5))) {
        perror("missing element 5");
    }

    sprintf(msg, "contains(5)");
    api_skip_dot_ex(of, list, gen++, msg, sprintf_ex_node);

    api_skip_dot_end_ex(of, gen);
    fclose(of);

    api_skip_free_ex(list);
    free(list);
    return rc;
}
#pragma GCC pop_options
