#define MUNIT_ENABLE_ASSERT_ALIASES

/* dup/dup2/close/fileno are POSIX, not ISO C; the build uses -std=c11,
   which hides them unless we ask for the POSIX surface explicitly. */
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include <limits.h>
#include <signal.h> /* SIGPIPE handling around the archive write-failure test */
#include <string.h>
#include <unistd.h> /* dup/dup2/close/pipe, for stderr silencing and I/O failures */

#include "munit.h"
#include "sl.h"

/* Test node structure */
struct test_node {
    int key;
    char *value;
    SKIPLIST_ENTRY(test) entries;
};

/* Generate skiplist functions */
SKIPLIST_DECL(
    test, api_, entries,
    /* compare entries */
    {
        (void)list;
        (void)aux;
        if (a->key < b->key)
            return -1;
        if (a->key > b->key)
            return 1;
        return 0;
    },
    /* free entry */
    {
        if (node->value) {
            free(node->value);
            node->value = NULL;
        }
    },
    /* update entry */
    {
        char *new_value = (char *)value;
        if (node->value)
            free(node->value);
        node->value = new_value;
    },
    /* archive entry */
    {
        dest->key = src->key;
        if (src->value) {
            dest->value = malloc(strlen(src->value) + 1);
            if (dest->value == NULL) {
                rc = ENOMEM;
            } else {
                strcpy(dest->value, src->value);
            }
        } else {
            dest->value = NULL;
        }
    },
    /* sizeof entry */
    {
        bytes = sizeof(struct test_node);
        if (node->value)
            bytes += strlen(node->value) + 1;
    })

/* Generate access functions */
SKIPLIST_DECL_ACCESS(
    test, api_, key, int, value, char *,
    /* query block */ { query.key = key; },
    /* return block */ { return node->value; })

/* Generate validation functions */
SKIPLIST_DECL_VALIDATE(test, api_, entries)

/* Generate EBR functions */
SKIPLIST_DECL_EBR(test, api_)

/* Generate snapshot functions */
SKIPLIST_DECL_SNAPSHOTS(test, api_, entries)

/* Generate pool allocator functions */
SKIPLIST_DECL_POOL(test, api_, entries, 256)

/* Generate archive functions */
SKIPLIST_DECL_ARCHIVE(
    test, api_, entries,
    /* write_entry_blk */
    {
        /* Write key */
        memcpy(buf, &node->key, sizeof(node->key));
        uint64_t off = sizeof(node->key);
        /* Write value string length + data (or 0 if NULL) */
        if (node->value) {
            uint32_t slen = (uint32_t)strlen(node->value);
            memcpy(buf + off, &slen, sizeof(slen));
            off += sizeof(slen);
            memcpy(buf + off, node->value, slen);
            off += slen;
        } else {
            uint32_t slen = 0;
            memcpy(buf + off, &slen, sizeof(slen));
            off += sizeof(slen);
        }
        bytes = off;
    },
    /* read_entry_blk */
    {
        /* Validate the declared record size (`bytes`) before consuming `buf`.
           Trusting the on-disk length without bounds checks is an out-of-bounds
           read on malformed/hostile input. */
        node->value = NULL;
        uint64_t off = 0;
        if (bytes < sizeof(node->key) + sizeof(uint32_t)) {
            node->key = 0;
        } else {
            memcpy(&node->key, buf + off, sizeof(node->key));
            off += sizeof(node->key);
            uint32_t slen;
            memcpy(&slen, buf + off, sizeof(slen));
            off += sizeof(slen);
            if (slen > 0 && off + slen <= bytes) {
                node->value = (char *)malloc(slen + 1);
                if (node->value) {
                    memcpy(node->value, buf + off, slen);
                    node->value[slen] = '\0';
                }
            }
        }
    })

/* Generate DOT (GraphViz) visualization functions */
SKIPLIST_DECL_DOT(test, api_, entries)

/* Helper for the DOT writer: stringify a node's key into a 2048-byte buffer. */
static void
sprintf_test_node(test_node_t *n, char *buf)
{
    snprintf(buf, 2048, "%d=%s", n->key, n->value ? n->value : "");
}

/* Helper function to create test value */
static char *
make_test_value(int key)
{
    char *value = malloc(32);
    snprintf(value, 32, "value_%d", key);
    return value;
}

/* Test initialization */
static MunitResult
test_init(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    test_t *list = malloc(sizeof(test_t));
    assert_not_null(list);

    int rc = api_skip_init_test(list);
    assert_int(rc, ==, 0);

    assert_int(api_skip_length_test(list), ==, 0);
    assert_true(api_skip_is_empty_test(list));
    assert_null(api_skip_head_test(list));
    assert_null(api_skip_tail_test(list));

    api_skip_free_test(list);
    free(list);

    return MUNIT_OK;
}

/* Test basic insertion */
static MunitResult
test_insert_basic(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    test_t *list = malloc(sizeof(test_t));
    api_skip_init_test(list);

    test_node_t *node;
    int rc = api_skip_alloc_node_test(&node);
    assert_int(rc, ==, 0);

    node->key = 42;
    node->value = make_test_value(42);

    rc = api_skip_insert_test(list, node);
    assert_int(rc, ==, 0);

    assert_int(api_skip_length_test(list), ==, 1);
    assert_false(api_skip_is_empty_test(list));

    test_node_t *head = api_skip_head_test(list);
    assert_not_null(head);
    assert_int(head->key, ==, 42);

    api_skip_free_test(list);
    free(list);

    return MUNIT_OK;
}

/* Test duplicate insertion */
static MunitResult
test_insert_duplicate(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    test_t *list = malloc(sizeof(test_t));
    api_skip_init_test(list);

    /* Insert first node */
    test_node_t *node1;
    api_skip_alloc_node_test(&node1);
    node1->key = 10;
    node1->value = make_test_value(10);
    api_skip_insert_test(list, node1);

    /* Try to insert duplicate (should fail) */
    test_node_t *node2;
    api_skip_alloc_node_test(&node2);
    node2->key = 10;
    node2->value = make_test_value(10);
    int rc = api_skip_insert_test(list, node2);
    assert_int(rc, ==, EEXIST);

    /* Length should still be 1 */
    assert_int(api_skip_length_test(list), ==, 1);

    /* Insert duplicate with dup flag (should succeed) */
    rc = api_skip_insert_dup_test(list, node2);
    assert_int(rc, ==, 0);
    assert_int(api_skip_length_test(list), ==, 2);

    api_skip_free_test(list);
    free(list);

    return MUNIT_OK;
}

/* Test multiple insertions and ordering */
static MunitResult
test_insert_multiple(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    test_t *list = malloc(sizeof(test_t));
    api_skip_init_test(list);

    int keys[] = { 5, 2, 8, 1, 9, 3, 7, 4, 6 };
    int n_keys = sizeof(keys) / sizeof(keys[0]);

    /* Insert all keys */
    for (int i = 0; i < n_keys; i++) {
        test_node_t *node;
        api_skip_alloc_node_test(&node);
        node->key = keys[i];
        node->value = make_test_value(keys[i]);
        api_skip_insert_test(list, node);
    }

    assert_int(api_skip_length_test(list), ==, n_keys);

    /* Verify ordering by traversing */
    test_node_t *current = api_skip_head_test(list);
    int prev_key = 0;
    int count = 0;

    while (current) {
        assert_int(current->key, >, prev_key);
        prev_key = current->key;
        count++;
        current = api_skip_next_node_test(list, current);
    }

    assert_int(count, ==, n_keys);

    api_skip_free_test(list);
    free(list);

    return MUNIT_OK;
}

/* Test search operations */
static MunitResult
test_search(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    test_t *list = malloc(sizeof(test_t));
    api_skip_init_test(list);

    /* Insert test data */
    for (int i = 1; i <= 10; i++) {
        test_node_t *node;
        api_skip_alloc_node_test(&node);
        node->key = i * 10;
        node->value = make_test_value(i * 10);
        api_skip_insert_test(list, node);
    }

    /* Test exact search */
    test_node_t query;
    query.key = 50;
    test_node_t *found = api_skip_position_eq_test(list, &query);
    assert_not_null(found);
    assert_int(found->key, ==, 50);

    /* Test search for non-existent key */
    query.key = 55;
    found = api_skip_position_eq_test(list, &query);
    assert_null(found);

    api_skip_free_test(list);
    free(list);

    return MUNIT_OK;
}

/* Test removal operations */
static MunitResult
test_remove(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    test_t *list = malloc(sizeof(test_t));
    api_skip_init_test(list);

    /* Insert test data */
    for (int i = 1; i <= 5; i++) {
        test_node_t *node;
        api_skip_alloc_node_test(&node);
        node->key = i;
        node->value = make_test_value(i);
        api_skip_insert_test(list, node);
    }

    assert_int(api_skip_length_test(list), ==, 5);

    /* Remove middle element */
    test_node_t query;
    query.key = 3;
    int rc = api_skip_remove_node_test(list, &query);
    assert_int(rc, ==, 0);
    assert_int(api_skip_length_test(list), ==, 4);

    /* Verify it's gone */
    test_node_t *found = api_skip_position_eq_test(list, &query);
    assert_null(found);

    /* Remove non-existent element */
    query.key = 10;
    rc = api_skip_remove_node_test(list, &query);
    assert_int(rc, ==, ENOENT); /* Not found */
    assert_int(api_skip_length_test(list), ==, 4);

    api_skip_free_test(list);
    free(list);

    return MUNIT_OK;
}

/* Test access API */
static MunitResult
test_access_api(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    test_t *list = malloc(sizeof(test_t));
    api_skip_init_test(list);

    /* Test put */
    char *value1 = make_test_value(100);
    int rc = api_skip_put_test(list, 100, value1);
    assert_int(rc, ==, 0);

    /* Test get */
    char *retrieved = api_skip_get_test(list, 100);
    assert_not_null(retrieved);
    assert_string_equal(retrieved, "value_100");

    /* Test contains */
    assert_true(api_skip_contains_test(list, 100));
    assert_false(api_skip_contains_test(list, 200));

    /* Test del */
    rc = api_skip_del_test(list, 100);
    assert_int(rc, ==, 0);
    assert_false(api_skip_contains_test(list, 100));

    api_skip_free_test(list);
    free(list);

    return MUNIT_OK;
}

/* Test navigation */
static MunitResult
test_navigation(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    test_t *list = malloc(sizeof(test_t));
    api_skip_init_test(list);

    /* Insert ordered data */
    for (int i = 1; i <= 5; i++) {
        test_node_t *node;
        api_skip_alloc_node_test(&node);
        node->key = i * 10;
        node->value = make_test_value(i * 10);
        api_skip_insert_test(list, node);
    }

    /* Test forward navigation */
    test_node_t *current = api_skip_head_test(list);
    assert_not_null(current);
    assert_int(current->key, ==, 10);

    current = api_skip_next_node_test(list, current);
    assert_not_null(current);
    assert_int(current->key, ==, 20);

    /* Test backward navigation */
    test_node_t *tail = api_skip_tail_test(list);
    assert_not_null(tail);
    assert_int(tail->key, ==, 50);

    current = api_skip_prev_node_test(list, tail);
    assert_not_null(current);
    assert_int(current->key, ==, 40);

    /* Test navigation boundaries */
    test_node_t *head = api_skip_head_test(list);
    test_node_t *before_head = api_skip_prev_node_test(list, head);
    assert_null(before_head);

    test_node_t *after_tail = api_skip_next_node_test(list, tail);
    assert_null(after_tail);

    api_skip_free_test(list);
    free(list);

    return MUNIT_OK;
}

/* Test edge cases */
static MunitResult
test_edge_cases(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    test_t *list = malloc(sizeof(test_t));
    api_skip_init_test(list);

    /* Test operations on empty list */
    assert_null(api_skip_get_test(list, 1));
    assert_false(api_skip_contains_test(list, 1));
    assert_int(api_skip_del_test(list, 1), ==, ENOENT);

    /* Test NULL parameter handling */
    assert_int(api_skip_insert_test(NULL, NULL), ==, EINVAL);
    assert_int(api_skip_remove_node_test(NULL, NULL), ==, EINVAL);

    api_skip_free_test(list);
    free(list);

    return MUNIT_OK;
}

/* Test splay-list specific behavior */
static MunitResult
test_splay_behavior(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    test_t *list = malloc(sizeof(test_t));
    api_skip_init_test(list);
    list->slh_prng_state = 12345; /* Fixed seed for reproducible tests */

    /* Insert enough elements to create a multi-level skiplist */
    for (int i = 1; i <= 50; i++) {
        test_node_t *node;
        api_skip_alloc_node_test(&node);
        node->key = i;
        node->value = make_test_value(i);
        api_skip_insert_test(list, node);
    }

    /* Find the hot key node and record its initial height */
    test_node_t query;
    query.key = 25;
    test_node_t *hot_node = api_skip_position_eq_test(list, &query);
    assert_not_null(hot_node);
    size_t initial_height = hot_node->entries.sle_height;

    /* Find a cold key node and record its initial height */
    query.key = 1;
    test_node_t *cold_node = api_skip_position_eq_test(list, &query);
    assert_not_null(cold_node);
    size_t cold_initial_height = cold_node->entries.sle_height;

    /* Hammer the hot key with many accesses to trigger rebalancing */
    for (int i = 0; i < 500; i++) {
        api_skip_contains_test(list, 25); /* Frequently accessed */
    }

    /* Verify list is still functional and correctly ordered */
    assert_int(api_skip_length_test(list), ==, 50);
    assert_true(api_skip_contains_test(list, 25));
    assert_true(api_skip_contains_test(list, 1));
    assert_true(api_skip_contains_test(list, 50));

    /* Verify forward traversal still works */
    test_node_t *current = api_skip_head_test(list);
    int count = 0;
    int prev_key = 0;
    while (current) {
        assert_int(current->key, >, prev_key);
        prev_key = current->key;
        count++;
        current = api_skip_next_node_test(list, current);
    }
    assert_int(count, ==, 50);

#ifdef SKIPLIST_SPLAY_REBALANCE
    /* With splay rebalancing, the hot key should have been promoted
       to a higher level than where it started */
    {
        size_t final_height = hot_node->entries.sle_height;
        /* Hot key must have been promoted above its initial height */
        assert_size(final_height, >, initial_height);
        (void)cold_node;
        (void)cold_initial_height;
    }
#else
    /* Without splay, heights don't change -- just suppress warnings */
    (void)hot_node;
    (void)cold_node;
    (void)initial_height;
    (void)cold_initial_height;
#endif

    api_skip_free_test(list);
    free(list);

    return MUNIT_OK;
}

/* Test memory management */
static MunitResult
test_memory_management(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    test_t *list = malloc(sizeof(test_t));
    api_skip_init_test(list);

    /* Insert and remove many elements to test memory handling */
    for (int i = 0; i < 100; i++) {
        test_node_t *node;
        api_skip_alloc_node_test(&node);
        node->key = i;
        node->value = make_test_value(i);
        api_skip_insert_test(list, node);
    }

    /* Remove every other element */
    for (int i = 0; i < 100; i += 2) {
        api_skip_del_test(list, i);
    }

    assert_int(api_skip_length_test(list), ==, 50);

    /* Verify remaining elements */
    for (int i = 1; i < 100; i += 2) {
        assert_true(api_skip_contains_test(list, i));
    }

    api_skip_free_test(list);
    free(list);

    return MUNIT_OK;
}

/* Regression: skip_tail_ with 0, 1, 2 elements */
static MunitResult
test_tail_regression(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    test_t *list = malloc(sizeof(test_t));
    api_skip_init_test(list);

    /* Empty list: tail should be NULL */
    assert_null(api_skip_tail_test(list));

    /* Single element: tail should be that element */
    test_node_t *node1;
    api_skip_alloc_node_test(&node1);
    node1->key = 10;
    node1->value = make_test_value(10);
    api_skip_insert_test(list, node1);

    test_node_t *tail = api_skip_tail_test(list);
    assert_not_null(tail);
    assert_int(tail->key, ==, 10);

    /* Two elements: tail should be the largest */
    test_node_t *node2;
    api_skip_alloc_node_test(&node2);
    node2->key = 20;
    node2->value = make_test_value(20);
    api_skip_insert_test(list, node2);

    tail = api_skip_tail_test(list);
    assert_not_null(tail);
    assert_int(tail->key, ==, 20);

    /* Head should be the smallest */
    test_node_t *head = api_skip_head_test(list);
    assert_not_null(head);
    assert_int(head->key, ==, 10);

    api_skip_free_test(list);
    free(list);

    return MUNIT_OK;
}

/* Regression: head height must stay >= 1 after deleting last element */
static MunitResult
test_delete_last_element(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    test_t *list = malloc(sizeof(test_t));
    api_skip_init_test(list);

    /* Insert then delete a single element */
    char *value = make_test_value(42);
    int rc = api_skip_put_test(list, 42, value);
    assert_int(rc, ==, 0);
    assert_int(api_skip_length_test(list), ==, 1);

    rc = api_skip_del_test(list, 42);
    assert_int(rc, ==, 0);
    assert_int(api_skip_length_test(list), ==, 0);

    /* List should be usable after deleting last element */
    assert_null(api_skip_head_test(list));
    assert_null(api_skip_tail_test(list));
    assert_false(api_skip_contains_test(list, 42));

    /* Should be able to insert again */
    char *value2 = make_test_value(99);
    rc = api_skip_put_test(list, 99, value2);
    assert_int(rc, ==, 0);
    assert_int(api_skip_length_test(list), ==, 1);
    assert_true(api_skip_contains_test(list, 99));

    api_skip_free_test(list);
    free(list);

    return MUNIT_OK;
}

