#include <assert.h>
#include <stdio.h>

#include "double_link_list.h"

void double_link_list_init(double_link_list_t *list) {
  list->head = NULL;
  list->tail = NULL;
  list->size = 0;
}

void double_link_list_append(double_link_list_t *list, double_link_list_node_t *node) {
  assert(list != NULL);
  assert(node != NULL);

  node->previous = list->tail;
  node->next = NULL;

  if (list->size == 0) {
    list->head = node;
  } else {
    list->tail->next = node;
  }
  list->tail = node;

  list->size++;
}

void double_link_list_remove(double_link_list_t *list, double_link_list_node_t *node) {
  assert(list != NULL);
  assert(node != NULL);

  if (list->head == node) {
    list->head = node->next;
  } else {
    node->previous->next = node->next;
  }

  if (list->tail == node) {
    list->tail = node->previous;
  } else {
    node->next->previous = node->previous;
  }

  list->size--;
}

double_link_list_status_t double_link_list_pop_front(double_link_list_t *list, double_link_list_node_t **node) {
  assert(list != NULL);
  assert(node != NULL);

  if (list->size == 0) {
    return DOUBLE_LINK_LIST_STATUS__EMPTY;
  }

  *node = list->head;
  double_link_list_remove(list, list->head);

  return DOUBLE_LINK_LIST_STATUS__SUCCESS;
}

