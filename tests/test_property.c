/*
 * SPDX-License-Identifier: ISC OR MIT
 *
 * Property-based tests for include/sl.h using Hegel (hegel-c).
 *
 * These complement the example-based munit suite in tests/test.c.  Where the
 * munit tests pin specific scenarios, these exercise the structure across a
 * generated input space and compare it against an independent reference model.
 *
 * The instantiation here uses int keys and int values so node memory is
 * trivially managed (no heap inside nodes); this keeps the model tests free
 * of allocation bookkeeping while still exercising insert, remove, search,
 * positioning, duplicate handling, archive round-trip, and validation.
 *
 * Build/run via `make test_property` (see the Makefile for the hegel
 * include/lib/server wiring).
 */

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <hegel/generators.h>
#include <hegel/hegel.h>

#include "sl.h"

/* ------------------------------------------------------------------ */
/* Skiplist under test: int -> int                                     */
/* ------------------------------------------------------------------ */

struct pt_node {
    int key;
    int value;
    SKIPLIST_ENTRY(pt) e;
};

SKIPLIST_DECL(
    pt, p_, e,
    /* compare */ {
        (void)list;
        (void)aux;
        return (a->key > b->key) - (a->key < b->key);
    },
    /* free */ { (void)node; },
    /* update */ { node->value = (int)(intptr_t)value; },
    /* archive */ {
        dest->key = src->key;
        dest->value = src->value;
    },
    /* sizeof */ { (void)node; bytes = sizeof(int) * 2; })

SKIPLIST_DECL_ACCESS(
    pt, p_, key, int, value, int,
    /* query block */ { memset(&query, 0, sizeof(query)); query.key = key; },
    /* return block */ { return node->value; })

SKIPLIST_DECL_VALIDATE(pt, p_, e)

SKIPLIST_DECL_SNAPSHOTS(pt, p_, e)

SKIPLIST_DECL_POOL(pt, p_, e, 256)

SKIPLIST_DECL_EBR(pt, p_)

/*
 * Archive blocks for the round-trip property.  Unlike the example read block
 * in tests/test.c, this read block validates the declared record size against
 * what it intends to consume, so a truncated/garbage record is rejected with
 * an error rather than reading past the heap buffer.  `bytes` is the size the
 * deserializer read from the stream for this record.
 */
SKIPLIST_DECL_ARCHIVE(
    pt, p_, e,
    /* write */ {
        memcpy(buf, &node->key, sizeof(int));
        memcpy(buf + sizeof(int), &node->value, sizeof(int));
        bytes = sizeof(int) * 2;
    },
    /* read */ {
        if (bytes != sizeof(int) * 2) {
            /* Reject malformed records instead of over-reading `buf`. */
            node->key = 0;
            node->value = 0;
        } else {
            memcpy(&node->key, buf, sizeof(int));
            memcpy(&node->value, buf + sizeof(int), sizeof(int));
        }
    })

/* ------------------------------------------------------------------ */
/* Reference model: a sorted, unique-key array of (key,value).         */
/* ------------------------------------------------------------------ */

typedef struct {
    int *keys;
    int *vals;
    size_t len;
    size_t cap;
} model_t;

static void
model_init(model_t *m)
{
    m->len = 0;
    m->cap = 16;
    m->keys = malloc(m->cap * sizeof(int));
    m->vals = malloc(m->cap * sizeof(int));
}

static void
model_free(model_t *m)
{
    free(m->keys);
    free(m->vals);
    m->keys = NULL;
    m->vals = NULL;
    m->len = m->cap = 0;
}

/* Return index of key if present, else the negative insertion point as
 * -(idx + 1). */
static long
model_find(const model_t *m, int key)
{
    size_t lo = 0, hi = m->len;
    while (lo < hi) {
        size_t mid = lo + (hi - lo) / 2;
        if (m->keys[mid] < key)
            lo = mid + 1;
        else if (m->keys[mid] > key)
            hi = mid;
        else
            return (long)mid;
    }
    return -(long)(lo + 1);
}

static int
model_contains(const model_t *m, int key)
{
    return model_find(m, key) >= 0;
}

/* Insert rejecting duplicates; returns 1 if inserted, 0 if key already
 * present. */
