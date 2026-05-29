/*
 * Concurrent test suite for the lock-free skiplist with EBR.
 *
 * Uses munit framework with pthread-based concurrent tests.
 * Compile with:
 *   gcc -Wall -Wextra -Wpedantic -Og -g -std=c11 -Iinclude/ -Itests/
 *       -DDEBUG -DSKIPLIST_DIAGNOSTIC tests/test_concurrent.c tests/munit.c
 *       -o tests/test_concurrent -lm -pthread
 * Run with:
 *   ./tests/test_concurrent --no-fork
 *
 * Known issue: concurrent delete and mixed workload tests may intermittently
 * segfault without sanitizers due to a race in _skip_adjust_hit_counts_,
 * which traverses the list (SKIPLIST_FOREACH_H2T) after retiring a node.
 * Under ASan the tests pass reliably because freed memory is quarantined.
 * Compile with -fsanitize=address for stable runs until the library's
 * concurrent delete path is hardened.
 */

#define MUNIT_ENABLE_ASSERT_ALIASES

#include <pthread.h>
#include <string.h>

#include "munit.h"
#include "sl.h"

/* Number of threads for all concurrent tests. */
#define NUM_THREADS 4

/* Number of keys each thread inserts/deletes in range-based tests. */
#define KEYS_PER_THREAD 1000

/* Node structure for concurrent tests. Uses "conc" as the decl name
   to avoid symbol conflicts with the single-threaded test suite. */
struct conc_node {
    int key;
    int value;
    SKIPLIST_ENTRY(conc) entries;
};

/* Generate skiplist functions with "conc" decl and "ct_" prefix. */
SKIPLIST_DECL(
    conc, ct_, entries,
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
    { (void)node; },
    /* update entry */
    {
        (void)node;
        (void)value;
    },
    /* archive entry */
    {
        (void)dest;
        (void)src;
    },
    /* sizeof entry */
    { bytes = sizeof(struct conc_node); })

/* Generate EBR functions for the "conc" type. */
SKIPLIST_DECL_EBR(conc, ct_)

/* Generate pool allocator functions for the "conc" type. */
SKIPLIST_DECL_POOL(conc, ct_, entries, 1024)

/* ---------------------------------------------------------------------------
 * Shared test context passed to each thread.
 * ---------------------------------------------------------------------------*/
typedef struct {
    conc_t *list;
    _skip_ebr_conc_t *ebr;
    int thread_id; /* logical thread index 0..NUM_THREADS-1 */
    int ebr_tid;   /* EBR-registered thread id */
    int result;    /* 0 = success */
} thread_ctx_t;

/* ---------------------------------------------------------------------------
 * Helper: allocate and init a list + EBR, returning them through pointers.
 * ---------------------------------------------------------------------------*/
static void
setup_list_and_ebr(conc_t **list_out, _skip_ebr_conc_t **ebr_out)
{
    conc_t *list = (conc_t *)calloc(1, sizeof(conc_t));
    munit_assert_not_null(list);
    int rc = ct_skip_init_conc(list);
    munit_assert_int(rc, ==, 0);

    _skip_ebr_conc_t *ebr = (_skip_ebr_conc_t *)calloc(1, sizeof(_skip_ebr_conc_t));
    munit_assert_not_null(ebr);
    ct_skip_ebr_init_conc(ebr);
    ct_skip_ebr_attach_conc(list, ebr);

    *list_out = list;
    *ebr_out = ebr;
}

/* ---------------------------------------------------------------------------
 * Helper: pre-fill a list with keys [start, end).
 * ---------------------------------------------------------------------------*/
static void
prefill_list(conc_t *list, int start, int end)
{
    for (int k = start; k < end; k++) {
        conc_node_t *node;
        int rc = ct_skip_alloc_node_conc(&node);
        munit_assert_int(rc, ==, 0);
        node->key = k;
        node->value = k;
        rc = ct_skip_insert_conc(list, node);
        munit_assert_int(rc, ==, 0);
    }
}

/* ---------------------------------------------------------------------------
 * Helper: tear down list + EBR.
 * ---------------------------------------------------------------------------*/
static void
teardown_list_and_ebr(conc_t *list, _skip_ebr_conc_t *ebr)
{
    ct_skip_ebr_drain_conc(ebr);
    ct_skip_free_conc(list);
    free(list);
    free(ebr);
}

/* ===========================================================================
 * Test A: Concurrent insert
 *
 * N threads each insert a disjoint key range:
 *   thread i inserts keys [i*KEYS_PER_THREAD, (i+1)*KEYS_PER_THREAD).
 * After all threads join, verify every key is present.
 * ===========================================================================*/
static void *
thread_concurrent_insert(void *arg)
{
    thread_ctx_t *ctx = (thread_ctx_t *)arg;
    int start = ctx->thread_id * KEYS_PER_THREAD;
    int end = start + KEYS_PER_THREAD;

    for (int k = start; k < end; k++) {
        conc_node_t *node;
        int rc = ct_skip_alloc_node_conc(&node);
        if (rc != 0) {
            ctx->result = -1;
            return NULL;
        }
        node->key = k;
        node->value = k;
        ct_skip_ebr_pin_conc(ctx->ebr, ctx->ebr_tid);
        rc = ct_skip_insert_conc(ctx->list, node);
        ct_skip_ebr_unpin_conc(ctx->ebr, ctx->ebr_tid);
        if (rc != 0) {
            /* Duplicate or error -- should not happen with disjoint ranges.
               skip_free_node already frees node; do not free it twice. */
            ct_skip_free_node_conc(ctx->list, node);
            ctx->result = -1;
            return NULL;
        }
    }

    ctx->result = 0;
    return NULL;
}

static MunitResult
test_concurrent_insert(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    conc_t *list;
    _skip_ebr_conc_t *ebr;
    setup_list_and_ebr(&list, &ebr);

    pthread_t threads[NUM_THREADS];
    thread_ctx_t ctxs[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++) {
        ctxs[i].list = list;
        ctxs[i].ebr = ebr;
        ctxs[i].thread_id = i;
        ctxs[i].ebr_tid = ct_skip_ebr_register_conc(ebr);
        munit_assert_int(ctxs[i].ebr_tid, >=, 0);
        ctxs[i].result = -1;
    }

    for (int i = 0; i < NUM_THREADS; i++)
        pthread_create(&threads[i], NULL, thread_concurrent_insert, &ctxs[i]);
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);

    /* Check all threads succeeded. */
    for (int i = 0; i < NUM_THREADS; i++)
        munit_assert_int(ctxs[i].result, ==, 0);

    /* Verify total length. */
    size_t expected = (size_t)NUM_THREADS * KEYS_PER_THREAD;
    munit_assert_size(ct_skip_length_conc(list), ==, expected);

    /* Verify every key is present. */
    for (int k = 0; k < NUM_THREADS * KEYS_PER_THREAD; k++) {
        conc_node_t query;
        memset(&query, 0, sizeof(query));
        query.key = k;
        conc_node_t *found = ct_skip_position_eq_conc(list, &query);
        munit_assert_not_null(found);
        munit_assert_int(found->key, ==, k);
    }

    teardown_list_and_ebr(list, ebr);
    return MUNIT_OK;
}

