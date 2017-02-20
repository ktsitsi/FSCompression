#include "list_node.h"

/* Node of a generic doubly linked list.
 * data : pointer to the data
 * next, prev : pointers to adjacent nodes 
*/
struct list_node
{
	void *data;
	struct list_node *next;
	struct list_node *prev;
};

/* Create and initialize node with payload 'data'*/
void list_node_create(list_node_t **node, void *data)
{
	if (((*node) = (list_node_t*)malloc(sizeof(list_node_t))) == NULL) 
	{
		fprintf(stderr, "Failed to allocate list node\n");
		exit(EXIT_FAILURE);
	};
	(*node)->data = data; 
	(*node)->next = NULL;
	(*node)->prev = NULL;
}

/* Remove node after freeing his element using 'free_fn' function */
void list_node_delete(list_node_t **node, free_func free_fn) 
{
	if (free_fn != NULL) free_fn((*node)->data);
	(*node)->data = NULL;
	free(*node);
	*node = NULL;
}

/* Accessors and mutators */

void * list_node_get_data(list_node_t *node)
{
	return node->data;
}

void list_node_set_data(list_node_t *node, void *data)
{
	node->data = data;
}

list_node_t * list_node_get_next(list_node_t *node)
{
	return node->next;
}

void list_node_set_next(list_node_t *node, list_node_t *next)
{
	node->next = next;
}

list_node_t * list_node_get_prev(list_node_t *node)
{
	return node->prev;
}

void list_node_set_prev(list_node_t *node, list_node_t *prev)
{
	node->prev = prev;
}