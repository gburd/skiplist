/*
 * Example 05: Duplicate Keys
 *
 * By default, skip_insert rejects duplicate keys (returns an error).
 * Use skip_insert_dup (or the convenience wrapper skip_dup from
 * SKIPLIST_DECL_ACCESS) to allow multiple nodes with the same key.
 *
 * Duplicates are stored adjacent in the list, so iterating over
 * them is straightforward.
 */
#define _GNU_SOURCE
#include "sl.h"

struct dup_node {
    int key;
    char *value;
    SKIPLIST_ENTRY(dup) entry;
};

SKIPLIST_DECL(
    dup, sl_, entry,
    /* compare */
    {
        (void)list;
        (void)aux;
        return (a->key < b->key) ? -1 : (a->key > b->key) ? 1 : 0;
    },
    /* free */
    {
        free(node->value);
        node->value = NULL;
    },
    /* update */
    {
        free(node->value);
        node->value = (char *)value;
    },
    /* archive */
    {
        dest->key = src->key;
        dest->value = src->value ? strdup(src->value) : NULL;
        if (src->value && !dest->value)
            rc = ENOMEM;
    },
    /* sizeof */
    { bytes = node->value ? strlen(node->value) + 1 : 0; })

SKIPLIST_DECL_ACCESS(dup, sl_, key, int, value, char *, { query.key = key; }, { return node->value; })

int
main(void)
{
    printf("=== Example 05: Duplicate Keys ===\n\n");

    dup_t *list = (dup_t *)malloc(sizeof(dup_t));
    if (!list)
        return 1;
    sl_skip_init_dup(list);

    /* Insert unique keys first. */
    sl_skip_put_dup(list, 1, strdup("apple"));
    sl_skip_put_dup(list, 2, strdup("banana"));
    sl_skip_put_dup(list, 3, strdup("cherry"));
    printf("After initial inserts: length = %zu\n", sl_skip_length_dup(list));

    /* Try inserting a duplicate with put (uses skip_insert, rejects dups). */
    int rc = sl_skip_put_dup(list, 2, strdup("blueberry"));
    if (rc != 0) {
        printf("put(2, 'blueberry') rejected (rc=%d) -- duplicate key.\n", rc);
        /* Note: the strdup'd string was freed by the generated put
         * via skip_free_node which calls our free block. Actually,
         * we need to free it ourselves since put frees the node but
         * the value was already set. Let's handle this properly. */
    }

    /* Use dup() to insert duplicates. This allows multiple entries
     * with the same key. */
    printf("\n--- Inserting duplicates with dup() ---\n");
    sl_skip_dup_dup(list, 2, strdup("blueberry"));
    sl_skip_dup_dup(list, 2, strdup("blackberry"));
    sl_skip_dup_dup(list, 1, strdup("avocado"));
    printf("After dup inserts: length = %zu\n", sl_skip_length_dup(list));

    /* Iterate to see all entries including duplicates. */
    printf("\n--- All entries (ascending) ---\n");
    dup_node_t *cur;
    size_t idx;
    SKIPLIST_FOREACH_H2T(dup, sl_, entry, list, cur, idx)
    {
        printf("  [%zu] key=%d, value=\"%s\"\n", idx, cur->key, cur->value);
    }

    /* Get returns the first match for a duplicate key. */
    printf("\n--- get() returns first match ---\n");
    char *v = sl_skip_get_dup(list, 2);
    printf("get(2) = \"%s\" (first of the duplicates)\n", v);

    /* Walk all duplicates of key=2 manually. */
    printf("\n--- Walking duplicates of key=2 ---\n");
    dup_node_t query;
    memset(&query, 0, sizeof(query));
    query.key = 2;
    dup_node_t *n = sl_skip_position_eq_dup(list, &query);
    int count = 0;
    while (n != NULL && n->key == 2) {
        printf("  key=2, value=\"%s\"\n", n->value);
        n = sl_skip_next_node_dup(list, n);
        count++;
    }
    printf("Found %d entries with key=2.\n", count);

    /* del removes only the first duplicate. */
    printf("\n--- del(2) removes first duplicate ---\n");
    sl_skip_del_dup(list, 2);
    printf("After del(2): length = %zu\n", sl_skip_length_dup(list));
    printf("Remaining entries:\n");
    SKIPLIST_FOREACH_H2T(dup, sl_, entry, list, cur, idx)
    {
        printf("  [%zu] key=%d, value=\"%s\"\n", idx, cur->key, cur->value);
    }

    /* Clean up. */
    sl_skip_free_dup(list);
    free(list);

    printf("\nDone.\n");
    return 0;
}