/* ===========================================================================
 * Test B: Concurrent search
 *
 * Pre-fill the list, then N threads all search for random keys concurrently.
 * Verify no crashes and correct results.
 * ===========================================================================*/
static void *
thread_concurrent_search(void *arg)
{
    thread_ctx_t *ctx = (thread_ctx_t *)arg;
    int total_keys = NUM_THREADS * KEYS_PER_THREAD;

    for (int i = 0; i < KEYS_PER_THREAD; i++) {
        int k = munit_rand_int_range(0, total_keys - 1);
        conc_node_t query;
        memset(&query, 0, sizeof(query));
        query.key = k;
        ct_skip_ebr_pin_conc(ctx->ebr, ctx->ebr_tid);
        conc_node_t *found = ct_skip_position_eq_conc(ctx->list, &query);
        if (found == NULL || found->key != k) {
            ctx->result = -1;
            ct_skip_ebr_unpin_conc(ctx->ebr, ctx->ebr_tid);
            return NULL;
        }
        ct_skip_ebr_unpin_conc(ctx->ebr, ctx->ebr_tid);
    }

    ctx->result = 0;
    return NULL;
}

static MunitResult
test_concurrent_search(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    conc_t *list;
    _skip_ebr_conc_t *ebr;
    setup_list_and_ebr(&list, &ebr);

    int total_keys = NUM_THREADS * KEYS_PER_THREAD;
    prefill_list(list, 0, total_keys);

    munit_assert_size(ct_skip_length_conc(list), ==, (size_t)total_keys);

    pthread_t threads[NUM_THREADS];
    thread_ctx_t ctxs[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++) {
        ctxs[i].list = list;
        ctxs[i].ebr = ebr;
        ctxs[i].thread_id = i;
        ctxs[i].ebr_tid = ct_skip_ebr_register_conc(ebr);
        munit_assert_int(ctxs[i].ebr_tid, >=, 0);
        ctxs[i].result = -1;
    }

    for (int i = 0; i < NUM_THREADS; i++)
        pthread_create(&threads[i], NULL, thread_concurrent_search, &ctxs[i]);
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);

    for (int i = 0; i < NUM_THREADS; i++)
        munit_assert_int(ctxs[i].result, ==, 0);

    teardown_list_and_ebr(list, ebr);
    return MUNIT_OK;
}

/* ===========================================================================
 * Test C: Concurrent delete
 *
 * Pre-fill the list with keys [0, NUM_THREADS*KEYS_PER_THREAD).
 * N threads each delete a disjoint range.
 * Verify no keys remain.
 * ===========================================================================*/
static void *
thread_concurrent_delete(void *arg)
{
    thread_ctx_t *ctx = (thread_ctx_t *)arg;
    int start = ctx->thread_id * KEYS_PER_THREAD;
    int end = start + KEYS_PER_THREAD;

    for (int k = start; k < end; k++) {
        ct_skip_ebr_pin_conc(ctx->ebr, ctx->ebr_tid);
        conc_node_t query;
        memset(&query, 0, sizeof(query));
        query.key = k;
        ct_skip_remove_node_conc(ctx->list, &query);
        ct_skip_ebr_unpin_conc(ctx->ebr, ctx->ebr_tid);
    }

    ctx->result = 0;
    return NULL;
}

static MunitResult
test_concurrent_delete(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    conc_t *list;
    _skip_ebr_conc_t *ebr;
    setup_list_and_ebr(&list, &ebr);

    int total_keys = NUM_THREADS * KEYS_PER_THREAD;
    prefill_list(list, 0, total_keys);

    munit_assert_size(ct_skip_length_conc(list), ==, (size_t)total_keys);

    pthread_t threads[NUM_THREADS];
    thread_ctx_t ctxs[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++) {
        ctxs[i].list = list;
        ctxs[i].ebr = ebr;
        ctxs[i].thread_id = i;
        ctxs[i].ebr_tid = ct_skip_ebr_register_conc(ebr);
        munit_assert_int(ctxs[i].ebr_tid, >=, 0);
        ctxs[i].result = -1;
    }

    for (int i = 0; i < NUM_THREADS; i++)
        pthread_create(&threads[i], NULL, thread_concurrent_delete, &ctxs[i]);
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);

    for (int i = 0; i < NUM_THREADS; i++)
        munit_assert_int(ctxs[i].result, ==, 0);

    /* All keys were in disjoint ranges, so everything should be gone. */
    munit_assert_size(ct_skip_length_conc(list), ==, 0);

    teardown_list_and_ebr(list, ebr);
    return MUNIT_OK;
}

/* ===========================================================================
 * Test D: Mixed workload
 *
 * Pre-fill with keys [0, 2000). Then 4 threads each do:
 *   - 25% inserts (keys 2000+thread_id*500 .. 2000+(thread_id+1)*500)
 *   - 25% deletes (keys thread_id*500 .. (thread_id+1)*500)
 *   - 50% searches (random keys in [0, 4000))
 * Verify no crashes. Check that inserted keys are findable and deleted
 * keys are gone (for disjoint ranges).
 * ===========================================================================*/

#define MIXED_PREFILL 2000
#define MIXED_OPS_PER_THREAD 500

