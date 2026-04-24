#pragma GCC push_options
#pragma GCC optimize("O0")

// OPTIONS to set before including sl.h
// ---------------------------------------------------------------------------
#define DEBUG
#define SKIPLIST_DIAGNOSTIC

// Include our monolithic ADT, the Skiplist!
// ---------------------------------------------------------------------------
#include <pthread.h>

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

/*
 * Optional: Epoch-Based Reclamation (EBR) for lock-free concurrent access
 *
 * Generates EBR functions that allow multiple threads to safely read and
 * modify the skiplist concurrently.  Threads "pin" before accessing the
 * list and "unpin" when done; nodes removed while readers are pinned
 * are deferred until all readers have advanced past that epoch.
 */
SKIPLIST_DECL_EBR(ex, api_)

static void
sprintf_ex_node(ex_node_t *node, char *buf)
{
    sprintf(buf, "%d:%s", node->key, node->value);
}

// Function for this demo application.
// ---------------------------------------------------------------------------

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

/* calculate the floor of the log base 2 of a number m (⌊log2(m)⌋) */
static int
floor_log2(unsigned int m)
{
    return (int)floor(log(m) / log(2));
}

#ifdef TODO_RESTORE_SNAPSHOTS
typedef struct {
    size_t length;
    size_t key;
    size_t snap_id;
} snap_info_t;
#endif

// ---------------------------------------------------------------------------

/* The head node's height is always 1 more than the tallest node, that location
   is where we store the total hits, or "m". */
#define splay_list_m(m) list->slh_head->entries.sle_levels[list->slh_head->entries.sle_height].hits

// ---------------------------------------------------------------------------
// Concurrent demo: demonstrates the lock-free API with EBR
// ---------------------------------------------------------------------------

#define CONC_NUM_THREADS 4
#define CONC_KEYS_PER_THREAD 100

typedef struct {
    ex_t *list;
    __skip_ebr_ex_t *ebr;
    int thread_id;
    int ebr_tid;
    int result;
} conc_thread_ctx_t;

/* Each thread inserts a disjoint range of keys, then searches for them. */
static void *
conc_thread_worker(void *arg)
{
    conc_thread_ctx_t *ctx = (conc_thread_ctx_t *)arg;
    int start = ctx->thread_id * CONC_KEYS_PER_THREAD;
    int end = start + CONC_KEYS_PER_THREAD;

    /* Phase 1: Insert keys. */
    for (int k = start; k < end; k++) {
        ex_node_t *node;
        int rc = api_skip_alloc_node_ex(&node);
        if (rc != 0) {
            ctx->result = -1;
            return NULL;
        }
        node->key = k;
        node->value = int_to_roman_numeral(k);

        api_skip_ebr_pin_ex(ctx->ebr, ctx->ebr_tid);
        rc = api_skip_insert_ex(ctx->list, node);
        api_skip_ebr_unpin_ex(ctx->ebr, ctx->ebr_tid);

        if (rc != 0) {
            free(node->value);
            api_skip_free_node_ex(ctx->list, node);
            free(node);
            ctx->result = -1;
            return NULL;
        }
    }

    /* Phase 2: Search for the keys we just inserted. */
    for (int k = start; k < end; k++) {
        ex_node_t query;
        memset(&query, 0, sizeof(query));
        query.key = k;

        api_skip_ebr_pin_ex(ctx->ebr, ctx->ebr_tid);
        ex_node_t *found = api_skip_position_eq_ex(ctx->list, &query);
        api_skip_ebr_unpin_ex(ctx->ebr, ctx->ebr_tid);

        if (found == NULL || found->key != k) {
            fprintf(stderr, "Thread %d: key %d not found!\n", ctx->thread_id, k);
            ctx->result = -1;
            return NULL;
        }
    }

    ctx->result = 0;
    return NULL;
}

