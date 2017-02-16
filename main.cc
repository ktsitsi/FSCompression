#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "parse.h"


#define ARGNUM 4

void printhelp(){
	printf("Help: -- FSCompression -- \n");
}

int main(int argc, char** argv){

	if(argc != ARGNUM){
		perror("Wrong number of arguments\n");
		printhelp();
		return -1;
	}

	//Defining functionality of the programm according command line
	options choices;
	if(parse(argv,&choices) < 0){
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