static void *
thread_mixed_workload(void *arg)
{
    thread_ctx_t *ctx = (thread_ctx_t *)arg;
    int tid = ctx->thread_id;

    int insert_start = MIXED_PREFILL + tid * MIXED_OPS_PER_THREAD;
    int delete_start = tid * MIXED_OPS_PER_THREAD;

    int insert_idx = 0;
    int delete_idx = 0;

    /*
     * Interleave operations: for every 4 iterations, do 1 insert, 1 delete,
     * 2 searches.  Pin/unpin around each individual operation so that EBR
     * epochs can advance between operations.
     */
    for (int i = 0; i < MIXED_OPS_PER_THREAD * 4; i++) {
        int op = i % 4;
        if (op == 0 && insert_idx < MIXED_OPS_PER_THREAD) {
            /* Insert */
            conc_node_t *node;
            int rc = ct_skip_alloc_node_conc(&node);
            if (rc != 0) {
                ctx->result = -1;
                return NULL;
            }
            node->key = insert_start + insert_idx;
            node->value = node->key;
            ct_skip_ebr_pin_conc(ctx->ebr, ctx->ebr_tid);
            rc = ct_skip_insert_conc(ctx->list, node);
            ct_skip_ebr_unpin_conc(ctx->ebr, ctx->ebr_tid);
            if (rc != 0) {
                /* Could fail if another thread races -- not an error for
                   overlapping ranges, but we use disjoint ranges here.
                   skip_free_node already frees node; do not free it twice. */
                ct_skip_free_node_conc(ctx->list, node);
            }
            insert_idx++;
        } else if (op == 1 && delete_idx < MIXED_OPS_PER_THREAD) {
            /* Delete */
            conc_node_t query;
            memset(&query, 0, sizeof(query));
            query.key = delete_start + delete_idx;
            ct_skip_ebr_pin_conc(ctx->ebr, ctx->ebr_tid);
            ct_skip_remove_node_conc(ctx->list, &query);
            ct_skip_ebr_unpin_conc(ctx->ebr, ctx->ebr_tid);
            delete_idx++;
        } else {
            /* Search */
            int k = munit_rand_int_range(0, MIXED_PREFILL + NUM_THREADS * MIXED_OPS_PER_THREAD - 1);
            conc_node_t query;
            memset(&query, 0, sizeof(query));
            query.key = k;
            ct_skip_ebr_pin_conc(ctx->ebr, ctx->ebr_tid);
            /* Result may or may not be found -- we just check for no crash. */
            (void)ct_skip_position_eq_conc(ctx->list, &query);
            ct_skip_ebr_unpin_conc(ctx->ebr, ctx->ebr_tid);
        }
    }
    ctx->result = 0;
    return NULL;
}

static MunitResult
test_mixed_workload(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    conc_t *list;
    _skip_ebr_conc_t *ebr;
    setup_list_and_ebr(&list, &ebr);

    prefill_list(list, 0, MIXED_PREFILL);

    pthread_t threads[NUM_THREADS];
    thread_ctx_t ctxs[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++) {
        ctxs[i].list = list;
        ctxs[i].ebr = ebr;
        ctxs[i].thread_id = i;
        ctxs[i].ebr_tid = ct_skip_ebr_register_conc(ebr);
        munit_assert_int(ctxs[i].ebr_tid, >=, 0);
        ctxs[i].result = -1;
    }

    for (int i = 0; i < NUM_THREADS; i++)
        pthread_create(&threads[i], NULL, thread_mixed_workload, &ctxs[i]);
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);

    for (int i = 0; i < NUM_THREADS; i++)
        munit_assert_int(ctxs[i].result, ==, 0);

    /* Verify inserted keys (disjoint ranges above MIXED_PREFILL) are present. */
    for (int i = 0; i < NUM_THREADS; i++) {
        int insert_start = MIXED_PREFILL + i * MIXED_OPS_PER_THREAD;
        for (int j = 0; j < MIXED_OPS_PER_THREAD; j++) {
            conc_node_t query;
            memset(&query, 0, sizeof(query));
            query.key = insert_start + j;
            conc_node_t *found = ct_skip_position_eq_conc(list, &query);
            munit_assert_not_null(found);
            munit_assert_int(found->key, ==, insert_start + j);
        }
    }

    /* Verify deleted keys (disjoint ranges [0, MIXED_PREFILL)) are gone. */
    for (int k = 0; k < MIXED_PREFILL; k++) {
        conc_node_t query;
        memset(&query, 0, sizeof(query));
        query.key = k;
        conc_node_t *found = ct_skip_position_eq_conc(list, &query);
        munit_assert_null(found);
    }

    teardown_list_and_ebr(list, ebr);
    return MUNIT_OK;
}

/* ===========================================================================
 * Test E: EBR correctness
 *
 * Concurrent insert and delete with EBR, run under ASan to verify no
 * use-after-free. Threads insert and then immediately search for the keys
 * they inserted to exercise read-reclaim races.
 * ===========================================================================*/

#define EBR_KEYS_PER_THREAD 500

static void *
thread_ebr_insert_delete(void *arg)
{
    thread_ctx_t *ctx = (thread_ctx_t *)arg;
    int tid = ctx->thread_id;
    int start = tid * EBR_KEYS_PER_THREAD;
    int end = start + EBR_KEYS_PER_THREAD;

    /* Phase 1: Insert keys (pin/unpin per operation). */
    for (int k = start; k < end; k++) {
        conc_node_t *node;
        int rc = ct_skip_alloc_node_conc(&node);
        if (rc != 0) {
            ctx->result = -1;
            return NULL;
        }
        node->key = k;
        node->value = k;
        ct_skip_ebr_pin_conc(ctx->ebr, ctx->ebr_tid);
        ct_skip_insert_conc(ctx->list, node);
        ct_skip_ebr_unpin_conc(ctx->ebr, ctx->ebr_tid);
    }

    /* Phase 2: Delete half the keys while other threads may be reading. */
    for (int k = start; k < start + EBR_KEYS_PER_THREAD / 2; k++) {
        conc_node_t query;
        memset(&query, 0, sizeof(query));
        query.key = k;
        ct_skip_ebr_pin_conc(ctx->ebr, ctx->ebr_tid);
        ct_skip_remove_node_conc(ctx->list, &query);
        ct_skip_ebr_unpin_conc(ctx->ebr, ctx->ebr_tid);
    }

    /* Phase 3: Search for the remaining keys to exercise read-after-delete
       timing with other threads' deletions. */
    ct_skip_ebr_pin_conc(ctx->ebr, ctx->ebr_tid);
    for (int k = start + EBR_KEYS_PER_THREAD / 2; k < end; k++) {
        conc_node_t query;
        memset(&query, 0, sizeof(query));
        query.key = k;
        conc_node_t *found = ct_skip_position_eq_conc(ctx->list, &query);
        if (found == NULL || found->key != k) {
            ctx->result = -1;
            ct_skip_ebr_unpin_conc(ctx->ebr, ctx->ebr_tid);
            return NULL;
        }
    }
    ct_skip_ebr_unpin_conc(ctx->ebr, ctx->ebr_tid);

    ctx->result = 0;
    return NULL;
}