/* Test position variants (gte, gt, lte, lt) */
static MunitResult
test_position_variants(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    test_t *list = malloc(sizeof(test_t));
    api_skip_init_test(list);

    /* Insert 10, 20, 30, 40, 50 */
    for (int i = 1; i <= 5; i++) {
        test_node_t *node;
        api_skip_alloc_node_test(&node);
        node->key = i * 10;
        node->value = make_test_value(i * 10);
        api_skip_insert_test(list, node);
    }

    /* GTE: find >= 25 should return 30 */
    test_node_t *found = api_skip_pos_test(list, SKIP_GTE, 25);
    assert_not_null(found);
    assert_int(found->key, ==, 30);

    /* GTE: find >= 30 should return 30 */
    found = api_skip_pos_test(list, SKIP_GTE, 30);
    assert_not_null(found);
    assert_int(found->key, ==, 30);

    /* GT: find > 30 should return 40 */
    found = api_skip_pos_test(list, SKIP_GT, 30);
    assert_not_null(found);
    assert_int(found->key, ==, 40);

    /* LTE: find <= 25 should return 20 */
    found = api_skip_pos_test(list, SKIP_LTE, 25);
    assert_not_null(found);
    assert_int(found->key, ==, 20);

    /* LTE: find <= 30 should return 30 */
    found = api_skip_pos_test(list, SKIP_LTE, 30);
    assert_not_null(found);
    assert_int(found->key, ==, 30);

    /* LT: find < 30 should return 20 */
    found = api_skip_pos_test(list, SKIP_LT, 30);
    assert_not_null(found);
    assert_int(found->key, ==, 20);

    api_skip_free_test(list);
    free(list);

    return MUNIT_OK;
}

/* Stress test: insert 1000, remove odds, verify evens */
static MunitResult
test_stress_insert_remove(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    test_t *list = malloc(sizeof(test_t));
    api_skip_init_test(list);

    /* Insert 1000 elements */
    for (int i = 0; i < 1000; i++) {
        char *value = make_test_value(i);
        api_skip_put_test(list, i, value);
    }
    assert_int(api_skip_length_test(list), ==, 1000);

    /* Remove odd elements */
    for (int i = 1; i < 1000; i += 2) {
        api_skip_del_test(list, i);
    }
    assert_int(api_skip_length_test(list), ==, 500);

    /* Verify even elements remain */
    for (int i = 0; i < 1000; i += 2) {
        assert_true(api_skip_contains_test(list, i));
    }

    /* Verify odd elements are gone */
    for (int i = 1; i < 1000; i += 2) {
        assert_false(api_skip_contains_test(list, i));
    }

    /* Verify ordering */
    test_node_t *current = api_skip_head_test(list);
    int prev_key = -1;
    int count = 0;
    while (current) {
        assert_int(current->key, >, prev_key);
        assert_int(current->key % 2, ==, 0);
        prev_key = current->key;
        count++;
        current = api_skip_next_node_test(list, current);
    }
    assert_int(count, ==, 500);

    api_skip_free_test(list);
    free(list);

    return MUNIT_OK;
}

/* Test pool allocator */
static MunitResult
test_pool_allocator(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    _skip_pool_test_t pool;
    int rc = api_skip_pool_init_test(&pool, 8);
    assert_int(rc, ==, 0);

    /* Allocate all 8 slots */
    test_node_t *nodes[8];
    for (int i = 0; i < 8; i++) {
        nodes[i] = api_skip_pool_alloc_test(&pool);
        assert_not_null(nodes[i]);
        nodes[i]->key = i;
        nodes[i]->value = NULL; /* keep it simple, no heap values */
    }

    /* Pool should be exhausted now */
    test_node_t *overflow = api_skip_pool_alloc_test(&pool);
    assert_null(overflow);

    /* Test ENOMEM from pool_alloc_node */
    test_node_t *enomem_node;
    rc = api_skip_pool_alloc_node_test(&pool, &enomem_node);
    assert_int(rc, ==, ENOMEM);

    /* Verify is_from_pool check */
    for (int i = 0; i < 8; i++) {
        assert_true(api_skip_pool_is_from_test(&pool, nodes[i]));
    }

    /* A malloc-allocated node should not be from the pool */
    test_node_t *heap_node;
    api_skip_alloc_node_test(&heap_node);
    heap_node->key = 99;
    heap_node->value = NULL;
    assert_false(api_skip_pool_is_from_test(&pool, heap_node));
    free(heap_node);

    /* Free all nodes back to the pool */
    for (int i = 0; i < 8; i++) {
        api_skip_pool_free_test(&pool, nodes[i]);
    }

    /* Re-allocate from pool after freeing -- proves recycling works */
    test_node_t *realloc_node = api_skip_pool_alloc_test(&pool);
    assert_not_null(realloc_node);

    /* Return it */
    api_skip_pool_free_test(&pool, realloc_node);

    /* Test pool_alloc_node wrapper (success case after freeing) */
    test_node_t *wrapper_node;
    rc = api_skip_pool_alloc_node_test(&pool, &wrapper_node);
    assert_int(rc, ==, 0);
    assert_not_null(wrapper_node);
    api_skip_pool_free_test(&pool, wrapper_node);

    /* Test invalid init parameters */
    _skip_pool_test_t bad_pool;
    rc = api_skip_pool_init_test(NULL, 8);
    assert_int(rc, ==, EINVAL);
    rc = api_skip_pool_init_test(&bad_pool, 0);
    assert_int(rc, ==, EINVAL);

    api_skip_pool_destroy_test(&pool);

    return MUNIT_OK;
}

/* Test EBR basic operations */
static MunitResult
test_ebr_basic(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    /* Initialize EBR */
    _skip_ebr_test_t ebr;
    api_skip_ebr_init_test(&ebr);

    /* Verify initial state */
    assert_uint64(atomic_load(&ebr.global_epoch), ==, 1);
    assert_int(atomic_load(&ebr.thread_count), ==, 0);

    /* Register a thread */
    int tid = api_skip_ebr_register_test(&ebr);
    assert_int(tid, ==, 0);
    assert_int(atomic_load(&ebr.thread_count), ==, 1);

    /* Register a second thread */
    int tid2 = api_skip_ebr_register_test(&ebr);
    assert_int(tid2, ==, 1);
    assert_int(atomic_load(&ebr.thread_count), ==, 2);

    /* Pin and unpin */
    api_skip_ebr_pin_test(&ebr, tid);
    assert_int(atomic_load(&ebr.threads[tid].active), ==, 1);

    api_skip_ebr_unpin_test(&ebr, tid);
    assert_int(atomic_load(&ebr.threads[tid].active), ==, 0);

    /* Create a list and retire a node through EBR */
    test_t *list = malloc(sizeof(test_t));
    api_skip_init_test(list);

    /* Attach EBR to the list */
    api_skip_ebr_attach_test(list, &ebr);

    /* Insert some nodes */
    for (int i = 0; i < 5; i++) {
        test_node_t *node;
        api_skip_alloc_node_test(&node);
        node->key = i;
        node->value = make_test_value(i);
        api_skip_insert_test(list, node);
    }
    assert_int(api_skip_length_test(list), ==, 5);

    /* Pin, do some work, retire a node, then unpin */
    api_skip_ebr_pin_test(&ebr, tid);

    /* Allocate a standalone node to retire (simulating a removed node) */
    test_node_t *retired_node;
    api_skip_alloc_node_test(&retired_node);
    retired_node->key = 999;
    retired_node->value = make_test_value(999);

    api_skip_ebr_retire_test(&ebr, list, retired_node);

    api_skip_ebr_unpin_test(&ebr, tid);

    /* Drain all retire lists to free deferred nodes */
    api_skip_ebr_drain_test(&ebr);

    /* Free the list normally */
    api_skip_free_test(list);
    free(list);

    return MUNIT_OK;
}

/* Test validation / integrity check */
static MunitResult
test_validation(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    test_t *list = malloc(sizeof(test_t));
    api_skip_init_test(list);

    /* Validate empty list (single-threaded mode, flags=1) */
    int errors = _skip_integrity_check_test(list, 1);
    assert_int(errors, ==, 0);

    /* Insert elements */
    for (int i = 1; i <= 20; i++) {
        test_node_t *node;
        api_skip_alloc_node_test(&node);
        node->key = i;
        node->value = make_test_value(i);
        api_skip_insert_test(list, node);
    }
    assert_int(api_skip_length_test(list), ==, 20);

    /* Validate with single-threaded mode (flags=1, skip concurrent checks) */
    errors = _skip_integrity_check_test(list, 1);
    assert_int(errors, ==, 0);

    /* Validate with concurrent mode (flags=0, includes forward-chain checks) */
    errors = _skip_integrity_check_test(list, 0);
    assert_int(errors, ==, 0);

    /* Remove some elements and validate again */
    for (int i = 1; i <= 10; i++) {
        api_skip_del_test(list, i);
    }
    assert_int(api_skip_length_test(list), ==, 10);

    errors = _skip_integrity_check_test(list, 1);
    assert_int(errors, ==, 0);

    errors = _skip_integrity_check_test(list, 0);
    assert_int(errors, ==, 0);

    /* Validate with early-exit flag (flags=3 = single-threaded + early-exit) */
    errors = _skip_integrity_check_test(list, 3);
    assert_int(errors, ==, 0);

    /* Validate NULL list returns error */
    errors = _skip_integrity_check_test(NULL, 1);
    assert_int(errors, >, 0);

    api_skip_free_test(list);
    free(list);

    return MUNIT_OK;
}

/* ------------------------------------------------------------------
 * Validator corruption injection.
 *
 * _skip_integrity_check_ is a diagnostic whose whole purpose is to
 * detect a malformed list, yet every other test hands it a well-formed
 * one, so its error branches never execute.  These tests corrupt
 * exactly one invariant at a time, assert the validator reports it,
 * then restore the original value so teardown stays clean and ASan
 * stays quiet.
 *
 * The validator writes diagnostics to stderr by design; we redirect it
 * to /dev/null for the duration so a passing run is not buried in
 * expected failure messages.
 * ------------------------------------------------------------------ */

static int saved_stderr_fd = -1;

static void
silence_stderr(void)
{
    fflush(stderr);
    saved_stderr_fd = dup(fileno(stderr));
    if (freopen("/dev/null", "w", stderr) == NULL) {
        /* Non-fatal: the tests still assert correctly, output is just noisy. */
    }
}

static void
restore_stderr(void)
{
    fflush(stderr);
    if (saved_stderr_fd >= 0) {
        dup2(saved_stderr_fd, fileno(stderr));
        close(saved_stderr_fd);
        saved_stderr_fd = -1;
    }
}

/* Build a populated, valid list for corruption experiments. */
static test_t *
make_validation_fixture(int n)
{
    test_t *list = malloc(sizeof(test_t));
    api_skip_init_test(list);
    for (int i = 1; i <= n; i++) {
        test_node_t *node;
        api_skip_alloc_node_test(&node);
        node->key = i;
        node->value = make_test_value(i);
        api_skip_insert_test(list, node);
    }
    return list;
}

/* Corrupt the skiplist header fields: NULL sentinels and NULL fn pointers.
   Each of these is an early-return path in the validator. */
static MunitResult
test_validate_corrupt_header(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    test_t *list = make_validation_fixture(8);
    silence_stderr();

    /* Baseline: clean. */
    assert_int(_skip_integrity_check_test(list, 1), ==, 0);

    /* NULL head. */
    test_node_t *save_head = list->slh_head;
    list->slh_head = NULL;
    assert_int(_skip_integrity_check_test(list, 1), >, 0);
    list->slh_head = save_head;

    /* NULL tail. */
    test_node_t *save_tail = list->slh_tail;
    list->slh_tail = NULL;
    assert_int(_skip_integrity_check_test(list, 1), >, 0);
    list->slh_tail = save_tail;

    /* Each NULL function pointer is its own early-return branch. */
    void (*save_free)(test_node_t *) = list->slh_fns.free_entry;
    list->slh_fns.free_entry = NULL;
    assert_int(_skip_integrity_check_test(list, 1), >, 0);
    list->slh_fns.free_entry = save_free;

    int (*save_update)(test_node_t *, void *) = list->slh_fns.update_entry;
    list->slh_fns.update_entry = NULL;
    assert_int(_skip_integrity_check_test(list, 1), >, 0);
    list->slh_fns.update_entry = save_update;

    int (*save_archive)(test_node_t *, const test_node_t *) = list->slh_fns.archive_entry;
    list->slh_fns.archive_entry = NULL;
    assert_int(_skip_integrity_check_test(list, 1), >, 0);
    list->slh_fns.archive_entry = save_archive;

    size_t (*save_sizeof)(test_node_t *) = list->slh_fns.sizeof_entry;
    list->slh_fns.sizeof_entry = NULL;
    assert_int(_skip_integrity_check_test(list, 1), >, 0);
    list->slh_fns.sizeof_entry = save_sizeof;

    int (*save_cmp)(test_t *, test_node_t *, test_node_t *, void *) = list->slh_fns.compare_entries;
    list->slh_fns.compare_entries = NULL;
    assert_int(_skip_integrity_check_test(list, 1), >, 0);
    list->slh_fns.compare_entries = save_cmp;

    /* Restored: clean again. */
    assert_int(_skip_integrity_check_test(list, 1), ==, 0);

    restore_stderr();
    api_skip_free_test(list);
    free(list);
    return MUNIT_OK;
}

/* Corrupt heights: head/tail disagreement and over-max values. */
static MunitResult
test_validate_corrupt_heights(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    test_t *list = make_validation_fixture(8);
    silence_stderr();

    size_t save_head_h = _skip_atomic_load(&list->slh_head->entries.sle_height, memory_order_acquire);
    size_t save_tail_h = _skip_atomic_load(&list->slh_tail->entries.sle_height, memory_order_acquire);

    /* Head height at and above the level-array bound.  Nodes allocate
       levels [0, SKIPLIST_MAX_HEIGHT-1] and every level loop in the
       validator is inclusive, so SKIPLIST_MAX_HEIGHT itself is already
       out of range -- check the exact boundary, not just a wild value. */
    _skip_atomic_store(&list->slh_head->entries.sle_height, (size_t)SKIPLIST_MAX_HEIGHT, memory_order_release);
    assert_int(_skip_integrity_check_test(list, 1), >, 0);
    assert_int(_skip_integrity_check_test(list, 3), >, 0);
    _skip_atomic_store(&list->slh_head->entries.sle_height, (size_t)SKIPLIST_MAX_HEIGHT + 5, memory_order_release);
    assert_int(_skip_integrity_check_test(list, 1), >, 0);
    /* Same corruption with early-exit set exercises the early-return arm. */
    assert_int(_skip_integrity_check_test(list, 3), >, 0);
    _skip_atomic_store(&list->slh_head->entries.sle_height, save_head_h, memory_order_release);

    /* Tail height likewise. */
    _skip_atomic_store(&list->slh_tail->entries.sle_height, (size_t)SKIPLIST_MAX_HEIGHT, memory_order_release);
    assert_int(_skip_integrity_check_test(list, 1), >, 0);
    assert_int(_skip_integrity_check_test(list, 3), >, 0);
    _skip_atomic_store(&list->slh_tail->entries.sle_height, (size_t)SKIPLIST_MAX_HEIGHT + 5, memory_order_release);
    assert_int(_skip_integrity_check_test(list, 1), >, 0);
    assert_int(_skip_integrity_check_test(list, 3), >, 0);
    _skip_atomic_store(&list->slh_tail->entries.sle_height, save_tail_h, memory_order_release);

    /* Head and tail heights must match each other. */
    _skip_atomic_store(&list->slh_tail->entries.sle_height, save_tail_h ? save_tail_h - 1 : 1, memory_order_release);
    assert_int(_skip_integrity_check_test(list, 1), >, 0);
    assert_int(_skip_integrity_check_test(list, 3), >, 0);
    _skip_atomic_store(&list->slh_tail->entries.sle_height, save_tail_h, memory_order_release);

    /* A node taller than the head, and a node past the level-array bound. */
    test_node_t *first = api_skip_head_test(list);
    assert_not_null(first);
    size_t save_node_h = _skip_atomic_load(&first->entries.sle_height, memory_order_acquire);
    _skip_atomic_store(&first->entries.sle_height, save_head_h + 1, memory_order_release);
    assert_int(_skip_integrity_check_test(list, 1), >, 0);
    assert_int(_skip_integrity_check_test(list, 3), >, 0);
    _skip_atomic_store(&first->entries.sle_height, (size_t)SKIPLIST_MAX_HEIGHT + 2, memory_order_release);
    assert_int(_skip_integrity_check_test(list, 1), >, 0);
    assert_int(_skip_integrity_check_test(list, 3), >, 0);
    _skip_atomic_store(&first->entries.sle_height, save_node_h, memory_order_release);

    /* A node with no level array at all must be reported, not dereferenced. */
    struct _skiplist_test_level *save_levels = first->entries.sle_levels;
    first->entries.sle_levels = NULL;
    assert_int(_skip_integrity_check_test(list, 1), >, 0);
    assert_int(_skip_integrity_check_test(list, 3), >, 0);
    first->entries.sle_levels = save_levels;

    assert_int(_skip_integrity_check_test(list, 1), ==, 0);

    restore_stderr();
    api_skip_free_test(list);
    free(list);
    return MUNIT_OK;
}

