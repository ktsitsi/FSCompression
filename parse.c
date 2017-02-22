#include "parse.h"
#include "help.h"
#include <string.h>
#include <stdio.h>

void init_choices(options* choices){
	choices->c = 0;
	choices->a = 0;
	choices->x = 0;
	choices->j = 0;
	choices->d = 0;
	choices->m = 0;
	choices->q = 0;
	choices->p = 0;
}
int parse(int argc, char** argv, options* choices, char** archive,list_t** filelist){
	
	list_t *arglist;
	list_create(&arglist,sizeof(flag_argument),free);
	flag_argument *oargument, *temp_argument;
	int number_of_flags = 0;

	//Searching for flags validity, count them, enqueue them
        int i;
	for(i=1; i<argc; i++){
		if(argv[i][0] == '-'){
			//This is flag
			if(strcmp(argv[i],"-c") && strcmp(argv[i],"-a") && strcmp(argv[i],"-x") && strcmp(argv[i],"-m") && strcmp(argv[i],"-q") && strcmp(argv[i],"-p") && strcmp(argv[i],"-j")){
				perror("Wrong argument flag type\n");
				printhelp();
				return -1;
			}
			oargument = (flag_argument*)malloc(sizeof(flag_argument));
			oargument->type = argv[i];
			list_enqueue(arglist,oargument);
		}
		else{
			number_of_flags = --i;
			break;
		}
	}

	//If more flags than the standard
	if(number_of_flags > NUM_OF_FLAGS){
		perror("Wrong number of flag arguments\n");
		printhelp();
		return -2;
	}

	//Recognize the flag list and initialize the options struct to store them
	init_choices(choices);
	list_iter_t* iter;
	list_iter_create(&iter);
	list_iter_init(iter,arglist,FORWARD);
	while((temp_argument = list_iter_next(iter)) != NULL){
		if(!strcmp(temp_argument->type,"-c")){
			choices->c = 1;
		}
		else if(!strcmp(temp_argument->type,"-a")){
			choices->a = 1;
		}
		else if(!strcmp(temp_argument->type,"-x")){
			choices->x = 1;
		}
		else if(!strcmp(temp_argument->type,"-m")){
			choices->m = 1;
		}
		else if(!strcmp(temp_argument->type,"-q")){
			choices->q = 1;
		}
		else if(!strcmp(temp_argument->type,"-p")){
			choices->p = 1;
		}
		else if(!strcmp(temp_argument->type,"-j")){
			choices->j = 1;
		}
	}
	list_iter_destroy(&iter);
	list_destroy(&arglist);
	//Store the destination archive file
	*archive = argv[number_of_flags + 1];
	
	//Store in a filelist the source files or dirs names to be archived
	file_argument *fargument;

	//Creates a list of files for the calling function and returns it in the filelist argument
	//destroy should locate on the outter process
	list_create(filelist,sizeof(file_argument),free);
	for(i = number_of_flags + 2; i<argc; i++){
		fargument = (file_argument*)malloc(sizeof(file_argument));
		fargument->filename = argv[i];
		list_enqueue(*filelist,fargument);
	}

	return 0;
}
