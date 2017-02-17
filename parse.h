#include "list.h"
#include "management.h"
#include "parse_data.h"

#define QUEUE_SIZE 8
#define NUM_OF_FLAGS 8

int parse(int argc, char** argv, options* choices, char** targetfile,Queue<file_argument>* filelist);