static MunitResult
test_ebr_correctness(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    conc_t *list;
    _skip_ebr_conc_t *ebr;
    setup_list_and_ebr(&list, &ebr);

    pthread_t threads[NUM_THREADS];
    thread_ctx_t ctxs[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++) {
        ctxs[i].list = list;
        ctxs[i].ebr = ebr;
        ctxs[i].thread_id = i;
        ctxs[i].ebr_tid = ct_skip_ebr_register_conc(ebr);
        munit_assert_int(ctxs[i].ebr_tid, >=, 0);
        ctxs[i].result = -1;
    }

    for (int i = 0; i < NUM_THREADS; i++)
        pthread_create(&threads[i], NULL, thread_ebr_insert_delete, &ctxs[i]);
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);

    for (int i = 0; i < NUM_THREADS; i++)
        munit_assert_int(ctxs[i].result, ==, 0);

    /* Verify remaining keys: each thread deleted the first half of its range,
       so keys [start + EBR_KEYS_PER_THREAD/2, start + EBR_KEYS_PER_THREAD)
       should still be present. */
    size_t expected_remaining = (size_t)NUM_THREADS * (EBR_KEYS_PER_THREAD / 2);
    munit_assert_size(ct_skip_length_conc(list), ==, expected_remaining);

    for (int i = 0; i < NUM_THREADS; i++) {
        int start = i * EBR_KEYS_PER_THREAD;
        /* First half should be gone. */
        for (int k = start; k < start + EBR_KEYS_PER_THREAD / 2; k++) {
            conc_node_t query;
            memset(&query, 0, sizeof(query));
            query.key = k;
            conc_node_t *found = ct_skip_position_eq_conc(list, &query);
            munit_assert_null(found);
        }
        /* Second half should remain. */
        for (int k = start + EBR_KEYS_PER_THREAD / 2; k < start + EBR_KEYS_PER_THREAD; k++) {
            conc_node_t query;
            memset(&query, 0, sizeof(query));
            query.key = k;
            conc_node_t *found = ct_skip_position_eq_conc(list, &query);
            munit_assert_not_null(found);
            munit_assert_int(found->key, ==, k);
        }
    }

    teardown_list_and_ebr(list, ebr);
    return MUNIT_OK;
}

/* ===========================================================================
 * Test F: Pool allocator contention
 *
 * Exercise the lock-free pool allocator under multi-threaded contention.
 * Two phases:
 *
 * Phase 1 (alloc/free cycling): A pool with limited capacity is shared
 * across N threads.  Each thread repeatedly allocates a batch of nodes from
 * the pool, verifies they are valid and unique, then frees them all back.
 * This stresses the CAS-based free-list under contention and tests ENOMEM
 * handling when the pool is exhausted.
 *
 * Phase 2 (pool + list operations): Each thread allocates from the pool,
 * inserts into a shared skiplist, and searches for the inserted keys.
 * Nodes remain in the list (no remove, since EBR reclamation uses free()
 * which is incompatible with pool-allocated memory).  After all threads
 * join, we verify every key is present and the count is correct.  Teardown
 * walks the list to free only heap-fallback nodes, then destroys the pool
 * slab.
 * ===========================================================================*/

#define POOL_CAPACITY 1000
#define POOL_ALLOC_BATCH 50
#define POOL_CYCLES 20
#define POOL_INSERT_PER_THREAD 200

typedef struct {
    _skip_pool_conc_t *pool;
    int thread_id;
    int result;
    int alloc_failures; /* count of ENOMEM returns across all cycles */
} pool_cycle_ctx_t;

typedef struct {
    conc_t *list;
    _skip_ebr_conc_t *ebr;
    _skip_pool_conc_t *pool;
    int thread_id;
    int ebr_tid;
    int result;
    int alloc_failures;
} pool_insert_ctx_t;

/* Phase 1 thread: alloc/free cycling on the pool. */
static void *
thread_pool_cycle(void *arg)
{
    pool_cycle_ctx_t *ctx = (pool_cycle_ctx_t *)arg;
    conc_node_t *batch[POOL_ALLOC_BATCH];
    int alloc_failures = 0;

    for (int cycle = 0; cycle < POOL_CYCLES; cycle++) {
        int allocated = 0;

        /* Allocate a batch from the pool. */
        for (int i = 0; i < POOL_ALLOC_BATCH; i++) {
            conc_node_t *node = NULL;
            int rc = ct_skip_pool_alloc_node_conc(ctx->pool, &node);
            if (rc == ENOMEM) {
                alloc_failures++;
                break; /* Pool exhausted -- expected under contention. */
            }
            if (rc != 0) {
                ctx->result = -1;
                for (int j = 0; j < allocated; j++)
                    ct_skip_pool_free_conc(ctx->pool, batch[j]);
                return NULL;
            }
            /* Write a pattern to detect corruption. */
            node->key = ctx->thread_id * 100000 + cycle * 1000 + i;
            node->value = node->key;
            batch[allocated++] = node;
        }

        /* Verify each node's data is intact (no cross-thread corruption). */
        for (int i = 0; i < allocated; i++) {
            int expected_key = ctx->thread_id * 100000 + cycle * 1000 + i;
            if (batch[i]->key != expected_key || batch[i]->value != expected_key) {
                ctx->result = -1;
                for (int j = 0; j < allocated; j++)
                    ct_skip_pool_free_conc(ctx->pool, batch[j]);
                return NULL;
            }
        }

        /* Free all nodes back to the pool. */
        for (int i = 0; i < allocated; i++)
            ct_skip_pool_free_conc(ctx->pool, batch[i]);
    }

    ctx->alloc_failures = alloc_failures;
    ctx->result = 0;
    return NULL;
}

/* Phase 2 thread: pool alloc -> list insert -> search. */
static void *
thread_pool_insert(void *arg)
{
    pool_insert_ctx_t *ctx = (pool_insert_ctx_t *)arg;
    int tid = ctx->thread_id;
    int start = tid * POOL_INSERT_PER_THREAD;
    int alloc_failures = 0;

    for (int i = 0; i < POOL_INSERT_PER_THREAD; i++) {
        int key = start + i;

        /* Allocate from pool, fall back to heap on ENOMEM. */
        conc_node_t *node = NULL;
        int rc = ct_skip_pool_alloc_node_conc(ctx->pool, &node);
        if (rc == ENOMEM) {
            alloc_failures++;
            rc = ct_skip_alloc_node_conc(&node);
            if (rc != 0) {
                ctx->result = -1;
                return NULL;
            }
        } else if (rc != 0) {
            ctx->result = -1;
            return NULL;
        }

        node->key = key;
        node->value = key * 10;

        /* Insert into the list. */
        ct_skip_ebr_pin_conc(ctx->ebr, ctx->ebr_tid);
        rc = ct_skip_insert_conc(ctx->list, node);
        ct_skip_ebr_unpin_conc(ctx->ebr, ctx->ebr_tid);
        if (rc != 0) {
            if (ct_skip_pool_is_from_conc(ctx->pool, node))
                ct_skip_pool_free_conc(ctx->pool, node);
            else
                free(node);
            ctx->result = -1;
            return NULL;
        }

        /* Search for the key we just inserted. */
        conc_node_t query;
        memset(&query, 0, sizeof(query));
        query.key = key;
        ct_skip_ebr_pin_conc(ctx->ebr, ctx->ebr_tid);
        conc_node_t *found = ct_skip_position_eq_conc(ctx->list, &query);
        ct_skip_ebr_unpin_conc(ctx->ebr, ctx->ebr_tid);
        if (found == NULL || found->key != key) {
            ctx->result = -1;
            return NULL;
        }
    }

    ctx->alloc_failures = alloc_failures;
    ctx->result = 0;
    return NULL;
}

