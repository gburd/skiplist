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

/* Counts comparator invocations, so tests can measure search cost in a way
 * that is independent of machine load (unlike wall-clock timing). */
static long sv_cmp_count;

SKIPLIST_DECL(sv, sv_, ent,
    /* cmp     */ { (void)list; (void)aux; sv_cmp_count++;
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

/* Contiguous-hot-range regression.
 *
 * The paper's target height h = K-1-log2(1/p) depends only on
 * p = u_hits/m_total, so every node sharing an access probability targets
 * the same height.  For a CONTIGUOUS hot range every key shares p, so the
 * whole range used to climb together and the upper levels degenerated into
 * copies of the base list over it.  Measured steady-state search cost went
 * from 25.8 comparisons per lookup with splay off to 467 with it on -- an
 * 18x regression on a common access pattern (a hot key prefix, a recent-ID
 * window).
 *
 * The promotion path now refuses a level whose level-0 successor is
 * comparably hot, which confines promotion inside a dense hot region while
 * leaving a lone hot key free to rise (that is what aksenov_target above
 * checks).  This test pins the outcome so the gate cannot be removed or
 * weakened silently.
 *
 * The cost is measured AFTER warming, deliberately.  A cumulative average
 * over the whole run keeps climbing long after the structure has settled,
 * because early cheap lookups are progressively diluted by later dear
 * ones -- which is exactly how the regression was first mis-diagnosed as
 * unbounded when it in fact converges.
 */
static MunitResult
test_contiguous_hot_range(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

#ifndef SKIPLIST_SPLAY_REBALANCE
    return MUNIT_SKIP;
#else
    enum { N = 20000, HOT = 200, WARM = 400000, MEASURE = 20000 };

    /* cmp_count is incremented by the comparison block in SKIPLIST_DECL. */
    sv_t list;
    sv_skip_init_sv(&list);
    list.slh_prng_state = 0xC0FFEE; /* pin the tower shape */

    for (int i = 0; i < N; i++) {
        sv_node_t *node;
        sv_skip_alloc_node_sv(&node);
        node->key = i;
        sv_skip_insert_sv(&list, node);
    }

    /* 90% of lookups into the contiguous range [0, HOT), 10% uniform. */
    uint32_t rng = 12345;
    for (long i = 0; i < WARM; i++) {
        rng = rng * 1103515245u + 12345u;
        int k = ((rng >> 16) % 10) ? (int)((rng >> 4) % HOT) : (int)((rng >> 4) % N);
        sv_node_t q;
        q.key = k;
        sv_skip_position_sv(&list, SKIP_EQ, &q);
    }

    /* Steady-state measurement. */
    sv_cmp_count = 0;
    rng = 12345;
    for (long i = 0; i < MEASURE; i++) {
        rng = rng * 1103515245u + 12345u;
        int k = ((rng >> 16) % 10) ? (int)((rng >> 4) % HOT) : (int)((rng >> 4) % N);
        sv_node_t q;
        q.key = k;
        sv_skip_position_sv(&list, SKIP_EQ, &q);
    }
    double cmp_per_op = (double)sv_cmp_count / (double)MEASURE;

    printf("\n  contiguous hot range: %.1f comparisons/lookup"
           " (n=%d, hot=[0,%d), warm=%d)\n",
        cmp_per_op, N, HOT, WARM);

    /* A plain skiplist over N keys needs roughly 2*log2(N) comparisons, so
     * ~29 here.  With the promotion gate this workload measures ~26; without
     * it, ~70.  The bound sits between the two, close enough to catch a
     * collapse back into a dense chain but loose enough to tolerate ordinary
     * variation in tower shape. */
    const double healthy = 2.0 * 14.3; /* 2*log2(20000) */
    munit_assert_double(cmp_per_op, <, healthy * 1.5);

    /* And the structure must still be sound. */
    munit_assert_size(sv_skip_length_sv(&list), ==, (size_t)N);

    sv_skip_free_sv(&list);
    return MUNIT_OK;
#endif
}

static MunitTest tests[] = {
    { (char *)"/splay_verify/contiguous_hot_range", test_contiguous_hot_range,
        NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
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