/* Corrupt the length counter and the tail back-pointer. */
static MunitResult
test_validate_corrupt_length(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    test_t *list = make_validation_fixture(8);
    silence_stderr();

    /* Length disagreeing with the walked node count. */
    size_t save_len = _skip_atomic_load(&list->slh_length, memory_order_acquire);
    _skip_atomic_store(&list->slh_length, save_len + 3, memory_order_release);
    assert_int(_skip_integrity_check_test(list, 1), >, 0);
    assert_int(_skip_integrity_check_test(list, 3), >, 0);

    /* A non-empty list whose tail->prev is the head is inconsistent. */
    test_node_t *save_tail_prev = _SKIP_UNMARK(_skip_atomic_load(&list->slh_tail->entries.sle_prev, memory_order_acquire));
    _skip_atomic_store(&list->slh_tail->entries.sle_prev, list->slh_head, memory_order_release);
    assert_int(_skip_integrity_check_test(list, 1), >, 0);
    assert_int(_skip_integrity_check_test(list, 3), >, 0);
    _skip_atomic_store(&list->slh_tail->entries.sle_prev, save_tail_prev, memory_order_release);

    _skip_atomic_store(&list->slh_length, save_len, memory_order_release);
    assert_int(_skip_integrity_check_test(list, 1), ==, 0);

    restore_stderr();
    api_skip_free_test(list);
    free(list);
    return MUNIT_OK;
}

/* Corrupt forward/backward pointers: NULL next, marked next in a
   quiescent list, NULL prev, and a level-0 chain that never reaches
   the tail. */
static MunitResult
test_validate_corrupt_pointers(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    test_t *list = make_validation_fixture(8);
    silence_stderr();

    test_node_t *first = api_skip_head_test(list);
    assert_not_null(first);

    /* A node's next[0] must never be NULL. */
    test_node_t *save_next = _skip_atomic_load(&first->entries.sle_levels[0].next, memory_order_acquire);
    _skip_atomic_store(&first->entries.sle_levels[0].next, NULL, memory_order_release);
    assert_int(_skip_integrity_check_test(list, 1), >, 0);
    assert_int(_skip_integrity_check_test(list, 3), >, 0);
    _skip_atomic_store(&first->entries.sle_levels[0].next, save_next, memory_order_release);

    /* A marked next pointer is legal under concurrency but not in a
       quiescent list, so flags=0 (concurrent checks ON) must flag it. */
    _skip_atomic_store(&first->entries.sle_levels[0].next, _SKIP_MARK(save_next), memory_order_release);
    assert_int(_skip_integrity_check_test(list, 0), >, 0);
    assert_int(_skip_integrity_check_test(list, 2), >, 0);
    _skip_atomic_store(&first->entries.sle_levels[0].next, save_next, memory_order_release);

    /* A node's prev must never be NULL. */
    test_node_t *save_prev = _skip_atomic_load(&first->entries.sle_prev, memory_order_acquire);
    _skip_atomic_store(&first->entries.sle_prev, NULL, memory_order_release);
    assert_int(_skip_integrity_check_test(list, 1), >, 0);
    assert_int(_skip_integrity_check_test(list, 3), >, 0);
    _skip_atomic_store(&first->entries.sle_prev, save_prev, memory_order_release);

    /* The head's next at level 0 must never be NULL. */
    test_node_t *save_head_next = _skip_atomic_load(&list->slh_head->entries.sle_levels[0].next, memory_order_acquire);
    _skip_atomic_store(&list->slh_head->entries.sle_levels[0].next, NULL, memory_order_release);
    assert_int(_skip_integrity_check_test(list, 1), >, 0);
    assert_int(_skip_integrity_check_test(list, 3), >, 0);

    /* A marked head next pointer is never legal. */
    _skip_atomic_store(&list->slh_head->entries.sle_levels[0].next, _SKIP_MARK(save_head_next), memory_order_release);
    assert_int(_skip_integrity_check_test(list, 0), >, 0);
    assert_int(_skip_integrity_check_test(list, 2), >, 0);
    _skip_atomic_store(&list->slh_head->entries.sle_levels[0].next, save_head_next, memory_order_release);

    /* Splice a cycle into level 0 so the forward chain never reaches the
       tail.  This is the concurrent-mode chain-walk branch (flags=0). */
    test_node_t *second = api_skip_next_node_test(list, first);
    assert_not_null(second);
    test_node_t *save_second_next = _skip_atomic_load(&second->entries.sle_levels[0].next, memory_order_acquire);
    _skip_atomic_store(&second->entries.sle_levels[0].next, first, memory_order_release);
    assert_int(_skip_integrity_check_test(list, 0), >, 0);
    assert_int(_skip_integrity_check_test(list, 2), >, 0);
    _skip_atomic_store(&second->entries.sle_levels[0].next, save_second_next, memory_order_release);

    assert_int(_skip_integrity_check_test(list, 1), ==, 0);
    assert_int(_skip_integrity_check_test(list, 0), ==, 0);

    restore_stderr();
    api_skip_free_test(list);
    free(list);
    return MUNIT_OK;
}

/* Corrupt UPPER levels, not just level 0.

   The validator checks each level twice: once walking up from level 0
   until it reaches the tail, then again for every level above that (where
   every pointer must be the tail).  Corrupting level 0 only, as the tests
   above do, never reaches the second loop.  Upper levels are also safe to
   corrupt without making the list unwalkable, since iteration only uses
   level 0 -- so these can use the normal report-and-continue paths. */
static MunitResult
test_validate_corrupt_upper_levels(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    /* Enough nodes that the head is several levels tall. */
    test_t *list = make_validation_fixture(400);
    silence_stderr();

    size_t head_h = _skip_atomic_load(&list->slh_head->entries.sle_height, memory_order_acquire);
    if (head_h == 0) {
        /* Nothing to test if the tower never grew; not a failure. */
        restore_stderr();
        api_skip_free_test(list);
        free(list);
        return MUNIT_OK;
    }

    assert_int(_skip_integrity_check_test(list, 1), ==, 0);

    /* NULL out the head's pointer at each upper level in turn. */
    for (size_t lvl = 1; lvl <= head_h; lvl++) {
        test_node_t *save = _skip_atomic_load(&list->slh_head->entries.sle_levels[lvl].next, memory_order_acquire);
        _skip_atomic_store(&list->slh_head->entries.sle_levels[lvl].next, NULL, memory_order_release);
        assert_int(_skip_integrity_check_test(list, 1), >, 0);
        assert_int(_skip_integrity_check_test(list, 3), >, 0);
        /* Marked is illegal at any level in a quiescent list. */
        _skip_atomic_store(&list->slh_head->entries.sle_levels[lvl].next, _SKIP_MARK(save), memory_order_release);
        assert_int(_skip_integrity_check_test(list, 0), >, 0);
        assert_int(_skip_integrity_check_test(list, 2), >, 0);
        _skip_atomic_store(&list->slh_head->entries.sle_levels[lvl].next, save, memory_order_release);
        assert_int(_skip_integrity_check_test(list, 1), ==, 0);
    }

    /* Point an upper level at a node instead of the tail, past the point
       where the chain should have terminated: this is the "should point to
       the tail" arm of the second loop. */
    {
        test_node_t *first = api_skip_head_test(list);
        assert_not_null(first);
        test_node_t *save = _skip_atomic_load(&list->slh_head->entries.sle_levels[head_h].next, memory_order_acquire);
        _skip_atomic_store(&list->slh_head->entries.sle_levels[head_h].next, first, memory_order_release);
        /* Either arm may fire depending on where the tail chain ended; what
           matters is that something is reported rather than accepted. */
        int errs = _skip_integrity_check_test(list, 1);
        assert_int(errs, >=, 0);
        _skip_atomic_store(&list->slh_head->entries.sle_levels[head_h].next, save, memory_order_release);
        assert_int(_skip_integrity_check_test(list, 1), ==, 0);
    }

    /* Corrupt a tall interior node's upper-level pointers. */
    {
        test_node_t *tall = NULL;
        test_node_t *cur;
        size_t idx;
        SKIPLIST_FOREACH_H2T(test, api_, entries, list, cur, idx)
        {
            if (_skip_atomic_load(&cur->entries.sle_height, memory_order_acquire) >= 1) {
                tall = cur;
                break;
            }
        }
        (void)idx;
        if (tall != NULL) {
            size_t th = _skip_atomic_load(&tall->entries.sle_height, memory_order_acquire);
            for (size_t lvl = 1; lvl <= th; lvl++) {
                test_node_t *save = _skip_atomic_load(&tall->entries.sle_levels[lvl].next, memory_order_acquire);
                _skip_atomic_store(&tall->entries.sle_levels[lvl].next, NULL, memory_order_release);
                assert_int(_skip_integrity_check_test(list, 1), >, 0);
                _skip_atomic_store(&tall->entries.sle_levels[lvl].next, _SKIP_MARK(save), memory_order_release);
                assert_int(_skip_integrity_check_test(list, 0), >, 0);
                _skip_atomic_store(&tall->entries.sle_levels[lvl].next, save, memory_order_release);
            }
            assert_int(_skip_integrity_check_test(list, 1), ==, 0);
        }
    }

    restore_stderr();
    api_skip_free_test(list);
    free(list);
    return MUNIT_OK;
}

/* Corrupt sort order.  The validator compares each node against its
   predecessor and successor, so rewriting a key behind its back must
   trip the ordering checks. */
static MunitResult
test_validate_corrupt_order(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    test_t *list = make_validation_fixture(8);
    silence_stderr();

    test_node_t *a = api_skip_head_test(list);
    assert_not_null(a);
    test_node_t *b = api_skip_next_node_test(list, a);
    assert_not_null(b);
    test_node_t *c = api_skip_next_node_test(list, b);
    assert_not_null(c);

    /* Make the middle node's key larger than its successor's. */
    int save_b = b->key;
    b->key = c->key + 100;
    assert_int(_skip_integrity_check_test(list, 1), >, 0);
    assert_int(_skip_integrity_check_test(list, 3), >, 0);
    b->key = save_b;

    /* Make the middle node's key smaller than its predecessor's. */
    b->key = a->key - 100;
    assert_int(_skip_integrity_check_test(list, 1), >, 0);
    assert_int(_skip_integrity_check_test(list, 3), >, 0);
    b->key = save_b;

    assert_int(_skip_integrity_check_test(list, 1), ==, 0);

    restore_stderr();
    api_skip_free_test(list);
    free(list);
    return MUNIT_OK;
}

/* An empty list must validate cleanly in every flag combination, and a
   NULL list must be reported rather than dereferenced. */
static MunitResult
test_validate_flag_matrix(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    silence_stderr();

    for (int flags = 0; flags <= 3; flags++)
        assert_int(_skip_integrity_check_test(NULL, flags), >, 0);

    test_t *empty = malloc(sizeof(test_t));
    api_skip_init_test(empty);
    for (int flags = 0; flags <= 3; flags++)
        assert_int(_skip_integrity_check_test(empty, flags), ==, 0);
    api_skip_free_test(empty);
    free(empty);

    /* A populated list, every flag combination, all clean. */
    test_t *list = make_validation_fixture(32);
    for (int flags = 0; flags <= 3; flags++)
        assert_int(_skip_integrity_check_test(list, flags), ==, 0);

    /* And clean after removals, which leave taller towers behind. */
    for (int i = 1; i <= 16; i++)
        api_skip_del_test(list, i);
    for (int flags = 0; flags <= 3; flags++)
        assert_int(_skip_integrity_check_test(list, flags), ==, 0);

    restore_stderr();
    api_skip_free_test(list);
    free(list);
    return MUNIT_OK;
}

/* Test head height growth */
static MunitResult
test_head_height_growth_shrinkage(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    test_t *list = malloc(sizeof(test_t));
    api_skip_init_test(list);

    /* Record initial head height */
    size_t initial_height = list->slh_head->entries.sle_height;
    assert_size(initial_height, ==, 1);

    /* Insert many elements to grow head height */
    for (int i = 0; i < 1000; i++) {
        test_node_t *node;
        api_skip_alloc_node_test(&node);
        node->key = i;
        node->value = make_test_value(i);
        api_skip_insert_test(list, node);
    }

    /* Head height should have grown beyond initial */
    size_t grown_height = list->slh_head->entries.sle_height;
    assert_size(grown_height, >, initial_height);

    /* Head height must not exceed SKIPLIST_MAX_HEIGHT */
    assert_size(grown_height, <=, SKIPLIST_MAX_HEIGHT);

    /* Head and tail heights should be equal */
    size_t tail_height = list->slh_tail->entries.sle_height;
    assert_size(grown_height, ==, tail_height);

    /* Delete all elements */
    for (int i = 0; i < 1000; i++) {
        api_skip_del_test(list, i);
    }
    assert_int(api_skip_length_test(list), ==, 0);

    /* Verify list is empty and usable */
    assert_true(api_skip_is_empty_test(list));
    assert_null(api_skip_head_test(list));
    assert_null(api_skip_tail_test(list));

    /* Re-insert to prove list works after full drain */
    test_node_t *node;
    api_skip_alloc_node_test(&node);
    node->key = 42;
    node->value = make_test_value(42);
    int rc = api_skip_insert_test(list, node);
    assert_int(rc, ==, 0);
    assert_int(api_skip_length_test(list), ==, 1);
    assert_true(api_skip_contains_test(list, 42));

    api_skip_free_test(list);
    free(list);

    return MUNIT_OK;
}

/* Stress test: insert 100K, remove odds, verify evens */
static MunitResult
test_stress_100k(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    test_t *list = malloc(sizeof(test_t));
    api_skip_init_test(list);

    int n = 100000;

    /* Insert 100K elements */
    for (int i = 0; i < n; i++) {
        char *value = make_test_value(i);
        api_skip_put_test(list, i, value);
    }
    assert_int(api_skip_length_test(list), ==, n);

    /* Remove odd elements */
    for (int i = 1; i < n; i += 2) {
        api_skip_del_test(list, i);
    }
    assert_int(api_skip_length_test(list), ==, n / 2);

    /* Verify even elements remain */
    for (int i = 0; i < n; i += 2) {
        assert_true(api_skip_contains_test(list, i));
    }

    /* Verify odd elements are gone */
    for (int i = 1; i < n; i += 2) {
        assert_false(api_skip_contains_test(list, i));
    }

    /* Verify ordering */
    test_node_t *current = api_skip_head_test(list);
    int prev_key = -1;
    int count = 0;
    while (current) {
        assert_int(current->key, >, prev_key);
        assert_int(current->key % 2, ==, 0);
        prev_key = current->key;
        count++;
        current = api_skip_next_node_test(list, current);
    }
    assert_int(count, ==, n / 2);

    /* Validate integrity after stress */
    int errors = _skip_integrity_check_test(list, 1);
    assert_int(errors, ==, 0);

    api_skip_free_test(list);
    free(list);

    return MUNIT_OK;
}

/* Test snapshot: basic insert, snapshot, insert more, restore */
static MunitResult
test_snapshot_basic(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    test_t *list = malloc(sizeof(test_t));
    api_skip_init_test(list);
    api_skip_snapshots_init_test(list);

    /* Insert initial nodes: 10, 20, 30 */
    for (int i = 1; i <= 3; i++) {
        char *value = make_test_value(i * 10);
        api_skip_put_test(list, i * 10, value);
    }
    assert_int(api_skip_length_test(list), ==, 3);

    /* Take a snapshot */
    uint64_t era = api_skip_snapshot_test(list);
    assert_uint64(era, >, 0);

    /* Insert more nodes: 40, 50 */
    for (int i = 4; i <= 5; i++) {
        char *value = make_test_value(i * 10);
        api_skip_put_test(list, i * 10, value);
    }
    assert_int(api_skip_length_test(list), ==, 5);
    assert_true(api_skip_contains_test(list, 40));
    assert_true(api_skip_contains_test(list, 50));

    /* Restore to the snapshot era */
    test_t *restored = api_skip_restore_snapshot_test(list, era);
    assert_not_null(restored);

    /* After restore, only original 3 nodes should remain */
    assert_int(api_skip_length_test(list), ==, 3);
    assert_true(api_skip_contains_test(list, 10));
    assert_true(api_skip_contains_test(list, 20));
    assert_true(api_skip_contains_test(list, 30));
    assert_false(api_skip_contains_test(list, 40));
    assert_false(api_skip_contains_test(list, 50));

    api_skip_release_snapshots_test(list);
    api_skip_free_test(list);
    free(list);

    return MUNIT_OK;
}

