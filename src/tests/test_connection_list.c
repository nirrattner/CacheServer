#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "connection_list.h"
#include "mock_connection.h"
#include "test_util.h"

static connection_t *new_connection(int file_descriptor);
static void assert_values(int *values, uint32_t value_size);

#define MAX_CAPACITY (16)
#define LISTEN_FILE_DESCRIPTOR (9999)

void it_opens(void) {
  PRINT_TEST()
  int expected_values[] = {};
  uint8_t result = connection_list_open(MAX_CAPACITY);
  assert(result == 0);

  connection_list_set_listen_file_descriptor(LISTEN_FILE_DESCRIPTOR);

  assert(connection_list_get_size() == 0);
  assert_values(expected_values, 0);

  connection_list_close();
}

void it_adds_and_gets(void) {
  PRINT_TEST()

  int expected_values[] = {1};
  uint8_t result = connection_list_open(MAX_CAPACITY);
  assert(result == 0);

  connection_list_set_listen_file_descriptor(LISTEN_FILE_DESCRIPTOR);

  connection_t *connection = new_connection(1);
  result = connection_list_add(connection);
  assert(result == 0);

  assert_values(expected_values, 1);

  connection_list_close();
  connection_deinit(connection);
}

void it_adds_and_gets_multiple(void) {
  PRINT_TEST()

  int expected_values[] = {1, 2, 3};
  uint8_t result = connection_list_open(MAX_CAPACITY);
  assert(result == 0);

  connection_list_set_listen_file_descriptor(LISTEN_FILE_DESCRIPTOR);

  connection_t *connection_1 = new_connection(1);
  connection_t *connection_2 = new_connection(2);
  connection_t *connection_3 = new_connection(3);

  result = connection_list_add(connection_1);
  assert(result == 0);

  result = connection_list_add(connection_2);
  assert(result == 0);

  result = connection_list_add(connection_3);
  assert(result == 0);

  assert_values(expected_values, 3);

  connection_list_close();
  connection_deinit(connection_1);
  connection_deinit(connection_2);
  connection_deinit(connection_3);
}

void it_adds_and_deletes_head(void) {
  PRINT_TEST()

  int expected_values[] = {3, 2};
  uint8_t result = connection_list_open(MAX_CAPACITY);
  assert(result == 0);

  connection_list_set_listen_file_descriptor(LISTEN_FILE_DESCRIPTOR);

  connection_t *connection_1 = new_connection(1);
  connection_t *connection_2 = new_connection(2);
  connection_t *connection_3 = new_connection(3);

  result = connection_list_add(connection_1);
  assert(result == 0);

  result = connection_list_add(connection_2);
  assert(result == 0);

  result = connection_list_add(connection_3);
  assert(result == 0);

  connection_list_remove(0);

  assert_values(expected_values, 2);

  connection_list_close();
  connection_deinit(connection_1);
  connection_deinit(connection_2);
  connection_deinit(connection_3);
}

void it_adds_and_deletes_middle(void) {
  PRINT_TEST()

  int expected_values[] = {1, 3};
  uint8_t result = connection_list_open(MAX_CAPACITY);
  assert(result == 0);

  connection_list_set_listen_file_descriptor(LISTEN_FILE_DESCRIPTOR);

  connection_t *connection_1 = new_connection(1);
  connection_t *connection_2 = new_connection(2);
  connection_t *connection_3 = new_connection(3);

  result = connection_list_add(connection_1);
  assert(result == 0);

  result = connection_list_add(connection_2);
  assert(result == 0);

  result = connection_list_add(connection_3);
  assert(result == 0);

  connection_list_remove(1);

  assert_values(expected_values, 2);

  connection_list_close();
  connection_deinit(connection_1);
  connection_deinit(connection_2);
  connection_deinit(connection_3);
}

void it_adds_and_deletes_tail(void) {
  PRINT_TEST()

  int expected_values[] = {1, 2};
  uint8_t result = connection_list_open(MAX_CAPACITY);
  assert(result == 0);

  connection_list_set_listen_file_descriptor(LISTEN_FILE_DESCRIPTOR);

  connection_t *connection_1 = new_connection(1);
  connection_t *connection_2 = new_connection(2);
  connection_t *connection_3 = new_connection(3);

  result = connection_list_add(connection_1);
  assert(result == 0);

  result = connection_list_add(connection_2);
  assert(result == 0);

  result = connection_list_add(connection_3);
  assert(result == 0);

  connection_list_remove(2);

  assert_values(expected_values, 2);

  connection_list_close();
  connection_deinit(connection_1);
  connection_deinit(connection_2);
  connection_deinit(connection_3);
}

