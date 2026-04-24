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
 * segfault without sanitizers due to a race in __skip_adjust_hit_counts_,
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
    {
        (void)node;
    },
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
    {
        bytes = sizeof(struct conc_node);
    })

/* Generate EBR functions for the "conc" type. */
SKIPLIST_DECL_EBR(conc, ct_)

/* ---------------------------------------------------------------------------
 * Shared test context passed to each thread.
 * ---------------------------------------------------------------------------*/
typedef struct {
    conc_t *list;
    __skip_ebr_conc_t *ebr;
    int thread_id;       /* logical thread index 0..NUM_THREADS-1 */
    int ebr_tid;         /* EBR-registered thread id */
    int result;          /* 0 = success */
} thread_ctx_t;

/* ---------------------------------------------------------------------------
 * Helper: allocate and init a list + EBR, returning them through pointers.
 * ---------------------------------------------------------------------------*/
static void
setup_list_and_ebr(conc_t **list_out, __skip_ebr_conc_t **ebr_out)
{
    conc_t *list = (conc_t *)calloc(1, sizeof(conc_t));
    munit_assert_not_null(list);
    int rc = ct_skip_init_conc(list);
    munit_assert_int(rc, ==, 0);

    __skip_ebr_conc_t *ebr = (__skip_ebr_conc_t *)calloc(1, sizeof(__skip_ebr_conc_t));
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
teardown_list_and_ebr(conc_t *list, __skip_ebr_conc_t *ebr)
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
            /* Duplicate or error -- should not happen with disjoint ranges. */
            ct_skip_free_node_conc(ctx->list, node);
            free(node);
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
    __skip_ebr_conc_t *ebr;
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
    __skip_ebr_conc_t *ebr;
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
    __skip_ebr_conc_t *ebr;
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
    int search_count = 0;

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
                   overlapping ranges, but we use disjoint ranges here. */
                ct_skip_free_node_conc(ctx->list, node);
                free(node);
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
            search_count++;
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
    __skip_ebr_conc_t *ebr;
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
    __skip_ebr_conc_t *ebr;
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
 * Test suite registration
 * ===========================================================================*/

static MunitTest conc_test_suite_tests[] = {
    { (char *)"/concurrent_insert", test_concurrent_insert, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/concurrent_search", test_concurrent_search, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/concurrent_delete", test_concurrent_delete, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/mixed_workload", test_mixed_workload, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/ebr_correctness", test_ebr_correctness, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
};

static const MunitSuite conc_test_suite = { (char *)"", conc_test_suite_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE };

int
main(int argc, char *argv[MUNIT_ARRAY_PARAM(argc + 1)])
{
    return munit_suite_main(&conc_test_suite, (void *)"concurrent-skiplist", argc, argv);
}
