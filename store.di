|                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             #ifndef META_H
#define META_H

#include "list.h"
#include "metadata.h"
#include "./blocks/blocks.h"

int get_current_dir(char** cwd,char** current_dir);
int create_hierarchical(list_t* filelist, list_t* hierarchical,arc_header *hdr,int archive_fd);
void insert_hierarchical_rec(list_t * hierarhy_list,dinode* leafdinode,char* filepath,arc_header* hdr,int archive_fd);
void insert_hierarchical(list_t *hierarhy_list,char* insert_file_path,arc_header* hdr,int archive_fd);
void dinodes_list_init(list_t *hierarhy_list,char* current_dir);
#endif#ifndef METADATA_H
#define METADATA_H

#include <sys/types.h>
#include <sys/stat.h>
#include "list.h"
#include <unistd.h>
#include <linux/limits.h>
#define DENTRIES_NUM 16

typedef struct dinode{
	ino_t dinode_number;
	mode_t permissions;
	uid_t user_id;
	gid_t group_id;
	off_t total_size;
	off_t file_off;
	time_t time_of_access;
	list_t* dentry_list;
}dinode;

typedef struct entry{
	char filename[PATH_MAX];
	ino_t dinode_num;
	dinode * dinode_idx;
}entry;

typedef struct dentry{
	int length;
	entry tuple_entry[DENTRIES_NUM];
}dentry;

void dinode_stat_init(dinode* target,struct stat buffer);
#endif                                                                                                                                                                                                                                                                                                                                                                                                    