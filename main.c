#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parse.h"
#include "help.h"
#include "meta.h"
#include "metadata.h"

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
	
	//Test case for the functionality of argument line
	printf("Archive file :%s\n",archive);
	file_argument temp_file;
	/*while(list_get_len(filelist) != 0){
		list_dequeue(filelist,&temp_file);
		printf("File: %s\n",temp_file.filename);
	}*/

	list_t* hierarchical_list;
	create_hierarchical(filelist,&hierarchical_list);
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
	list_destroy(&filelist);
	list_destroy(&hierarchical_list);
	return 0;
}