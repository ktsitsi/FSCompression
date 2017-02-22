�                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             #ifndef METADATA_H
#define METADATA_H

#include <sys/types.h>
#include "list.h"
#include <unistd.h>
#define DENTRIES_NUM 16

typedef struct dinode{
	ino_t dinode_number;
	mode_t permissions;
	uid_t user_id;
	gid_t group_id;
	time_t time_of_access;

	off_t total_size;
    off_t file_off;
	list_t* dentry_list;
}dinode;

typedef struct entry{
	char filename[256];
	ino_t dinode_num;
	dinode* dinode_idx;
}entry;

typedef struct dentry{
	int length;
	entry tuple_entry[DENTRIES_NUM];
}dentry;

#endif
             H
#define META_H

#include "list.h"

int get_current_dir(char** cwd,char** current_dir);
int create_hierarchical(list_t* filelist, list_t** hierarchical);
void dinodes_list_init(list_t **hierarhy_list,char* current_dir);
#endif                                                                                                                                                                                                                                                                                             y{
	int length;
	entry tuple_entry[DENTRIES_NUM];
}dentry;

void dinode_stat_init(dinode* target,struct stat buffer);
#endif                                                                                                                                                                                                                                                                                                                                                                                                    