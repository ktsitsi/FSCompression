#include <sys/types.h>
#include <unistd.h>
#define DENTRIES_NUM 16

typedef struct dinode{
	ino_t dinode_number;
	mode_t permissions;
	uid_t user_id;
	gid_t group_id;
	off_t total_size;
	time_t time_of_access;
}dinode;

typedef struct entry{
	char filename[256];
	dinode* dinode_ptr;
}entry;

typedef struct dentry{
	entry tuple_entry[DENTRIES_NUM];
}dentry;