#include "list.h"

/* 
 * Generic doubly linked list.
 * head,tail: pointers to first,last element
 * len : number of nodes
 * data_size: size of element data in bytes
 * free_fn: function for deleting data
*/
struct list
{
	list_node_t *head;
	list_node_t *tail;
	unsigned int len;
	unsigned int data_size;
	free_func free_fn;
};

/* 
 * Iterator for the list. 
 * prev,next: pointers to adjacent nodes at current position
 * direction: movement type
*/
struct list_iterator
{
	list_node_t *prev;
	list_node_t *next;
	iter_direction direction;
};

/* Create and initialize list */
void list_create(list_t **_list,unsigned int data_size, free_func free_fn)
{
	if (((*_list) = (list_t*)malloc(sizeof(list_t))) == NULL) {
		fprintf(stderr, "Failed to allocate linked list\n");
		exit(EXIT_FAILURE);
	};
	(*_list)->head = NULL; 
	(*_list)->tail = NULL;
	(*_list)->len = 0;
	(*_list)->data_size = data_size;
	(*_list)->free_fn = free_fn;
}

/* Delete list node */
static void list_delete(list_t *_list,list_node_t **node) 
{
	if ((!_list->len) || node == NULL) return;
	list_node_t *next_node,*prev_node;
	next_node = list_node_get_next(*node);
	prev_node = list_node_get_prev(*node);

	list_node_delete(node, _list->free_fn);

	// Adjust previous and next node
	if (prev_node != NULL) list_node_set_next(prev_node,next_node);
	else _list->head = next_node;
	
	if (next_node != NULL) list_node_set_prev(next_node,prev_node);
	else _list->tail = prev_node;
	
	_list->len--;
}

/* Delete list and all of its nodes */
void list_destroy(list_t **_list)
{
	while ((*_list)->len) {
		list_delete(*_list,&(*_list)->head);
	}
	free(*_list);
	*_list = NULL;
}

/* Insert element after node 'prev_node'*/
static void list_insert_after(list_t *_list, list_node_t * prev_node, void *element) 
{
	list_node_t *new_node, *next_node;
	list_node_create(&new_node,element);
	list_node_set_prev(new_node, prev_node);
	// Adjust previous and new node
	if (prev_node != NULL) {
		list_node_set_next(new_node,list_node_get_next(prev_node));
		list_node_set_next(prev_node,new_node);
	} else {
		list_node_set_next(new_node,_list->head);
		_list->head = new_node;
	}

	// Adjust next node too
	next_node = list_node_get_next(new_node);
	if (next_node != NULL) list_node_set_prev(next_node,new_node);
	else _list->tail = new_node;
	
	_list->len++;
}

// Specific insertions

void list_insert_after_iter(list_t *_list, list_iter_t * prev_iter, void *element) 
{
	list_insert_after(_list, prev_iter->prev, element);
}

void list_push(list_t *_list, void *element) 
{
	list_insert_after(_list, NULL, element);
}

void list_enqueue(list_t *_list, void *element) 
{
	list_insert_after(_list, _list->tail, element);
}

/* Insert to a sorted list keeping the list sorted */
void list_insert_sorted(list_t *_list, void *element, comp_func comp_fn) 
{
	list_node_t *cur = _list->head;
	if (cur == NULL || comp_fn(list_node_get_data(cur),element) >= 0 ) {
		list_push(_list,element);
	} else {
		// Find correct position to insert
		while(cur!=NULL && comp_fn(list_node_get_data(cur),element)<0) {
			cur = list_node_get_next(cur);
		}
		if (cur != NULL)list_insert_after(_list,list_node_get_prev(cur),element);
		else list_enqueue(_list,element);
	}
}

/* Specific deletion operations*/

void list_delete_at_iter(list_t *_list,list_iter_t *iter) 
{
	list_delete(_list,&iter->prev);
}

/* Remove first/last node and return their data*/

void list_pop(list_t *_list,void *front_data)
{
	if (!_list->len) return;
	memcpy(front_data,list_node_get_data(_list->head),_list->data_size);
	list_delete(_list,&_list->head);
}

void list_dequeue(list_t *_list,void *tail_data)
{
	if (!_list->len) return;
	memcpy(tail_data,list_node_get_data(_list->tail),_list->data_size);
	list_delete(_list,&_list->tail);
	
}

unsigned int list_get_len(list_t *_list)
{
	return _list->len;
}

/* Create an iterator used to visit and operate on nodes */
void list_iter_create(list_iter_t **iter)
{
	if (((*iter) = (list_iter_t*)malloc(sizeof(list_iter_t))) == NULL) {
		fprintf(stderr, "Failed to allocate list iterator\n");
		exit(EXIT_FAILURE);
	};
	
}

/* Position iterator correctly depenging on the direction */
void list_iter_init(list_iter_t *iter, list_t *_list,iter_direction dir)
{
	
	iter->direction = dir;
	iter->prev = NULL;
	if (_list != NULL) iter->next = dir == FORWARD ? _list->head : _list->tail;
	else iter->next = NULL;
}

void list_iter_destroy (list_iter_t **iter) 
{
	free (*iter);
	*iter = NULL;
}

/* Move to the next/previous node depending on direction*/
void * list_iter_next(list_iter_t *iter) 
{
	iter->prev = iter->next;
	if (iter->next != NULL) iter->next = iter->direction == FORWARD 
									? list_node_get_next(iter->next)
									: list_node_get_prev(iter->next);
	return iter->prev != NULL ? list_node_get_data(iter->prev): NULL;
}

/* Sort list in descending order using selection sort */
void list_sort(list_t *_list, comp_func comp_fn)
{
	if (_list->len < 2) return;
	list_node_t *head = _list->head, *i, *j;
	for (j=head;list_node_get_next(j)!=NULL;j=list_node_get_next(j)) {
		// Find the maximum element in the unsorted sublist of indexes j..n-1
		list_node_t *iMax = j;
		for (i = list_node_get_next(j);i!=NULL;i=list_node_get_next(i)) {
			if (comp_fn(list_node_get_data(i),list_node_get_data(iMax))>0) iMax = i;
		}
	
		if (iMax != j) {
			void *tmp = list_node_get_data(j);
			list_node_set_data(j,list_node_get_data(iMax));
			list_node_set_data(iMax,tmp);
		}
	}
}