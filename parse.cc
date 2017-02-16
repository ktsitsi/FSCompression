#include "parse.h"
#include "management.h"

int parse_options(char** argv, options* mt){
	
	Queue<inline_argument> arglist(QUEUE_SIZE);
	inline_argument argument, temp_argument;

	for(int i=0; i<NUM_OF_FLAGS; i++){
		if(strcmp(argv[i],"-c") && strcmp(argv[i],"-a") && strcmp(argv[i],"-x") && strcmp(argv[i],"-m") && strcmp(argv[i],"-d") && strcmp(argv[i],"-p") && strcmp(argv[i],"-j")){
			perror("Wrong argument flag\n");
			return -1;
		}
		argument.type = argv[i];
		arglist.Enqueue(argument);
	}

	while(!arglist.is_empty){
		temp_argument = arglist.get_front();
		if(!strcmp(temp_argument.type,"-c")){
			mt->c = 1;
		}
		else if(!strcmp(temp_argument.type,"-a")){
			mt->a = 1;
		}
		else if(!strcmp(temp_argument.type,"-x")){
			mt->x = 1;
		}
		else if(!strcmp(temp_argument.type,"-m")){
			mt->m = 1;
		}
		else if(!strcmp(temp_argument.type,"-d")){
			mt->d = 1;
		}
		else if(!strcmp(temp_argument.type,"-p")){
			mt->p = 1;
		}
		else if(!strcmp(temp_argument.type,"-j")){
			mt->j = 1;
		}
		arglist.Pop();
	}
	return 0;
}

