
#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define DEBUG 1
#define SKIPLIST_DEBUG slex

/* Setting this will do two things:
 * 1) limit our max height across all instances of this datastructure.
 * 2) remove a heap allocation on frequently used paths, insert/remove/etc.
 * so, use it when you need it.
 */
#define SKIPLIST_MAX_HEIGHT 12
#include "../include/sl.h"

/*
 * SKIPLIST EXAMPLE:
 *
 * This example creates a Skiplist keys and values are integers.
 * 'slex' - meaning: SkipList EXample
 */

/*
 * To start, you must create a type node that will contain the
 * fields you'd like to maintain in your Skiplist.  In this case
 * we map int -> int, but what you put here is up to you.  You
 * don't even need a "key", just a way to compare one node against
 * another, logic you'll provide in SKIP_DECL as a block below.
 */
struct slex_node {
    int key;
    char *value;
    /* NOTE: This _must_ be last element in node for snapshots to work!!! */
    SKIPLIST_ENTRY(slex_node) entries;
};

/*
 * Generate all the access functions for our type of Skiplist.
 */
SKIPLIST_DECL(
    slex, api_, entries,
    /* free node */ { free(node->value); },
    /* update node */
    {
        //char *old = node->value;
        node->value = new->value;
        // In this case, don't free, we're just calling to_upper and using the same memory.
        // free(old);
    },
    /* archive a node */
    {
        new->key = node->key;
        char *nv = calloc(strlen(node->value) + 1, sizeof(char));
        if (nv == NULL)
            rc = ENOMEM;
        else {
            strncpy(nv, node->value, strlen(node->value));
            new->value = nv;
        }
    },
    /* size in bytes of the content stored in an entry by you */
    { size = strlen(node->value) + 1; })

/*
 * Optional: Create a function that validates as much as possible the
 * integrity of a Skiplist.  This is called by the DOT function to
 * ensure that it's possible to generate a graph.
 */
SKIPLIST_INTEGRITY_CHECK(slex, api_, entries)

/* Optional: Create the functions used to visualize a Skiplist (DOT/Graphviz) */
SKIPLIST_DECL_DOT(slex, api_, entries)

void
sprintf_slex_node(slex_node_t *node, char *buf)
{
    sprintf(buf, "%d:%s", node->key, node->value);
}

/*
 * Getters and Setters
 * It can be useful to have simple get/put-style API, but to
 * do that you'll have to supply some blocks of code used to
 * extract data from within your nodes.
 */
SKIPLIST_KV_ACCESS(
    slex, api_, key, int, value, char *,
    /* query blk */ { query.key = key; },
    /* return blk */ { return node->value; })

/*
 * Now we need a way to compare the nodes you defined above.
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
 */
int
__slm_key_compare(slex_t *list, slex_node_t *a, slex_node_t *b, void *aux)
{
    (void)list;
    (void)aux;
    if (a->key < b->key)
        return -1;
    if (a->key > b->key)
        return 1;
    return 0;
}

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
    char *res = (char *)calloc(64, sizeof(char));
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

#define INTEGRITY
#ifdef INTEGRITY
#define INTEGRITY_CHK __skip_integrity_check_slex(list, 0)
#else
#define INTEGRITY_CKH ((void)0)
#endif

#define SNAPSHOTS
#define DOT
#define TEST_ARRAY_SIZE 5