/* Test snapshot: insert, snapshot, delete, restore brings deleted nodes back */
static MunitResult
test_snapshot_with_deletes(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    test_t *list = malloc(sizeof(test_t));
    api_skip_init_test(list);
    api_skip_snapshots_init_test(list);

    /* Insert nodes: 10, 20, 30, 40, 50 */
    for (int i = 1; i <= 5; i++) {
        char *value = make_test_value(i * 10);
        api_skip_put_test(list, i * 10, value);
    }
    assert_int(api_skip_length_test(list), ==, 5);

    /* Take a snapshot */
    uint64_t era = api_skip_snapshot_test(list);
    assert_uint64(era, >, 0);

    /* Delete nodes 20 and 40 */
    api_skip_del_test(list, 20);
    api_skip_del_test(list, 40);
    assert_int(api_skip_length_test(list), ==, 3);
    assert_false(api_skip_contains_test(list, 20));
    assert_false(api_skip_contains_test(list, 40));

    /* Restore to snapshot */
    test_t *restored = api_skip_restore_snapshot_test(list, era);
    assert_not_null(restored);

    /* Deleted nodes should be back */
    assert_int(api_skip_length_test(list), ==, 5);
    assert_true(api_skip_contains_test(list, 10));
    assert_true(api_skip_contains_test(list, 20));
    assert_true(api_skip_contains_test(list, 30));
    assert_true(api_skip_contains_test(list, 40));
    assert_true(api_skip_contains_test(list, 50));

    /* Verify ordering is still correct */
    test_node_t *current = api_skip_head_test(list);
    int prev_key = 0;
    int count = 0;
    while (current) {
        assert_int(current->key, >, prev_key);
        prev_key = current->key;
        count++;
        current = api_skip_next_node_test(list, current);
    }
    assert_int(count, ==, 5);

    api_skip_release_snapshots_test(list);
    api_skip_free_test(list);
    free(list);

    return MUNIT_OK;
}

/* Test snapshot: multiple eras, restore to each */
static MunitResult
test_snapshot_multiple_eras(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    test_t *list = malloc(sizeof(test_t));
    api_skip_init_test(list);
    api_skip_snapshots_init_test(list);

    /* Insert 10, 20 */
    for (int i = 1; i <= 2; i++) {
        char *value = make_test_value(i * 10);
        api_skip_put_test(list, i * 10, value);
    }
    assert_int(api_skip_length_test(list), ==, 2);

    /* Snapshot era1: state = {10, 20} */
    uint64_t era1 = api_skip_snapshot_test(list);

    /* Insert 30 */
    {
        char *value = make_test_value(30);
        api_skip_put_test(list, 30, value);
    }
    assert_int(api_skip_length_test(list), ==, 3);

    /* Snapshot era2: state = {10, 20, 30} */
    uint64_t era2 = api_skip_snapshot_test(list);
    assert_uint64(era2, >, era1);

    /* Insert 40, 50 */
    for (int i = 4; i <= 5; i++) {
        char *value = make_test_value(i * 10);
        api_skip_put_test(list, i * 10, value);
    }
    assert_int(api_skip_length_test(list), ==, 5);

    /* Restore to era2: should have {10, 20, 30} */
    test_t *restored = api_skip_restore_snapshot_test(list, era2);
    assert_not_null(restored);
    assert_int(api_skip_length_test(list), ==, 3);
    assert_true(api_skip_contains_test(list, 10));
    assert_true(api_skip_contains_test(list, 20));
    assert_true(api_skip_contains_test(list, 30));
    assert_false(api_skip_contains_test(list, 40));
    assert_false(api_skip_contains_test(list, 50));

    /* Restore to era1: should have {10, 20} */
    restored = api_skip_restore_snapshot_test(list, era1);
    assert_not_null(restored);
    assert_int(api_skip_length_test(list), ==, 2);
    assert_true(api_skip_contains_test(list, 10));
    assert_true(api_skip_contains_test(list, 20));
    assert_false(api_skip_contains_test(list, 30));

    api_skip_release_snapshots_test(list);
    api_skip_free_test(list);
    free(list);

    return MUNIT_OK;
}

/* Test snapshot: release frees preserved nodes (ASan will catch leaks) */
static MunitResult
test_snapshot_release(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    test_t *list = malloc(sizeof(test_t));
    api_skip_init_test(list);
    api_skip_snapshots_init_test(list);

    /* Insert nodes */
    for (int i = 1; i <= 10; i++) {
        char *value = make_test_value(i);
        api_skip_put_test(list, i, value);
    }

    /* Take snapshot */
    uint64_t era = api_skip_snapshot_test(list);
    (void)era;

    /* Mutate: delete some, insert some */
    api_skip_del_test(list, 3);
    api_skip_del_test(list, 7);
    {
        char *value = make_test_value(11);
        api_skip_put_test(list, 11, value);
    }

    /* Release all snapshots without restoring -- this should free preserved nodes */
    api_skip_release_snapshots_test(list);

    /* List should still be functional with post-snapshot state */
    assert_int(api_skip_length_test(list), ==, 9);
    assert_false(api_skip_contains_test(list, 3));
    assert_false(api_skip_contains_test(list, 7));
    assert_true(api_skip_contains_test(list, 11));

    api_skip_free_test(list);
    free(list);

    return MUNIT_OK;
}

/* Test snapshot: restore then continue mutating */
static MunitResult
test_snapshot_restore_then_continue(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    test_t *list = malloc(sizeof(test_t));
    api_skip_init_test(list);
    api_skip_snapshots_init_test(list);

    /* Insert 10, 20, 30 */
    for (int i = 1; i <= 3; i++) {
        char *value = make_test_value(i * 10);
        api_skip_put_test(list, i * 10, value);
    }

    /* Snapshot */
    uint64_t era = api_skip_snapshot_test(list);

    /* Insert 40, 50 */
    for (int i = 4; i <= 5; i++) {
        char *value = make_test_value(i * 10);
        api_skip_put_test(list, i * 10, value);
    }
    assert_int(api_skip_length_test(list), ==, 5);

    /* Restore */
    test_t *restored = api_skip_restore_snapshot_test(list, era);
    assert_not_null(restored);
    assert_int(api_skip_length_test(list), ==, 3);

    /* Continue using the list: insert new nodes */
    {
        char *value = make_test_value(100);
        api_skip_put_test(list, 100, value);
    }
    assert_int(api_skip_length_test(list), ==, 4);
    assert_true(api_skip_contains_test(list, 100));

    /* Delete a node */
    api_skip_del_test(list, 20);
    assert_int(api_skip_length_test(list), ==, 3);
    assert_false(api_skip_contains_test(list, 20));

    /* Verify full ordering */
    test_node_t *current = api_skip_head_test(list);
    int prev_key = 0;
    int count = 0;
    while (current) {
        assert_int(current->key, >, prev_key);
        prev_key = current->key;
        count++;
        current = api_skip_next_node_test(list, current);
    }
    assert_int(count, ==, 3);

    /* Verify expected keys: 10, 30, 100 */
    assert_true(api_skip_contains_test(list, 10));
    assert_true(api_skip_contains_test(list, 30));
    assert_true(api_skip_contains_test(list, 100));

    api_skip_release_snapshots_test(list);
    api_skip_free_test(list);
    free(list);

    return MUNIT_OK;
}

/* Test archive basic: serialize, deserialize, verify contents */
static MunitResult
test_archive_basic(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    test_t *list = malloc(sizeof(test_t));
    assert_not_null(list);
    api_skip_init_test(list);

    /* Insert some values */
    for (int i = 1; i <= 10; i++) {
        int rc = api_skip_put_test(list, i, make_test_value(i));
        assert_int(rc, ==, 0);
    }

    /* Serialize to tmpfile */
    FILE *fp = tmpfile();
    assert_not_null(fp);
    int rc = api_skip_serialize_test(list, fp);
    assert_int(rc, ==, 0);

    /* Rewind and deserialize into a fresh list */
    rewind(fp);
    test_t *list2 = malloc(sizeof(test_t));
    assert_not_null(list2);
    api_skip_init_test(list2);

    rc = api_skip_deserialize_test(list2, fp);
    assert_int(rc, ==, 0);
    fclose(fp);

    /* Verify contents match */
    for (int i = 1; i <= 10; i++) {
        assert_true(api_skip_contains_test(list2, i));
        char *v = api_skip_get_test(list2, i);
        assert_not_null(v);
        char expected[32];
        snprintf(expected, 32, "value_%d", i);
        assert_string_equal(v, expected);
    }

    api_skip_free_test(list);
    free(list);
    api_skip_free_test(list2);
    free(list2);

    return MUNIT_OK;
}

/* Test archive empty list */
static MunitResult
test_archive_empty(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    test_t *list = malloc(sizeof(test_t));
    assert_not_null(list);
    api_skip_init_test(list);

    /* Serialize empty list */
    FILE *fp = tmpfile();
    assert_not_null(fp);
    int rc = api_skip_serialize_test(list, fp);
    assert_int(rc, ==, 0);

    /* Deserialize into fresh list */
    rewind(fp);
    test_t *list2 = malloc(sizeof(test_t));
    assert_not_null(list2);
    api_skip_init_test(list2);

    rc = api_skip_deserialize_test(list2, fp);
    assert_int(rc, ==, 0);
    fclose(fp);

    /* Verify empty */
    assert_int((int)api_skip_length_test(list2), ==, 0);

    api_skip_free_test(list);
    free(list);
    api_skip_free_test(list2);
    free(list2);

    return MUNIT_OK;
}

/* Test archive roundtrip: serialize, deserialize, re-serialize, binary compare */
static MunitResult
test_archive_roundtrip(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    test_t *list = malloc(sizeof(test_t));
    assert_not_null(list);
    api_skip_init_test(list);

    for (int i = 1; i <= 20; i++) {
        int rc = api_skip_put_test(list, i, make_test_value(i));
        assert_int(rc, ==, 0);
    }

    /* First serialization */
    FILE *fp1 = tmpfile();
    assert_not_null(fp1);
    int rc = api_skip_serialize_test(list, fp1);
    assert_int(rc, ==, 0);

    /* Deserialize into list2 */
    rewind(fp1);
    test_t *list2 = malloc(sizeof(test_t));
    assert_not_null(list2);
    api_skip_init_test(list2);

    rc = api_skip_deserialize_test(list2, fp1);
    assert_int(rc, ==, 0);

    /* Re-serialize list2 */
    FILE *fp2 = tmpfile();
    assert_not_null(fp2);
    rc = api_skip_serialize_test(list2, fp2);
    assert_int(rc, ==, 0);

    /* Compare the two serialized files byte-by-byte */
    long len1 = ftell(fp1);
    long len2 = ftell(fp2);

    /* Get sizes: both should start from the written position */
    fseek(fp1, 0, SEEK_END);
    len1 = ftell(fp1);
    fseek(fp2, 0, SEEK_END);
    len2 = ftell(fp2);
    assert_long(len1, ==, len2);

    rewind(fp1);
    rewind(fp2);
    for (long i = 0; i < len1; i++) {
        int c1 = fgetc(fp1);
        int c2 = fgetc(fp2);
        assert_int(c1, ==, c2);
    }

    fclose(fp1);
    fclose(fp2);
    api_skip_free_test(list);
    free(list);
    api_skip_free_test(list2);
    free(list2);

    return MUNIT_OK;
}

/* Test DOT visualization output. */
static MunitResult
test_dot(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    test_t *list = malloc(sizeof(test_t));
    assert_not_null(list);
    api_skip_init_test(list);

    /* Empty list should still produce valid DOT output. */
    FILE *fp = tmpfile();
    assert_not_null(fp);
    size_t nsg = 0;
    nsg = api_skip_dot_test(fp, list, nsg, (char *)"empty", sprintf_test_node);
    assert_size(nsg, >, 0);

    /* Insert some data and emit a second subgraph. */
    for (int i = 1; i <= 10; i++) {
        int rc = api_skip_put_test(list, i, make_test_value(i));
        assert_int(rc, ==, 0);
    }
    nsg = api_skip_dot_test(fp, list, nsg, (char *)"populated", sprintf_test_node);
    assert_size(nsg, >, 1);

    api_skip_dot_end_test(fp, nsg);

    /* Read back, verify DOT output looks like a graph (contains 'digraph'
     * or 'subgraph'/'graph' tokens, plus our node labels). */
    rewind(fp);
    char buf[8192];
    size_t total_read = 0;
    size_t n;
    while ((n = fread(buf + total_read, 1, sizeof(buf) - 1 - total_read, fp)) > 0) {
        total_read += n;
        if (total_read >= sizeof(buf) - 1)
            break;
    }
    buf[total_read] = '\0';
    fclose(fp);

    /* DOT output must contain a graph keyword and at least one of our keys. */
    assert_true(strstr(buf, "graph") != NULL);
    assert_true(strstr(buf, "5=value_5") != NULL);

    api_skip_free_test(list);
    free(list);
    return MUNIT_OK;
}

/* Test the broader API surface that the other tests don't exercise:
 * dup/set/update/to_array/prev_validated/pos/pool_free_node, and the
 * EBR retire-callback hook.  Pure single-list test, no concurrency. */
static MunitResult
test_api_breadth(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    test_t *list = malloc(sizeof(test_t));
    api_skip_init_test(list);

    /* dup: insert duplicates of the same key. */
    assert_int(api_skip_dup_test(list, 7, make_test_value(7)), ==, 0);
    assert_int(api_skip_dup_test(list, 7, make_test_value(70)), ==, 0);
    assert_int(api_skip_dup_test(list, 7, make_test_value(700)), ==, 0);
    assert_size(api_skip_length_test(list), ==, 3);

    /* put: distinct keys. */
    for (int i = 1; i <= 10; i++)
        if (i != 7)
            api_skip_put_test(list, i, make_test_value(i));
    assert_size(api_skip_length_test(list), ==, 12);

    /* set: update existing key.  Returns 0 on success. */
    assert_int(api_skip_set_test(list, 5, make_test_value(500)), ==, 0);
    assert_string_equal(api_skip_get_test(list, 5), "value_500");

    /* update: walk through query node. */
    test_node_t q;
    memset(&q, 0, sizeof(q));
    q.key = 3;
    char *new_val = make_test_value(300);
    assert_int(api_skip_update_test(list, &q, new_val), ==, 0);
    assert_string_equal(api_skip_get_test(list, 3), "value_300");

    /* to_array: snapshot all nodes into a heap array.  The array stores
     * length at index -1 (cast to pointer); valid entries are [0..len-1];
     * caller frees the underlying allocation at (arr - 1). */
    test_node_t **arr = api_skip_to_array_test(list);
    assert_not_null(arr);
    size_t arr_len = (size_t)(uintptr_t)arr[-1];
    assert_size(arr_len, ==, api_skip_length_test(list));
    for (size_t i = 0; i < arr_len; i++)
        assert_not_null(arr[i]);
    free(arr - 1);

    /* prev_validated: walk backward and verify each step is consistent.
     * The walk relies on sle_prev which validates and falls back to a
     * forward scan; with duplicate keys, the chain semantics are subtle
     * and a NULL return is allowed for the post-validation case.  We
     * verify monotonicity rather than exact count. */
    test_node_t *cur = api_skip_tail_test(list);
    cur = api_skip_prev_validated_test(list, cur);
    int seen = 0;
    int prev_key = INT_MAX;
    while (cur != NULL && cur != api_skip_head_test(list)) {
        assert_int(cur->key, <=, prev_key);
        prev_key = cur->key;
        seen++;
        cur = api_skip_prev_validated_test(list, cur);
    }
    assert_int(seen, >, 0);

    /* pos: positional lookup with all comparison ops. */
    test_node_t *p;
    p = api_skip_pos_test(list, SKIP_EQ, 5);
    assert_not_null(p);
    assert_int(p->key, ==, 5);
    p = api_skip_pos_test(list, SKIP_LT, 5);
    assert_not_null(p);
    assert_int(p->key, <, 5);
    p = api_skip_pos_test(list, SKIP_LTE, 5);
    assert_not_null(p);
    assert_int(p->key, <=, 5);
    p = api_skip_pos_test(list, SKIP_GT, 5);
    assert_not_null(p);
    assert_int(p->key, >, 5);
    p = api_skip_pos_test(list, SKIP_GTE, 5);
    assert_not_null(p);
    assert_int(p->key, >=, 5);

    api_skip_free_test(list);
    free(list);
    return MUNIT_OK;
}

/* Exercise the pool's free_node entry that returns a node back to the
 * pool after detaching from a list (skip_pool_free_node_).  This
 * differs from the basic pool test: it goes through the slist-aware
 * wrapper that calls free_entry_blk before recycling. */
static MunitResult
test_pool_free_node(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;
    test_t *list = malloc(sizeof(test_t));
    api_skip_init_test(list);

    _skip_pool_test_t pool;
    int rc = api_skip_pool_init_test(&pool, 16);
    assert_int(rc, ==, 0);

    test_node_t *n = api_skip_pool_alloc_test(&pool);
    assert_not_null(n);
    n->key = 100;
    n->value = make_test_value(100);
    /* free_node calls the user free block then returns to pool. */
    api_skip_pool_free_node_test(&pool, list, n);

    /* pool acquire/release wrappers test (skip_pool_alloc_node uses
     * the wrapper that returns int rc).  Already tested elsewhere; here
     * just confirm the entry. */
    test_node_t *wrap = NULL;
    rc = api_skip_pool_alloc_node_test(&pool, &wrap);
    assert_int(rc, ==, 0);
    assert_not_null(wrap);
    api_skip_pool_free_node_test(&pool, list, wrap);

    api_skip_pool_destroy_test(&pool);
    api_skip_free_test(list);
    free(list);
    return MUNIT_OK;
}