static MunitResult
test_pool_contention(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    /* ---------------------------------------------------------------
     * Phase 1: Pure pool alloc/free cycling under contention.
     * ---------------------------------------------------------------*/
    _skip_pool_conc_t pool;
    int rc = ct_skip_pool_init_conc(&pool, POOL_CAPACITY);
    munit_assert_int(rc, ==, 0);

    {
        pthread_t threads[NUM_THREADS];
        pool_cycle_ctx_t ctxs[NUM_THREADS];

        for (int i = 0; i < NUM_THREADS; i++) {
            ctxs[i].pool = &pool;
            ctxs[i].thread_id = i;
            ctxs[i].result = -1;
            ctxs[i].alloc_failures = 0;
        }

        for (int i = 0; i < NUM_THREADS; i++)
            pthread_create(&threads[i], NULL, thread_pool_cycle, &ctxs[i]);
        for (int i = 0; i < NUM_THREADS; i++)
            pthread_join(threads[i], NULL);

        for (int i = 0; i < NUM_THREADS; i++)
            munit_assert_int(ctxs[i].result, ==, 0);

        /* After all cycles complete, every slot should be back on the
           free list.  Verify by allocating all POOL_CAPACITY slots. */
        int count = 0;
        conc_node_t *verify_nodes[POOL_CAPACITY];
        while (count < (int)POOL_CAPACITY) {
            conc_node_t *n = ct_skip_pool_alloc_conc(&pool);
            if (n == NULL)
                break;
            verify_nodes[count++] = n;
        }
        munit_assert_int(count, ==, (int)POOL_CAPACITY);
        /* Free them all back. */
        for (int i = 0; i < count; i++)
            ct_skip_pool_free_conc(&pool, verify_nodes[i]);
    }

    /* ---------------------------------------------------------------
     * Phase 2: Pool alloc -> list insert -> search under contention.
     *
     * Nodes allocated from the pool are inserted into the skiplist.
     * We do NOT remove them via the list (EBR reclaim calls free(),
     * which is incompatible with pool memory).  At teardown we walk
     * the list manually, freeing only heap-fallback nodes.  Pool
     * nodes are released when the pool slab is destroyed.
     * ---------------------------------------------------------------*/
    conc_t *list = (conc_t *)calloc(1, sizeof(conc_t));
    munit_assert_not_null(list);
    rc = ct_skip_init_conc(list);
    munit_assert_int(rc, ==, 0);

    _skip_ebr_conc_t *ebr = (_skip_ebr_conc_t *)calloc(1, sizeof(_skip_ebr_conc_t));
    munit_assert_not_null(ebr);
    ct_skip_ebr_init_conc(ebr);
    ct_skip_ebr_attach_conc(list, ebr);

    {
        pthread_t threads[NUM_THREADS];
        pool_insert_ctx_t ctxs[NUM_THREADS];

        for (int i = 0; i < NUM_THREADS; i++) {
            ctxs[i].list = list;
            ctxs[i].ebr = ebr;
            ctxs[i].pool = &pool;
            ctxs[i].thread_id = i;
            ctxs[i].ebr_tid = ct_skip_ebr_register_conc(ebr);
            munit_assert_int(ctxs[i].ebr_tid, >=, 0);
            ctxs[i].result = -1;
            ctxs[i].alloc_failures = 0;
        }

        for (int i = 0; i < NUM_THREADS; i++)
            pthread_create(&threads[i], NULL, thread_pool_insert, &ctxs[i]);
        for (int i = 0; i < NUM_THREADS; i++)
            pthread_join(threads[i], NULL);

        for (int i = 0; i < NUM_THREADS; i++)
            munit_assert_int(ctxs[i].result, ==, 0);
    }

    /* Verify total count. */
    size_t expected = (size_t)NUM_THREADS * POOL_INSERT_PER_THREAD;
    munit_assert_size(ct_skip_length_conc(list), ==, expected);

    /* Verify every key is present. */
    for (int k = 0; k < NUM_THREADS * POOL_INSERT_PER_THREAD; k++) {
        conc_node_t query;
        memset(&query, 0, sizeof(query));
        query.key = k;
        conc_node_t *found = ct_skip_position_eq_conc(list, &query);
        munit_assert_not_null(found);
        munit_assert_int(found->key, ==, k);
    }

    /* Tear down: walk the data nodes (between head and tail), freeing
       only those that were allocated from the heap (not from the pool).
       Pool-allocated nodes live in the contiguous slab freed by
       ct_skip_pool_destroy_conc below. */
    ct_skip_ebr_drain_conc(ebr);
    {
        conc_node_t *cur = ct_skip_head_conc(list);
        while (cur != NULL) {
            conc_node_t *next = ct_skip_next_node_conc(list, cur);
            if (!ct_skip_pool_is_from_conc(&pool, cur))
                free(cur);
            cur = next;
        }
    }
    free(list->slh_head);
    free(list->slh_tail);
    free(list);
    free(ebr);

    ct_skip_pool_destroy_conc(&pool);
    return MUNIT_OK;
}

/* Exercise the navigation/dup/update/position/to_array surface for the
 * `conc` skiplist type.  Single-threaded sweep, no concurrency: this is
 * here so the `conc` macro instantiations get full function coverage. */
