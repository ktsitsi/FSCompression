#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "parse.h"
#include "help.h"
#include <inttypes.h>
#include "meta.h"
#include "./blocks/convert.h"
#include "metadata.h"

#include "./blocks/metadata_disk.h"
#define ARGNUM 4
typedef struct convert_node
{
    entry *_entry;
    off_t off;
} convert_node;

int main(int argc, char** argv){

	//Defining functionality of the programm according command line
	options choices;
	char* archive;
	list_t *filelist;
	if(parse(argc,argv,&choices,&archive,&filelist) < 0){
		printhelp();
		return -2;
	}

	//Option j can be called only along -c or -a
	if(choices.j){
		if(!choices.c && !choices.a){
			perror("Flag -j can only be used along with flag -c or -a\n");
			return -3;
		}
	}
	
	list_t* hierarchical_list;
	
	arc_header hdr;
	header_init(&hdr);
	//WARNING NOT TO FORGET TO CHANGE IT IN THE APPEND

	int archive_fd = open(archive,O_CREAT|O_RDWR,0644);
	if(archive_fd <0) return -4;
	//WARNING WITH ARCHIVE FILE o_create

	list_create(&hierarchical_list,sizeof(dinode),free);
	create_hierarchical(filelist,hierarchical_list,&hdr,archive_fd);

	list_iter_t *hier_iter;
	list_iter_create(&hier_iter);
	list_iter_init(hier_iter,hierarchical_list,FORWARD);
	dinode* current;

	list_iter_t* dentries_iter;
	list_iter_create(&dentries_iter);
	dentry* current_dentry;
	int i;

	//PRINT LIST
	/*list_iter_init(hier_iter,hierarchical_list,FORWARD);
	printf("Size of list is %d\n",list_get_len(hierarchical_list));
	while((current = list_iter_next(hier_iter)) != NULL){
		list_iter_init(dentries_iter,current->dentry_list,FORWARD);
		while((current_dentry = list_iter_next(dentries_iter))!= NULL){
			printf("The length is %d\n",current_dentry->length);
			for(i=0;i<current_dentry->length;i++){
				printf("File %d: %s\n",i,(current_dentry->tuple_entry[i]).filename);
			}
		}
	}*/


	char* array = (char*)malloc(list_array_size(hierarchical_list)*sizeof(char));
	//List To Array
	list_iter_t *di_iter;
    list_iter_create(&di_iter);
    list_iter_init(di_iter,hierarchical_list,FORWARD);
    dinode *di_n = (dinode*)list_iter_next(di_iter);

    list_iter_t *de_iter;
    list_iter_create(&de_iter);

    list_t *queue;
    list_create(&queue,sizeof(dinode),NULL);
    list_enqueue(queue,di_n);

    off_t dentry_off=0,dinode_off = 0;
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

        memcpy(array+dinode_off,&dn_disk,sizeof(dinode_disk));
        dentry_off = dinode_off + sizeof(dinode_disk);
        dinode_off += sizeof(dinode_disk) + 
            list_get_len(dn.dentry_list)*sizeof(dentry_disk);

        dentry *de;
        list_iter_init(de_iter,dn.dentry_list,FORWARD);
        while((de=(dentry*)list_iter_next(de_iter)) != NULL) {
            dentry_disk de_d;
            de_d.length = de->length;
            entry_disk entries[DENTRIES_NUM];
            unsigned int i;
            entry *en;
            off_t entry_off = dinode_off;
            for (i=0;i<de->length;++i) {
                en = &(de->tuple_entry[i]);
                strcpy(entries[i].filename,en->filename);
                entries[i].dinode_num = en->dinode_num;
                if (!strcmp(en->filename,".") || !strcmp(en->filename,"..")) continue;
                //off_t entry_off = dinode_off;
                entries[i].dinode_off = entry_off;
                entry_off += sizeof(dinode_disk) + list_get_len(en->dinode_idx->dentry_list)*sizeof(dentry_disk);
                list_enqueue(queue,en->dinode_idx);
            }
            memcpy(&(de_d.tuple_entry),entries,DENTRIES_NUM*sizeof(entry_disk));
            memcpy(array+dentry_off,&de_d,sizeof(dentry_disk));
            dentry_off += sizeof(dentry_disk);
        }
    }

    list_iter_destroy(&di_iter);
    list_iter_destroy(&de_iter);
    list_destroy(&queue);
	


	//Array to list

    list_t *newhlist;

    list_create(&newhlist,sizeof(dinode),free);

    list_t *stack;
    list_create(&stack,sizeof(convert_node),free);

    convert_node *cv = malloc(sizeof(convert_node));
    cv->_entry = NULL;
    cv->off=0;
    list_push(stack,cv);

    while(list_get_len(stack)) {
        convert_node cnode;
        list_pop(stack,&cnode);
        dinode_disk *di_d = (dinode_disk*)(array + cnode.off);
        dinode *di = malloc(sizeof(dinode));
        if (cnode._entry != NULL) (cnode._entry)->dinode_idx = di;
        list_enqueue(newhlist,di);

        di->dinode_number = di_d->dinode_number;
        di->permissions = di_d->permissions;
        di->user_id = di_d->user_id;
        di->group_id = di_d->group_id;
        di->time_of_access = di_d->time_of_access;

        di->total_size = di_d->total_size;
        di->file_off = di_d->file_off;

        list_create(&(di->dentry_list),sizeof(dentry),free);

        size_t n_dentries = di_d->n_dentries;
        dentry_disk *de_d = (dentry_disk*)((char*)di_d + sizeof(dinode_disk));

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

                if (!strcmp(en_d->filename,".") || !strcmp(en_d->filename,"..")) continue;
                convert_node *cv = malloc(sizeof(convert_node));
                cv->off = en_d->dinode_off;
                cv->_entry = en;

                list_push(stack,cv);
            }
            list_enqueue(di->dentry_list,de);
            de_d = (dentry_disk*)((char*)de_d + (sizeof(dentry_disk)));
        }
    }
    list_destroy(&stack);

    printf("~~~~~~~~~~~~~~~~\n");

    //BACK TO THE ORIGINAL
    //PRINT LIST
    /*list_iter_init(hier_iter,newhlist,FORWARD);
	printf("Size of list is %d\n",list_get_len(newhlist));
	while((current = list_iter_next(hier_iter)) != NULL){
		list_iter_init(dentries_iter,current->dentry_list,FORWARD);
		while((current_dentry = list_iter_next(dentries_iter))!= NULL){
			printf("The length is %d\n",current_dentry->length);
			for(i=0;i<current_dentry->length;i++){
				printf("File %d: %s\n",i,(current_dentry->tuple_entry[i]).filename);
			}
		}
	}*/

	//DESTROY
    list_iter_init(hier_iter,hierarchical_list,FORWARD);
	while((current = list_iter_next(hier_iter)) != NULL){
		list_destroy(&(current->dentry_list));
	}
	list_iter_destroy(&hier_iter);
	list_destroy(&filelist);
	list_destroy(&hierarchical_list);
	
	free(array);
	header_write(&hdr,archive_fd);
	close(archive_fd);
	return 0;
}