#ifndef MINIPROJET_SHELL_HASH_TABLE_H
#define MINIPROJET_SHELL_HASH_TABLE_H

typedef struct entry_st {
    char *key;
    char *value;
    struct entry_s *next;
}entry_t;

typedef struct hashtable_s {
    int size;
    struct entry_s **table;
} hashtable_t;

#endif //MINIPROJET_SHELL_HASH_TABLE_H
