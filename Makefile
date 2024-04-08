
OBJS =
STATIC_LIB =
SHARED_LIB =

# https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html
#CFLAGS = -Wall -Wextra -Wpedantic -Of -std=c99 -Iinclude/ -fPIC
CFLAGS = -Wall -Wextra -Wpedantic -Og -g -std=c99 -Iinclude/ -fPIC
#CFLAGS = -Wall -Wextra -Wpedantic -Og -g -fsanitize=address,leak,object-size,pointer-compare,pointer-subtract,null,return,bounds,pointer-overflow,undefined -fsanitize-address-use-after-scope -std=c99 -Iinclude/ -fPIC
#CFLAGS = -Wall -Wextra -Wpedantic -Og -g -fsanitize=all -fhardened -std=c99 -Iinclude/ -fPIC
#env ASAN_OPTIONS=detect_leaks=1 LSAN_OPTIONS=verbosity=1:log_threads=1 ./examples/mls

TEST_FLAGS = -Itests/

TESTS = tests/test
TEST_OBJS = tests/test.o tests/munit.o
EXAMPLES = examples/ex1.c

.PHONY: all shared static clean test examples mls

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

clean:
	rm -f $(OBJS) munit.o test.o
	rm -f examples/mls.c
	rm -f $(STATIC_LIB)
	rm -f $(SHARED_LIB)
	rm -f $(TESTS)
	rm -f $(EXAMPLES)

format:
	clang-format -i include/*.h src/*.c tests/*.c tests/*.h examples/*.c

%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $^

tests/%.o: tests/%.c
	$(CC) $(CFLAGS) -c -o $@ $^

examples/%.o: examples/%.c
	$(CC) $(CFLAGS) -c -o $@ $^

examples/mls.c: examples/ex1.c
	$(CC) $(CFLAGS) -C -E examples/ex1.c | sed -e '1,7d' -e '/^# [0-9]* "/d' | clang-format > examples/mls.c

examples/mls: examples/mls.o $(STATIC_LIB)
	$(CC) $^ -o $@ $(CFLAGS) $(TEST_FLAGS) -lm -pthread

#dot:
#	./examples/mls
#	dot -Tpdf /tmp/ex1.dot -o /tmp/ex1.pdf >/dev/null 2>&1

#re-write CPP line information comments, but keep them
#	$(CC) $(CFLAGS) -C -E examples/ex1.c | sed -e '1,7d' -e 's/^#\( [0-9]* ".*$$\)/\/\* \1 \*\//' | clang-format > examples/mls.c

# workflow:
# clear; rm examples/mls.c;  make examples/mls && env ASAN_OPTIONS=detect_leaks=1 LSAN_OPTIONS=verbosity=1:log_threads=1 ./examples/mls #&& dot -Tpdf /tmp/ex1.dot -o /tmp/ex1.pdf
# cp include/sl.h /tmp/foo; clang-format -i include/sl.h
