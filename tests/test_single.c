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
#include <unistd.h> /* dup/dup2/close/fileno, for silencing validator output */

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
    st, st_, key, int, value, char *,
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

/* ------------------------------------------------------------------
 * Error and edge paths.
 *
 * The tests above drive the happy path, which leaves every NULL guard,
 * every I/O failure return, and every empty-list early exit in the
 * single-threaded expansion unexecuted.  These cover them.
 * ------------------------------------------------------------------ */

/* Every public entry point given a NULL list or NULL argument. */
static MunitResult
test_st_null_guards(const MunitParameter p[], void *d)
{
    (void)p;
    (void)d;

    /* Archive rejects NULL list and NULL stream rather than crashing. */
    FILE *fp = tmpfile();
    assert_not_null(fp);
    assert_int(st_skip_serialize_st(NULL, fp), ==, EINVAL);
    assert_int(st_skip_deserialize_st(NULL, fp), ==, EINVAL);

    st_t *l = malloc(sizeof(*l));
    st_skip_init_st(l);
    assert_int(st_skip_serialize_st(l, NULL), ==, EINVAL);
    assert_int(st_skip_deserialize_st(l, NULL), ==, EINVAL);
    fclose(fp);

    /* Navigation on a NULL list, and on an empty list. */
    assert_null(st_skip_tail_st(NULL));
    assert_null(st_skip_next_node_st(NULL, NULL));
    assert_null(st_skip_prev_node_st(NULL, NULL));
    assert_null(st_skip_head_st(l));
    assert_null(st_skip_tail_st(l));

    /* next/prev given a NULL node on a valid list. */
    assert_null(st_skip_next_node_st(l, NULL));
    assert_null(st_skip_prev_node_st(l, NULL));

    /* Lookups against an empty list. */
    assert_false(st_skip_contains_st(l, 1));
    assert_null(st_skip_get_st(l, 1));
    assert_int(st_skip_del_st(l, 1), !=, 0);

    /* Position queries against an empty list find nothing. */
    st_node_t q;
    memset(&q, 0, sizeof(q));
    q.key = 1;
    assert_null(st_skip_position_st(l, SKIP_EQ, &q));
    assert_null(st_skip_position_lt_st(l, &q));
    assert_null(st_skip_position_gt_st(l, &q));

    st_skip_free_st(l);
    free(l);
    return MUNIT_OK;
}

/* Deserialize must reject malformed input rather than trusting it. */
static MunitResult
test_st_deserialize_robustness(const MunitParameter p[], void *d)
{
    (void)p;
    (void)d;

    /* Truncated / empty stream. */
    {
        FILE *fp = tmpfile();
        st_t *l = malloc(sizeof(*l));
        st_skip_init_st(l);
        assert_int(st_skip_deserialize_st(l, fp), ==, EIO);
        fclose(fp);
        st_skip_free_st(l);
        free(l);
    }

    /* Wrong magic. */
    {
        FILE *fp = tmpfile();
        fwrite("XXXX", 1, 4, fp);
        rewind(fp);
        st_t *l = malloc(sizeof(*l));
        st_skip_init_st(l);
        assert_int(st_skip_deserialize_st(l, fp), ==, EINVAL);
        fclose(fp);
        st_skip_free_st(l);
        free(l);
    }

    /* Right magic, unsupported version. */
    {
        FILE *fp = tmpfile();
        uint8_t ver[4] = { 99, 0, 0, 0 };
        fwrite("SKPL", 1, 4, fp);
        fwrite(ver, 1, 4, fp);
        rewind(fp);
        st_t *l = malloc(sizeof(*l));
        st_skip_init_st(l);
        assert_int(st_skip_deserialize_st(l, fp), ==, EINVAL);
        fclose(fp);
        st_skip_free_st(l);
        free(l);
    }

    /* Valid header, then truncated before the node count. */
    {
        FILE *fp = tmpfile();
        uint8_t ver[4] = { 1, 0, 0, 0 };
        fwrite("SKPL", 1, 4, fp);
        fwrite(ver, 1, 4, fp);
        rewind(fp);
        st_t *l = malloc(sizeof(*l));
        st_skip_init_st(l);
        assert_int(st_skip_deserialize_st(l, fp), ==, EIO);
        fclose(fp);
        st_skip_free_st(l);
        free(l);
    }

    /* Valid header and count, but the record length is absurd: must be
       rejected by the SKIPLIST_ARCHIVE_MAX_RECORD bound, not attempted. */
    {
        FILE *fp = tmpfile();
        uint8_t ver[4] = { 1, 0, 0, 0 };
        uint8_t cnt[8] = { 1, 0, 0, 0, 0, 0, 0, 0 };
        uint8_t huge[8];
        uint64_t big = SKIPLIST_ARCHIVE_MAX_RECORD + 1;
        for (int i = 0; i < 8; i++)
            huge[i] = (uint8_t)((big >> (8 * i)) & 0xff);
        fwrite("SKPL", 1, 4, fp);
        fwrite(ver, 1, 4, fp);
        fwrite(cnt, 1, 8, fp);
        fwrite(huge, 1, 8, fp);
        rewind(fp);
        st_t *l = malloc(sizeof(*l));
        st_skip_init_st(l);
        assert_int(st_skip_deserialize_st(l, fp), ==, EINVAL);
        fclose(fp);
        st_skip_free_st(l);
        free(l);
    }

    /* Valid header and count, record length truncated mid-payload. */
    {
        FILE *fp = tmpfile();
        uint8_t ver[4] = { 1, 0, 0, 0 };
        uint8_t cnt[8] = { 1, 0, 0, 0, 0, 0, 0, 0 };
        uint8_t len[8] = { 64, 0, 0, 0, 0, 0, 0, 0 };
        fwrite("SKPL", 1, 4, fp);
        fwrite(ver, 1, 4, fp);
        fwrite(cnt, 1, 8, fp);
        fwrite(len, 1, 8, fp);
        fwrite("short", 1, 5, fp); /* far fewer than 64 bytes */
        rewind(fp);
        st_t *l = malloc(sizeof(*l));
        st_skip_init_st(l);
        assert_int(st_skip_deserialize_st(l, fp), ==, EIO);
        fclose(fp);
        st_skip_free_st(l);
        free(l);
    }

    /* An empty but well-formed archive round-trips to an empty list. */
    {
        st_t *src = malloc(sizeof(*src));
        st_skip_init_st(src);
        FILE *fp = tmpfile();
        assert_int(st_skip_serialize_st(src, fp), ==, 0);
        rewind(fp);
        st_t *dst = malloc(sizeof(*dst));
        st_skip_init_st(dst);
        assert_int(st_skip_deserialize_st(dst, fp), ==, 0);
        assert_size(st_skip_length_st(dst), ==, 0);
        fclose(fp);
        st_skip_free_st(src);
        free(src);
        st_skip_free_st(dst);
        free(dst);
    }

    return MUNIT_OK;
}

