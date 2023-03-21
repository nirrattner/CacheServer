CC=gcc
CFLAGS=-I./src -I./src/tests/mocks

VPATH=src src/tests src/tests/mocks

obj/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: all
all: test_memory_queue test_entry_hash_map test_connection_list cache_server

.PHONY: clean
clean:
	rm obj/*

.PHONY: tests
tests: test_memory_queue test_entry_hash_map test_connection_list
	bin/test_memory_queue; bin/test_entry_hash_map; bin/test_connection_list

test_memory_queue: obj/entry_header.o obj/mock_entry_hash_map.o obj/memory_queue.o obj/test_memory_queue.o
	$(CC) -o bin/$@ $^ $(CFLAGS)

test_entry_hash_map: obj/entry_header.o obj/hashmap.o obj/entry_hash_map.o obj/test_entry_hash_map.o
	$(CC) -o bin/$@ $^ $(CFLAGS)

test_connection_list: obj/mock_connection.o obj/connection_list.o obj/test_connection_list.o
	$(CC) -o bin/$@ $^ $(CFLAGS)

cache_server: obj/configuration.o obj/time_util.o obj/entry_header.o obj/hashmap.o obj/entry_hash_map.o obj/memory_queue.o obj/connection.o obj/blocked_connections.o obj/connection_list.o obj/cache_server.o obj/main.o
	$(CC) -o bin/$@ $^ $(CFLAGS)

