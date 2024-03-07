LDFLAGS = -pthread
CFLAGS = \
	-g -D_GNU_SOURCE \
	-I. -I./src -I./debug -I./include -I./examples -I./tests \
	-fPIC \

CFLAGS += -Wall
CFLAGS += -O3
#CFLAGS += -fsanitize=address -fuse-ld=gold

SKIPLIST = src/skiplist.o
SHARED_LIB = libskiplist.so
STATIC_LIB = libskiplist.a

EXAMPLE = \
	src/example.o \
	$(STATIC_LIB) \

PROGRAMS = \
	src/example \
	libskiplist.so \
	libskiplist.a \

all: $(PROGRAMS)

libskiplist.so: $(SKIPLIST)
	$(CC) -shared $(LDBFALGS) -o $(SHARED_LIB) $(SKIPLIST)

libskiplist.a: $(SKIPLIST)
	ar rcs $(STATIC_LIB) $(SKIPLIST)

src/example: $(EXAMPLE)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

format:
	clang-format -i src/*.[ch]

clean:
	rm -rf $(PROGRAMS) ./*.o ./*.so ./*/*.o ./*/*.so
