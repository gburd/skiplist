/*
 * Example 02: Key/Value API
 *
 * SKIPLIST_DECL_ACCESS generates convenient key/value functions:
 *   put     -- insert a key/value pair
 *   get     -- look up a value by key
 *   set     -- update an existing key's value in-place
 *   del     -- remove a key
 *   contains -- check if a key exists
 *   dup     -- insert allowing duplicate keys
 *   pos     -- position to GTE/GT/LTE/LT/EQ of a key
 *
 * This example uses int keys and heap-allocated string values.
 */
#include "sl.h"

struct kv_node {
    int key;
    char *value;
    SKIPLIST_ENTRY(kv) entry;
};

SKIPLIST_DECL(
    kv, api_, entry,
    /* compare */
    {
        (void)list;
        (void)aux;
        if (a->key < b->key)
            return -1;
        if (a->key > b->key)
            return 1;
        return 0;
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
        if (src->value && dest->value == NULL)
            rc = ENOMEM;
    },
    /* sizeof */
    { bytes = node->value ? strlen(node->value) + 1 : 0; })

/* SKIPLIST_DECL_ACCESS generates: api_skip_get_kv, api_skip_put_kv, etc.
 *
 * Arguments:
 *   decl   = "kv"
 *   prefix = "api_"
 *   key    = field name for the key in the node struct
 *   ktype  = key type (int)
 *   value  = field name for the value in the node struct
 *   vtype  = value type (char *)
 *   qblk   = block to populate a query node from a key
 *   rblk   = block to return the value from a found node */
SKIPLIST_DECL_ACCESS(
    kv, api_, key, int, value, char *,
    /* query block: variable 'query' (a kv_node_t) and 'key' in scope */
    { query.key = key; },
    /* return block: variable 'node' (kv_node_t *) in scope */
    { return node->value; })

int
main(void)
{
    printf("=== Example 02: Key/Value API ===\n\n");

    kv_t *list = (kv_t *)malloc(sizeof(kv_t));
    if (!list)
        return 1;
    api_skip_init_kv(list);

    /* put: insert key/value pairs. Values are heap-allocated. */
    printf("--- Inserting with put ---\n");
    api_skip_put_kv(list, 1, strdup("one"));
    api_skip_put_kv(list, 2, strdup("two"));
    api_skip_put_kv(list, 3, strdup("three"));
    api_skip_put_kv(list, 4, strdup("four"));
    api_skip_put_kv(list, 5, strdup("five"));
    printf("Inserted 5 entries, length = %zu\n", api_skip_length_kv(list));

    /* get: retrieve values by key. */
    printf("\n--- Looking up with get ---\n");
    for (int k = 1; k <= 6; k++) {
        char *v = api_skip_get_kv(list, k);
        if (v)
            printf("get(%d) = \"%s\"\n", k, v);
        else
            printf("get(%d) = NULL (not found)\n", k);
    }

    /* contains: check existence without retrieving the value. */
    printf("\n--- Checking with contains ---\n");
    printf("contains(3) = %s\n", api_skip_contains_kv(list, 3) ? "true" : "false");
    printf("contains(9) = %s\n", api_skip_contains_kv(list, 9) ? "true" : "false");

    /* set: update an existing key's value in-place. */
    printf("\n--- Updating with set ---\n");
    printf("Before: get(2) = \"%s\"\n", api_skip_get_kv(list, 2));
    api_skip_set_kv(list, 2, strdup("TWO (updated)"));
    printf("After:  get(2) = \"%s\"\n", api_skip_get_kv(list, 2));

    /* del: remove a key. */
    printf("\n--- Deleting with del ---\n");
    printf("Before del(4): length = %zu\n", api_skip_length_kv(list));
    api_skip_del_kv(list, 4);
    printf("After  del(4): length = %zu\n", api_skip_length_kv(list));
    printf("get(4) = %s\n", api_skip_get_kv(list, 4) ? "found" : "NULL (deleted)");

    /* pos: position cursor relative to a key. */
    printf("\n--- Positioning with pos ---\n");
    kv_node_t *n;
    n = api_skip_pos_kv(list, SKIP_GTE, 2);
    printf("pos(GTE, 2) -> key=%d, value=\"%s\"\n", n->key, n->value);

    n = api_skip_pos_kv(list, SKIP_GT, 2);
    printf("pos(GT,  2) -> key=%d, value=\"%s\"\n", n->key, n->value);

    n = api_skip_pos_kv(list, SKIP_LTE, 4);
    printf("pos(LTE, 4) -> key=%d, value=\"%s\"\n", n->key, n->value);

    n = api_skip_pos_kv(list, SKIP_LT, 3);
    printf("pos(LT,  3) -> key=%d, value=\"%s\"\n", n->key, n->value);

    /* Clean up. */
    api_skip_free_kv(list);
    free(list);

    printf("\nDone.\n");
    return 0;
}