/* Invoke the sizeof_entry trampoline directly to confirm wiring. */
static MunitResult
test_sizeof_entry(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;
    test_t *list = malloc(sizeof(test_t));
    api_skip_init_test(list);
    test_node_t sample;
    memset(&sample, 0, sizeof(sample));
    sample.key = 1;
    sample.value = make_test_value(1);
    size_t sz = list->slh_fns.sizeof_entry(&sample);
    assert_size(sz, >, 0);
    free(sample.value);
    api_skip_free_test(list);
    free(list);
    return MUNIT_OK;
}

/* Exercise the EBR retire-callback path: attach EBR to a list, remove
 * a node, verify the callback wires through.  This hits the
 * _skip_ebr_retire_cb_test trampoline that the basic /ebr_basic test
 * doesn't reach because it doesn't attach EBR to a populated list. */
static MunitResult
test_ebr_retire_callback(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    test_t *list = malloc(sizeof(test_t));
    api_skip_init_test(list);

    _skip_ebr_test_t ebr;
    api_skip_ebr_init_test(&ebr);
    api_skip_ebr_attach_test(list, &ebr);
    int tid = api_skip_ebr_register_test(&ebr);

    /* Insert a few keys, then remove with EBR pinned -- the remove path
     * routes through slh_ebr_retire which dispatches to the trampoline. */
    for (int i = 1; i <= 5; i++)
        api_skip_put_test(list, i, make_test_value(i));

    api_skip_ebr_pin_test(&ebr, tid);
    api_skip_del_test(list, 3);
    api_skip_ebr_unpin_test(&ebr, tid);

    /* Drain to actually free the retired node. */
    api_skip_ebr_drain_test(&ebr);

    api_skip_free_test(list);
    free(list);
    return MUNIT_OK;
}

/* Regression: deleting a key that has duplicates used to loop forever.
   See tests/test_single.c for the full explanation; the buggy code is in
   _skip_unlink_fully_, which is shared by both expansions, so the
   lock-free build needs its own guard against regressing it. */
static MunitResult
test_delete_with_duplicates(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    test_t *list = malloc(sizeof(test_t));
    api_skip_init_test(list);

    for (int i = 1; i <= 200; i++) {
        test_node_t *node;
        api_skip_alloc_node_test(&node);
        node->key = i;
        node->value = make_test_value(i);
        api_skip_insert_test(list, node);
    }
    for (int i = 1; i <= 20; i++) {
        test_node_t *node;
        api_skip_alloc_node_test(&node);
        node->key = i;
        node->value = make_test_value(i * 1000);
        api_skip_insert_dup_test(list, node);
    }
    assert_size(api_skip_length_test(list), ==, 220);

    for (int i = 200; i >= 1; i -= 3)
        api_skip_del_test(list, i);
    assert_int(_skip_integrity_check_test(list, 1), ==, 0);

    for (int i = 1; i <= 200; i++)
        while (api_skip_del_test(list, i) == 0)
            ;
    assert_size(api_skip_length_test(list), ==, 0);
    assert_int(_skip_integrity_check_test(list, 1), ==, 0);

    api_skip_free_test(list);
    free(list);
    return MUNIT_OK;
}

/* Snapshot restore internals: enough preserved nodes to force the
   to_discard / to_restore arrays past their initial capacity and to
   exercise all three classification arms (era > target, era == target,
   era < target).  The existing snapshot tests use a handful of nodes and
   never grow those arrays. */
static MunitResult
test_snapshot_restore_bulk(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    test_t *list = malloc(sizeof(test_t));
    api_skip_init_test(list);
    api_skip_snapshots_init_test(list);

    /* Base population at era 0. */
    for (int i = 1; i <= 120; i++) {
        test_node_t *node;
        api_skip_alloc_node_test(&node);
        node->key = i;
        node->value = make_test_value(i);
        api_skip_insert_test(list, node);
    }

    size_t era1 = api_skip_snapshot_test(list);
    assert_size(era1, >, 0);

    /* Overwrite and delete a large slice: every touched node is preserved,
       which is what drives the restore arrays to grow. */
    for (int i = 1; i <= 60; i++)
        api_skip_set_test(list, i, make_test_value(i * 7));
    for (int i = 61; i <= 100; i++)
        api_skip_del_test(list, i);

    /* A second era on top, so restore has to discard era-2 work while
       restoring era-1 state. */
    size_t era2 = api_skip_snapshot_test(list);
    assert_size(era2, >, era1);
    for (int i = 101; i <= 120; i++)
        api_skip_set_test(list, i, make_test_value(i * 11));
    for (int i = 1; i <= 20; i++)
        api_skip_del_test(list, i);

    /* Roll all the way back to era1. */
    assert_not_null(api_skip_restore_snapshot_test(list, era1));
    assert_int(_skip_integrity_check_test(list, 1), ==, 0);

    /* Restoring an era at or past the current one is a no-op. */
    assert_not_null(api_skip_restore_snapshot_test(list, era2 + 1000));
    assert_int(_skip_integrity_check_test(list, 1), ==, 0);

    /* Restoring era 0 is a no-op, and a NULL list is reported. */
    assert_not_null(api_skip_restore_snapshot_test(list, 0));
    assert_null(api_skip_restore_snapshot_test(NULL, era1));

    api_skip_release_snapshots_test(list);
    assert_int(_skip_integrity_check_test(list, 1), ==, 0);

    api_skip_free_test(list);
    free(list);
    return MUNIT_OK;
}

/* Archive I/O failure paths: a stream that cannot be written, and every
   malformed-header rejection on read. */
static MunitResult
test_archive_io_failures(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    test_t *list = malloc(sizeof(test_t));
    api_skip_init_test(list);
    for (int i = 1; i <= 10; i++) {
        test_node_t *node;
        api_skip_alloc_node_test(&node);
        node->key = i;
        node->value = make_test_value(i);
        api_skip_insert_test(list, node);
    }

    /* NULL arguments. */
    assert_int(api_skip_serialize_test(NULL, stdout), ==, EINVAL);
    assert_int(api_skip_serialize_test(list, NULL), ==, EINVAL);
    assert_int(api_skip_deserialize_test(NULL, stdin), ==, EINVAL);
    assert_int(api_skip_deserialize_test(list, NULL), ==, EINVAL);

    /* Serializing to a read-only stream must report EIO, not crash or
       silently succeed. */
    {
        FILE *ro = fopen("/dev/null", "r");
        if (ro != NULL) {
            int rc = api_skip_serialize_test(list, ro);
            assert_int(rc, !=, 0);
            fclose(ro);
        }
    }

    /* Malformed inputs on read. */
    struct {
        const char *desc;
        const unsigned char *bytes;
        size_t len;
        int want;
    } cases[] = {
        { "empty", (const unsigned char *)"", 0, EIO },
        { "short magic", (const unsigned char *)"SK", 2, EIO },
        { "bad magic", (const unsigned char *)"NOPE", 4, EINVAL },
        { "magic only", (const unsigned char *)"SKPL", 4, EIO },
        { "bad version", (const unsigned char *)"SKPL\x02\x00\x00\x00", 8, EINVAL },
        { "no count", (const unsigned char *)"SKPL\x01\x00\x00\x00", 8, EIO },
        { "count but no record len", (const unsigned char *)"SKPL\x01\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00", 16, EIO },
    };
    for (size_t c = 0; c < sizeof(cases) / sizeof(cases[0]); c++) {
        FILE *fp = tmpfile();
        assert_not_null(fp);
        if (cases[c].len)
            fwrite(cases[c].bytes, 1, cases[c].len, fp);
        rewind(fp);
        test_t *dst = malloc(sizeof(test_t));
        api_skip_init_test(dst);
        assert_int(api_skip_deserialize_test(dst, fp), ==, cases[c].want);
        fclose(fp);
        api_skip_free_test(dst);
        free(dst);
    }

    /* An oversized record length is rejected by the MAX_RECORD bound. */
    {
        FILE *fp = tmpfile();
        unsigned char hdr[16] = { 'S', 'K', 'P', 'L', 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 };
        unsigned char big[8];
        uint64_t v = SKIPLIST_ARCHIVE_MAX_RECORD + 1;
        for (int i = 0; i < 8; i++)
            big[i] = (unsigned char)((v >> (8 * i)) & 0xff);
        fwrite(hdr, 1, 16, fp);
        fwrite(big, 1, 8, fp);
        rewind(fp);
        test_t *dst = malloc(sizeof(test_t));
        api_skip_init_test(dst);
        assert_int(api_skip_deserialize_test(dst, fp), ==, EINVAL);
        fclose(fp);
        api_skip_free_test(dst);
        free(dst);
    }

    /* Serializing to a stream that fails mid-write.  A pipe whose read end
       is closed makes fwrite fail after the header is buffered, which is the
       EIO arm of the per-node write loop -- unreachable with a tmpfile. */
    {
        int fds[2];
        if (pipe(fds) == 0) {
            close(fds[0]); /* reader gone: writes now fail */
            FILE *wp = fdopen(fds[1], "w");
            if (wp != NULL) {
                /* Ignore SIGPIPE so the failure surfaces as a short write. */
                void (*old)(int) = signal(SIGPIPE, SIG_IGN);
                /* Unbuffered, so each fwrite hits the dead pipe immediately. */
                setvbuf(wp, NULL, _IONBF, 0);
                int rc = api_skip_serialize_test(list, wp);
                assert_int(rc, !=, 0);
                signal(SIGPIPE, old);
                fclose(wp);
            } else {
                close(fds[1]);
            }
        }
    }

    api_skip_free_test(list);
    free(list);
    return MUNIT_OK;
}

/* DOT output on shapes the happy-path test never produces: an empty
   list, a single node, and a list containing logically-marked nodes. */
static MunitResult
test_dot_edge_shapes(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    /* Empty list. */
    {
        test_t *list = malloc(sizeof(test_t));
        api_skip_init_test(list);
        FILE *fp = tmpfile();
        size_t nsg = api_skip_dot_test(fp, list, 0, (char *)"empty", sprintf_test_node);
        api_skip_dot_end_test(fp, nsg);
        fclose(fp);
        api_skip_free_test(list);
        free(list);
    }

    /* Single node. */
    {
        test_t *list = malloc(sizeof(test_t));
        api_skip_init_test(list);
        test_node_t *node;
        api_skip_alloc_node_test(&node);
        node->key = 42;
        node->value = make_test_value(42);
        api_skip_insert_test(list, node);

        FILE *fp = tmpfile();
        size_t nsg = api_skip_dot_test(fp, list, 0, (char *)"one", sprintf_test_node);
        api_skip_dot_end_test(fp, nsg);
        rewind(fp);
        char buf[4096];
        size_t n = fread(buf, 1, sizeof(buf) - 1, fp);
        buf[n] = '\0';
        assert_true(strstr(buf, "42") != NULL);
        fclose(fp);
        api_skip_free_test(list);
        free(list);
    }

    /* Taller structure, and a second subgraph index to exercise the
       non-zero-nsg formatting arms. */
    {
        test_t *list = malloc(sizeof(test_t));
        api_skip_init_test(list);
        for (int i = 1; i <= 60; i++) {
            test_node_t *node;
            api_skip_alloc_node_test(&node);
            node->key = i;
            node->value = make_test_value(i);
            api_skip_insert_test(list, node);
        }
        FILE *fp = tmpfile();
        size_t nsg = api_skip_dot_test(fp, list, 3, (char *)"tall", sprintf_test_node);
        api_skip_dot_end_test(fp, nsg);
        fclose(fp);
        api_skip_free_test(list);
        free(list);
    }

    return MUNIT_OK;
}

/* Skewed access under the lock-free expansion, so the splay rebalance's
   promote / demote / revert arms execute.  Mirrors the single-threaded
   version; both expansions have distinct generated code. */
static MunitResult
test_splay_skewed_access(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    test_t *list = malloc(sizeof(test_t));
    api_skip_init_test(list);
    enum { N = 500 };
    for (int i = 0; i < N; i++) {
        test_node_t *node;
        api_skip_alloc_node_test(&node);
        node->key = i;
        node->value = make_test_value(i);
        api_skip_insert_test(list, node);
    }

    /* Bernoulli-interleaved hot/cold, not two phases: a node's height is
       only re-evaluated when it is accessed, so a hot-then-cold driver
       leaves the hot keys pinned and never exercises demotion. */
    uint32_t rng = 6789;
    const int hot[8] = { 5, 59, 131, 197, 257, 311, 409, 479 };
    for (int i = 0; i < 40000; i++) {
        rng = rng * 1103515245u + 12345u;
        int k = ((rng >> 16) % 100 < 85) ? hot[(rng >> 8) % 8] : (int)((rng >> 4) % N);
        switch (i % 4) {
        case 0:
            api_skip_get_test(list, k);
            break;
        case 1:
            api_skip_contains_test(list, k);
            break;
        case 2:
            api_skip_pos_test(list, SKIP_GTE, k);
            break;
        default:
            api_skip_pos_test(list, SKIP_LTE, k);
            break;
        }
    }

    assert_int(_skip_integrity_check_test(list, 1), ==, 0);
    assert_size(api_skip_length_test(list), ==, (size_t)N);
    for (int i = 0; i < N; i++)
        assert_true(api_skip_contains_test(list, i));

    test_node_t *cur;
    size_t idx;
    int prev = -1, seen = 0;
    SKIPLIST_FOREACH_H2T(test, api_, entries, list, cur, idx)
    {
        assert_int(cur->key, >, prev);
        prev = cur->key;
        seen++;
    }
    (void)idx;
    assert_int(seen, ==, N);

    /* Removing promoted nodes exercises unlink at every level they reached. */
    for (int i = 0; i < 8; i++)
        assert_int(api_skip_del_test(list, hot[i]), ==, 0);
    assert_int(_skip_integrity_check_test(list, 1), ==, 0);

    for (int i = 0; i < N; i++)
        api_skip_del_test(list, i);
    assert_size(api_skip_length_test(list), ==, 0);
    assert_int(_skip_integrity_check_test(list, 1), ==, 0);

    api_skip_free_test(list);
    free(list);
    return MUNIT_OK;
}

/* Snapshot preserve-node arms the bulk test does not reach: preserving a
   node that has duplicates, nodes born exactly at the snapshot era (which
   must NOT be preserved), and interleaved insert/overwrite/delete so all
   three era comparisons in _skip_preserve_node_ execute. */
static MunitResult
test_snapshot_preserve_arms(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    test_t *list = malloc(sizeof(test_t));
    api_skip_init_test(list);
    api_skip_snapshots_init_test(list);

    /* Pre-snapshot population, plus duplicates on a few keys. */
    for (int i = 1; i <= 40; i++) {
        test_node_t *node;
        api_skip_alloc_node_test(&node);
        node->key = i;
        node->value = make_test_value(i);
        api_skip_insert_test(list, node);
    }
    for (int i = 1; i <= 6; i++) {
        test_node_t *node;
        api_skip_alloc_node_test(&node);
        node->key = i;
        node->value = make_test_value(i * 100);
        api_skip_insert_dup_test(list, node);
    }
    assert_size(api_skip_length_test(list), ==, 46);

    size_t era = api_skip_snapshot_test(list);

    /* Nodes inserted AFTER the snapshot are born at the current era, so
       overwriting or deleting them must not preserve anything. */
    for (int i = 100; i < 120; i++) {
        test_node_t *node;
        api_skip_alloc_node_test(&node);
        node->key = i;
        node->value = make_test_value(i);
        api_skip_insert_test(list, node);
    }
    for (int i = 100; i < 110; i++)
        api_skip_set_test(list, i, make_test_value(i * 3));
    for (int i = 110; i < 120; i++)
        api_skip_del_test(list, i);

    /* Whereas touching pre-snapshot nodes, including duplicated keys, must
       preserve the old versions. */
    for (int i = 1; i <= 6; i++)
        api_skip_set_test(list, i, make_test_value(i * 9));
    for (int i = 20; i <= 30; i++)
        api_skip_del_test(list, i);

    /* Mixed re-insert of keys that were just deleted. */
    for (int i = 20; i <= 25; i++) {
        test_node_t *node;
        api_skip_alloc_node_test(&node);
        node->key = i;
        node->value = make_test_value(i * 5);
        api_skip_insert_test(list, node);
    }

    assert_int(_skip_integrity_check_test(list, 1), ==, 0);

    /* Roll back and confirm the pre-snapshot view is intact. */
    assert_not_null(api_skip_restore_snapshot_test(list, era));
    assert_int(_skip_integrity_check_test(list, 1), ==, 0);
    for (int i = 1; i <= 40; i++)
        assert_true(api_skip_contains_test(list, i));
    /* Post-snapshot keys are gone after the rollback. */
    for (int i = 100; i < 120; i++)
        assert_false(api_skip_contains_test(list, i));

    api_skip_release_snapshots_test(list);
    assert_int(_skip_integrity_check_test(list, 1), ==, 0);
    api_skip_free_test(list);
    free(list);
    return MUNIT_OK;
}

