#ifndef SHELL_BINARY_TREE_H
#define SHELL_BINARY_TREE_H

typedef struct environnement_var_st {
    char *var;
    char *content;
} environment_var;

typedef struct node {
    environment_var *data;
    struct node *left;
    struct node *right;
} node;

node *create_node(environment_var *data);

node *insert_node(node *root, environment_var *data);

node *delete_node(node *root, environment_var *data);

node *search(node *root, const char *data);

void dispose(node *root);

void display(node *nd);

void display_tree(node *nd);

#endif //SHELL_BINARY_TREE_H