/* Validator error paths in the single-threaded expansion.  Same approach
   as tests/test.c: corrupt one invariant, assert it is reported, restore. */
static MunitResult
test_st_validate_corruption(const MunitParameter p[], void *d)
{
    (void)p;
    (void)d;

    st_t *l = malloc(sizeof(*l));
    st_skip_init_st(l);
    /* Enough nodes for the head tower to reach several levels, so the
       upper-level checks below have something to corrupt. */
    for (int i = 1; i <= 400; i++)
        st_skip_put_st(l, i, mk(i));

    /* The validator reports to stderr by design; keep the run readable. */
    fflush(stderr);
    int saved = dup(fileno(stderr));
    if (freopen("/dev/null", "w", stderr) == NULL) {
        /* Non-fatal; assertions below still hold. */
    }

    for (int flags = 0; flags <= 3; flags++)
        assert_int(_skip_integrity_check_st(l, flags), ==, 0);

    /* NULL sentinels and NULL dispatch entries. */
    st_node_t *save_head = l->slh_head;
    l->slh_head = NULL;
    assert_int(_skip_integrity_check_st(l, 1), >, 0);
    l->slh_head = save_head;

    st_node_t *save_tail = l->slh_tail;
    l->slh_tail = NULL;
    assert_int(_skip_integrity_check_st(l, 1), >, 0);
    l->slh_tail = save_tail;

    void (*save_free)(st_node_t *) = l->slh_fns.free_entry;
    l->slh_fns.free_entry = NULL;
    assert_int(_skip_integrity_check_st(l, 1), >, 0);
    l->slh_fns.free_entry = save_free;

    int (*save_cmp)(st_t *, st_node_t *, st_node_t *, void *) = l->slh_fns.compare_entries;
    l->slh_fns.compare_entries = NULL;
    assert_int(_skip_integrity_check_st(l, 1), >, 0);
    l->slh_fns.compare_entries = save_cmp;

    /* Heights: at the level-array bound and mismatched head/tail. */
    size_t save_hh = l->slh_head->entries.sle_height;
    size_t save_th = l->slh_tail->entries.sle_height;
    l->slh_head->entries.sle_height = (size_t)SKIPLIST_MAX_HEIGHT;
    assert_int(_skip_integrity_check_st(l, 1), >, 0);
    assert_int(_skip_integrity_check_st(l, 3), >, 0);
    l->slh_head->entries.sle_height = save_hh;

    l->slh_tail->entries.sle_height = save_th ? save_th - 1 : 1;
    assert_int(_skip_integrity_check_st(l, 1), >, 0);
    l->slh_tail->entries.sle_height = save_th;

    /* Length counter disagreeing with the walked count. */
    size_t save_len = l->slh_length;
    l->slh_length = save_len + 7;
    assert_int(_skip_integrity_check_st(l, 1), >, 0);
    l->slh_length = save_len;

    /* Sort order violated behind the validator's back. */
    st_node_t *a = st_skip_head_st(l);
    assert_not_null(a);
    st_node_t *b = st_skip_next_node_st(l, a);
    assert_not_null(b);
    st_node_t *c = st_skip_next_node_st(l, b);
    assert_not_null(c);
    int save_key = b->key;
    b->key = c->key + 100;
    assert_int(_skip_integrity_check_st(l, 1), >, 0);
    b->key = a->key - 100;
    assert_int(_skip_integrity_check_st(l, 1), >, 0);
    b->key = save_key;

    /* A NULL level-0 next makes the node unwalkable: must be reported,
       not dereferenced, and must terminate. */
    st_node_t *save_next = a->entries.sle_levels[0].next;
    a->entries.sle_levels[0].next = NULL;
    assert_int(_skip_integrity_check_st(l, 1), >, 0);
    a->entries.sle_levels[0].next = save_next;

    /* A cycle in the level-0 chain must be detected, not spun on. */
    st_node_t *save_bnext = b->entries.sle_levels[0].next;
    b->entries.sle_levels[0].next = a;
    assert_int(_skip_integrity_check_st(l, 0), >, 0);
    b->entries.sle_levels[0].next = save_bnext;

    /* Upper-level corruption.  The validator checks each level twice --
       walking up from level 0 to the tail, then again for every level
       above -- and level-0-only corruption never reaches the second loop.
       Upper levels are also safe to corrupt without making the list
       unwalkable, since iteration only follows level 0. */
    {
        size_t head_h = l->slh_head->entries.sle_height;
        for (size_t lvl = 1; lvl <= head_h; lvl++) {
            st_node_t *save = l->slh_head->entries.sle_levels[lvl].next;
            l->slh_head->entries.sle_levels[lvl].next = NULL;
            assert_int(_skip_integrity_check_st(l, 1), >, 0);
            assert_int(_skip_integrity_check_st(l, 3), >, 0);
            l->slh_head->entries.sle_levels[lvl].next = _SKIP_MARK(save);
            assert_int(_skip_integrity_check_st(l, 0), >, 0);
            assert_int(_skip_integrity_check_st(l, 2), >, 0);
            l->slh_head->entries.sle_levels[lvl].next = save;
            assert_int(_skip_integrity_check_st(l, 1), ==, 0);
        }
    }

    /* Clean again after every restore. */
    for (int flags = 0; flags <= 3; flags++)
        assert_int(_skip_integrity_check_st(l, flags), ==, 0);

    fflush(stderr);
    dup2(saved, fileno(stderr));
    close(saved);

    st_skip_free_st(l);
    free(l);
    return MUNIT_OK;
}