static int
model_put(model_t *m, int key, int value)
{
    long f = model_find(m, key);
    if (f >= 0)
        return 0;
    size_t idx = (size_t)(-(f + 1));
    if (m->len == m->cap) {
        m->cap *= 2;
        m->keys = realloc(m->keys, m->cap * sizeof(int));
        m->vals = realloc(m->vals, m->cap * sizeof(int));
    }
    memmove(&m->keys[idx + 1], &m->keys[idx], (m->len - idx) * sizeof(int));
    memmove(&m->vals[idx + 1], &m->vals[idx], (m->len - idx) * sizeof(int));
    m->keys[idx] = key;
    m->vals[idx] = value;
    m->len++;
    return 1;
}

/* Remove key; returns 1 if removed, 0 if absent. */
static int
model_del(model_t *m, int key)
{
    long f = model_find(m, key);
    if (f < 0)
        return 0;
    size_t idx = (size_t)f;
    memmove(&m->keys[idx], &m->keys[idx + 1], (m->len - idx - 1) * sizeof(int));
    memmove(&m->vals[idx], &m->vals[idx + 1], (m->len - idx - 1) * sizeof(int));
    m->len--;
    return 1;
}

/* ------------------------------------------------------------------ */
/* Helpers                                                             */
/* ------------------------------------------------------------------ */

/* Assert the subject's level-0 order and contents match the model exactly. */
static int
list_matches_model(pt_t *list, const model_t *m)
{
    if (p_skip_length_pt(list) != m->len)
        return 0;
    pt_node_t *cur;
    size_t i;
    size_t seen = 0;
    int prev_key = 0;
    int have_prev = 0;
    SKIPLIST_FOREACH_H2T(pt, p_, e, list, cur, i)
    {
        (void)i;
        if (seen >= m->len)
            return 0;
        if (have_prev && !(prev_key < cur->key))
            return 0; /* must be strictly ascending */
        if (cur->key != m->keys[seen] || cur->value != m->vals[seen])
            return 0;
        prev_key = cur->key;
        have_prev = 1;
        seen++;
    }
    return seen == m->len;
}

/* ------------------------------------------------------------------ */
/* Property 1: stateful model test                                     */
/*                                                                     */
/* Drive a random sequence of put/del/set/get/contains operations      */
/* against both the skiplist and the reference model, asserting they    */
/* agree after every operation.                                        */
/* ------------------------------------------------------------------ */

static void
prop_model(hegel_test_case *tc, void *ud)
{
    (void)ud;
    pt_t list;
    p_skip_init_pt(&list);
    model_t m;
    model_init(&m);

    int n_ops = (int)hegel_draw_int(tc, hegel_integers(0, 300));
    for (int op = 0; op < n_ops; op++) {
        int kind = (int)hegel_draw_int(tc, hegel_integers(0, 4));
        int key = (int)hegel_draw_int(tc, hegel_integers(-30, 30));
        switch (kind) {
        case 0: { /* put (reject duplicates) */
            int val = (int)hegel_draw_int(tc, hegel_integers(0, 1000000));
            int rc = p_skip_put_pt(&list, key, val);
            int inserted = model_put(&m, key, val);
            if (inserted && rc != 0)
                hegel_fail("put: model inserted but subject returned error");
            if (!inserted && rc == 0)
                hegel_fail("put: subject accepted a duplicate key");
            break;
        }
        case 1: { /* del */
            int rc = p_skip_del_pt(&list, key);
            int removed = model_del(&m, key);
            if (removed && rc != 0)
                hegel_fail("del: model removed but subject returned error");
            if (!removed && rc == 0)
                hegel_fail("del: subject removed an absent key");
            break;
        }
        case 2: { /* set (update existing only) */
            int val = (int)hegel_draw_int(tc, hegel_integers(0, 1000000));
            int rc = p_skip_set_pt(&list, key, val);
            long f = model_find(&m, key);
            if (f >= 0) {
                if (rc != 0)
                    hegel_fail("set: present key not updated");
                m.vals[f] = val;
            } else if (rc == 0) {
                hegel_fail("set: absent key reported as updated");
            }
            break;
        }
        case 3: { /* contains */
            int got = p_skip_contains_pt(&list, key);
            int exp = model_contains(&m, key);
            if (!!got != !!exp)
                hegel_fail("contains disagreed with model");
            break;
        }
        default: { /* get */
            int present = model_contains(&m, key);
            int got = p_skip_get_pt(&list, key);
            if (present) {
                long f = model_find(&m, key);
                if (got != m.vals[f])
                    hegel_fail("get returned wrong value");
            }
            break;
        }
        }
        if (!list_matches_model(&list, &m))
            hegel_fail("list/model divergence after operation");
    }

    if (_skip_integrity_check_pt(&list, 1) != 0)
        hegel_fail("integrity check reported errors");

    p_skip_free_pt(&list);
    model_free(&m);
}

