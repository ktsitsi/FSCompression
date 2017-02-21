#include "metadata.h"
#include "metadata_disk.h"
#include "list.h"

off_t list_disk_size(list_t *dinodes)
{
    list_iter_t *iter;
    list_iter_create(&iter);
    list_iter_init(iter,dinodes,FORWARD);

    off_t total_size = 0;
    dinode *di_n;

    while((di_n=(dinode*)list_iter_next(iter))!=NULL) 
        total_size += sizeof(dinode_disk) + 
            list_get_len(di_n->dentry_list) * sizeof(dentry_disk);

    list_iter_destroy(&iter);
}

void list_to_array(list_t *dinodes,char *array) 
{
    list_iter_t *di_iter;
    list_iter_create(&di_iter);
    list_iter_init(di_iter,dinodes,FORWARD);
    dinode *di_n = (dinode*)list_iter_next(di_iter);

    list_iter_t *de_iter;
    list_iter_create(&de_iter);

    list_t *queue;
    list_create(&queue,sizeof(dinode*),free);
    list_enqueue(queue,di_n);

    off_t write_off=0,new_off = 0;
    while(list_get_len(queue)) {
        dinode *dn;
        dinode_disk dn_disk;
        list_pop(queue,&dn);

        dn_disk.dinode_number = dn->dinode_number;
        dn_disk.permissions = dn->permissions;
        dn_disk.user_id = dn->user_id;
        dn_disk.group_id = dn->group_id;
        dn_disk.time_of_access = dn->time_of_access;

        dn_disk.total_size = dn->total_size;
        dn_disk.file_off = dn->file_off;
        dn_disk.n_dentries = list_get_len(dn->dentry_list);

        memcpy(array,&dn_disk,sizeof(dinode_disk));
        write_off += sizeof(dinode_disk);
        new_off += sizeof(dinode_disk) + 
            list_get_len(dn->dentry_list)*sizeof(dentry_disk);

        dentry *de;
        list_iter_init(de_iter,dn->dentry_list,FORWARD);
        while((de=(dentry*)list_iter_next(de_iter)) != NULL) {
            dentry_disk de_d;
            de_d.length = de->length;
            entry_disk entries[DENTRIES_NUM];
            unsigned int i;
            entry *en;
            for (i=0;i<de->length;++i) {
                en = &(de->tuple_entry[i]);
                strcpy(entries[i].filename,en->filename);
                entries[i].dinode_num = en->dinode_num;
                if (!strcpy(en->filename,".") || !strcpy(en->filename,"..")) continue;
                entries[i].dinode_off = new_off;
                new_off += sizeof(dinode_disk) + list_get_len(en->dinode_idx->dentry_list);
                list_enqueue(queue,en->dinode_idx);
            }
            memcpy(&(de_d.tuple_entry),entries,DENTRIES_NUM*sizeof(entry_disk));
            memcpy(array+write_off,&de_d,sizeof(dentry_disk));
            write_off += sizeof(dentry_disk);
        }
    }

    list_iter_destroy(&di_iter);
    list_iter_destroy(&de_iter);
    list_destroy(&queue);
}
