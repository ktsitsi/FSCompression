#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "metadata.h"
#include "parse_data.h"
#include "meta.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <inttypes.h>

int get_current_dir(char** cwd,char** current_dir){
	if(getcwd(*cwd,PATH_MAX) == NULL){
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

char* normalize_path(char* resolved_path, char* cwd){
	char *rtoken;
	char *cwdtoken;
	char *store_resolved = (char*)calloc(PATH_MAX,sizeof(char));
	rtoken = strtok(resolved_path,"/\n");
	cwdtoken = strtok(cwd,"/\n");
	int counter = 0;
	while(!strcmp(rtoken, cwdtoken)){
		/* Update strtok pointers */
		rtoken += strlen(rtoken) + 1;
		sprintf(store_resolved,"%s",rtoken);
		cwdtoken += strlen(cwdtoken) + 1;
		/* Get next token */
		rtoken = strtok(rtoken, "/\n");
		cwdtoken = strtok(cwdtoken, "/\n");
		counter++;
		if(cwdtoken == NULL && rtoken != NULL){
			//resolved path is PREDECESSOR of cwd
			break;
		}
		if(rtoken == NULL && cwdtoken == NULL){
			//resoved is the same path with cwd
			break;
		}
		if(rtoken == NULL && cwdtoken != NULL){
			//resolved path is ancestor of the cwd FAILURE
			return NULL;
		}
	}

	if(rtoken != NULL && cwdtoken != NULL){
		//different branch of two paths
		return NULL;
	}
	//printf("Similar to depth %d\n",counter);
	return store_resolved;
}

void dinodes_list_init(list_t *hierarhy_list,char* current_dir){

	
	//Create-Init the first pseudoroot dinode
	dinode* pseudoroot = (dinode*)malloc(sizeof(dinode));
	pseudoroot->dinode_number = 0;

	//Initialize the current directory dinode
	dinode* cwd_din = (dinode*)malloc(sizeof(dinode));
	cwd_din->dinode_number = 1;
	//Take all the necessary info from lstat

	dentry *rootindex = (dentry*)malloc(sizeof(dentry));

	strcpy((rootindex->tuple_entry[0]).filename,current_dir);
	(rootindex->tuple_entry[0]).dinode_num = 1;
	(rootindex->tuple_entry[0]).dinode_idx = cwd_din;
	rootindex->length = 1;

	//Enqueue the dentry of pseudo root in the dinode of pseudoroot
	list_create(&(pseudoroot->dentry_list),sizeof(dentry),free);
	list_enqueue(pseudoroot->dentry_list,rootindex);
	
	//Init the cyclic reference of the cwd with 2 entries in the dentry
	dentry *cwdidx = (dentry*)malloc(sizeof(dentry));
	strcpy((cwdidx->tuple_entry[0]).filename,".");
	(cwdidx->tuple_entry[0]).dinode_num = 1;
	(cwdidx->tuple_entry[0]).dinode_idx = cwd_din;
	strcpy((cwdidx->tuple_entry[1]).filename,"..");
	(cwdidx->tuple_entry[1]).dinode_num = 1;
	(cwdidx->tuple_entry[1]).dinode_idx = cwd_din;
	cwdidx->length = 2;

	//Enqueue the dentry in the list of index of the cwd
	list_create(&(cwd_din->dentry_list),sizeof(dentry),free);
	list_enqueue(cwd_din->dentry_list,cwdidx);

	//Create the first 2 nodes of the hierarchy list of dinodes
	list_enqueue(hierarhy_list,pseudoroot);
	list_enqueue(hierarhy_list,cwd_din);

}
void insert_hierarchical_rec(list_t * hierarhy_list,dinode* leafdinode,char* filepath,arc_header* hdr,int archive_fd){
	struct stat buffer;
	lstat(filepath,&buffer);
	if(S_ISREG(buffer.st_mode)){
		printf("We reached file %s\n",filepath);
		int fd = open(filepath,O_CREAT|O_RDWR,0644);
		if(fd<0) printf("Couldn't open the file\n");
		leafdinode->file_off = file_archive(hdr,fd,archive_fd);
		close(fd);
		//regular file create the dinode and assign data offset
		return;
	}
	else if(S_ISDIR(buffer.st_mode)){
		DIR *dirp;
		struct dirent *dp;
		dirp = opendir(filepath);
		do{
			if((dp = readdir(dirp)) != NULL){
				if(!strcmp(dp->d_name,".") || !strcmp(dp->d_name,".."))
					continue;
				else{
					list_iter_t* dentries_iter;
					list_iter_create(&dentries_iter);
					list_iter_init(dentries_iter,leafdinode->dentry_list,BACKWARD);
					dentry* last_dentry = list_iter_next(dentries_iter);

					dinode* newdinode = (dinode*)malloc(sizeof(dinode));
					//INIT THE STATS OF THE FILE
					dinode_stat_init(newdinode,buffer);
					list_create(&(newdinode->dentry_list),sizeof(dentry),free);

					dentry *init_entry = (dentry*)malloc(sizeof(dentry));
					init_entry->length = 2;
					strcpy((init_entry->tuple_entry[0]).filename,".");
					(init_entry->tuple_entry[0]).dinode_num = 3068;
					(init_entry->tuple_entry[0]).dinode_idx = newdinode;
					strcpy((init_entry->tuple_entry[1]).filename,"..");
					(init_entry->tuple_entry[1]).dinode_num = 3068;
					(init_entry->tuple_entry[1]).dinode_idx = NULL;
					list_enqueue(newdinode->dentry_list,init_entry);
					
					if(last_dentry->length < DENTRIES_NUM){
						strcpy((last_dentry->tuple_entry[last_dentry->length]).filename,dp->d_name);
						(last_dentry->tuple_entry[last_dentry->length]).dinode_num= 1024;
						(last_dentry->tuple_entry[last_dentry->length]).dinode_idx = newdinode;
						last_dentry->length++; 
					}
					else{
						dentry* new_dentry = (dentry*)malloc(sizeof(dentry));
						new_dentry->length = 1;
						strcpy((new_dentry->tuple_entry[0]).filename,dp->d_name);
						(new_dentry->tuple_entry[0]).dinode_num = 1024;
						(new_dentry->tuple_entry[0]).dinode_idx = newdinode;
						list_enqueue(leafdinode->dentry_list,new_dentry);
					}
					list_enqueue(hierarhy_list,newdinode);
					char rec_path[PATH_MAX];
					sprintf(rec_path,"%s/%s",filepath,dp->d_name);
					insert_hierarchical_rec(hierarhy_list,newdinode,rec_path,hdr,archive_fd);
				}
			}
		}while(dp != NULL);
	}
	return;
}

void insert_hierarchical(list_t *hierarhy_list,char* insert_file_path,arc_header* hdr,int archive_fd){
	char initial_path[PATH_MAX];
	strcpy(initial_path,insert_file_path);
	if(!strcmp(insert_file_path,"")){
		printf("insert hierarchical: The whole cwd was given\n");
		list_iter_t *dinode_iter;
		list_iter_t *dentries_iter;
		list_iter_create(&dinode_iter);
		list_iter_init(dinode_iter,hierarhy_list,FORWARD);
		dinode *current_din = (dinode*)list_iter_next(dinode_iter);
		list_iter_create(&dentries_iter);
		list_iter_init(dentries_iter,current_din->dentry_list,FORWARD);
		dentry *current_den = (dentry*)list_iter_next(dentries_iter);

		//Pointer to cwd of the list e.g ad
		current_din = (current_den->tuple_entry[0]).dinode_idx;
		insert_hierarchical_rec(hierarhy_list,current_din,".",hdr,archive_fd);
		list_iter_destroy (&dinode_iter);
		list_iter_destroy (&dentries_iter);
	}
	else{
		list_iter_t *dinode_iter;
		list_iter_t *dentries_iter;
		
		list_iter_create(&dinode_iter);
		list_iter_init(dinode_iter,hierarhy_list,FORWARD);
		dinode *current_din = (dinode*)list_iter_next(dinode_iter);

		list_iter_create(&dentries_iter);
		list_iter_init(dentries_iter,current_din->dentry_list,FORWARD);
		dentry *current_den = (dentry*)list_iter_next(dentries_iter);

		//Pointer to cwd of the list e.g ad
		current_din = (current_den->tuple_entry[0]).dinode_idx;

		char *path_token = strtok(insert_file_path,"/\n");
		int i;
		dinode* lastdinode;
		struct stat buffer;
		//Dentry list of cwd for first path token
		char current_path[PATH_MAX];
		sprintf(current_path,"./%s",path_token);
		while(path_token != NULL){
			int found = 0;
			printf("Current path is %s\n",current_path);
			list_iter_init(dentries_iter,current_din->dentry_list,FORWARD);
			while((current_den = list_iter_next(dentries_iter)) != NULL){
				for(i=0;i<current_den->length;i++){
					if(!strcmp((current_den->tuple_entry[i]).filename,path_token)){
						//Found in the entry table
						found = 1;
						lastdinode = (current_den->tuple_entry[i]).dinode_idx;
						current_din = (current_den->tuple_entry[i]).dinode_idx;
						//lastdinode = current_din;
						break;
					}
				}
				if(found) break;
			}
			if(found == 0){
				//The path token is not in the hierarhy
				printf("The token %s is not in the hierarhy\n",path_token);
				list_iter_init(dentries_iter,current_din->dentry_list,BACKWARD);
				current_den = (dentry*)list_iter_next(dentries_iter);

				dinode *newdinode = (dinode*)malloc(sizeof(dinode));
				//TODO: INIT THE STATS OF THE FILE
				lstat(current_path,&buffer);
				dinode_stat_init(newdinode,buffer);				

				list_create(&(newdinode->dentry_list),sizeof(dentry),free);
				dentry *init_entry = (dentry*)malloc(sizeof(dentry));
				init_entry->length = 2;
				strcpy((init_entry->tuple_entry[0]).filename,".");
				(init_entry->tuple_entry[0]).dinode_num = 2056;
				(init_entry->tuple_entry[0]).dinode_idx = newdinode;
				strcpy((init_entry->tuple_entry[1]).filename,"..");
				(init_entry->tuple_entry[1]).dinode_num = 2056;
				(init_entry->tuple_entry[1]).dinode_idx = NULL;
				
				list_enqueue(newdinode->dentry_list,init_entry);

				if(current_den->length < DENTRIES_NUM){
					//Put it in this dentry element
					strcpy((current_den->tuple_entry[current_den->length]).filename,path_token);
					(current_den->tuple_entry[current_den->length]).dinode_num = 1024;
					(current_den->tuple_entry[current_den->length]).dinode_idx = newdinode;
					current_den->length++; 
				}
				else{
					dentry* new_dentry = (dentry*)malloc(sizeof(dentry));
					new_dentry->length = 1;
					strcpy((new_dentry->tuple_entry[0]).filename,path_token);
					(new_dentry->tuple_entry[0]).dinode_num = 1024;
					(new_dentry->tuple_entry[0]).dinode_idx = newdinode;
					list_enqueue(current_din->dentry_list,new_dentry);
				}
				lastdinode = newdinode;
				list_enqueue(hierarhy_list,newdinode);
				current_din = newdinode;
			}
			path_token = strtok(NULL, "/\n");
			sprintf(current_path,"%s/%s",current_path,path_token);
		}

		//The last dinode has the pointer to the dinode of the leaf of the path
		//insert file path has the whole file
		char rec_filepath_arg[PATH_MAX];
		sprintf(rec_filepath_arg,"./%s",initial_path);
		printf("The recfilepath given is %s\n",rec_filepath_arg);
		insert_hierarchical_rec(hierarhy_list,lastdinode,rec_filepath_arg,hdr,archive_fd);
		//Decide if the leaf node is file or dir
		list_iter_destroy (&dinode_iter);
		list_iter_destroy (&dentries_iter);
	}
}

int create_hierarchical(list_t *filelist, list_t* hierarhy_list,arc_header* hdr,int archive_fd){
	char *cwd = (char*)malloc(PATH_MAX*sizeof(char));
	char *current_dir;
	if(get_current_dir(&cwd,&current_dir) < 0){
		return -1;
	}
	//Initialize dinode list with 2 first naive nodes of pseudo and cwd
	dinodes_list_init(hierarhy_list,current_dir);
	
	list_iter_t *file_iterator;
	list_iter_create(&file_iterator);
	list_iter_init(file_iterator,filelist,FORWARD);

	//For al files in the command line
	file_argument* cur_file;
	printf("Length list:%d\n",list_get_len(filelist));
	struct stat buffer;
	while((cur_file = (file_argument*)list_iter_next(file_iterator)) != NULL){
		//find reals path of this argument
		char *resolved_path = (char*)calloc(PATH_MAX,sizeof(char));
		printf("create hierarchical: Resolved:%s\n",resolved_path);
		realpath(cur_file->filename, resolved_path);
		printf("create hierarchical: RealPath:%s\n",resolved_path);
		if(lstat(resolved_path,&buffer) == 0){
			//File exists in the filesystem
			char* normalized_filepath;
			normalized_filepath = normalize_path(resolved_path,cwd);
			printf("create hierarchical: RealPath:%s\n",normalized_filepath);
			if(normalized_filepath != NULL){
				printf("create hierarchical: Before insert hierarchical\n");
				insert_hierarchical(hierarhy_list,normalized_filepath,hdr,archive_fd);
				free(normalized_filepath);
			}
			else{
				printf("Error in normalization of the path\n");
				continue;
			}
		}
		else{
			//no such file in the FS
			continue;
		}
		free(resolved_path);
	}

	list_iter_destroy(&file_iterator);
	free(cwd);
	return 0;
}
