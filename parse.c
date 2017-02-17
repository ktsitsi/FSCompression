#include "parse.h"
#include "help.h"
#include <string.h>
#include <stdio.h>

int parse(int argc, char** argv, options* choices, char** archive,Queue<file_argument>* filelist){
	
	Queue<flag_argument> arglist(QUEUE_SIZE);
	flag_argument oargument, temp_argument;
	int number_of_flags = 0;

	//Searching for flags validity, count them, enqueue them
	for(int i=1; i<argc; i++){
		if(argv[i][0] == '-'){
			//This is flag
			if(strcmp(argv[i],"-c") && strcmp(argv[i],"-a") && strcmp(argv[i],"-x") && strcmp(argv[i],"-m") && strcmp(argv[i],"-d") && strcmp(argv[i],"-p") && strcmp(argv[i],"-j")){
				perror("Wrong argument flag type\n");
				printhelp();
				return -1;
			}
			oargument.type = argv[i];
			arglist.Enqueue(oargument);
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
	while(!arglist.is_empty()){
		temp_argument = arglist.get_front();
		if(!strcmp(temp_argument.type,"-c")){
			choices->c = 1;
		}
		else if(!strcmp(temp_argument.type,"-a")){
			choices->a = 1;
		}
		else if(!strcmp(temp_argument.type,"-x")){
			choices->x = 1;
		}
		else if(!strcmp(temp_argument.type,"-m")){
			choices->m = 1;
		}
		else if(!strcmp(temp_argument.type,"-d")){
			choices->d = 1;
		}
		else if(!strcmp(temp_argument.type,"-p")){
			choices->p = 1;
		}
		else if(!strcmp(temp_argument.type,"-j")){
			choices->j = 1;
		}
		arglist.Pop();
	}

	//Store the destination archive file
	*archive = argv[number_of_flags + 1];

	//Store in a filelist the source files or dirs names to be archived
	file_argument fargument;
	for(int i = number_of_flags + 2; i<argc; i++){
		fargument.filename = argv[i];
		filelist->Enqueue(fargument);
	}

	return 0;
}