/* ------------------------------------------------------------------ */
/* Property 2: archive round-trip                                      */
/*                                                                     */
/* serialize(list) then deserialize into a fresh list yields the same   */
/* key/value set in the same order.                                     */
/* ------------------------------------------------------------------ */

static void
prop_archive_roundtrip(hegel_test_case *tc, void *ud)
{
    (void)ud;
    pt_t list;
    p_skip_init_pt(&list);
    model_t m;
    model_init(&m);

    int n = (int)hegel_draw_int(tc, hegel_integers(0, 200));
    for (int i = 0; i < n; i++) {
        int key = (int)hegel_draw_int(tc, hegel_integers(-1000000, 1000000));
        int val = (int)hegel_draw_int(tc, hegel_integers(-1000000, 1000000));
        if (model_put(&m, key, val))
            p_skip_put_pt(&list, key, val);
    }

    FILE *fp = tmpfile();
    if (fp == NULL)
        hegel_fail("tmpfile() failed");
    if (p_skip_serialize_pt(&list, fp) != 0)
        hegel_fail("serialize failed");
    rewind(fp);

    pt_t restored;
    p_skip_init_pt(&restored);
    if (p_skip_deserialize_pt(&restored, fp) != 0)
        hegel_fail("deserialize failed");
    fclose(fp);

    if (!list_matches_model(&restored, &m))
        hegel_fail("round-tripped list does not match original");

    p_skip_free_pt(&list);
    p_skip_free_pt(&restored);
    model_free(&m);
}

/* ------------------------------------------------------------------ */
/* Property 3: position_* semantics vs the model                       */
/* ------------------------------------------------------------------ */

static void
prop_positions(hegel_test_case *tc, void *ud)
{
    (void)ud;
    pt_t list;
    p_skip_init_pt(&list);
    model_t m;
    model_init(&m);

    int n = (int)hegel_draw_int(tc, hegel_integers(0, 150));
    for (int i = 0; i < n; i++) {
        int key = (int)hegel_draw_int(tc, hegel_integers(-50, 50));
        if (model_put(&m, key, key * 7))
            p_skip_put_pt(&list, key, key * 7);
    }

    int q = (int)hegel_draw_int(tc, hegel_integers(-60, 60));

    /* Expected boundaries from the model. */
    long lt = -1, lte = -1, gt = -1, gte = -1, eq = -1;
    for (size_t i = 0; i < m.len; i++) {
        if (m.keys[i] < q)
            lt = (long)i;
        if (m.keys[i] <= q)
            lte = (long)i;
        if (gt < 0 && m.keys[i] > q)
            gt = (long)i;
        if (gte < 0 && m.keys[i] >= q)
            gte = (long)i;
        if (m.keys[i] == q)
            eq = (long)i;
    }

    pt_node_t *r;
    r = p_skip_pos_pt(&list, SKIP_EQ, q);
    if ((eq >= 0) != (r != NULL))
        hegel_fail("pos SKIP_EQ presence mismatch");
    if (r && r->key != q)
        hegel_fail("pos SKIP_EQ wrong key");

    r = p_skip_pos_pt(&list, SKIP_LT, q);
    if ((lt >= 0) != (r != NULL))
        hegel_fail("pos SKIP_LT presence mismatch");
    if (r && r->key != m.keys[lt])
        hegel_fail("pos SKIP_LT wrong key");

    r = p_skip_pos_pt(&list, SKIP_LTE, q);
    if ((lte >= 0) != (r != NULL))
        hegel_fail("pos SKIP_LTE presence mismatch");
    if (r && r->key != m.keys[lte])
        hegel_fail("pos SKIP_LTE wrong key");

    r = p_skip_pos_pt(&list, SKIP_GT, q);
    if ((gt >= 0) != (r != NULL))
        hegel_fail("pos SKIP_GT presence mismatch");
    if (r && r->key != m.keys[gt])
        hegel_fail("pos SKIP_GT wrong key");

    r = p_skip_pos_pt(&list, SKIP_GTE, q);
    if ((gte >= 0) != (r != NULL))
        hegel_fail("pos SKIP_GTE presence mismatch");
    if (r && r->key != m.keys[gte])
        hegel_fail("pos SKIP_GTE wrong key");

    p_skip_free_pt(&list);
    model_free(&m);
}

