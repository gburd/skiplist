
CFLAGS = -Wall -Wextra -Wpedantic -Og -g -std=c99 -Iinclude/ -fPIC
TEST_FLAGS = -Itests/
 #-fsanitize=address,undefined

OBJS = skiplist.o
STATIC_LIB = libskiplist.a
SHARED_LIB = libskiplist.so

TESTS = tests/test
EXAMPLES = examples/example

.PHONY: all shared static clean tests examples

%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $^

all: static shared

static: $(STATIC_LIB)

shared: $(SHARED_LIB)

$(STATIC_LIB): $(OBJS)
	ar rcs $(STATIC_LIB) $?

$(SHARED_LIB): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $? -shared

tests/%.o: tests/%.c
	$(CC) $(CFLAGS) -c -o $@ $^

test: $(TESTS)
	./tests/test
#	env LSAN_OPTIONS=verbosity=1:log_threads=1 ./tests/test

tests/test: tests/test.o tests/munit.o $(STATIC_LIB)
	$(CC) $^ -o $@ $(CFLAGS) $(TEST_FLAGS) -pthread

examples: $(EXAMPLES)

examples/example: examples/example.c $(STATIC_LIB)
	$(CC) $^ -o $@ $(CFLAGS) $(TEST_FLAGS) -pthread

clean:
	rm -f $(OBJS) munit.o test.o
	rm -f $(STATIC_LIB)
	rm -f $(TESTS)
	rm -f $(EXAMPLES)

format:
	clang-format -i include/*.h src/*.c tests/*.c tests/*.h
