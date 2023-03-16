CC=gcc
CFLAGS=-I./src

VPATH=src src/tests

obj/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: test_memory_queue test_entry_hash_map

test_memory_queue: obj/memory_queue.o obj/test_memory_queue.o
	$(CC) -o bin/$@ $^ $(CFLAGS)

test_entry_hash_map: obj/hashmap.o obj/entry_hash_map.o test_entry_hash_map.o
	$(CC) -o bin/$@ $^ $(CFLAGS)

