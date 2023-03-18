CC=gcc
CFLAGS=-I./src -I./src/tests/mocks

VPATH=src src/tests src/tests/mocks

obj/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: test_memory_queue test_entry_hash_map test_connection_list cache_server

clean:
	rm bin/*

test_memory_queue: obj/mock_entry_hash_map.o obj/memory_queue.o obj/test_memory_queue.o
	$(CC) -o bin/$@ $^ $(CFLAGS)

test_entry_hash_map: obj/hashmap.o obj/entry_hash_map.o test_entry_hash_map.o
	$(CC) -o bin/$@ $^ $(CFLAGS)

test_connection_list: obj/mock_connection.o  obj/connection_list.o obj/test_connection_list.o
	$(CC) -o bin/$@ $^ $(CFLAGS)

tests: test_memory_queue test_entry_hash_map test_connection_list
	bin/test_memory_queue; bin/test_entry_hash_map; bin/test_connection_list

cache_server: obj/configuration.o obj/time_util.o obj/hashmap.o obj/entry_hash_map.o obj/memory_queue.o obj/connection.o obj/connection_list.o obj/cache_server.o obj/main.o
	$(CC) -o bin/$@ $^ $(CFLAGS)

