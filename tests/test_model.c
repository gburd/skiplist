/*
 * test_model.c -- self-contained randomized differential ("property") test.
 *
 * Unlike tests/test_property.c (which uses the external hegel-c framework),
 * this test has NO external dependencies: it drives the skiplist with a
 * seeded PRNG and checks every operation against a trivial reference model
 * (a dense array over a small key space).  It therefore runs unconditionally
 * in CI and exercises the search/insert/remove/update/position branches plus
 * the integrity validator and the archive serialize/deserialize round-trip.
 *
 * Determinism: the seed defaults to 0x5eed1e55 and may be overridden with the
 * SKIPLIST_SEED environment variable so a CI failure is exactly reproducible.
 * The op count defaults to 200000 and may be overridden with SKIPLIST_OPS.
 *
 * Build (see Makefile target `test_model`):
 *   cc -Iinclude -std=c11 tests/test_model.c -o tests/test_model -lm
 * Optionally with -DSKIPLIST_SPLAY_REBALANCE to cover the adaptive variant.
 */

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sl.h"

/* ---- skiplist instantiation: int key -> int value --------------------- */

struct model_node {
    int key;
    int value;
    SKIPLIST_ENTRY(model) entries;
};

SKIPLIST_DECL(
    model, m_, entries,
    /* compare */
    {
        (void)list;
        (void)aux;
        return (a->key < b->key) ? -1 : (a->key > b->key) ? 1 : 0;
    },
    /* free */ { (void)node; },
    /* update */ { node->value = (int)(intptr_t)value; },
    /* archive (deep copy for snapshots) */
    {
        dest->key = src->key;
        dest->value = src->value;
    },
    /* sizeof */
    {
        (void)node;
        bytes = sizeof(int) * 2;
    })

SKIPLIST_DECL_ACCESS(
    model, m_, key, int, value, int,
    /* query */ { query.key = key; },
    /* return */ { return node->value; })

SKIPLIST_DECL_VALIDATE(model, m_, entries)

SKIPLIST_DECL_ARCHIVE(
    model, m_, entries,
    /* write */
    {
        memcpy(buf, &node->key, sizeof(int));
        memcpy(buf + sizeof(int), &node->value, sizeof(int));
        bytes = sizeof(int) * 2;
    },
    /* read */
    {
        if (bytes < sizeof(int) * 2) {
            node->key = 0;
            node->value = 0;
        } else {
            memcpy(&node->key, buf, sizeof(int));
            memcpy(&node->value, buf + sizeof(int), sizeof(int));
        }
    })

/* ---- reference model -------------------------------------------------- */

#define KSPACE 512

static int ref_present[KSPACE];
static int ref_value[KSPACE];
static size_t ref_count;

/* ---- deterministic PRNG ----------------------------------------------- */

static uint64_t rng_state;
static uint32_t
rng_next(void)
{
    /* SplitMix64-derived 32-bit output; fully deterministic. */
    uint64_t z = (rng_state += 0x9e3779b97f4a7c15ULL);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return (uint32_t)((z ^ (z >> 31)) >> 16);
}
static int
rng_key(void)
{
    return (int)(rng_next() % KSPACE);
}

/* ---- failure reporting ------------------------------------------------ */

static unsigned long g_op;
static unsigned int g_seed;

#define FAIL(...)                                                                          \
    do {                                                                                   \
        fprintf(stderr, "FAIL (seed=0x%x op=%lu): ", g_seed, g_op);                        \
        fprintf(stderr, __VA_ARGS__);                                                      \
        fprintf(stderr, "\n  reproduce: SKIPLIST_SEED=0x%x ./tests/test_model\n", g_seed); \
        exit(1);                                                                           \
    } while (0)

/* Expected position-query result against the reference model. */
static int
ref_position(skip_pos_model_t op, int key)
{
    switch (op) {
    case SKIP_EQ:
        return ref_present[key] ? key : -1;
    case SKIP_GTE:
        for (int k = key; k < KSPACE; k++)
            if (ref_present[k])
                return k;
        return -1;
    case SKIP_GT:
        for (int k = key + 1; k < KSPACE; k++)
            if (ref_present[k])
                return k;
        return -1;
    case SKIP_LTE:
        for (int k = key; k >= 0; k--)
            if (ref_present[k])
                return k;
        return -1;
    case SKIP_LT:
        for (int k = key - 1; k >= 0; k--)
            if (ref_present[k])
                return k;
        return -1;
    }
    return -1;
}

static void
check_full_equivalence(model_t *sl, const char *where)
{
    if (m_skip_length_model(sl) != ref_count)
        FAIL("%s: length %zu != model %zu", where, m_skip_length_model(sl), ref_count);

    for (int k = 0; k < KSPACE; k++) {
        int contains = m_skip_contains_model(sl, k);
        if (contains != (ref_present[k] != 0))
            FAIL("%s: contains(%d)=%d, model=%d", where, k, contains, ref_present[k]);
        if (ref_present[k]) {
            model_node_t *n = m_skip_position_eq_model(sl, &(model_node_t) { .key = k });
            if (n == NULL)
                FAIL("%s: present key %d not found", where, k);
            if (n->value != ref_value[k])
                FAIL("%s: key %d value %d != model %d", where, k, n->value, ref_value[k]);
        }
    }
}

