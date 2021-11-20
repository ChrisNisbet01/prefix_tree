#ifndef PREFIX_TREE_H__
#define PREFIX_TREE_H__

#include <stdbool.h>

typedef struct prefix_tree prefix_tree;

void
prefix_tree_lookup(
    prefix_tree * tree,
    char const * prefix,
    void (*cb)(char const * word, void * cb_ctx),
    void * cb_ctx);

bool
prefix_tree_insert_word(prefix_tree * tree, char const * word);

void
prefix_tree_destroy(prefix_tree * tree);

prefix_tree *
prefix_tree_create(void);

#endif /* PREFIX_TREE_H__ */

