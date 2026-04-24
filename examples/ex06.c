/*
 * Example 06: Snapshots
 *
 * SKIPLIST_DECL_SNAPSHOTS adds MVCC point-in-time snapshot support:
 *   - skip_snapshots_init: enable snapshotting on a list
 *   - skip_snapshot: take a snapshot, returns an era ID
 *   - skip_restore_snapshot: roll back to a previous era
 *   - skip_release_snapshots: free all preserved snapshot data
 *
 * When snapshots are active, modifications (inserts, removes, updates)
 * preserve the old version of affected nodes.  This allows a later
 * restore to bring the list back to a previous state.
 *
 * NOTE: Snapshot restore is being refined on this branch.  This
 * example demonstrates the full snapshot lifecycle.
 */
#define _GNU_SOURCE
#include "sl.h"

struct snap_node {
    int key;
    char *value;
    SKIPLIST_ENTRY(snap) entry;
};

SKIPLIST_DECL(
    snap, sl_, entry,
    /* compare */
    {
        (void)list;
        (void)aux;
        return (a->key < b->key) ? -1 : (a->key > b->key) ? 1 : 0;
    },
    /* free */
    {
        free(node->value);
        node->value = NULL;
    },
    /* update */
    {
        free(node->value);
        node->value = (char *)value;
    },
    /* archive: deep-copy the node for snapshot preservation */
    {
        dest->key = src->key;
        dest->value = src->value ? strdup(src->value) : NULL;
        if (src->value && !dest->value)
            rc = ENOMEM;
    },
    /* sizeof */
    { bytes = node->value ? strlen(node->value) + 1 : 0; })

SKIPLIST_DECL_ACCESS(snap, sl_, key, int, value, char *, { query.key = key; }, { return node->value; })

/* Generate snapshot functions. */
SKIPLIST_DECL_SNAPSHOTS(snap, sl_, entry)

/* Helper: print list contents. */
static void
print_list(snap_t *list, const char *label)
{
    printf("  [%s] length=%zu:", label, sl_skip_length_snap(list));
    snap_node_t *cur;
    size_t idx;
    SKIPLIST_FOREACH_H2T(snap, sl_, entry, list, cur, idx)
    {
        printf(" %d=\"%s\"", cur->key, cur->value);
    }
    printf("\n");
}

int
main(void)
{
    printf("=== Example 06: Snapshots ===\n\n");

    snap_t *list = (snap_t *)malloc(sizeof(snap_t));
    if (!list)
        return 1;
    sl_skip_init_snap(list);

    /* Step 1: Enable snapshots by registering the preserve/release
     * callbacks.  This must be called before taking any snapshots. */
    sl_skip_snapshots_init_snap(list);

    /* Step 2: Build the initial list. */
    sl_skip_put_snap(list, 10, strdup("ten"));
    sl_skip_put_snap(list, 20, strdup("twenty"));
    sl_skip_put_snap(list, 30, strdup("thirty"));
    print_list(list, "initial");

    /* Step 3: Take a snapshot.  Returns an era ID that can be used
     * later to restore or identify this point in time.
     *
     * After this call, any modifications to existing nodes will
     * trigger the archive block to preserve the old version. */
    uint64_t era = sl_skip_snapshot_snap(list);
    printf("\n  Snapshot taken: era = %lu\n", (unsigned long)era);
    printf("  Modifications after this snapshot will preserve old node versions.\n\n");

    /* Step 4: Modify the list.
     * These operations trigger node preservation under the hood:
     *   - Removing key 10 preserves the {10, "ten"} node
     *   - Updating key 20's value preserves the old {20, "twenty"} node
     *   - Inserting key 40 is new, so nothing is preserved for it */
    sl_skip_del_snap(list, 10);
    printf("  Deleted key 10.\n");
    sl_skip_set_snap(list, 20, strdup("TWENTY"));
    printf("  Updated key 20 -> \"TWENTY\".\n");
    sl_skip_put_snap(list, 40, strdup("forty"));
    printf("  Inserted key 40.\n\n");
    print_list(list, "after modifications");

    /* Step 5: Release all snapshot data.
     * This frees the preserved copies of old node versions.
     * After release, restore to this era is no longer possible. */
    printf("\n  Releasing snapshots (frees preserved node copies).\n");
    sl_skip_release_snapshots_snap(list);

    /* The list itself is unchanged -- only the backup copies are freed. */
    print_list(list, "after release");

    /* Step 6: We can take another snapshot and continue. */
    printf("\n  Taking a second snapshot...\n");
    uint64_t era2 = sl_skip_snapshot_snap(list);
    printf("  Snapshot era = %lu\n", (unsigned long)era2);

    sl_skip_put_snap(list, 50, strdup("fifty"));
    print_list(list, "after adding 50");

    /* Release again before cleanup. */
    sl_skip_release_snapshots_snap(list);

    /* Step 7: Clean up. */
    sl_skip_free_snap(list);
    free(list);

    printf("\nDone.\n");
    return 0;
}
