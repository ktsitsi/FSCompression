#include "blocks.h"
#include "metadata_disk.h"
#include "list.h"

typedef struct stack_node
{
    entry* _entry;
    unsigned int depth;
} stack_node;

void tree_print(char *metadata) 
{
    list_t *stack;
    list_create(&stack,sizeof(stack_node),free);

    dinode* di_n;
    dentry* d_en;

    d_en = (dentry*)(metadata + sizeof(dinode));

    stack_node *sn = malloc(sizeof(stack_node));
    sn->_entry = &(d_en->tuple_entry[0]);
    sn->depth = 0;
    list_push(stack,sn);

    while(list_get_len(stack)) {
        stack_node cur_node;
        list_pop(stack,&cur_node); 

        entry *en = cur_node._entry;

        unsigned int i;
        for (i=0;i<cur_node.depth;++i) printf("\t");
        printf("%s\n",en->filename);

        di_n = (dinode*)(metadata + en->dinode_off);
        size_t n_dentries = di_n->n_dentries;
        d_en = (dentry*)(di_n + sizeof(dinode));

        for (i=0;i<n_dentries;++i){
            unsigned int j;
            for (j=0;j<d_en->length;++j) {
                stack_node *node = malloc(sizeof(stack_node));
                node->_entry = &(d_en->tuple_entry[j]);
                node->depth = cur_node.depth + 1; 
                list_push(stack,node);
            }
            d_en += (sizeof(dentry));
        }

    }

    list_destroy(&stack);
}