/* The single-threaded validator expansion has two gaps the existing
 * corruption test leaves open.
 *
 * First, three dispatch-table guards: /single/validate_corruption nulls
 * free_entry and compare_entries but never update_entry, archive_entry or
 * sizeof_entry, so those early returns never run in this build.
 *
 * Second, the `flags & 2` early-exit arms.  The existing test passes
 * flags=0/1/3 but only ever asserts "> 0", which cannot distinguish
 * stop-at-first from tally-everything.  Asserting == 1 under flags=3 is
 * the actual contract and is what drives the early-return arms.
 *
 * Fixture stays at 64 nodes: the check is an O(n*levels) walk called many
 * times here. */
static MunitResult
test_st_validate_early_exit_arms(const MunitParameter p[], void *d)
{
    (void)p;
    (void)d;

    st_t *l = malloc(sizeof(*l));
    st_skip_init_st(l);
    for (int i = 1; i <= 64; i++)
        st_skip_put_st(l, i, mk(i));

    fflush(stderr);
    int saved = dup(fileno(stderr));
    if (freopen("/dev/null", "w", stderr) == NULL) {
        /* Non-fatal; the assertions below still hold. */
    }

    assert_int(_skip_integrity_check_st(l, 1), ==, 0);
    assert_int(_skip_integrity_check_st(l, 3), ==, 0);

    /* ---- dispatch-table guards this build never exercised ---- */
    int (*save_update)(st_node_t *, void *) = l->slh_fns.update_entry;
    l->slh_fns.update_entry = NULL;
    assert_int(_skip_integrity_check_st(l, 1), >, 0);
    l->slh_fns.update_entry = save_update;

    int (*save_archive)(st_node_t *, const st_node_t *) = l->slh_fns.archive_entry;
    l->slh_fns.archive_entry = NULL;
    assert_int(_skip_integrity_check_st(l, 1), >, 0);
    l->slh_fns.archive_entry = save_archive;

    size_t (*save_sizeof)(st_node_t *) = l->slh_fns.sizeof_entry;
    l->slh_fns.sizeof_entry = NULL;
    assert_int(_skip_integrity_check_st(l, 1), >, 0);
    l->slh_fns.sizeof_entry = save_sizeof;

    assert_int(_skip_integrity_check_st(l, 1), ==, 0);

    /* ---- early-exit arms: exactly one error reported ---- */
#define ST_ASSERT_EARLY_ONE()                                   \
    do {                                                        \
        assert_int(_skip_integrity_check_st(l, 1), >, 0);         \
        assert_int(_skip_integrity_check_st(l, 3), ==, 1);        \
    } while (0)

    size_t save_hh = l->slh_head->entries.sle_height;
    size_t save_th = l->slh_tail->entries.sle_height;

    l->slh_head->entries.sle_height = (size_t)SKIPLIST_MAX_HEIGHT;
    ST_ASSERT_EARLY_ONE();
    l->slh_head->entries.sle_height = save_hh;

    l->slh_tail->entries.sle_height = (size_t)SKIPLIST_MAX_HEIGHT;
    ST_ASSERT_EARLY_ONE();
    l->slh_tail->entries.sle_height = save_th;

    l->slh_tail->entries.sle_height = save_th ? save_th - 1 : 1;
    ST_ASSERT_EARLY_ONE();
    l->slh_tail->entries.sle_height = save_th;

    size_t save_len = l->slh_length;
    l->slh_length = save_len + 9;
    ST_ASSERT_EARLY_ONE();
    l->slh_length = save_len;

    /* per-node arms */
    st_node_t *n1 = st_skip_head_st(l);
    assert_not_null(n1);
    st_node_t *n2 = st_skip_next_node_st(l, n1);
    assert_not_null(n2);
    st_node_t *n3 = st_skip_next_node_st(l, n2);
    assert_not_null(n3);

    size_t save_n2h = n2->entries.sle_height;
    n2->entries.sle_height = save_hh + 1;
    ST_ASSERT_EARLY_ONE();
    n2->entries.sle_height = save_n2h;

    n2->entries.sle_height = (size_t)SKIPLIST_MAX_HEIGHT;
    assert_int(_skip_integrity_check_st(l, 1), >, 0);
    assert_int(_skip_integrity_check_st(l, 3), >, 0);
    n2->entries.sle_height = save_n2h;

    st_node_t *save_prev = n2->entries.sle_prev;
    n2->entries.sle_prev = NULL;
    assert_int(_skip_integrity_check_st(l, 1), >, 0);
    assert_int(_skip_integrity_check_st(l, 3), >, 0);
    n2->entries.sle_prev = save_prev;

    int save_key = n2->key;
    n2->key = n3->key + 100;
    ST_ASSERT_EARLY_ONE();
    n2->key = n1->key - 100;
    ST_ASSERT_EARLY_ONE();
    n2->key = save_key;

    /* upper-level arms, both report modes.  Level 0 is left alone: a fault
     * there makes the list unwalkable and returns unconditionally. */
    {
        size_t head_h = l->slh_head->entries.sle_height;
        for (size_t lvl = 1; lvl <= head_h; lvl++) {
            st_node_t *save = l->slh_head->entries.sle_levels[lvl].next;

            l->slh_head->entries.sle_levels[lvl].next = NULL;
            assert_int(_skip_integrity_check_st(l, 1), >, 0);
            assert_int(_skip_integrity_check_st(l, 3), >, 0);

            l->slh_head->entries.sle_levels[lvl].next = _SKIP_MARK(save);
            assert_int(_skip_integrity_check_st(l, 0), >, 0);
            assert_int(_skip_integrity_check_st(l, 2), >, 0);

            l->slh_head->entries.sle_levels[lvl].next = save;
            assert_int(_skip_integrity_check_st(l, 1), ==, 0);
            assert_int(_skip_integrity_check_st(l, 3), ==, 0);
        }
    }

    /* marked pointer on a live node, first per-node loop */
    {
        st_node_t *save_next = n2->entries.sle_levels[0].next;
        n2->entries.sle_levels[0].next = _SKIP_MARK(save_next);
        assert_int(_skip_integrity_check_st(l, 0), >, 0);
        assert_int(_skip_integrity_check_st(l, 2), >, 0);
        n2->entries.sle_levels[0].next = save_next;
    }

#undef ST_ASSERT_EARLY_ONE

    for (int flags = 0; flags <= 3; flags++)
        assert_int(_skip_integrity_check_st(l, flags), ==, 0);

    fflush(stderr);
    dup2(saved, fileno(stderr));
    close(saved);

    st_skip_free_st(l);
    free(l);
    return MUNIT_OK;
}

