CC=gcc
CFLAGS=-I./src

VPATH=src src/tests

obj/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

test_double_link_list: obj/double_link_list.o obj/test_double_link_list.o
	$(CC) -o bin/$@ $^ $(CFLAGS)

