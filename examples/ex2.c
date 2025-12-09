#pragma GCC push_options
#pragma GCC optimize("O0")

// OPTIONS to set before including sl.h
// ---------------------------------------------------------------------------
#define DEBUG
#define SKIPLIST_DIAGNOSTIC

// Include our monolithic ADT, the Skiplist!
// ---------------------------------------------------------------------------
#include "../include/sl.h"

// Local demo application OPTIONS:
// ---------------------------------------------------------------------------
#define TEST_ARRAY_SIZE 10
#define VALIDATE
// define SNAPSHOTS
// define TODO_RESTORE_SNAPSHOTS
#define STABLE_SEED
#define DOT

#ifdef DOT
size_t gen = 0;
FILE *of = 0;
#endif

// ---------------------------------------------------------------------------
#ifdef VALIDATE
#define CHECK __skip_integrity_check_ex(list, 0)
#else
#define CHECK ((void)0)
#endif

/*
 * SKIPLIST EXAMPLE:
 *
 * This example creates an "ex" Skiplist where keys are integers, values are
 * strings containing the roman numeral for the key allocated on the heap.
 */

/*
 * To start, you must create a type node that will contain the
 * fields you'd like to maintain in your Skiplist.  In this case
 * we map int -> char [] on the heap, but what you put here is up
 * to you.  You don't even need a "key", just a way to compare one
 * node against another, logic you'll provide in SKIP_DECL as a
 * block below.
 */
struct ex_node {
    int key;
    char *value;
    SKIPLIST_ENTRY(ex) entries;
};

/*
 * Generate all the access functions for our type of Skiplist.
 */
SKIPLIST_DECL(
    ex, api_, entries,
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
        node->value = NULL;
    },
    /* update entry: rc, node, value */
    {
        char *numeral = (char *)value;
        if (node->value)
            free(node->value);
        node->value = numeral;
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
    { bytes = strlen(node->value) + 1; })

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
__ex_key_compare(ex_t *list, ex_node_t *a, ex_node_t *b, void *aux)
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
    ex, api_, key, int, value, char *,
    /* query blk */ { query.key = key; },
    /* return blk */ { return node->value; })

/*
 * Optional: Snapshots
 *
 * Enable functions that enable returning to an earlier point in
 * time when a snapshot was created.
 */
SKIPLIST_DECL_SNAPSHOTS(ex, api_, entries)

/*
 * Optional: Archive to/from bytes
 *
 * Enable functions that can write/read the content of your Skiplist
 * out/in to/from an array of bytes.
 */
// TODO SKIPLIST_DECL_ARCHIVE(ex, api_, entries)

/*
 * Optional: As Hashtable
 *
 * Turn your Skiplist into a hash table.
 */
// TODO SKIPLIST_DECL_HASHTABLE(ex, api_, entries, snaps)

/*
 * Optional: Check Skiplists at runtime
 *
 * Create a functions that validate the integrity of a Skiplist.
 */
SKIPLIST_DECL_VALIDATE(ex, api_, entries)

/* Optional: Visualize your Skiplist using DOT/Graphviz in PDF
 *
 * Create the functions used to annotate a visualization of a Skiplist.
 */
SKIPLIST_DECL_DOT(ex, api_, entries)

static void
sprintf_ex_node(ex_node_t *node, char *buf)
{
    sprintf(buf, "%d:%s", node->key, node->value);
}

// Function for this demo application.
// ---------------------------------------------------------------------------
int __xorshift32_state = 0;

// Xorshift algorithm for PRNG
static uint32_t
xorshift32()
{
    uint32_t x = __xorshift32_state;
    if (x == 0)
        x = 123456789;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    __xorshift32_state = x;
    return x;
}

static void
xorshift32_seed()
{
    // Seed the PRNG
#ifdef STABLE_SEED
    __xorshift32_state = 8675309;
#else
    __xorshift32_state = (unsigned int)time(NULL) ^ getpid();
#endif
}

/* convert upper case characters to lower case */
static char *
to_lower(char *str)
{
    char *p = str;
    for (; *p; ++p)
        *p = (char)(*p >= 'A' && *p <= 'Z' ? *p | 0x60 : *p);
    return str;
}

/* convert lower case characters to upper case */
static char *
to_upper(char *str)
{
    char *p = str;
    for (; *p; ++p)
        *p = (char)(*p >= 'a' && *p <= 'z' ? *p & ~0x20 : *p);
    return str;
}

