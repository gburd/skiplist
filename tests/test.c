#define MUNIT_ENABLE_ASSERT_ALIASES
#pragma GCC push_options
#pragma GCC optimize("O0")

#include <string.h>

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
        uint64_t off = 0;
        memcpy(&node->key, buf + off, sizeof(node->key));
        off += sizeof(node->key);
        uint32_t slen;
        memcpy(&slen, buf + off, sizeof(slen));
        off += sizeof(slen);
        if (slen > 0) {
            node->value = (char *)malloc(slen + 1);
            memcpy(node->value, buf + off, slen);
            node->value[slen] = '\0';
        } else {
            node->value = NULL;
        }
    })

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
    /* Without splay, heights don't change — just suppress warnings */
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
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL } };

static const MunitSuite test_suite = { (char *)"", test_suite_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE };

int
main(int argc, char *argv[MUNIT_ARRAY_PARAM(argc + 1)])
{
    return munit_suite_main(&test_suite, (void *)"splay-list", argc, argv);
}