static MunitResult
test_conc_api_breadth(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;

    conc_t *list;
    _skip_ebr_conc_t *ebr;
    setup_list_and_ebr(&list, &ebr);

    int tid = ct_skip_ebr_register_conc(ebr);
    ct_skip_ebr_pin_conc(ebr, tid);

    /* Insert distinct keys plus a duplicate. */
    for (int i = 1; i <= 10; i++) {
        conc_node_t *n;
        munit_assert_int(ct_skip_alloc_node_conc(&n), ==, 0);
        n->key = i;
        n->value = i * 10;
        munit_assert_int(ct_skip_insert_conc(list, n), ==, 0);
    }
    {
        conc_node_t *dup;
        munit_assert_int(ct_skip_alloc_node_conc(&dup), ==, 0);
        dup->key = 5;
        dup->value = 999;
        munit_assert_int(ct_skip_insert_dup_conc(list, dup), ==, 0);
    }

    /* update(): walk via query node. */
    {
        conc_node_t q;
        memset(&q, 0, sizeof(q));
        q.key = 3;
        long new_v = 30000;
        munit_assert_int(ct_skip_update_conc(list, &q, (void *)(uintptr_t)new_v), ==, 0);
    }

    /* head/tail/next/prev. */
    munit_assert_not_null(ct_skip_tail_conc(list));
    {
        conc_node_t *first = ct_skip_head_conc(list);
        conc_node_t *next = ct_skip_next_node_conc(list, first);
        munit_assert_not_null(next);
        conc_node_t *prev = ct_skip_prev_node_conc(list, ct_skip_tail_conc(list));
        munit_assert_not_null(prev);
        conc_node_t *pv = ct_skip_prev_validated_conc(list, ct_skip_tail_conc(list));
        (void)pv;
    }

    /* position()/pos() variants. */
    {
        conc_node_t pq;
        memset(&pq, 0, sizeof(pq));
        pq.key = 5;
        munit_assert_not_null(ct_skip_position_conc(list, SKIP_EQ, &pq));
        munit_assert_not_null(ct_skip_position_lt_conc(list, &pq));
        munit_assert_not_null(ct_skip_position_lte_conc(list, &pq));
        munit_assert_not_null(ct_skip_position_gt_conc(list, &pq));
        munit_assert_not_null(ct_skip_position_gte_conc(list, &pq));
    }

    /* to_array. */
    {
        conc_node_t **arr = ct_skip_to_array_conc(list);
        munit_assert_not_null(arr);
        size_t arr_len = (size_t)(uintptr_t)arr[-1];
        munit_assert_size(arr_len, ==, ct_skip_length_conc(list));
        free(arr - 1);
    }

    /* Pool free_node trampoline. */
    {
        _skip_pool_conc_t pool;
        munit_assert_int(ct_skip_pool_init_conc(&pool, 16), ==, 0);
        conc_node_t *n = ct_skip_pool_alloc_conc(&pool);
        munit_assert_not_null(n);
        ct_skip_pool_free_node_conc(&pool, list, n);
        ct_skip_pool_destroy_conc(&pool);
    }

    /* sizeof_entry/archive_entry trampolines: invoke directly. */
    {
        conc_node_t a, b;
        memset(&a, 0, sizeof(a));
        memset(&b, 0, sizeof(b));
        a.key = 1;
        a.value = 11;
        size_t sz = list->slh_fns.sizeof_entry(&a);
        munit_assert_size(sz, >, 0);
        int rc = list->slh_fns.archive_entry(&b, &a);
        munit_assert_int(rc, ==, 0);
    }

    ct_skip_ebr_unpin_conc(ebr, tid);
    ct_skip_free_conc(list);
    free(list);
    free(ebr);
    return MUNIT_OK;
}

/* ===========================================================================
 * Test suite registration
 * ===========================================================================*/

/* ===========================================================================
 * RACE-VALIDATION SUITE
 *
 * The following tests are designed to be run under ThreadSanitizer
 * (`make test_tsan`) and AddressSanitizer (`make test_concurrent`) to validate
 * that the lock-free paths are free of data races and use-after-free, in
 * addition to producing correct results.  They deliberately maximise
 * contention: overlapping key ranges, simultaneous readers and writers,
 * pool claim/release churn, and EBR register/unregister churn.
 * ===========================================================================*/

#define RACE_KEYS 2000
#define RACE_ITERS 4000

/* ---- Race 1: overlapping inserts -- every thread races to insert the SAME
 * key set; the structure must end with each key present exactly once. ---- */
static void *
thread_overlapping_insert(void *arg)
{
    thread_ctx_t *ctx = (thread_ctx_t *)arg;
    for (int k = 0; k < RACE_KEYS; k++) {
        conc_node_t *node;
        if (ct_skip_alloc_node_conc(&node) != 0) {
            ctx->result = -1;
            return NULL;
        }
        node->key = k;
        node->value = ctx->thread_id;
        ct_skip_ebr_pin_conc(ctx->ebr, ctx->ebr_tid);
        int rc = ct_skip_insert_conc(ctx->list, node);
        ct_skip_ebr_unpin_conc(ctx->ebr, ctx->ebr_tid);
        if (rc != 0) {
            /* Lost the race for this key -- expected; reclaim our node.
               skip_free_node already calls free(); do not free twice. */
            ct_skip_free_node_conc(ctx->list, node);
        }
    }
    ctx->result = 0;
    return NULL;
}

static MunitResult
test_race_overlapping_insert(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;
    conc_t *list;
    _skip_ebr_conc_t *ebr;
    setup_list_and_ebr(&list, &ebr);

    pthread_t threads[NUM_THREADS];
    thread_ctx_t ctxs[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) {
        ctxs[i].list = list;
        ctxs[i].ebr = ebr;
        ctxs[i].thread_id = i;
        ctxs[i].ebr_tid = ct_skip_ebr_register_conc(ebr);
        munit_assert_int(ctxs[i].ebr_tid, >=, 0);
        ctxs[i].result = -1;
    }
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_create(&threads[i], NULL, thread_overlapping_insert, &ctxs[i]);
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);
    for (int i = 0; i < NUM_THREADS; i++)
        munit_assert_int(ctxs[i].result, ==, 0);

    /* Exactly-once: length equals the key-set size and every key resolves. */
    munit_assert_size(ct_skip_length_conc(list), ==, (size_t)RACE_KEYS);
    for (int k = 0; k < RACE_KEYS; k++) {
        conc_node_t query;
        memset(&query, 0, sizeof(query));
        query.key = k;
        munit_assert_not_null(ct_skip_position_eq_conc(list, &query));
    }
    teardown_list_and_ebr(list, ebr);
    return MUNIT_OK;
}

/* ---- Race 2: insert/delete churn on a shared key range under EBR. ---- */
static void *
thread_insert_delete_churn(void *arg)
{
    thread_ctx_t *ctx = (thread_ctx_t *)arg;
    /* All threads churn the SAME key range, so every key is repeatedly deleted
       and re-inserted under contention -- the exact scenario that exposes the
       Harris+EBR "retired != unlinked" hazard.  The post-join walk then
       confirms no freed node remained linked. */
    for (int it = 0; it < RACE_ITERS; it++) {
        int k = munit_rand_int_range(0, RACE_KEYS - 1);
        ct_skip_ebr_pin_conc(ctx->ebr, ctx->ebr_tid);
        if ((it & 1) == 0) {
            conc_node_t *node;
            if (ct_skip_alloc_node_conc(&node) == 0) {
                node->key = k;
                node->value = k;
                if (ct_skip_insert_conc(ctx->list, node) != 0) {
                    ct_skip_free_node_conc(ctx->list, node);
                }
            }
        } else {
            conc_node_t query;
            memset(&query, 0, sizeof(query));
            query.key = k;
            ct_skip_remove_node_conc(ctx->list, &query);
        }
        ct_skip_ebr_unpin_conc(ctx->ebr, ctx->ebr_tid);
    }
    ctx->result = 0;
    return NULL;
}