static int
concurrent_demo(void)
{
    printf("\n--- Concurrent Demo: %d threads, %d keys each ---\n", CONC_NUM_THREADS, CONC_KEYS_PER_THREAD);

    /* Allocate and initialize a skiplist for concurrent use. */
    ex_t *list = (ex_t *)malloc(sizeof(ex_t));
    if (list == NULL)
        return ENOMEM;
    int rc = api_skip_init_ex(list);
    if (rc) {
        free(list);
        return rc;
    }

    /* Set up Epoch-Based Reclamation (EBR). */
    __skip_ebr_ex_t *ebr = (__skip_ebr_ex_t *)malloc(sizeof(__skip_ebr_ex_t));
    if (ebr == NULL) {
        api_skip_free_ex(list);
        free(list);
        return ENOMEM;
    }
    api_skip_ebr_init_ex(ebr);
    api_skip_ebr_attach_ex(list, ebr);

    /* Spawn threads. */
    pthread_t threads[CONC_NUM_THREADS];
    conc_thread_ctx_t ctxs[CONC_NUM_THREADS];

    for (int i = 0; i < CONC_NUM_THREADS; i++) {
        ctxs[i].list = list;
        ctxs[i].ebr = ebr;
        ctxs[i].thread_id = i;
        ctxs[i].ebr_tid = api_skip_ebr_register_ex(ebr);
        ctxs[i].result = -1;
        if (ctxs[i].ebr_tid < 0) {
            fprintf(stderr, "Failed to register EBR thread %d\n", i);
            api_skip_ebr_drain_ex(ebr);
            api_skip_free_ex(list);
            free(list);
            free(ebr);
            return -1;
        }
    }

    for (int i = 0; i < CONC_NUM_THREADS; i++)
        pthread_create(&threads[i], NULL, conc_thread_worker, &ctxs[i]);
    for (int i = 0; i < CONC_NUM_THREADS; i++)
        pthread_join(threads[i], NULL);

    /* Check results. */
    int failed = 0;
    for (int i = 0; i < CONC_NUM_THREADS; i++) {
        if (ctxs[i].result != 0) {
            fprintf(stderr, "Thread %d failed!\n", i);
            failed = 1;
        }
    }

    size_t total = (size_t)CONC_NUM_THREADS * CONC_KEYS_PER_THREAD;
    printf("List length after concurrent inserts: %zu (expected %zu)\n", api_skip_length_ex(list), total);

    if (api_skip_length_ex(list) != total)
        failed = 1;

    /* Clean up: drain EBR deferred frees, then free the list. */
    api_skip_ebr_drain_ex(ebr);
    api_skip_free_ex(list);
    free(list);
    free(ebr);

    printf("Concurrent demo %s.\n", failed ? "FAILED" : "passed");
    return failed ? -1 : 0;
}

// ---------------------------------------------------------------------------

