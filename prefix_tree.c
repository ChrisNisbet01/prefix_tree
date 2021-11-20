#include "prefix_tree.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>

typedef struct prefix_node
{
    SIMPLEQ_ENTRY(prefix_node) entry;
    SIMPLEQ_HEAD(, prefix_node) children;

    char id;
    bool is_leaf;
} prefix_node;

typedef struct word_buffer_st
{
    char * buffer;
    size_t len;
} word_buffer_st;

struct prefix_tree
{
    /*
     * The number of descendants. Indicates the maximum length of any word in
     * the tree.
     */
    size_t depth;
    prefix_node * root;
};

static void
iterate_leaves(
    prefix_tree * tree,
    word_buffer_st * word,
    prefix_node * node,
    void (*cb)(char const * word, void * cb_ctx),
    void * cb_ctx);

static void
node_free(prefix_node * node);

static void
append_char_to_word_buffer(word_buffer_st * const word, char const ch)
{
    word->buffer[word->len] = ch;
    word->len++;
    word->buffer[word->len] = '\0';
}

static void
remove_char_from_word_buffer(word_buffer_st * const word)
{
    word->len--;
    word->buffer[word->len] = '\0';
}

static prefix_node *
node_alloc(char const id)
{
    prefix_node * const node = calloc(1, sizeof *node);

    if (node == NULL)
    {
        goto done;
    }

    node->id = id;
    SIMPLEQ_INIT(&node->children);

done:
    return node;
}

static void
node_free_children(prefix_node * const node)
{
    for (prefix_node * child = SIMPLEQ_FIRST(&node->children);
         child != NULL;
         child = SIMPLEQ_FIRST(&node->children)
         )
    {
        SIMPLEQ_REMOVE(&node->children, child, prefix_node, entry);
        node_free(child);
    }
}

static void
node_free(prefix_node * const node)
{
    if (node != NULL)
    {
        /*
         * Free the node's children first, then the node itself to avoid
         * referencing memory made invalid once a node is freed.
         */
        node_free_children(node);
        free(node);
    }
}

static void
add_child_to_node(prefix_node * const parent, prefix_node * const node)
{
    SIMPLEQ_INSERT_HEAD(&parent->children, node, entry);
}

static prefix_node *
child_lookup_by_id(prefix_node * const tree, char const id)
{
    prefix_node * node;

    SIMPLEQ_FOREACH(node, &tree->children, entry)
    {
        if (node->id == id)
        {
            goto done;
        }
    }

    node = NULL;

done:
    return node;
}

static bool
insert_prefix(prefix_node * const tree, char const * const prefix)
{
    bool inserted;

    if (*prefix == '\0')
    {
        tree->is_leaf = true;
        inserted = true;
        goto done;
    }

    prefix_node * node = child_lookup_by_id(tree, *prefix);

    if (node == NULL)
    {
        node = node_alloc(*prefix);
        if (node == NULL)
        {
            inserted = false;
            goto done;
        }
        add_child_to_node(tree, node);
    }

    inserted = insert_prefix(node, prefix + 1);

done:
    return inserted;
}

static void iterate_children(
    prefix_tree * const tree,
    word_buffer_st * const word,
    prefix_node * const node,
    void (*cb)(char const * word, void * cb_ctx),
    void * const cb_ctx)
{
    prefix_node * child;

    SIMPLEQ_FOREACH(child, &node->children, entry)
    {
        append_char_to_word_buffer(word, child->id);
        iterate_leaves(tree, word, child, cb, cb_ctx);
        remove_char_from_word_buffer(word);
    }
}

/*
 * Given a node in the tree, call the callback for all leaf nodes at or below
 * this point in the prefix_tree_match.
 * The given node will be determined by searching a user-supplied prefix string
 * against the nodes in the tree.
 */
static void
iterate_leaves(
    prefix_tree * const tree,
    word_buffer_st * const word,
    prefix_node * const node,
    void (*cb)(char const * word, void * cb_ctx),
    void * const cb_ctx)
{
    if (node->is_leaf)
    {
        cb(word->buffer, cb_ctx);
    }

    iterate_children(tree, word, node, cb, cb_ctx);
}

/*
 * Move through the nodes in the tree until the prefix is empty, finding the
 * node matching the current location in the prefix.
 * All leaf nodes at or below this point are matches for the given prefix.
 */
static prefix_node *
find_node_matching_prefix(
    prefix_tree * const tree,
    struct word_buffer_st * const word,
    char const * const prefix_in)
{
    prefix_node * node = tree->root;

    for (char const * prefix = prefix_in; *prefix != '\0'; prefix++)
    {
        node = child_lookup_by_id(node, *prefix);
        if (node == NULL)
        {
            goto done;
        }

        append_char_to_word_buffer(word, node->id);
    }

done:
    return node;
}

void
prefix_tree_lookup(
    prefix_tree * const tree,
    char const * const prefix,
    void (*cb)(char const * word, void * cb_ctx),
    void * const cb_ctx)
{
    struct word_buffer_st word = { 0 };

    /* Ensure that the user has provided a tree and a callback to call. */
    if (tree == NULL || cb == NULL)
    {
        goto done;
    }

    word.buffer = malloc(tree->depth + 1);
    word.len = 0;
    if (word.buffer == NULL)
    {
        goto done;
    }

    prefix_node * const node = find_node_matching_prefix(tree, &word, prefix);

    if (node == NULL)
    {
        goto done;
    }

    iterate_leaves(tree, &word, node, cb, cb_ctx);

done:
    free(word.buffer);

    return;
}


bool
prefix_tree_insert_word(prefix_tree * const tree, char const * const word)
{
    bool inserted;

    if (tree == NULL)
    {
        inserted = false;
        goto done;
    }

    size_t const depth = strlen(word);

    if (depth > tree->depth)
    {
        tree->depth = depth;
    }

    inserted = insert_prefix(tree->root, word);

done:
    return inserted;
}

void
prefix_tree_destroy(prefix_tree * const tree)
{
    node_free(tree->root);
    free(tree);
}

prefix_tree *
prefix_tree_create(void)
{
    prefix_tree * tree = calloc(1, sizeof *tree);

    if (tree == NULL)
    {
        goto done;
    }

    tree->root = node_alloc('\0');
    if (tree->root == NULL)
    {
        prefix_tree_destroy(tree);
        tree = NULL;
    }

done:
    return tree;
}

