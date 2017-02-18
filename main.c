#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parse.h"
#include "help.h"
#include "meta.h"

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
	/*printf("Archive file :%s\n",archive);
	file_argument temp_file;
	while(list_get_len(filelist) != 0){
		list_dequeue(filelist,&temp_file);
		printf("File: %s\n",temp_file.filename);
	}*/
	//list_destroy(&filelist);
	create_hierarchical(filelist);

	list_destroy(&filelist);
	return 0;
}