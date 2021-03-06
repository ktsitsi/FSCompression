#include "list.h"
#include "metadata.h"
#include <sys/types.h>
#include <unistd.h>


typedef struct dinode_disk
{
    ino_t dinode_number;
    mode_t permissions;
    uid_t user_id;
    gid_t group_id;
    time_t time_of_access;

    off_t total_size;
    off_t file_off;
    size_t n_dentries;
} dinode_disk;

typedef struct entry_disk
{
    char filename[256];
    ino_t dinode_num;
    off_t dinode_off;
} entry_disk;

typedef struct dentry_disk {
    int length;
    entry_disk tuple_entry[DENTRIES_NUM];
} dentry_disk;
