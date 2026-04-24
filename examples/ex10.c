/*
 * Example 10: Concurrent Access with Epoch-Based Reclamation (EBR)
 *
 * SKIPLIST_DECL_EBR generates epoch-based reclamation support for
 * safe concurrent access to the skiplist from multiple threads:
 *
 *   skip_ebr_init_TYPE     -- initialize EBR state
 *   skip_ebr_register_TYPE -- register a thread (returns tid)
 *   skip_ebr_pin_TYPE      -- enter critical section (read epoch)
 *   skip_ebr_unpin_TYPE    -- leave critical section
 *   skip_ebr_attach_TYPE   -- attach EBR to a skiplist
 *   skip_ebr_retire_TYPE   -- defer freeing a removed node
 *   skip_ebr_drain_TYPE    -- force-free all deferred nodes
 *
 * Threads "pin" before accessing the list and "unpin" when done.
 * Removed nodes are deferred until all readers have advanced past
 * the removal epoch, preventing use-after-free.
 */
#include <pthread.h>

#include "sl.h"

#define NUM_THREADS 4
#define KEYS_PER_THREAD 200

struct conc_node {
    int key;
    int value;
    SKIPLIST_ENTRY(conc) entry;
};

SKIPLIST_DECL(
    conc, sl_, entry,
    /* compare */
    {
        (void)list;
        (void)aux;
        return (a->key < b->key) ? -1 : (a->key > b->key) ? 1 : 0;
    },
    /* free: nothing heap-allocated */
    { (void)node; },
    /* update */
    {
        int *v = (int *)value;
        node->value = *v;
    },
    /* archive */
    {
        dest->key = src->key;
        dest->value = src->value;
        (void)rc;
    },
    /* sizeof */
    { bytes = sizeof(*node); })

/* Generate EBR functions. */
SKIPLIST_DECL_EBR(conc, sl_)

/* Per-thread context passed to the worker function. */
typedef struct {
    conc_t *list;
    _skip_ebr_conc_t *ebr;
    int thread_id;
    int ebr_tid;
    int errors;
} thread_ctx_t;

/* Worker thread: insert a disjoint range, then search for them. */
static void *
worker(void *arg)
{
    thread_ctx_t *ctx = (thread_ctx_t *)arg;
    int start = ctx->thread_id * KEYS_PER_THREAD;
    int end = start + KEYS_PER_THREAD;
    ctx->errors = 0;

    /* Phase 1: Insert keys [start, end). */
    for (int k = start; k < end; k++) {
        conc_node_t *node;
        int rc = sl_skip_alloc_node_conc(&node);
        if (rc) {
            ctx->errors++;
            continue;
        }
        node->key = k;
        node->value = k * 10;

        sl_skip_ebr_pin_conc(ctx->ebr, ctx->ebr_tid);
        rc = sl_skip_insert_conc(ctx->list, node);
        sl_skip_ebr_unpin_conc(ctx->ebr, ctx->ebr_tid);

        if (rc) {
            /* Insert failed (e.g., duplicate). Free the node. */
            sl_skip_free_node_conc(ctx->list, node);
            ctx->errors++;
        }
    }

    /* Phase 2: Verify all inserted keys are findable. */
    for (int k = start; k < end; k++) {
        conc_node_t query;
        memset(&query, 0, sizeof(query));
        query.key = k;

        sl_skip_ebr_pin_conc(ctx->ebr, ctx->ebr_tid);
        conc_node_t *found = sl_skip_position_eq_conc(ctx->list, &query);
        sl_skip_ebr_unpin_conc(ctx->ebr, ctx->ebr_tid);

        if (!found || found->key != k) {
            fprintf(stderr, "Thread %d: key %d not found!\n", ctx->thread_id, k);
            ctx->errors++;
        }
    }

    return NULL;
}

int
main(void)
{
    printf("=== Example 10: Concurrent Access with EBR ===\n\n");
    printf("Configuration: %d threads, %d keys each\n\n", NUM_THREADS, KEYS_PER_THREAD);

    /* Step 1: Initialize the skiplist. */
    conc_t *list = (conc_t *)malloc(sizeof(conc_t));
    if (!list)
        return 1;
    int rc = sl_skip_init_conc(list);
    if (rc) {
        free(list);
        return rc;
    }

    /* Step 2: Initialize EBR and attach it to the list. */
    _skip_ebr_conc_t *ebr = (_skip_ebr_conc_t *)malloc(sizeof(_skip_ebr_conc_t));
    if (!ebr) {
        sl_skip_free_conc(list);
        free(list);
        return 1;
    }
    sl_skip_ebr_init_conc(ebr);
    sl_skip_ebr_attach_conc(list, ebr);

    /* Step 3: Set up thread contexts and register EBR threads. */
    pthread_t threads[NUM_THREADS];
    thread_ctx_t ctxs[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++) {
        ctxs[i].list = list;
        ctxs[i].ebr = ebr;
        ctxs[i].thread_id = i;
        ctxs[i].errors = 0;
        ctxs[i].ebr_tid = sl_skip_ebr_register_conc(ebr);
        if (ctxs[i].ebr_tid < 0) {
            fprintf(stderr, "Failed to register EBR thread %d\n", i);
            sl_skip_ebr_drain_conc(ebr);
            sl_skip_free_conc(list);
            free(list);
            free(ebr);
            return 1;
        }
    }

    /* Step 4: Spawn threads. */
    printf("--- Spawning %d threads ---\n", NUM_THREADS);
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_create(&threads[i], NULL, worker, &ctxs[i]);

    /* Wait for all threads to complete. */
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);

    /* Step 5: Check results. */
    printf("\n--- Results ---\n");
    int total_errors = 0;
    for (int i = 0; i < NUM_THREADS; i++) {
        printf("  Thread %d: %s (%d errors)\n", i, ctxs[i].errors == 0 ? "OK" : "ERRORS", ctxs[i].errors);
        total_errors += ctxs[i].errors;
    }

    size_t expected = (size_t)NUM_THREADS * KEYS_PER_THREAD;
    size_t actual = sl_skip_length_conc(list);
    printf("\nList length: %zu (expected %zu)\n", actual, expected);

    if (actual != expected) {
        printf("WARNING: length mismatch!\n");
        total_errors++;
    }

    /* Step 6: Clean up.
     * Drain EBR to free any deferred nodes, then free the list. */
    sl_skip_ebr_drain_conc(ebr);
    sl_skip_free_conc(list);
    free(list);
    free(ebr);

    printf("\n%s\n", total_errors == 0 ? "All tests passed." : "Some tests FAILED.");
    printf("\nDone.\n");
    return total_errors == 0 ? 0 : 1;
}