static void
archive_roundtrip_check(model_t *sl)
{
    FILE *fp = tmpfile();
    if (fp == NULL)
        FAIL("tmpfile failed: %s", strerror(errno));

    if (m_skip_serialize_model(sl, fp) != 0)
        FAIL("serialize failed");
    rewind(fp);

    model_t copy;
    if (m_skip_init_model(&copy) != 0)
        FAIL("init copy failed");
    if (m_skip_deserialize_model(&copy, fp) != 0)
        FAIL("deserialize failed");
    fclose(fp);

    if (m_skip_length_model(&copy) != ref_count)
        FAIL("archive: restored length %zu != model %zu", m_skip_length_model(&copy), ref_count);
    for (int k = 0; k < KSPACE; k++) {
        if (ref_present[k]) {
            model_node_t *n = m_skip_position_eq_model(&copy, &(model_node_t) { .key = k });
            if (n == NULL || n->value != ref_value[k])
                FAIL("archive: key %d mismatch after round-trip", k);
        }
    }
    if (_skip_integrity_check_model(&copy, 1) != 0)
        FAIL("archive: restored list failed integrity check");
    m_skip_free_model(&copy);
}

int
main(void)
{
    const char *seed_env = getenv("SKIPLIST_SEED");
    g_seed = seed_env ? (unsigned int)strtoul(seed_env, NULL, 0) : 0x5eed1e55u;
    rng_state = g_seed;

    const char *ops_env = getenv("SKIPLIST_OPS");
    unsigned long ops = ops_env ? strtoul(ops_env, NULL, 0) : 200000UL;

    model_t sl;
    if (m_skip_init_model(&sl) != 0) {
        fprintf(stderr, "init failed\n");
        return 1;
    }

    for (g_op = 0; g_op < ops; g_op++) {
        int k = rng_key();
        int v = (int)rng_next();
        unsigned int choice = rng_next() % 100;

        if (choice < 35) {
            /* put: insert unique */
            int rc = m_skip_put_model(&sl, k, v);
            if (ref_present[k]) {
                if (rc != EEXIST)
                    FAIL("put(%d) on existing key returned %d, want EEXIST", k, rc);
            } else {
                if (rc != 0)
                    FAIL("put(%d) returned %d, want 0", k, rc);
                ref_present[k] = 1;
                ref_value[k] = v;
                ref_count++;
            }
        } else if (choice < 55) {
            /* del */
            int rc = m_skip_del_model(&sl, k);
            if (ref_present[k]) {
                if (rc != 0)
                    FAIL("del(%d) returned %d, want 0", k, rc);
                ref_present[k] = 0;
                ref_count--;
            } else {
                if (rc != ENOENT)
                    FAIL("del(%d) on absent key returned %d, want ENOENT", k, rc);
            }
        } else if (choice < 70) {
            /* set: update existing */
            int rc = m_skip_set_model(&sl, k, v);
            if (ref_present[k]) {
                if (rc != 0)
                    FAIL("set(%d) returned %d, want 0", k, rc);
                ref_value[k] = v;
            } else {
                if (rc != ENOENT)
                    FAIL("set(%d) on absent key returned %d, want ENOENT", k, rc);
            }
        } else if (choice < 80) {
            /* get */
            int got = m_skip_get_model(&sl, k);
            if (ref_present[k] && got != ref_value[k])
                FAIL("get(%d)=%d, model=%d", k, got, ref_value[k]);
        } else {
            /* position query */
            skip_pos_model_t ops_tbl[5] = { SKIP_EQ, SKIP_LT, SKIP_LTE, SKIP_GT, SKIP_GTE };
            skip_pos_model_t pop = ops_tbl[rng_next() % 5];
            model_node_t *n = m_skip_pos_model(&sl, pop, k);
            int expect = ref_position(pop, k);
            if (expect < 0) {
                if (n != NULL)
                    FAIL("pos(op=%d,%d) returned key %d, model=none", pop, k, n->key);
            } else {
                if (n == NULL)
                    FAIL("pos(op=%d,%d) returned none, model=%d", pop, k, expect);
                if (n->key != expect)
                    FAIL("pos(op=%d,%d) returned key %d, model=%d", pop, k, n->key, expect);
            }
        }

        /* Periodic deep checks: validator + full model equivalence + archive. */
        if ((g_op & 0x3ff) == 0) {
            if (_skip_integrity_check_model(&sl, 1) != 0)
                FAIL("integrity check failed");
            check_full_equivalence(&sl, "periodic");
            archive_roundtrip_check(&sl);
        }
    }

    check_full_equivalence(&sl, "final");
    if (_skip_integrity_check_model(&sl, 1) != 0)
        FAIL("final integrity check failed");

    m_skip_free_model(&sl);
    printf("test_model: PASS (seed=0x%x, ops=%lu, final length=%zu)\n", g_seed, ops, ref_count);
    return 0;
}
