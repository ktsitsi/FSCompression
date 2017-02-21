#include "blocks.h"
#include "metadata_disk.h"
#include "meta.h"
#include "list.h"
#include <linux/limits.h>
#include <inttypes.h>

#define CWD_LENGTH 128

char* normalize_path(char* resolved_path, char* cwd);

typedef struct stack_node
{
    entry_disk* _entry;
    unsigned int depth;
} stack_node;

void tree_print(char *metadata) 
{
    list_t *stack;
    list_create(&stack,sizeof(stack_node),free);

    dinode_disk* di_n;
    dentry_disk* d_en;

    d_en = (dentry_disk*)(metadata + sizeof(dinode_disk));

    stack_node *sn = malloc(sizeof(stack_node));
    sn->_entry = &(d_en->tuple_entry[0]);
    sn->depth = 0;
    list_push(stack,sn);

    while(list_get_len(stack)) {
        stack_node cur_node;
        list_pop(stack,&cur_node); 

        entry_disk *en = cur_node._entry;
        unsigned int i;
        for (i=0;i<cur_node.depth;++i) printf("\t");
        printf("%s\n",en->filename);

        di_n = (dinode_disk*)(metadata + en->dinode_off);
        size_t n_dentries = di_n->n_dentries;
        d_en = (dentry_disk*)(di_n + sizeof(dinode_disk));

        for (i=0;i<n_dentries;++i){
            unsigned int j;
            for (j=0;j<d_en->length;++j) {
                en = &(d_en->tuple_entry[j]);
                if (!strcpy(en->filename,".") || !strcpy(en->filename,"..")) continue;
                stack_node *node = malloc(sizeof(stack_node));
                node->_entry = en; 
                node->depth = cur_node.depth + 1; 
                list_push(stack,node);
            }
            d_en += (sizeof(dentry_disk));
        }

    }

    list_destroy(&stack);
}

int query(char *metadata,char *path) 
{
    char *cwd = (char*)malloc(CWD_LENGTH*sizeof(char));
    char *current_dir;
    if(get_current_dir(&cwd,&current_dir) < 0){
        free(cwd);
        return -1;
    }

    char *resolved_path = (char*)calloc(PATH_MAX,sizeof(char));
    realpath(path, resolved_path); 
    char* normalized_filepath;          //!!!!!!!!!carefull to free normalized path!!!!!!!!!
    normalized_filepath = normalize_path(resolved_path,cwd);

    free(resolved_path);
    free(cwd);

    if(normalized_filepath != NULL){
        if(!strcmp(normalized_filepath,"")){
            printf("Same directory\n");
        }
        else{
            printf("The path in the hierarchy is: %s\n",normalized_filepath);
        }
    }
    else{
        printf("Error in normalization of the path\n");
        free(normalized_filepath);
        return 0;
    }

    int result = 1;
    char *token;
    token = strtok(normalized_filepath,"/");

    dinode_disk* di_n;
    dentry_disk* d_en = (dentry_disk*)(metadata + sizeof(dinode_disk));
    entry_disk *cur_entry = &(d_en->tuple_entry[0]);

    if (strcmp(token,cur_entry->filename)) {
        result = 0;
    } else {
        while(token!=NULL) {
            token = strtok(NULL,"/");
            di_n = (dinode_disk*)(metadata + cur_entry->dinode_off);
            size_t n_dentries = di_n->n_dentries;
            d_en = (dentry_disk*)(di_n + sizeof(dinode_disk));

            char matched = 0;
            unsigned int i;
            for (i=0;i<n_dentries && !matched;++i){
                unsigned int j;
                for (j=0;j<d_en->length && !matched;++j) {
                    cur_entry = &(d_en->tuple_entry[j]);
                    if (!strcmp(token,cur_entry->filename)) matched = 1; 
                }
                d_en += (sizeof(dentry_disk));
            }
            if (!matched){
                result = 0;
                break;
            }
        }
    }

    free(normalized_filepath);
    return result;

}
