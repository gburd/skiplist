/*
 * Example 07: Pool Allocator
 *
 * SKIPLIST_DECL_POOL generates a slab-based pool allocator that
 * pre-allocates a contiguous block of memory for skiplist nodes.
 *
 * Benefits:
 *   - Avoids per-node malloc/free overhead
 *   - Cache-friendly: nodes are contiguous, 64-byte aligned
 *   - Lock-free allocation via CAS on the free-list head
 *
 * API:
 *   skip_pool_init_TYPE       -- initialize pool with N slots
 *   skip_pool_alloc_TYPE      -- pop a slot from the free list
 *   skip_pool_free_TYPE       -- push a slot back
 *   skip_pool_is_from_TYPE    -- check if a node belongs to pool
 *   skip_pool_alloc_node_TYPE -- alloc, returns ENOMEM on exhaustion
 *   skip_pool_free_node_TYPE  -- free user resources + return to pool
 *   skip_pool_destroy_TYPE    -- release the entire slab
 *
 * This example demonstrates pool allocation as a standalone allocator,
 * separate from skiplist insertion, to avoid the complexity of mixed
 * heap/pool node ownership during teardown.
 */
#include "sl.h"

struct pool_node {
    int key;
    int value;
    SKIPLIST_ENTRY(pool) entry;
};

SKIPLIST_DECL(
    pool, sl_, entry,
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

/* Generate pool functions. The 4th argument is a capacity hint. */
SKIPLIST_DECL_POOL(pool, sl_, entry, 64)

int
main(void)
{
    printf("=== Example 07: Pool Allocator ===\n\n");

    /* Step 1: Initialize the pool. Each slot is sized to hold one
     * pool_node_t plus its sle_levels array, rounded up to 64 bytes
     * for cache-line alignment. */
    _skip_pool_pool_t mypool;
    int rc = sl_skip_pool_init_pool(&mypool, 8);
    if (rc) {
        printf("Pool init failed: %d\n", rc);
        return rc;
    }
    printf("Pool initialized:\n");
    printf("  Capacity:  %zu slots\n", mypool.capacity);
    printf("  Slot size: %zu bytes (cache-line aligned)\n", mypool.slot_size);
    printf("  Total:     %zu bytes\n\n", mypool.capacity * mypool.slot_size);

    /* Step 2: Allocate nodes from the pool. */
    printf("--- Allocating from pool ---\n");
    pool_node_t *nodes[8];
    int count = 0;
    for (int i = 0; i < 10; i++) {
        pool_node_t *node = sl_skip_pool_alloc_pool(&mypool);
        if (!node) {
            printf("  [%d] Pool exhausted (NULL returned).\n", i);
            continue;
        }
        node->key = (i + 1) * 10;
        node->value = (i + 1) * 100;
        nodes[count] = node;
        count++;
        printf("  [%d] Allocated: key=%d, value=%d\n", i, node->key, node->value);
    }
    printf("\nAllocated %d nodes total.\n", count);

    /* Step 3: Check pool membership. */
    printf("\n--- Pool membership ---\n");
    for (int i = 0; i < count; i++) {
        printf("  nodes[%d] (key=%d): from_pool = %s\n", i, nodes[i]->key, sl_skip_pool_is_from_pool(&mypool, nodes[i]) ? "yes" : "no");
    }

    /* Compare with a heap-allocated node. */
    pool_node_t *heap_node;
    sl_skip_alloc_node_pool(&heap_node);
    heap_node->key = 999;
    printf("  heap_node (key=%d): from_pool = %s\n", heap_node->key, sl_skip_pool_is_from_pool(&mypool, heap_node) ? "yes" : "no");
    free(heap_node);

    /* Step 4: Return nodes to the pool and re-allocate. */
    printf("\n--- Free and re-allocate ---\n");
    printf("  Returning nodes[2] (key=%d) and nodes[5] (key=%d) to pool.\n", nodes[2]->key, nodes[5]->key);
    sl_skip_pool_free_pool(&mypool, nodes[2]);
    sl_skip_pool_free_pool(&mypool, nodes[5]);

    /* Re-allocate -- should get the freed slots back. */
    pool_node_t *reused1 = sl_skip_pool_alloc_pool(&mypool);
    pool_node_t *reused2 = sl_skip_pool_alloc_pool(&mypool);
    pool_node_t *should_fail = sl_skip_pool_alloc_pool(&mypool);

    if (reused1) {
        reused1->key = 777;
        printf("  Re-allocated slot -> key=%d (from_pool=%s)\n", reused1->key, sl_skip_pool_is_from_pool(&mypool, reused1) ? "yes" : "no");
    }
    if (reused2) {
        reused2->key = 888;
        printf("  Re-allocated slot -> key=%d (from_pool=%s)\n", reused2->key, sl_skip_pool_is_from_pool(&mypool, reused2) ? "yes" : "no");
    }
    printf("  Third alloc (should fail): %s\n", should_fail == NULL ? "NULL (expected)" : "unexpected success");

    /* Step 5: pool_alloc_node wrapper returns ENOMEM on exhaustion
     * instead of NULL, which is sometimes more convenient. */
    printf("\n--- pool_alloc_node (ENOMEM wrapper) ---\n");
    pool_node_t *errnode;
    rc = sl_skip_pool_alloc_node_pool(&mypool, &errnode);
    printf("  pool_alloc_node on full pool: rc=%d (%s)\n", rc, rc == ENOMEM ? "ENOMEM" : "unexpected");

    /* Step 6: Destroy the pool. This frees the entire contiguous slab
     * in one call. All pointers into the pool become invalid. */
    printf("\n--- Destroying pool ---\n");
    sl_skip_pool_destroy_pool(&mypool);
    printf("  Pool destroyed. All %d nodes freed at once.\n", count);

    printf("\nDone.\n");
    return 0;
}
