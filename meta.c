#include <stdio.h>
#include <string.h>
#include "metadata.h"
#include "meta.h"

int get_current_dir(char** cwd,char** current_dir){
	if(getcwd(*cwd,sizeof(*cwd)) == NULL){
		perror("Couldn't retrieve cwd\n");
		return -1;
	}
	char* path_part;
	path_part = strtok(*cwd,"/\n");
	while(path_part != NULL){
		*current_dir = path_part;
		path_part = strtok(NULL,"/\n");
	}
	//printf("Current working dir is %s\n",current_dir);
	return 0;
}

int create_hierarchical(Queue<file_argument> filelist){
	char cwd[256];
	char* current_dir;
	if(get_current_dir(&cwd,&current_dir) < 0){
		return -1;
	}
	//In current dir is the current directory located

	return 0;
}