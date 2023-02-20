#ifndef DOUBLE_LINK_LIST_H
#define DOUBLE_LINK_LIST_H

#include <stdint.h>

typedef enum {
  DOUBLE_LINK_LIST_STATUS__SUCCESS,
  DOUBLE_LINK_LIST_STATUS__EMPTY,
} double_link_list_status_t;

typedef struct double_link_list_node {
  struct double_link_list_node *previous;
  struct double_link_list_node *next;
  void *data;
} double_link_list_node_t;

typedef struct {
  double_link_list_node_t *head;
  double_link_list_node_t *tail;
  size_t size;
} double_link_list_t;

void double_link_list_init(double_link_list_t *list);
void double_link_list_append(double_link_list_t *list, double_link_list_node_t *node);
void double_link_list_remove(double_link_list_t *list, double_link_list_node_t *node);
double_link_list_status_t double_link_list_pop_front(double_link_list_t *list, double_link_list_node_t **node);

#endif

