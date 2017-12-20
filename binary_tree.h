#ifndef SHELL_BINARY_TREE_H
#define SHELL_BINARY_TREE_H

typedef struct environnement_var_st {
    char *var;
    char *content;
} environment_var;

typedef struct node_st {
    environment_var *data;
    struct node_st *left;
    struct node_st *right;
} node_t;

node_t *create_node(environment_var *data);

node_t **insert_node(node_t **root, environment_var *data);

node_t *delete_node(node_t *root, environment_var *data);

node_t *search(node_t *root, const char *data);

void dispose(node_t *root);

void display(node_t *nd);

void display_tree(node_t *nd);

void free_env_var(environment_var *toFree);

#endif //SHELL_BINARY_TREE_H
