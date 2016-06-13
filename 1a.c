/*****************************************
*     Aleks Nazarenko      13422452      *
*****************************************/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

struct node {
    double key;
    double probability;
    
    struct node *parent;
    struct node *children[256];
};

//---- Function Prototypes ----
double parse_fraction(char*, char*);
double ceil(double);
double assign_keys(const void*, int);
int compare_struct_node(const void*, const void*);
unsigned concatenate_numbers(unsigned, unsigned);
unsigned length_of_number(unsigned);

int main(int argc, char** argv){
    double X[argc-2];   
    int arity = atof(argv[argc-1]);
    
    printf("\nArity :=\t%d\n\n", arity);
    
    //---- Read CLA PMF into *X ----
    for (int i = 1; i < argc-1; i++){
        char* x = strstr(argv[i], "/");
        if (x != NULL){
            X[i-1] = parse_fraction(argv[i], x+1);
        } else {
            X[i-1] = atof(argv[i]);
        }
    }
    
    //---- Initialize base nodes ----
    int base_nodes = arity + ceil(((sizeof(X)/sizeof(*X)) - (double) arity)/(arity - 1))*(arity - 1);
    
    struct node *nodes = calloc(2 * base_nodes, sizeof(struct node));
    
    if (!nodes) { printf("Failed to allocate memory\n"); return 1;}
    
    for (int i = 0; i < base_nodes; i++){
        if (i < (sizeof(X)/sizeof(*X))) {
            nodes[i].probability = X[i];
        } else {
            nodes[i].probability = 0;
        }
    }
    
    
    struct node *first_node = nodes;
    int initial_node_count = base_nodes;
    int k;
    
    //---- Create Node Tree ----
    for(k = 0; base_nodes != 1; k++) {
        qsort(nodes, base_nodes, sizeof(*nodes), compare_struct_node);
        
        nodes[base_nodes].probability = 0;
        
        for (int i = 0; i < arity; i++) {
            nodes[base_nodes].probability += nodes[i].probability;
            nodes[base_nodes].children[i] = &nodes[i];
            nodes[i].parent = &nodes[base_nodes];
        }
        
        for(int i = 0; i < arity; i++){
            nodes++;
        }
        
        base_nodes -= (arity - 1);
    }
    
    printf("\nExpected Average Length:\t%f\n", assign_keys(nodes, arity));
    
    return 0;
}

double assign_keys(const void *nodes, int arity) {
    struct node *root = ((struct node *) nodes);
    double expected_code_length = 0;
    
    for (int i = arity - 1; i >= 0; i--) {
        if (root->children[i] != 0) {
            root->children[i]->key = (double) concatenate_numbers((unsigned) root->key,(unsigned) i);
             expected_code_length += assign_keys(root->children[i], arity);
        } else if (root->probability != 0) { 
            printf("Item with probability %f has key %d\n", root->probability, (int) root->key);
            expected_code_length += (root->probability * (int) length_of_number((unsigned) root->key));
            return expected_code_length;
        }
    }
    
    return expected_code_length;
}

unsigned concatenate_numbers(unsigned a, unsigned b) {
    unsigned pow = 10;
    while (b >= pow) pow *= 10;
    
    return (a * pow) + b;
}

unsigned length_of_number(unsigned n) {
    if (n < 10) return 1;
    return (1 + length_of_number(n/10));
}


double ceil(double number) {
    if ((int) number == number) return number;
    return (double) (int) number + 1;
}

int compare_struct_node(const void *a,const void *b) {
    if (((struct node *) a)->probability < ((struct node *) b)->probability) return -1;
    else if (((struct node *) a)->probability > ((struct node *) b)->probability) return 1; 
    return 0;
}

double parse_fraction(char* frac, char* x){
    return (double)(atof(frac))/(atof(x));
}