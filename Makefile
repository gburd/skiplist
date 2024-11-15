
OBJS =
STATIC_LIB =
SHARED_LIB =

# https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html
CFLAGS = -Wall -Wextra -Wpedantic -Og -g -std=c99 -Iinclude/ -fPIC
#CFLAGS = -Wall -Wextra -Wpedantic -Of -std=c99 -Iinclude/ -fPIC
#CFLAGS = -Wall -Wextra -Wpedantic -Og -g -fsanitize=address,leak,object-size,pointer-compare,pointer-subtract,null,return,bounds,pointer-overflow,undefined -fsanitize-address-use-after-scope -std=c99 -Iinclude/ -fPIC
#CFLAGS = -Wall -Wextra -Wpedantic -Og -g -fsanitize=all -fhardened -std=c99 -Iinclude/ -fPIC
#env ASAN_OPTIONS=detect_leaks=1 LSAN_OPTIONS=verbosity=1:log_threads=1 ./examples/mls

EXAMPLES = src/ex1 src/ex2

.PHONY: all clean examples

examples: $(EXAMPLES)

clean:
	rm -f *.o src/*.o src/ex?
	rm -f ex1_sl.c ex2_sl.c ex1_sl.o ex2_sl.o ex1_sl ex2_sl

format:
	clang-format -i include/*.h src/*.c tests/*.c tests/*.h src/*.c

%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $^

ex%: ex%.o
	$(CC) $^ -o $@ $(CFLAGS) -lm -pthread

sl: ex1_sl ex2_sl

ex1_sl.c: src/ex1.c
	$(CC) $(CFLAGS) -C -E src/ex1.c | sed -e '1,7d' -e '/^# [0-9]* "/d' | clang-format > ex1_sl.c

ex2_sl.c: src/ex2.c
	$(CC) $(CFLAGS) -C -E src/ex2.c | sed -e '1,7d' -e '/^# [0-9]* "/d' | clang-format > ex2_sl.c

ex1_sl: ex1_sl.o
	$(CC) $^ -o $@ $(CFLAGS) -lm -pthread

ex2_sl: ex2_sl.o
	$(CC) $^ -o $@ $(CFLAGS) -lm -pthread

dot:
	./src/mls
	dot -Tpdf /tmp/ex1.dot -o /tmp/ex1.pdf >/dev/null 2>&1

#re-write CPP line information comments, but keep them
#	$(CC) $(CFLAGS) -C -E src/ex1.c | sed -e '1,7d' -e 's/^#\( [0-9]* ".*$$\)/\/\* \1 \*\//' | clang-format > src/mls.c

# workflow:
# clear; rm src/mls.c;  make src/mls && env ASAN_OPTIONS=detect_leaks=1 LSAN_OPTIONS=verbosity=1:log_threads=1 ./src/mls #&& dot -Tpdf /tmp/ex1.dot -o /tmp/ex1.pdf
# cp include/sl.h /tmp/foo; clang-format -i include/sl.h
