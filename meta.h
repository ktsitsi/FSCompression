#ifndef META_H
#define META_H

#include "list.h"

int get_current_dir(char** cwd,char** current_dir);
int create_hierarchical(list_t* filelist, list_t** hierarchical);
void dinodes_list_init(list_t **hierarhy_list,char* current_dir);
#endif