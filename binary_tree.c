#include "binary_tree.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

node_t *create_node(environment_var *data) {
    printf("create_node init\n");
    fflush(stdout);
    node_t *new_node = (node_t *) malloc(sizeof(node_t));

    if (new_node == NULL) {
        fprintf(stderr, "Out of memory!!! (create_node)\n");
        exit(1);
    }

    new_node->data = malloc(sizeof(environment_var));
    if (new_node->data == NULL) {
        fprintf(stderr, "Out of memory!!! (create_node)\n");
        exit(1);
    }

    printf("create_node after malloc\n");
    printf("create_node before calloc of (%zu, %zu)\n", strlen(data->var), sizeof(char));
    fflush(stdout);
    new_node->data->var = calloc(strlen(data->var), sizeof(char));
    printf("create_node after first calloc\n");
    fflush(stdout);
    if (new_node->data->var == NULL) {
        fprintf(stderr, "create_node malloc failed\n");
        exit(EXIT_FAILURE);
    }
    strncpy(new_node->data->var, data->var, strlen(data->var));
    printf("create_node after first strncpy\n");
    fflush(stdout);
    new_node->data->content = calloc(strlen(data->content), sizeof(char));
    printf("create_node after second calloc\n");
    fflush(stdout);
    if (new_node->data->content == NULL) {
        fprintf(stderr, "create_node malloc failed\n");
        exit(EXIT_FAILURE);
    }
    strncpy(new_node->data->content, data->content, strlen(data->content));
    printf("create_node after second strncpy\n");
    fflush(stdout);
    new_node->left = NULL;
    new_node->right = NULL;
    return new_node;
}

node_t **insert_node(node_t **root, environment_var *data) {

    printf("insert_node init\n");
    fflush(stdout);
    if (*root == NULL) {
        *root = create_node(data);
    } else {
        int is_left = 0;
        int r = 0;
        node_t *cursor = *root;
        node_t *prev = cursor;
        node_t *new_node = create_node(data);

        while (cursor != NULL) {
            r = strcmp(data->var, cursor->data->var);
            prev = cursor;
            if (r < 0) {
                is_left = 1;
                cursor = cursor->left;
            } else if (r > 0) {
                is_left = 0;
                cursor = cursor->right;
            } else { //if the node_t exists, then we erase the old value
                new_node->left = cursor->left;
                new_node->right = cursor->right;
                free(cursor);
                break;
            }
        }
        if (is_left) {
            prev->left = new_node;
        } else {
            prev->right = new_node;
        }

    }

    return root;
}

node_t *delete_node(node_t *root, environment_var *data) {
    if (root == NULL)
        return NULL;

    node_t *cursor;
    int r = strcmp(data->var, root->data->var);
    if (r < 0)
        root->left = delete_node(root->left, data);
    else if (r > 0)
        root->right = delete_node(root->right, data);
    else {
        if (root->left == NULL) {
            cursor = root->right;
            free(root);
            root = cursor;
        } else if (root->right == NULL) {
            cursor = root->left;
            free(root);
            root = cursor;
        } else {   //2 children
            cursor = root->right;
            node_t *parent = NULL;

            while (cursor->left != NULL) {
                parent = cursor;
                cursor = cursor->left;
            }
            root->data = cursor->data;
            if (parent != NULL) {
                parent->left = delete_node(parent->left, parent->left->data);
            } else {
                root->right = delete_node(root->right, root->right->data);
            }
        }
    }
    return root;
}

node_t *search(node_t *root, const char *var) {
    if (root == NULL)
        return NULL;

    int r;
    node_t *cursor = root;
    while (cursor != NULL) {
        r = strcmp(var, cursor->data->var);
        if (r < 0) {
            cursor = cursor->left;
        } else if (r > 0) {
            cursor = cursor->right;
        } else {
            break;
        }
    }
    return cursor;

}

/*
    recursively remove all nodes of the tree
*/
void dispose(node_t *root) {
    if (root != NULL) {
        dispose(root->left);
        dispose(root->right);
        free_env_var(root->data);
        free(root);
    }
}

void display(node_t* nd)
{
    if(nd != NULL)
        printf("%s=%s\n",nd->data->var, nd->data->content);
}

void display_tree(node_t* nd)
{
    if (nd == NULL)
        return;
    /* display node_t data */
    display(nd);
    /*
    if(nd->left != NULL)
        printf("(L:%s)",nd->left->data->var);
    if(nd->right != NULL)
        printf("(R:%s)",nd->right->data->var);
    printf("\n");
     */

    display_tree(nd->left);
    display_tree(nd->right);
}

void free_env_var(environment_var *toFree) {
    if (toFree == NULL) return;

    free(toFree->content);
    free(toFree->var);
    free(toFree);
}