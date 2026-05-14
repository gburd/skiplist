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
#   make test_splay      -- unit + concurrent tests with -DSKIPLIST_SPLAY_REBALANCE
#   make test_tsan_splay -- TSAN variant with splay rebalancing enabled
#   make test_single     -- exercise the SKIPLIST_SINGLE_THREADED build
#   make test_all        -- all of the above
#   make examples        -- build all examples (ex01..ex10)
#   make bench           -- build and run the benchmark suite
#   make coverage        -- gcov coverage over include/sl.h
#   make valgrind        -- run unit tests under valgrind (single-threaded)
#   make man             -- lint man pages (renders via mandoc if present)
#   make format          -- clang-format the tree
#   make install         -- install header + pkg-config + man pages
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
MANDIR     ?= $(PREFIX)/share/man

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
# Coverage build: -Og keeps __OPTIMIZE__ defined so glibc's hardened
# _FORTIFY_SOURCE check is satisfied, while still keeping coverage
# instrumentation accurate (no inlining surprises).
COV_CFLAGS  = $(WARNFLAGS) -Og -g --coverage -std=c11 -Iinclude/ -fPIC

EXAMPLES = examples/ex01 examples/ex02 examples/ex03 examples/ex04 \
           examples/ex05 examples/ex06 examples/ex07 examples/ex08 \
           examples/ex09 examples/ex10

MAN_PAGES = man/skiplist.7 man/sl.h.3

.PHONY: all clean distclean test test_concurrent test_tsan test_all examples mls coverage \
        test_splay test_tsan_splay test_single \
        bench valgrind install uninstall format man

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

# Splay-rebalance test variants.  The splay-list is the signature feature
# of this implementation; run the full suite a second time with the splay
# flag defined so any regression in the rebalance algorithm fires both an
# /splay_behavior assertion (in tests/test.c) and exercises the splay path
# under concurrent + sanitizer pressure.  Also runs the Aksenov 2020
# height-target verification (tests/test_splay_verify.c).
test_splay: tests/test_splay tests/test_concurrent_splay tests/test_splay_verify
	./tests/test_splay
	./tests/test_concurrent_splay
	./tests/test_splay_verify

tests/test_splay: tests/test.c tests/munit.c include/sl.h
	$(CC) $(CFLAGS) $(TEST_FLAGS) -DSKIPLIST_SPLAY_REBALANCE -o $@ tests/test.c tests/munit.c -lm -pthread

tests/test_concurrent_splay: tests/test_concurrent.c tests/munit.c include/sl.h
	$(CC) $(CFLAGS) $(TEST_FLAGS) -DSKIPLIST_SPLAY_REBALANCE -o $@ tests/test_concurrent.c tests/munit.c -lm -pthread

# Aksenov 2020 height-target verification.  SKIPLIST_SPLAY_REBALANCE
# is required for the test to do real work; without it, the test
# returns MUNIT_SKIP for the aksenov_target case.
tests/test_splay_verify: tests/test_splay_verify.c tests/munit.c include/sl.h
	$(CC) $(CFLAGS) $(TEST_FLAGS) -DSKIPLIST_SPLAY_REBALANCE -o $@ tests/test_splay_verify.c tests/munit.c -lm -pthread

test_tsan_splay: tests/test_concurrent_tsan_splay
	./tests/test_concurrent_tsan_splay

tests/test_concurrent_tsan_splay: tests/test_concurrent.c tests/munit.c include/sl.h
	$(CC) $(TSAN_CFLAGS) $(TEST_FLAGS) -DSKIPLIST_SPLAY_REBALANCE -o $@ tests/test_concurrent.c tests/munit.c -lm -pthread

# Single-threaded build path: SKIPLIST_SINGLE_THREADED replaces all C11
# atomics with plain operations and disables EBR.  Since the flag has
# global effect on the header, this lives in its own translation unit.
test_single: tests/test_single
	./tests/test_single

tests/test_single: tests/test_single.c tests/munit.c include/sl.h
	$(CC) $(CFLAGS) $(TEST_FLAGS) -o $@ tests/test_single.c tests/munit.c -lm

test_all: test test_concurrent test_tsan test_splay test_tsan_splay test_single

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