/* Pool exhaustion and the malloc fallback, plus repeated claim/release. */
static MunitResult
test_st_pool_exhaustion(const MunitParameter p[], void *d)
{
    (void)p;
    (void)d;

    _skip_pool_st_t pool;
    assert_int(st_skip_pool_init_st(&pool, 8), ==, 0);

    /* pool_free_node dispatches through slist->slh_fns.free_entry, so it
       needs a real list even when the node came from the pool. */
    st_t *l = malloc(sizeof(*l));
    st_skip_init_st(l);

    /* Claim every slot. */
    st_node_t *nodes[8];
    for (int i = 0; i < 8; i++) {
        assert_int(st_skip_pool_alloc_node_st(&pool, &nodes[i]), ==, 0);
        assert_not_null(nodes[i]);
        assert_true(st_skip_pool_is_from_st(&pool, nodes[i]));
        nodes[i]->key = i;
        nodes[i]->value = mk(i);
    }

    /* One past capacity reports ENOMEM so the caller can fall back. */
    st_node_t *overflow = NULL;
    assert_int(st_skip_pool_alloc_node_st(&pool, &overflow), ==, ENOMEM);

    /* A malloc'd node is correctly identified as not pool-owned, and
       pool_free_node routes it to plain free() instead of the pool. */
    st_node_t *heap = NULL;
    assert_int(st_skip_alloc_node_st(&heap), ==, 0);
    assert_false(st_skip_pool_is_from_st(&pool, heap));
    heap->key = 99;
    heap->value = mk(99);
    st_skip_pool_free_node_st(&pool, l, heap);

    /* Release and re-claim exercises slot reuse. */
    for (int i = 0; i < 8; i++)
        st_skip_pool_free_node_st(&pool, l, nodes[i]);
    for (int i = 0; i < 8; i++) {
        assert_int(st_skip_pool_alloc_node_st(&pool, &nodes[i]), ==, 0);
        nodes[i]->value = mk(i);
    }
    for (int i = 0; i < 8; i++)
        st_skip_pool_free_node_st(&pool, l, nodes[i]);

    st_skip_pool_destroy_st(&pool);
    st_skip_free_st(l);
    free(l);
    return MUNIT_OK;
}