void it_adds_and_deletes_multiple(void) {
  PRINT_TEST()

  int expected_values[] = {2};
  uint8_t result = connection_list_open(MAX_CAPACITY);
  assert(result == 0);

  connection_list_set_listen_file_descriptor(LISTEN_FILE_DESCRIPTOR);

  connection_t *connection_1 = new_connection(1);
  connection_t *connection_2 = new_connection(2);
  connection_t *connection_3 = new_connection(3);

  result = connection_list_add(connection_1);
  assert(result == 0);

  result = connection_list_add(connection_2);
  assert(result == 0);

  result = connection_list_add(connection_3);
  assert(result == 0);

  connection_list_remove(0);
  connection_list_remove(0);

  assert_values(expected_values, 1);

  connection_list_close();
  connection_deinit(connection_1);
  connection_deinit(connection_2);
  connection_deinit(connection_3);
}

void it_deletes_all_and_adds(void) {
  PRINT_TEST()

  int expected_values[] = {3};
  uint8_t result = connection_list_open(MAX_CAPACITY);
  assert(result == 0);

  connection_list_set_listen_file_descriptor(LISTEN_FILE_DESCRIPTOR);

  connection_t *connection_1 = new_connection(1);
  connection_t *connection_2 = new_connection(2);
  connection_t *connection_3 = new_connection(3);

  result = connection_list_add(connection_1);
  assert(result == 0);

  result = connection_list_add(connection_2);
  assert(result == 0);

  connection_list_remove(0);
  connection_list_remove(0);

  connection_list_add(connection_3);

  assert_values(expected_values, 1);

  connection_list_close();
  connection_deinit(connection_1);
  connection_deinit(connection_2);
  connection_deinit(connection_3);
}

void it_reinserts_first(void) {
  PRINT_TEST()

  int expected_values[] = {3, 2, 1};
  uint8_t result = connection_list_open(MAX_CAPACITY);
  assert(result == 0);

  connection_list_set_listen_file_descriptor(LISTEN_FILE_DESCRIPTOR);

  connection_t *connection_1 = new_connection(1);
  connection_t *connection_2 = new_connection(2);
  connection_t *connection_3 = new_connection(3);

  result = connection_list_add(connection_1);
  assert(result == 0);

  result = connection_list_add(connection_2);
  assert(result == 0);

  result = connection_list_add(connection_3);
  assert(result == 0);

  connection_list_remove(0);
  connection_list_add(connection_1);

  assert_values(expected_values, 3);

  connection_list_close();
  connection_deinit(connection_1);
  connection_deinit(connection_2);
  connection_deinit(connection_3);
}

void it_reinserts_middle(void) {
  PRINT_TEST()

  int expected_values[] = {1, 3, 2};
  uint8_t result = connection_list_open(MAX_CAPACITY);
  assert(result == 0);

  connection_list_set_listen_file_descriptor(LISTEN_FILE_DESCRIPTOR);

  connection_t *connection_1 = new_connection(1);
  connection_t *connection_2 = new_connection(2);
  connection_t *connection_3 = new_connection(3);

  result = connection_list_add(connection_1);
  assert(result == 0);

  result = connection_list_add(connection_2);
  assert(result == 0);

  result = connection_list_add(connection_3);
  assert(result == 0);

  connection_list_remove(1);
  connection_list_add(connection_2);

  assert_values(expected_values, 3);

  connection_list_close();
  connection_deinit(connection_1);
  connection_deinit(connection_2);
  connection_deinit(connection_3);
}

void it_resizes_on_add(void) {
  PRINT_TEST()

  uint8_t index;
  uint8_t size = 10;
  int expected_values[size];
  connection_t *connections[size];

  uint8_t result = connection_list_open(MAX_CAPACITY);
  assert(result == 0);

  connection_list_set_listen_file_descriptor(LISTEN_FILE_DESCRIPTOR);

  for (index = 0; index < size; index++) {
    expected_values[index] = index;
    connections[index] = new_connection(index);
    result = connection_list_add(connections[index]);
    assert(result == 0);
  }

  assert_values(expected_values, size);

  connection_list_close();
  for (index = 0; index < size; index++) {
    connection_deinit(connections[index]);
  }
}

static connection_t *new_connection(int file_descriptor) {
  connection_t *connection = connection_init(file_descriptor);
  assert(connection != NULL);
  return connection;
}

static void assert_values(int *values, uint32_t value_size) {
  assert(connection_list_get_size() == value_size);
  uint32_t index;
  struct pollfd *pollfd;

  pollfd = connection_list_get_listen_pollfd(); 
  assert(pollfd->fd == LISTEN_FILE_DESCRIPTOR);
  assert(pollfd->events == POLLIN);

  for (index = 0; index < value_size; index++) {
    assert(connection_get_file_descriptor(connection_list_get(index)) == values[index]);

    pollfd = connection_list_get_connection_pollfd(index); 
    assert(pollfd->fd == values[index]);
    assert(pollfd->events == POLLIN);
  }
}

int main() {
  it_opens();
  it_adds_and_gets();
  it_adds_and_gets_multiple();
  it_adds_and_deletes_head();
  it_adds_and_deletes_middle();
  it_adds_and_deletes_tail();
  it_adds_and_deletes_multiple();
  it_deletes_all_and_adds();
  it_reinserts_first();
  it_reinserts_middle();
  it_resizes_on_add();

  return 0;
}

