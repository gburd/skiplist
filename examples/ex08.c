/*
 * Example 08: Serialization (Archive)
 *
 * SKIPLIST_DECL_ARCHIVE generates functions to serialize a skiplist
 * to a binary file and deserialize it back:
 *
 *   skip_serialize_TYPE(list, FILE*)   -- write list to file
 *   skip_deserialize_TYPE(list, FILE*) -- read list from file
 *
 * You provide two code blocks:
 *   write_entry_blk -- serialize one node into a byte buffer
 *   read_entry_blk  -- deserialize one node from a byte buffer
 *
 * The file format is:
 *   [4 bytes magic "SKPL"] [4 bytes version] [8 bytes node count]
 *   For each node: [8 bytes entry_size] [entry_size bytes of data]
 */
#define _GNU_SOURCE
#include "sl.h"

struct arc_node {
    int key;
    char *value;
    SKIPLIST_ENTRY(arc) entry;
};

SKIPLIST_DECL(
    arc, sl_, entry,
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

SKIPLIST_DECL_ACCESS(arc, sl_, key, int, value, char *, { query.key = key; }, { return node->value; })

/* Generate serialize/deserialize functions.
 *
 * In the write block:
 *   - 'node' is the current node being serialized
 *   - 'buf' is a uint8_t* buffer (4096 bytes) to write into
 *   - 'bytes' is a uint64_t to set to the number of bytes written
 *
 * In the read block:
 *   - 'node' is a freshly allocated node to populate
 *   - 'buf' is a uint8_t* buffer containing the serialized data
 *   - 'bytes' is a uint64_t with the number of bytes available
 */
SKIPLIST_DECL_ARCHIVE(
    arc, sl_, entry,
    /* write_entry_blk */
    {
        uint64_t off = 0;
        /* Write the key. */
        memcpy(buf + off, &node->key, sizeof(node->key));
        off += sizeof(node->key);
        /* Write value: length prefix + string data. */
        if (node->value) {
            uint32_t slen = (uint32_t)strlen(node->value);
            memcpy(buf + off, &slen, sizeof(slen));
            off += sizeof(slen);
            memcpy(buf + off, node->value, slen);
            off += slen;
        } else {
            uint32_t slen = 0;
            memcpy(buf + off, &slen, sizeof(slen));
            off += sizeof(slen);
        }
        bytes = off;
    },
    /* read_entry_blk */
    {
        uint64_t off = 0;
        /* Read the key. */
        memcpy(&node->key, buf + off, sizeof(node->key));
        off += sizeof(node->key);
        /* Read the value. */
        uint32_t slen;
        memcpy(&slen, buf + off, sizeof(slen));
        off += sizeof(slen);
        if (slen > 0) {
            node->value = (char *)malloc(slen + 1);
            memcpy(node->value, buf + off, slen);
            node->value[slen] = '\0';
        } else {
            node->value = NULL;
        }
    })

/* Helper: print list contents. */
static void
print_list(arc_t *list, const char *label)
{
    printf("  [%s] length=%zu\n", label, sl_skip_length_arc(list));
    arc_node_t *cur;
    size_t idx;
    SKIPLIST_FOREACH_H2T(arc, sl_, entry, list, cur, idx)
    {
        printf("    [%zu] %d -> \"%s\"\n", idx, cur->key, cur->value ? cur->value : "(null)");
    }
}

int
main(void)
{
    printf("=== Example 08: Serialization ===\n\n");

    const char *filepath = "/tmp/claude-1000/ex08_skiplist.bin";

    /* Create and populate a skiplist. */
    arc_t *list = (arc_t *)malloc(sizeof(arc_t));
    if (!list)
        return 1;
    sl_skip_init_arc(list);

    sl_skip_put_arc(list, 10, strdup("ten"));
    sl_skip_put_arc(list, 20, strdup("twenty"));
    sl_skip_put_arc(list, 30, strdup("thirty"));
    sl_skip_put_arc(list, 40, strdup("forty"));
    sl_skip_put_arc(list, 50, strdup("fifty"));

    print_list(list, "original");

    /* Serialize to a file. */
    printf("\n--- Serializing to %s ---\n", filepath);
    FILE *fp = fopen(filepath, "wb");
    if (!fp) {
        perror("fopen");
        sl_skip_free_arc(list);
        free(list);
        return 1;
    }
    int rc = sl_skip_serialize_arc(list, fp);
    fclose(fp);
    if (rc) {
        printf("Serialize failed: %d\n", rc);
        sl_skip_free_arc(list);
        free(list);
        return rc;
    }
    printf("  Serialization successful.\n");

    /* Free the original list. */
    sl_skip_free_arc(list);
    free(list);

    /* Deserialize into a new list. */
    printf("\n--- Deserializing from %s ---\n", filepath);
    arc_t *list2 = (arc_t *)malloc(sizeof(arc_t));
    if (!list2)
        return 1;
    sl_skip_init_arc(list2);

    fp = fopen(filepath, "rb");
    if (!fp) {
        perror("fopen");
        sl_skip_free_arc(list2);
        free(list2);
        return 1;
    }
    rc = sl_skip_deserialize_arc(list2, fp);
    fclose(fp);
    if (rc) {
        printf("Deserialize failed: %d\n", rc);
        sl_skip_free_arc(list2);
        free(list2);
        return rc;
    }

    print_list(list2, "deserialized");

    /* Verify data integrity. */
    printf("\n--- Verification ---\n");
    int keys[] = { 10, 20, 30, 40, 50 };
    const char *expected[] = { "ten", "twenty", "thirty", "forty", "fifty" };
    int ok = 1;
    for (int i = 0; i < 5; i++) {
        char *v = sl_skip_get_arc(list2, keys[i]);
        if (v == NULL || strcmp(v, expected[i]) != 0) {
            printf("  MISMATCH: key=%d, got=\"%s\", expected=\"%s\"\n", keys[i], v ? v : "(null)", expected[i]);
            ok = 0;
        }
    }
    if (ok)
        printf("  All entries match after round-trip serialization.\n");

    /* Clean up. */
    sl_skip_free_arc(list2);
    free(list2);
    remove(filepath);

    printf("\nDone.\n");
    return 0;
}
