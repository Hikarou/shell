#include "binary_tree.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

node *create_node(environnement_var *data) {
    node *new_node = (node *) malloc(sizeof(node));
    if (new_node == NULL) {
        fprintf(stderr, "Out of memory!!! (create_node)\n");
        exit(1);
    }
    memcpy(new_node->data.var, data->var, strlen(data->var));
    memcpy(new_node->data.content, data->content, strlen(data->content));
    new_node->left = NULL;
    new_node->right = NULL;
    return new_node;
}

node *insert_node(node *root, environnement_var *data) {

    if (root == NULL) {
        root = create_node(data);
    } else {
        int is_left = 0;
        int r = 0;
        node *cursor = root;
        node *prev = NULL;

        while (cursor != NULL) {
            r = strcmp(data->var, cursor->data.var);
            prev = cursor;
            if (r < 0) {
                is_left = 1;
                cursor = cursor->left;
            } else if (r > 0) {
                is_left = 0;
                cursor = cursor->right;
            }

        }
        if (is_left) {
            prev->left = create_node(data);
        } else {
            prev->right = create_node(data);
        }

    }

    return root;
}

node *delete_node(node *root, environnement_var *data) {
    if (root == NULL)
        return NULL;

    node *cursor;
    int r = strcmp(data->var, root->data.var);
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
            node *parent = NULL;

            while (cursor->left != NULL) {
                parent = cursor;
                cursor = cursor->left;
            }
            root->data = cursor->data;
            if (parent != NULL) {
                parent->left = delete_node(parent->left, &parent->left->data);
            } else {
                root->right = delete_node(root->right, &root->right->data);
            }
        }
    }
    return root;
}

node *search(node *root, const environnement_var *data) {
    if (root == NULL)
        return NULL;

    int r;
    node *cursor = root;
    while (cursor != NULL) {
        r = strcmp(data->var, cursor->data.var);
        if (r < 0)
            cursor = cursor->left;
        else if (r > 0)
            cursor = cursor->right;
        else
            return cursor;
    }
    return cursor;

}

// in order
void traverse(node *root, callback cb) {
    node *cursor, *pre;

    if (root == NULL)
        return;

    cursor = root;

    while (cursor != NULL) {
        if (cursor->left != NULL) {
            cb(cursor);
            cursor = cursor->right;
        } else {
            pre = cursor->left;

            while (pre->right != NULL && pre->right != cursor)
                pre = pre->right;

            if (pre->right != NULL) {
                pre->right = cursor;
                cursor = cursor->left;
            } else {
                pre->right = NULL;
                cb(cursor);
                cursor = cursor->right;
            }
        }
    }
}

/*
    recursively remove all nodes of the tree
*/
void dispose(node *root) {
    if (root != NULL) {
        dispose(root->left);
        dispose(root->right);
        free(root);
    }
}