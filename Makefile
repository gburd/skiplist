# Skiplist -- header-only C splay-list with lock-free concurrency
#
# Build system #1 of 2.  This Makefile is the canonical entry for
# Unix-like systems and CI.  A meson.build is also provided for
# downstream consumers that prefer meson/ninja; see README for details.
#
# Targets:
#   make test            -- build and run the unit-test suite
#   make test_concurrent -- build and run multi-threaded tests
#   make test_tsan       -- run multi-threaded tests under ThreadSanitizer
#   make test_all        -- all of the above
#   make examples        -- build all examples (ex01..ex10)
#   make bench           -- build and run the benchmark suite
#   make coverage        -- gcov coverage over include/sl.h
#   make valgrind        -- run unit tests under valgrind (single-threaded)
#   make format          -- clang-format the tree
#   make install         -- install header + pkg-config file
#   make clean           -- remove build artefacts
#   make distclean       -- clean + remove configure-generated files
#
# Variables (override on the command line):
#   CC=clang             -- pick compiler
#   PREFIX=/opt          -- install prefix (default /usr/local)
#
# After running ./configure, this Makefile picks up substituted
# variables (CC, CFLAGS, sanitizer/coverage flags, install paths)
# from config.mk; without it, the defaults below apply.

-include config.mk

PREFIX     ?= /usr/local
INCLUDEDIR ?= $(PREFIX)/include
LIBDIR     ?= $(PREFIX)/lib

CC         ?= cc
INSTALL    ?= install

# Shared warning flags for all targets.  -Werror keeps the worktree
# warning-free under both gcc and clang; tighten or relax via WARNFLAGS.
WARNFLAGS  ?= -Wall -Wextra -Wpedantic -Werror

# Default debug-with-sanitizers build.  -Og keeps debug-info-friendly
# generation while still defining __OPTIMIZE__ so glibc's
# _FORTIFY_SOURCE checks are satisfied on hardened distros.
SAN_FLAGS   = -fsanitize=address,leak,object-size,pointer-compare,pointer-subtract,null,return,bounds,pointer-overflow,undefined \
              -fsanitize-address-use-after-scope
CFLAGS     ?= $(WARNFLAGS) -Og -g $(SAN_FLAGS) -std=c11 -Iinclude/ -fPIC
CFLAGS     += $(EXTRA_CFLAGS)
LDFLAGS    += $(EXTRA_LDFLAGS)

# Test-only additions (DEBUG enables internal asserts; SKIPLIST_DIAGNOSTIC
# enables runtime invariant checks in the implementation).
TEST_FLAGS  = -Itests/ -DDEBUG -DSKIPLIST_DIAGNOSTIC

# ThreadSanitizer cannot be combined with AddressSanitizer; use a
# distinct flag set for that target.
TSAN_CFLAGS = $(WARNFLAGS) -Og -g -fsanitize=thread -std=c11 -Iinclude/ -fPIC

# Optimised, NDEBUG benchmark build.
BENCH_CFLAGS = $(WARNFLAGS) -O2 -std=c11 -Iinclude/ -fPIC -DNDEBUG

# Coverage build: no sanitizers (incompatible with --coverage on some
# toolchains), no optimisation, full debug info, gcov instrumentation.
COV_CFLAGS  = $(WARNFLAGS) -O0 -g --coverage -std=c11 -Iinclude/ -fPIC

EXAMPLES = examples/ex01 examples/ex02 examples/ex03 examples/ex04 \
           examples/ex05 examples/ex06 examples/ex07 examples/ex08 \
           examples/ex09 examples/ex10

.PHONY: all clean distclean test test_concurrent test_tsan test_all examples mls coverage \
        bench valgrind install uninstall format

# Header-only library: no .a / .so / .o to produce.  "all" builds
# everything that exercises the header, which is the meaningful default.
all: test test_concurrent examples bench

# ----------------------------------------------------------------------
# Tests
# ----------------------------------------------------------------------

test: tests/test
	./tests/test

tests/test: tests/test.o tests/munit.o
	$(CC) $^ -o $@ $(CFLAGS) $(TEST_FLAGS) -lm -pthread

test_concurrent: tests/test_concurrent
	./tests/test_concurrent

tests/test_concurrent: tests/test_concurrent.o tests/munit.o
	$(CC) $^ -o $@ $(CFLAGS) $(TEST_FLAGS) -lm -pthread

test_tsan: tests/test_concurrent_tsan
	./tests/test_concurrent_tsan

tests/test_concurrent_tsan: tests/test_concurrent.c tests/munit.c include/sl.h
	$(CC) $(TSAN_CFLAGS) $(TEST_FLAGS) -o $@ tests/test_concurrent.c tests/munit.c -lm -pthread

test_all: test test_concurrent test_tsan

tests/%.o: tests/%.c include/sl.h
	$(CC) $(CFLAGS) $(TEST_FLAGS) -c -o $@ $<

# ----------------------------------------------------------------------
# Examples
# ----------------------------------------------------------------------

examples: $(EXAMPLES)

examples/ex01: examples/ex01.o
	$(CC) $^ -o $@ $(CFLAGS) -lm
examples/ex02: examples/ex02.o
	$(CC) $^ -o $@ $(CFLAGS) -lm
