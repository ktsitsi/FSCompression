#include <stdio.h>
#include <string.h>
#include "metadata.h"
#include "meta.h"
#include <inttypes.h>

#define CWD_LENGTH 128

int get_current_dir(char** cwd,char** current_dir){
	if(getcwd(*cwd,CWD_LENGTH) == NULL){
		perror("Couldn't retrieve cwd\n");
		return -1;
	}
	char* path_part;
	path_part = strtok(*cwd,"/\n");
	while(path_part != NULL){
		*current_dir = path_part;
		path_part = strtok(NULL,"/\n");
	}
	return 0;
}

void dinodes_list_init(list_t **hierarhy_list,char* current_dir){

	//Initialize the current directory dinode
	dinode* cwd_din = (dinode*)malloc(sizeof(dinode));
	cwd_din->dinode_number = 1;
	//Take all the necessary info from lstat
	
	//Init the cyclic reference of the cwd with 2 entries in the dentry
	dentry *cwdidx = (dentry*)malloc(sizeof(dentry));
	strcpy((cwdidx->tuple_entry[0]).filename,".");
	(cwdidx->tuple_entry[0]).dinode_num = 1;
	(cwdidx->tuple_entry[0]).dinode_idx = cwd_din;
	strcpy((cwdidx->tuple_entry[1]).filename,"..");
	(cwdidx->tuple_entry[1]).dinode_num = 1;
	(cwdidx->tuple_entry[1]).dinode_idx = cwd_din;

	//Enqueue the dentry in the list of index of the cwd
	list_create(&(cwd_din->dentry_list),sizeof(dentry),free);
	list_enqueue(cwd_din->dentry_list,cwdidx);

	//Create-Init the first pseudoroot dinode
	dinode* pseudoroot = (dinode*)malloc(sizeof(dinode));
	pseudoroot->dinode_number = 0;

	//Create the list with one dentry element for pseudoroot
	//Init first index to cwd in the pseudoroot
	dentry *rootindex = (dentry*)malloc(sizeof(dentry));
	strcpy((rootindex->tuple_entry[0]).filename,current_dir);
	(rootindex->tuple_entry[0]).dinode_num = 1;
	(rootindex->tuple_entry[0]).dinode_idx = cwd_din;

	//Enqueue the dentry of pseudo root in the dinode of pseudoroot
	list_create(&(pseudoroot->dentry_list),sizeof(dentry),free);
	list_enqueue(pseudoroot->dentry_list,rootindex);

	//Create the first 2 nodes of the hierarchy list of dinodes
	list_create(hierarhy_list,sizeof(dinode),free);
	list_enqueue(*hierarhy_list,pseudoroot);
	list_enqueue(*hierarhy_list,cwd_din);

}

/*test_function(){

	list_iter_t *hier_iter;
	list_iter_create(&hier_iter);
	list_iter_init(hier_iter,*hierarhy_list,FORWARD);

	dinode *temp;
	list_iter_t *dentry_iter;
	list_iter_create(&dentry_iter);
	dentry *temp2;
	while((temp = list_iter_next(hier_iter)) != NULL){
		printf("Dinode number: %ju\n", (uintmax_t)temp->dinode_number);
		list_iter_init(dentry_iter,temp->dentry_list,FORWARD);
		while((temp2 = list_iter_next(dentry_iter)) != NULL){
			printf("Filename0:%s\n",(temp2->tuple_entry[0]).filename);
			printf("Filename1:%s\n",(temp2->tuple_entry[1]).filename);
		}
	}

	list_iter_destroy(&hier_iter);
	list_iter_destroy(&dentry_iter);
}*/

int create_hierarchical(list_t *filelist, list_t** hierarhy_list){
	char *cwd = (char*)malloc(CWD_LENGTH*sizeof(char));
	char *current_dir;
	if(get_current_dir(&cwd,&current_dir) < 0){
		return -1;
	}

	//printf("Current working dir is %s\n",current_dir);

	//Create a picture of the current state of the filesystem
	//starting from that cwd as root

	//This representation will be a list of dinodes and dentries
	//all stored in memory with pointers and after that will be converted
	//in blocks pointers->offsets so it can be stored in the archive file

	//Now begin from cwd and first create the dinodes for the predecessors
	//and link them in the list in a recursive manner for all until the end of depth
	//predecessors are coming from the filelist

	//Initialize dinode list with 2 first naive nodes of pseudo and cwd
	dinodes_list_init(hierarhy_list,current_dir);

	free(cwd);
	return 0;
}