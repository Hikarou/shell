#ifndef SHELL_BINARY_TREE_H
#define SHELL_BINARY_TREE_H

typedef struct environnement_var_st {
    char * var;
    char * content;
} environnement_var;

typedef struct node {
    environnement_var data;
    struct node *left;
    struct node *right;
} node;

typedef void (*callback)(node*);

node *create_node(environnement_var *data);

node *insert_node(node *root, environnement_var *data);

node *delete_node(node *root, environnement_var *data);

node *search(node *root, const environnement_var *data);

void traverse(node *root,callback cb);

void dispose(node* root);

void display(node* nd);

void display_tree(node* nd);

#endif //SHELL_BINARY_TREE_H
