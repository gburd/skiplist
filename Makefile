
OBJS =
STATIC_LIB =
SHARED_LIB =

# https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html
#CFLAGS = -Wall -Wextra -Wpedantic -Of -std=c99 -Iinclude/ -fPIC
#CFLAGS = -Wall -Wextra -Wpedantic -Og -g -std=c99 -Iinclude/ -fPIC
CFLAGS = -Wall -Wextra -Wpedantic -Og -g -fsanitize=address,leak,object-size,pointer-compare,pointer-subtract,null,return,bounds,pointer-overflow,undefined -fsanitize-address-use-after-scope -std=c11 -Iinclude/ -fPIC
#CFLAGS = -Wall -Wextra -Wpedantic -Og -g -fsanitize=all -fhardened -std=c99 -Iinclude/ -fPIC
#env ASAN_OPTIONS=detect_leaks=1 LSAN_OPTIONS=verbosity=1:log_threads=1 ./examples/mls

TEST_FLAGS = -Itests/ -DDEBUG -DSKIPLIST_DIAGNOSTIC

TESTS = tests/test
TEST_OBJS = tests/test.o tests/munit.o
EXAMPLES = examples/ex1 examples/ex2

.PHONY: all shared static clean test test_concurrent test_tsan test_all examples mls coverage bench

all: static shared

static: $(STATIC_LIB)

shared: $(SHARED_LIB)

$(STATIC_LIB): $(OBJS)
	ar rcs $(STATIC_LIB) $?

$(SHARED_LIB): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $? -shared

examples: $(EXAMPLES)

mls: examples/mls

test: $(TESTS)
	./tests/test
#	env LSAN_OPTIONS=verbosity=1:log_threads=1 ./tests/test

tests/test: $(TEST_OBJS) $(STATIC_LIB)
	$(CC) $^ -o $@ $(CFLAGS) $(TEST_FLAGS) -lm -pthread

test_concurrent: tests/test_concurrent
	./tests/test_concurrent

tests/test_concurrent: tests/test_concurrent.o tests/munit.o $(STATIC_LIB)
	$(CC) $^ -o $@ $(CFLAGS) $(TEST_FLAGS) -lm -pthread

test_tsan: tests/test_concurrent_tsan
	./tests/test_concurrent_tsan

tests/test_concurrent_tsan: tests/test_concurrent.c tests/munit.c include/sl.h
	$(CC) $(TSAN_CFLAGS) $(TEST_FLAGS) -o $@ tests/test_concurrent.c tests/munit.c -lm -pthread

test_all: test test_concurrent test_tsan

COV_CFLAGS = -Wall -Wextra -Wpedantic -O0 -g --coverage -std=c11 -Iinclude/ -fPIC
BENCH_CFLAGS = -Wall -Wextra -Wpedantic -O2 -std=c11 -Iinclude/ -fPIC -DNDEBUG
TSAN_CFLAGS = -Wall -Wextra -Wpedantic -Og -g -fsanitize=thread -std=c11 -Iinclude/ -fPIC

coverage:
	rm -f tests/test_cov tests/test_cov.o tests/munit_cov.o
	rm -f tests/*.gcda tests/*.gcno
	$(CC) $(COV_CFLAGS) -c -o tests/test_cov.o tests/test.c
	$(CC) $(COV_CFLAGS) -c -o tests/munit_cov.o tests/munit.c
	$(CC) tests/test_cov.o tests/munit_cov.o -o tests/test_cov $(COV_CFLAGS) $(TEST_FLAGS) -lm -pthread
	./tests/test_cov
	gcov -r -b tests/test_cov.gcda 2>/dev/null | grep -A1 "^File.*sl\.h"

bench: bench/bench
	./bench/bench

bench/bench: bench/bench.c include/sl.h
	$(CC) $(BENCH_CFLAGS) bench/bench.c -o bench/bench -lm -pthread

clean:
	rm -f $(OBJS) $(TEST_OBJS)
	rm -f examples/mls.c
	rm -f $(STATIC_LIB)
	rm -f $(SHARED_LIB)
	rm -f $(TESTS)
	rm -f $(EXAMPLES)
	rm -f tests/test_cov tests/test_cov.o tests/munit_cov.o
	rm -f tests/*.gcda tests/*.gcno *.gcov
	rm -f tests/test_concurrent tests/test_concurrent.o
	rm -f tests/test_concurrent_tsan tests/test_concurrent_tsan.o tests/munit_tsan.o
	rm -f bench/bench

format:
	clang-format -i include/*.h tests/*.c tests/*.h examples/*.c bench/*.c

%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $^

tests/%.o: tests/%.c
	$(CC) $(CFLAGS) $(TEST_FLAGS) -c -o $@ $^

examples/ex1: examples/ex1.o
	$(CC) $^ -o $@ $(CFLAGS) -lm -pthread

examples/ex2: examples/ex2.o
	$(CC) $^ -o $@ $(CFLAGS) -lm

examples/%.o: examples/%.c
	$(CC) $(CFLAGS) -c -o $@ $^

examples/mls.c: examples/ex1.c
	$(CC) $(CFLAGS) -C -E examples/ex1.c | sed -e '1,7d' -e '/^# [0-9]* "/d' | clang-format > examples/mls.c

examples/ex1_sl.c: examples/ex1.c
	$(CC) $(CFLAGS) -C -E examples/ex2.c | sed -e '1,7d' -e '/^# [0-9]* "/d' | clang-format > examples/ex1_sl.c

examples/ex2_sl.c: examples/ex2.c
	$(CC) $(CFLAGS) -C -E examples/ex2.c | sed -e '1,7d' -e '/^# [0-9]* "/d' | clang-format > examples/ex2_sl.c

examples/mls: examples/mls.o $(STATIC_LIB)
	$(CC) $^ -o $@ $(CFLAGS) $(TEST_FLAGS) -lm -pthread

examples/ex1_sl: examples/ex1_sl.o $(STATIC_LIB)
	$(CC) $^ -o $@ $(CFLAGS) $(TEST_FLAGS) -lm -pthread

examples/ex2_sl: examples/ex2_sl.o $(STATIC_LIB)
	$(CC) $^ -o $@ $(CFLAGS) $(TEST_FLAGS) -lm -pthread

#dot:
#	./examples/mls
#	dot -Tpdf /tmp/ex1.dot -o /tmp/ex1.pdf >/dev/null 2>&1

#re-write CPP line information comments, but keep them
#	$(CC) $(CFLAGS) -C -E examples/ex1.c | sed -e '1,7d' -e 's/^#\( [0-9]* ".*$$\)/\/\* \1 \*\//' | clang-format > examples/mls.c

# workflow:
# clear; rm examples/mls.c;  make examples/mls && env ASAN_OPTIONS=detect_leaks=1 LSAN_OPTIONS=verbosity=1:log_threads=1 ./examples/mls #&& dot -Tpdf /tmp/ex1.dot -o /tmp/ex1.pdf
# cp include/sl.h /tmp/foo; clang-format -i include/sl.h
# clear; rm ./examples/ex2_sl ./examples/ex2_sl.c ex2_sl.o; make examples/ex2_sl && env ASAN_OPTIONS=detect_leaks=1 LSAN_OPTIONS=verbosity=1:log_threads=1 ./examples/ex2_sl && dot -Tpdf /tmp/ex2.dot -o /tmp/ex2.pdf