examples/ex03: examples/ex03.o
	$(CC) $^ -o $@ $(CFLAGS) -lm
examples/ex04: examples/ex04.o
	$(CC) $^ -o $@ $(CFLAGS) -lm
examples/ex05: examples/ex05.o
	$(CC) $^ -o $@ $(CFLAGS) -lm
examples/ex06: examples/ex06.o
	$(CC) $^ -o $@ $(CFLAGS) -lm
examples/ex07: examples/ex07.o
	$(CC) $^ -o $@ $(CFLAGS) -lm
examples/ex08: examples/ex08.o
	$(CC) $^ -o $@ $(CFLAGS) -lm
examples/ex09: examples/ex09.o
	$(CC) $^ -o $@ $(CFLAGS) -lm -pthread
examples/ex10: examples/ex10.o
	$(CC) $^ -o $@ $(CFLAGS) -lm -pthread

examples/%.o: examples/%.c include/sl.h
	$(CC) $(CFLAGS) -c -o $@ $<

# Preprocessor expansion of ex01.c, useful for inspecting macro output.
mls: examples/mls

examples/mls.c: examples/ex01.c include/sl.h
	$(CC) $(CFLAGS) -C -E examples/ex01.c | sed -e '1,7d' -e '/^# [0-9]* "/d' | clang-format > examples/mls.c

examples/mls: examples/mls.c
	$(CC) $(CFLAGS) $(TEST_FLAGS) -o $@ examples/mls.c -lm -pthread

# ----------------------------------------------------------------------
# Benchmark
# ----------------------------------------------------------------------

bench: bench/bench
	./bench/bench

bench/bench: bench/bench.c include/sl.h
	$(CC) $(BENCH_CFLAGS) bench/bench.c -o bench/bench -lm -pthread

# ----------------------------------------------------------------------
# Coverage / valgrind
# ----------------------------------------------------------------------

coverage:
	rm -f tests/test_cov tests/test_cov.o tests/munit_cov.o
	rm -f tests/*.gcda tests/*.gcno
	$(CC) $(COV_CFLAGS) $(TEST_FLAGS) -c -o tests/test_cov.o tests/test.c
	$(CC) $(COV_CFLAGS) $(TEST_FLAGS) -c -o tests/munit_cov.o tests/munit.c
	$(CC) tests/test_cov.o tests/munit_cov.o -o tests/test_cov $(COV_CFLAGS) $(TEST_FLAGS) -lm -pthread
	./tests/test_cov
	gcov -r -b tests/test_cov.gcda 2>/dev/null | grep -A1 "^File.*sl\.h" || true

# Valgrind cannot be combined with AddressSanitizer.  Build a separate
# unsanitized binary just for this target.
valgrind: tests/test_valgrind
	valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 ./tests/test_valgrind

tests/test_valgrind: tests/test.c tests/munit.c include/sl.h
	$(CC) $(WARNFLAGS) -Og -g -std=c11 -Iinclude/ $(TEST_FLAGS) -o $@ tests/test.c tests/munit.c -lm -pthread

# ----------------------------------------------------------------------
# Install / pkg-config
# ----------------------------------------------------------------------

install: skiplist.pc
	$(INSTALL) -d $(DESTDIR)$(INCLUDEDIR)
	$(INSTALL) -m 644 include/sl.h $(DESTDIR)$(INCLUDEDIR)/sl.h
	$(INSTALL) -d $(DESTDIR)$(LIBDIR)/pkgconfig
	$(INSTALL) -m 644 skiplist.pc $(DESTDIR)$(LIBDIR)/pkgconfig/skiplist.pc

uninstall:
	rm -f $(DESTDIR)$(INCLUDEDIR)/sl.h
	rm -f $(DESTDIR)$(LIBDIR)/pkgconfig/skiplist.pc

# When ./configure was run, skiplist.pc is generated from skiplist.pc.in
# by the substitution of @PREFIX@/@INCLUDEDIR@/@LIBDIR@ via config.status.
# Without configure, this rule produces it directly.
skiplist.pc: skiplist.pc.in
	sed -e 's|@PREFIX@|$(PREFIX)|g' \
	    -e 's|@INCLUDEDIR@|$(INCLUDEDIR)|g' \
	    -e 's|@LIBDIR@|$(LIBDIR)|g' \
	    skiplist.pc.in > skiplist.pc

# ----------------------------------------------------------------------
# Format / clean
# ----------------------------------------------------------------------

format:
	clang-format -i include/*.h tests/*.c tests/*.h examples/*.c bench/*.c

clean:
	rm -f tests/*.o tests/test tests/test_concurrent tests/test_concurrent_tsan
	rm -f tests/test_cov tests/test_cov.o tests/munit_cov.o
	rm -f tests/test_valgrind
	rm -f tests/*.gcda tests/*.gcno *.gcov
	rm -f examples/*.o $(EXAMPLES)
	rm -f examples/mls examples/mls.c
	rm -f bench/bench
	rm -f skiplist.pc

distclean: clean
	rm -f config.mk config.log config.status
	rm -f configure aclocal.m4
	rm -f autom4te.cache/* autom4te.cache/*/* 2>/dev/null || true
	rmdir autom4te.cache/output.* autom4te.cache 2>/dev/null || true
	rm -f build-aux/*
	rmdir build-aux 2>/dev/null || true