/* Snapshot edges: no snapshot taken, restore to a stale era, release
   without restore, and repeated snapshot/restore cycles. */
static MunitResult
test_st_snapshot_edges(const MunitParameter p[], void *d)
{
    (void)p;
    (void)d;

    st_t *l = malloc(sizeof(*l));
    st_skip_init_st(l);
    st_skip_snapshots_init_st(l);

    /* Restoring era 0 (no snapshot) is a no-op, not a crash. */
    st_skip_restore_snapshot_st(l, 0);

    for (int i = 1; i <= 10; i++)
        st_skip_put_st(l, i, mk(i));

    size_t era = st_skip_snapshot_st(l);
    assert_size(era, >, 0);

    /* An era at or beyond the current one has nothing to roll back. */
    st_skip_restore_snapshot_st(l, era + 100);
    assert_int(_skip_integrity_check_st(l, 1), ==, 0);

    /* Mutate, then release the snapshot without restoring: the preserved
       list must be freed rather than leaked (LSan enforces this). */
    for (int i = 1; i <= 5; i++)
        st_skip_del_st(l, i);
    st_skip_release_snapshots_st(l);
    assert_int(_skip_integrity_check_st(l, 1), ==, 0);

    /* Repeated snapshot/restore cycles keep the structure valid. */
    for (int cycle = 0; cycle < 3; cycle++) {
        size_t e = st_skip_snapshot_st(l);
        for (int i = 20 + cycle * 10; i < 25 + cycle * 10; i++)
            st_skip_put_st(l, i, mk(i));
        st_skip_restore_snapshot_st(l, e);
        assert_int(_skip_integrity_check_st(l, 1), ==, 0);
    }

    st_skip_free_st(l);
    free(l);
    return MUNIT_OK;
}

/* Height growth and shrink: enough keys to push the head up several
   levels, then remove nearly all of them to drive it back down. */
