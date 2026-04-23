/*
 * Skiplist benchmark suite
 *
 * Measures throughput and latency across key workloads:
 *   1. Sequential insert
 *   2. Random insert
 *   3. Sequential search (hit)
 *   4. Random search (hit)
 *   5. Search miss
 *   6. Sequential delete
 *   7. Random delete
 *   8. Mixed read-heavy (95% search / 5% insert)
 *   9. Mixed write-heavy (50% insert / 50% delete)
 *  10. Iteration (forward traversal)
 *  11. Splay adaptation (repeated access to hot keys)
 *
 * Build:  make bench
 */

#define _POSIX_C_SOURCE 199309L

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "sl.h"

/* ── Skiplist type definition ─────────────────────────────────────── */

struct bench_node {
    int key;
    long val;
    SKIPLIST_ENTRY(bench) entries;
};

SKIPLIST_DECL(bench, api_, entries,
    /* compare */ {
        (void)list;
        (void)aux;
        return (a->key > b->key) - (a->key < b->key);
    },
    /* free */ { (void)node; },
    /* update */ { (void)node; (void)value; },
    /* archive */ { dest->key = src->key; dest->val = src->val; },
    /* sizeof */ { bytes = sizeof(struct bench_node); })

SKIPLIST_DECL_ACCESS(bench, api_, key, int, val, long,
    { query.key = key; },
    { return node->val; })

/* ── PRNG (xorshift64*) ──────────────────────────────────────────── */

static uint64_t rng_state;

static void
rng_seed(uint64_t s)
{
    rng_state = s ? s : 1;
}

static uint64_t
rng_next(void)
{
    uint64_t x = rng_state;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    rng_state = x;
    return x * UINT64_C(0x2545F4914F6CDD1D);
}

/* Fisher-Yates shuffle */
static void
shuffle_array(int *arr, int n)
{
    for (int i = n - 1; i > 0; i--) {
        int j = (int)(rng_next() % (uint64_t)(i + 1));
        int tmp = arr[i];
        arr[i] = arr[j];
        arr[j] = tmp;
    }
}

/* ── Timing helpers ──────────────────────────────────────────────── */

