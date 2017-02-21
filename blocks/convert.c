#include "metadata.h"
#include "metadata_disk.h"
#include "list.h"

typedef struct convert_node
{
    entry *_entry;
    off_t off;
} convert_node;

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
    list_create(&queue,sizeof(dinode),NULL);
    list_enqueue(queue,di_n);

    off_t write_off=0,new_off = 0;
    while(list_get_len(queue)) {
        dinode dn;
        dinode_disk dn_disk;
        list_pop(queue,&dn);

        dn_disk.dinode_number = dn.dinode_number;
        dn_disk.permissions = dn.permissions;
        dn_disk.user_id = dn.user_id;
        dn_disk.group_id = dn.group_id;
        dn_disk.time_of_access = dn.time_of_access;

        dn_disk.total_size = dn.total_size;
        dn_disk.file_off = dn.file_off;
        dn_disk.n_dentries = list_get_len(dn.dentry_list);

        memcpy(array,&dn_disk,sizeof(dinode_disk));
        write_off += sizeof(dinode_disk);
        new_off += sizeof(dinode_disk) + 
            list_get_len(dn.dentry_list)*sizeof(dentry_disk);

        dentry *de;
        list_iter_init(de_iter,dn.dentry_list,FORWARD);
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

void array_to_list(char *array,list_t **dinodes)
{
    list_create(dinodes,sizeof(dinode),free);

    list_t *stack;
    list_create(&stack,sizeof(convert_node),free);

    convert_node *cv = malloc(sizeof(convert_node));
    cv->_entry = NULL;
    cv->off=0;
    list_push(stack,cv);

    while(list_get_len(stack)) {
        convert_node cnode;
        list_pop(stack,&cnode);
        dinode_disk *di_d = (dinode_disk*)array + cnode.off;

        dinode *di = malloc(sizeof(dinode));
        if (cnode._entry != NULL) (cnode._entry)->dinode_idx = di;
        list_enqueue(*dinodes,di);

        di->dinode_number = di_d->dinode_number;
        di->permissions = di_d->permissions;
        di->user_id = di_d->user_id;
        di->group_id = di_d->group_id;
        di->time_of_access = di_d->time_of_access;

        di->total_size = di_d->total_size;
        di->file_off = di_d->file_off;

        list_create(&(di->dentry_list),sizeof(dentry),free);

        size_t n_dentries = di_d->n_dentries;
        dentry_disk *de_d = (dentry_disk*)(di_d + sizeof(dinode_disk));

        unsigned int i;
        for (i=0;i<n_dentries;++i){

            dentry *de = malloc(sizeof(dentry));
            de->length = de_d->length;

            unsigned int j;
            for (j=0;j<de_d->length;++j) {
                entry_disk *en_d = &(de_d->tuple_entry[j]);
                entry *en = &(de->tuple_entry[j]);

                strcpy(en->filename,en_d->filename);
                en->dinode_num = en_d->dinode_num;

                convert_node *cv = malloc(sizeof(convert_node));
                cv->off = en_d->dinode_off;
                cv->_entry = en;

                list_push(stack,cv);
            }
            list_enqueue(di->dentry_list,de);
            de_d += (sizeof(dentry_disk));
        }
    }
    list_destroy(&stack);
}
