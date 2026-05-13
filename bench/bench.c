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
 * Concurrent benchmarks:
 *  12. Concurrent insert (2, 4, 8 threads, disjoint key ranges)
 *  13. Concurrent search (2, 4, 8 reader threads on pre-filled list)
 *  14. Mixed workload (95% read / 5% write, 2, 4, 8 threads)
 *  15. Pool allocator vs malloc (sequential insert throughput)
 *
 * Build:  make bench
 */

#define _POSIX_C_SOURCE 200112L

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "sl.h"

/* -- Skiplist type definition --------------------------------------- */

struct bench_node {
    int key;
    long val;
    SKIPLIST_ENTRY(bench) entries;
};

SKIPLIST_DECL(
    bench, api_, entries,
    /* compare */
    {
        (void)list;
        (void)aux;
        return (a->key > b->key) - (a->key < b->key);
    },
    /* free */ { (void)node; },
    /* update */
    {
        (void)node;
        (void)value;
    },
    /* archive */
    {
        dest->key = src->key;
        dest->val = src->val;
    },
    /* sizeof */ { bytes = sizeof(struct bench_node); })

SKIPLIST_DECL_ACCESS(bench, api_, key, int, val, long, { query.key = key; }, { return node->val; })

SKIPLIST_DECL_EBR(bench, api_)
SKIPLIST_DECL_POOL(bench, api_, entries, 1024)

/* -- PRNG (xorshift64*) -------------------------------------------- */

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

/* -- Timing helpers ------------------------------------------------ */

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
    printf("  %-40s  %8d ops  %12.0f ops/s  %8.1f ns/op\n", r->name, r->n, r->ops_per_sec, r->ns_per_op);
}

/* -- Benchmark implementations ------------------------------------- */

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

    bench_result_t r = { "Sequential insert", n, elapsed, (double)n / ((double)elapsed / 1e9), (double)elapsed / (double)n };
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

    bench_result_t r = { "Random insert", n, elapsed, (double)n / ((double)elapsed / 1e9), (double)elapsed / (double)n };
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

    bench_result_t r = { "Sequential search (hit)", n, elapsed, (double)n / ((double)elapsed / 1e9), (double)elapsed / (double)n };
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

    bench_result_t r = { "Random search (hit)", n, elapsed, (double)n / ((double)elapsed / 1e9), (double)elapsed / (double)n };
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

    bench_result_t r = { "Search miss", n, elapsed, (double)n / ((double)elapsed / 1e9), (double)elapsed / (double)n };
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

    bench_result_t r = { "Sequential delete", n, elapsed, (double)n / ((double)elapsed / 1e9), (double)elapsed / (double)n };
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

    bench_result_t r = { "Random delete", n, elapsed, (double)n / ((double)elapsed / 1e9), (double)elapsed / (double)n };
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

    bench_result_t r = { "Mixed read-heavy (95/5)", n, elapsed, (double)n / ((double)elapsed / 1e9), (double)elapsed / (double)n };
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

    bench_result_t r = { "Mixed write-heavy (50/50)", n, elapsed, (double)n / ((double)elapsed / 1e9), (double)elapsed / (double)n };
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

    bench_result_t r = { "Forward iteration", n, elapsed, (double)n / ((double)elapsed / 1e9), (double)elapsed / (double)n };
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

    bench_result_t r = { "Splay hotspot (10 hot keys)", ops, elapsed, (double)ops / ((double)elapsed / 1e9), (double)elapsed / (double)ops };
    return r;
}

/* -- Latency distribution ------------------------------------------ */

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

/* -- Concurrent benchmark helpers ----------------------------------- */

/* Thread-local PRNG to avoid data races on the global rng_state. */
typedef struct {
    uint64_t state;
} thread_rng_t;

static uint64_t
thread_rng_next(thread_rng_t *rng)
{
    uint64_t x = rng->state;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    rng->state = x;
    return x * UINT64_C(0x2545F4914F6CDD1D);
}

/* Shared context for concurrent insert benchmark. */
typedef struct {
    bench_t *list;
    _skip_ebr_bench_t *ebr;
    pthread_barrier_t *barrier;
    int thread_id;
    int num_threads;
    int ops_per_thread;
    uint64_t elapsed_ns;
} conc_insert_ctx_t;

