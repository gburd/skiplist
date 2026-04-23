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
    assert_int(rc, ==, -1);

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
    assert_int(rc, ==, 0); /* Should not error */
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
    assert_int(api_skip_del_test(list, 1), ==, 0);

    /* Test NULL parameter handling */
    assert_int(api_skip_insert_test(NULL, NULL), ==, ENOENT);
    assert_int(api_skip_remove_node_test(NULL, NULL), ==, -1);

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
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL } };

static const MunitSuite test_suite = { (char *)"", test_suite_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE };

int
main(int argc, char *argv[MUNIT_ARRAY_PARAM(argc + 1)])
{
    return munit_suite_main(&test_suite, (void *)"splay-list", argc, argv);
}