int
main()
{
    int rc;
    char *numeral;
#ifdef DOT
    char msg[1024];
    memset(msg, 0, 1024);
#endif
#ifdef TODO_RESTORE_SNAPSHOTS
    size_t n_snaps = 0;
    snap_info_t snaps[TEST_ARRAY_SIZE * 2 + 1];
#endif

#ifdef DOT
    of = fopen("/tmp/ex1.dot", "w");
    if (!of) {
        perror("Failed to open file /tmp/ex1.dot");
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
    list->slh_prng_state = 12;
#ifdef SNAPSHOTS
    api_skip_snapshots_init_ex(list);
#endif
#ifdef DOT
    api_skip_dot_ex(of, list, gen++, "init", sprintf_ex_node);
#endif

    /* This example mirrors the example given in the paper about splay-lists
       to test implementation against research. */

    for (int i = 1; i < 8; i++) {
        numeral = int_to_roman_numeral(i);
        if ((rc = api_skip_put_ex(list, i, numeral)))
            perror("put failed");
#ifdef DOT
        sprintf(msg, "put key: %d value: %s", i, numeral);
        api_skip_dot_ex(of, list, gen++, msg, sprintf_ex_node);
#endif
    }
    CHECK;

    /* Now we're going to poke around the internals a bit to set things up.
       This first time around we're going to build the list by hand, later
       we'll ensure that we can build this shape using only API calls. */

    ex_node_t *head = list->slh_head;
    ex_node_t *tail = list->slh_tail;
    ex_node_t *node_1 = head->entries.sle_levels[0].next;
    ex_node_t *node_2 = node_1->entries.sle_levels[0].next;
    ex_node_t *node_3 = node_2->entries.sle_levels[0].next;
    ex_node_t *node_4 = node_3->entries.sle_levels[0].next;
    ex_node_t *node_5 = node_4->entries.sle_levels[0].next;
    ex_node_t *node_6 = node_5->entries.sle_levels[0].next;

    // Head/Tail-nodes are height 3, ...
    head->entries.sle_height = tail->entries.sle_height = 3;

    // Head-node
    head->entries.sle_levels[3].hits = 10;
    head->entries.sle_levels[2].hits = 5;
    head->entries.sle_levels[1].hits = 1;
    head->entries.sle_levels[0].hits = 1;
    head->entries.sle_levels[1].next = node_2;
    head->entries.sle_levels[2].next = node_6;
    head->entries.sle_levels[3].next = tail;

    // Tail-node
    tail->entries.sle_levels[3].hits = 0;
    tail->entries.sle_levels[2].hits = 0;
    tail->entries.sle_levels[1].hits = 0;
    tail->entries.sle_levels[0].hits = 1;
    tail->entries.sle_levels[1].next = tail;
    tail->entries.sle_levels[2].next = tail;
    tail->entries.sle_levels[3].next = tail;

    // First node has key "1", height "0", hits(0) = 1
    node_1->entries.sle_height = 0;
    node_1->entries.sle_levels[0].hits = 1;

    // Second node has key "2", height "1", hits(0) = 1, hits(1) = 0
    node_2->entries.sle_height = 1;
    node_2->entries.sle_levels[0].hits = 1;
    node_2->entries.sle_levels[1].hits = 0;
    node_2->entries.sle_levels[1].next = node_3;

    // Third node has key "3", height "1", hits(0) = 1, hits(1) = 2
    node_3->entries.sle_height = 1;
    node_3->entries.sle_levels[0].hits = 1;
    node_3->entries.sle_levels[1].hits = 2;
    node_3->entries.sle_levels[1].next = node_6;

    // Fourth node has key "4", height "0", hits(0) = 1
    node_4->entries.sle_height = 0;
    node_4->entries.sle_levels[0].hits = 1;

    // Fifth node has key "5", height "0", hits(0) = 1
    node_5->entries.sle_height = 0;
    node_5->entries.sle_levels[0].hits = 1;

    // Sixth node has key "6", height "2", hits(0) = 5, hits(1) = 0, hits(2) = 0
    node_6->entries.sle_height = 2;
    node_6->entries.sle_levels[0].hits = 5;
    node_6->entries.sle_levels[1].hits = 0;
    node_6->entries.sle_levels[2].hits = 0;
    node_6->entries.sle_levels[1].next = tail;
    node_6->entries.sle_levels[2].next = tail;

    CHECK;

#ifdef DOT
    sprintf(msg, "manually adjusted");
    api_skip_dot_ex(of, list, gen++, msg, sprintf_ex_node);
#endif

    printf("m = %ld; ", splay_list_m(list));
    printf("(⌊log2(m)⌋) = %d\n", floor_log2(splay_list_m(list)));

    if (!(rc = api_skip_contains_ex(list, 5)))
        perror("missing element 5");

#ifdef DOT
    sprintf(msg, "contains(5)");
    api_skip_dot_ex(of, list, gen++, msg, sprintf_ex_node);
#endif

#ifdef DOT
    api_skip_dot_end_ex(of, gen);
    fclose(of);
#endif
    api_skip_free_ex(list);
    free(list);

    /* Run the concurrent lock-free demo. */
    int conc_rc = concurrent_demo();
    if (conc_rc != 0)
        return conc_rc;

    return rc;
}
#pragma GCC pop_options
