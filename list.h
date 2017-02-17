#ifndef LIST_H
#define LIST_H
#include "list_node.h"

typedef enum {FORWARD,BACKWARD} iter_direction;
typedef struct list list_t;
typedef struct list_iterator list_iter_t;

typedef int (*comp_func)(void *,void *);	// Type of function for comparisons of data of nodes

void list_create(list_t **_list,unsigned int data_size, free_func free_func);
void list_destroy(list_t **_list);

void list_insert_after_iter(list_t *_list, list_iter_t * prev_iter, void *element) ;
void list_insert_sorted(list_t *_list, void *element, comp_func comp_fn);
void list_delete_at_iter(list_t *_list,list_iter_t *iter) ;

void list_push(list_t *_list, void *element);
void list_pop(list_t *_list,void *front_data);

void list_enqueue(list_t *_list, void *element);
void list_dequeue(list_t *_list,void *tail_data);

unsigned int list_get_len(list_t *_list);

void list_iter_create(list_iter_t **iter);
void list_iter_init(list_iter_t *iter, list_t *_list,iter_direction dir);
void list_iter_destroy (list_iter_t **iter);
void * list_iter_next(list_iter_t *iter);

void list_sort(list_t *_list, comp_func comp_fn);

#endif 