/* convert a number into the Roman numeral equivalent, allocates a string caller must free */
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
    if (num > 10000) {
        sprintf(res, "The person you were looking for is not here, their mailbox is full, good bye.");
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

/* shuffle an array of length n */
static void
shuffle(int *array, size_t n)
{
    if (n > 1) {
        size_t i;
        for (i = n - 1; i > 0; i--) {
            size_t j = (unsigned int)(xorshift32() % (i + 1)); /* NOLINT(*-msc50-cpp) */
            int t = array[j];
            array[j] = array[i];
            array[i] = t;
        }
    }
}

#ifdef TODO_RESTORE_SNAPSHOTS
typedef struct {
    size_t length;
    size_t key;
    size_t snap_id;
} snap_info_t;
#endif

// ---------------------------------------------------------------------------
int
main()
{
    int rc;
#ifdef TODO_RESTORE_SNAPSHOTS
    size_t n_snaps = 0;
    snap_info_t snaps[TEST_ARRAY_SIZE * 2 + 1];
#endif

    xorshift32_seed();

#ifdef DOT
    of = fopen("/tmp/ex2.dot", "w");
    if (!of) {
        perror("Failed to open file /tmp/ex2.dot");
        return 1;
    }
#endif

    /* Allocate and initialize a Skiplist. */
    ex_t *list = (ex_t *)malloc(sizeof(ex_t));
    if (list == NULL)
        return ENOMEM;

    rc = api_skip_init_ex(list);
    if (rc)
        return rc;

        /* Set the PRNG state to a known constant for reproducible generation, easing debugging. */
#ifdef STABLE_SEED
    list->slh_prng_state = 12;
#endif
#ifdef SNAPSHOTS
    api_skip_snapshots_init_ex(list);
#endif
#ifdef DOT
    api_skip_dot_ex(of, list, gen++, "init", sprintf_ex_node);
#endif
    if (api_skip_get_ex(list, 0) != NULL)
        perror("found a non-existent item!");
    api_skip_del_ex(list, 0);
    CHECK;

    /* Insert TEST_ARRAY_SIZE key/value pairs into the list. */
    int i, j;
    char *numeral;
#ifdef DOT
    char msg[1024];
    memset(msg, 0, 1024);
#endif
    int amt = TEST_ARRAY_SIZE, asz = (amt * 2) + 1;
    int array[(TEST_ARRAY_SIZE * 2) + 1];
    for (j = 0, i = -amt; i <= amt; i++, j++)
        array[j] = i;
    shuffle(array, asz);

    for (i = 0; i < asz; i++) {
#ifdef SNAPSHOTS
        api_skip_snapshot_ex(list);
#elif defined(TODO_RESTORE_SNAPSHOTS)
        /* Snapshot the first iteration, and then every 5th after that. */
        if (i % 5 == 0) {
            snaps[i].length = api_skip_length_ex(list);
            snaps[i].key = array[i];
            snaps[i].snap_id = api_skip_snapshot_ex(list);
            n_snaps++;
            CHECK;
        }
#endif
        numeral = to_lower(int_to_roman_numeral(array[i]));
        rc = api_skip_put_ex(list, array[i], numeral);
        CHECK;
#ifdef DOT
        sprintf(msg, "put key: %d value: %s", array[i], numeral);
        api_skip_dot_ex(of, list, gen++, msg, sprintf_ex_node);
#endif
        char *v = api_skip_get_ex(list, array[i]);
#ifdef DOT
        sprintf(msg, "get key: %d", array[i]);
        api_skip_dot_ex(of, list, gen++, msg, sprintf_ex_node);
#endif
        CHECK;
        char *upper_numeral = calloc(1, strlen(v) + 1);
        strncpy(upper_numeral, v, strlen(v));
        assert(strncmp(v, upper_numeral, strlen(upper_numeral)) == 0);
        to_upper(upper_numeral);
        api_skip_set_ex(list, array[i], upper_numeral);
#ifdef DOT
        sprintf(msg, "set key: %d value: %s", array[i], upper_numeral);
        api_skip_dot_ex(of, list, gen++, msg, sprintf_ex_node);
#endif
        CHECK;

        for (size_t j = 0, k = api_skip_length_ex(list); j < k; j++) {
            int n = xorshift32() % api_skip_length_ex(list);
            api_skip_contains_ex(list, array[n]);
            CHECK;
        }
#if 0
        sprintf(msg, "locate all the keys!!!");
        api_skip_dot_ex(of, list, gen++, msg, sprintf_ex_node);
#endif
    }
    numeral = int_to_roman_numeral(-1);
    api_skip_dup_ex(list, -1, numeral);
    CHECK;
#ifdef DOT
    sprintf(msg, "put dup key: %d value: %s", -1, numeral);
    api_skip_dot_ex(of, list, gen++, msg, sprintf_ex_node);
#endif
    numeral = int_to_roman_numeral(1);
    api_skip_dup_ex(list, 1, numeral);
    CHECK;
#ifdef DOT
    sprintf(msg, "put dup key: %d value: %s", 1, numeral);
    api_skip_dot_ex(of, list, gen++, msg, sprintf_ex_node);
#endif

    api_skip_del_ex(list, 0);
    CHECK;
#ifdef DOT
    sprintf(msg, "deleted key: %d, value: %s", 0, numeral);
    api_skip_dot_ex(of, list, gen++, msg, sprintf_ex_node);
#endif
    if (api_skip_get_ex(list, 0) != NULL)
        perror("found a deleted item!");
    api_skip_del_ex(list, 0);
    CHECK;
    if (api_skip_get_ex(list, 0) != NULL)
        perror("found a deleted item!");
    int key = TEST_ARRAY_SIZE + 1;
    api_skip_del_ex(list, key);
    CHECK;
    key = -(TEST_ARRAY_SIZE)-1;
    api_skip_del_ex(list, key);
    CHECK;

    numeral = int_to_roman_numeral(-(TEST_ARRAY_SIZE));
    assert(strcmp(api_skip_pos_ex(list, SKIP_GTE, -(TEST_ARRAY_SIZE)-1)->value, numeral) == 0);
    free(numeral);
    numeral = int_to_roman_numeral(-2);
    assert(strcmp(api_skip_pos_ex(list, SKIP_GTE, -2)->value, numeral) == 0);
    free(numeral);
    numeral = int_to_roman_numeral(1);
    assert(strcmp(api_skip_pos_ex(list, SKIP_GTE, 0)->value, numeral) == 0);
    free(numeral);
    numeral = int_to_roman_numeral(2);
    assert(strcmp(api_skip_pos_ex(list, SKIP_GTE, 2)->value, numeral) == 0);
    free(numeral);
    assert(api_skip_pos_ex(list, SKIP_GTE, (TEST_ARRAY_SIZE + 1)) == NULL);

    numeral = int_to_roman_numeral(-(TEST_ARRAY_SIZE));
    assert(strcmp(api_skip_pos_ex(list, SKIP_GT, -(TEST_ARRAY_SIZE)-1)->value, numeral) == 0);
    free(numeral);
    numeral = int_to_roman_numeral(-1);
    assert(strcmp(api_skip_pos_ex(list, SKIP_GT, -2)->value, numeral) == 0);
    free(numeral);
    numeral = int_to_roman_numeral(1);
    assert(strcmp(api_skip_pos_ex(list, SKIP_GT, 0)->value, numeral) == 0);
    free(numeral);
    numeral = int_to_roman_numeral(2);
    assert(strcmp(api_skip_pos_ex(list, SKIP_GT, 1)->value, numeral) == 0);
    free(numeral);
    assert(api_skip_pos_ex(list, SKIP_GT, TEST_ARRAY_SIZE) == NULL);

    assert(api_skip_pos_ex(list, SKIP_LT, -(TEST_ARRAY_SIZE)) == NULL);
    numeral = int_to_roman_numeral(-2);
    assert(strcmp(api_skip_pos_ex(list, SKIP_LT, -1)->value, numeral) == 0);
    free(numeral);
    numeral = int_to_roman_numeral(-1);
    assert(strcmp(api_skip_pos_ex(list, SKIP_LT, 0)->value, numeral) == 0);
    free(numeral);
    numeral = int_to_roman_numeral(1);
    assert(strcmp(api_skip_pos_ex(list, SKIP_LT, 2)->value, numeral) == 0);
    free(numeral);
    numeral = int_to_roman_numeral(TEST_ARRAY_SIZE);
    assert(strcmp(api_skip_pos_ex(list, SKIP_LT, (TEST_ARRAY_SIZE + 1))->value, numeral) == 0);
    free(numeral);

    assert(api_skip_pos_ex(list, SKIP_LTE, -(TEST_ARRAY_SIZE)-1) == NULL);
    numeral = int_to_roman_numeral(-2);
    assert(strcmp(api_skip_pos_ex(list, SKIP_LTE, -2)->value, numeral) == 0);
    free(numeral);
    numeral = int_to_roman_numeral(-1);
    assert(strcmp(api_skip_pos_ex(list, SKIP_LTE, 0)->value, numeral) == 0);
    free(numeral);
    numeral = int_to_roman_numeral(2);
    assert(strcmp(api_skip_pos_ex(list, SKIP_LTE, 2)->value, numeral) == 0);
    free(numeral);
    numeral = int_to_roman_numeral(TEST_ARRAY_SIZE);
    assert(strcmp(api_skip_pos_ex(list, SKIP_LTE, (TEST_ARRAY_SIZE + 1))->value, numeral) == 0);
    free(numeral);

#ifdef TODO_RESTORE_SNAPSHOTS
    // Walk backward by 2 and test snapshot restore.
    for (i = n_snaps; i > 0; i -= 2) {
        api_skip_restore_snapshot_ex(list, snaps[i].snap_id);
        CHECK;
        assert(api_skip_length_ex(list) == snaps[i].length);
        numeral = int_to_roman_numeral(snaps[i].key);
        assert(strncmp(api_skip_get_ex(list, snaps[i].key), numeral, strlen(numeral)) == 0);
        free(numeral);
    }
    api_skip_release_snapshots_ex(list);
#endif

    for (size_t i = 0; i < 1000000; i++) {
        for (size_t j = 0, k = api_skip_length_ex(list); j < k; j++) {
            int n = xorshift32() % (api_skip_length_ex(list) - 1);
            api_skip_contains_ex(list, array[n]);
            CHECK;
        }
    }
#ifdef DOT
    sprintf(msg, "lots-o-contains later...");
    api_skip_dot_ex(of, list, gen++, msg, sprintf_ex_node);
#endif

#ifdef DOT
    api_skip_dot_end_ex(of, gen);
    fclose(of);
#endif
    api_skip_free_ex(list);
    free(list);
    return rc;
}
#pragma GCC pop_options
