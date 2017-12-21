/**
 * Binary tree done with the help of
 * "Introduction of Algorithms, Third Edition,
 * Thomas H. Cormen,
 * Charles E. Leiserson,
 * Ronald L. Rivest,
 * Clifford Stein,
 * 2009" [1]
 * pages 286-298
 */

#ifndef SHELL_BINARY_TREE_H
#define SHELL_BINARY_TREE_H

typedef struct environnement_var_st {
    char *var;
    char *content;
} environment_var;

typedef struct node_st { /// Binary tree
    environment_var *data;
    struct node_st *left;
    struct node_st *right;
} node_t;

/**
 * Creates a new node
 * @param data The data to keep in the node
 * @return the node
 */
node_t *create_node(environment_var *data);

/**
 * Inserts a node in a Binary tree
 * @param root A pointer to the root of the binary tree
 * @param data The data to keep in the tree
 * @return a pointer to the filled tree
 */
node_t **insert_node(node_t **root, environment_var *data);

/**
 * Deletes the binary tree from the node where the var equals to the given data
 * @param root The root to the binary tree
 * @param data The data to find
 * @return A pointer to the emptied tree
 */
node_t *delete_node(node_t *root, environment_var *data);

/**
 * Searches the node in the binary tree where lies the same data
 * @param root The root of the binary tree to search
 * @param data The data to find in the binary tree
 * @return A pointer to the node in the tree if found; NULL otherwise
 */
node_t *search(node_t *root, const char *data);

/**
 * Recursively removes all nodes of the tree
 * @param root The binary tree to be erased
 */
void dispose(node_t *root);

/**
 * Prints the content of the node
 * @param nd The node to be printed
 */
void display(node_t *nd);

/**
 * Displays the binary tree first depth from left to right
 * @param nd The root of the tree to print
 */
void display_tree(node_t *nd);

/**
 * Frees the environment variable structure
 * @param toFree A pointer to the structure to be freed
 */
void free_env_var(environment_var *toFree);

#endif //SHELL_BINARY_TREE_H