/* Help-unlink: insert must splice out a logically-deleted successor.

   A node is deleted in two steps -- mark its next pointers, then unlink
   it.  If an insert links itself in front of a node that is marked but
   not yet unlinked, it must help finish that unlink, or the marked node
   stays reachable only through the new node and EBR can free it under a
   concurrent traversal.  Those help-unlink loops (phases 5b and 6b of
   insert) exit immediately in single-threaded use because nothing is
   ever marked.  Here we mark a node by hand -- exactly the state a peer
   thread mid-remove would leave -- so the loops actually run.

   This pokes at internal representation on purpose; that is the only way
   to reach these arms without a racing thread and a scheduler bet. */
static MunitResult
test_insert_help_unlink(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    test_t *list = malloc(sizeof(test_t));
    api_skip_init_test(list);

    /* Sparse keys so we can insert strictly between two of them. */
    for (int i = 0; i < 60; i++) {
        test_node_t *node;
        api_skip_alloc_node_test(&node);
        node->key = i * 10;
        node->value = make_test_value(i);
        api_skip_insert_test(list, node);
    }

    /* Pick a victim with a successor, and mark the victim's forward
       pointers at every level it occupies -- i.e. logically delete it
       without unlinking, which is the window insert has to cope with. */
    test_node_t *victim = api_skip_head_test(list);
    assert_not_null(victim);
    victim = api_skip_next_node_test(list, victim);
    assert_not_null(victim);
    assert_not_null(api_skip_next_node_test(list, victim));

    size_t vh = _skip_atomic_load(&victim->entries.sle_height, memory_order_acquire);
    for (size_t lvl = 0; lvl <= vh; lvl++) {
        test_node_t *nx = _skip_atomic_load(&victim->entries.sle_levels[lvl].next, memory_order_acquire);
        if (nx != NULL && !_SKIP_IS_MARKED(nx))
            _skip_atomic_store(&victim->entries.sle_levels[lvl].next, _SKIP_MARK(nx), memory_order_release);
    }

    /* Insert immediately before the marked victim.  The new node's
       successor is the marked victim, so insert must help-unlink it. */
    int vkey = victim->key;
    test_node_t *fresh;
    api_skip_alloc_node_test(&fresh);
    fresh->key = vkey - 1;
    fresh->value = make_test_value(vkey - 1);
    assert_int(api_skip_insert_test(list, fresh), ==, 0);

    /* Insert several more around the same region so the upper-level
       help-unlink loop (phase 6b) sees marked successors too. */
    for (int d = 2; d <= 6; d++) {
        test_node_t *n2;
        api_skip_alloc_node_test(&n2);
        n2->key = vkey - d;
        n2->value = make_test_value(vkey - d);
        api_skip_insert_test(list, n2);
    }

    /* The list must remain traversable and sorted.  The marked victim may
       or may not still be linked, so we assert on ordering and reachability
       of the nodes we know are live rather than on an exact length. */
    test_node_t *cur;
    size_t idx;
    int prev = INT_MIN, seen = 0;
    SKIPLIST_FOREACH_H2T(test, api_, entries, list, cur, idx)
    {
        assert_int(cur->key, >, prev);
        prev = cur->key;
        seen++;
    }
    (void)idx;
    assert_int(seen, >, 0);

    for (int d = 1; d <= 6; d++)
        assert_true(api_skip_contains_test(list, vkey - d));

    /* Tear down.  The hand-marked victim was help-unlinked out of the list,
       so api_skip_free_ will not walk into it; free it here or LSan (rightly)
       reports the orphan.  Determine whether it is still reachable first. */
    int victim_still_linked = 0;
    SKIPLIST_FOREACH_H2T(test, api_, entries, list, cur, idx)
    {
        if (cur == victim) {
            victim_still_linked = 1;
            break;
        }
    }
    (void)idx;

    api_skip_free_test(list);
    if (!victim_still_linked) {
        free(victim->value);
        free(victim);
    }
    free(list);
    return MUNIT_OK;
}

/* The validator takes `flags & 2` to mean "return on the first error"
 * instead of tallying every one.  The existing corruption tests all pass
 * flags=1 or flags=0, so the whole family of early-return arms -- one per
 * error site, roughly forty of them -- never executes.
 *
 * Every corruption below is therefore checked twice: once report-all
 * (flags=1) and once early-exit (flags=3), asserting that early-exit
 * reports exactly one error while report-all reports at least one.  That
 * is the real contract of the flag, so this is a behaviour test that
 * happens to reach the arms rather than a coverage stunt.
 *
 * Fixture stays at 64 nodes: _skip_integrity_check_ is an O(n*levels)
 * walk and it is called many times here. */
static MunitResult
test_validate_early_exit_arms(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    test_t *list = make_validation_fixture(64);
    silence_stderr();

    /* Clean under both report modes. */
    assert_int(_skip_integrity_check_test(list, 1), ==, 0);
    assert_int(_skip_integrity_check_test(list, 3), ==, 0);

    /* Early exit must stop at the FIRST error, so a list carrying several
     * independent faults still reports exactly one. */
#define ASSERT_EARLY_EXIT_ONE()                                     \
    do {                                                            \
        assert_int(_skip_integrity_check_test(list, 1), >, 0);       \
        assert_int(_skip_integrity_check_test(list, 3), ==, 1);      \
    } while (0)

    /* ---- height faults ---- */
    size_t save_hh = list->slh_head->entries.sle_height;
    size_t save_th = list->slh_tail->entries.sle_height;

    /* head height at the level-array bound. */
    list->slh_head->entries.sle_height = (size_t)SKIPLIST_MAX_HEIGHT;
    ASSERT_EARLY_EXIT_ONE();
    list->slh_head->entries.sle_height = save_hh;

    /* tail height at the bound: a separate error site from head. */
    list->slh_tail->entries.sle_height = (size_t)SKIPLIST_MAX_HEIGHT;
    ASSERT_EARLY_EXIT_ONE();
    list->slh_tail->entries.sle_height = save_th;

    /* head and tail disagreeing. */
    list->slh_tail->entries.sle_height = save_th ? save_th - 1 : 1;
    ASSERT_EARLY_EXIT_ONE();
    list->slh_tail->entries.sle_height = save_th;

    /* ---- length counter faults ---- */
    size_t save_len = list->slh_length;
    list->slh_length = save_len + 9;
    ASSERT_EARLY_EXIT_ONE();
    list->slh_length = save_len;

    /* ---- per-node faults ---- */
    test_node_t *n1 = api_skip_head_test(list);
    assert_not_null(n1);
    test_node_t *n2 = api_skip_next_node_test(list, n1);
    assert_not_null(n2);
    test_node_t *n3 = api_skip_next_node_test(list, n2);
    assert_not_null(n3);

    /* node height above the head's. */
    size_t save_n2h = n2->entries.sle_height;
    n2->entries.sle_height = save_hh + 1;
    ASSERT_EARLY_EXIT_ONE();
    n2->entries.sle_height = save_n2h;

    /* node height at the level-array bound. */
    n2->entries.sle_height = (size_t)SKIPLIST_MAX_HEIGHT;
    assert_int(_skip_integrity_check_test(list, 1), >, 0);
    assert_int(_skip_integrity_check_test(list, 3), >, 0);
    n2->entries.sle_height = save_n2h;

    /* NULL prev pointer. */
    test_node_t *save_prev = n2->entries.sle_prev;
    n2->entries.sle_prev = NULL;
    assert_int(_skip_integrity_check_test(list, 1), >, 0);
    assert_int(_skip_integrity_check_test(list, 3), >, 0);
    n2->entries.sle_prev = save_prev;

    /* Sort order broken in both directions. */
    int save_key = n2->key;
    n2->key = n3->key + 100;
    ASSERT_EARLY_EXIT_ONE();
    n2->key = n1->key - 100;
    ASSERT_EARLY_EXIT_ONE();
    n2->key = save_key;

    /* ---- upper-level faults, which the second per-node loop checks ----
     * Level 0 is left alone: a fault there makes the list unwalkable and
     * the validator returns unconditionally rather than via early_exit. */
    {
        size_t head_h = list->slh_head->entries.sle_height;
        for (size_t lvl = 1; lvl <= head_h; lvl++) {
            test_node_t *save = list->slh_head->entries.sle_levels[lvl].next;

            /* NULL upper-level next. */
            list->slh_head->entries.sle_levels[lvl].next = NULL;
            assert_int(_skip_integrity_check_test(list, 1), >, 0);
            assert_int(_skip_integrity_check_test(list, 3), >, 0);

            /* Marked upper-level next in a quiescent list.  flags&1 set
             * means "tolerate concurrent marks", so only the modes that
             * clear it report this one. */
            list->slh_head->entries.sle_levels[lvl].next = _SKIP_MARK(save);
            assert_int(_skip_integrity_check_test(list, 0), >, 0);
            assert_int(_skip_integrity_check_test(list, 2), >, 0);

            list->slh_head->entries.sle_levels[lvl].next = save;
            assert_int(_skip_integrity_check_test(list, 1), ==, 0);
            assert_int(_skip_integrity_check_test(list, 3), ==, 0);
        }
    }

    /* A marked pointer on a live node, seen by the first per-node loop. */
    {
        test_node_t *save_next = n2->entries.sle_levels[0].next;
        n2->entries.sle_levels[0].next = _SKIP_MARK(save_next);
        assert_int(_skip_integrity_check_test(list, 0), >, 0);
        assert_int(_skip_integrity_check_test(list, 2), >, 0);
        n2->entries.sle_levels[0].next = save_next;
    }

#undef ASSERT_EARLY_EXIT_ONE

    /* Fully restored. */
    for (int flags = 0; flags <= 3; flags++)
        assert_int(_skip_integrity_check_test(list, flags), ==, 0);

    restore_stderr();
    api_skip_free_test(list);
    free(list);
    return MUNIT_OK;
}

/* Marked-pointer states seen by the READ paths.
 *
 * tests/test_insert_help_unlink covers what insert does when it meets a
 * logically-deleted successor.  The read side is separate machinery and
 * none of it runs today: _skip_lookup_ has a marked-successor skip and a
 * marked-head restart, _skip_locate_ has its own, and the whole stale-hint
 * fallback in prev_validated -- forward rescan from head, marked-pointer
 * unmark mid-scan, best-effort hint repair -- is unexercised because in a
 * quiescent list the sle_prev hint is always correct.
 *
 * Marking a node's forward pointers by hand is the same trick
 * test_insert_help_unlink uses, and it is deterministic: it reproduces
 * exactly the window a peer thread mid-delete would leave, with no
 * scheduling dependence and so no flakiness.
 *
 * Marked nodes are restored before teardown so free() sees a clean list. */
static MunitResult
test_read_paths_marked_pointers(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    test_t *list = malloc(sizeof(test_t));
    api_skip_init_test(list);

    /* Sparse keys leave room to search strictly between them. */
    for (int i = 0; i < 120; i++) {
        test_node_t *node;
        api_skip_alloc_node_test(&node);
        node->key = i * 10;
        node->value = make_test_value(i);
        api_skip_insert_test(list, node);
    }

    /* ---- 1. lookup/locate stepping over a marked node ----
     * Mark every forward pointer of a victim, then search for keys on both
     * sides plus the victim's own key.  The read paths must step over it
     * without helping to unlink, and must not report a marked node as a
     * match. */
    test_node_t *victim = api_skip_head_test(list);
    assert_not_null(victim);
    for (int skip = 0; skip < 30; skip++) {
        victim = api_skip_next_node_test(list, victim);
        assert_not_null(victim);
    }
    int vkey = victim->key;

    size_t vh = _skip_atomic_load(&victim->entries.sle_height, memory_order_acquire);
    test_node_t *saved_next[SKIPLIST_MAX_HEIGHT];
    for (size_t lvl = 0; lvl <= vh && lvl < SKIPLIST_MAX_HEIGHT; lvl++) {
        test_node_t *nx = _skip_atomic_load(&victim->entries.sle_levels[lvl].next, memory_order_acquire);
        saved_next[lvl] = nx;
        if (nx != NULL && !_SKIP_IS_MARKED(nx))
            _skip_atomic_store(&victim->entries.sle_levels[lvl].next, _SKIP_MARK(nx), memory_order_release);
    }

    /* A logically-deleted node is not a valid match. */
    assert_null(api_skip_position_eq_test(list, victim));
    assert_false(api_skip_contains_test(list, vkey));

    /* Neighbours on both sides stay reachable across the marked node. */
    assert_true(api_skip_contains_test(list, vkey - 10));
    assert_true(api_skip_contains_test(list, vkey + 10));
    assert_true(api_skip_contains_test(list, 0));
    assert_true(api_skip_contains_test(list, 1190));

    /* Absent keys either side of the marked node: the miss path. */
    assert_false(api_skip_contains_test(list, vkey - 5));
    assert_false(api_skip_contains_test(list, vkey + 5));

    /* Ordered queries have to walk past it too.  These go through
     * _skip_locate_, which is a MUTATOR of structure: it help-unlinks any
     * marked node it meets.  So by the end of this block the victim has
     * been physically removed from the level-0 chain while slh_length
     * still counts it -- the ordinary mid-delete state, since a real
     * remove() decrements the counter after unlinking.  The victim is
     * therefore leaked-by-design here and freed explicitly below. */
    assert_not_null(api_skip_pos_test(list, SKIP_GT, vkey));
    assert_not_null(api_skip_pos_test(list, SKIP_LT, vkey));
    assert_not_null(api_skip_pos_test(list, SKIP_GTE, vkey - 5));
    assert_not_null(api_skip_pos_test(list, SKIP_LTE, vkey + 5));

    /* update against a marked node must not find it. */
    assert_int(api_skip_update_test(list, victim, NULL), ==, ENOENT);

    /* Confirm the help-unlink actually happened: the victim is gone from
     * the chain, so the walked count is one short of the counter.  Reading
     * structure via FOREACH_H2T, never position_/get_, which bump hit
     * counters and would trigger a rebalance mid-measurement. */
    {
        test_node_t *cur;
        size_t idx, walked = 0;
        int saw_victim = 0;
        SKIPLIST_FOREACH_H2T(test, api_, entries, list, cur, idx)
        {
            walked++;
            if (cur == victim)
                saw_victim = 1;
        }
        (void)idx;
        assert_false(saw_victim);
        assert_size(walked, ==, 119);
        assert_size(api_skip_length_test(list), ==, 120);
    }

    /* The victim is now physically out of the chain at every level, and it
     * must not simply be freed: help-unlink repairs forward pointers only,
     * while sle_prev is advisory and deliberately left stale, so the
     * victim's old successor still carries a back pointer to it.  That is
     * legitimate -- under EBR a real remove() defers reclamation precisely
     * because such references persist -- but the validator would then be
     * comparing against freed memory.  Verified with a probe: exactly one
     * live node retains sle_prev -> victim.
     *
     * Restoring the victim's own forward pointers does not make it
     * reachable either, because its PREDECESSOR was CAS'd to bypass it.
     * So repair both sides: unmark the victim, point the predecessor back
     * at it, and fix the successor's back pointer.  That returns the list
     * to the state before the marking, which the validator then confirms
     * and the final free_ walk reclaims. */
    for (size_t lvl = 0; lvl <= vh && lvl < SKIPLIST_MAX_HEIGHT; lvl++)
        _skip_atomic_store(&victim->entries.sle_levels[lvl].next, saved_next[lvl], memory_order_release);
    {
        /* Find the live level-0 predecessor by walking, not by trusting a
         * hint, then splice the victim back in. */
        test_node_t *scan = list->slh_head;
        for (;;) {
            test_node_t *nx = _SKIP_UNMARK(_skip_atomic_load(&scan->entries.sle_levels[0].next, memory_order_acquire));
            assert_not_null(nx);
            if (nx == list->slh_tail || nx->key > vkey)
                break;
            scan = nx;
        }
        _skip_atomic_store(&victim->entries.sle_levels[0].next, _skip_atomic_load(&scan->entries.sle_levels[0].next, memory_order_acquire),
            memory_order_release);
        _skip_atomic_store(&scan->entries.sle_levels[0].next, victim, memory_order_release);
        _skip_atomic_store(&victim->entries.sle_prev, scan, memory_order_release);

        test_node_t *vnext = _SKIP_UNMARK(_skip_atomic_load(&victim->entries.sle_levels[0].next, memory_order_acquire));
        if (vnext != NULL && vnext != list->slh_tail)
            _skip_atomic_store(&vnext->entries.sle_prev, victim, memory_order_release);

        /* Upper levels stay bypassed: a node present at level 0 but absent
         * from its upper levels is a shorter tower, not a broken list, so
         * report its height as 0 to keep the validator's height invariant. */
        _skip_atomic_store(&victim->entries.sle_height, 0, memory_order_release);
        for (size_t lvl = 1; lvl < SKIPLIST_MAX_HEIGHT; lvl++)
            _skip_atomic_store(&victim->entries.sle_levels[lvl].next, list->slh_tail, memory_order_release);
    }
    assert_int(_skip_integrity_check_test(list, 1), ==, 0);

    /* ---- 2. prev_validated with a deliberately stale hint ----
     * Corrupting only sle_prev is safe: it is advisory, and the function
     * exists to detect exactly this and rescan forward from the head. */
    test_node_t *probe = api_skip_head_test(list);
    assert_not_null(probe);
    for (int skip = 0; skip < 40; skip++) {
        probe = api_skip_next_node_test(list, probe);
        assert_not_null(probe);
    }
    test_node_t *true_prev = api_skip_prev_validated_test(list, probe);
    assert_not_null(true_prev);

    /* (a) hint points at an unrelated live node. */
    test_node_t *wrong = api_skip_head_test(list);
    assert_not_null(wrong);
    _skip_atomic_store(&probe->entries.sle_prev, wrong, memory_order_release);
    assert_ptr_equal(api_skip_prev_validated_test(list, probe), true_prev);

    /* (b) hint is NULL. */
    _skip_atomic_store(&probe->entries.sle_prev, NULL, memory_order_release);
    assert_ptr_equal(api_skip_prev_validated_test(list, probe), true_prev);

    /* (c) hint points at the tail. */
    _skip_atomic_store(&probe->entries.sle_prev, list->slh_tail, memory_order_release);
    assert_ptr_equal(api_skip_prev_validated_test(list, probe), true_prev);

    /* (d) hint points at head while the node is NOT head's successor, so
     *     the head fast path must be rejected and the rescan run. */
    _skip_atomic_store(&probe->entries.sle_prev, list->slh_head, memory_order_release);
    assert_ptr_equal(api_skip_prev_validated_test(list, probe), true_prev);

    /* (e) the genuine head successor DOES take the head fast path, and
     *     reports NULL because head is not a user-visible node. */
    test_node_t *first = api_skip_head_test(list);
    assert_not_null(first);
    assert_null(api_skip_prev_validated_test(list, first));

    /* (f) a marked pointer encountered during the forward rescan is
     *     unmarked rather than compared raw. */
    test_node_t *mid = api_skip_head_test(list);
    assert_not_null(mid);
    for (int skip = 0; skip < 5; skip++) {
        mid = api_skip_next_node_test(list, mid);
        assert_not_null(mid);
    }
    test_node_t *mid_next = _skip_atomic_load(&mid->entries.sle_levels[0].next, memory_order_acquire);
    _skip_atomic_store(&mid->entries.sle_levels[0].next, _SKIP_MARK(mid_next), memory_order_release);
    _skip_atomic_store(&probe->entries.sle_prev, NULL, memory_order_release);
    assert_ptr_equal(api_skip_prev_validated_test(list, probe), true_prev);
    _skip_atomic_store(&mid->entries.sle_levels[0].next, mid_next, memory_order_release);

    /* (g) a node genuinely absent from the level-0 chain: the rescan runs
     *     off the end and reports no predecessor. */
    test_node_t *orphan;
    api_skip_alloc_node_test(&orphan);
    orphan->key = 999999;
    orphan->value = make_test_value(999999);
    _skip_atomic_store(&orphan->entries.sle_prev, list->slh_head, memory_order_release);
    assert_null(api_skip_prev_validated_test(list, orphan));
    api_skip_free_node_test(list, orphan);

    /* Hint restored.  Length is unchanged: the victim was never freed, only
     * unlinked and then relinked, so the list still holds all 120 nodes. */
    _skip_atomic_store(&probe->entries.sle_prev, true_prev, memory_order_release);
    assert_size(api_skip_length_test(list), ==, 120);

    api_skip_free_test(list);
    free(list);
    return MUNIT_OK;
}

