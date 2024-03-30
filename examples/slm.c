
#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// OPTIONS to set before including sl.h
// ---------------------------------------------------------------------------
#define DEBUG
#define SKIPLIST_DIAGNOSTIC
/* Setting SKIPLIST_MAX_HEIGHT will do two things:
 * 1) limit our max height across all instances of this data structure.
 * 2) remove a heap allocation on frequently used paths, insert/remove/etc.
 * so, use it when you need it.
 */
#define SKIPLIST_MAX_HEIGHT 12

// Include our monolithic ADT, the Skiplist!
// ---------------------------------------------------------------------------
#include "../include/sl.h"

// Local demo application OPTIONS:
// ---------------------------------------------------------------------------
#define VALIDATE
// TODO define SNAPSHOTS
#define DOT
#define TEST_ARRAY_SIZE 10

// ---------------------------------------------------------------------------
#ifdef VALIDATE
#define CHECK __skip_integrity_check_sample(list, 0)
#else
#define CHECK ((void)0)
#endif


/*
 * SKIPLIST EXAMPLE:
 *
 * This example creates a "sample" Skiplist where keys are integers, values are
 * strings allocated on the heap.
 *
 * 'sample' - meaning: EXample
 */

/*
 * To start, you must create a type node that will contain the
 * fields you'd like to maintain in your Skiplist.  In this case
 * we map int -> char [] on the heap, but what you put here is up
 * to you.  You don't even need a "key", just a way to compare one
 * node against another, logic you'll provide in SKIP_DECL as a
 * block below.
 */
struct sample_node {
    int key;
    char *value;
    SKIPLIST_ENTRY(sample_node) entries;
// TODO    SKIPLIST_SNAPS(sample_node) snaps;
};

/*
 * Generate all the access functions for our type of Skiplist.
 */
SKIPLIST_DECL(
    sample, api_, entries,
    /* compare entries: list, a, b, aux */
    {
        (void)list;
        (void)aux;
        if (a->key < b->key)
            return -1;
        if (a->key > b->key)
            return 1;
        return 0;
    },
    /* free entry: node */
    {
        free(node->value);
    },
    /* update entry: rc, src, dest */
    {
        char *new = calloc(strlen(src->value) + 1, sizeof(char));
        if (new == NULL) {
            rc = ENOMEM;
        } else {
            strncpy(new, src->value, strlen(src->value));
            free(dest->value);
            dest->value = new;
        }
    },
    /* archive an entry: rc, src, dest */
    {
        dest->key = src->key;
        char *nv = calloc(strlen(src->value) + 1, sizeof(char));
        if (nv == NULL)
            rc = ENOMEM;
        else {
            strncpy(nv, src->value, strlen(src->value));
            dest->value = nv;
        }
    },
    /* size in bytes of the content stored in an entry: bytes */
    {
        bytes = strlen(node->value) + 1;
    })

/*
 * Skiplists are ordered, we need a way to compare entries.
 * Let's create a function with four arguments:
 *   - a reference to the Skiplist, `slist`
 *   - the two nodes to compare, `a` and `b`
 *   - and `aux`, which you can use to pass into this function
 *     any additional information required to compare objects.
 *     `aux` is passed from the value in the Skiplist, you can
 *     modify that value at any time to suit your needs.
 *
 * Your function should result in a return statement:
 *   a  < b : return -1
 *   a == b : return 0
 *   a  > b : return 1
 *
 * This result provides the ordering within the Skiplist.  Sometimes
 * your function will not be used when comparing nodes.  This will
 * happen when `a` or `b` are references to the head or tail of the
 * list or when `a == b`.  In those cases the comparison function
 * returns before using the code in your block, don't panic. :)
int
__sample_key_compare(sample_t *list, sample_node_t *a, sample_node_t *b, void *aux)
{
    (void)list;
    (void)aux;
    if (a->key < b->key)
        return -1;
    if (a->key > b->key)
        return 1;
    return 0;
}
*/

/*
 * Optional: Getters and Setters
 *  - get, put, dup(put), del, etc. functions
 *
 * It can be useful to have simple get/put-style API, but to
 * do that you'll have to supply some blocks of code used to
 * extract data from within your nodes.
 */
SKIPLIST_DECL_ACCESS(
    sample, api_, key, int, value, char *,
    /* query blk */ { query.key = key; },
    /* return blk */ { return node->value; })

/*
 * Optional: Snapshots
 *
 * TODO
 */
//SKIPLIST_DECL_SNAPSHOTS(sample, api_, entries, snaps)

/*
 * Optional: Archive to/from bytes
 *
 * TODO
 */
SKIPLIST_DECL_ARCHIVE(sample, api_, entries)

/*
 * Optional: As Hashtable
 *
 * Turn your Skiplist into a hash table. TODO
 */
//SKIPLIST_DECL_HASHTABLE(sample, api_, entries, snaps)