static void *
conc_insert_thread(void *arg)
{
    conc_insert_ctx_t *ctx = (conc_insert_ctx_t *)arg;
    int base = ctx->thread_id * ctx->ops_per_thread;
    int ebr_tid = api_skip_ebr_register_bench(ctx->ebr);

    pthread_barrier_wait(ctx->barrier);
    uint64_t start = now_ns();

    for (int i = 0; i < ctx->ops_per_thread; i++) {
        api_skip_ebr_pin_bench(ctx->ebr, ebr_tid);
        bench_node_t *node;
        api_skip_alloc_node_bench(&node);
        node->key = base + i;
        node->val = (long)(base + i);
        api_skip_insert_bench(ctx->list, node);
        api_skip_ebr_unpin_bench(ctx->ebr, ebr_tid);
    }

    ctx->elapsed_ns = now_ns() - start;
    return NULL;
}

static bench_result_t
bench_concurrent_insert(int n, int num_threads)
{
    bench_t *list = malloc(sizeof(bench_t));
    api_skip_init_bench(list);

    _skip_ebr_bench_t ebr;
    api_skip_ebr_init_bench(&ebr);
    api_skip_ebr_attach_bench(list, &ebr);

    pthread_barrier_t barrier;
    pthread_barrier_init(&barrier, NULL, (unsigned)num_threads);

    int ops_per_thread = n / num_threads;
    int total_ops = ops_per_thread * num_threads;
    pthread_t *threads = malloc(sizeof(pthread_t) * (size_t)num_threads);
    conc_insert_ctx_t *ctxs = malloc(sizeof(conc_insert_ctx_t) * (size_t)num_threads);

    for (int i = 0; i < num_threads; i++) {
        ctxs[i] = (conc_insert_ctx_t) { .list = list,
            .ebr = &ebr,
            .barrier = &barrier,
            .thread_id = i,
            .num_threads = num_threads,
            .ops_per_thread = ops_per_thread,
            .elapsed_ns = 0 };
        pthread_create(&threads[i], NULL, conc_insert_thread, &ctxs[i]);
    }

    uint64_t wall_start = now_ns();
    for (int i = 0; i < num_threads; i++)
        pthread_join(threads[i], NULL);
    uint64_t wall_elapsed = now_ns() - wall_start;

    pthread_barrier_destroy(&barrier);
    api_skip_ebr_drain_bench(&ebr);
    api_skip_free_bench(list);
    free(list);
    free(threads);
    free(ctxs);

    char name[64];
    snprintf(name, sizeof(name), "Concurrent insert (%d threads)", num_threads);
    bench_result_t r = { "", total_ops, wall_elapsed, (double)total_ops / ((double)wall_elapsed / 1e9), (double)wall_elapsed / (double)total_ops };
    /* Copy name into static storage for printing. */
    static char name_bufs[16][64];
    static int name_idx = 0;
    int idx = name_idx++ % 16;
    snprintf(name_bufs[idx], sizeof(name_bufs[idx]), "%s", name);
    r.name = name_bufs[idx];
    return r;
}

/* Shared context for concurrent search benchmark. */
typedef struct {
    bench_t *list;
    _skip_ebr_bench_t *ebr;
    pthread_barrier_t *barrier;
    int ops_per_thread;
    int max_key;
    uint64_t elapsed_ns;
} conc_search_ctx_t;

static void *
conc_search_thread(void *arg)
{
    conc_search_ctx_t *ctx = (conc_search_ctx_t *)arg;
    thread_rng_t rng = { .state = (uint64_t)(uintptr_t)arg ^ 0xdeadbeefULL };
    int ebr_tid = api_skip_ebr_register_bench(ctx->ebr);
    volatile long sink = 0;

    pthread_barrier_wait(ctx->barrier);
    uint64_t start = now_ns();

    for (int i = 0; i < ctx->ops_per_thread; i++) {
        int k = (int)(thread_rng_next(&rng) % (uint64_t)ctx->max_key);
        api_skip_ebr_pin_bench(ctx->ebr, ebr_tid);
        sink += api_skip_contains_bench(ctx->list, k);
        api_skip_ebr_unpin_bench(ctx->ebr, ebr_tid);
    }

    (void)sink;
    ctx->elapsed_ns = now_ns() - start;
    return NULL;
}

