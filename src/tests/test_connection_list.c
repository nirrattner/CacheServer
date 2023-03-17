#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "connection_list.h"
#include "test_util.h"

static connection_t *new_connection(int file_descriptor);
static void assert_values(int *values, uint16_t value_count);

// TODO: Test reinsert

void it_opens(void) {
  PRINT_TEST()
  int expected_values[] = {};
  uint8_t result = connection_list_open();
  assert(result == 0);
  assert(connection_list_get_count() == 0);
  assert_values(expected_values, 0);

  connection_list_close();
}

void it_appends_and_gets(void) {
  PRINT_TEST()

  int expected_values[] = {1};
  uint8_t result = connection_list_open();
  assert(result == 0);

  connection_t *connection = new_connection(1);
  connection_list_append(connection);

  assert(connection_list_get_head() == connection);
  assert(connection_list_get_tail() == connection);
  assert_values(expected_values, 1);

  free(connection);
  connection_list_close();
}

void it_appends_and_gets_multiple(void) {
  PRINT_TEST()

  int expected_values[] = {1, 2, 3};
  uint8_t result = connection_list_open();
  assert(result == 0);

  connection_t *connection_1 = new_connection(1);
  connection_t *connection_2 = new_connection(2);
  connection_t *connection_3 = new_connection(3);
  connection_list_append(connection_1);
  connection_list_append(connection_2);
  connection_list_append(connection_3);

  assert(connection_list_get_head() == connection_1);
  assert(connection_list_get_tail() == connection_3);
  assert_values(expected_values, 3);

  free(connection_1);
  free(connection_2);
  free(connection_3);
  connection_list_close();
}

void it_appends_and_deletes_head(void) {
  PRINT_TEST()

  int expected_values[] = {2, 3};
  uint8_t result = connection_list_open();
  assert(result == 0);

  connection_t *connection_1 = new_connection(1);
  connection_t *connection_2 = new_connection(2);
  connection_t *connection_3 = new_connection(3);
  connection_list_append(connection_1);
  connection_list_append(connection_2);
  connection_list_append(connection_3);

  connection_list_remove(connection_1);

  assert(connection_list_get_head() == connection_2);
  assert(connection_list_get_tail() == connection_3);
  assert_values(expected_values, 2);

  free(connection_1);
  free(connection_2);
  free(connection_3);
  connection_list_close();
}

void it_appends_and_deletes_middle(void) {
  PRINT_TEST()

  int expected_values[] = {1, 3};
  uint8_t result = connection_list_open();
  assert(result == 0);

  connection_t *connection_1 = new_connection(1);
  connection_t *connection_2 = new_connection(2);
  connection_t *connection_3 = new_connection(3);
  connection_list_append(connection_1);
  connection_list_append(connection_2);
  connection_list_append(connection_3);

  connection_list_remove(connection_2);

  assert(connection_list_get_head() == connection_1);
  assert(connection_list_get_tail() == connection_3);
  assert_values(expected_values, 2);

  free(connection_1);
  free(connection_2);
  free(connection_3);
  connection_list_close();
}

void it_appends_and_deletes_tail(void) {
  PRINT_TEST()

  int expected_values[] = {1, 2};
  uint8_t result = connection_list_open();
  assert(result == 0);

  connection_t *connection_1 = new_connection(1);
  connection_t *connection_2 = new_connection(2);
  connection_t *connection_3 = new_connection(3);
  connection_list_append(connection_1);
  connection_list_append(connection_2);
  connection_list_append(connection_3);

  connection_list_remove(connection_3);

  assert(connection_list_get_head() == connection_1);
  assert(connection_list_get_tail() == connection_2);
  assert_values(expected_values, 2);

  free(connection_1);
  free(connection_2);
  free(connection_3);
  connection_list_close();
}

void it_appends_and_deletes_multiple(void) {
  PRINT_TEST()

  int expected_values[] = {2};
  uint8_t result = connection_list_open();
  assert(result == 0);

  connection_t *connection_1 = new_connection(1);
  connection_t *connection_2 = new_connection(2);
  connection_t *connection_3 = new_connection(3);
  connection_list_append(connection_1);
  connection_list_append(connection_2);
  connection_list_append(connection_3);

  connection_list_remove(connection_1);
  connection_list_remove(connection_3);

  assert(connection_list_get_head() == connection_2);
  assert(connection_list_get_tail() == connection_2);
  assert_values(expected_values, 1);

  free(connection_1);
  free(connection_2);
  free(connection_3);
  connection_list_close();
}

void it_deletes_all_and_appends(void) {
  PRINT_TEST()

  int expected_values[] = {3};
  uint8_t result = connection_list_open();
  assert(result == 0);

  connection_t *connection_1 = new_connection(1);
  connection_t *connection_2 = new_connection(2);
  connection_t *connection_3 = new_connection(3);
  connection_list_append(connection_1);
  connection_list_append(connection_2);
  connection_list_remove(connection_1);
  connection_list_remove(connection_2);

  connection_list_append(connection_3);

  assert(connection_list_get_head() == connection_3);
  assert(connection_list_get_tail() == connection_3);
  assert_values(expected_values, 1);

  free(connection_1);
  free(connection_2);
  free(connection_3);
  connection_list_close();
}

static connection_t *new_connection(int file_descriptor) {
  connection_t *connection = connection_init(file_descriptor);
  assert(connection != NULL);
  return connection;
}

static void assert_values(int *values, uint16_t value_count) {
  assert(connection_list_get_count() == value_count);
  connection_t *current_connection = connection_list_get_head();
  connection_t *next_connection;
  uint16_t index;

  for (index = 0; index < value_count; index++) {
    assert(values[index] == current_connection->file_descriptor);
    next_connection = connection_get_next(current_connection);
    if (index == value_count - 1) {
      assert(next_connection == NULL);
    } else {
      assert(connection_get_previous(next_connection) == current_connection);
    }
    current_connection = next_connection;
  }
}

int main() {
  it_opens();
  it_appends_and_gets();
  it_appends_and_gets_multiple();
  it_appends_and_deletes_head();
  it_appends_and_deletes_middle();
  it_appends_and_deletes_tail();
  it_appends_and_deletes_multiple();
  it_deletes_all_and_appends();

  return 0;
}