/*
 * Optional: Check Skiplists at runtime
 *
 * Create a functions that validate the integrity of a Skiplist.
 */
SKIPLIST_DECL_VALIDATE(sample, api_, entries)

/* Optional: Visualize your Skiplist using DOT/Graphviz in PDF
 *
 * Create the functions used to annotate a visualization of a Skiplist.
 */
SKIPLIST_DECL_DOT(sample, api_, entries)

void
sprintf_sample_node(sample_node_t *node, char *buf)
{
    sprintf(buf, "%d:%s", node->key, node->value);
}

// Function for this demo application.
// ---------------------------------------------------------------------------
static char *
to_lower(char *str)
{
    char *p = str;
    for (; *p; ++p)
        *p = (char)(*p >= 'A' && *p <= 'Z' ? *p | 0x60 : *p);
    return str;
}

static char *
to_upper(char *str)
{
    char *p = str;
    for (; *p; ++p)
        *p = (char)(*p >= 'a' && *p <= 'z' ? *p & ~0x20 : *p);
    return str;
}

static char *
int_to_roman_numeral(int num)
{
    int del[] = { 1000, 900, 500, 400, 100, 90, 50, 40, 10, 9, 5, 4, 1 };                    // Key value in Roman counting
    char *sym[] = { "M", "CM", "D", "CD", "C", "XC", "L", "XL", "X", "IX", "V", "IV", "I" }; // Symbols for key values
    // The maximum length of the Roman numeral representation for the maximum signed 64-bit integer would be approximately 19 * 3 = 57 characters, assuming
    // every digit is represented by its Roman numeral equivalent up to 3 repetitions.  Therefore, 64 should be more than enough.
    char *res = (char *)calloc(4096, sizeof(char));
    int i = 0;
    if (num < 0) {
        res[0] = '-';
        i++;
        num = -num;
    }
    if (num == 0) {
        res[0] = '0';
        return res;
    }
    while (num) {                // while input number is not zero
        while (num / del[i]) {   // while a number contains the largest key value possible
            strcat(res, sym[i]); // append the symbol for this key value to res string
            num -= del[i];       // subtract the key value from number
        }
        i++; // proceed to the next key value
    }
    return res;
}

void
shuffle(int *array, size_t n)
{
    if (n > 1) {
        size_t i;
        for (i = n - 1; i > 0; i--) {
            size_t j = (unsigned int)(rand() % (i + 1)); /* NOLINT(*-msc50-cpp) */
            int t = array[j];
            array[j] = array[i];
            array[i] = t;
        }
    }
}

