#ifndef CONVERT_H
#define CONVERT_H

#include "list.h"

off_t list_disk_size(list_t *dinodes);
void list_to_array(list_t *dinodes,char *array);
void array_to_list(char *array,list_t **dinodes);

#endif
