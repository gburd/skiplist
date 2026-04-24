/*
 * Example 01: Minimal Skiplist
 *
 * This is the simplest possible skiplist example. It demonstrates:
 *   - Defining a node struct with SKIPLIST_ENTRY
 *   - Generating skiplist functions with SKIPLIST_DECL
 *   - Initializing, inserting, searching, and freeing a skiplist
 *
 * The skiplist maps integer keys to string values.
 */
#include "sl.h"

/* Step 1: Define your node structure.
 *
 * Every node must embed a SKIPLIST_ENTRY(decl) field. The "decl" name
 * ties together the node struct, the list struct, and all generated
 * functions. You can put any additional fields you like. */
struct ex01_node {
    int key;
    char *value;
    SKIPLIST_ENTRY(ex01) entry;
};

/* Step 2: Generate all skiplist functions with SKIPLIST_DECL.
 *
 * Arguments:
 *   decl   = "ex01"   -- names the types: ex01_t, ex01_node_t
 *   prefix = "sl_"    -- names the functions: sl_skip_init_ex01(), etc.
 *   field  = "entry"  -- which struct member holds the SKIPLIST_ENTRY
 *
 * Then five code blocks:
 *   compare -- how to order two nodes (must return -1, 0, or +1)
 *   free    -- release any user-owned resources in a node
 *   update  -- replace a node's value in-place
 *   archive -- deep-copy a node (used by snapshots)
 *   sizeof  -- report the byte size of user data in a node */
SKIPLIST_DECL(
    ex01, sl_, entry,
    /* compare: variables in scope are list, a, b, aux */
    {
        (void)list;
        (void)aux;
        if (a->key < b->key)
            return -1;
        if (a->key > b->key)
            return 1;
        return 0;
    },
    /* free: variable in scope is node */
    {
        free(node->value);
        node->value = NULL;
    },
    /* update: variables in scope are rc, node, value */
    {
        (void)rc;
        free(node->value);
        node->value = (char *)value;
    },
    /* archive: variables in scope are rc, src, dest */
    {
        dest->key = src->key;
        dest->value = strdup(src->value);
        if (dest->value == NULL)
            rc = ENOMEM;
    },
    /* sizeof: variables in scope are bytes, node */
    {
        bytes = sizeof(*node);
        if (node->value)
            bytes += strlen(node->value) + 1;
    })

int
main(void)
{
    printf("=== Example 01: Minimal Skiplist ===\n\n");

    /* Step 3: Allocate and initialize the skiplist. */
    ex01_t *list = (ex01_t *)malloc(sizeof(ex01_t));
    if (list == NULL)
        return 1;

    int rc = sl_skip_init_ex01(list);
    if (rc) {
        free(list);
        return rc;
    }

    printf("Empty list: length = %zu, is_empty = %s\n", sl_skip_length_ex01(list), sl_skip_is_empty_ex01(list) ? "true" : "false");

    /* Step 4: Allocate nodes and insert them. */
    const char *names[] = { "alice", "bob", "charlie", "diana", "eve" };
    int keys[] = { 30, 10, 50, 20, 40 };

    for (int i = 0; i < 5; i++) {
        ex01_node_t *node;
        rc = sl_skip_alloc_node_ex01(&node);
        if (rc)
            break;
        node->key = keys[i];
        node->value = strdup(names[i]);
        rc = sl_skip_insert_ex01(list, node);
        if (rc) {
            free(node->value);
            sl_skip_free_node_ex01(list, node);
            break;
        }
        printf("Inserted: key=%d, value=\"%s\"\n", keys[i], names[i]);
    }

    printf("\nAfter inserts: length = %zu\n", sl_skip_length_ex01(list));

    /* Step 5: Search for nodes using position_eq. */
    printf("\n--- Searching ---\n");
    int search_keys[] = { 10, 30, 42 };
    for (int i = 0; i < 3; i++) {
        ex01_node_t query;
        memset(&query, 0, sizeof(query));
        query.key = search_keys[i];
        ex01_node_t *found = sl_skip_position_eq_ex01(list, &query);
        if (found)
            printf("Found key=%d -> \"%s\"\n", search_keys[i], found->value);
        else
            printf("Key=%d not found\n", search_keys[i]);
    }

    /* Step 6: Walk the list head-to-tail to verify sorted order. */
    printf("\n--- Sorted order (head to tail) ---\n");
    ex01_node_t *cur = sl_skip_head_ex01(list);
    while (cur != NULL) {
        printf("  key=%d, value=\"%s\"\n", cur->key, cur->value);
        cur = sl_skip_next_node_ex01(list, cur);
    }

    /* Step 7: Remove a node. */
    printf("\n--- Removing key=30 ---\n");
    ex01_node_t remove_query;
    memset(&remove_query, 0, sizeof(remove_query));
    remove_query.key = 30;
    rc = sl_skip_remove_node_ex01(list, &remove_query);
    printf("Remove returned: %d, length = %zu\n", rc, sl_skip_length_ex01(list));

    /* Step 8: Free the entire skiplist (frees all nodes too). */
    sl_skip_free_ex01(list);
    free(list);

    printf("\nDone.\n");
    return 0;
}