static bench_result_t
bench_concurrent_search(int n, int num_threads)
{
    bench_t *list = build_list(n);

    _skip_ebr_bench_t ebr;
    api_skip_ebr_init_bench(&ebr);
    api_skip_ebr_attach_bench(list, &ebr);

    pthread_barrier_t barrier;
    pthread_barrier_init(&barrier, NULL, (unsigned)num_threads);

    int ops_per_thread = n / num_threads;
    int total_ops = ops_per_thread * num_threads;
    pthread_t *threads = malloc(sizeof(pthread_t) * (size_t)num_threads);
    conc_search_ctx_t *ctxs = malloc(sizeof(conc_search_ctx_t) * (size_t)num_threads);

    for (int i = 0; i < num_threads; i++) {
        ctxs[i] = (conc_search_ctx_t) { .list = list, .ebr = &ebr, .barrier = &barrier, .ops_per_thread = ops_per_thread, .max_key = n, .elapsed_ns = 0 };
        pthread_create(&threads[i], NULL, conc_search_thread, &ctxs[i]);
    }

    uint64_t wall_start = now_ns();
    for (int i = 0; i < num_threads; i++)
        pthread_join(threads[i], NULL);
    uint64_t wall_elapsed = now_ns() - wall_start;

    pthread_barrier_destroy(&barrier);
    api_skip_ebr_drain_bench(&ebr);
    api_skip_free_bench(list);
    free(list);
    free(threads);
    free(ctxs);

    static char name_bufs[16][64];
    static int name_idx = 0;
    int idx = name_idx++ % 16;
    snprintf(name_bufs[idx], sizeof(name_bufs[idx]), "Concurrent search (%d threads)", num_threads);
    bench_result_t r = { name_bufs[idx], total_ops, wall_elapsed, (double)total_ops / ((double)wall_elapsed / 1e9), (double)wall_elapsed / (double)total_ops };
    return r;
}

/* Shared context for mixed workload benchmark. */
typedef struct {
    bench_t *list;
    _skip_ebr_bench_t *ebr;
    pthread_barrier_t *barrier;
    int thread_id;
    int ops_per_thread;
    int max_key; /* max key in the pre-filled list */
    uint64_t elapsed_ns;
} conc_mixed_ctx_t;

static void *
conc_mixed_thread(void *arg)
{
    conc_mixed_ctx_t *ctx = (conc_mixed_ctx_t *)arg;
    thread_rng_t rng = { .state = (uint64_t)(ctx->thread_id + 1) * 6364136223846793005ULL };
    int ebr_tid = api_skip_ebr_register_bench(ctx->ebr);
    volatile long sink = 0;
    /* Each thread inserts into a unique range to avoid duplicates. */
    int insert_base = ctx->max_key + ctx->thread_id * ctx->ops_per_thread;
    int insert_next = insert_base;

    pthread_barrier_wait(ctx->barrier);
    uint64_t start = now_ns();

    for (int i = 0; i < ctx->ops_per_thread; i++) {
        api_skip_ebr_pin_bench(ctx->ebr, ebr_tid);
        if (thread_rng_next(&rng) % 100 < 95) {
            /* Read: search for a random key in the pre-filled range. */
            int k = (int)(thread_rng_next(&rng) % (uint64_t)ctx->max_key);
            sink += api_skip_contains_bench(ctx->list, k);
        } else {
            /* Write: insert a new unique key. */
            bench_node_t *node;
            api_skip_alloc_node_bench(&node);
            node->key = insert_next++;
            node->val = (long)node->key;
            api_skip_insert_bench(ctx->list, node);
        }
        api_skip_ebr_unpin_bench(ctx->ebr, ebr_tid);
    }

    (void)sink;
    ctx->elapsed_ns = now_ns() - start;
    return NULL;
}

