/*
 * tests/test_splay_verify.c -- verify the splay rebalance reproduces
 * the Aksenov 2020 "The Splay-List" target depth.
 *
 * Theory (Aksenov, Alistarh, Drozdova, Mohtashami 2020):
 *   A node accessed with frequency p = u/T should settle at height
 *   K - 1 - log2(1/p), placing it at depth log2(1/p) below the head.
 *   This is the information-theoretic optimum for search cost: a
 *   key looked up with probability p should require O(log(1/p))
 *   pointer chases.
 *
 * Methodology:
 *   Build a 200-key skiplist, then issue 20,000 lookups distributed
 *   between a designated hot key and uniformly-spread cold keys via
 *   a Bernoulli(p) draw.  Splay is intentionally lazy -- a node's
 *   height is re-evaluated only when that node is accessed -- so the
 *   draw must INTERLEAVE hot and cold accesses to give the algorithm
 *   a chance to demote as well as promote.
 *
 *   Tolerance: +/-1 level.  The rebalance fires every
 *   SKIPLIST_SPLAY_INTERVAL accesses and adjusts by one level per
 *   pass, so a setting that has not yet converged can lag the
 *   theoretical target by one rung.
 *
 * Build:
 *   This translation unit is only meaningful with
 *   -DSKIPLIST_SPLAY_REBALANCE.  Without that flag the test compiles
 *   but reports a single skipped result (the splay function is a
 *   no-op).
 */
#define _POSIX_C_SOURCE 200809L
#define MUNIT_ENABLE_ASSERT_ALIASES

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "munit.h"
#include "sl.h"

struct sv_node {
    int key;
    SKIPLIST_ENTRY(sv) ent;
};

SKIPLIST_DECL(sv, sv_, ent,
    /* cmp     */ { (void)list; (void)aux;
                    return (a->key > b->key) - (a->key < b->key); },
    /* free    */ { (void)node; },
    /* update  */ { (void)node; (void)value; rc = 0; },
    /* archive */ { (void)dest; (void)src; rc = 0; },
    /* sizeof  */ { (void)node; bytes = sizeof(sv_node_t); })

#ifdef SKIPLIST_SPLAY_REBALANCE

/* Run a single Bernoulli(p) workload and return the hot key's final
 * height plus the head's current height. */
static void
run_workload(int n, int hot_key, double p, int total_accesses,
    int *out_hot_h, int *out_head_h)
{
    sv_t list;
    sv_skip_init_sv(&list);
    list.slh_prng_state = 0xC0FFEE;

    for (int i = 1; i <= n; i++) {
        sv_node_t *node;
        sv_skip_alloc_node_sv(&node);
        node->key = i;
        sv_skip_insert_sv(&list, node);
    }

    /* Deterministic LCG for reproducible coin flips. */
    uint64_t s = 0xDEADBEEFCAFEBABEULL;
    int cold_idx = 0;
    for (int i = 0; i < total_accesses; i++) {
        s = s * 6364136223846793005ULL + 1442695040888963407ULL;
        double u = (double)((s >> 11) & ((1ULL << 53) - 1)) / (double)(1ULL << 53);
        int k;
        if (u < p) {
            k = hot_key;
        } else {
            cold_idx = (cold_idx + 1) % n;
            k = 1 + cold_idx;
            if (k == hot_key)
                k = (k % n) + 1;
        }
        sv_node_t q = { .key = k };
        (void)sv_skip_position_eq_sv(&list, &q);
    }

    sv_node_t hq = { .key = hot_key };
    sv_node_t *hot = sv_skip_position_eq_sv(&list, &hq);
    *out_hot_h = (int)hot->ent.sle_height;
    *out_head_h = (int)list.slh_head->ent.sle_height;

    sv_skip_free_sv(&list);
}

/* Integer log2 (floor) of a positive double; returns 0 for v >= 1.0. */
static int
ilog2_inv(double p)
{
    int k = 0;
    double v = p;
    if (v >= 1.0)
        return 0;
    while (v < 1.0) {
        v *= 2.0;
        k--;
    }
    return k; /* k is negative; caller uses K - 1 + k as target */
}

struct sv_case {
    const char *label;
    double p;
};

static const struct sv_case sv_cases[] = {
    { "p=1.000 (pure hot)",   1.0 },
    { "p=0.500 (half-hot)",   0.5 },
    { "p=0.250 (quarter)",    0.25 },
    { "p=0.125 (eighth)",     0.125 },
    { "p=0.062 (sixteenth)",  0.0625 },
    { "p=0.005 (cold)",       0.005 },
};
#endif /* SKIPLIST_SPLAY_REBALANCE */

static MunitResult
test_aksenov_target(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

#ifndef SKIPLIST_SPLAY_REBALANCE
    return MUNIT_SKIP;
#else
    const int n = 200;
    const int hot_key = 100;
    const int total = 20000;
    const int n_cases = (int)(sizeof(sv_cases) / sizeof(sv_cases[0]));
    const int tol = 1; /* +/-1 level convergence tolerance */

    /* Print header.  munit captures stdout per test; this surfaces in
     * verbose / failure runs. */
    printf("\n");
    printf("Aksenov 2020 splay-list height verification\n");
    printf("  workload: n=%d keys, total accesses=%d\n", n, total);
    printf("  target:   h = K - 1 - log2(1/p) (+/- %d level)\n", tol);
    printf("\n  %-22s %5s %5s %8s %8s %8s\n",
        "case", "K", "h", "expect", "result", "delta");
    printf("  %-22s %5s %5s %8s %8s %8s\n",
        "----------------------", "-----", "-----", "------", "------", "-----");

    int failed = 0;
    for (int i = 0; i < n_cases; i++) {
        int hot_h = 0, head_h = 0;
        run_workload(n, hot_key, sv_cases[i].p, total, &hot_h, &head_h);

        /* target = K - 1 + log2(p), clamped to >= 0 */
        int log_p = ilog2_inv(sv_cases[i].p);
        int expect = head_h - 1 + log_p;
        if (expect < 0)
            expect = 0;
        int delta = hot_h - expect;
        int ok = (delta >= -tol && delta <= tol);
        if (!ok)
            failed++;
        printf("  %-22s %5d %5d %8d %8s %+8d\n",
            sv_cases[i].label, head_h, hot_h, expect,
            ok ? "PASS" : "FAIL", delta);
    }
    printf("\n");

    munit_assert_int(failed, ==, 0);
    return MUNIT_OK;
#endif
}

/* Sanity test that runs even without the splay flag: confirms the
 * test binary at least links and can construct/teardown a list with
 * the SKIPLIST_DECL we use above. */
static MunitResult
test_basic_construct(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;
    sv_t list;
    sv_skip_init_sv(&list);
    for (int i = 1; i <= 10; i++) {
        sv_node_t *node;
        sv_skip_alloc_node_sv(&node);
        node->key = i;
        sv_skip_insert_sv(&list, node);
    }
    munit_assert_size(sv_skip_length_sv(&list), ==, 10);
    sv_skip_free_sv(&list);
    return MUNIT_OK;
}

static MunitTest tests[] = {
    { (char *)"/splay_verify/basic_construct", test_basic_construct,
        NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/splay_verify/aksenov_target", test_aksenov_target,
        NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
};

static const MunitSuite suite = {
    (char *)"", tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};

int
main(int argc, char *argv[MUNIT_ARRAY_PARAM(argc + 1)])
{
    return munit_suite_main(&suite, (void *)"splay_verify", argc, argv);
}
