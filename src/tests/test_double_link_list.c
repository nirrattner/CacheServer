#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "double_link_list.h"
#include "test_util.h"

static double_link_list_node_t *generate_int_node(int value);
static void assert_values(double_link_list_t *list, size_t expected_size, ...);

// TODO: Clean up memory leaks of allocating nodes and values

static void it_appends_nodes(void) {
  PRINT_TEST()

  double_link_list_t list;
  double_link_list_init(&list);
  
  double_link_list_append(&list, generate_int_node(11));
  double_link_list_append(&list, generate_int_node(12));
  double_link_list_append(&list, generate_int_node(13));
  double_link_list_append(&list, generate_int_node(14));
  double_link_list_append(&list, generate_int_node(15));

  assert_values(&list, 5, 11, 12, 13, 14, 15);
}

static void it_removes_head_node(void) {
  PRINT_TEST()

  double_link_list_t list;
  double_link_list_init(&list);

  double_link_list_node_t *removed_node = generate_int_node(11);
  
  double_link_list_append(&list, removed_node);
  double_link_list_append(&list, generate_int_node(12));
  double_link_list_append(&list, generate_int_node(13));
  double_link_list_append(&list, generate_int_node(14));
  double_link_list_append(&list, generate_int_node(15));

  double_link_list_remove(&list, removed_node);

  assert_values(&list, 4, 12, 13, 14, 15);
}

static void it_removes_middle_node(void) {
  PRINT_TEST()

  double_link_list_t list;
  double_link_list_init(&list);

  double_link_list_node_t *removed_node = generate_int_node(13);
  
  double_link_list_append(&list, generate_int_node(11));
  double_link_list_append(&list, generate_int_node(12));
  double_link_list_append(&list, removed_node);
  double_link_list_append(&list, generate_int_node(14));
  double_link_list_append(&list, generate_int_node(15));

  double_link_list_remove(&list, removed_node);

  assert_values(&list, 4, 11, 12, 14, 15);
}

static void it_removes_tail_node(void) {
  PRINT_TEST()

  double_link_list_t list;
  double_link_list_init(&list);

  double_link_list_node_t *removed_node = generate_int_node(15);
  
  double_link_list_append(&list, generate_int_node(11));
  double_link_list_append(&list, generate_int_node(12));
  double_link_list_append(&list, generate_int_node(13));
  double_link_list_append(&list, generate_int_node(14));
  double_link_list_append(&list, removed_node);

  double_link_list_remove(&list, removed_node);

  assert_values(&list, 4, 11, 12, 13, 14);
}

static void it_pops_front(void) {
  PRINT_TEST()

  double_link_list_t list;
  double_link_list_init(&list);

  double_link_list_node_t *popped_node;

  double_link_list_append(&list, generate_int_node(11));
  double_link_list_append(&list, generate_int_node(12));
  double_link_list_append(&list, generate_int_node(13));
  double_link_list_append(&list, generate_int_node(14));
  double_link_list_append(&list, generate_int_node(15));

  double_link_list_pop_front(&list, &popped_node);

  assert(*((int *)popped_node->data) == 11);
  assert_values(&list, 4, 12, 13, 14, 15);
}

static double_link_list_node_t *generate_int_node(int value) {
  double_link_list_node_t *node = (double_link_list_node_t *)malloc(sizeof(double_link_list_node_t));
  node->data = malloc(sizeof(int));
  *((int *)node->data) = value;
  return node;
}

static void assert_values(double_link_list_t *list, size_t expected_size, ...) {
  assert(list->size == expected_size);

  double_link_list_node_t *current_node = list->head;

  va_list expected_values;
  va_start(expected_values, expected_size);

  for (size_t index = 0; index < list->size; index++) {
    int expected_value = va_arg(expected_values, int);
    assert(expected_value == *((int *)current_node->data));
    if (current_node->next) {
      assert(current_node == current_node->next->previous);
    }
    current_node = current_node->next;
  }
  
  va_end(expected_values);
}

int main() {
  it_appends_nodes();
  it_removes_head_node();
  it_removes_middle_node();
  it_removes_tail_node();
  it_pops_front();

  return 0;
}

