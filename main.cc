#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "parse.h"


#define ARGNUM 4

int main(int argc, char** argv){

	//Defining functionality of the programm according command line
	options choices;
	char* archive;
	Queue<file_argument> filelist(QUEUE_SIZE);
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


	return 0;
}