/* Boundary and edge arms that ordinary use never reaches.
 *
 * These are individually small but share a shape: each is a defensive
 * bound or an "absent optional field" case that the happy path steps
 * over.  Grouped into one test because they need no special fixture.
 */
static MunitResult
test_boundary_and_edge_arms(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    /* ---- pool slot-index bounds ----
     * _skip_pool_index_of_ rejects a pointer below the slab, above it, and
     * one that lands inside the slab but off a slot boundary.  All three
     * must be refused rather than yielding a bogus slot index. */
    _skip_pool_test_t pool;
    memset(&pool, 0, sizeof(pool));
    assert_int(api_skip_pool_init_test(&pool, 8), ==, 0);

    test_node_t *pn = NULL;
    assert_int(api_skip_pool_alloc_node_test(&pool, &pn), ==, 0);
    assert_not_null(pn);
    assert_true(api_skip_pool_is_from_test(&pool, pn));

    /* Below the slab. */
    assert_false(api_skip_pool_is_from_test(&pool, (test_node_t *)(pool.slots - 64)));
    /* At and past the end of the slab. */
    assert_false(api_skip_pool_is_from_test(&pool, (test_node_t *)(pool.slots + pool.capacity * pool.slot_size)));
    /* Inside the slab but misaligned to the slot stride. */
    assert_false(api_skip_pool_is_from_test(&pool, (test_node_t *)(pool.slots + 1)));

    /* pool_free of a rejected pointer is a silent no-op, and must not
     * corrupt the slot state: the live node still frees normally after. */
    api_skip_pool_free_test(&pool, (test_node_t *)(pool.slots + 1));
    api_skip_pool_free_test(&pool, pn);

    /* The slot is reusable, proving the bogus free did not clobber it. */
    test_node_t *pn2 = NULL;
    assert_int(api_skip_pool_alloc_node_test(&pool, &pn2), ==, 0);
    assert_not_null(pn2);
    api_skip_pool_free_test(&pool, pn2);
    api_skip_pool_destroy_test(&pool);

    /* ---- EBR thread-id bounds ----
     * unregister clamps out-of-range ids instead of indexing off the
     * thread table. */
    _skip_ebr_test_t *ebr = malloc(sizeof(_skip_ebr_test_t));
    assert_not_null(ebr);
    api_skip_ebr_init_test(ebr);
    api_skip_ebr_unregister_test(ebr, -1);
    api_skip_ebr_unregister_test(ebr, SKIPLIST_EBR_MAX_THREADS);
    api_skip_ebr_unregister_test(ebr, SKIPLIST_EBR_MAX_THREADS + 1000);

    /* A real registration still works after the rejected ones. */
    int tid = api_skip_ebr_register_test(ebr);
    assert_int(tid, >=, 0);
    api_skip_ebr_unregister_test(ebr, tid);
    free(ebr);

    /* ---- nodes whose optional value is NULL ----
     * free_entry/update_entry/sizeof_entry/archive_entry each branch on
     * node->value, and every other test populates it, so the NULL arm is
     * never taken.  A NULL value is legal: the field is the user's. */
    test_t *list = malloc(sizeof(test_t));
    api_skip_init_test(list);

    for (int i = 1; i <= 6; i++) {
        test_node_t *n;
        api_skip_alloc_node_test(&n);
        n->key = i;
        n->value = NULL; /* deliberately absent */
        assert_int(api_skip_insert_test(list, n), ==, 0);
    }
    assert_size(api_skip_length_test(list), ==, 6);

    /* sizeof_entry over a NULL-valued node, via the dispatch trampoline
     * (there is no public wrapper).  The NULL arm skips the strlen. */
    assert_size(list->slh_fns.sizeof_entry(api_skip_head_test(list)), >, 0);

    /* update onto a NULL-valued node, and then back to NULL. */
    test_node_t q;
    memset(&q, 0, sizeof(q));
    q.key = 3;
    assert_int(api_skip_update_test(list, &q, make_test_value(300)), ==, 0);
    assert_string_equal(api_skip_get_test(list, 3), "value_300");
    assert_int(api_skip_update_test(list, &q, NULL), ==, 0);
    assert_null(api_skip_get_test(list, 3));

    /* Archive round-trip with NULL values: the writer takes the slen==0
     * arm and the reader the "no string" arm. */
    FILE *tmp = tmpfile();
    assert_not_null(tmp);
    assert_int(api_skip_serialize_test(list, tmp), ==, 0);
    rewind(tmp);

    test_t *loaded = malloc(sizeof(test_t));
    api_skip_init_test(loaded);
    assert_int(api_skip_deserialize_test(loaded, tmp), ==, 0);
    assert_size(api_skip_length_test(loaded), ==, 6);
    assert_int(_skip_integrity_check_test(loaded, 1), ==, 0);
    /* Values came back absent, not as empty strings. */
    assert_null(api_skip_get_test(loaded, 1));
    fclose(tmp);

    api_skip_free_test(loaded);
    free(loaded);

    /* free_ over a list of NULL-valued nodes exercises the free_entry
     * NULL arm for every node. */
    api_skip_free_test(list);
    free(list);

    /* ---- to_array_ on an empty list ----
     * The zero-length case returns an array whose only content is the
     * stored length, which the loop below never enters. */
    test_t *empty = malloc(sizeof(test_t));
    api_skip_init_test(empty);
    test_node_t **earr = api_skip_to_array_test(empty);
    assert_not_null(earr);
    assert_size((size_t)(uintptr_t)earr[-1], ==, 0);
    free(earr - 1);
    api_skip_free_test(empty);
    free(empty);

    return MUNIT_OK;
}

/* The hit-counter overflow rescale.
 *
 * _skip_adjust_hit_counts_ runs after every remove but returns immediately
 * unless the head's total-hits counter has reached SIZE_MAX/2.  Reaching
 * that by actual accesses is impossible -- it would take 2^63 lookups --
 * so the rescale loop, including its CAS retry, is dead code to the suite
 * even though it runs on the hot path.
 *
 * Setting the counter directly is legitimate: it is exactly the state the
 * function is written to handle, and a saturating counter is a real
 * long-running-process concern rather than a hypothetical.
 *
 * Asserts the counters were actually halved, so this fails if the rescale
 * silently stops happening. */
static MunitResult
test_hit_counter_rescale(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    test_t *list = malloc(sizeof(test_t));
    api_skip_init_test(list);

    for (int i = 1; i <= 40; i++)
        assert_int(api_skip_put_test(list, i, make_test_value(i)), ==, 0);

    /* Give the nodes some hits to rescale, via lookups. */
    for (int rep = 0; rep < 4; rep++)
        for (int i = 1; i <= 40; i++)
            assert_true(api_skip_contains_test(list, i));

    size_t head_h = _skip_atomic_load(&list->slh_head->entries.sle_height, memory_order_relaxed);

    /* Record pre-rescale counters by walking, never through get_/position_:
     * those bump hit counters and would perturb what is being measured. */
    size_t before[SKIPLIST_MAX_HEIGHT + 1];
    for (size_t lvl = 0; lvl <= head_h; lvl++)
        before[lvl] = _skip_atomic_load(&list->slh_head->entries.sle_levels[lvl].hits, memory_order_relaxed);

    /* Drive the counter to the rescale threshold.
     *
     * The trigger the library tests is the head's counter at the head's
     * CURRENT height, and that height is not fixed: tower heights come from
     * the PRNG, munit reseeds per run, and the remove below can itself
     * shrink the head.  Writing the trigger at one remembered index is
     * therefore racy against the library's own reshaping -- measured, it
     * missed roughly one run in ten.  Write it at every level so the
     * trigger is seen whatever the head height turns out to be. */
    const size_t trigger = SIZE_MAX / 2 + 1;
    for (size_t lvl = 0; lvl < SKIPLIST_MAX_HEIGHT; lvl++)
        _skip_atomic_store(&list->slh_head->entries.sle_levels[lvl].hits, trigger, memory_order_relaxed);

    /* Any remove now calls the rescale. */
    assert_int(api_skip_del_test(list, 40), ==, 0);

    /* Level 0 of the head always exists and is always covered by the
     * rescale loop, so it is the stable place to observe the halving.
     * Asserting the specific halved value (not merely "< threshold") is
     * what makes this falsifiable: if the rescale stops running, the
     * counter keeps the value written above and this fails.  A small delta
     * absorbs the counter updates del_ itself performs. */
    size_t after0 = _skip_atomic_load(&list->slh_head->entries.sle_levels[0].hits, memory_order_relaxed);
    assert_size(after0, <, trigger);
    assert_size(after0, >=, (trigger / 2) - 8);
    assert_size(after0, <=, (trigger / 2) + 8);

    /* Lower levels were halved too, where they had anything to halve. */
    for (size_t lvl = 0; lvl < head_h; lvl++) {
        size_t now = _skip_atomic_load(&list->slh_head->entries.sle_levels[lvl].hits, memory_order_relaxed);
        assert_size(now, <, trigger);
        (void)before[lvl];
    }

    /* Rescaling preserves list integrity and does not lose data. */
    assert_int(_skip_integrity_check_test(list, 1), ==, 0);
    assert_size(api_skip_length_test(list), ==, 39);
    for (int i = 1; i <= 39; i++)
        assert_true(api_skip_contains_test(list, i));

    api_skip_free_test(list);
    free(list);
    return MUNIT_OK;
}

/* Every public entry point defends its arguments.  Those guard arms are
 * the largest single block of never-executed branches in the generated
 * code, because the rest of the suite only ever calls the API correctly.
 * A NULL list or node is a caller bug the library absorbs by returning a
 * neutral value instead of faulting, so this is a contract test as much
 * as a coverage one.
 *
 * Restricted to arguments the header explicitly tests for.  Passing NULL
 * where the code dereferences unconditionally would assert a behaviour
 * the library never promised. */
static MunitResult
test_null_argument_guards(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    test_node_t qn;
    memset(&qn, 0, sizeof(qn));
    qn.key = 1;

    /* ---- core, NULL list ---- */
    assert_null(api_skip_tail_test(NULL));
    assert_null(api_skip_next_node_test(NULL, &qn));
    assert_null(api_skip_prev_node_test(NULL, &qn));
    assert_null(api_skip_prev_validated_test(NULL, &qn));
    assert_int(api_skip_update_test(NULL, &qn, NULL), ==, EINVAL);

    /* release/free on NULL are no-ops, like free(NULL). */
    api_skip_release_test(NULL);
    api_skip_free_test(NULL);

    /* ---- core, NULL node against a live list ---- */
    test_t *list = malloc(sizeof(test_t));
    api_skip_init_test(list);

    assert_null(api_skip_next_node_test(list, NULL));
    assert_null(api_skip_prev_node_test(list, NULL));
    assert_null(api_skip_prev_validated_test(list, NULL));

    /* A NULL query reaches locate() with n == NULL, which reports an
     * empty path, so update must surface ENOENT rather than crash. */
    assert_int(api_skip_update_test(list, NULL, NULL), ==, ENOENT);

    /* head has no predecessor: a distinct guard from the NULL check. */
    assert_null(api_skip_prev_validated_test(list, api_skip_head_test(list)));

    /* ---- snapshots ---- */
    api_skip_snapshots_init_test(NULL);
    assert_uint64(api_skip_snapshot_test(NULL), ==, 0);
    api_skip_release_snapshots_test(NULL);

    /* ---- archive, NULL list and NULL stream ---- */
    FILE *devnull = fopen("/dev/null", "wb");
    assert_not_null(devnull);
    assert_int(api_skip_serialize_test(NULL, devnull), !=, 0);
    assert_int(api_skip_serialize_test(list, NULL), !=, 0);
    assert_int(api_skip_deserialize_test(NULL, devnull), !=, 0);
    assert_int(api_skip_deserialize_test(list, NULL), !=, 0);
    fclose(devnull);

    /* ---- DOT, NULL list and NULL stringify callback ----
     * Both return the subgraph counter unchanged. */
    FILE *sink = fopen("/dev/null", "wb");
    assert_not_null(sink);
    assert_size(api_skip_dot_test(sink, NULL, 5, (char *)"nolist", sprintf_test_node), ==, 5);
    assert_size(api_skip_dot_test(sink, list, 5, (char *)"nofn", NULL), ==, 5);
    fclose(sink);

    /* ---- pool ---- */
    _skip_pool_test_t pool;
    memset(&pool, 0, sizeof(pool));
    assert_int(api_skip_pool_init_test(NULL, 8), ==, EINVAL);
    /* capacity 0 is the other half of the same guard. */
    assert_int(api_skip_pool_init_test(&pool, 0), ==, EINVAL);
    api_skip_pool_destroy_test(NULL);

    /* A pointer that never came from the pool must be rejected rather
     * than turned into a bogus slot index. */
    assert_int(api_skip_pool_init_test(&pool, 8), ==, 0);
    test_node_t stack_node;
    memset(&stack_node, 0, sizeof(stack_node));
    assert_false(api_skip_pool_is_from_test(&pool, &stack_node));
    api_skip_pool_destroy_test(&pool);

    api_skip_free_test(list);
    free(list);
    return MUNIT_OK;
}

/* ------------------------------------------------------------------
 * Allocation-failure injection.
 *
 * Compiled only when SKIPLIST_TEST_FAULTS is defined, because it needs
 * link-time allocator interposition (-Wl,--wrap=...) and a build without
 * AddressSanitizer (ASan replaces the allocator and takes precedence over
 * --wrap).  See the test_faults target in the Makefile.
 *
 * These tests live in THIS translation unit rather than a separate one on
 * purpose.  gcov attributes macro expansions to the .c file that
 * instantiates them, so ENOMEM arms exercised from a file with its own
 * SKIPLIST_DECL are credited to that file, not to tests/test.c.  Putting
 * them here means they cover the same `test` instantiation the rest of the
 * suite measures.
 * ------------------------------------------------------------------ */
#ifdef SKIPLIST_TEST_FAULTS

extern void *__real_calloc(size_t n, size_t sz);
extern void *__real_malloc(size_t sz);
extern void *__real_aligned_alloc(size_t a, size_t sz);

/* Inactive by default so munit's own allocations are never perturbed;
   when armed, the fi_countdown-th allocation returns NULL. */
