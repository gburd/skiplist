/*
 * Example 09: Validation and DOT Visualization
 *
 * SKIPLIST_DECL_VALIDATE generates a runtime integrity checker:
 *   _skip_integrity_check_TYPE(list, flags)
 * It verifies link consistency, ordering, height invariants, etc.
 *
 * SKIPLIST_DECL_DOT generates GraphViz DOT output:
 *   prefix_skip_dot_TYPE(FILE*, list, gen, msg, sprintf_fn)
 *   prefix_skip_dot_end_TYPE(FILE*, gen)
 *
 * The DOT output creates a visual multi-page graph where each
 * "generation" is a subgraph showing the skiplist state at that
 * point in time.
 */
#define _GNU_SOURCE
#define SKIPLIST_DIAGNOSTIC
#include "sl.h"

struct viz_node {
    int key;
    char *value;
    SKIPLIST_ENTRY(viz) entry;
};

SKIPLIST_DECL(
    viz, sl_, entry,
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
    /* archive */
    {
        dest->key = src->key;
        dest->value = src->value ? strdup(src->value) : NULL;
        if (src->value && !dest->value)
            rc = ENOMEM;
    },
    /* sizeof */
    { bytes = node->value ? strlen(node->value) + 1 : 0; })

SKIPLIST_DECL_ACCESS(viz, sl_, key, int, value, char *, { query.key = key; }, { return node->value; })

/* Generate validation functions. */
SKIPLIST_DECL_VALIDATE(viz, sl_, entry)

/* Generate DOT visualization functions. */
SKIPLIST_DECL_DOT(viz, sl_, entry)

/* Callback for DOT: format a node's label. */
static void
sprintf_viz_node(viz_node_t *node, char *buf)
{
    sprintf(buf, "%d: %s", node->key, node->value ? node->value : "?");
}

int
main(void)
{
    printf("=== Example 09: Validation & DOT Visualization ===\n\n");

    viz_t *list = (viz_t *)malloc(sizeof(viz_t));
    if (!list)
        return 1;
    sl_skip_init_viz(list);

    /* Open a DOT file for visualization output. */
    const char *dot_path = "/tmp/claude-1000/ex09.dot";
    FILE *dot_fp = fopen(dot_path, "w");
    if (!dot_fp) {
        perror("fopen");
        sl_skip_free_viz(list);
        free(list);
        return 1;
    }

    size_t gen = 0;

    /* Generation 0: empty list. */
    sl_skip_dot_viz(dot_fp, list, gen++, "empty list", sprintf_viz_node);

    /* Insert some entries, taking a DOT snapshot at each step. */
    printf("--- Inserting entries ---\n");
    const char *fruits[] = { "apple", "banana", "cherry", "date", "elderberry" };
    int keys[] = { 50, 20, 80, 10, 60 };
    char msg[256];

    for (int i = 0; i < 5; i++) {
        sl_skip_put_viz(list, keys[i], strdup(fruits[i]));
        printf("  Inserted key=%d (\"%s\")\n", keys[i], fruits[i]);

        snprintf(msg, sizeof(msg), "after insert %d=\"%s\"", keys[i], fruits[i]);
        sl_skip_dot_viz(dot_fp, list, gen++, msg, sprintf_viz_node);
    }

    /* Run the integrity check. */
    printf("\n--- Running integrity check ---\n");
    int errors = _skip_integrity_check_viz(list, 0);
    printf("  Integrity check returned %d error(s).\n", errors);

    /* Remove a node and check again. */
    printf("\n--- Removing key=20 ---\n");
    sl_skip_del_viz(list, 20);
    snprintf(msg, sizeof(msg), "after remove key=20");
    sl_skip_dot_viz(dot_fp, list, gen++, msg, sprintf_viz_node);

    errors = _skip_integrity_check_viz(list, 0);
    printf("  Integrity check after removal: %d error(s).\n", errors);

    /* Print final list contents. */
    printf("\n--- Final list ---\n");
    viz_node_t *cur;
    size_t idx;
    SKIPLIST_FOREACH_H2T(viz, sl_, entry, list, cur, idx)
    {
        printf("  key=%d, value=\"%s\"\n", cur->key, cur->value);
    }

    /* Close the DOT file. */
    sl_skip_dot_end_viz(dot_fp, gen);
    fclose(dot_fp);
    printf("\n  DOT file written to: %s\n", dot_path);
    printf("  Convert to PDF with: dot -Tpdf %s -o /tmp/claude-1000/ex09.pdf\n", dot_path);

    /* Clean up. */
    sl_skip_free_viz(list);
    free(list);

    printf("\nDone.\n");
    return 0;
}
