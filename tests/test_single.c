/*
 * tests/test_single.c -- exercise the SKIPLIST_SINGLE_THREADED build path.
 *
 * Defining SKIPLIST_SINGLE_THREADED before including sl.h replaces all
 * <stdatomic.h> primitives with plain operations and disables EBR.  The
 * resulting code paths are physically different (different macro
 * expansions) so they need their own compile + run.
 *
 * This translation unit declares a small skiplist with the access API,
 * snapshots, pool, archive, validate, and DOT layers all enabled, then
 * runs a representative subset of the multi-threaded test contract: it
 * verifies the structure stays correct, reachable, ordered, and freeable
 * under single-threaded mutations.
 */
#define _POSIX_C_SOURCE 200809L
#define SKIPLIST_SINGLE_THREADED
#define MUNIT_ENABLE_ASSERT_ALIASES

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "munit.h"
#include "sl.h"

struct st_node {
    int key;
    char *value;
    SKIPLIST_ENTRY(st) entries;
};

SKIPLIST_DECL(
    st, st_, entries,
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
        rc = 0;
    },
    /* archive */
    {
        dest->key = src->key;
        dest->value = src->value ? strdup(src->value) : NULL;
        if (src->value && dest->value == NULL)
            rc = ENOMEM;
    },
    /* sizeof */
    {
        bytes = sizeof(*node);
        if (node->value)
            bytes += strlen(node->value) + 1;
    })

SKIPLIST_DECL_ACCESS(
    st, st_,
    key, int,
    value, char *,
    /* qblk -- in scope: 'query' (local query node) and 'key' (the key arg) */
    { query.key = key; },
    /* rblk -- in scope: 'node' (matched node) */
    { return node->value; })