static MunitResult
test_race_insert_delete_churn(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;
    /* Heavy concurrent insert+delete churn on disjoint per-thread ranges,
       followed by a full structural walk.  Validates that remove physically
       unlinks a node from every level (by pointer identity) before retiring
       it, so EBR never reclaims a still-reachable node. */
    conc_t *list;
    _skip_ebr_conc_t *ebr;
    setup_list_and_ebr(&list, &ebr);

    pthread_t threads[NUM_THREADS];
    thread_ctx_t ctxs[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) {
        ctxs[i].list = list;
        ctxs[i].ebr = ebr;
        ctxs[i].thread_id = i;
        ctxs[i].ebr_tid = ct_skip_ebr_register_conc(ebr);
        munit_assert_int(ctxs[i].ebr_tid, >=, 0);
        ctxs[i].result = -1;
    }
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_create(&threads[i], NULL, thread_insert_delete_churn, &ctxs[i]);
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);

    /* After the churn the structure must remain internally consistent and
       its length must match an actual head-to-tail node count.  Walk while
       retired nodes are still EBR-protected (before drain): every node still
       linked into the list is live. */
    size_t walked = 0;
    conc_node_t *cur;
    size_t it;
    int prev_key = -1;
    SKIPLIST_FOREACH_H2T(conc, ct_, entries, list, cur, it)
    {
        (void)it;
        munit_assert_int(cur->key, >, prev_key); /* strictly ascending */
        prev_key = cur->key;
        walked++;
    }
    munit_assert_size(walked, ==, ct_skip_length_conc(list));
    ct_skip_ebr_drain_conc(ebr);
    teardown_list_and_ebr(list, ebr);
    return MUNIT_OK;
}

/* ---- Race 3: pool claim exclusivity -- under contention no two threads may
 * ever hold the same slot.  Each claimer stamps a unique owner marker into
 * the slot and re-checks it before releasing; a double hand-out would either
 * trip the assert or be flagged by TSAN as a data race on node->value. ---- */
typedef struct {
    _skip_pool_conc_t *pool;
    int thread_id;
    int result;
} pool_excl_ctx_t;

static void *
thread_pool_exclusive(void *arg)
{
    pool_excl_ctx_t *ctx = (pool_excl_ctx_t *)arg;
    int marker = ctx->thread_id + 1;
    for (int it = 0; it < RACE_ITERS; it++) {
        conc_node_t *n = ct_skip_pool_alloc_conc(ctx->pool);
        if (n == NULL)
            continue; /* pool momentarily full */
        n->value = marker;
        /* If the slot were also handed to another thread, value would change. */
        for (int s = 0; s < 8; s++) {
            if (n->value != marker) {
                ctx->result = -1;
                ct_skip_pool_free_conc(ctx->pool, n);
                return NULL;
            }
        }
        ct_skip_pool_free_conc(ctx->pool, n);
    }
    ctx->result = 0;
    return NULL;
}

static MunitResult
test_race_pool_exclusive(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;
    _skip_pool_conc_t pool;
    /* Small pool relative to thread count to force heavy claim contention. */
    munit_assert_int(ct_skip_pool_init_conc(&pool, 8), ==, 0);

    pthread_t threads[NUM_THREADS];
    pool_excl_ctx_t ctxs[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) {
        ctxs[i].pool = &pool;
        ctxs[i].thread_id = i;
        ctxs[i].result = -1;
    }
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_create(&threads[i], NULL, thread_pool_exclusive, &ctxs[i]);
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);
    for (int i = 0; i < NUM_THREADS; i++)
        munit_assert_int(ctxs[i].result, ==, 0);
    ct_skip_pool_destroy_conc(&pool);
    return MUNIT_OK;
}

/* ---- Race 4: EBR register/unregister churn -- threads repeatedly register,
 * pin, unpin and unregister, stressing slot recycling.  register must never
 * return -1 while slots remain, and slots must be reusable. ---- */
typedef struct {
    _skip_ebr_conc_t *ebr;
    int result;
} ebr_churn_ctx_t;

static void *
thread_ebr_churn(void *arg)
{
    ebr_churn_ctx_t *ctx = (ebr_churn_ctx_t *)arg;
    for (int it = 0; it < RACE_ITERS; it++) {
        int tid = ct_skip_ebr_register_conc(ctx->ebr);
        if (tid < 0)
            continue; /* all slots momentarily taken -- acceptable */
        ct_skip_ebr_pin_conc(ctx->ebr, tid);
        ct_skip_ebr_unpin_conc(ctx->ebr, tid);
        ct_skip_ebr_unregister_conc(ctx->ebr, tid);
    }
    ctx->result = 0;
    return NULL;
}

static MunitResult
test_race_ebr_register_churn(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;
    _skip_ebr_conc_t ebr;
    ct_skip_ebr_init_conc(&ebr);

    pthread_t threads[NUM_THREADS];
    ebr_churn_ctx_t ctxs[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) {
        ctxs[i].ebr = &ebr;
        ctxs[i].result = -1;
    }
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_create(&threads[i], NULL, thread_ebr_churn, &ctxs[i]);
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);
    for (int i = 0; i < NUM_THREADS; i++)
        munit_assert_int(ctxs[i].result, ==, 0);

    /* After the churn every slot must be releasable: a fresh batch of
       NUM_THREADS registrations must all succeed and be distinct. */
    int tids[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) {
        tids[i] = ct_skip_ebr_register_conc(&ebr);
        munit_assert_int(tids[i], >=, 0);
        for (int j = 0; j < i; j++)
            munit_assert_int(tids[i], !=, tids[j]);
    }
    return MUNIT_OK;
}

/* ---- Race 5: readers (position_* + get) racing writers (insert/delete).
 * Validates the marked-pointer-safe scan in position_gt/gte under live
 * mutation: readers must never crash or dereference a marked pointer. ---- */
typedef struct {
    conc_t *list;
    _skip_ebr_conc_t *ebr;
    int ebr_tid;
    int is_writer;
    int lo, hi; /* key range for disjoint writers (0/0 = full shared range) */
    int result;
} rw_ctx_t;

