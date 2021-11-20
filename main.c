#include "prefix_tree.h"

#include <stdio.h>
#include <stdlib.h>

static void match_cb(char const * const word, void * const cb_ctx)
{
    (void)cb_ctx;

    printf("%s\n", word);
}

static void
print_words_with_prefix(struct prefix_tree * const tree, char const * const prefix)
{
    prefix_tree_lookup(tree, prefix, match_cb, NULL);
}

static inline void
prefix_insert_word_or_die(struct prefix_tree * const tree, const char * const word)
{
    if (!prefix_tree_insert_word(tree, word))
    {
        fprintf(stderr, "Failed to insert word '%s'\n", word);
        exit(EXIT_FAILURE);
    }
}

int main(int const argc, char * * argv)
{
    (void)argc;
    (void)argv;

    if (argc < 2)
    {
        printf("\r\nmust supply prefix to search for\n");
        return EXIT_FAILURE;
    }

    struct prefix_tree * const tree = prefix_tree_create();

    prefix_insert_word_or_die(tree, "apple");
    prefix_insert_word_or_die(tree, "ap");
    prefix_insert_word_or_die(tree, "avocado");
    prefix_insert_word_or_die(tree, "banana");

    print_words_with_prefix(tree, argv[1]);

    prefix_tree_destroy(tree);

    return EXIT_SUCCESS;
}

