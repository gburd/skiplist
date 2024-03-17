#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "../include/skiplist.h"

// Define a node that contains key and value pair.
struct my_node {
    // Metadata for skiplist node.
    sl_node snode;
    // My data here: {int, int} pair.
    int key;
    int value;
};

// Define a comparison function for `my_node`.
static int
my_cmp(sl_node *a, sl_node *b, void *aux)
{
    // Get `my_node` from skiplist node `a` and `b`.
    struct my_node *aa, *bb;
    aa = sl_get_entry(a, struct my_node, snode);
    bb = sl_get_entry(b, struct my_node, snode);

    // aa  < bb: return neg
    // aa == bb: return 0
    // aa  > bb: return pos
    if (aa->key < bb->key)
        return -1;
    if (aa->key > bb->key)
        return 1;
    return 0;
}

#define NUM_NODES 10000

int
main()
{
    // seed the PRNG
    srandom((unsigned)(time(NULL) | getpid()));

    sl_raw slist;

    // Initialize skiplist.
    sl_init(&slist, my_cmp);

    //   << Insertion >>
    // Allocate & insert NUM_NODES KV pairs: {0, 0}, {1, 10}, {2, 20}.
    struct my_node *nodes[NUM_NODES];
    for (int i = 0; i < NUM_NODES; ++i) {
        // Allocate memory.
        nodes[i] = (struct my_node *)malloc(sizeof(struct my_node));
        // Initialize node.
        sl_init_node(&nodes[i]->snode);
        // Assign key and value.
        nodes[i]->key = i;
        nodes[i]->value = i * 10;
        // Insert into skiplist.
        sl_insert(&slist, &nodes[i]->snode);
    }

    //   << Point lookup >>
    for (int i = 0; i < NUM_NODES; ++i) {
        // Define a query.
        struct my_node query;
        int min = 1, max = NUM_NODES - 1;
        int k = min + (int)random() / (RAND_MAX / (max - min + 1) + 1);
        query.key = k;
        // Find a skiplist node `cursor`.
        sl_node *cursor = sl_find(&slist, &query.snode);
        // If `cursor` is NULL, key doesn't exist.
        if (!cursor)
            continue;
        // Get `my_node` from `cursor`.
        // Note: found->snode == *cursor
        struct my_node *found = sl_get_entry(cursor, struct my_node, snode);
        printf("[point lookup] key: %d, value: %d\n", found->key, found->value);
        if (found->key != found->value / 10) {
            printf("FAILURE: key: %d * 10 !=  value: %d\n", found->key,
                found->value);
            exit(-1);
        }
        // Release `cursor` (== &found->snode).
        // Other thread cannot free `cursor` until `cursor` is released.
        sl_release_node(cursor);
    }

    //   << Erase >>
    // Erase the KV pair for key 1: {1, 10}.
    {
        // Define a query.
        struct my_node query;
        query.key = 1;
        // Find a skiplist node `cursor`.
        sl_node *cursor = sl_find(&slist, &query.snode);
        // Get `my_node` from `cursor`.
        // Note: found->snode == *cursor
        struct my_node *found = sl_get_entry(cursor, struct my_node, snode);
        printf("[erase] key: %d, value: %d\n", found->key, found->value);

        // Detach `found` from skiplist.
        sl_erase_node(&slist, &found->snode);
        // Release `found`, to free its memory.
        sl_release_node(&found->snode);
        // Free `found` after it becomes safe.
        sl_wait_for_free(&found->snode);
        sl_free_node(&found->snode);
        free(found);
    }

    //   << Iteration >>
    {
        // Get the first cursor.
        sl_node *cursor = sl_begin(&slist);
        while (cursor) {
            // Get `entry` from `cursor`.
            // Note: entry->snode == *cursor
            struct my_node *entry = sl_get_entry(cursor, struct my_node, snode);
            printf("[iteration] key: %d, value: %d\n", entry->key,
                entry->value);
            // Get next `cursor`.
            cursor = sl_next(&slist, cursor);
            // Release `entry`.
            sl_release_node(&entry->snode);
        }
    }

    //   << Destroy >>
    {
        // Iterate and free all nodes.
        sl_node *cursor = sl_begin(&slist);
        while (cursor) {
            struct my_node *entry = sl_get_entry(cursor, struct my_node, snode);
            printf("[destroy] key: %d, value: %d\n", entry->key, entry->value);
            // Get next `cursor`.
            cursor = sl_next(&slist, cursor);

            // Detach `entry` from skiplist.
            sl_erase_node(&slist, &entry->snode);
            // Release `entry`, to free its memory.
            sl_release_node(&entry->snode);
            // Free `entry` after it becomes safe.
            sl_wait_for_free(&entry->snode);
            sl_free_node(&entry->snode);
            free(entry);
        }
    }

    // Free skiplist.
    sl_free(&slist);

    return 0;
}
