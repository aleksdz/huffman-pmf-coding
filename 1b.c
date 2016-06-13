/*****************************************
*     Aleks Nazarenko      13422452      *
* Encodes and decodes a number of items  *
* according to a static PMF		 *
*                                        *
*****************************************/
#include <math.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

struct node {
    double key;
    double probability;
    char value;
    
    struct node *parent;
    struct node *children[256];
};

//---- Function Prototypes ----
double encode(const void *, char *, unsigned, double *);
void decode(const void *, double *, unsigned, char *);
double parse_fraction(char*, char*);
double ceil(double);
double assign_keys(const void *, int);
double get_key(const void *, char);
char get_value(const void *, double);
int compare_struct_node(const void*, const void*);
unsigned concatenate_numbers(unsigned, unsigned);
unsigned length_of_number(unsigned);
char static_gen_pmf(void);

int main(int argc, char** argv) {
	srand(time(NULL));
    double distribution_of_characters[6] = {0};   
    double theoretical_distribution[] = {1.0/21, 2.0/21, 3.0/21, 4.0/21, 5.0/21, 6.0/21};
    char *generated_characters = malloc(atoi(argv[argc-1])*sizeof(char));
    
    
    for (int i = 2; i < 5; i++) {
        int arity = i;
        printf("\nArity :=\t%d\n\n", arity);
        
        for (int i = 0; i < atoi(argv[argc-1]); i++) {
            generated_characters[i] = static_gen_pmf();
            distribution_of_characters[(int)(generated_characters[i] - 'a')]++; //static_gen_pmf must return values >= 'a'
        }
        
        for (int i = 0; i < sizeof(distribution_of_characters)/sizeof(*distribution_of_characters); i++) {
            distribution_of_characters[i] /= (double)(atof(argv[argc-1]));
        }
        
        
        
        //---- Initialize base nodes ----
        int base_nodes = arity + ceil(((sizeof(distribution_of_characters)/sizeof(*distribution_of_characters)) - (double) arity)/(arity - 1))*(arity - 1);
        
        struct node *nodes = calloc(2 * base_nodes, sizeof(struct node));
        
        if (!nodes) { printf("Failed to allocate memory\n"); return 1;}
        
        for (int i = 0; i < base_nodes; i++){
            if (i < (sizeof(distribution_of_characters)/sizeof(*distribution_of_characters))) {
                nodes[i].probability = distribution_of_characters[i];
                nodes[i].value = (i + 'a');
            } else {
                nodes[i].probability = 0;
            }
        }
        
        
        //---- Create Node Tree ----
        while (base_nodes != 1) {
            qsort(nodes, base_nodes, sizeof(*nodes), compare_struct_node);
            
            nodes[base_nodes].probability = 0;
            
            for (int i = 0; i < arity; i++) {
                nodes[base_nodes].probability += nodes[i].probability;
                nodes[base_nodes].children[i] = &nodes[i];
                nodes[base_nodes].value = (char) 0;
                nodes[i].parent = &nodes[base_nodes];
            }
            
            for(int i = 0; i < arity; i++){
                nodes++;
            }
            
            base_nodes -= (arity - 1);
        }
        
        //---- Calculate Entropy of PMF ----
        double entropy = 0;       
        for (int i = 0; i < sizeof(theoretical_distribution)/sizeof(*theoretical_distribution); i++) {
            entropy -= (theoretical_distribution[i] * (log10(theoretical_distribution[i])/log10(arity))); 
        }
        printf("Entropy of Distribution:\t%f\n", entropy);
        

        printf("Expected Average Length:\t%f\n", assign_keys(nodes, arity));
        
        //---- Encode and decode string ----
        double encoding[atoi(argv[argc-1])];
        char decoded_string[atoi(argv[argc-1])];
        double average_size = encode(nodes, generated_characters, sizeof(generated_characters)/sizeof(*generated_characters), encoding);  
        decode(nodes, encoding, sizeof(encoding)/sizeof(*encoding), decoded_string);
        
        printf("Actual Average Length:\t\t%f\n", average_size);
        
        for (int i = 0; i < atoi(argv[argc-1]); i++) {
            if (decoded_string[i] != generated_characters[i]) break;
            if (i == atoi(argv[argc-1]) - 1) printf("Successfully decoded encoded string\n");
        }
        
    }
    return 0;
}