static MunitResult
test_st_height_churn(const MunitParameter p[], void *d)
{
    (void)p;
    (void)d;

    st_t *l = malloc(sizeof(*l));
    st_skip_init_st(l);

    /* 400 keys is enough to drive the head up several levels.  Note the
       validator is a full O(n * levels) walk, so it is called a handful of
       times here rather than inside the mutation loops -- calling it per
       operation makes this test quadratic and it ran for minutes. */
    for (int i = 1; i <= 400; i++)
        assert_int(st_skip_put_st(l, i, mk(i)), ==, 0);
    assert_size(st_skip_length_st(l), ==, 400);
    assert_int(_skip_integrity_check_st(l, 0), ==, 0);
    assert_size(l->slh_head->entries.sle_height, >, 0);

    /* Duplicate keys alongside the distinct ones. */
    for (int i = 1; i <= 20; i++)
        assert_int(st_skip_dup_st(l, i, mk(i * 1000)), ==, 0);
    assert_int(_skip_integrity_check_st(l, 0), ==, 0);

    /* Remove in a non-sequential order to exercise unlink at many levels. */
    for (int i = 400; i >= 1; i -= 3)
        st_skip_del_st(l, i);
    assert_int(_skip_integrity_check_st(l, 0), ==, 0);

    /* Drain completely, driving the head height back down. */
    for (int i = 1; i <= 400; i++)
        st_skip_del_st(l, i);
    assert_int(_skip_integrity_check_st(l, 0), ==, 0);

    /* Reinsert into the drained list. */
    for (int i = 1; i <= 50; i++)
        st_skip_put_st(l, i, mk(i));
    assert_int(_skip_integrity_check_st(l, 0), ==, 0);

    st_skip_free_st(l);
    free(l);
    return MUNIT_OK;
}

/* Position operators against boundary keys: below the minimum, above the
   maximum, and on absent interior keys. */
static MunitResult
test_st_position_boundaries(const MunitParameter p[], void *d)
{
    (void)p;
    (void)d;

    st_t *l = malloc(sizeof(*l));
    st_skip_init_st(l);
    /* Even keys only, so odd keys are known-absent probes. */
    for (int i = 2; i <= 20; i += 2)
        st_skip_put_st(l, i, mk(i));

    /* Below the minimum: no LT/LTE match, GT/GTE find the minimum. */
    assert_null(st_skip_pos_st(l, SKIP_LT, 1));
    assert_null(st_skip_pos_st(l, SKIP_LTE, 1));
    assert_null(st_skip_pos_st(l, SKIP_EQ, 1));
    st_node_t *n = st_skip_pos_st(l, SKIP_GT, 1);
    assert_not_null(n);
    assert_int(n->key, ==, 2);
    n = st_skip_pos_st(l, SKIP_GTE, 1);
    assert_not_null(n);
    assert_int(n->key, ==, 2);

    /* Above the maximum: no GT/GTE match, LT/LTE find the maximum. */
    assert_null(st_skip_pos_st(l, SKIP_GT, 100));
    assert_null(st_skip_pos_st(l, SKIP_GTE, 100));
    assert_null(st_skip_pos_st(l, SKIP_EQ, 100));
    n = st_skip_pos_st(l, SKIP_LT, 100);
    assert_not_null(n);
    assert_int(n->key, ==, 20);
    n = st_skip_pos_st(l, SKIP_LTE, 100);
    assert_not_null(n);
    assert_int(n->key, ==, 20);

    /* An absent interior key straddles two present neighbours. */
    assert_null(st_skip_pos_st(l, SKIP_EQ, 11));
    n = st_skip_pos_st(l, SKIP_LT, 11);
    assert_not_null(n);
    assert_int(n->key, ==, 10);
    n = st_skip_pos_st(l, SKIP_GT, 11);
    assert_not_null(n);
    assert_int(n->key, ==, 12);
    n = st_skip_pos_st(l, SKIP_LTE, 11);
    assert_not_null(n);
    assert_int(n->key, ==, 10);
    n = st_skip_pos_st(l, SKIP_GTE, 11);
    assert_not_null(n);
    assert_int(n->key, ==, 12);

    /* Reverse iteration over the whole list. */
    st_node_t *cur;
    size_t idx;
    int prev = 1000;
    int seen = 0;
    SKIPLIST_FOREACH_T2H(st, st_, entries, l, cur, idx)
    {
        assert_int(cur->key, <, prev);
        prev = cur->key;
        seen++;
    }
    (void)idx;
    assert_int(seen, ==, 10);

    st_skip_free_st(l);
    free(l);
    return MUNIT_OK;
}

/* Regression: deleting a key that has duplicates used to loop forever.

   _skip_unlink_fully_ carries a `pred` hint down between levels.  It used
   to advance that hint onto any node comparing <= the target, including an
   equal-key duplicate sorted after the target at level 0.  Lower levels
   then began scanning past the node, so it was never unlinked, while the
   level-0 reachability check kept finding it -- an infinite loop inside
   remove().  Needs enough keys for the head to grow past level 0, which is
   why the small ad-hoc cases never caught it. */