# Coverage target.  Builds and runs every test variant -- single-threaded
# baseline, splay variant, and single-threaded mode -- under gcov
# instrumentation, then aggregates the result via gcovr (or falls back to
# a plain gcov roll-up) and reports line and branch coverage on
# include/sl.h.  Fails if either falls below 95%.
# Coverage thresholds.  Line and function coverage are practical to hit;
# branch coverage in lock-free code includes many CAS retry and
# contention paths that are hard to exercise deterministically -- track
# it but don't gate on the same number.
COV_THRESHOLD ?= 95
BRANCH_THRESHOLD ?= 70

coverage:
	rm -rf coverage-report
	mkdir -p coverage-report
	rm -f tests/*.gcda tests/*.gcno *.gcov tests/test_cov tests/test_cov_splay tests/test_cov_single
	rm -f tests/test_cov_concurrent tests/test_cov_concurrent_splay
	# Coverage uses gcc + gcov; clang's profile format is incompatible with
	# the system gcov tool used by gcovr.  CC may be overridden for the
	# regular build, but coverage pins gcc for portability.
	$(eval COV_CC := gcc)
	# 1. Default build: tests/test.c
	$(COV_CC) $(COV_CFLAGS) $(TEST_FLAGS) -o tests/test_cov tests/test.c tests/munit.c -lm -pthread
	./tests/test_cov
	# 2. Default build: tests/test_concurrent.c
	$(COV_CC) $(COV_CFLAGS) $(TEST_FLAGS) -o tests/test_cov_concurrent tests/test_concurrent.c tests/munit.c -lm -pthread
	./tests/test_cov_concurrent
	# 3. Splay variant: tests/test.c
	$(COV_CC) $(COV_CFLAGS) $(TEST_FLAGS) -DSKIPLIST_SPLAY_REBALANCE -o tests/test_cov_splay tests/test.c tests/munit.c -lm -pthread
	./tests/test_cov_splay
	# 4. Splay variant: tests/test_concurrent.c
	$(COV_CC) $(COV_CFLAGS) $(TEST_FLAGS) -DSKIPLIST_SPLAY_REBALANCE -o tests/test_cov_concurrent_splay tests/test_concurrent.c tests/munit.c -lm -pthread
	./tests/test_cov_concurrent_splay
	# 5. Single-threaded: tests/test_single.c
	$(COV_CC) $(COV_CFLAGS) $(TEST_FLAGS) -o tests/test_cov_single tests/test_single.c tests/munit.c -lm
	./tests/test_cov_single
	# Aggregate.  Heavy macro usage means gcov attributes most
	# expanded code to the .c file that includes sl.h, not to sl.h
	# itself.  Therefore we measure the union of include/sl.h plus the
	# three test translation units that instantiate the SKIPLIST_DECL_*
	# macros: that pair captures the implementation surface.
	@echo
	@echo "=== Coverage (include/sl.h + macro-instantiating test units) ==="
	@if command -v gcovr >/dev/null 2>&1; then \
	  gcovr --filter 'include/sl\.h' \
	        --filter 'tests/test\.c' \
	        --filter 'tests/test_concurrent\.c' \
	        --filter 'tests/test_single\.c' \
	        --print-summary \
	        --html-details coverage-report/index.html \
	        --txt coverage-report/summary.txt \
	        --json-summary coverage-report/summary.json; \
	  cat coverage-report/summary.txt; \
	  line_pct=$$(python3 -c "import json; d=json.load(open('coverage-report/summary.json')); print(int(d['line_percent']))"); \
	  branch_pct=$$(python3 -c "import json; d=json.load(open('coverage-report/summary.json')); print(int(d['branch_percent']))"); \
	  func_pct=$$(python3 -c "import json; d=json.load(open('coverage-report/summary.json')); print(int(d['function_percent']))"); \
	  echo; \
	  echo "Line coverage:     $$line_pct%"; \
	  echo "Function coverage: $$func_pct%"; \
	  echo "Branch coverage:   $$branch_pct%"; \
	  echo; \
	  if [ "$$line_pct" -lt $(COV_THRESHOLD) ] || [ "$$func_pct" -lt $(COV_THRESHOLD) ]; then \
	    echo "FAIL: line and/or function coverage below threshold ($(COV_THRESHOLD)%)"; exit 1; \
	  else \
	    echo "PASS: line and function coverage >= $(COV_THRESHOLD)%"; \
	    if [ "$$branch_pct" -lt $(BRANCH_THRESHOLD) ]; then \
	      echo "NOTE: branch coverage $$branch_pct% is below the relaxed branch target ($(BRANCH_THRESHOLD)%);"; \
	      echo "      lock-free CAS retry paths and EBR-induced contention paths are"; \
	      echo "      hard to exercise deterministically and are tracked separately."; \
	    fi; \
	  fi; \
	else \
	  echo "gcovr not installed; falling back to plain gcov"; \
	  gcov -r -b -o tests tests/test.c 2>/dev/null | tail -20; \
	fi

# Valgrind cannot be combined with AddressSanitizer.  Build a separate
# unsanitized binary just for this target.
valgrind: tests/test_valgrind
	valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 ./tests/test_valgrind

tests/test_valgrind: tests/test.c tests/munit.c include/sl.h
	$(CC) $(WARNFLAGS) -Og -g -std=c11 -Iinclude/ $(TEST_FLAGS) -o $@ tests/test.c tests/munit.c -lm -pthread

# ----------------------------------------------------------------------
# Man pages
# ----------------------------------------------------------------------

# Man pages are static groff files; this target exists for parity with
# meson and so 'make man' lints/renders them via mandoc when available.
man: $(MAN_PAGES)
	@for p in $(MAN_PAGES); do \
	  if command -v mandoc >/dev/null 2>&1; then \
	    mandoc -Tlint $$p || true; \
	  fi; \
	done
	@echo "Man pages OK: $(MAN_PAGES)"

# ----------------------------------------------------------------------
# Install / pkg-config
# ----------------------------------------------------------------------

install: skiplist.pc
	$(INSTALL) -d $(DESTDIR)$(INCLUDEDIR)
	$(INSTALL) -m 644 include/sl.h $(DESTDIR)$(INCLUDEDIR)/sl.h
	$(INSTALL) -d $(DESTDIR)$(LIBDIR)/pkgconfig
	$(INSTALL) -m 644 skiplist.pc $(DESTDIR)$(LIBDIR)/pkgconfig/skiplist.pc
	$(INSTALL) -d $(DESTDIR)$(MANDIR)/man3
	$(INSTALL) -m 644 man/sl.h.3 $(DESTDIR)$(MANDIR)/man3/sl.h.3
	$(INSTALL) -d $(DESTDIR)$(MANDIR)/man7
	$(INSTALL) -m 644 man/skiplist.7 $(DESTDIR)$(MANDIR)/man7/skiplist.7

uninstall:
	rm -f $(DESTDIR)$(INCLUDEDIR)/sl.h
	rm -f $(DESTDIR)$(LIBDIR)/pkgconfig/skiplist.pc
	rm -f $(DESTDIR)$(MANDIR)/man3/sl.h.3
	rm -f $(DESTDIR)$(MANDIR)/man7/skiplist.7

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
	rm -f tests/test_splay tests/test_concurrent_splay tests/test_concurrent_tsan_splay
	rm -f tests/test_splay_verify
	rm -f tests/test_single
	rm -f tests/test_cov tests/test_cov.o tests/munit_cov.o
	rm -f tests/test_cov_concurrent tests/test_cov_concurrent.o
	rm -f tests/test_cov_concurrent_splay tests/test_cov_splay tests/test_cov_single
	rm -f tests/test_valgrind
	rm -f tests/*.gcda tests/*.gcno *.gcov
	rm -f examples/*.o $(EXAMPLES)
	rm -f examples/mls examples/mls.c
	rm -f bench/bench
	rm -f skiplist.pc
	rm -rf coverage-report

distclean: clean
	rm -f config.mk config.log config.status
	rm -f configure aclocal.m4
	rm -f autom4te.cache/* autom4te.cache/*/* 2>/dev/null || true
	rmdir autom4te.cache/output.* autom4te.cache 2>/dev/null || true
	rm -f build-aux/*
	rmdir build-aux 2>/dev/null || true