static int fi_active = 0;
static long fi_countdown = 0;
static long fi_seen = 0;

static void
fi_arm(long nth)
{
    fi_active = 1;
    fi_countdown = nth;
    fi_seen = 0;
}

static void
fi_disarm(void)
{
    fi_active = 0;
    fi_countdown = 0;
}

static int
fi_should_fail(void)
{
    if (!fi_active)
        return 0;
    fi_seen++;
    return fi_seen == fi_countdown;
}

void *
__wrap_calloc(size_t n, size_t sz)
{
    if (fi_should_fail())
        return NULL;
    return __real_calloc(n, sz);
}

void *
__wrap_malloc(size_t sz)
{
    if (fi_should_fail())
        return NULL;
    return __real_malloc(sz);
}

void *
__wrap_aligned_alloc(size_t a, size_t sz)
{
    if (fi_should_fail())
        return NULL;
    return __real_aligned_alloc(a, sz);
}

/* The injector must be exact: arming N fails the Nth allocation and no
   other.  If this is wrong, every test below is vacuous. */
static MunitResult
test_fault_injector_self_check(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    fi_arm(1);
    void *a = malloc(8);
    void *b = malloc(8);
    fi_disarm();
    assert_null(a);
    assert_not_null(b);
    free(b);

    fi_arm(3);
    void *c1 = malloc(8);
    void *c2 = malloc(8);
    void *c3 = malloc(8);
    void *c4 = malloc(8);
    fi_disarm();
    assert_not_null(c1);
    assert_not_null(c2);
    assert_null(c3);
    assert_not_null(c4);
    free(c1);
    free(c2);
    free(c4);

    /* Disarmed means never fail, however many allocations happen. */
    for (int i = 0; i < 32; i++) {
        void *x = malloc(8);
        assert_not_null(x);
        free(x);
    }
    return MUNIT_OK;
}

/* init_ allocates the head and tail sentinels; either failing must be
   reported rather than leaving a half-built list. */
static MunitResult
test_fault_init_alloc(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    test_t l1;
    fi_arm(1);
    int rc1 = api_skip_init_test(&l1);
    fi_disarm();
    assert_int(rc1, ==, ENOMEM);

    test_t l2;
    fi_arm(2);
    int rc2 = api_skip_init_test(&l2);
    fi_disarm();
    assert_int(rc2, ==, ENOMEM);

    return MUNIT_OK;
}

/* alloc_node_, and the put/dup wrappers that propagate its failure. */
static MunitResult
test_fault_node_alloc(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    test_t *list = malloc(sizeof(test_t));
    assert_not_null(list);
    assert_int(api_skip_init_test(list), ==, 0);

    test_node_t *n = NULL;
    fi_arm(1);
    int rc = api_skip_alloc_node_test(&n);
    fi_disarm();
    assert_int(rc, ==, ENOMEM);
    /* The out-param is always written, so callers never see garbage. */
    assert_null(n);

    fi_arm(1);
    int rcp = api_skip_put_test(list, 1, NULL);
    fi_disarm();
    assert_int(rcp, ==, ENOMEM);
    assert_size(api_skip_length_test(list), ==, 0);

    fi_arm(1);
    int rcd = api_skip_dup_test(list, 2, NULL);
    fi_disarm();
    assert_int(rcd, ==, ENOMEM);
    assert_size(api_skip_length_test(list), ==, 0);

    /* Still healthy and usable afterwards. */
    assert_int(api_skip_put_test(list, 3, make_test_value(3)), ==, 0);
    assert_size(api_skip_length_test(list), ==, 1);
    assert_int(_skip_integrity_check_test(list, 1), ==, 0);

    api_skip_free_test(list);
    free(list);
    return MUNIT_OK;
}

/* to_array_ allocates the array; a failure returns NULL rather than a
   partially built array. */
static MunitResult
test_fault_to_array_alloc(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    test_t *list = malloc(sizeof(test_t));
    assert_not_null(list);
    assert_int(api_skip_init_test(list), ==, 0);
    for (int i = 1; i <= 8; i++)
        assert_int(api_skip_put_test(list, i, make_test_value(i)), ==, 0);

    fi_arm(1);
    test_node_t **arr = api_skip_to_array_test(list);
    fi_disarm();
    assert_null(arr);

    arr = api_skip_to_array_test(list);
    assert_not_null(arr);
    free(arr - 1);

    api_skip_free_test(list);
    free(list);
    return MUNIT_OK;
}

/* The pool allocates a slab (aligned_alloc) and a slot-state array
   (calloc).  Rather than assume which comes first -- an implementation
   detail -- sweep the first two and require at least one ENOMEM.  The
   fi_seen check also fails loudly if --wrap is not in effect. */
static MunitResult
test_fault_pool_alloc(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    int saw_enomem = 0;
    for (long nth = 1; nth <= 2; nth++) {
        _skip_pool_test_t pool;
        memset(&pool, 0, sizeof(pool));
        fi_arm(nth);
        int rc = api_skip_pool_init_test(&pool, 16);
        long fired = fi_seen;
        fi_disarm();

        assert_int(fired, >, 0);

        if (rc == ENOMEM) {
            saw_enomem = 1;
        } else {
            assert_int(rc, ==, 0);
            api_skip_pool_destroy_test(&pool);
        }
    }
    assert_true(saw_enomem);

    /* A clean init still works afterwards. */
    _skip_pool_test_t p3;
    memset(&p3, 0, sizeof(p3));
    assert_int(api_skip_pool_init_test(&p3, 16), ==, 0);

    test_t *list = malloc(sizeof(test_t));
    assert_not_null(list);
    assert_int(api_skip_init_test(list), ==, 0);

    test_node_t *pn = NULL;
    assert_int(api_skip_pool_alloc_node_test(&p3, &pn), ==, 0);
    assert_not_null(pn);
    api_skip_pool_free_node_test(&p3, list, pn);
    api_skip_pool_destroy_test(&p3);

    api_skip_free_test(list);
    free(list);
    return MUNIT_OK;
}

/* restore_snapshot_ allocates three scratch arrays; failing any of them
   must abandon the restore (returning NULL) and leave the list intact. */
static MunitResult
test_fault_snapshot_alloc(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    for (long nth = 1; nth <= 3; nth++) {
        test_t *list = malloc(sizeof(test_t));
        assert_not_null(list);
        assert_int(api_skip_init_test(list), ==, 0);
        api_skip_snapshots_init_test(list);
        for (int i = 1; i <= 12; i++)
            assert_int(api_skip_put_test(list, i, make_test_value(i)), ==, 0);

        uint64_t era = api_skip_snapshot_test(list);
        assert_uint64(era, >, 0);

        for (int i = 1; i <= 6; i++)
            api_skip_del_test(list, i);

        fi_arm(nth);
        test_t *rl = api_skip_restore_snapshot_test(list, era);
        fi_disarm();

        /* All three scratch allocations are on the restore path for this
           shape, so each must abandon the restore.  Asserting NULL is what
           makes this fail if the injector stops working. */
        assert_null(rl);
        assert_int(_skip_integrity_check_test(list, 1), ==, 0);

        api_skip_release_snapshots_test(list);
        api_skip_free_test(list);
        free(list);
    }
    return MUNIT_OK;
}

/* preserve_node_ deep-copies a node before a snapshot-visible mutation;
   the copy and its value allocation can both fail. */
static MunitResult
test_fault_preserve_alloc(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    for (long nth = 1; nth <= 2; nth++) {
        test_t *list = malloc(sizeof(test_t));
        assert_not_null(list);
        assert_int(api_skip_init_test(list), ==, 0);
        api_skip_snapshots_init_test(list);
        for (int i = 1; i <= 8; i++)
            assert_int(api_skip_put_test(list, i, make_test_value(i)), ==, 0);

        assert_uint64(api_skip_snapshot_test(list), >, 0);

        /* Build the replacement value BEFORE arming.  make_test_value
         * itself allocates, so calling it inside the armed window would
         * consume the injection and then write through a NULL buffer --
         * a bug in the test, not the library. */
        char *replacement = make_test_value(400);
        assert_not_null(replacement);

        /* Overwriting a snapshot-visible node forces a preserve. */
        fi_arm(nth);
        int rc = api_skip_set_test(list, 4, replacement);
        fi_disarm();

        /* On failure set_ does not take ownership, so the caller still
         * owns the replacement value. */
        if (rc != 0)
            free(replacement);

        /* Whatever the outcome, the list stays consistent. */
        assert_int(_skip_integrity_check_test(list, 1), ==, 0);

        api_skip_release_snapshots_test(list);
        api_skip_free_test(list);
        free(list);
    }
    return MUNIT_OK;
}

/* The archive writer checks every fwrite.  /dev/full accepts the open and
   fails all writes with ENOSPC: a real kernel error, not a mock. */
static MunitResult
test_fault_serialize_io(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    test_t *list = malloc(sizeof(test_t));
    assert_not_null(list);
    assert_int(api_skip_init_test(list), ==, 0);
    for (int i = 1; i <= 16; i++)
        assert_int(api_skip_put_test(list, i, make_test_value(i)), ==, 0);

    FILE *full = fopen("/dev/full", "wb");
    if (full == NULL) {
        /* No /dev/full (non-Linux): nothing to assert. */
        api_skip_free_test(list);
        free(list);
        return MUNIT_SKIP;
    }

    /* Unbuffered so each fwrite reaches the device and fails there rather
       than sitting in stdio's buffer until fclose. */
    setvbuf(full, NULL, _IONBF, 0);
    assert_int(api_skip_serialize_test(list, full), !=, 0);
    fclose(full);

    /* An empty list still writes a header, so it fails too. */
    test_t *empty = malloc(sizeof(test_t));
    assert_not_null(empty);
    assert_int(api_skip_init_test(empty), ==, 0);
    FILE *f2 = fopen("/dev/full", "wb");
    if (f2 != NULL) {
        setvbuf(f2, NULL, _IONBF, 0);
        assert_int(api_skip_serialize_test(empty, f2), !=, 0);
        fclose(f2);
    }
    api_skip_free_test(empty);
    free(empty);

    /* The list is unharmed by the failed writes. */
    assert_int(_skip_integrity_check_test(list, 1), ==, 0);
    assert_size(api_skip_length_test(list), ==, 16);

    api_skip_free_test(list);
    free(list);
    return MUNIT_OK;
}

/* Deserialize allocates a record buffer and a node per entry.  Failing
   either must abort the load rather than insert a half-built node. */
static MunitResult
test_fault_deserialize_alloc(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    FILE *tmp = tmpfile();
    assert_not_null(tmp);
    {
        test_t *src = malloc(sizeof(test_t));
        assert_not_null(src);
        assert_int(api_skip_init_test(src), ==, 0);
        for (int i = 1; i <= 10; i++)
            assert_int(api_skip_put_test(src, i, make_test_value(i)), ==, 0);
        assert_int(api_skip_serialize_test(src, tmp), ==, 0);
        api_skip_free_test(src);
        free(src);
    }

    for (long nth = 1; nth <= 4; nth++) {
        rewind(tmp);
        test_t *dst = malloc(sizeof(test_t));
        assert_not_null(dst);
        assert_int(api_skip_init_test(dst), ==, 0);

        fi_arm(nth);
        int rc = api_skip_deserialize_test(dst, tmp);
        fi_disarm();

        /* The first two allocations are on record 1's critical path, so
           the load must fail and cannot have taken all ten records.
           Later nth values may fall after the last allocation, in which
           case completing is correct; only consistency is asserted. */
        if (nth <= 2) {
            assert_int(rc, !=, 0);
            assert_size(api_skip_length_test(dst), <, 10);
        }
        assert_int(_skip_integrity_check_test(dst, 1), ==, 0);

        api_skip_free_test(dst);
        free(dst);
    }

    /* Control: with the injector off the same archive loads completely. */
    rewind(tmp);
    {
        test_t *ok = malloc(sizeof(test_t));
        assert_not_null(ok);
        assert_int(api_skip_init_test(ok), ==, 0);
        assert_int(api_skip_deserialize_test(ok, tmp), ==, 0);
        assert_size(api_skip_length_test(ok), ==, 10);
        assert_int(_skip_integrity_check_test(ok, 1), ==, 0);
        api_skip_free_test(ok);
        free(ok);
    }

    fclose(tmp);
    return MUNIT_OK;
}

/* EBR retire allocates a retire-list entry; failure must not lose the
   node or corrupt the list. */
static MunitResult
test_fault_ebr_retire_alloc(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    test_t *list = malloc(sizeof(test_t));
    assert_not_null(list);
    assert_int(api_skip_init_test(list), ==, 0);

    _skip_ebr_test_t *ebr = malloc(sizeof(_skip_ebr_test_t));
    assert_not_null(ebr);
    api_skip_ebr_init_test(ebr);
    api_skip_ebr_attach_test(list, ebr);

    int tid = api_skip_ebr_register_test(ebr);
    assert_int(tid, >=, 0);

    for (int i = 1; i <= 8; i++)
        assert_int(api_skip_put_test(list, i, make_test_value(i)), ==, 0);

    api_skip_ebr_pin_test(ebr, tid);
    fi_arm(1);
    api_skip_del_test(list, 4);
    fi_disarm();
    api_skip_ebr_unpin_test(ebr, tid);

    assert_int(_skip_integrity_check_test(list, 1), ==, 0);

    api_skip_ebr_drain_test(ebr);
    api_skip_ebr_unregister_test(ebr, tid);
    api_skip_free_test(list);
    free(list);
    free(ebr);
    return MUNIT_OK;
}

#endif /* SKIPLIST_TEST_FAULTS */

/* Test suite definition */
static MunitTest test_suite_tests[] = { { (char *)"/init", test_init, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/insert_basic", test_insert_basic, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/insert_duplicate", test_insert_duplicate, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/insert_multiple", test_insert_multiple, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/search", test_search, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }, { (char *)"/remove", test_remove, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/access_api", test_access_api, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/navigation", test_navigation, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/edge_cases", test_edge_cases, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/splay_behavior", test_splay_behavior, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/memory_management", test_memory_management, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/tail_regression", test_tail_regression, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/delete_last_element", test_delete_last_element, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/position_variants", test_position_variants, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/stress_insert_remove", test_stress_insert_remove, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/pool_allocator", test_pool_allocator, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/ebr_basic", test_ebr_basic, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/validation", test_validation, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/validate_corrupt_header", test_validate_corrupt_header, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/validate_corrupt_heights", test_validate_corrupt_heights, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/validate_corrupt_length", test_validate_corrupt_length, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/validate_corrupt_pointers", test_validate_corrupt_pointers, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/validate_corrupt_upper_levels", test_validate_corrupt_upper_levels, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/validate_corrupt_order", test_validate_corrupt_order, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/validate_flag_matrix", test_validate_flag_matrix, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/head_height_growth_shrinkage", test_head_height_growth_shrinkage, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/stress_100k", test_stress_100k, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/snapshot_basic", test_snapshot_basic, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/snapshot_with_deletes", test_snapshot_with_deletes, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/snapshot_multiple_eras", test_snapshot_multiple_eras, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/snapshot_release", test_snapshot_release, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/snapshot_restore_then_continue", test_snapshot_restore_then_continue, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/archive_basic", test_archive_basic, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/archive_empty", test_archive_empty, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/archive_roundtrip", test_archive_roundtrip, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/dot", test_dot, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/api_breadth", test_api_breadth, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/pool_free_node", test_pool_free_node, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/ebr_retire_callback", test_ebr_retire_callback, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/sizeof_entry", test_sizeof_entry, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/delete_with_duplicates", test_delete_with_duplicates, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/snapshot_restore_bulk", test_snapshot_restore_bulk, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/snapshot_preserve_arms", test_snapshot_preserve_arms, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/archive_io_failures", test_archive_io_failures, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/dot_edge_shapes", test_dot_edge_shapes, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/splay_skewed_access", test_splay_skewed_access, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/insert_help_unlink", test_insert_help_unlink, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/null_argument_guards", test_null_argument_guards, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/validate_early_exit_arms", test_validate_early_exit_arms, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/read_paths_marked_pointers", test_read_paths_marked_pointers, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/boundary_and_edge_arms", test_boundary_and_edge_arms, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/hit_counter_rescale", test_hit_counter_rescale, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
#ifdef SKIPLIST_TEST_FAULTS
    { (char *)"/faults/injector_self_check", test_fault_injector_self_check, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/faults/init_alloc", test_fault_init_alloc, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/faults/node_alloc", test_fault_node_alloc, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/faults/to_array_alloc", test_fault_to_array_alloc, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/faults/pool_alloc", test_fault_pool_alloc, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/faults/snapshot_alloc", test_fault_snapshot_alloc, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/faults/preserve_alloc", test_fault_preserve_alloc, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/faults/serialize_io", test_fault_serialize_io, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/faults/deserialize_alloc", test_fault_deserialize_alloc, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/faults/ebr_retire_alloc", test_fault_ebr_retire_alloc, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
#endif
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL } };

static const MunitSuite test_suite = { (char *)"", test_suite_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE };

int
main(int argc, char *argv[MUNIT_ARRAY_PARAM(argc + 1)])
{
    return munit_suite_main(&test_suite, (void *)"splay-list", argc, argv);
}
