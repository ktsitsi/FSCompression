#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "metadata.h"
#include "parse_data.h"
#include "meta.h"
#include <linux/limits.h>
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

char* normalize_path(char* resolved_path, char* cwd){
	char *rtoken;
	char *cwdtoken;
	char *store_resolved = (char*)calloc(PATH_MAX,sizeof(char));
	rtoken = strtok(resolved_path,"/\n");
	cwdtoken = strtok(cwd,"/\n");
	int counter = 0;
	while(!strcmp(rtoken, cwdtoken)){
	    /* Update strtok pointers */
	    rtoken += strlen(rtoken) + 1;
	    sprintf(store_resolved,"%s",rtoken);
	    cwdtoken += strlen(cwdtoken) + 1;
	    /* Get next token */
	    rtoken = strtok(rtoken, "/\n");
	    cwdtoken = strtok(cwdtoken, "/\n");
	    counter++;
	    if(cwdtoken == NULL && rtoken != NULL){
	    	//resolved path is PREDECESSOR of cwd
	    	break;
	    }
	    if(rtoken == NULL && cwdtoken == NULL){
	    	//resoved is the same path with cwd
	    	break;
	    }
	    if(rtoken == NULL && cwdtoken != NULL){
	    	//resolved path is ancestor of the cwd FAILURE
	    	return NULL;
	    }
	}

	if(rtoken != NULL && cwdtoken != NULL){
		//different branch of two paths
		return NULL;
	}
	//printf("Similar to depth %d\n",counter);
	return store_resolved;
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

int create_hierarchical(list_t *filelist, list_t** hierarhy_list){
	char *cwd = (char*)malloc(CWD_LENGTH*sizeof(char));
	char *current_dir;
	if(get_current_dir(&cwd,&current_dir) < 0){
		return -1;
	}
	//Initialize dinode list with 2 first naive nodes of pseudo and cwd
	dinodes_list_init(hierarhy_list,current_dir);
	
	list_iter_t *file_iterator;
	list_iter_create(&file_iterator);
	list_iter_init(file_iterator,filelist,FORWARD);

	//For al files in the command line
	file_argument* cur_file;
	printf("Length list:%d\n",list_get_len(filelist));
	while((cur_file = (file_argument*)list_iter_next(file_iterator)) != NULL){
		//find real path of this argument
		char *resolved_path = (char*)calloc(PATH_MAX,sizeof(char));
        realpath(cur_file->filename, resolved_path); 
       	char* normalized_filepath; 			//!!!!!!!!!carefull to free normalized path!!!!!!!!!
		normalized_filepath = normalize_path(resolved_path,cwd);
		if(normalized_filepath != NULL){
			if(!strcmp(normalized_filepath,"")){
				printf("Same directory\n");
			}
			else{
				printf("The path in the hierarchy is: %s\n",normalized_filepath);
			}
		}
		else{
			printf("Error in normalization of the path\n");
			return -2;
		}

		

		free(resolved_path);
		free(normalized_filepath);
	}

	list_iter_destroy(&file_iterator);
	free(cwd);
	return 0;
}