static bench_result_t
bench_concurrent_mixed(int n, int num_threads)
{
    /* Pre-fill with n/2 keys, then run n total ops across threads. */
    int base = n / 2;
    bench_t *list = build_list(base);

    _skip_ebr_bench_t ebr;
    api_skip_ebr_init_bench(&ebr);
    api_skip_ebr_attach_bench(list, &ebr);

    pthread_barrier_t barrier;
    pthread_barrier_init(&barrier, NULL, (unsigned)num_threads);

    int ops_per_thread = n / num_threads;
    int total_ops = ops_per_thread * num_threads;
    pthread_t *threads = malloc(sizeof(pthread_t) * (size_t)num_threads);
    conc_mixed_ctx_t *ctxs = malloc(sizeof(conc_mixed_ctx_t) * (size_t)num_threads);

    for (int i = 0; i < num_threads; i++) {
        ctxs[i] = (conc_mixed_ctx_t) { .list = list,
            .ebr = &ebr,
            .barrier = &barrier,
            .thread_id = i,
            .ops_per_thread = ops_per_thread,
            .max_key = base,
            .elapsed_ns = 0 };
        pthread_create(&threads[i], NULL, conc_mixed_thread, &ctxs[i]);
    }

    uint64_t wall_start = now_ns();
    for (int i = 0; i < num_threads; i++)
        pthread_join(threads[i], NULL);
    uint64_t wall_elapsed = now_ns() - wall_start;

    pthread_barrier_destroy(&barrier);
    api_skip_ebr_drain_bench(&ebr);
    api_skip_free_bench(list);
    free(list);
    free(threads);
    free(ctxs);

    static char name_bufs[16][64];
    static int name_idx = 0;
    int idx = name_idx++ % 16;
    snprintf(name_bufs[idx], sizeof(name_bufs[idx]), "Concurrent mixed 95/5 (%d threads)", num_threads);
    bench_result_t r = { name_bufs[idx], total_ops, wall_elapsed, (double)total_ops / ((double)wall_elapsed / 1e9), (double)wall_elapsed / (double)total_ops };
    return r;
}

/* Pool allocator vs malloc comparison (sequential). */
static bench_result_t
bench_pool_insert(int n)
{
    bench_t *list = malloc(sizeof(bench_t));
    api_skip_init_bench(list);

    _skip_pool_bench_t pool;
    api_skip_pool_init_bench(&pool, (size_t)n);

    uint64_t start = now_ns();
    for (int i = 0; i < n; i++) {
        bench_node_t *node;
        api_skip_pool_alloc_node_bench(&pool, &node);
        node->key = i;
        node->val = (long)i;
        api_skip_insert_bench(list, node);
    }
    uint64_t elapsed = now_ns() - start;

    /* Free head/tail (malloc'd by init); pool slab has all data nodes. */
    free(list->slh_head);
    free(list->slh_tail);
    api_skip_pool_destroy_bench(&pool);
    free(list);

    bench_result_t r = { "Pool insert (sequential)", n, elapsed, (double)n / ((double)elapsed / 1e9), (double)elapsed / (double)n };
    return r;
}

static void
run_concurrent_benchmarks(int n)
{
    int thread_counts[] = { 2, 4, 8 };
    int n_tc = (int)(sizeof(thread_counts) / sizeof(thread_counts[0]));

    printf("\n  -- Concurrent insert --\n");
    for (int i = 0; i < n_tc; i++) {
        bench_result_t r = bench_concurrent_insert(n, thread_counts[i]);
        print_result(&r);
    }

    printf("\n  -- Concurrent search --\n");
    for (int i = 0; i < n_tc; i++) {
        bench_result_t r = bench_concurrent_search(n, thread_counts[i]);
        print_result(&r);
    }

    printf("\n  -- Concurrent mixed (95%% read / 5%% write) --\n");
    for (int i = 0; i < n_tc; i++) {
        bench_result_t r = bench_concurrent_mixed(n, thread_counts[i]);
        print_result(&r);
    }

    printf("\n  -- Pool allocator vs malloc --\n");
    bench_result_t r_malloc = bench_sequential_insert(n);
    bench_result_t r_pool = bench_pool_insert(n);
    /* Re-label the malloc result for comparison. */
    r_malloc.name = "Malloc insert (sequential)";
    print_result(&r_malloc);
    print_result(&r_pool);
}

/* -- Main ---------------------------------------------------------- */

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

        printf("\n  --- Concurrent & Pool Benchmarks ---\n");
        run_concurrent_benchmarks(n);

        free(keys);
    }

    printf("\n");
    return 0;
}
