#include <stdio.h>
#include <string.h>
#include "metadata.h"
#include "meta.h"
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

int create_hierarchical(list_t *filelist){
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



	free(cwd);
	return 0;
}