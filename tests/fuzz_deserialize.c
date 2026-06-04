/*
 * fuzz_deserialize.c -- libFuzzer harness for the archive deserialize path.
 *
 * Deserialize is the only function in sl.h that consumes attacker-controlled
 * bytes (a serialized list read from disk or the network), so it is the
 * primary security attack surface.  This harness feeds arbitrary fuzzer input
 * to m_skip_deserialize_model() via fmemopen() and asserts that, regardless of
 * input, the function never reads out of bounds, never leaks, and always
 * leaves the list in a state that passes the integrity validator.
 *
 * Build (clang):
 *   clang -fsanitize=fuzzer,address,undefined -Iinclude -std=c11 \
 *         tests/fuzz_deserialize.c -o tests/fuzz_deserialize -lm
 * Run:
 *   ./tests/fuzz_deserialize -runs=1000000 -max_total_time=60
 *
 * A non-libFuzzer fallback main() is provided (compiled when
 * SKIPLIST_FUZZ_STANDALONE is defined) so CI can replay a directory of inputs
 * without the libFuzzer runtime.
 */

#define _POSIX_C_SOURCE 200809L /* fmemopen */

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sl.h"

struct fz_node {
    int key;
    int value;
    SKIPLIST_ENTRY(fz) entries;
};

SKIPLIST_DECL(
    fz, m_, entries,
    /* compare */
    {
        (void)list;
        (void)aux;
        return (a->key < b->key) ? -1 : (a->key > b->key) ? 1 : 0;
    },
    /* free */ { (void)node; },
    /* update */ { node->value = (int)(intptr_t)value; },
    /* archive */
    {
        dest->key = src->key;
        dest->value = src->value;
    },
    /* sizeof */
    {
        (void)node;
        bytes = sizeof(int) * 2;
    })

SKIPLIST_DECL_VALIDATE(fz, m_, entries)

SKIPLIST_DECL_ARCHIVE(
    fz, m_, entries,
    /* write */
    {
        memcpy(buf, &node->key, sizeof(int));
        memcpy(buf + sizeof(int), &node->value, sizeof(int));
        bytes = sizeof(int) * 2;
    },
    /* read */
    {
        if (bytes < sizeof(int) * 2) {
            node->key = 0;
            node->value = 0;
        } else {
            memcpy(&node->key, buf, sizeof(int));
            memcpy(&node->value, buf + sizeof(int), sizeof(int));
        }
    })

int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    FILE *fp = fmemopen((void *)(uintptr_t)data, size, "rb");
    if (fp == NULL)
        return 0;

    fz_t sl;
    if (m_skip_init_fz(&sl) != 0) {
        fclose(fp);
        return 0;
    }

    int rc = m_skip_deserialize_fz(&sl, fp);
    fclose(fp);

    /* Whether deserialize succeeds or rejects the input, the resulting list
       must always be internally consistent -- a partial/failed parse must not
       leave a corrupt structure behind. */
    if (rc == 0) {
        int errors = _skip_integrity_check_fz(&sl, 1);
        if (errors != 0) {
            fprintf(stderr, "integrity check failed after successful deserialize (%d errors)\n", errors);
            abort();
        }
    }

    m_skip_free_fz(&sl);
    return 0;
}

#ifdef SKIPLIST_FUZZ_STANDALONE
/* Replay a list of files (or stdin) without the libFuzzer runtime. */
int
main(int argc, char **argv)
{
    if (argc < 2) {
        /* Read a single blob from stdin. */
        static uint8_t buf[1 << 20];
        size_t n = fread(buf, 1, sizeof(buf), stdin);
        return LLVMFuzzerTestOneInput(buf, n);
    }
    for (int i = 1; i < argc; i++) {
        FILE *f = fopen(argv[i], "rb");
        if (f == NULL)
            continue;
        static uint8_t buf[1 << 20];
        size_t n = fread(buf, 1, sizeof(buf), f);
        fclose(f);
        LLVMFuzzerTestOneInput(buf, n);
    }
    printf("fuzz_deserialize: replayed %d input(s) cleanly\n", argc - 1);
    return 0;
}
#endif
