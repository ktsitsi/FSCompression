#ifndef META_H
#define META_H

#include "list.h"
#include "metadata.h"
#include "./blocks/blocks.h"

int get_current_dir(char** cwd,char** current_dir);
int create_hierarchical(list_t* filelist, list_t* hierarchical,arc_header *hdr,int archive_fd);
void insert_hierarchical_rec(list_t * hierarhy_list,dinode* leafdinode,char* filepath,arc_header* hdr,int archive_fd);
void insert_hierarchical(list_t *hierarhy_list,char* insert_file_path,arc_header* hdr,int archive_fd);
void dinodes_list_init(list_t *hierarhy_list,char* current_dir);
#endif