//---- Encode a given string ----
double encode(const void *nodes, char *string, unsigned size,  double *encoding) {
    struct node *root = ((struct node *) nodes);
    double total_size = 0;
    
    for (int i = 0; i < (int) size; i++) {
        encoding[i] = get_key(root, string[i]);
        total_size += length_of_number(encoding[i]);
    }
    
    return total_size/size;
}

//---- Decode a given array of encoding ----
void decode(const void *nodes, double *encoding, unsigned size, char *string) {
    struct node *root = ((struct node *) nodes);
    
    for (int i = 0; i < (int) size; i++) {
        string[i] = get_value(root, encoding[i]);
    }
}

//---- Get key from Node Tree given value ----
double get_key(const void *nodes, char c) {
    struct node *root = ((struct node *) nodes);
    double key = 0;
    int i = 0;
    if (root->children[0] != 0) {
        while (root->children[i] != 0) {
            key += get_key(root->children[i], c);
            i++;
        }
    } else if (root->value == c) return root->key;
    return key;
}

//---- Get value from Node Tree given key ----
char get_value(const void *nodes, double key) {
    struct node *root = ((struct node *) nodes);
    char value = (char) 0;
    int i = 0;    
    
    if (root->children[0] != 0) {
        while (root->children[i] != 0) {
            value += get_value(root->children[i], key);
            i++;
        }
    } else if (root->key == key) return root->value;
    return value;
}

//---- Assign Huffman Keys to Nodes in Tree ----
double assign_keys(const void *nodes, int arity) {
    struct node *root = ((struct node *) nodes);
    double expected_code_length = 0;
    
    for (int i = arity - 1; i >= 0; i--) {
        if (root->children[i] != 0) {
            root->children[i]->key = (double) concatenate_numbers((unsigned) root->key,(unsigned) i);
             expected_code_length += assign_keys(root->children[i], arity);
        } else if (root->probability != 0) { 
            //printf("Item with probability %f has key %d\n", root->probability, (int) root->key);
            expected_code_length += (root->probability * (int) length_of_number((unsigned) root->key));
            return expected_code_length;
        }
    }
    
    return expected_code_length;
}

//---- Concatenate Two Numbers ----
unsigned concatenate_numbers(unsigned a, unsigned b) {
    unsigned pow = 10;
    while (b >= pow) pow *= 10;
    
    return (a * pow) + b;
}

//---- Get number of digits in a number ----
unsigned length_of_number(unsigned n) {
    if (n < 10) return 1;
    return (1 + length_of_number(n/10));
}

//---- Get ceiling of a number ----
double ceil(double number) {
    if ((int) number == number) return number;
    return (double) (int) number + 1;
}

//---- Compare two nodes for sorting ----
int compare_struct_node(const void *a,const void *b) {
    if (((struct node *) a)->probability < ((struct node *) b)->probability) return -1;
    else if (((struct node *) a)->probability > ((struct node *) b)->probability) return 1; 
    return 0;
}

//---- Parse text fraction ----
double parse_fraction(char* frac, char* x){
    return (double)(atof(frac))/(atof(x));
}

//---- Generate Character with Chance according to this Static PMF ----
char static_gen_pmf(void) {
	int r = rand() % 21;
    
    if (r > 5) {
        if (r > 10) {
            if (r > 14) {
                if (r > 17) {
                    if (r > 19) {
                        return 'f';
                    } else return 'e';
                }
                return 'd';
            }
            return 'c';
        }
        return 'b';
    }
    return 'a';
}
