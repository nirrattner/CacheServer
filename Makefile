CC=gcc
CFLAGS=-I./src

VPATH=src src/tests

obj/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: test_double_link_list test_memory_pool

test_double_link_list: obj/double_link_list.o obj/test_double_link_list.o
	$(CC) -o bin/$@ $^ $(CFLAGS)

test_memory_pool: obj/memory_pool.o obj/test_memory_pool.o
	$(CC) -o bin/$@ $^ $(CFLAGS)