static MunitResult
test_st_delete_with_duplicates(const MunitParameter p[], void *d)
{
    (void)p;
    (void)d;

    st_t *l = malloc(sizeof(*l));
    st_skip_init_st(l);

    for (int i = 1; i <= 200; i++)
        assert_int(st_skip_put_st(l, i, mk(i)), ==, 0);
    /* Duplicates on the low keys, which is where the carried hint lands. */
    for (int i = 1; i <= 20; i++)
        assert_int(st_skip_dup_st(l, i, mk(i * 1000)), ==, 0);
    assert_size(st_skip_length_st(l), ==, 220);
    assert_size(l->slh_head->entries.sle_height, >, 0);

    /* Descending stride deletion crosses the duplicated key range. */
    for (int i = 200; i >= 1; i -= 3)
        st_skip_del_st(l, i);
    assert_int(_skip_integrity_check_st(l, 0), ==, 0);

    /* Delete every remaining key, duplicates included. */
    for (int i = 1; i <= 200; i++)
        while (st_skip_del_st(l, i) == 0)
            ;
    assert_size(st_skip_length_st(l), ==, 0);
    assert_int(_skip_integrity_check_st(l, 0), ==, 0);

    st_skip_free_st(l);
    free(l);
    return MUNIT_OK;
}

/* prev_validated: the advisory sle_prev hint and its recovery path.
   The happy path is covered elsewhere; what is not is the branch taken
   when the hint is stale, which forces a forward re-scan from the head. */
static MunitResult
test_st_prev_validated_paths(const MunitParameter p[], void *d)
{
    (void)p;
    (void)d;

    st_t *l = malloc(sizeof(*l));
    st_skip_init_st(l);
    for (int i = 1; i <= 30; i++)
        st_skip_put_st(l, i, mk(i));

    /* NULL arguments and the head sentinel all return NULL. */
    assert_null(st_skip_prev_validated_st(NULL, NULL));
    assert_null(st_skip_prev_validated_st(l, NULL));
    assert_null(st_skip_prev_validated_st(NULL, st_skip_head_st(l)));

    st_node_t *first = st_skip_head_st(l);
    assert_not_null(first);
    /* The first node's predecessor is the head sentinel, reported as NULL. */
    assert_null(st_skip_prev_validated_st(l, first));

    st_node_t *second = st_skip_next_node_st(l, first);
    assert_not_null(second);
    /* A valid hint returns the real predecessor. */
    assert_ptr_equal(st_skip_prev_validated_st(l, second), first);

    /* Corrupt the hint so it no longer points at a node whose next[0] is
       `second`.  The function must fall back to a forward scan and still
       return the correct predecessor. */
    st_node_t *third = st_skip_next_node_st(l, second);
    assert_not_null(third);
    st_node_t *save_prev = second->entries.sle_prev;
    second->entries.sle_prev = third; /* deliberately wrong */
    assert_ptr_equal(st_skip_prev_validated_st(l, second), first);
    second->entries.sle_prev = save_prev;

    /* A hint of NULL also forces the scan. */
    second->entries.sle_prev = NULL;
    assert_ptr_equal(st_skip_prev_validated_st(l, second), first);
    second->entries.sle_prev = save_prev;

    /* A hint pointing at the tail is rejected and rescanned. */
    second->entries.sle_prev = l->slh_tail;
    assert_ptr_equal(st_skip_prev_validated_st(l, second), first);
    second->entries.sle_prev = save_prev;

    /* Walk the whole list backwards through prev_validated. */
    st_node_t *cur = st_skip_tail_st(l);
    int count = 0;
    while (cur != NULL) {
        count++;
        cur = st_skip_prev_validated_st(l, cur);
    }
    assert_int(count, ==, 30);

    assert_int(_skip_integrity_check_st(l, 1), ==, 0);
    st_skip_free_st(l);
    free(l);
    return MUNIT_OK;
}

/* Update on absent keys, duplicate handling, and to_array on an empty
   list -- small early-return arms the breadth test does not reach. */
static MunitResult
test_st_update_and_array_edges(const MunitParameter p[], void *d)
{
    (void)p;
    (void)d;

    st_t *l = malloc(sizeof(*l));
    st_skip_init_st(l);

    /* to_array on an empty list. */
    st_node_t **empty = st_skip_to_array_st(l);
    if (empty != NULL) {
        assert_size((size_t)(uintptr_t)empty[-1], ==, 0);
        free(empty - 1);
    }

    /* update() and set() against a key that does not exist. */
    st_node_t q;
    memset(&q, 0, sizeof(q));
    q.key = 999;
    char *v = mk(999);
    assert_int(st_skip_update_st(l, &q, v), !=, 0);
    free(v);

    for (int i = 1; i <= 20; i++)
        st_skip_put_st(l, i, mk(i));

    /* put() on an existing key is refused.  Note it takes ownership of the
       value either way: on failure it frees the node, and free_entry frees
       the value with it, so the caller must NOT free it again. */
    assert_int(st_skip_put_st(l, 1, mk(1)), !=, 0);

    /* set() on an existing key replaces the value in place. */
    assert_int(st_skip_set_st(l, 10, mk(1010)), ==, 0);
    assert_string_equal(st_skip_get_st(l, 10), "v_1010");

    /* Several duplicates of one key, then delete them all. */
    for (int i = 0; i < 5; i++)
        assert_int(st_skip_dup_st(l, 7, mk(700 + i)), ==, 0);
    assert_size(st_skip_length_st(l), ==, 25);
    int removed = 0;
    while (st_skip_del_st(l, 7) == 0)
        removed++;
    assert_int(removed, ==, 6); /* the original plus five duplicates */
    assert_false(st_skip_contains_st(l, 7));
    assert_int(_skip_integrity_check_st(l, 1), ==, 0);

    st_skip_free_st(l);
    free(l);
    return MUNIT_OK;
}