static inline uint64_t
now_ns(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

typedef struct {
    const char *name;
    int n;
    uint64_t total_ns;
    double ops_per_sec;
    double ns_per_op;
} bench_result_t;

static void
print_result(bench_result_t *r)
{
    printf("  %-40s  %8d ops  %12.0f ops/s  %8.1f ns/op\n",
        r->name, r->n, r->ops_per_sec, r->ns_per_op);
}

/* ── Benchmark implementations ───────────────────────────────────── */

static bench_result_t
bench_sequential_insert(int n)
{
    bench_t *list = malloc(sizeof(bench_t));
    api_skip_init_bench(list);

    uint64_t start = now_ns();
    for (int i = 0; i < n; i++) {
        bench_node_t *node;
        api_skip_alloc_node_bench(&node);
        node->key = i;
        node->val = (long)i;
        api_skip_insert_bench(list, node);
    }
    uint64_t elapsed = now_ns() - start;

    api_skip_free_bench(list);
    free(list);

    bench_result_t r = { "Sequential insert", n, elapsed,
        (double)n / ((double)elapsed / 1e9),
        (double)elapsed / (double)n };
    return r;
}

static bench_result_t
bench_random_insert(int n, int *keys)
{
    bench_t *list = malloc(sizeof(bench_t));
    api_skip_init_bench(list);

    uint64_t start = now_ns();
    for (int i = 0; i < n; i++) {
        bench_node_t *node;
        api_skip_alloc_node_bench(&node);
        node->key = keys[i];
        node->val = (long)keys[i];
        api_skip_insert_bench(list, node);
    }
    uint64_t elapsed = now_ns() - start;

    api_skip_free_bench(list);
    free(list);

    bench_result_t r = { "Random insert", n, elapsed,
        (double)n / ((double)elapsed / 1e9),
        (double)elapsed / (double)n };
    return r;
}

/* Helper: build a list with n sequential keys */
static bench_t *
build_list(int n)
{
    bench_t *list = malloc(sizeof(bench_t));
    api_skip_init_bench(list);
    for (int i = 0; i < n; i++) {
        bench_node_t *node;
        api_skip_alloc_node_bench(&node);
        node->key = i;
        node->val = (long)i;
        api_skip_insert_bench(list, node);
    }
    return list;
}

static bench_result_t
bench_sequential_search(int n)
{
    bench_t *list = build_list(n);

    uint64_t start = now_ns();
    volatile long sink = 0;
    for (int i = 0; i < n; i++) {
        sink += api_skip_get_bench(list, i);
    }
    uint64_t elapsed = now_ns() - start;
    (void)sink;

    api_skip_free_bench(list);
    free(list);

    bench_result_t r = { "Sequential search (hit)", n, elapsed,
        (double)n / ((double)elapsed / 1e9),
        (double)elapsed / (double)n };
    return r;
}

static bench_result_t
bench_random_search(int n, int *keys)
{
    bench_t *list = build_list(n);

    uint64_t start = now_ns();
    volatile long sink = 0;
    for (int i = 0; i < n; i++) {
        sink += api_skip_get_bench(list, keys[i]);
    }
    uint64_t elapsed = now_ns() - start;
    (void)sink;

    api_skip_free_bench(list);
    free(list);

    bench_result_t r = { "Random search (hit)", n, elapsed,
        (double)n / ((double)elapsed / 1e9),
        (double)elapsed / (double)n };
    return r;
}

static bench_result_t
bench_search_miss(int n)
{
    bench_t *list = build_list(n);

    uint64_t start = now_ns();
    volatile int sink = 0;
    for (int i = 0; i < n; i++) {
        sink += api_skip_contains_bench(list, n + i);
    }
    uint64_t elapsed = now_ns() - start;
    (void)sink;

    api_skip_free_bench(list);
    free(list);

    bench_result_t r = { "Search miss", n, elapsed,
        (double)n / ((double)elapsed / 1e9),
        (double)elapsed / (double)n };
    return r;
}

static bench_result_t
bench_sequential_delete(int n)
{
    bench_t *list = build_list(n);

    uint64_t start = now_ns();
    for (int i = 0; i < n; i++) {
        api_skip_del_bench(list, i);
    }
    uint64_t elapsed = now_ns() - start;

    api_skip_free_bench(list);
    free(list);

    bench_result_t r = { "Sequential delete", n, elapsed,
        (double)n / ((double)elapsed / 1e9),
        (double)elapsed / (double)n };
    return r;
}

static bench_result_t
bench_random_delete(int n, int *keys)
{
    bench_t *list = build_list(n);

    uint64_t start = now_ns();
    for (int i = 0; i < n; i++) {
        api_skip_del_bench(list, keys[i]);
    }
    uint64_t elapsed = now_ns() - start;

    api_skip_free_bench(list);
    free(list);

    bench_result_t r = { "Random delete", n, elapsed,
        (double)n / ((double)elapsed / 1e9),
        (double)elapsed / (double)n };
    return r;
}

static bench_result_t
bench_mixed_read_heavy(int n)
{
    int base = n / 2;
    bench_t *list = build_list(base);
    int next_key = base;

    uint64_t start = now_ns();
    volatile long sink = 0;
    for (int i = 0; i < n; i++) {
        if (rng_next() % 100 < 95) {
            int k = (int)(rng_next() % (uint64_t)next_key);
            sink += api_skip_contains_bench(list, k);
        } else {
            bench_node_t *node;
            api_skip_alloc_node_bench(&node);
            node->key = next_key;
            node->val = (long)next_key;
            api_skip_insert_bench(list, node);
            next_key++;
        }
    }
    uint64_t elapsed = now_ns() - start;
    (void)sink;

    api_skip_free_bench(list);
    free(list);

    bench_result_t r = { "Mixed read-heavy (95/5)", n, elapsed,
        (double)n / ((double)elapsed / 1e9),
        (double)elapsed / (double)n };
    return r;
}

static bench_result_t
bench_mixed_write_heavy(int n)
{
    int base = n / 4;
    bench_t *list = build_list(base);
    int next_key = base;
    int del_key = 0;

    uint64_t start = now_ns();
    for (int i = 0; i < n; i++) {
        if (rng_next() % 2 == 0) {
            bench_node_t *node;
            api_skip_alloc_node_bench(&node);
            node->key = next_key;
            node->val = (long)next_key;
            api_skip_insert_bench(list, node);
            next_key++;
        } else {
            if (del_key < next_key) {
                api_skip_del_bench(list, del_key);
                del_key++;
            }
        }
    }
    uint64_t elapsed = now_ns() - start;

    api_skip_free_bench(list);
    free(list);

    bench_result_t r = { "Mixed write-heavy (50/50)", n, elapsed,
        (double)n / ((double)elapsed / 1e9),
        (double)elapsed / (double)n };
    return r;
}

static bench_result_t
bench_iteration(int n)
{
    bench_t *list = build_list(n);

    uint64_t start = now_ns();
    volatile long sink = 0;
    bench_node_t *node = api_skip_head_bench(list);
    while (node) {
        sink += node->val;
        node = api_skip_next_node_bench(list, node);
    }
    uint64_t elapsed = now_ns() - start;
    (void)sink;

    api_skip_free_bench(list);
    free(list);

    bench_result_t r = { "Forward iteration", n, elapsed,
        (double)n / ((double)elapsed / 1e9),
        (double)elapsed / (double)n };
    return r;
}

static bench_result_t
bench_splay_hotspot(int n)
{
    bench_t *list = build_list(n);
    int hot_keys[10];
    for (int i = 0; i < 10; i++)
        hot_keys[i] = (int)(rng_next() % (uint64_t)n);

    int ops = n;
    uint64_t start = now_ns();
    volatile long sink = 0;
    for (int i = 0; i < ops; i++) {
        sink += api_skip_get_bench(list, hot_keys[i % 10]);
    }
    uint64_t elapsed = now_ns() - start;
    (void)sink;

    api_skip_free_bench(list);
    free(list);

    bench_result_t r = { "Splay hotspot (10 hot keys)", ops, elapsed,
        (double)ops / ((double)elapsed / 1e9),
        (double)elapsed / (double)ops };
    return r;
}

/* ── Latency distribution ────────────────────────────────────────── */

static int
cmp_uint64(const void *a, const void *b)
{
    uint64_t va = *(const uint64_t *)a;
    uint64_t vb = *(const uint64_t *)b;
    return (va > vb) - (va < vb);
}

static void
bench_latency_distribution(int n, int *keys)
{
    bench_t *list = build_list(n);

    uint64_t *latencies = malloc(sizeof(uint64_t) * (size_t)n);
    if (!latencies) {
        api_skip_free_bench(list);
        free(list);
        return;
    }

    for (int i = 0; i < n; i++) {
        uint64_t t0 = now_ns();
        api_skip_contains_bench(list, keys[i]);
        uint64_t t1 = now_ns();
        latencies[i] = t1 - t0;
    }

    qsort(latencies, (size_t)n, sizeof(uint64_t), cmp_uint64);

    printf("\n  Latency distribution (random search, %d ops):\n", n);
    printf("    p50  = %6lu ns\n", (unsigned long)latencies[n / 2]);
    printf("    p90  = %6lu ns\n", (unsigned long)latencies[n * 9 / 10]);
    printf("    p95  = %6lu ns\n", (unsigned long)latencies[n * 95 / 100]);
    printf("    p99  = %6lu ns\n", (unsigned long)latencies[n * 99 / 100]);
    printf("    p999 = %6lu ns\n", (unsigned long)latencies[n * 999 / 1000]);
    printf("    max  = %6lu ns\n", (unsigned long)latencies[n - 1]);

    free(latencies);
    api_skip_free_bench(list);
    free(list);
}

/* ── Main ────────────────────────────────────────────────────────── */

int
main(int argc, char **argv)
{
    int sizes[] = { 1000, 10000, 100000, 1000000 };
    int n_sizes = (int)(sizeof(sizes) / sizeof(sizes[0]));

    if (argc > 1) {
        sizes[0] = atoi(argv[1]);
        n_sizes = 1;
    }

    rng_seed(42);

    for (int si = 0; si < n_sizes; si++) {
        int n = sizes[si];
        printf("\n======================================================================\n");
        printf("  N = %d\n", n);
        printf("======================================================================\n\n");

        int *keys = malloc(sizeof(int) * (size_t)n);
        for (int i = 0; i < n; i++)
            keys[i] = i;
        shuffle_array(keys, n);

        bench_result_t results[12];
        int ri = 0;

        results[ri++] = bench_sequential_insert(n);
        results[ri++] = bench_random_insert(n, keys);
        results[ri++] = bench_sequential_search(n);
        results[ri++] = bench_random_search(n, keys);
        results[ri++] = bench_search_miss(n);
        results[ri++] = bench_sequential_delete(n);
        results[ri++] = bench_random_delete(n, keys);
        results[ri++] = bench_mixed_read_heavy(n);
        results[ri++] = bench_mixed_write_heavy(n);
        results[ri++] = bench_iteration(n);
        results[ri++] = bench_splay_hotspot(n);

        for (int i = 0; i < ri; i++)
            print_result(&results[i]);

        bench_latency_distribution(n, keys);

        free(keys);
    }

    printf("\n");
    return 0;
}