int
main()
{
    int rc;

#ifdef DOT
    size_t gen = 0;
    FILE *of = fopen("/tmp/slm.dot", "w");
    if (!of) {
        perror("Failed to open file /tmp/slm.dot");
        return 1;
    }
#endif

    /* Allocate and initialize a Skiplist. */
    slex_t *list = (slex_t *)malloc(sizeof(slex_t));
    if (list == NULL)
        return ENOMEM;

    rc = api_skip_init_slex(list, 12, __slm_key_compare);
    if (rc)
        return rc;
#ifdef DOT
    api_skip_dot_slex(of, list, gen++, "init", sprintf_slex_node);
#endif

#ifdef SNAPSHOTS
    /* Test creating a snapshot of an empty Skiplist */
    size_t snp[TEST_ARRAY_SIZE * 2 + 10];
    snp[0] = api_skip_snapshot_slex(list);
#endif

    /* Insert 7 key/value pairs into the list. */
    int i, j;
    char *numeral, msg[1024];
    int amt = TEST_ARRAY_SIZE, asz = (amt * 2) + 1;
    int array[(TEST_ARRAY_SIZE * 2) + 1];
    for (j = 0, i = -amt; i <= amt; i++, j++)
        array[j] = i;
    shuffle(array, asz);

    for (i = 0; i < asz; i++) {
        numeral = int_to_roman_numeral(array[i]);
        rc = api_skip_put_slex(list, array[i], to_lower(numeral));
        //rc = api_skip_put_slex(list, array[i], numeral);
        INTEGRITY_CHK;
#ifdef SNAPSHOTS
        snp[i + 1] = api_skip_snapshot_slex(list);
#endif
        INTEGRITY_CHK;
#ifdef DOT
        sprintf(msg, "put key: %d value: %s", i, numeral);
        api_skip_dot_slex(of, list, gen++, msg, sprintf_slex_node);
        INTEGRITY_CHK;
#endif
        char *v = api_skip_get_slex(list, array[i]);
        INTEGRITY_CHK;
        api_skip_set_slex(list, array[i], to_upper(v));
        INTEGRITY_CHK;
    }
#ifdef SNAPSHOTS
    int r = i;
#endif
    numeral = int_to_roman_numeral(-1);
    api_skip_dup_slex(list, -1, numeral);
    INTEGRITY_CHK;
#ifdef DOT
    sprintf(msg, "put dup key: %d value: %s", i, numeral);
    api_skip_dot_slex(of, list, gen++, msg, sprintf_slex_node);
    INTEGRITY_CHK;
#endif
    numeral = int_to_roman_numeral(1);
    api_skip_dup_slex(list, 1, numeral);
    INTEGRITY_CHK;
#ifdef DOT
    sprintf(msg, "put dup key: %d value: %s", i, numeral);
    api_skip_dot_slex(of, list, gen++, msg, sprintf_slex_node);
    INTEGRITY_CHK;
#endif
#ifdef SNAPSHOTS
    snp[++i] = api_skip_snapshot_slex(list);
    INTEGRITY_CHK;
#endif

    api_skip_del_slex(list, 0);
    INTEGRITY_CHK;
#ifdef SNAPSHOTS
    snp[++i] = api_skip_snapshot_slex(list);
    INTEGRITY_CHK;
#endif

#ifdef DOT
    sprintf(msg, "deleted key: %d, value: %s", 0, numeral);
    api_skip_dot_slex(of, list, gen++, msg, sprintf_slex_node);
    INTEGRITY_CHK;
#endif

#ifdef SNAPSHOTS
    slex_t *restored = api_skip_restore_snapshot_slex(list, snp[r]);
    api_skip_dispose_snapshot_slex(list, r+1);
    api_skip_destroy_slex(restored);
#endif

    assert(strcmp(api_skip_pos_slex(list, SKIP_GTE, -(TEST_ARRAY_SIZE)-1)->value, int_to_roman_numeral(-(TEST_ARRAY_SIZE))) == 0);
    assert(strcmp(api_skip_pos_slex(list, SKIP_GTE, -2)->value, int_to_roman_numeral(-2)) == 0);
    assert(strcmp(api_skip_pos_slex(list, SKIP_GTE, 0)->value, int_to_roman_numeral(1)) == 0);
    assert(strcmp(api_skip_pos_slex(list, SKIP_GTE, 2)->value, int_to_roman_numeral(2)) == 0);
    assert(api_skip_pos_slex(list, SKIP_GTE, (TEST_ARRAY_SIZE + 1)) == NULL);

    assert(strcmp(api_skip_pos_slex(list, SKIP_GT, -(TEST_ARRAY_SIZE)-1)->value, int_to_roman_numeral(-(TEST_ARRAY_SIZE))) == 0);
    assert(strcmp(api_skip_pos_slex(list, SKIP_GT, -2)->value, int_to_roman_numeral(-1)) == 0);
    assert(strcmp(api_skip_pos_slex(list, SKIP_GT, 0)->value, int_to_roman_numeral(1)) == 0);
    assert(strcmp(api_skip_pos_slex(list, SKIP_GT, 1)->value, int_to_roman_numeral(2)) == 0);
    assert(api_skip_pos_slex(list, SKIP_GT, TEST_ARRAY_SIZE) == NULL);

    assert(api_skip_pos_slex(list, SKIP_LT, -(TEST_ARRAY_SIZE)) == NULL);
    assert(strcmp(api_skip_pos_slex(list, SKIP_LT, -1)->value, int_to_roman_numeral(-2)) == 0);
    assert(strcmp(api_skip_pos_slex(list, SKIP_LT, 0)->value, int_to_roman_numeral(-1)) == 0);
    assert(strcmp(api_skip_pos_slex(list, SKIP_LT, 2)->value, int_to_roman_numeral(1)) == 0);
    assert(strcmp(api_skip_pos_slex(list, SKIP_LT, (TEST_ARRAY_SIZE + 1))->value, int_to_roman_numeral(TEST_ARRAY_SIZE)) == 0);

    assert(api_skip_pos_slex(list, SKIP_LTE, -(TEST_ARRAY_SIZE)-1) == NULL);
    assert(strcmp(api_skip_pos_slex(list, SKIP_LTE, -2)->value, int_to_roman_numeral(-2)) == 0);
    assert(strcmp(api_skip_pos_slex(list, SKIP_LTE, 0)->value, int_to_roman_numeral(-1)) == 0);
    assert(strcmp(api_skip_pos_slex(list, SKIP_LTE, 2)->value, int_to_roman_numeral(2)) == 0);
    assert(strcmp(api_skip_pos_slex(list, SKIP_LTE, (TEST_ARRAY_SIZE + 1))->value, int_to_roman_numeral(TEST_ARRAY_SIZE)) == 0);

    api_skip_destroy_slex(list);
#ifdef DOT
    api_skip_dot_end_slex(of, gen);
    fclose(of);
#endif
    return rc;
}