/* Skewed access, so the splay rebalance actually runs.

   Under -DSKIPLIST_SPLAY_REBALANCE the rebalance pass fires once every
   SKIPLIST_SPLAY_INTERVAL read-only accesses and promotes/demotes nodes
   along the recorded path.  None of the other single-threaded tests do
   enough repeated lookups to trigger it, so its promote, demote, revert,
   and head-growth arms were never executed in this expansion.  Without
   the flag this is still a useful skewed-workload correctness test. */
static MunitResult
test_st_splay_skewed_access(const MunitParameter p[], void *d)
{
    (void)p;
    (void)d;

    st_t *l = malloc(sizeof(*l));
    st_skip_init_st(l);
    enum { N = 500 };
    for (int i = 0; i < N; i++)
        assert_int(st_skip_put_st(l, i, mk(i)), ==, 0);

    /* Interleave hot and cold lookups with a deterministic LCG.  Two
       phases (all hot, then all cold) would leave the hot keys pinned at
       their peak height, because a node's height is only re-evaluated
       when that node is accessed -- so demotion would never run. */
    uint32_t rng = 12345;
    const int hot[8] = { 3, 61, 127, 199, 251, 313, 401, 487 };
    for (int i = 0; i < 40000; i++) {
        rng = rng * 1103515245u + 12345u;
        int k = ((rng >> 16) % 100 < 85) ? hot[(rng >> 8) % 8] : (int)((rng >> 4) % N);
        /* Mix the read-only entry points, since each takes its own path. */
        switch (i % 3) {
        case 0:
            st_skip_get_st(l, k);
            break;
        case 1:
            st_skip_contains_st(l, k);
            break;
        default: {
            st_node_t q;
            memset(&q, 0, sizeof(q));
            q.key = k;
            st_skip_position_st(l, SKIP_EQ, &q);
            break;
        }
        }
    }

    /* Whatever the rebalance did, the structure must still be a valid,
       fully-reachable sorted set. */
    assert_int(_skip_integrity_check_st(l, 0), ==, 0);
    assert_size(st_skip_length_st(l), ==, (size_t)N);
    for (int i = 0; i < N; i++)
        assert_true(st_skip_contains_st(l, i));

    /* Ordering survives rebalancing. */
    st_node_t *cur;
    size_t idx;
    int prev = -1, seen = 0;
    SKIPLIST_FOREACH_H2T(st, st_, entries, l, cur, idx)
    {
        assert_int(cur->key, >, prev);
        prev = cur->key;
        seen++;
    }
    (void)idx;
    assert_int(seen, ==, N);

    /* Now delete the hot keys, which are the ones most likely to have been
       promoted: unlinking a tall node touches every level it reached. */
    for (int i = 0; i < 8; i++)
        assert_int(st_skip_del_st(l, hot[i]), ==, 0);
    assert_int(_skip_integrity_check_st(l, 0), ==, 0);

    /* And drain the rest. */
    for (int i = 0; i < N; i++)
        st_skip_del_st(l, i);
    assert_size(st_skip_length_st(l), ==, 0);
    assert_int(_skip_integrity_check_st(l, 0), ==, 0);

    st_skip_free_st(l);
    free(l);
    return MUNIT_OK;
}

static MunitTest tests[] = {
    { (char *)"/single/splay_skewed_access", test_st_splay_skewed_access, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/single/prev_validated_paths", test_st_prev_validated_paths, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/single/update_and_array_edges", test_st_update_and_array_edges, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/single/delete_with_duplicates", test_st_delete_with_duplicates, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/single/null_guards", test_st_null_guards, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/single/deserialize_robustness", test_st_deserialize_robustness, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/single/validate_corruption", test_st_validate_corruption, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/single/validate_early_exit_arms", test_st_validate_early_exit_arms, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/single/pool_exhaustion", test_st_pool_exhaustion, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/single/snapshot_edges", test_st_snapshot_edges, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/single/height_churn", test_st_height_churn, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/single/position_boundaries", test_st_position_boundaries, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
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