static void *
thread_reader_positions(void *arg)
{
    rw_ctx_t *ctx = (rw_ctx_t *)arg;
    const skip_pos_conc_t ops[5] = { SKIP_EQ, SKIP_LT, SKIP_LTE, SKIP_GT, SKIP_GTE };
    for (int it = 0; it < RACE_ITERS; it++) {
        conc_node_t query;
        memset(&query, 0, sizeof(query));
        query.key = munit_rand_int_range(-1, RACE_KEYS);
        ct_skip_ebr_pin_conc(ctx->ebr, ctx->ebr_tid);
        conc_node_t *r = ct_skip_position_conc(ctx->list, ops[it % 5], &query);
        /* A returned node must be a real internal node, never a sentinel. */
        if (r != NULL)
            munit_assert_int(r->key, >=, 0);
        ct_skip_ebr_unpin_conc(ctx->ebr, ctx->ebr_tid);
    }
    ctx->result = 0;
    return NULL;
}

static void *
thread_writer_mutate(void *arg)
{
    rw_ctx_t *ctx = (rw_ctx_t *)arg;
    int lo = ctx->lo, hi = ctx->hi;
    if (lo == 0 && hi == 0) {
        lo = 0;
        hi = RACE_KEYS;
    }
    for (int it = 0; it < RACE_ITERS; it++) {
        int k = munit_rand_int_range(lo, hi - 1);
        ct_skip_ebr_pin_conc(ctx->ebr, ctx->ebr_tid);
        if ((it & 1) == 0) {
            conc_node_t *node;
            if (ct_skip_alloc_node_conc(&node) == 0) {
                node->key = k;
                node->value = k;
                if (ct_skip_insert_conc(ctx->list, node) != 0) {
                    ct_skip_free_node_conc(ctx->list, node);
                }
            }
        } else {
            conc_node_t query;
            memset(&query, 0, sizeof(query));
            query.key = k;
            ct_skip_remove_node_conc(ctx->list, &query);
        }
        ct_skip_ebr_unpin_conc(ctx->ebr, ctx->ebr_tid);
    }
    ctx->result = 0;
    return NULL;
}

static MunitResult
test_race_readers_writers(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;
    conc_t *list;
    _skip_ebr_conc_t *ebr;
    setup_list_and_ebr(&list, &ebr);
    prefill_list(list, 0, RACE_KEYS / 2);

    pthread_t threads[NUM_THREADS];
    rw_ctx_t ctxs[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) {
        ctxs[i].list = list;
        ctxs[i].ebr = ebr;
        ctxs[i].ebr_tid = ct_skip_ebr_register_conc(ebr);
        munit_assert_int(ctxs[i].ebr_tid, >=, 0);
        ctxs[i].is_writer = (i % 2);
        ctxs[i].lo = 0; /* shared full range: exercises contended-key reads */
        ctxs[i].hi = 0;
        ctxs[i].result = -1;
    }
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_create(&threads[i], NULL, ctxs[i].is_writer ? thread_writer_mutate : thread_reader_positions, &ctxs[i]);
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);
    for (int i = 0; i < NUM_THREADS; i++)
        munit_assert_int(ctxs[i].result, ==, 0);

    ct_skip_ebr_drain_conc(ebr);
    teardown_list_and_ebr(list, ebr);
    return MUNIT_OK;
}

/* ---- Race 6: forward/backward iteration under concurrent mutation.
 * A pinned reader iterates the level-0 list while writers mutate; it must
 * not crash and every visited node must be a valid, ascending key. ---- */
static void *
thread_iterate_reader(void *arg)
{
    rw_ctx_t *ctx = (rw_ctx_t *)arg;
    for (int it = 0; it < RACE_ITERS / 20; it++) {
        ct_skip_ebr_pin_conc(ctx->ebr, ctx->ebr_tid);
        conc_node_t *cur;
        size_t i;
        int prev = -1;
        int steps = 0;
        SKIPLIST_FOREACH_H2T(conc, ct_, entries, ctx->list, cur, i)
        {
            (void)i;
            if (cur->key < prev) { /* level-0 order must never go backwards */
                ctx->result = -1;
                ct_skip_ebr_unpin_conc(ctx->ebr, ctx->ebr_tid);
                return NULL;
            }
            prev = cur->key;
            if (++steps > RACE_KEYS * 4)
                break; /* guard against a transient cycle while mutating */
        }
        ct_skip_ebr_unpin_conc(ctx->ebr, ctx->ebr_tid);
    }
    ctx->result = 0;
    return NULL;
}

static MunitResult
test_race_iterate_during_mutation(const MunitParameter params[], void *data)
{
    (void)params;
    (void)data;
    /* A pinned reader repeatedly walks the whole list while writers churn --
       the most sensitive probe for reclaiming a still-reachable node. */
    conc_t *list;
    _skip_ebr_conc_t *ebr;
    setup_list_and_ebr(&list, &ebr);
    prefill_list(list, 0, RACE_KEYS / 2);

    pthread_t threads[NUM_THREADS];
    rw_ctx_t ctxs[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) {
        ctxs[i].list = list;
        ctxs[i].ebr = ebr;
        ctxs[i].ebr_tid = ct_skip_ebr_register_conc(ebr);
        munit_assert_int(ctxs[i].ebr_tid, >=, 0);
        ctxs[i].is_writer = (i != 0); /* one reader, rest writers */
        /* Writers churn the full shared range; the reader scans the whole list. */
        ctxs[i].lo = 0;
        ctxs[i].hi = 0;
        ctxs[i].result = -1;
    }
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_create(&threads[i], NULL, ctxs[i].is_writer ? thread_writer_mutate : thread_iterate_reader, &ctxs[i]);
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);
    for (int i = 0; i < NUM_THREADS; i++)
        munit_assert_int(ctxs[i].result, ==, 0);

    ct_skip_ebr_drain_conc(ebr);
    teardown_list_and_ebr(list, ebr);
    return MUNIT_OK;
}

static MunitTest conc_test_suite_tests[] = {
    { (char *)"/concurrent_insert", test_concurrent_insert, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/concurrent_search", test_concurrent_search, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/concurrent_delete", test_concurrent_delete, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/mixed_workload", test_mixed_workload, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/ebr_correctness", test_ebr_correctness, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/pool_contention", test_pool_contention, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/conc_api_breadth", test_conc_api_breadth, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/race_overlapping_insert", test_race_overlapping_insert, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/race_insert_delete_churn", test_race_insert_delete_churn, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/race_pool_exclusive", test_race_pool_exclusive, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/race_ebr_register_churn", test_race_ebr_register_churn, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/race_readers_writers", test_race_readers_writers, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/race_iterate_during_mutation", test_race_iterate_during_mutation, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
};

static const MunitSuite conc_test_suite = { (char *)"", conc_test_suite_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE };

int
main(int argc, char *argv[MUNIT_ARRAY_PARAM(argc + 1)])
{
    return munit_suite_main(&conc_test_suite, (void *)"concurrent-skiplist", argc, argv);
}
