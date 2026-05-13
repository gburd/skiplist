/*
 * scripts/splay_verify.c -- end-to-end verification that splay
 * rebalancing reproduces the Aksenov 2020 paper's height target.
 *
 * The paper's claim: a node with hit ratio p = u/T should settle at
 * depth log2(1/p) below the head.  Equivalently, height = K - 1 -
 * log2(T/u) where K is the head's current height.  We test three
 * regimes:
 *
 *   1. Pure hot (p = 1.0): height should approach K - 1.
 *   2. Half-hot (p = 0.5): height should approach K - 2.
 *   3. Quarter-hot (p = 0.25): height should approach K - 3.
 *
 * Build: gcc -O2 -std=c11 -Iinclude/ -DSKIPLIST_SPLAY_REBALANCE \
 *            scripts/splay_verify.c -lm -pthread -o splay_verify
 */
#define _POSIX_C_SOURCE 200809L

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sl.h"

struct k_node {
    int key;
    SKIPLIST_ENTRY(k) ent;
};

SKIPLIST_DECL(k, k_, ent,
    /* cmp     */ { (void)list; (void)aux;
                    return (a->key > b->key) - (a->key < b->key); },
    /* free    */ { (void)node; },
    /* update  */ { (void)node; (void)value; rc = 0; },
    /* archive */ { (void)dest; (void)src; rc = 0; },
    /* sizeof  */ { (void)node; bytes = sizeof(k_node_t); });

/* Hammer key=hot_key with ratio (out of total_accesses), distributing
 * the remainder uniformly across the other (n-1) keys.  Splay is
 * lazy: a node's height is only re-evaluated when that node is
 * accessed, so we INTERLEAVE hot and cold accesses to give the
 * algorithm a chance to settle to the steady-state height. */
static void
run(int n, int hot_key, double p, int total_accesses, int *out_hot_h, int *out_head_h)
{
    k_t list;
    k_skip_init_k(&list);
    list.slh_prng_state = 0xC0FFEE;

    for (int i = 1; i <= n; i++) {
        k_node_t *node;
        k_skip_alloc_node_k(&node);
        node->key = i;
        k_skip_insert_k(&list, node);
    }

    /* Bernoulli mixing: at every access, draw uniformly and dispatch
     * to either the hot key or a cold key.  Use a deterministic LCG
     * for repeatability. */
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
        k_node_t q = { .key = k };
        (void)k_skip_position_eq_k(&list, &q);
    }

    k_node_t hq = { .key = hot_key };
    k_node_t *hot = k_skip_position_eq_k(&list, &hq);
    *out_hot_h = (int)hot->ent.sle_height;
    *out_head_h = (int)list.slh_head->ent.sle_height;

    k_skip_free_k(&list);
}

int
main(void)
{
    int n = 200;
    int hot_key = 100;
    int total = 20000;

    struct {
        const char *label;
        double p;
    } cases[] = {
        { "p=1.000 (pure hot)   ", 1.0 },
        { "p=0.500 (half-hot)   ", 0.5 },
        { "p=0.250 (quarter)    ", 0.25 },
        { "p=0.125 (eighth)     ", 0.125 },
        { "p=0.062 (sixteenth)  ", 0.0625 },
        { "p=0.005 (cold)       ", 0.005 },
    };

    int n_cases = sizeof(cases) / sizeof(cases[0]);

    printf("Aksenov 2020 splay-list height verification (n=%d, T=%d)\n", n, total);
    printf("Target: hot key height = K - 1 - log2(1/p) = K - 1 + log2(p)\n\n");
    printf("%-22s %5s %5s %8s %8s %8s\n",
           "case", "K", "h", "expect", "ok?", "delta");
    printf("%-22s %5s %5s %8s %8s %8s\n",
           "-------------------", "---", "--", "-----", "----", "-----");

    int all_ok = 1;
    for (int i = 0; i < n_cases; i++) {
        int hot_h = 0, head_h = 0;
        run(n, hot_key, cases[i].p, total, &hot_h, &head_h);
        /* Theoretical target: log2(p) is negative for p < 1, so we
         * compute K - 1 + log2(p) (clamped to >= 0). */
        double log_p;
        if (cases[i].p >= 1.0)
            log_p = 0.0;
        else {
            double v = cases[i].p;
            int k = 0;
            while (v < 1.0) {
                v *= 2.0;
                k--;
            }
            log_p = (double)k;
        }
        int expect = (int)(head_h - 1 + log_p);
        if (expect < 0)
            expect = 0;
        /* Allow +/-1 tolerance: the rebalance fires every
         * SKIPLIST_SPLAY_INTERVAL accesses and adjusts by 1 each pass,
         * so it can take a few intervals to settle. */
        int delta = hot_h - expect;
        int ok = (delta >= -1 && delta <= 1);
        printf("%-22s %5d %5d %8d %8s %+8d\n",
               cases[i].label, head_h, hot_h, expect, ok ? "PASS" : "FAIL", delta);
        if (!ok)
            all_ok = 0;
    }

    printf("\n");
    if (all_ok) {
        printf("OVERALL: PASS -- splay rebalance reproduces the Aksenov target.\n");
        return 0;
    } else {
        printf("OVERALL: FAIL -- one or more cases off by more than +/-1 level.\n");
        return 1;
    }
}