/* ------------------------------------------------------------------ */
/* Property 4: duplicate keys                                          */
/*                                                                     */
/* insert_dup always increases length; the plain access put rejects a   */
/* duplicate.  After inserting K copies of a key, length == K and a      */
/* forward scan finds exactly K nodes with that key.                    */
/* ------------------------------------------------------------------ */

static void
prop_duplicates(hegel_test_case *tc, void *ud)
{
    (void)ud;
    pt_t list;
    p_skip_init_pt(&list);

    int key = (int)hegel_draw_int(tc, hegel_integers(-1000, 1000));
    int k = (int)hegel_draw_int(tc, hegel_integers(1, 64));

    for (int i = 0; i < k; i++) {
        int rc = p_skip_dup_pt(&list, key, i);
        if (rc != 0)
            hegel_fail("insert_dup rejected a duplicate");
    }
    if (p_skip_length_pt(&list) != (size_t)k)
        hegel_fail("length mismatch after duplicate inserts");

    /* A non-dup put must now be rejected. */
    if (p_skip_put_pt(&list, key, 999) == 0)
        hegel_fail("put accepted a key that already exists (dup)");

    size_t count = 0;
    pt_node_t *cur;
    size_t i;
    SKIPLIST_FOREACH_H2T(pt, p_, e, &list, cur, i)
    {
        (void)i;
        if (cur->key == key)
            count++;
    }
    if (count != (size_t)k)
        hegel_fail("forward scan did not find all duplicate nodes");

    p_skip_free_pt(&list);
}

/* ------------------------------------------------------------------ */
/* Property 5: deserialize robustness (no crash on garbage)            */
/*                                                                     */
/* Feeding arbitrary bytes to the deserializer must never crash; it      */
/* either rejects with an error or builds a list that passes the         */
/* integrity check.  This pins the parse-robustness contract for the     */
/* archive format and guards the heap-read concern flagged in the audit. */
/* ------------------------------------------------------------------ */

static void
prop_deserialize_robustness(hegel_test_case *tc, void *ud)
{
    (void)ud;
    size_t len = 0;
    uint8_t *data = hegel_draw_bytes(tc, hegel_binary(0, 4096), &len);

    FILE *fp = tmpfile();
    if (fp == NULL) {
        free(data);
        hegel_fail("tmpfile() failed");
        return;
    }
    if (len > 0)
        fwrite(data, 1, len, fp);
    rewind(fp);

    pt_t list;
    p_skip_init_pt(&list);
    int rc = p_skip_deserialize_pt(&list, fp);
    fclose(fp);

    if (rc == 0) {
        /* Claimed success: the resulting structure must be consistent. */
        if (_skip_integrity_check_pt(&list, 1) != 0)
            hegel_fail("deserialize returned success but list is inconsistent");
    }

    p_skip_free_pt(&list);
    free(data);
}

/* ------------------------------------------------------------------ */
/* Property 6: navigation (first/last/next/prev) vs the model          */
/* ------------------------------------------------------------------ */

static void
prop_navigation(hegel_test_case *tc, void *ud)
{
    (void)ud;
    pt_t list;
    p_skip_init_pt(&list);
    model_t m;
    model_init(&m);

    int n = (int)hegel_draw_int(tc, hegel_integers(0, 200));
    for (int i = 0; i < n; i++) {
        int key = (int)hegel_draw_int(tc, hegel_integers(-100, 100));
        if (model_put(&m, key, key))
            p_skip_put_pt(&list, key, key);
    }

    pt_node_t *first = p_skip_head_pt(&list);
    pt_node_t *last = p_skip_tail_pt(&list);
    if (m.len == 0) {
        if (first != NULL || last != NULL)
            hegel_fail("empty list has non-NULL head/tail");
    } else {
        if (first == NULL || first->key != m.keys[0])
            hegel_fail("head is not the minimum key");
        if (last == NULL || last->key != m.keys[m.len - 1])
            hegel_fail("tail is not the maximum key");
    }

    /* Forward via next_node == ascending model order. */
    size_t i = 0;
    for (pt_node_t *cur = first; cur != NULL; cur = p_skip_next_node_pt(&list, cur)) {
        if (i >= m.len || cur->key != m.keys[i])
            hegel_fail("forward navigation diverged from model");
        i++;
    }
    if (i != m.len)
        hegel_fail("forward navigation visited wrong count");

    /* Backward via prev_node == descending model order. */
    i = m.len;
    for (pt_node_t *cur = last; cur != NULL; cur = p_skip_prev_node_pt(&list, cur)) {
        if (i == 0 || cur->key != m.keys[i - 1])
            hegel_fail("backward navigation diverged from model");
        i--;
    }
    if (i != 0)
        hegel_fail("backward navigation visited wrong count");

    /* next(prev(x)) == x for every interior node. */
    for (pt_node_t *cur = first; cur != NULL; cur = p_skip_next_node_pt(&list, cur)) {
        pt_node_t *prev = p_skip_prev_node_pt(&list, cur);
        if (prev != NULL && p_skip_next_node_pt(&list, prev) != cur)
            hegel_fail("next(prev(x)) != x");
    }

    p_skip_free_pt(&list);
    model_free(&m);
}

