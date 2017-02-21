#include "metadata.h"

void dinode_stat_init(dinode* target,struct stat buffer){
	target->dinode_number = buffer.st_ino;
	target->permissions = buffer.st_mode;
	target->user_id = buffer.st_uid;
	target->group_id = buffer.st_gid;
	target->total_size = buffer.st_size;
	target->time_of_access = buffer.st_atime;
}