SKIPLIST_DECL_VALIDATE(st, st_, entries)
SKIPLIST_DECL_SNAPSHOTS(st, st_, entries)
SKIPLIST_DECL_POOL(st, st_, entries, 256)
SKIPLIST_DECL_ARCHIVE(
    st, st_, entries,
    /* write_entry_blk */
    {
        memcpy(buf, &node->key, sizeof(node->key));
        uint64_t off = sizeof(node->key);
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
        memcpy(&node->key, buf, sizeof(node->key));
        uint64_t off = sizeof(node->key);
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

SKIPLIST_DECL_DOT(st, st_, entries)

static char *
mk(int k)
{
    char *buf = malloc(16);
    snprintf(buf, 16, "v_%d", k);
    return buf;
}

static MunitResult
test_st_basic(const MunitParameter p[], void *d)
{
    (void)p;
    (void)d;
    st_t *l = malloc(sizeof(*l));
    st_skip_init_st(l);

    for (int i = 1; i <= 50; i++)
        assert_int(st_skip_put_st(l, i, mk(i)), ==, 0);

    assert_size(st_skip_length_st(l), ==, 50);

    for (int i = 1; i <= 50; i++)
        assert_true(st_skip_contains_st(l, i));

    char *v = st_skip_get_st(l, 25);
    assert_not_null(v);
    assert_string_equal(v, "v_25");

    /* Ordered iteration. */
    st_node_t *cur;
    size_t idx;
    int prev = 0;
    SKIPLIST_FOREACH_H2T(st, st_, entries, l, cur, idx)
    {
        assert_int(cur->key, >, prev);
        prev = cur->key;
    }
    (void)idx;
    assert_int(prev, ==, 50);

    /* Validation with all flag combinations. */
    assert_int(_skip_integrity_check_st(l, 1), ==, 0);
    assert_int(_skip_integrity_check_st(l, 0), ==, 0);
    assert_int(_skip_integrity_check_st(l, 3), ==, 0);

    /* Removal preserves invariants. */
    for (int i = 1; i <= 50; i += 2)
        assert_int(st_skip_del_st(l, i), ==, 0);
    assert_size(st_skip_length_st(l), ==, 25);
    assert_int(_skip_integrity_check_st(l, 0), ==, 0);

    st_skip_free_st(l);
    free(l);
    return MUNIT_OK;
}

static MunitResult
test_st_snapshot(const MunitParameter p[], void *d)
{
    (void)p;
    (void)d;
    st_t *l = malloc(sizeof(*l));
    st_skip_init_st(l);
    st_skip_snapshots_init_st(l);

    for (int i = 1; i <= 10; i++)
        assert_int(st_skip_put_st(l, i, mk(i)), ==, 0);

    uint64_t era = st_skip_snapshot_st(l);

    /* Mutate after snapshot. */
    for (int i = 11; i <= 20; i++)
        assert_int(st_skip_put_st(l, i, mk(i)), ==, 0);
    assert_size(st_skip_length_st(l), ==, 20);
    for (int i = 1; i <= 10; i++)
        assert_int(st_skip_del_st(l, i), ==, 0);
    assert_size(st_skip_length_st(l), ==, 10);

    /* Restore brings us back: list mutates in place to era's content. */
    st_t *snap = st_skip_restore_snapshot_st(l, era);
    assert_not_null(snap);
    for (int i = 1; i <= 10; i++)
        assert_true(st_skip_contains_st(l, i));

    st_skip_release_snapshots_st(l);
    st_skip_free_st(l);
    free(l);
    return MUNIT_OK;
}

static MunitResult
test_st_archive(const MunitParameter p[], void *d)
{
    (void)p;
    (void)d;
    st_t *l = malloc(sizeof(*l));
    st_skip_init_st(l);
    for (int i = 1; i <= 25; i++)
        st_skip_put_st(l, i, mk(i));

    FILE *fp = tmpfile();
    assert_int(st_skip_serialize_st(l, fp), ==, 0);
    rewind(fp);

    st_t *l2 = malloc(sizeof(*l2));
    st_skip_init_st(l2);
    assert_int(st_skip_deserialize_st(l2, fp), ==, 0);
    fclose(fp);

    assert_size(st_skip_length_st(l2), ==, 25);
    for (int i = 1; i <= 25; i++) {
        char *v = st_skip_get_st(l2, i);
        assert_not_null(v);
        char want[16];
        snprintf(want, 16, "v_%d", i);
        assert_string_equal(v, want);
    }

    st_skip_free_st(l);
    free(l);
    st_skip_free_st(l2);
    free(l2);
    return MUNIT_OK;
}

static void
sprintf_st_node(st_node_t *n, char *buf)
{
    snprintf(buf, 2048, "%d=%s", n->key, n->value ? n->value : "");
}

static MunitResult
test_st_dot(const MunitParameter p[], void *d)
{
    (void)p;
    (void)d;
    st_t *l = malloc(sizeof(*l));
    st_skip_init_st(l);
    for (int i = 1; i <= 10; i++)
        st_skip_put_st(l, i, mk(i));

    FILE *fp = tmpfile();
    size_t nsg = st_skip_dot_st(fp, l, 0, (char *)"st", sprintf_st_node);
    assert_size(nsg, >, 0);
    st_skip_dot_end_st(fp, nsg);

    rewind(fp);
    char buf[4096];
    size_t n = fread(buf, 1, sizeof(buf) - 1, fp);
    buf[n] = '\0';
    fclose(fp);
    assert_true(strstr(buf, "graph") != NULL);
    assert_true(strstr(buf, "5=v_5") != NULL);

    st_skip_free_st(l);
    free(l);
    return MUNIT_OK;
}

static MunitResult
test_st_pool(const MunitParameter p[], void *d)
{
    (void)p;
    (void)d;
    st_t *l = malloc(sizeof(*l));
    st_skip_init_st(l);

    /* Pool acquire/release exercise. */
    _skip_pool_st_t pool;
    assert_int(st_skip_pool_init_st(&pool, 64), ==, 0);
    for (int i = 0; i < 10; i++) {
        st_node_t *n = st_skip_pool_alloc_st(&pool);
        assert_not_null(n);
        st_skip_pool_free_st(&pool, n);
    }

    /* Wrapper variants. */
    st_node_t *node = NULL;
    assert_int(st_skip_pool_alloc_node_st(&pool, &node), ==, 0);
    assert_not_null(node);
    /* pool_is_from: confirm node belongs to pool. */
    assert_true(st_skip_pool_is_from_st(&pool, node));
    st_skip_pool_free_node_st(&pool, l, node);

    st_skip_pool_destroy_st(&pool);
    st_skip_free_st(l);
    free(l);
    return MUNIT_OK;
}

/* Exercise the navigation, dup, set, update, position, to_array, and
 * pos APIs in a single sweep.  These functions exist for every
 * SKIPLIST_DECL'd type but the basic test only invokes a small subset. */
static MunitResult
test_st_api_breadth(const MunitParameter p[], void *d)
{
    (void)p;
    (void)d;
    st_t *l = malloc(sizeof(*l));
    st_skip_init_st(l);

    /* Insert distinct keys plus duplicates of one. */
    for (int i = 1; i <= 10; i++)
        st_skip_put_st(l, i, mk(i));
    assert_int(st_skip_dup_st(l, 5, mk(50)), ==, 0);
    assert_int(st_skip_dup_st(l, 5, mk(500)), ==, 0);
    assert_int(st_skip_set_st(l, 1, mk(11)), ==, 0);
    assert_string_equal(st_skip_get_st(l, 1), "v_11");

    /* update(): walk via query node. */
    st_node_t q;
    memset(&q, 0, sizeof(q));
    q.key = 2;
    char *new_val = mk(22);
    assert_int(st_skip_update_st(l, &q, new_val), ==, 0);

    /* head/tail/next/prev navigation. */
    st_node_t *first = st_skip_head_st(l);
    assert_not_null(first);
    st_node_t *next = st_skip_next_node_st(l, first);
    assert_not_null(next);
    st_node_t *last = st_skip_tail_st(l);
    assert_not_null(last);
    st_node_t *prev = st_skip_prev_node_st(l, last);
    assert_not_null(prev);
    /* prev_validated should agree on a non-corrupted list. */
    st_node_t *pv = st_skip_prev_validated_st(l, last);
    /* pv is allowed to be NULL only when validation falls through. */
    (void)pv;

    /* position()/pos() variants. */
    st_node_t pq;
    memset(&pq, 0, sizeof(pq));
    pq.key = 5;
    assert_not_null(st_skip_position_st(l, SKIP_EQ, &pq));
    assert_not_null(st_skip_position_lt_st(l, &pq));
    assert_not_null(st_skip_position_lte_st(l, &pq));
    assert_not_null(st_skip_position_gt_st(l, &pq));
    assert_not_null(st_skip_position_gte_st(l, &pq));

    assert_not_null(st_skip_pos_st(l, SKIP_EQ, 5));
    assert_not_null(st_skip_pos_st(l, SKIP_LT, 5));
    assert_not_null(st_skip_pos_st(l, SKIP_LTE, 5));
    assert_not_null(st_skip_pos_st(l, SKIP_GT, 5));
    assert_not_null(st_skip_pos_st(l, SKIP_GTE, 5));

    /* to_array. */
    st_node_t **arr = st_skip_to_array_st(l);
    assert_not_null(arr);
    size_t arr_len = (size_t)(uintptr_t)arr[-1];
    assert_size(arr_len, ==, st_skip_length_st(l));
    free(arr - 1);

    /* insert_dup(): allocate a node and use the lower-level insert_dup. */
    st_node_t *dup_node;
    assert_int(st_skip_alloc_node_st(&dup_node), ==, 0);
    dup_node->key = 5;
    dup_node->value = mk(5000);
    assert_int(st_skip_insert_dup_st(l, dup_node), ==, 0);

    /* sizeof_entry: invoke the user's sizeof block via the function-pointer
     * dispatch table.  Useful for archive sizing computations. */
    {
        st_node_t sample;
        memset(&sample, 0, sizeof(sample));
        sample.key = 1;
        sample.value = mk(1);
        size_t sz = l->slh_fns.sizeof_entry(&sample);
        assert_size(sz, >, 0);
        free(sample.value);
    }

    /* Validation failure path: NULL list yields a non-zero error count. */
    assert_int(_skip_integrity_check_st(NULL, 0), >, 0);

    st_skip_free_st(l);
    free(l);
    return MUNIT_OK;
}

static MunitTest tests[] = {
    { (char *)"/single/basic", test_st_basic, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/single/snapshot", test_st_snapshot, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/single/archive", test_st_archive, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/single/dot", test_st_dot, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/single/pool", test_st_pool, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/single/api_breadth", test_st_api_breadth, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
};

static const MunitSuite suite = { (char *)"", tests, NULL, 1, MUNIT_SUITE_OPTION_NONE };

int
main(int argc, char *argv[MUNIT_ARRAY_PARAM(argc + 1)])
{
    return munit_suite_main(&suite, (void *)"single", argc, argv);
}