// ---------------------------------------------------------------------------
int
main()
{
    int rc;
#ifdef SNAPSHOTS
    size_t snap_i = 0;
    uint64_t snaps[2048];
#endif

#ifdef DOT
    size_t gen = 0;
    FILE *of = fopen("/tmp/slm.dot", "w");
    if (!of) {
        perror("Failed to open file /tmp/slm.dot");
        return 1;
    }
#endif

    /* Allocate and initialize a Skiplist. */
    sample_t *list = (sample_t *)malloc(sizeof(sample_t));
    if (list == NULL)
        return ENOMEM;

    rc = api_skip_init_sample(list, 12); //TODO -12
    if (rc)
        return rc;
#ifdef DOT
    api_skip_dot_sample(of, list, gen++, "init", sprintf_sample_node);
#endif
    if (api_skip_get_sample(list, 0) != NULL)
        perror("found a non-existent item!");
    api_skip_del_sample(list, 0);
    CHECK;

#ifdef SNAPSHOTS
    /* Test creating a snapshot of an empty Skiplist */
    snaps[snap_i++] = api_skip_snapshot_sample(list);
#endif

    /* Insert 7 key/value pairs into the list. */
    int i, j;
    char *numeral;
#ifdef DOT
    char msg[1024];
#endif
    int amt = TEST_ARRAY_SIZE, asz = (amt * 2) + 1;
    int array[(TEST_ARRAY_SIZE * 2) + 1];
    for (j = 0, i = -amt; i <= amt; i++, j++)
        array[j] = i;
    shuffle(array, asz);

    for (i = 0; i < asz; i++) {
        numeral = int_to_roman_numeral(array[i]);
        rc = api_skip_put_sample(list, array[i], to_lower(numeral));
        //rc = api_skip_put_sample(list, array[i], numeral);
        CHECK;
#ifdef SNAPSHOTS
        if (i > TEST_ARRAY_SIZE + 1) {
            snaps[snap_i++] = api_skip_snapshot_sample(list);
            CHECK;
        }
#endif
#ifdef DOT
        sprintf(msg, "put key: %d value: %s", i, numeral);
        api_skip_dot_sample(of, list, gen++, msg, sprintf_sample_node);
        CHECK;
#endif
        char *v = api_skip_get_sample(list, array[i]);
        CHECK;
        api_skip_set_sample(list, array[i], to_upper(v));
        CHECK;
    }
    numeral = int_to_roman_numeral(-1);
    api_skip_dup_sample(list, -1, numeral);
    CHECK;
#ifdef DOT
    sprintf(msg, "put dup key: %d value: %s", i, numeral);
    api_skip_dot_sample(of, list, gen++, msg, sprintf_sample_node);
    CHECK;
#endif
    numeral = int_to_roman_numeral(1);
    api_skip_dup_sample(list, 1, numeral);
    CHECK;
#ifdef DOT
    sprintf(msg, "put dup key: %d value: %s", i, numeral);
    api_skip_dot_sample(of, list, gen++, msg, sprintf_sample_node);
    CHECK;
#endif

    api_skip_del_sample(list, 0);
    CHECK;
    if (api_skip_get_sample(list, 0) != NULL)
        perror("found a deleted item!");
    api_skip_del_sample(list, 0);
    CHECK;
    if (api_skip_get_sample(list, 0) != NULL)
        perror("found a deleted item!");
    int key = TEST_ARRAY_SIZE + 1;
    api_skip_del_sample(list, key);
    CHECK;
    key = -(TEST_ARRAY_SIZE) - 1;
    numeral = int_to_roman_numeral(key);
    api_skip_del_sample(list, key);
    CHECK;

#ifdef DOT
    sprintf(msg, "deleted key: %d, value: %s", 0, numeral);
    api_skip_dot_sample(of, list, gen++, msg, sprintf_sample_node);
    CHECK;
#endif

#ifdef SNAPSHOTS
    sample_t *restored = api_skip_restore_snapshot_sample(list, snaps[snap_i - 1] );
    api_skip_release_snapshot_sample(list);
    api_skip_free_sample(restored);
#endif

    assert(strcmp(api_skip_pos_sample(list, SKIP_GTE, -(TEST_ARRAY_SIZE)-1)->value, int_to_roman_numeral(-(TEST_ARRAY_SIZE))) == 0);
    assert(strcmp(api_skip_pos_sample(list, SKIP_GTE, -2)->value, int_to_roman_numeral(-2)) == 0);
    assert(strcmp(api_skip_pos_sample(list, SKIP_GTE, 0)->value, int_to_roman_numeral(1)) == 0);
    assert(strcmp(api_skip_pos_sample(list, SKIP_GTE, 2)->value, int_to_roman_numeral(2)) == 0);
    assert(api_skip_pos_sample(list, SKIP_GTE, (TEST_ARRAY_SIZE + 1)) == NULL);

    assert(strcmp(api_skip_pos_sample(list, SKIP_GT, -(TEST_ARRAY_SIZE)-1)->value, int_to_roman_numeral(-(TEST_ARRAY_SIZE))) == 0);
    assert(strcmp(api_skip_pos_sample(list, SKIP_GT, -2)->value, int_to_roman_numeral(-1)) == 0);
    assert(strcmp(api_skip_pos_sample(list, SKIP_GT, 0)->value, int_to_roman_numeral(1)) == 0);
    assert(strcmp(api_skip_pos_sample(list, SKIP_GT, 1)->value, int_to_roman_numeral(2)) == 0);
    assert(api_skip_pos_sample(list, SKIP_GT, TEST_ARRAY_SIZE) == NULL);

    assert(api_skip_pos_sample(list, SKIP_LT, -(TEST_ARRAY_SIZE)) == NULL);
    assert(strcmp(api_skip_pos_sample(list, SKIP_LT, -1)->value, int_to_roman_numeral(-2)) == 0);
    assert(strcmp(api_skip_pos_sample(list, SKIP_LT, 0)->value, int_to_roman_numeral(-1)) == 0);
    assert(strcmp(api_skip_pos_sample(list, SKIP_LT, 2)->value, int_to_roman_numeral(1)) == 0);
    assert(strcmp(api_skip_pos_sample(list, SKIP_LT, (TEST_ARRAY_SIZE + 1))->value, int_to_roman_numeral(TEST_ARRAY_SIZE)) == 0);

    assert(api_skip_pos_sample(list, SKIP_LTE, -(TEST_ARRAY_SIZE)-1) == NULL);
    assert(strcmp(api_skip_pos_sample(list, SKIP_LTE, -2)->value, int_to_roman_numeral(-2)) == 0);
    assert(strcmp(api_skip_pos_sample(list, SKIP_LTE, 0)->value, int_to_roman_numeral(-1)) == 0);
    assert(strcmp(api_skip_pos_sample(list, SKIP_LTE, 2)->value, int_to_roman_numeral(2)) == 0);
    assert(strcmp(api_skip_pos_sample(list, SKIP_LTE, (TEST_ARRAY_SIZE + 1))->value, int_to_roman_numeral(TEST_ARRAY_SIZE)) == 0);

    api_skip_free_sample(list);
#ifdef DOT
    api_skip_dot_end_sample(of, gen);
    fclose(of);
#endif
    return rc;
}
