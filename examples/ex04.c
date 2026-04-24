/*
 * Example 04: Iteration and Positioning
 *
 * Demonstrates the full range of iteration and cursor positioning:
 *   - SKIPLIST_FOREACH_H2T: iterate head-to-tail (ascending)
 *   - SKIPLIST_FOREACH_T2H: iterate tail-to-head (descending)
 *   - skip_head / skip_tail: first/last elements
 *   - skip_next_node / skip_prev_node: step forward/backward
 *   - skip_pos with SKIP_GTE, SKIP_GT, SKIP_LTE, SKIP_LT: cursor
 */
#define _GNU_SOURCE
#include "sl.h"

struct iter_node {
    int key;
    char *value;
    SKIPLIST_ENTRY(iter) entry;
};

SKIPLIST_DECL(
    iter, sl_, entry,
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

SKIPLIST_DECL_ACCESS(iter, sl_, key, int, value, char *, { query.key = key; }, { return node->value; })

int
main(void)
{
    printf("=== Example 04: Iteration & Positioning ===\n\n");

    iter_t *list = (iter_t *)malloc(sizeof(iter_t));
    if (!list)
        return 1;
    sl_skip_init_iter(list);

    /* Insert even numbers 2..20. */
    for (int i = 2; i <= 20; i += 2) {
        char buf[32];
        snprintf(buf, sizeof(buf), "val_%d", i);
        sl_skip_put_iter(list, i, strdup(buf));
    }
    printf("Inserted %zu elements (even numbers 2..20).\n\n", sl_skip_length_iter(list));

    /* 1. FOREACH head-to-tail (ascending order). */
    printf("--- FOREACH_H2T (ascending) ---\n");
    iter_node_t *elm;
    size_t idx;
    SKIPLIST_FOREACH_H2T(iter, sl_, entry, list, elm, idx)
    {
        printf("  [%zu] key=%d, value=\"%s\"\n", idx, elm->key, elm->value);
    }

    /* 2. FOREACH tail-to-head (descending order). */
    printf("\n--- FOREACH_T2H (descending) ---\n");
    SKIPLIST_FOREACH_T2H(iter, sl_, entry, list, elm, idx)
    {
        printf("  [%zu] key=%d, value=\"%s\"\n", idx, elm->key, elm->value);
    }

    /* 3. Manual iteration with head/next. */
    printf("\n--- Manual: head + next_node (first 3) ---\n");
    iter_node_t *cur = sl_skip_head_iter(list);
    for (int i = 0; i < 3 && cur; i++) {
        printf("  key=%d, value=\"%s\"\n", cur->key, cur->value);
        cur = sl_skip_next_node_iter(list, cur);
    }

    /* 4. Manual iteration with tail/prev. */
    printf("\n--- Manual: tail + prev_node (last 3) ---\n");
    cur = sl_skip_tail_iter(list);
    for (int i = 0; i < 3 && cur; i++) {
        printf("  key=%d, value=\"%s\"\n", cur->key, cur->value);
        cur = sl_skip_prev_node_iter(list, cur);
    }

    /* 5. Cursor positioning with pos (GTE, GT, LTE, LT).
     * Our list has even keys only, so searching for 7 tests gaps. */
    printf("\n--- Positioning around key=7 (not in list) ---\n");
    iter_node_t *n;
    n = sl_skip_pos_iter(list, SKIP_GTE, 7);
    printf("  pos(GTE, 7) -> key=%d\n", n ? n->key : -1);

    n = sl_skip_pos_iter(list, SKIP_GT, 7);
    printf("  pos(GT,  7) -> key=%d\n", n ? n->key : -1);

    n = sl_skip_pos_iter(list, SKIP_LTE, 7);
    printf("  pos(LTE, 7) -> key=%d\n", n ? n->key : -1);

    n = sl_skip_pos_iter(list, SKIP_LT, 7);
    printf("  pos(LT,  7) -> key=%d\n", n ? n->key : -1);

    /* Positioning at the edges. */
    printf("\n--- Edge positioning ---\n");
    n = sl_skip_pos_iter(list, SKIP_LT, 2);
    printf("  pos(LT,  2)  -> %s\n", n ? "found" : "NULL (no element < 2)");

    n = sl_skip_pos_iter(list, SKIP_GT, 20);
    printf("  pos(GT, 20) -> %s\n", n ? "found" : "NULL (no element > 20)");

    n = sl_skip_pos_iter(list, SKIP_GTE, 10);
    printf("  pos(GTE, 10) -> key=%d (exact match)\n", n ? n->key : -1);

    /* Clean up. */
    sl_skip_free_iter(list);
    free(list);

    printf("\nDone.\n");
    return 0;
}