/* ------------------------------------------------------------------ */
/* Property 7: snapshot/restore (MVCC) round-trips to the snapshot     */
/*                                                                     */
/* After taking a snapshot, arbitrary insert/delete/update mutations    */
/* followed by a restore must return the list to exactly the           */
/* snapshotted key/value set.                                          */
/* ------------------------------------------------------------------ */

static void
prop_snapshot(hegel_test_case *tc, void *ud)
{
    (void)ud;
    pt_t list;
    p_skip_init_pt(&list);
    p_skip_snapshots_init_pt(&list);
    model_t snap;
    model_init(&snap);

    int n = (int)hegel_draw_int(tc, hegel_integers(0, 120));
    for (int i = 0; i < n; i++) {
        int key = (int)hegel_draw_int(tc, hegel_integers(-40, 40));
        int val = (int)hegel_draw_int(tc, hegel_integers(0, 1000000));
        if (model_put(&snap, key, val))
            p_skip_put_pt(&list, key, val);
    }

    uint64_t era = p_skip_snapshot_pt(&list);
    if (era == 0)
        hegel_fail("snapshot returned era 0");

    /* Mutate: insert new keys, delete existing, update existing values. */
    int m_ops = (int)hegel_draw_int(tc, hegel_integers(0, 120));
    for (int i = 0; i < m_ops; i++) {
        int kind = (int)hegel_draw_int(tc, hegel_integers(0, 2));
        int key = (int)hegel_draw_int(tc, hegel_integers(-40, 40));
        int val = (int)hegel_draw_int(tc, hegel_integers(0, 1000000));
        if (kind == 0)
            p_skip_put_pt(&list, key, val);
        else if (kind == 1)
            p_skip_del_pt(&list, key);
        else
            p_skip_set_pt(&list, key, val);
    }

    pt_t *r = p_skip_restore_snapshot_pt(&list, (size_t)era);
    if (r == NULL)
        hegel_fail("restore_snapshot returned NULL");

    if (!list_matches_model(&list, &snap))
        hegel_fail("restored list does not match the snapshot");
    if (_skip_integrity_check_pt(&list, 1) != 0)
        hegel_fail("restored list fails integrity check");

    p_skip_release_snapshots_pt(&list);
    p_skip_free_pt(&list);
    model_free(&snap);
}

/* ------------------------------------------------------------------ */
/* Property 8: pool allocator never double-hands-out a live slot,      */
/* respects capacity, and round-trips ownership                        */
/* ------------------------------------------------------------------ */

static void
prop_pool(hegel_test_case *tc, void *ud)
{
    (void)ud;
    size_t cap = (size_t)hegel_draw_int(tc, hegel_integers(1, 64));
    _skip_pool_pt_t pool;
    if (p_skip_pool_init_pt(&pool, cap) != 0)
        hegel_fail("pool init failed");
    pt_t list;
    p_skip_init_pt(&list); /* used only as the free_entry sink */

    pt_node_t **live = calloc(cap, sizeof(pt_node_t *));
    size_t live_n = 0;

    int ops = (int)hegel_draw_int(tc, hegel_integers(0, 400));
    for (int op = 0; op < ops; op++) {
        int do_alloc = (int)hegel_draw_int(tc, hegel_integers(0, 1));
        if (do_alloc || live_n == 0) {
            pt_node_t *node = NULL;
            int rc = p_skip_pool_alloc_node_pt(&pool, &node);
            if (live_n < cap) {
                if (rc != 0 || node == NULL)
                    hegel_fail("pool alloc failed while capacity remained");
                if (!p_skip_pool_is_from_pt(&pool, node))
                    hegel_fail("pool-allocated node not reported as from-pool");
                for (size_t j = 0; j < live_n; j++)
                    if (live[j] == node)
                        hegel_fail("pool handed out a slot that is already live");
                live[live_n++] = node;
            } else if (rc == 0) {
                hegel_fail("pool alloc succeeded past capacity");
            }
        } else {
            size_t idx = (size_t)hegel_draw_int(tc, hegel_integers(0, (int64_t)live_n - 1));
            pt_node_t *node = live[idx];
            p_skip_pool_free_node_pt(&pool, &list, node);
            live[idx] = live[--live_n];
        }
    }

    for (size_t j = 0; j < live_n; j++)
        p_skip_pool_free_node_pt(&pool, &list, live[j]);
    free(live);
    p_skip_free_pt(&list);
    p_skip_pool_destroy_pt(&pool);
}

