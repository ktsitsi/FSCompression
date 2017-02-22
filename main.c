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
        printf("dINODEOFF %jd\n",dinode_off);
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
                printf("Current %s\n",entries[i].filename);
                printf("Dinode Pointers %p:\n",en->dinode_idx);
                entry_off += sizeof(dinode_disk) + list_get_len(en->dinode_idx->dentry_list);
                list_enqueue(queue,en->dinode_idx);
            }
            memcpy(&(de_d.tuple_entry),entries,DENTRIES_NUM*sizeof(entry_disk));
            memcpy(array+dentry_off,&de_d,sizeof(dentry_disk));
            printf("dentryOFF %jd\n",dentry_off);
            dentry_off += sizeof(dentry_disk);
        }
    }

    list_iter_destroy(&di_iter);
    list_iter_destroy(&de_iter);
    list_destroy(&queue);
	


	//Array to list

    list_t *newhlist;
    array_to_list(array,&newhlist);

    printf("~~~~~~~~~~~~~~~~\n");
    list_iter_t* hier_iter;
    list_iter_create(&hier_iter);
	list_iter_init(hier_iter,newhlist,FORWARD);
	dinode* current;

	list_iter_t* dentries_iter;
	list_iter_create(&dentries_iter);
	printf("Size of list is %d\n",list_get_len(newhlist));
	dentry* current_dentry;
	int i;
	while((current = list_iter_next(hier_iter)) != NULL){
		list_iter_init(dentries_iter,current->dentry_list,FORWARD);
		while((current_dentry = list_iter_next(dentries_iter))!= NULL){
			printf("The length is %d\n",current_dentry->length);
			for(i=0;i<current_dentry->length;i++){
				printf("File %d: %s\n",i,(current_dentry->tuple_entry[i]).filename);
			}
		}
	}

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


//Print the hierarchy list
	/*list_iter_t *hier_iter;
	list_iter_create(&hier_iter);
	list_iter_init(hier_iter,hierarchical_list,FORWARD);
	dinode* current;

	list_iter_t* dentries_iter;
	list_iter_create(&dentries_iter);
	dentry* current_dentry;
	int i;
	while((current = list_iter_next(hier_iter)) != NULL){
		list_iter_init(dentries_iter,current->dentry_list,FORWARD);
		while((current_dentry = list_iter_next(dentries_iter))!= NULL){
			printf("The length is %d\n",current_dentry->length);
			for(i=0;i<current_dentry->length;i++){
				printf("File %d: %s\n",i,(current_dentry->tuple_entry[i]).filename);
			}
		}
	}
	list_iter_destroy(&hier_iter);*/