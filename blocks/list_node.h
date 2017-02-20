#ifndef LIST_NODE_H
#define LIST_NODE_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct list_node list_node_t;
typedef void (*free_func)(void *);	// Type of function for deleting data of nodes

void list_node_create(list_node_t **node, void *data);
void list_node_delete(list_node_t **node, free_func free_fn);

void * list_node_get_data(list_node_t *node);
void list_node_set_data(list_node_t *node, void *data);

list_node_t * list_node_get_next(list_node_t *node);
void list_node_set_next(list_node_t *node, list_node_t *next);

list_node_t * list_node_get_prev(list_node_t *node);
void list_node_set_prev(list_node_t *node, list_node_t *prev);

#endif 