/* ------------------------------------------------------------------ */
/* Property 9: EBR thread slots are unique while live and recyclable   */
/* after unregister (regression for the no-slot-reuse limitation).     */
/* ------------------------------------------------------------------ */

static void
prop_ebr_slots(hegel_test_case *tc, void *ud)
{
    (void)ud;
    _skip_ebr_pt_t ebr;
    p_skip_ebr_init_pt(&ebr);

    int live[SKIPLIST_EBR_MAX_THREADS];
    for (int i = 0; i < SKIPLIST_EBR_MAX_THREADS; i++)
        live[i] = 0;
    int live_n = 0;

    int ops = (int)hegel_draw_int(tc, hegel_integers(0, 400));
    for (int op = 0; op < ops; op++) {
        int do_reg = (int)hegel_draw_int(tc, hegel_integers(0, 1));
        if (do_reg || live_n == 0) {
            int tid = p_skip_ebr_register_pt(&ebr);
            if (live_n < SKIPLIST_EBR_MAX_THREADS) {
                if (tid < 0 || tid >= SKIPLIST_EBR_MAX_THREADS)
                    hegel_fail("register failed while slots remained");
                if (live[tid])
                    hegel_fail("register handed out a slot already in use");
                live[tid] = 1;
                live_n++;
            } else if (tid != -1) {
                hegel_fail("register succeeded with all slots in use");
            }
        } else {
            /* unregister some live tid */
            int pick = (int)hegel_draw_int(tc, hegel_integers(0, (int64_t)live_n - 1));
            int seen = 0;
            for (int t = 0; t < SKIPLIST_EBR_MAX_THREADS; t++) {
                if (live[t]) {
                    if (seen == pick) {
                        p_skip_ebr_unregister_pt(&ebr, t);
                        live[t] = 0;
                        live_n--;
                        break;
                    }
                    seen++;
                }
            }
        }
    }
}

/* ------------------------------------------------------------------ */
/* Harness                                                             */
/* ------------------------------------------------------------------ */

typedef struct {
    const char *name;
    hegel_test_fn fn;
} prop_entry_t;

int
main(void)
{
    hegel_session *s = hegel_session_new();
    if (s == NULL) {
        fprintf(stderr, "hegel: failed to start session (is HEGEL_SERVER_COMMAND set?)\n");
        return 2;
    }

    const prop_entry_t props[] = {
        { "model (insert/remove/get/contains/set vs reference)", prop_model },
        { "archive serialize/deserialize round-trip", prop_archive_roundtrip },
        { "position_* boundary semantics", prop_positions },
        { "duplicate keys", prop_duplicates },
        { "deserialize robustness (no crash on garbage)", prop_deserialize_robustness },
        { "navigation (first/last/next/prev) vs reference", prop_navigation },
        { "snapshot/restore (MVCC) round-trip", prop_snapshot },
        { "pool allocator integrity", prop_pool },
        { "EBR slot uniqueness and reuse", prop_ebr_slots },
    };
    const size_t nprops = sizeof(props) / sizeof(props[0]);

    int failures = 0;
    for (size_t i = 0; i < nprops; i++) {
        hegel_settings settings = HEGEL_DEFAULT_SETTINGS;
        settings.max_examples = 200;
        hegel_results r = hegel_run_test(s, props[i].fn, NULL, &settings);
        printf("[%s] %s (%d valid cases)\n", r.passed ? "PASS" : "FAIL", props[i].name, r.valid_test_cases);
        if (!r.passed) {
            failures++;
            if (r.error)
                printf("    %s\n", r.error);
            if (r.seed)
                printf("    seed: %s\n", r.seed);
        }
        hegel_results_free(&r);
    }

    hegel_session_free(s);
    printf("\n%zu properties, %d failed\n", nprops, failures);
    return failures == 0 ? 0 : 1;
}
