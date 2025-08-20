/*
 * Copyright (c) 2024
 *	Gregory Burd <greg@burd.me>.  All rights reserved.
 *
 * ISC License Permission to use, copy, modify, and/or distribute this software
 * for any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * I'd like to thank others for thoughtfully licensing their work, the
 * community of software engineers succeeds when we work together.
 *
 * Portions of this code are derived from other copyrighted works:
 *
 *  - MIT License
 *    - https://github.com/greensky00/skiplist
 *      2017-2024 Jung-Sang Ahn <jungsang.ahn@gmail.com>
 *    - https://github.com/paulross/skiplist
 *      Copyright (c) 2017-2023 Paul Ross <paulross@uky.edu>
 *    - https://github.com/JP-Ellis/rust-skiplist
 *      Copyright (c) 2015 Joshua Ellis <github@jpellis.me>
 *  - Public Domain
 *    - https://gist.github.com/zhpengg/2873424
 *      Zhipeng Li <zhpeng.is@gmail.com>
 */

#define _USE_MATH_DEFINES // needed to have definition of M_LOG2E
#define _GNU_SOURCE

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <assert.h>
#include <errno.h>
#include <math.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#pragma GCC diagnostic pop

#ifndef _SKIPLIST_H_
#define _SKIPLIST_H_

/*
 * This file defines a skip-list data structure written in C.  Implemented as
 * using macros this code provides a way to essentially "template" (as in C++)
 * and emit code with types and functions specific to your use case.  You can
 * apply these macros multiple times safely, once for each list type you need.
 *
 * A skip-list is a sorted list with O(log(n)) on average for most operations.
 * It is a probabilistic data structure, meaning that it does not guarantee
 * O(log(n)), but it has been shown to approximate it over time.  This
 * implementation includes the re-balancing techniques that improve on that
 * approximation using an adaptive technique called "splay-list". It is similar
 * to a standard skip-list, with the key distinction that the height of each
 * element adapts dynamically to its access rate: popular elements increase in
 * height, whereas rarely-accessed elements decrease in height. See below for
 * the link to the research behind this adaptive technique.
 *
 * Conceptually, at a high level, a skip-list is arranged as follows:
 *
 * <head> ----------> [2] --------------------------------------------------> [9] ---------->
 * <head> ----------> [2] ------------------------------------[7] ----------> [9] ---------->
 * <head> ----------> [2] ----------> [4] ------------------> [7] ----------> [9] --> [10] ->
 * <head> --> [1] --> [2] --> [3] --> [4] --> [5] --> [6] --> [7] --> [8] --> [9] --> [10] ->
 *
 * Each node contains at the very least a link to the next element in the list
 * (corresponding to the lowest level in the above diagram), but it can randomly
 * contain more links which skip further down the list (the towers in the above
 * diagram). This allows for the algorithm to move down the list faster than
 * having to visit every element.
 *
 * A skip-list can be thought of as a stack of linked lists. At the very bottom
 * is a linked list with every element, and each layer above corresponds to a
 * linked list containing a random subset of the elements from the layer
 * immediately below it. The probability distribution that determines this
 * random subset can be customized, but typically a layer will contain half the
 * nodes from the layer below.
 *
 * This implementation maintains a doubly-linked list at the bottom layer to
 * support efficient iteration in either direction.  There is also a guard node
 * at the tail rather than simply pointing to NULL.
 *
 * <head> <-> [1] <-> [2] <-> [3] <-> [4] <-> [5] <-> [6] <-> [7] <-> <tail>
 *
 * Safety:
 *
 * The ordered skip-list relies on a well-behaved comparison
 * function. Specifically, given some ordering function f(a, b), it must satisfy
 * the following properties:
 *
 * 1) Be well defined: f(a, b) should always return the same value
 * 2) Be anti-symmetric: f(a, b) == Greater if and only if f(b, a) == Less, and
 *    f(a, b) == Equal == f(b, a).
 * 3) Be transitive: If f(a, b) == Greater and f(b, c) == Greater then f(a, c)
 *    == Greater.
 *
 * Failure to satisfy these properties can result in unexpected behavior at
 * best, and at worst will cause a segfault, null deref, or some other bad
 * behavior.
 *
 * References for this implementation include, but are not limited to:
 *
 *  - Skip lists: a probabilistic alternative to balanced trees
 *    @article{10.1145/78973.78977,
 *      author = {Pugh, William},
 *      title = {Skip lists: a probabilistic alternative to balanced trees},
 *      year = {1990}, issue_date = {June 1990},
 *      publisher = {Association for Computing Machinery},
 *      address = {New York, NY, USA},
 *      volume = {33}, number = {6}, issn = {0001-0782},
 *      url = {https://doi.org/10.1145/78973.78977},
 *      doi = {10.1145/78973.78977},
 *      journal = {Commun. ACM}, month = {jun}, pages = {668-676}, numpages = {9},
 *      keywords = {trees, searching, data structures},
 *      download = {https://www.cl.cam.ac.uk/teaching/2005/Algorithms/skiplists.pdf}
 *    }
 *
 *  - Tutorial: The Ubiquitous Skiplist, its Variants, and Applications in Modern Big Data Systems
 *    @article{Vadrevu2023TutorialTU,
 *      title={Tutorial: The Ubiquitous Skiplist, its Variants, and Applications in Modern Big Data Systems},
 *      author={Venkata Sai Pavan Kumar Vadrevu and Lu Xing and Walid G. Aref},
 *      journal={ArXiv},
 *      year={2023},
 *      volume={abs/2304.09983},
 *      url={https://api.semanticscholar.org/CorpusID:258236678},
 *      download={https://arxiv.org/pdf/2304.09983.pdf}
 *    }
 *
 *  - The Splay-List: A Distribution-Adaptive Concurrent Skip-List
 *    @misc{aksenov2020splaylist,
 *      title={The Splay-List: A Distribution-Adaptive Concurrent Skip-List},
 *      author={Vitaly Aksenov and Dan Alistarh and Alexandra Drozdova and Amirkeivan Mohtashami},
 *      year={2020},
 *      eprint={2008.01009},
 *      archivePrefix={arXiv},
 *      primaryClass={cs.DC},
 *      download={https://arxiv.org/pdf/2008.01009.pdf}
 *    }
 *
 *  - JellyFish: A Fast Skip List with MVCC},
 *    @article{Yeon2020JellyFishAF,
 *      title={JellyFish: A Fast Skip List with MVCC},
 *      author={Jeseong Yeon and Leeju Kim and Youil Han and Hyeon Gyu Lee and Eunji Lee and Bryan Suk Joon Kim},
 *      journal={Proceedings of the 21st International Middleware Conference},
 *      year={2020},
 *      url={https://api.semanticscholar.org/CorpusID:228086012}
 *    }
 */

#if defined(SKIPLIST_DIAGNOSTIC) && defined(DEBUG)

#ifndef SKIP_DIAG
#define SKIP_DIAG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wvariadic-macros"
#define __skip_diag(format, ...) __skip_diag_(__FILE__, __LINE__, __func__, format, ##__VA_ARGS__)
#pragma GCC diagnostic pop

/**                                                                                                                                                     \
 * -- __skip_diag__                                                                                                                                     \
 *                                                                                                                                                      \
 * Write debug message to stderr with origin of message.                                                                                                \
 */
void __attribute__((format(printf, 4, 5))) __skip_diag_(const char *file, int line, const char *func, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    fprintf(stderr, "%s:%d:%s(): ", file, line, func);
    vfprintf(stderr, format, args);
    va_end(args);
}

#else
#define __skip_diag(file, line, func, format, ...) ((void)0)
#endif

#ifndef SKIP_ASSERT
#define SKIP_ASSERT
#define __skip_assert(expr) \
    if (!(expr))            \
    fprintf(stderr, "%s:%d:%s(): assertion failed! %s", __FILE__, __LINE__, __func__, #expr)
#else
#define __skip_assert(expr) ((void)0)
#endif
#endif

/*
 * Skiplist declarations.
 */

#ifndef SKIPLIST_MAX_HEIGHT
#define SKIPLIST_MAX_HEIGHT 64
#endif

/*
 * Every Skiplist node has to have an additional section of data used to manage
 * nodes in the list. The rest of the data structure is defined by the use case.
 * This housekeeping portion is the SKIPLIST_ENTRY, see below.  It maintains the
 * array of forward pointers to nodes and has a height, this height is a
 * zero-based count of levels, so a height of `0` means one (1) level and a
 * height of `4` means five (5) forward pointers (levels) in the node, [0-4).
 */
#define SKIPLIST_ENTRY(decl)               \
    struct __skiplist_##decl##_entry {     \
        size_t sle_era;                    \
        size_t sle_height;                 \
        struct decl##_node *sle_prev;      \
        struct __skiplist_##decl##_level { \
            struct decl##_node *next;      \
            size_t hits;                   \
        } *sle_levels;                     \
    }

#define SKIPLIST_FOREACH_H2T(decl, prefix, field, list, elm, iter) \
    for (iter = 0, (elm) = (list)->slh_head; ((elm) = (elm)->field.sle_levels[0].next) != (list)->slh_tail; iter++)

/* Iterate from tail to head over the nodes. */
#define SKIPLIST_FOREACH_T2H(decl, prefix, field, list, elm, iter) \
    for (iter = (list)->slh_length, (elm) = (list)->slh_tail; ((elm) = (elm)->field.sle_prev) != (list)->slh_head; iter--)

/* Iterate over the next pointers in a node from bottom to top (B2T) or top to bottom (T2B). */
#define __SKIP_ALL_ENTRIES_T2B(field, elm) for (size_t lvl = slist->slh_head->field.sle_height - 1; lvl != (size_t)-1; lvl--)
#define __SKIP_ENTRIES_T2B(field, elm) for (size_t lvl = elm->field.sle_height; lvl != (size_t)-1; lvl--)
#define __SKIP_ENTRIES_T2B_FROM(field, elm, off) for (size_t lvl = off; lvl != (size_t)-1; lvl--)
#define __SKIP_IS_LAST_ENTRY_T2B() if (lvl == 0)

#define __SKIP_ALL_ENTRIES_B2T(field, elm) for (size_t lvl = 0; lvl < slist->slh_head->field.sle_height - 1; lvl++)
#define __SKIP_ENTRIES_B2T(field, elm) for (size_t lvl = 0; lvl <= elm->field.sle_height; lvl++)
#define __SKIP_ENTRIES_B2T_FROM(field, elm, off) for (size_t lvl = off; lvl <= elm->field.sle_height; lvl++)
#define __SKIP_IS_LAST_ENTRY_B2T() if (lvl + 1 == elm->field.sle_height)

/* Iterate over the left (v) subtree or right (u) subtree or "CHu" and "CHv". */
#define __SKIP_SUBTREE_CHv(decl, field, list, path, nth) \
    for (decl##_node_t *elm = path[nth].node; elm->field.sle_levels[path[nth].in].next == path[nth].node; elm = elm->field.sle_prev)
#define __SKIP_SUBTREE_CHu(decl, field, list, path, nth) \
    for (decl##_node_t *elm = path[nth].node; elm != path[nth].node->field.sle_levels[0].next; elm = elm->field.sle_levels[0].next)
/* Iterate over a subtree starting at provided path element, u = path.in */
#define __SKIP_SUBTREE_CHux(decl, field, list, node, level) \
    for (decl##_node_t *elm = node->field.sle_levels[level].next->field.sle_prev; elm != node->field.sle_prev; elm = elm->field.sle_prev)

/*
 * Skiplist declarations and access methods.
 */
#define SKIPLIST_DECL(decl, prefix, field, compare_entries_blk, free_entry_blk, update_entry_blk, archive_entry_blk, sizeof_entry_blk)   \
                                                                                                                                         \
    /* Used when positioning a cursor within a Skiplist. */                                                                              \
    typedef enum { SKIP_EQ = 0, SKIP_LTE = -1, SKIP_LT = -2, SKIP_GTE = 1, SKIP_GT = 2 } skip_pos_##decl_t;                              \
                                                                                                                                         \
    /* Skiplist node type */                                                                                                             \
    typedef struct decl##_node decl##_node_t;                                                                                            \
                                                                                                                                         \
    /* Skiplist type. */                                                                                                                 \
    typedef struct decl decl##_t;                                                                                                        \
                                                                                                                                         \
    /* Skiplist structure */                                                                                                             \
    struct decl {                                                                                                                        \
        size_t slh_length;                                                                                                               \
        void *slh_aux;                                                                                                                   \
        int slh_prng_state;                                                                                                              \
        decl##_node_t *slh_head;                                                                                                         \
        decl##_node_t *slh_tail;                                                                                                         \
        struct {                                                                                                                         \
            void (*free_entry)(decl##_node_t *);                                                                                         \
            int (*update_entry)(decl##_node_t *, void *);                                                                                \
            int (*archive_entry)(decl##_node_t *, const decl##_node_t *);                                                                \
            size_t (*sizeof_entry)(decl##_node_t *);                                                                                     \
            int (*compare_entries)(decl##_t *, decl##_node_t *, decl##_node_t *, void *);                                                \
                                                                                                                                         \
            /* Optional: Snapshots */                                                                                                    \
            int (*snapshot_preserve_node)(decl##_t * slist, const decl##_node_t *src, decl##_node_t **preserved);                        \
            void (*snapshot_release)(decl##_t *);                                                                                        \
        } slh_fns;                                                                                                                       \
        struct {                                                                                                                         \
            size_t cur_era;                                                                                                              \
            size_t pres_era;                                                                                                             \
            decl##_node_t *pres;                                                                                                         \
        } slh_snap;                                                                                                                      \
    };                                                                                                                                   \
                                                                                                                                         \
    typedef struct __skiplist_path_##decl {                                                                                              \
        decl##_node_t *node; /* node traversed in the act of location */                                                                 \
        size_t in;           /* level at which the node was intersected */                                                               \
        size_t pu;           /* see "partial sums trick" */                                                                              \
    } __skiplist_path_##decl##_t;                                                                                                        \
                                                                                                                                         \
    /* Xorshift algorithm for PRNG */                                                                                                    \
    static uint32_t __##decl##_xorshift32(int *state)                                                                                    \
    {                                                                                                                                    \
        uint32_t x = *state;                                                                                                             \
        if (x == 0)                                                                                                                      \
            x = 123456789;                                                                                                               \
        x ^= x << 13;                                                                                                                    \
        x ^= x >> 17;                                                                                                                    \
        x ^= x << 5;                                                                                                                     \
        *state = x;                                                                                                                      \
        return x;                                                                                                                        \
    }                                                                                                                                    \
                                                                                                                                         \
    /**                                                                                                                                  \
     * -- __skip_compare_entries_fn_                                                                                                     \
     *                                                                                                                                   \
     * Wraps the `compare_entries_blk` code into `slh_fns.compare_entries`.                                                              \
     */                                                                                                                                  \
    static int __skip_compare_entries_fn_##decl(decl##_t *list, decl##_node_t *a, decl##_node_t *b, void *aux)                           \
    {                                                                                                                                    \
        compare_entries_blk;                                                                                                             \
    }                                                                                                                                    \
                                                                                                                                         \
    /**                                                                                                                                  \
     * -- __skip_free_entry_fn                                                                                                           \
     *                                                                                                                                   \
     * Wraps the `free_entry_blk` code into `slh_fns.free_entry`.                                                                        \
     */                                                                                                                                  \
    static void __skip_free_entry_fn_##decl(decl##_node_t *node)                                                                         \
    {                                                                                                                                    \
        free_entry_blk;                                                                                                                  \
    }                                                                                                                                    \
                                                                                                                                         \
    /**                                                                                                                                  \
     * -- __skip_update_entry_fn_                                                                                                        \
     *                                                                                                                                   \
     * Wraps the `update_entry_blk` code into `slh_fns.update_entry`.                                                                    \
     */                                                                                                                                  \
    static int __skip_update_entry_fn_##decl(decl##_node_t *node, void *value)                                                           \
    {                                                                                                                                    \
        int rc = 0;                                                                                                                      \
        update_entry_blk;                                                                                                                \
        return rc;                                                                                                                       \
    }                                                                                                                                    \
                                                                                                                                         \
    /**                                                                                                                                  \
     * -- __skip_archive_entry_fn_                                                                                                       \
     *                                                                                                                                   \
     * Wraps the `archive_entry_blk` code into `slh_fns.archive_entry`.                                                                  \
     */                                                                                                                                  \
    static int __skip_archive_entry_fn_##decl(decl##_node_t *dest, const decl##_node_t *src)                                             \
    {                                                                                                                                    \
        int rc = 0;                                                                                                                      \
        archive_entry_blk;                                                                                                               \
        return rc;                                                                                                                       \
    }                                                                                                                                    \
                                                                                                                                         \
    /**                                                                                                                                  \
     * -- __skip_sizeof_entry_fn_                                                                                                        \
     *                                                                                                                                   \
     * Wraps the `sizeof_entry_blk` code into `slh_fns.sizeof_entry`.                                                                    \
     */                                                                                                                                  \
    static size_t __skip_sizeof_entry_fn_##decl(decl##_node_t *node)                                                                     \
    {                                                                                                                                    \
        size_t bytes = 0;                                                                                                                \
        sizeof_entry_blk;                                                                                                                \
        return bytes;                                                                                                                    \
    }                                                                                                                                    \
                                                                                                                                         \
    /**                                                                                                                                  \
     * -- __skip_compare_nodes_                                                                                                          \
     *                                                                                                                                   \
     * This function takes four arguments:                                                                                               \
     *   - a reference to the Skiplist                                                                                                   \
     *   - the two nodes to compare, `a` and `b`                                                                                         \
     *   - `aux` an additional auxiliary argument                                                                                        \
     * and returns:                                                                                                                      \
     *   a  < b : return -1                                                                                                              \
     *   a == b : return 0                                                                                                               \
     *   a  > b : return 1                                                                                                               \
     */                                                                                                                                  \
    static int __skip_compare_nodes_##decl(decl##_t *slist, decl##_node_t *a, decl##_node_t *b, void *aux)                               \
    {                                                                                                                                    \
        if (a == b)                                                                                                                      \
            return 0;                                                                                                                    \
        if (a == NULL)                                                                                                                   \
            return -1;                                                                                                                   \
        if (b == NULL)                                                                                                                   \
            return 1;                                                                                                                    \
        if (a == slist->slh_head || b == slist->slh_tail)                                                                                \
            return -1;                                                                                                                   \
        if (a == slist->slh_tail || b == slist->slh_head)                                                                                \
            return 1;                                                                                                                    \
        return slist->slh_fns.compare_entries(slist, a, b, aux);                                                                         \
    }                                                                                                                                    \
                                                                                                                                         \
    /**                                                                                                                                  \
     * -- __skip_toss_                                                                                                                   \
     *                                                                                                                                   \
     * A "coin toss" function that is critical to the proper operation of the                                                            \
     * Skiplist.  For example, when `max = 6` this function returns 0 with                                                               \
     * probability 0.5, 1 with 0.25, 2 with 0.125, etc. until 6 with 0.5^7.                                                              \
     */                                                                                                                                  \
    static int __skip_toss_##decl(decl##_t *slist, size_t max)                                                                           \
    {                                                                                                                                    \
        size_t level = 0;                                                                                                                \
        double probability = 0.5;                                                                                                        \
                                                                                                                                         \
        double random_value = (double)__##decl##_xorshift32(&slist->slh_prng_state) / RAND_MAX;                                          \
        while (random_value < probability && level < max) {                                                                              \
            level++;                                                                                                                     \
            probability *= 0.5;                                                                                                          \
        }                                                                                                                                \
        return level;                                                                                                                    \
    }                                                                                                                                    \
                                                                                                                                         \
    /**                                                                                                                                  \
     * -- skip_alloc_node_                                                                                                               \
     *                                                                                                                                   \
     * Allocates a new node on the heap and sets default values.                                                                         \
     */                                                                                                                                  \
    int prefix##skip_alloc_node_##decl(decl##_node_t **node)                                                                             \
    {                                                                                                                                    \
        decl##_node_t *n;                                                                                                                \
        /* Calculate the size of the struct sle within decl##_node_t, multiply                                                           \
           by array size. (16/24 bytes on 32/64-bit systems) */                                                                          \
        size_t sle_arr_sz = sizeof(struct __skiplist_##decl##_level) * SKIPLIST_MAX_HEIGHT;                                              \
        n = (decl##_node_t *)calloc(1, sizeof(decl##_node_t) + sle_arr_sz);                                                              \
        if (n == NULL)                                                                                                                   \
            return ENOMEM;                                                                                                               \
        n->field.sle_height = 0;                                                                                                         \
        n->field.sle_levels = (struct __skiplist_##decl##_level *)((uintptr_t)n + sizeof(decl##_node_t));                                \
        *node = n;                                                                                                                       \
        return 0;                                                                                                                        \
    }                                                                                                                                    \
                                                                                                                                         \
    /**                                                                                                                                  \
     * -- skip_init_                                                                                                                     \
     *                                                                                                                                   \
     * Initializes a Skiplist to the default values, this must be                                                                        \
     * called before using the list.                                                                                                     \
     */                                                                                                                                  \
    int prefix##skip_init_##decl(decl##_t *slist)                                                                                        \
    {                                                                                                                                    \
        int rc = 0;                                                                                                                      \
        size_t i;                                                                                                                        \
                                                                                                                                         \
        slist->slh_length = 0;                                                                                                           \
        slist->slh_snap.cur_era = 0;                                                                                                     \
        slist->slh_snap.pres_era = 0;                                                                                                    \
        slist->slh_snap.pres = 0;                                                                                                        \
        slist->slh_fns.free_entry = __skip_free_entry_fn_##decl;                                                                         \
        slist->slh_fns.update_entry = __skip_update_entry_fn_##decl;                                                                     \
        slist->slh_fns.archive_entry = __skip_archive_entry_fn_##decl;                                                                   \
        slist->slh_fns.sizeof_entry = __skip_sizeof_entry_fn_##decl;                                                                     \
        slist->slh_fns.compare_entries = __skip_compare_entries_fn_##decl;                                                               \
        rc = prefix##skip_alloc_node_##decl(&slist->slh_head);                                                                           \
        if (rc)                                                                                                                          \
            goto fail;                                                                                                                   \
        rc = prefix##skip_alloc_node_##decl(&slist->slh_tail);                                                                           \
        if (rc)                                                                                                                          \
            goto fail;                                                                                                                   \
                                                                                                                                         \
        /* Initial height at head is 0 (meaning 0 sub-lists), all next point to tail */                                                  \
        slist->slh_head->field.sle_height = 1;                                                                                           \
        for (i = 0; i < slist->slh_head->entries.sle_height + 1; i++)                                                                    \
            slist->slh_head->field.sle_levels[i].next = slist->slh_tail;                                                                 \
        slist->slh_head->field.sle_prev = NULL;                                                                                          \
                                                                                                                                         \
        /* Initial height at tail is also 0 and all next point to tail */                                                                \
        slist->slh_tail->field.sle_height = slist->slh_head->field.sle_height;                                                           \
        for (i = 0; i < slist->slh_head->entries.sle_height + 1; i++)                                                                    \
            slist->slh_tail->field.sle_levels[i].next = NULL;                                                                            \
        slist->slh_tail->field.sle_prev = slist->slh_head;                                                                               \
        slist->slh_prng_state = ((unsigned int)time(NULL) ^ getpid());                                                                   \
    fail:;                                                                                                                               \
        return rc;                                                                                                                       \
    }                                                                                                                                    \
                                                                                                                                         \
    /**                                                                                                                                  \
     * -- skip_free_node_                                                                                                                \
     *                                                                                                                                   \
     * Properly releases heap memory allocated for use as a node.                                                                        \
     * This function invokes the `free_node_blk` within which you                                                                        \
     * should release any heap objects or other resources held by                                                                        \
     * this node in the list.                                                                                                            \
     */                                                                                                                                  \
    void prefix##skip_free_node_##decl(decl##_t *slist, decl##_node_t *node)                                                             \
    {                                                                                                                                    \
        slist->slh_fns.free_entry(node);                                                                                                 \
        free(node);                                                                                                                      \
    }                                                                                                                                    \
                                                                                                                                         \
    /**                                                                                                                                  \
     * -- skip_length_                                                                                                                   \
     *                                                                                                                                   \
     * Returns the current length of the list.                                                                                           \
     */                                                                                                                                  \
    size_t prefix##skip_length_##decl(decl##_t *slist)                                                                                   \
    {                                                                                                                                    \
        return slist->slh_length;                                                                                                        \
    }                                                                                                                                    \
                                                                                                                                         \
    /**                                                                                                                                  \
     * -- skip_is_empty_                                                                                                                 \
     *                                                                                                                                   \
     * Returns non-zero when the list is empty.                                                                                          \
     */                                                                                                                                  \
    int prefix##skip_is_empty_##decl(decl##_t *slist)                                                                                    \
    {                                                                                                                                    \
        return slist->slh_length == 0;                                                                                                   \
    }                                                                                                                                    \
                                                                                                                                         \
    /**                                                                                                                                  \
     * -- skip_head_                                                                                                                     \
     *                                                                                                                                   \
     * Returns the node containing the first (smallest) element in the                                                                   \
     * list which can be used to traverse the list.                                                                                      \
     */                                                                                                                                  \
    decl##_node_t *prefix##skip_head_##decl(decl##_t *slist)                                                                             \
    {                                                                                                                                    \
        return slist->slh_head->field.sle_levels[0].next == slist->slh_tail ? NULL : slist->slh_head->field.sle_levels[0].next;          \
    }                                                                                                                                    \
                                                                                                                                         \
    /**                                                                                                                                  \
     * -- skip_tail_                                                                                                                     \
     *                                                                                                                                   \
     * Returns the node containing the last (largest) element in the                                                                     \
     * list which can be used to traverse the list.                                                                                      \
     */                                                                                                                                  \
    decl##_node_t *prefix##skip_tail_##decl(decl##_t *slist)                                                                             \
    {                                                                                                                                    \
        if (slist == NULL)                                                                                                               \
            return NULL;                                                                                                                 \
        return slist->slh_tail->field.sle_prev == slist->slh_head->field.sle_levels[0].next ? NULL : slist->slh_tail->field.sle_prev;    \
    }                                                                                                                                    \
                                                                                                                                         \
    /**                                                                                                                                  \
     * -- skip_next_node_                                                                                                                \
     *                                                                                                                                   \
     * A node reference can be thought of as a cursor.  This moves the cursor                                                            \
     * to the next node in the list or returns NULL if the next is the tail.                                                             \
     */                                                                                                                                  \
    decl##_node_t *prefix##skip_next_node_##decl(decl##_t *slist, decl##_node_t *n)                                                      \
    {                                                                                                                                    \
        if (slist == NULL || n == NULL)                                                                                                  \
            return NULL;                                                                                                                 \
        if (n->field.sle_levels[0].next == slist->slh_tail)                                                                              \
            return NULL;                                                                                                                 \
        return n->field.sle_levels[0].next;                                                                                              \
    }                                                                                                                                    \
                                                                                                                                         \
    /**                                                                                                                                  \
     * -- skip_prev_node_                                                                                                                \
     *                                                                                                                                   \
     * A node reference can be thought of as a cursor.  This moves the cursor                                                            \
     * to the previous node in the list or returns NULL if the previous node                                                             \
     * is the head.                                                                                                                      \
     */                                                                                                                                  \
    decl##_node_t *prefix##skip_prev_node_##decl(decl##_t *slist, decl##_node_t *n)                                                      \
    {                                                                                                                                    \
        if (slist == NULL || n == NULL)                                                                                                  \
            return NULL;                                                                                                                 \
        if (n->field.sle_prev == slist->slh_head)                                                                                        \
            return NULL;                                                                                                                 \
        return n->field.sle_prev;                                                                                                        \
    }                                                                                                                                    \
                                                                                                                                         \
    /**                                                                                                                                  \
     * -- skip_release_                                                                                                                  \
     *                                                                                                                                   \
     * Release all nodes and their associated heap objects, but not the list                                                             \
     * itself. The list is still valid, only empty.                                                                                      \
     */                                                                                                                                  \
    void prefix##skip_release_##decl(decl##_t *slist)                                                                                    \
    {                                                                                                                                    \
        decl##_node_t *node, *next;                                                                                                      \
                                                                                                                                         \
        if (slist == NULL)                                                                                                               \
            return;                                                                                                                      \
        if (prefix##skip_is_empty_##decl(slist))                                                                                         \
            return;                                                                                                                      \
        node = slist->slh_head->field.sle_levels[0].next;                                                                                \
        while (node != slist->slh_tail) {                                                                                                \
            next = node->field.sle_levels[0].next;                                                                                       \
            prefix##skip_free_node_##decl(slist, node);                                                                                  \
            node = next;                                                                                                                 \
        }                                                                                                                                \
        if (slist->slh_snap.pres_era > 0)                                                                                                \
            slist->slh_snap.cur_era++;                                                                                                   \
        return;                                                                                                                          \
    }                                                                                                                                    \
                                                                                                                                         \
    /**                                                                                                                                  \
     * -- skip_to_array_                                                                                                                 \
     *                                                                                                                                   \
     * Returns a heap allocated array of nodes in the order they exist.                                                                  \
     * This isn't maintained by the list, if you add/remove nodes it is                                                                  \
     * no longer accurate. At [-1] is the length of the array.                                                                           \
     * NOTE: Caller must deallocate.                                                                                                     \
     */                                                                                                                                  \
    decl##_node_t **prefix##skip_to_array_##decl(decl##_t *slist)                                                                        \
    {                                                                                                                                    \
        size_t nth, len = prefix##skip_length_##decl(slist);                                                                             \
        decl##_node_t *node, **nodes = NULL;                                                                                             \
        nodes = (decl##_node_t **)calloc(sizeof(decl##_node_t *), len + 1);                                                              \
        if (nodes != NULL) {                                                                                                             \
            nodes[0] = (decl##_node_t *)(uintptr_t)len;                                                                                  \
            nodes++;                                                                                                                     \
            SKIPLIST_FOREACH_H2T(decl, prefix, field, slist, node, nth)                                                                  \
            {                                                                                                                            \
                nodes[nth] = node;                                                                                                       \
            }                                                                                                                            \
        }                                                                                                                                \
        return nodes;                                                                                                                    \
    }                                                                                                                                    \
                                                                                                                                         \
    /**                                                                                                                                  \
     * -- __skip_adjust_hit_counts_ TODO                                                                                                 \
     *                                                                                                                                   \
     * On delete we check the hit counts across all nodes and next[] pointers                                                            \
     * and find the smallest counter then subtract that + 1 from all hit                                                                 \
     * counters.                                                                                                                         \
     *                                                                                                                                   \
     */                                                                                                                                  \
    static void __skip_adjust_hit_counts_##decl(decl##_t *slist)                                                                         \
    {                                                                                                                                    \
        ((void)slist);                                                                                                                   \
    }                                                                                                                                    \
                                                                                                                                         \
    /**                                                                                                                                  \
     * -- __skip_rebalance_                                                                                                              \
     *                                                                                                                                   \
     * Restore balance to our list by adjusting heights and forward pointers                                                             \
     * according to the algorithm put forth in "The Splay-List: A                                                                        \
     * Distribution-Adaptive Concurrent Skip-List".                                                                                      \
     *                                                                                                                                   \
     */                                                                                                                                  \
    static void __skip_rebalance_##decl(decl##_t *slist, size_t len, __skiplist_path_##decl##_t path[])                                  \
    {                                                                                                                                    \
        size_t i, lvl, u_hits, hits_CHu = 0, hits_CHv = 0, delta_height;                                                                 \
        size_t k_threshold, m_total_hits, expected_height;                                                                               \
        double asc_cond, dsc_cond;                                                                                                       \
        __skiplist_path_##decl##_t *p, path_u, path_v;                                                                                   \
        decl##_node_t *node;                                                                                                             \
                                                                                                                                         \
        /* Total hits, `k`, across all nodes. */                                                                                         \
        m_total_hits = slist->slh_head->field.sle_levels[slist->slh_head->field.sle_height].hits;                                        \
                                                                                                                                         \
        /* Should we adjust the height? */                                                                                               \
        if (m_total_hits > 0) {                                                                                                          \
            expected_height = floor(log(m_total_hits) / log(2));                                                                         \
            if (expected_height > slist->slh_head->field.sle_height && expected_height < SKIPLIST_MAX_HEIGHT - 1) {                      \
                slist->slh_head->field.sle_height++;                                                                                     \
                slist->slh_head->field.sle_levels[slist->slh_head->field.sle_height].next = slist->slh_tail;                             \
                slist->slh_head->field.sle_levels[slist->slh_head->field.sle_height].hits =                                              \
                    slist->slh_head->field.sle_levels[slist->slh_head->field.sle_height - 1].hits;                                       \
                slist->slh_head->field.sle_levels[slist->slh_head->field.sle_height - 1].hits = 0;                                       \
                slist->slh_tail->field.sle_height = slist->slh_head->field.sle_height;                                                   \
            }                                                                                                                            \
        }                                                                                                                                \
                                                                                                                                         \
        /* Height of the head node, should be equal to floor(log(m_total_hits)). */                                                      \
        k_threshold = slist->slh_head->field.sle_height;                                                                                 \
                                                                                                                                         \
        /* Moving backwards along the path...                                                                                            \
         *  - path[0] contains a match, if there was one                                                                                 \
         *  - path[1..len] will be the nodes traversed along the way                                                                     \
         *  - path[len] is where the locate() terminated, just before path[0]                                                            \
         *    if there was a match                                                                                                       \
         */                                                                                                                              \
        p = &path[0];                                                                                                                    \
        for (i = 0; i < len; i++, p++) {                                                                                                 \
            hits_CHu = hits_CHv = 0;                                                                                                     \
            path_u = *p;                                                                                                                 \
            if (path_u.node == slist->slh_head || path_u.node == slist->slh_tail)                                                        \
                continue;                                                                                                                \
            path_v = *(p + 1);                                                                                                           \
                                                                                                                                         \
            /* Evaluate the subtree 'u'. */                                                                                              \
            __SKIP_SUBTREE_CHux(decl, field, slist, path_u.node, path_u.in)                                                              \
            {                                                                                                                            \
                hits_CHu += elm->field.sle_levels[0].hits;                                                                               \
            }                                                                                                                            \
            /* Evaluate the subtree 'v' at the same level. */                                                                            \
            __SKIP_SUBTREE_CHux(decl, field, slist, (*(p + 1)).node->field.sle_levels[path_u.in].next, path_u.in)                        \
            {                                                                                                                            \
                hits_CHv += elm->field.sle_levels[0].hits;                                                                               \
            }                                                                                                                            \
                                                                                                                                         \
            /* (a) Check the decent condition:                                                                                           \
             *     u_hits <= m_total_hits / (2 ^ (k_threshold - height of node))                                                         \
             *   When met we:                                                                                                            \
             *     1) go backwards along path from where we are until head                                                               \
             *     2) adjust any forward pointers, then...                                                                               \
             *     3) adjust hits, and...                                                                                                \
             *     4) lower the path[i]'s node height by 1                                                                               \
             */                                                                                                                          \
            delta_height = k_threshold - path_u.node->field.sle_height;                                                                  \
            dsc_cond = m_total_hits / pow(2.0, delta_height);                                                                            \
            u_hits = hits_CHu + hits_CHv;                                                                                                \
            if (u_hits <= dsc_cond && path_u.node->field.sle_height > 0) {                                                               \
                lvl = path_u.node->field.sle_height;                                                                                     \
                /* 1) go backwards along path from where we are until head */                                                            \
                node = path_u.node;                                                                                                      \
                do {                                                                                                                     \
                    node = node->field.sle_prev;                                                                                         \
                    /* 2) adjust forward pointers */                                                                                     \
                    if (node->field.sle_height >= lvl && node->field.sle_levels[lvl].next == path_u.node) {                              \
                        node->field.sle_levels[lvl].next = path_u.node->field.sle_levels[lvl].next;                                      \
                        /* 3) adjust hits */                                                                                             \
                        node->field.sle_levels[lvl].hits += 1;                                                                           \
                        break;                                                                                                           \
                    }                                                                                                                    \
                } while (node != slist->slh_head);                                                                                       \
                /* 4) reduce height by one */                                                                                            \
                path_u.node->field.sle_height--;                                                                                         \
            }                                                                                                                            \
            /* (b) Check the ascent condition:                                                                                           \
             *   path[i].pu + node_hits > hits total / (2 ^ (height of head - height of node - 1))                                       \
             *   When met we:                                                                                                            \
             *     1) check the ascent condition, then iff true ...                                                                      \
             *     2) add a level, and ...                                                                                               \
             *     3) set its hits to the prev node at intersection height                                                               \
             *     4) set prev node hits to 0 and forward to this new level                                                              \
             */                                                                                                                          \
            /* 1) check ascent condition */                                                                                              \
            asc_cond = m_total_hits / pow(2.0, delta_height - 1);                                                                        \
            if (path_u.pu > asc_cond && path_u.node->entries.sle_height < 64 - 1) {                                                      \
                if (path_u.node->field.sle_height + 1 > path_v.node->field.sle_height) {                                                 \
                    continue;                                                                                                            \
                }                                                                                                                        \
                lvl = ++path_u.node->field.sle_height;                                                                                   \
                /* Don't adjust hits when the previous node on the path is the head. */                                                  \
                if (path_v.node != slist->slh_head) {                                                                                    \
                    path_u.node->field.sle_levels[lvl].hits = path_v.node->entries.sle_levels[lvl].hits;                                 \
                    path_v.node->field.sle_levels[lvl].hits = 0;                                                                         \
                }                                                                                                                        \
                path_u.node->field.sle_levels[lvl].next = path_v.node->field.sle_levels[lvl].next;                                       \
                path_v.node->field.sle_levels[lvl].next = path_u.node;                                                                   \
            }                                                                                                                            \
        }                                                                                                                                \
    }                                                                                                                                    \
                                                                                                                                         \
    /**                                                                                                                                  \
     * -- __skip_locate_                                                                                                                 \
     *                                                                                                                                   \
     * Locates a node that matches another node updating `path` and then                                                                 \
     * returning the length of that path + 1 to the node and the matching                                                                \
     * node in path[0], or NULL at path[0] where there wasn't a match.                                                                   \
     * sizeof(path) should be `slist->slh_head->field.sle_height + 1`                                                                    \
     */                                                                                                                                  \
    static size_t __skip_locate_##decl(decl##_t *slist, decl##_node_t *n, __skiplist_path_##decl##_t path[])                             \
    {                                                                                                                                    \
        unsigned int i;                                                                                                                  \
        size_t len = 0;                                                                                                                  \
        decl##_node_t *elm;                                                                                                              \
        __skiplist_path_##decl##_t *cur;                                                                                                 \
                                                                                                                                         \
        if (slist == NULL || n == NULL)                                                                                                  \
            return 0;                                                                                                                    \
                                                                                                                                         \
        elm = slist->slh_head;                                                                                                           \
                                                                                                                                         \
        /* Find the node that matches `node` or NULL. */                                                                                 \
        i = slist->slh_head->field.sle_height - 1;                                                                                       \
        do {                                                                                                                             \
            cur = &path[i + 1];                                                                                                          \
            cur->pu = 0;                                                                                                                 \
            while (elm != slist->slh_tail && __skip_compare_nodes_##decl(slist, elm->field.sle_levels[i].next, n, slist->slh_aux) < 0) { \
                cur->in = i;                                                                                                             \
                elm = elm->field.sle_levels[i].next;                                                                                     \
            }                                                                                                                            \
            cur->node = elm;                                                                                                             \
            cur->node->field.sle_levels[cur->in].hits += i ? 1 : 0;                                                                      \
            cur->pu += cur->node->field.sle_levels[0].hits;                                                                              \
            len++;                                                                                                                       \
        } while (i--);                                                                                                                   \
        elm = elm->field.sle_levels[0].next;                                                                                             \
        if (__skip_compare_nodes_##decl(slist, elm, n, slist->slh_aux) == 0) {                                                           \
            path[0].node = elm;                                                                                                          \
            path[0].node->field.sle_levels[0].hits++;                                                                                    \
            cur->pu += path[0].node->field.sle_levels[0].hits;                                                                           \
            slist->slh_head->field.sle_levels[slist->slh_head->field.sle_height].hits++;                                                 \
            __skip_rebalance_##decl(slist, len, path);                                                                                   \
        }                                                                                                                                \
        return len;                                                                                                                      \
    }                                                                                                                                    \
                                                                                                                                         \
    /**                                                                                                                                  \
     * -- __skip_insert_                                                                                                                 \
     *                                                                                                                                   \
     * Inserts the node `new` into the list `slist`, when `flags` is non-zero                                                            \
     * duplicate keys are allowed. Duplicates are grouped together by key but                                                            \
     * are otherwise unordered.                                                                                                          \
     */                                                                                                                                  \
    static int __skip_insert_##decl(decl##_t *slist, decl##_node_t *new, int flags)                                                      \
    {                                                                                                                                    \
        static __skiplist_path_##decl##_t apath[SKIPLIST_MAX_HEIGHT + 1];                                                                \
        int rc = 0;                                                                                                                      \
        size_t i, len, loc = 0, current_height, new_height;                                                                              \
        decl##_node_t *node;                                                                                                             \
        __skiplist_path_##decl##_t *path = apath;                                                                                        \
                                                                                                                                         \
        if (slist == NULL || new == NULL)                                                                                                \
            return ENOENT;                                                                                                               \
                                                                                                                                         \
        memset(path, 0, sizeof(__skiplist_path_##decl##_t) * SKIPLIST_MAX_HEIGHT + 1);                                                   \
                                                                                                                                         \
        /* Find a `path` to `new` in the list and a match (`path[0]`) if it exists. */                                                   \
        len = __skip_locate_##decl(slist, new, path);                                                                                    \
        node = path[0].node;                                                                                                             \
        if (len > 0) {                                                                                                                   \
            if ((node != NULL) && (flags == 0)) {                                                                                        \
                /* Don't insert, duplicate if flag not set. */                                                                           \
                return -1;                                                                                                               \
            }                                                                                                                            \
            current_height = slist->slh_head->field.sle_height - 1;                                                                      \
            /* Coin toss to determine level of this new node [0, current max height) */                                                  \
            new_height = __skip_toss_##decl(slist, current_height);                                                                      \
            new->field.sle_height = new_height;                                                                                          \
            /* Trim the path to at most the new height for the new node. */                                                              \
            for (i = current_height + 1; i <= new_height; i++) {                                                                         \
                path[i + 1].node = slist->slh_tail;                                                                                      \
            }                                                                                                                            \
            /* Ensure all next[] point to tail. */                                                                                       \
            __SKIP_ALL_ENTRIES_B2T(field, new)                                                                                           \
            {                                                                                                                            \
                new->field.sle_levels[lvl].next = slist->slh_tail;                                                                       \
            }                                                                                                                            \
            /* Adjust all forward pointers for each element in the path. */                                                              \
            for (i = 0; i <= new_height; i++) {                                                                                          \
                /* The tail's next[i] is always NULL, we don't want that in the                                                          \
                   next[i] for our new node. Also, don't set the tail's next[i]                                                          \
                   because it is always NULL. */                                                                                         \
                if (path[i + 1].node != slist->slh_tail) {                                                                               \
                    new->field.sle_levels[i].next = path[i + 1].node->field.sle_levels[i].next;                                          \
                    path[i + 1].node->field.sle_levels[i].next = new;                                                                    \
                    loc = path[i + 1].node == slist->slh_head ? i : loc;                                                                 \
                } else {                                                                                                                 \
                    new->field.sle_levels[i].next = slist->slh_tail;                                                                     \
                }                                                                                                                        \
            }                                                                                                                            \
            /* Adjust the previous pointers in the nodes. */                                                                             \
            new->field.sle_prev = path[1].node;                                                                                          \
            new->field.sle_levels[0].next->field.sle_prev = new;                                                                         \
            /* Account for insert at tail. */                                                                                            \
            if (new->field.sle_levels[0].next == slist->slh_tail)                                                                        \
                slist->slh_tail->field.sle_prev = new;                                                                                   \
            /* Record the era for this node to enable snapshots. */                                                                      \
            if (slist->slh_snap.pres_era > 0) {                                                                                          \
                /* Increase the list's era/age and record it. */                                                                         \
                new->field.sle_era = slist->slh_snap.cur_era++;                                                                          \
            }                                                                                                                            \
            /* Set hits for re-balancing to 1 when newborn. */                                                                           \
            new->field.sle_levels[new_height].hits = 1;                                                                                  \
            /* Increase our list length (aka. size, count, etc.) by one. */                                                              \
            slist->slh_length++;                                                                                                         \
                                                                                                                                         \
            if (SKIPLIST_MAX_HEIGHT == 1)                                                                                                \
                free(path);                                                                                                              \
        }                                                                                                                                \
        return rc;                                                                                                                       \
    }                                                                                                                                    \
                                                                                                                                         \
    /**                                                                                                                                  \
     * -- skip_insert_                                                                                                                   \
     *                                                                                                                                   \
     * Insert into the list `slist` the node `n`.                                                                                        \
     */                                                                                                                                  \
    int prefix##skip_insert_##decl(decl##_t *slist, decl##_node_t *n)                                                                    \
    {                                                                                                                                    \
        return __skip_insert_##decl(slist, n, 0);                                                                                        \
    }                                                                                                                                    \
                                                                                                                                         \
    /**                                                                                                                                  \
     * -- skip_insert_dup_                                                                                                               \
     *                                                                                                                                   \
     * Inserts into `slist` the node `n` even if that node's key already                                                                 \
     * exists in the list.                                                                                                               \
     */                                                                                                                                  \
    int prefix##skip_insert_dup_##decl(decl##_t *slist, decl##_node_t *n)                                                                \
    {                                                                                                                                    \
        return __skip_insert_##decl(slist, n, 1);                                                                                        \
    }                                                                                                                                    \
                                                                                                                                         \
    /**                                                                                                                                  \
     * -- skip_position_eq_                                                                                                              \
     *                                                                                                                                   \
     * Find a node that matches the node `n`.  This differs from the function                                                            \
     * locate() in that it does not return the path to the node, only the match.                                                         \
     */                                                                                                                                  \
    decl##_node_t *prefix##skip_position_eq_##decl(decl##_t *slist, decl##_node_t *query)                                                \
    {                                                                                                                                    \
        static __skiplist_path_##decl##_t apath[SKIPLIST_MAX_HEIGHT + 1];                                                                \
        decl##_node_t *node = NULL;                                                                                                      \
        __skiplist_path_##decl##_t *path = apath;                                                                                        \
                                                                                                                                         \
        memset(path, 0, sizeof(__skiplist_path_##decl##_t) * SKIPLIST_MAX_HEIGHT + 1);                                                   \
                                                                                                                                         \
        /* Find a `path` to `new` in the list and a match (`path[0]`) if it exists. */                                                   \
        __skip_locate_##decl(slist, query, path);                                                                                        \
        node = path[0].node;                                                                                                             \
        if (SKIPLIST_MAX_HEIGHT == 1)                                                                                                    \
            free(path);                                                                                                                  \
                                                                                                                                         \
        return node;                                                                                                                     \
    }                                                                                                                                    \
                                                                                                                                         \
    /**                                                                                                                                  \
     * -- skip_position_gte                                                                                                              \
     *                                                                                                                                   \
     * Position and return a cursor at the first node that is equal to                                                                   \
     * or greater than the provided node `n`, otherwise if the largest                                                                   \
     * key is less than the key in `n` return NULL.                                                                                      \
     */                                                                                                                                  \
    decl##_node_t *prefix##skip_position_gte_##decl(decl##_t *slist, decl##_node_t *query)                                               \
    {                                                                                                                                    \
        static __skiplist_path_##decl##_t apath[SKIPLIST_MAX_HEIGHT + 1];                                                                \
        int cmp;                                                                                                                         \
        decl##_node_t *node;                                                                                                             \
        __skiplist_path_##decl##_t *path = apath;                                                                                        \
                                                                                                                                         \
        memset(path, 0, sizeof(__skiplist_path_##decl##_t) * SKIPLIST_MAX_HEIGHT + 1);                                                   \
                                                                                                                                         \
        /* Find a `path` to `new` in the list and a match (`path[0]`) if it exists. */                                                   \
        __skip_locate_##decl(slist, query, path);                                                                                        \
        node = path[1].node;                                                                                                             \
        do {                                                                                                                             \
            node = node->field.sle_levels[0].next;                                                                                       \
            cmp = __skip_compare_nodes_##decl(slist, node, query, slist->slh_aux);                                                       \
        } while (cmp < 0);                                                                                                               \
        if (SKIPLIST_MAX_HEIGHT == 1)                                                                                                    \
            free(path);                                                                                                                  \
                                                                                                                                         \
        return node;                                                                                                                     \
    }                                                                                                                                    \
                                                                                                                                         \
    /**                                                                                                                                  \
     * -- skip_position_gt_                                                                                                              \
     *                                                                                                                                   \
     * Position and return a cursor at the first node that is greater than                                                               \
     * the provided node `n`. If the largest key is less than the key in `n`                                                             \
     * return NULL.                                                                                                                      \
     */                                                                                                                                  \
    decl##_node_t *prefix##skip_position_gt_##decl(decl##_t *slist, decl##_node_t *query)                                                \
    {                                                                                                                                    \
        static __skiplist_path_##decl##_t apath[SKIPLIST_MAX_HEIGHT + 1];                                                                \
        int cmp;                                                                                                                         \
        decl##_node_t *node;                                                                                                             \
        __skiplist_path_##decl##_t *path = apath;                                                                                        \
                                                                                                                                         \
        memset(path, 0, sizeof(__skiplist_path_##decl##_t) * SKIPLIST_MAX_HEIGHT + 1);                                                   \
                                                                                                                                         \
        /* Find a `path` to `new` in the list and a match (`path[0]`) if it exists. */                                                   \
        __skip_locate_##decl(slist, query, path);                                                                                        \
        node = path[1].node;                                                                                                             \
        if (node == slist->slh_tail)                                                                                                     \
            goto done;                                                                                                                   \
        do {                                                                                                                             \
            node = node->field.sle_levels[0].next;                                                                                       \
            cmp = __skip_compare_nodes_##decl(slist, node, query, slist->slh_aux);                                                       \
        } while (cmp <= 0 && node != slist->slh_tail);                                                                                   \
    done:;                                                                                                                               \
        if (SKIPLIST_MAX_HEIGHT == 1)                                                                                                    \
            free(path);                                                                                                                  \
                                                                                                                                         \
        return node;                                                                                                                     \
    }                                                                                                                                    \
                                                                                                                                         \
    /**                                                                                                                                  \
     * -- skip_position_lte                                                                                                              \
     *                                                                                                                                   \
     * Position and return a cursor at the last node that is less than                                                                   \
     * or equal to node `n`.                                                                                                             \
     * Return NULL if nothing is less than or equal.                                                                                     \
     */                                                                                                                                  \
    decl##_node_t *prefix##skip_position_lte_##decl(decl##_t *slist, decl##_node_t *query)                                               \
    {                                                                                                                                    \
        static __skiplist_path_##decl##_t apath[SKIPLIST_MAX_HEIGHT + 1];                                                                \
        decl##_node_t *node;                                                                                                             \
        __skiplist_path_##decl##_t *path = apath;                                                                                        \
                                                                                                                                         \
        memset(path, 0, sizeof(__skiplist_path_##decl##_t) * SKIPLIST_MAX_HEIGHT + 1);                                                   \
                                                                                                                                         \
        /* Find a `path` to `new` in the list and a match (`path[0]`) if it exists. */                                                   \
        __skip_locate_##decl(slist, query, path);                                                                                        \
        node = path[0].node;                                                                                                             \
        if (node)                                                                                                                        \
            goto done;                                                                                                                   \
        node = path[1].node;                                                                                                             \
    done:;                                                                                                                               \
        if (SKIPLIST_MAX_HEIGHT == 1)                                                                                                    \
            free(path);                                                                                                                  \
                                                                                                                                         \
        return node;                                                                                                                     \
    }                                                                                                                                    \
                                                                                                                                         \
    /**                                                                                                                                  \
     * -- skip_position_lt_                                                                                                              \
     *                                                                                                                                   \
     * Position and return a cursor at the last node that is less than                                                                   \
     * to the node `n`. Return NULL if nothing is less than or equal.                                                                    \
     */                                                                                                                                  \
    decl##_node_t *prefix##skip_position_lt_##decl(decl##_t *slist, decl##_node_t *query)                                                \
    {                                                                                                                                    \
        static __skiplist_path_##decl##_t apath[SKIPLIST_MAX_HEIGHT + 1];                                                                \
        decl##_node_t *node;                                                                                                             \
        __skiplist_path_##decl##_t *path = apath;                                                                                        \
                                                                                                                                         \
        memset(path, 0, sizeof(__skiplist_path_##decl##_t) * SKIPLIST_MAX_HEIGHT + 1);                                                   \
                                                                                                                                         \
        /* Find a `path` to `new` in the list and a match (`path[0]`) if it exists. */                                                   \
        __skip_locate_##decl(slist, query, path);                                                                                        \
        node = path[1].node;                                                                                                             \
        if (SKIPLIST_MAX_HEIGHT == 1)                                                                                                    \
            free(path);                                                                                                                  \
                                                                                                                                         \
        return node;                                                                                                                     \
    }                                                                                                                                    \
                                                                                                                                         \
    /**                                                                                                                                  \
     * -- skip_position_                                                                                                                 \
     *                                                                                                                                   \
     * Position a cursor relative to `n`.                                                                                                \
     */                                                                                                                                  \
    decl##_node_t *prefix##skip_position_##decl(decl##_t *slist, skip_pos_##decl_t op, decl##_node_t *query)                             \
    {                                                                                                                                    \
        decl##_node_t *node;                                                                                                             \
                                                                                                                                         \
        switch (op) {                                                                                                                    \
        case (SKIP_LT):                                                                                                                  \
            node = prefix##skip_position_lt_##decl(slist, query);                                                                        \
            break;                                                                                                                       \
        case (SKIP_LTE):                                                                                                                 \
            node = prefix##skip_position_lte_##decl(slist, query);                                                                       \
            break;                                                                                                                       \
        case (SKIP_GTE):                                                                                                                 \
            node = prefix##skip_position_gte_##decl(slist, query);                                                                       \
            break;                                                                                                                       \
        case (SKIP_GT):                                                                                                                  \
            node = prefix##skip_position_gt_##decl(slist, query);                                                                        \
            break;                                                                                                                       \
        default:                                                                                                                         \
        case (SKIP_EQ):                                                                                                                  \
            node = prefix##skip_position_eq_##decl(slist, query);                                                                        \
            break;                                                                                                                       \
        }                                                                                                                                \
        return node;                                                                                                                     \
    }                                                                                                                                    \
                                                                                                                                         \
    /**                                                                                                                                  \
     * -- skip_update_                                                                                                                   \
     *                                                                                                                                   \
     * Locates a node in the list that equals the `new` node and then                                                                    \
     * uses the `update_entry_blk` to update the contents.                                                                               \
     *                                                                                                                                   \
     * WARNING: Do not update the portion of the node used for ordering                                                                  \
     * (e.g. `key`) unless you really know what you're doing.                                                                            \
     */                                                                                                                                  \
    int prefix##skip_update_##decl(decl##_t *slist, decl##_node_t *query, void *value)                                                   \
    {                                                                                                                                    \
        static __skiplist_path_##decl##_t apath[SKIPLIST_MAX_HEIGHT + 1];                                                                \
        int rc = 0, np;                                                                                                                  \
        decl##_node_t *node;                                                                                                             \
        __skiplist_path_##decl##_t *path = apath;                                                                                        \
                                                                                                                                         \
        if (slist == NULL)                                                                                                               \
            return -1;                                                                                                                   \
                                                                                                                                         \
        memset(path, 0, sizeof(__skiplist_path_##decl##_t) * SKIPLIST_MAX_HEIGHT + 1);                                                   \
                                                                                                                                         \
        __skip_locate_##decl(slist, query, path);                                                                                        \
        node = path[0].node;                                                                                                             \
                                                                                                                                         \
        if (SKIPLIST_MAX_HEIGHT == 1)                                                                                                    \
            free(path);                                                                                                                  \
                                                                                                                                         \
        if (node == NULL)                                                                                                                \
            return -1;                                                                                                                   \
                                                                                                                                         \
        /* If the optional snapshots feature is configured, use it now.                                                                  \
           Snapshots preserve the node if it is older than our snapshot                                                                  \
           and about to be mutated. */                                                                                                   \
        if (slist->slh_snap.pres_era > 0) {                                                                                              \
            /* Preserve the node. */                                                                                                     \
            np = slist->slh_fns.snapshot_preserve_node(slist, node, NULL);                                                               \
            if (np > 0)                                                                                                                  \
                return np;                                                                                                               \
                                                                                                                                         \
            /* Increase the list's era/age. */                                                                                           \
            slist->slh_snap.cur_era++;                                                                                                   \
        }                                                                                                                                \
                                                                                                                                         \
        slist->slh_fns.update_entry(node, value);                                                                                        \
                                                                                                                                         \
        return rc;                                                                                                                       \
    }                                                                                                                                    \
                                                                                                                                         \
    /**                                                                                                                                  \
     * -- skip_remove_node_                                                                                                              \
     *                                                                                                                                   \
     * Removes the node `n` from the `slist` if present.                                                                                 \
     */                                                                                                                                  \
    int prefix##skip_remove_node_##decl(decl##_t *slist, decl##_node_t *query)                                                           \
    {                                                                                                                                    \
        static __skiplist_path_##decl##_t apath[SKIPLIST_MAX_HEIGHT + 1];                                                                \
        int np = 0;                                                                                                                      \
        size_t i, len, height;                                                                                                           \
        decl##_node_t *node;                                                                                                             \
        __skiplist_path_##decl##_t *path = apath;                                                                                        \
                                                                                                                                         \
        if (slist == NULL || query == NULL)                                                                                              \
            return -1;                                                                                                                   \
        if (slist->slh_length == 0)                                                                                                      \
            return 0;                                                                                                                    \
                                                                                                                                         \
        memset(path, 0, sizeof(__skiplist_path_##decl##_t) * SKIPLIST_MAX_HEIGHT + 1);                                                   \
                                                                                                                                         \
        /* Attempt to locate the node in the list. */                                                                                    \
        len = __skip_locate_##decl(slist, query, path);                                                                                  \
        node = path[0].node;                                                                                                             \
        if (node) {                                                                                                                      \
            /* If the optional snapshots feature is configured, use it now.                                                              \
               Snapshots preserve the node if it is older than our snapshot                                                              \
               and about to be mutated. */                                                                                               \
            if (slist->slh_snap.pres_era > 0) {                                                                                          \
                /* Preserve the node. */                                                                                                 \
                np = slist->slh_fns.snapshot_preserve_node(slist, node, NULL);                                                           \
                if (np > 0)                                                                                                              \
                    return np;                                                                                                           \
                                                                                                                                         \
                /* Increase the list's era/age. */                                                                                       \
                slist->slh_snap.cur_era++;                                                                                               \
            }                                                                                                                            \
            /* We found it, set the next->prev to the node->prev keeping in mind                                                         \
               that the next node might be the tail). */                                                                                 \
            node->field.sle_levels[0].next->field.sle_prev = node->field.sle_prev;                                                       \
            /* Walk the path, stop when the next node is not the one we're                                                               \
               removing. At each step along our walk... */                                                                               \
            for (i = 0; i < len; i++) {                                                                                                  \
                if (path[i + 1].node->field.sle_levels[i].next != node)                                                                  \
                    break;                                                                                                               \
                /* ... adjust the next pointer at that level. */                                                                         \
                path[i + 1].node->field.sle_levels[i].next = node->field.sle_levels[i].next;                                             \
                /* Adjust the height such that we're only pointing at the tail once at                                                   \
                   the top that way we don't waste steps later when searching. */                                                        \
                if (path[i + 1].node->field.sle_levels[i].next == slist->slh_tail) {                                                     \
                    height = path[i + 1].node->field.sle_height;                                                                         \
                    path[i + 1].node->field.sle_height = height - 1;                                                                     \
                }                                                                                                                        \
            }                                                                                                                            \
            /* Account for delete at tail. */                                                                                            \
            if (node->field.sle_levels[0].next == slist->slh_tail) {                                                                     \
                slist->slh_tail->field.sle_prev = query->field.sle_prev;                                                                 \
            }                                                                                                                            \
                                                                                                                                         \
            if (SKIPLIST_MAX_HEIGHT == 1)                                                                                                \
                free(path);                                                                                                              \
                                                                                                                                         \
            slist->slh_fns.free_entry(node);                                                                                             \
            free(node);                                                                                                                  \
                                                                                                                                         \
            /* Reduce the height of the head/tail nodes. */                                                                              \
            for (i = 0; slist->slh_head->field.sle_levels[i].next != slist->slh_tail && i < SKIPLIST_MAX_HEIGHT; i++)                    \
                ;                                                                                                                        \
            slist->slh_head->field.sle_levels[i].hits = slist->slh_head->field.sle_levels[slist->slh_head->field.sle_height].hits;       \
            slist->slh_head->field.sle_height = slist->slh_tail->field.sle_height = i;                                                   \
                                                                                                                                         \
            slist->slh_length--;                                                                                                         \
            __skip_adjust_hit_counts_##decl(slist);                                                                                      \
        }                                                                                                                                \
        return 0;                                                                                                                        \
    }                                                                                                                                    \
                                                                                                                                         \
    /**                                                                                                                                  \
     * -- skip_free_                                                                                                                     \
     *                                                                                                                                   \
     * Release all nodes and their associated heap objects.  The list reference                                                          \
     * is no longer valid after this call. To make it valid again call _init().                                                          \
     */                                                                                                                                  \
    void prefix##skip_free_##decl(decl##_t *slist)                                                                                       \
    {                                                                                                                                    \
        if (slist == NULL)                                                                                                               \
            return;                                                                                                                      \
                                                                                                                                         \
        if (slist->slh_snap.pres_era > 0 && slist->slh_fns.snapshot_release)                                                             \
            slist->slh_fns.snapshot_release(slist);                                                                                      \
                                                                                                                                         \
        prefix##skip_release_##decl(slist);                                                                                              \
                                                                                                                                         \
        free(slist->slh_head);                                                                                                           \
        free(slist->slh_tail);                                                                                                           \
    }

#define SKIPLIST_DECL_SNAPSHOTS(decl, prefix, field)                                                             \
                                                                                                                 \
    /**                                                                                                          \
     * -- skip_snapshot_                                                                                         \
     *                                                                                                           \
     * A snapshot is a read-only view of a Skiplist at a point in time.  Once                                    \
     * taken, a snapshot must be restored or released. Any number of snapshots                                   \
     * can be created. Return the `era` of the snapshot.                                                         \
     */                                                                                                          \
    uint64_t prefix##skip_snapshot_##decl(decl##_t *slist)                                                       \
    {                                                                                                            \
        if (slist == NULL)                                                                                       \
            return 0;                                                                                            \
                                                                                                                 \
        return ++slist->slh_snap.pres_era;                                                                       \
    }                                                                                                            \
                                                                                                                 \
    /**                                                                                                          \
     * -- skip_release_snapshots_                                                                                \
     *                                                                                                           \
     */                                                                                                          \
    void prefix##skip_release_snapshots_##decl(decl##_t *slist)                                                  \
    {                                                                                                            \
        decl##_node_t *node, *next;                                                                              \
                                                                                                                 \
        if (slist == NULL)                                                                                       \
            return;                                                                                              \
                                                                                                                 \
        if (slist->slh_snap.pres_era == 0)                                                                       \
            return;                                                                                              \
                                                                                                                 \
        node = slist->slh_snap.pres;                                                                             \
        while (node) {                                                                                           \
            next = node->field.sle_levels[0].next;                                                               \
            prefix##skip_free_node_##decl(slist, node);                                                          \
            node = next;                                                                                         \
        }                                                                                                        \
        slist->slh_snap.pres = NULL;                                                                             \
        slist->slh_snap.pres_era = 0;                                                                            \
    }                                                                                                            \
                                                                                                                 \
    /**                                                                                                          \
     * -- __skip_preserve_node_                                                                                  \
     *                                                                                                           \
     * Preserve given node in the slh_snap.pres list.                                                            \
     *                                                                                                           \
     * ALGORITHM:                                                                                                \
     *   a) allocate a new node                                                                                  \
     *   b) copy the node into the new node                                                                      \
     *   c) as necessary, allocate/copy any user-supplied items.                                                 \
     *   d) determine if this is a duplicate, if so in (d) we set                                                \
     *      the sle.next[1] field to 0x1 as a reminder to re-insert                                              \
     *      this element as a duplicate in the restore function.                                                 \
     *   e) zero out the next sle.prev/next[] pointers                                                           \
     *   f) mark as duplicate, set sle.next[1] = 0x1                                                             \
     *   g) insert the node's copy into the slh_pres singly-linked                                               \
     *      list.                                                                                                \
     */                                                                                                          \
    static int __skip_preserve_node_##decl(decl##_t *slist, const decl##_node_t *src, decl##_node_t **preserved) \
    {                                                                                                            \
        int rc = 0;                                                                                              \
        decl##_node_t *dest, *is_dup = 0;                                                                        \
                                                                                                                 \
        if (slist == NULL || src == NULL)                                                                        \
            return 0;                                                                                            \
                                                                                                                 \
        /* Never preserve the head or the tail. */                                                               \
        if (src == slist->slh_head || src == slist->slh_tail)                                                    \
            return 0;                                                                                            \
                                                                                                                 \
        /* If the era into which the node `src` was born preceeded the latest                                    \
           snapshot era, then we need to preserve the older version of this                                      \
           node.  Said another way, we preserve anything with an era that is                                     \
           less than the slh_snap.cur_era. */                                                                    \
        if (src->field.sle_era > slist->slh_snap.pres_era)                                                       \
            return 0;                                                                                            \
                                                                                                                 \
        /* (a) alloc, ... */                                                                                     \
        size_t sle_arr_sz = sizeof(struct __skiplist_##decl##_level) * slist->slh_head->field.sle_height - 1;    \
        rc = prefix##skip_alloc_node_##decl(&dest);                                                              \
        if (rc)                                                                                                  \
            return rc;                                                                                           \
                                                                                                                 \
        /* (b) shallow copy, copied sle_levels pointer is to the src list, so                                    \
           update that to point to the offset in this heap object, ... */                                        \
        memcpy(dest, src, sizeof(decl##_node_t) + sle_arr_sz);                                                   \
        dest->field.sle_levels = (struct __skiplist_##decl##_level *)((uintptr_t)dest + sizeof(decl##_node_t));  \
                                                                                                                 \
        /* (c) then user-supplied copy */                                                                        \
        slist->slh_fns.archive_entry(dest, src);                                                                 \
        if (rc) {                                                                                                \
            prefix##skip_free_node_##decl(slist, dest);                                                          \
            return rc;                                                                                           \
        }                                                                                                        \
                                                                                                                 \
        /* (d) is this a duplicate? */                                                                           \
        if (__skip_compare_nodes_##decl(slist, dest, dest->field.sle_levels[0].next, slist->slh_aux) == 0 ||     \
            __skip_compare_nodes_##decl(slist, dest, dest->field.sle_prev, slist->slh_aux) == 0)                 \
            is_dup = (decl##_node_t *)0x1;                                                                       \
                                                                                                                 \
        /* (e) zero out the next pointers */                                                                     \
        dest->field.sle_prev = NULL;                                                                             \
        __SKIP_ALL_ENTRIES_B2T(field, dest)                                                                      \
        {                                                                                                        \
            dest->field.sle_levels[lvl].next = NULL;                                                             \
        }                                                                                                        \
                                                                                                                 \
        /* (f) set duplicate flag */                                                                             \
        dest->field.sle_levels[1].next = is_dup;                                                                 \
                                                                                                                 \
        /* (g) insert node into slh_pres list at head */                                                         \
        if (slist->slh_snap.pres == NULL) {                                                                      \
            dest->field.sle_levels[0].next = NULL;                                                               \
            slist->slh_snap.pres = dest;                                                                         \
        } else {                                                                                                 \
            /* The next[0] pointer forms the singly-linked list when                                             \
               preserved. */                                                                                     \
            dest->field.sle_levels[0].next = slist->slh_snap.pres;                                               \
            slist->slh_snap.pres = dest;                                                                         \
        }                                                                                                        \
                                                                                                                 \
        if (preserved)                                                                                           \
            *preserved = dest;                                                                                   \
                                                                                                                 \
        rc = 1;                                                                                                  \
        return -rc;                                                                                              \
    }                                                                                                            \
                                                                                                                 \
    /**                                                                                                          \
     * -- skip_restore_snapshot_ TODO/WIP                                                                        \
     *                                                                                                           \
     * Restores the Skiplist to generation `era`.  Once you restore `era` you                                    \
     * can no longer restore any [era, current era] only those earlier than                                      \
     * era.                                                                                                      \
     *                                                                                                           \
     * ALGORITHM:                                                                                                \
     * iterate over the preserved nodes (slist->slh_snap.pres)                                                   \
     *  a) remove/free nodes with node->era > era from slist                                                     \
     *  b) remove/free nodes > era from slh_pres                                                                 \
     *  c) restore nodes == era by...                                                                            \
     *     i) remove node from slh_pres list                                                                     \
     *     ii) _insert(node) or                                                                                  \
     *         _insert_dup() if node->field.sle_levels[1].next != 0 (clear that)                                 \
     *  d) set slist's era to `era`                                                                              \
     *                                                                                                           \
     * NOTES:                                                                                                    \
     * - Starting with slh_pres, the `node->field.sle_levels[0].next` form a                                     \
     *   singly-linked list.                                                                                     \
     */                                                                                                          \
    decl##_t *prefix##skip_restore_snapshot_##decl(decl##_t *slist, size_t era)                                  \
    {                                                                                                            \
        size_t i, cur_era;                                                                                       \
        decl##_node_t *node, *prev;                                                                              \
                                                                                                                 \
        if (slist == NULL)                                                                                       \
            return NULL;                                                                                         \
                                                                                                                 \
        if (slist->slh_snap.pres_era == 0)                                                                       \
            return NULL;                                                                                         \
                                                                                                                 \
        if (era >= slist->slh_snap.cur_era || slist->slh_snap.pres == NULL)                                      \
            return slist;                                                                                        \
                                                                                                                 \
        cur_era = slist->slh_snap.cur_era;                                                                       \
                                                                                                                 \
        /* (a) remove and free nodes from slist that are newer than era */                                       \
        SKIPLIST_FOREACH_H2T(decl, prefix, field, slist, node, i)                                                \
        {                                                                                                        \
            ((void)i);                                                                                           \
            if (node->field.sle_era > era) {                                                                     \
                prefix##skip_remove_node_##decl(slist, node);                                                    \
            }                                                                                                    \
        }                                                                                                        \
                                                                                                                 \
        prev = NULL;                                                                                             \
        node = slist->slh_snap.pres;                                                                             \
        while (node) {                                                                                           \
            /* (b) remove nodes from slh_pres that are newer than era */                                         \
            if (node->field.sle_era > era) {                                                                     \
                /* remove node from slh_snap.pres list */                                                        \
                if (slist->slh_snap.pres == node)                                                                \
                    slist->slh_snap.pres = node->field.sle_levels[0].next;                                       \
                else {                                                                                           \
                    if (node->field.sle_levels[0].next == NULL)                                                  \
                        if (node == slist->slh_snap.pres)                                                        \
                            slist->slh_snap.pres = NULL;                                                         \
                        else                                                                                     \
                            prev->field.sle_levels[0].next = NULL;                                               \
                    else                                                                                         \
                        prev->field.sle_levels[0].next = node->field.sle_levels[0].next;                         \
                }                                                                                                \
                prefix##skip_free_node_##decl(slist, node);                                                      \
            }                                                                                                    \
                                                                                                                 \
            /* (c) restore nodes from slh_pres that match the era */                                             \
            prev = NULL;                                                                                         \
            if (node->field.sle_era == era) {                                                                    \
                /* remove node from slh_snap.pres list */                                                        \
                if (slist->slh_snap.pres == node)                                                                \
                    slist->slh_snap.pres = node->field.sle_levels[0].next;                                       \
                else {                                                                                           \
                    if (node->field.sle_levels[0].next == NULL)                                                  \
                        if (node == slist->slh_snap.pres)                                                        \
                            slist->slh_snap.pres = NULL;                                                         \
                        else                                                                                     \
                            prev->field.sle_levels[0].next = NULL;                                               \
                    else                                                                                         \
                        prev->field.sle_levels[0].next = node->field.sle_levels[0].next;                         \
                }                                                                                                \
                                                                                                                 \
                node->field.sle_prev = NULL;                                                                     \
                if (node->field.sle_levels[1].next != 0) {                                                       \
                    node->field.sle_levels[1].next = NULL;                                                       \
                    prefix##skip_insert_dup_##decl(slist, node);                                                 \
                } else {                                                                                         \
                    prefix##skip_insert_##decl(slist, node);                                                     \
                }                                                                                                \
            }                                                                                                    \
            prev = node;                                                                                         \
            node = node->field.sle_levels[0].next;                                                               \
        }                                                                                                        \
                                                                                                                 \
        /* (d) set list's era */                                                                                 \
        slist->slh_snap.pres_era = slist->slh_snap.pres == NULL ? 0 : cur_era;                                   \
                                                                                                                 \
        return slist;                                                                                            \
    }                                                                                                            \
                                                                                                                 \
    /**                                                                                                          \
     * -- skip_snapshots_init_                                                                                   \
     *                                                                                                           \
     * Adds the ability to take a single stable snapshot to the Skiplist API.                                    \
     */                                                                                                          \
    void prefix##skip_snapshots_init_##decl(decl##_t *slist)                                                     \
    {                                                                                                            \
        if (slist != NULL) {                                                                                     \
            slist->slh_fns.snapshot_preserve_node = __skip_preserve_node_##decl;                                 \
            slist->slh_fns.snapshot_release = prefix##skip_release_snapshots_##decl;                             \
        }                                                                                                        \
    }

#define SKIPLIST_DECL_VALIDATE(decl, prefix, field)                                                                                                          \
    /**                                                                                                                                                      \
     * -- __skip_integrity_failure_                                                                                                                          \
     */                                                                                                                                                      \
    static void __attribute__((format(printf, 1, 2))) __skip_integrity_failure_##decl(const char *format, ...)                                               \
    {                                                                                                                                                        \
        va_list args;                                                                                                                                        \
        va_start(args, format);                                                                                                                              \
        vfprintf(stderr, format, args);                                                                                                                      \
        va_end(args);                                                                                                                                        \
    }                                                                                                                                                        \
                                                                                                                                                             \
    /**                                                                                                                                                      \
     * -- __skip_integrity_check_                                                                                                                            \
     */                                                                                                                                                      \
    static int __skip_integrity_check_##decl(decl##_t *slist, int flags)                                                                                     \
    {                                                                                                                                                        \
        size_t n = 0;                                                                                                                                        \
        unsigned long nth, n_err = 0;                                                                                                                        \
        decl##_node_t *node, *prev, *next;                                                                                                                   \
        struct __skiplist_##decl##_entry *this;                                                                                                              \
                                                                                                                                                             \
        if (slist == NULL) {                                                                                                                                 \
            __skip_integrity_failure_##decl("slist was NULL, nothing to check\n");                                                                           \
            n_err++;                                                                                                                                         \
            return n_err;                                                                                                                                    \
        }                                                                                                                                                    \
                                                                                                                                                             \
        /* Check the Skiplist header (slh) */                                                                                                                \
                                                                                                                                                             \
        if (slist->slh_head == NULL) {                                                                                                                       \
            __skip_integrity_failure_##decl("skiplist slh_head is NULL\n");                                                                                  \
            n_err++;                                                                                                                                         \
            return n_err;                                                                                                                                    \
        }                                                                                                                                                    \
                                                                                                                                                             \
        if (slist->slh_tail == NULL) {                                                                                                                       \
            __skip_integrity_failure_##decl("skiplist slh_tail is NULL\n");                                                                                  \
            n_err++;                                                                                                                                         \
            return n_err;                                                                                                                                    \
        }                                                                                                                                                    \
                                                                                                                                                             \
        if (slist->slh_fns.free_entry == NULL) {                                                                                                             \
            __skip_integrity_failure_##decl("skiplist free_entry fn is NULL\n");                                                                             \
            n_err++;                                                                                                                                         \
            return n_err;                                                                                                                                    \
        }                                                                                                                                                    \
                                                                                                                                                             \
        if (slist->slh_fns.update_entry == NULL) {                                                                                                           \
            __skip_integrity_failure_##decl("skiplist update_entry fn is NULL\n");                                                                           \
            n_err++;                                                                                                                                         \
            return n_err;                                                                                                                                    \
        }                                                                                                                                                    \
                                                                                                                                                             \
        if (slist->slh_fns.archive_entry == NULL) {                                                                                                          \
            __skip_integrity_failure_##decl("skiplist archive_entry fn is NULL\n");                                                                          \
            n_err++;                                                                                                                                         \
            return n_err;                                                                                                                                    \
        }                                                                                                                                                    \
                                                                                                                                                             \
        if (slist->slh_fns.sizeof_entry == NULL) {                                                                                                           \
            __skip_integrity_failure_##decl("skiplist sizeof_entry fn is NULL\n");                                                                           \
            n_err++;                                                                                                                                         \
            return n_err;                                                                                                                                    \
        }                                                                                                                                                    \
                                                                                                                                                             \
        if (slist->slh_fns.compare_entries == NULL) {                                                                                                        \
            __skip_integrity_failure_##decl("skiplist compare_entries fn is NULL\n");                                                                        \
            n_err++;                                                                                                                                         \
            return n_err;                                                                                                                                    \
        }                                                                                                                                                    \
                                                                                                                                                             \
        if (slist->slh_head->field.sle_height > SKIPLIST_MAX_HEIGHT) {                                                                                       \
            __skip_integrity_failure_##decl("skiplist head height > SKIPLIST_MAX_HEIGHT\n");                                                                 \
            n_err++;                                                                                                                                         \
            if (flags)                                                                                                                                       \
                return n_err;                                                                                                                                \
        }                                                                                                                                                    \
                                                                                                                                                             \
        if (slist->slh_tail->field.sle_height > SKIPLIST_MAX_HEIGHT) {                                                                                       \
            __skip_integrity_failure_##decl("skiplist tail height > SKIPLIST_MAX_HEIGHT\n");                                                                 \
            n_err++;                                                                                                                                         \
            if (flags)                                                                                                                                       \
                return n_err;                                                                                                                                \
        }                                                                                                                                                    \
                                                                                                                                                             \
        if (slist->slh_head->field.sle_height != slist->slh_tail->field.sle_height) {                                                                        \
            __skip_integrity_failure_##decl("skiplist head & tail height are not equal\n");                                                                  \
            n_err++;                                                                                                                                         \
            if (flags)                                                                                                                                       \
                return n_err;                                                                                                                                \
        }                                                                                                                                                    \
                                                                                                                                                             \
        /* TODO: slh_head->field.sle_height should == log(m) where m is the sum of all hits on all nodes */                                                  \
                                                                                                                                                             \
        if (SKIPLIST_MAX_HEIGHT < 1) {                                                                                                                       \
            __skip_integrity_failure_##decl("SKIPLIST_MAX_HEIGHT cannot be less than 1\n");                                                                  \
            n_err++;                                                                                                                                         \
            if (flags)                                                                                                                                       \
                return n_err;                                                                                                                                \
        }                                                                                                                                                    \
                                                                                                                                                             \
        node = slist->slh_head;                                                                                                                              \
        __SKIP_ENTRIES_B2T(field, node)                                                                                                                      \
        {                                                                                                                                                    \
            if (node->field.sle_levels[lvl].next == NULL) {                                                                                                  \
                __skip_integrity_failure_##decl("the head's %lu next node should not be NULL\n", lvl);                                                       \
                n_err++;                                                                                                                                     \
                if (flags)                                                                                                                                   \
                    return n_err;                                                                                                                            \
            }                                                                                                                                                \
            n = lvl;                                                                                                                                         \
            if (node->field.sle_levels[lvl].next == slist->slh_tail)                                                                                         \
                break;                                                                                                                                       \
        }                                                                                                                                                    \
        n++;                                                                                                                                                 \
        __SKIP_ENTRIES_B2T_FROM(field, node, n)                                                                                                              \
        {                                                                                                                                                    \
            if (node->field.sle_levels[lvl].next == NULL) {                                                                                                  \
                __skip_integrity_failure_##decl("the head's %lu next node should not be NULL\n", lvl);                                                       \
                n_err++;                                                                                                                                     \
                if (flags)                                                                                                                                   \
                    return n_err;                                                                                                                            \
            }                                                                                                                                                \
            /* TODO: really?                                                                                                                                 \
            if (node->field.sle_levels[lvl].next != slist->slh_tail) {                                                                                       \
                __skip_integrity_failure_##decl("after internal nodes, the head's %lu next node should always be the tail\n", lvl);                          \
                n_err++;                                                                                                                                     \
                if (flags)                                                                                                                                   \
                    return n_err;                                                                                                                            \
            }                                                                                                                                                \
            */                                                                                                                                               \
        }                                                                                                                                                    \
                                                                                                                                                             \
        if (slist->slh_length > 0 && slist->slh_tail->field.sle_prev == slist->slh_head) {                                                                   \
            __skip_integrity_failure_##decl("slist->slh_length is 0, but tail->prev == head, not an internal node\n");                                       \
            n_err++;                                                                                                                                         \
            if (flags)                                                                                                                                       \
                return n_err;                                                                                                                                \
        }                                                                                                                                                    \
                                                                                                                                                             \
        /* Validate the head node */                                                                                                                         \
                                                                                                                                                             \
        /* Validate the tail node */                                                                                                                         \
                                                                                                                                                             \
        /* Validate each node */                                                                                                                             \
        SKIPLIST_FOREACH_H2T(decl, prefix, field, slist, node, nth)                                                                                          \
        {                                                                                                                                                    \
            this = &node->field;                                                                                                                             \
                                                                                                                                                             \
            if (this->sle_height > slist->slh_head->field.sle_height) {                                                                                      \
                __skip_integrity_failure_##decl("the %lu node's [%p] height %lu is > head %lu\n", nth, (void *)node, this->sle_height,                       \
                    slist->slh_head->field.sle_height);                                                                                                      \
                n_err++;                                                                                                                                     \
                if (flags)                                                                                                                                   \
                    return n_err;                                                                                                                            \
            }                                                                                                                                                \
                                                                                                                                                             \
            if (this->sle_levels == NULL) {                                                                                                                  \
                __skip_integrity_failure_##decl("the %lu node's [%p] next field should never be NULL\n", nth, (void *)node);                                 \
                n_err++;                                                                                                                                     \
                if (flags)                                                                                                                                   \
                    return n_err;                                                                                                                            \
            }                                                                                                                                                \
                                                                                                                                                             \
            if (this->sle_prev == NULL) {                                                                                                                    \
                __skip_integrity_failure_##decl("the %lu node [%p] prev field should never be NULL\n", nth, (void *)node);                                   \
                n_err++;                                                                                                                                     \
                if (flags)                                                                                                                                   \
                    return n_err;                                                                                                                            \
            }                                                                                                                                                \
                                                                                                                                                             \
            __SKIP_ENTRIES_B2T(field, node)                                                                                                                  \
            {                                                                                                                                                \
                if (this->sle_levels[lvl].next == NULL) {                                                                                                    \
                    __skip_integrity_failure_##decl("the %lu node's next[%lu] should not be NULL\n", nth, lvl);                                              \
                    n_err++;                                                                                                                                 \
                    if (flags)                                                                                                                               \
                        return n_err;                                                                                                                        \
                }                                                                                                                                            \
                n = lvl;                                                                                                                                     \
                if (this->sle_levels[lvl].next == slist->slh_tail)                                                                                           \
                    break;                                                                                                                                   \
            }                                                                                                                                                \
            n++;                                                                                                                                             \
            __SKIP_ENTRIES_B2T_FROM(field, node, n)                                                                                                          \
            {                                                                                                                                                \
                if (this->sle_levels[lvl].next == NULL) {                                                                                                    \
                    __skip_integrity_failure_##decl("after the %lunth the %lu node's next[%lu] should not be NULL\n", n, nth, lvl);                          \
                    n_err++;                                                                                                                                 \
                    if (flags)                                                                                                                               \
                        return n_err;                                                                                                                        \
                } else if (this->sle_levels[lvl].next != slist->slh_tail) {                                                                                  \
                    __skip_integrity_failure_##decl("after the %lunth the %lu node's next[%lu] should point to the tail\n", n, nth, lvl);                    \
                    n_err++;                                                                                                                                 \
                    if (flags)                                                                                                                               \
                        return n_err;                                                                                                                        \
                }                                                                                                                                            \
            }                                                                                                                                                \
                                                                                                                                                             \
            decl##_node_t *a = (decl##_node_t *)(uintptr_t)this->sle_levels;                                                                                 \
            decl##_node_t *b = (decl##_node_t *)(intptr_t)((uintptr_t)node + sizeof(decl##_node_t));                                                         \
            if (a != b) {                                                                                                                                    \
                __skip_integrity_failure_##decl("the %lu node's [%p] next field isn't at the proper offset relative to the node\n", nth, (void *)node);      \
                n_err++;                                                                                                                                     \
                if (flags)                                                                                                                                   \
                    return n_err;                                                                                                                            \
            }                                                                                                                                                \
                                                                                                                                                             \
            next = this->sle_levels[0].next;                                                                                                                 \
            prev = this->sle_prev;                                                                                                                           \
            if (__skip_compare_nodes_##decl(slist, node, node, slist->slh_aux) != 0) {                                                                       \
                __skip_integrity_failure_##decl("the %lu node [%p] is not equal to itself\n", nth, (void *)node);                                            \
                n_err++;                                                                                                                                     \
                if (flags)                                                                                                                                   \
                    return n_err;                                                                                                                            \
            }                                                                                                                                                \
                                                                                                                                                             \
            if (__skip_compare_nodes_##decl(slist, node, prev, slist->slh_aux) < 0) {                                                                        \
                __skip_integrity_failure_##decl("the %lu node [%p] is not greater than the prev node [%p]\n", nth, (void *)node, (void *)prev);              \
                n_err++;                                                                                                                                     \
                if (flags)                                                                                                                                   \
                    return n_err;                                                                                                                            \
            }                                                                                                                                                \
                                                                                                                                                             \
            if (__skip_compare_nodes_##decl(slist, node, next, slist->slh_aux) > 0) {                                                                        \
                __skip_integrity_failure_##decl("the %lu node [%p] is not less than the next node [%p]\n", nth, (void *)node, (void *)next);                 \
                n_err++;                                                                                                                                     \
                if (flags)                                                                                                                                   \
                    return n_err;                                                                                                                            \
            }                                                                                                                                                \
                                                                                                                                                             \
            if (__skip_compare_nodes_##decl(slist, prev, node, slist->slh_aux) > 0) {                                                                        \
                __skip_integrity_failure_##decl("the prev node [%p] is not less than the %lu node [%p]\n", (void *)prev, nth, (void *)node);                 \
                n_err++;                                                                                                                                     \
                if (flags)                                                                                                                                   \
                    return n_err;                                                                                                                            \
            }                                                                                                                                                \
                                                                                                                                                             \
            if (__skip_compare_nodes_##decl(slist, next, node, slist->slh_aux) < 0) {                                                                        \
                __skip_integrity_failure_##decl("the next node [%p] is not greater than the %lu node [%p]\n", (void *)next, nth, (void *)node);              \
                n_err++;                                                                                                                                     \
                if (flags)                                                                                                                                   \
                    return n_err;                                                                                                                            \
            }                                                                                                                                                \
        }                                                                                                                                                    \
                                                                                                                                                             \
        if (slist->slh_length != nth) {                                                                                                                      \
            __skip_integrity_failure_##decl("slist->slh_length (%lu) doesn't match the count (%lu) of nodes between the head and tail\n", slist->slh_length, \
                nth);                                                                                                                                        \
            n_err++;                                                                                                                                         \
            if (flags)                                                                                                                                       \
                return n_err;                                                                                                                                \
        }                                                                                                                                                    \
                                                                                                                                                             \
        return 0;                                                                                                                                            \
    }

#define SKIPLIST_DECL_ACCESS(decl, prefix, key, ktype, value, vtype, qblk, rblk)             \
    /**                                                                                      \
     * skip_get_ --                                                                          \
     *                                                                                       \
     * Get the value for the given key. In the presence of duplicate keys this               \
     * returns the value from the first duplicate.                                           \
     */                                                                                      \
    vtype prefix##skip_get_##decl(decl##_t *slist, ktype key)                                \
    {                                                                                        \
        decl##_node_t *node, query;                                                          \
                                                                                             \
        qblk;                                                                                \
        node = prefix##skip_position_eq_##decl(slist, &query);                               \
        if (node) {                                                                          \
            rblk;                                                                            \
        }                                                                                    \
        return (vtype)0;                                                                     \
    }                                                                                        \
                                                                                             \
    /**                                                                                      \
     * skip_contains_ --                                                                     \
     *                                                                                       \
     * Returns true if there is at least one match for the `key` in the list.                \
     */                                                                                      \
    int prefix##skip_contains_##decl(decl##_t *slist, ktype key)                             \
    {                                                                                        \
        decl##_node_t *node, query;                                                          \
                                                                                             \
        qblk;                                                                                \
        node = prefix##skip_position_eq_##decl(slist, &query);                               \
        if (node)                                                                            \
            return 1;                                                                        \
        return 0;                                                                            \
    }                                                                                        \
                                                                                             \
    /**                                                                                      \
     * skip_pos_ --                                                                          \
     *                                                                                       \
     * Position a "cursor" (get a "node") from the list that satisfies the                   \
     * condition (`op`) or return NULL if the condition cannot be satisfied.                 \
     * The condition is a skip_pos_##decl_t enum type:                                       \
     *                                                                                       \
     * SKIP_GT  -> greater than                                                              \
     * SKIP_GTE -> greater than or equal to                                                  \
     * SKIP_EQ  -> equal to                                                                  \
     * SKIP_LTE -> less than or equal to                                                     \
     * SKIP_LT  -> less than                                                                 \
     *                                                                                       \
     */                                                                                      \
    decl##_node_t *prefix##skip_pos_##decl(decl##_t *slist, skip_pos_##decl_t op, ktype key) \
    {                                                                                        \
        decl##_node_t *node, query;                                                          \
                                                                                             \
        qblk;                                                                                \
        node = prefix##skip_position_##decl(slist, op, &query);                              \
        if (node != slist->slh_head && node != slist->slh_tail)                              \
            return node;                                                                     \
        return NULL;                                                                         \
    }                                                                                        \
                                                                                             \
    /**                                                                                      \
     * skip_put_ --                                                                          \
     *                                                                                       \
     * Inserts `key` into the list within a node that contains `value`.                      \
     */                                                                                      \
    int prefix##skip_put_##decl(decl##_t *slist, ktype key, vtype value)                     \
    {                                                                                        \
        int rc;                                                                              \
        decl##_node_t *node;                                                                 \
        rc = prefix##skip_alloc_node_##decl(&node);                                          \
        if (rc)                                                                              \
            return rc;                                                                       \
        node->key = key;                                                                     \
        node->value = value;                                                                 \
        rc = prefix##skip_insert_##decl(slist, node);                                        \
        if (rc)                                                                              \
            prefix##skip_free_node_##decl(slist, node);                                      \
        return rc;                                                                           \
    }                                                                                        \
                                                                                             \
    /**                                                                                      \
     * skip_dup_ --                                                                          \
     *                                                                                       \
     * Inserts `key` into the list allowing for duplicates within a node that                \
     * contains `value`.                                                                     \
     */                                                                                      \
    int prefix##skip_dup_##decl(decl##_t *slist, ktype key, vtype value)                     \
    {                                                                                        \
        int rc;                                                                              \
        decl##_node_t *node;                                                                 \
        rc = prefix##skip_alloc_node_##decl(&node);                                          \
        if (rc)                                                                              \
            return rc;                                                                       \
        node->key = key;                                                                     \
        node->value = value;                                                                 \
        rc = prefix##skip_insert_dup_##decl(slist, node);                                    \
        if (rc)                                                                              \
            prefix##skip_free_node_##decl(slist, node);                                      \
        return rc;                                                                           \
    }                                                                                        \
                                                                                             \
    /**                                                                                      \
     * skip_set_ --                                                                          \
     *                                                                                       \
     * Updates in-place the node to contain the new `value`. In the presence of              \
     * duplicate keys in the list, the first key's value will be updated.                    \
     */                                                                                      \
    int prefix##skip_set_##decl(decl##_t *slist, ktype key, vtype value)                     \
    {                                                                                        \
        decl##_node_t node;                                                                  \
        node.key = key;                                                                      \
        return prefix##skip_update_##decl(slist, &node, (void *)value);                      \
    }                                                                                        \
                                                                                             \
    /**                                                                                      \
     * skip_del_ --                                                                          \
     *                                                                                       \
     * Removes the node from the list with a matching `key`. In the presence of              \
     * duplicate keys in the list, this will remove the first duplicate.                     \
     */                                                                                      \
    int prefix##skip_del_##decl(decl##_t *slist, ktype key)                                  \
    {                                                                                        \
        decl##_node_t node;                                                                  \
        node.key = key;                                                                      \
        return prefix##skip_remove_node_##decl(slist, &node);                                \
    }

#define SKIPLIST_DECL_DOT(decl, prefix, field)                                                                                      \
                                                                                                                                    \
    /* A type for a function that writes into a char[2048] buffer                                                                   \
     * a description of the value within the node. */                                                                               \
    typedef void (*skip_sprintf_node_##decl##_t)(decl##_node_t *, char *);                                                          \
                                                                                                                                    \
    /* -- __skip_dot_node_                                                                                                          \
     * Writes out a fragment of a DOT file representing a node.                                                                     \
     */                                                                                                                             \
    static size_t __skip_dot_width_##decl(decl##_t *slist, decl##_node_t *from, decl##_node_t *to)                                  \
    {                                                                                                                               \
        size_t w = 1;                                                                                                               \
        decl##_node_t *n = to;                                                                                                      \
                                                                                                                                    \
        if (from == NULL || to == NULL)                                                                                             \
            return 0;                                                                                                               \
                                                                                                                                    \
        while (n->field.sle_prev != from) {                                                                                         \
            w++;                                                                                                                    \
            n = prefix##skip_prev_node_##decl(slist, n);                                                                            \
        }                                                                                                                           \
                                                                                                                                    \
        return w;                                                                                                                   \
    }                                                                                                                               \
                                                                                                                                    \
    static inline void __skip_dot_write_node_##decl(FILE *os, size_t nsg, decl##_node_t *node)                                      \
    {                                                                                                                               \
        if (node)                                                                                                                   \
            fprintf(os, "\"node%lu %p\"", nsg, (void *)node);                                                                       \
        else                                                                                                                        \
            fprintf(os, "\"node%lu NULL\"", nsg);                                                                                   \
    }                                                                                                                               \
                                                                                                                                    \
    /* -- __skip_dot_node_                                                                                                          \
     * Writes out a fragment of a DOT file representing a node.                                                                     \
     */                                                                                                                             \
    static void __skip_dot_node_##decl(FILE *os, decl##_t *slist, decl##_node_t *node, size_t nsg, skip_sprintf_node_##decl##_t fn) \
    {                                                                                                                               \
        char buf[2048];                                                                                                             \
        decl##_node_t *next;                                                                                                        \
                                                                                                                                    \
        __skip_dot_write_node_##decl(os, nsg, node);                                                                                \
        fprintf(os, " [label = \"");                                                                                                \
        fflush(os);                                                                                                                 \
        __SKIP_ENTRIES_T2B(field, node)                                                                                             \
        {                                                                                                                           \
            next = (node->field.sle_levels[lvl].next == slist->slh_tail) ? NULL : node->field.sle_levels[lvl].next;                 \
            (void)__skip_dot_width_##decl;                                                                                          \
            /* size_t width = __skip_dot_width_##decl(slist, node, next ? next : slist->slh_tail); */                               \
            fprintf(os, " { <w%lu> %lu | <f%lu> ", lvl, node->field.sle_levels[lvl].hits, lvl);                                     \
            if (next)                                                                                                               \
                fprintf(os, "%p } |", (void *)next);                                                                                \
            else                                                                                                                    \
                fprintf(os, "0x0 } |");                                                                                             \
            fflush(os);                                                                                                             \
        }                                                                                                                           \
        if (fn) {                                                                                                                   \
            fn(node, buf);                                                                                                          \
            fprintf(os, " <f0> \u219F %lu \u226B %s \"\n", node->field.sle_height, buf);                                            \
        } else {                                                                                                                    \
            fprintf(os, " <f0> \u219F %lu \"\n", node->field.sle_height);                                                           \
        }                                                                                                                           \
        fprintf(os, "shape = \"record\"\n");                                                                                        \
        fprintf(os, "];\n");                                                                                                        \
        fflush(os);                                                                                                                 \
                                                                                                                                    \
        /* Now edges */                                                                                                             \
        __SKIP_ENTRIES_B2T(field, node)                                                                                             \
        {                                                                                                                           \
            next = (node->field.sle_levels[lvl].next == slist->slh_tail) ? NULL : node->field.sle_levels[lvl].next;                 \
            __skip_dot_write_node_##decl(os, nsg, node);                                                                            \
            fprintf(os, ":f%lu -> ", lvl);                                                                                          \
            __skip_dot_write_node_##decl(os, nsg, next);                                                                            \
            fprintf(os, ":w%lu [];\n", lvl);                                                                                        \
            fflush(os);                                                                                                             \
        }                                                                                                                           \
    }                                                                                                                               \
                                                                                                                                    \
    /* -- _skip_dot_finish_                                                                                                         \
     * Finalize the DOT file of the internal representation.                                                                        \
     */                                                                                                                             \
    void prefix##skip_dot_end_##decl(FILE *os, size_t nsg)                                                                          \
    {                                                                                                                               \
        size_t i;                                                                                                                   \
        if (nsg > 0) {                                                                                                              \
            /* Link the nodes together with an invisible node.                                                                      \
             *    node0 [shape=record, label = "<f0> | <f1> | <f2> | <f3> |                                                         \
             * <f4> | <f5> | <f6> | <f7> | <f8> | ", style=invis, width=0.01];                                                      \
             */                                                                                                                     \
            fprintf(os, "node0 [shape=record, label = \"");                                                                         \
            for (i = 0; i < nsg; ++i) {                                                                                             \
                fprintf(os, "<f%lu> | ", i);                                                                                        \
            }                                                                                                                       \
            fprintf(os, "\", style=invis, width=0.01];\n");                                                                         \
                                                                                                                                    \
            /* Now connect nodes with invisible edges                                                                               \
             *                                                                                                                      \
             *    node0:f0 -> HeadNode [style=invis];                                                                               \
             *    node0:f1 -> HeadNode1 [style=invis];                                                                              \
             */                                                                                                                     \
            for (i = 0; i < nsg; ++i) {                                                                                             \
                fprintf(os, "node0:f%lu -> HeadNode%lu [style=invis];\n", i, i);                                                    \
            }                                                                                                                       \
            nsg = 0;                                                                                                                \
        }                                                                                                                           \
        fprintf(os, "}\n");                                                                                                         \
    }                                                                                                                               \
                                                                                                                                    \
    /* -- skip_dot_                                                                                                                 \
     * Create a DOT file of the internal representation of the                                                                      \
     * Skiplist on the provided file descriptor (default: STDOUT).                                                                  \
     *                                                                                                                              \
     * To view the output:                                                                                                          \
     * $ dot -Tps filename.dot -o outfile.ps                                                                                        \
     * You can change the output format by varying the value after -T and                                                           \
     * choosing an appropriate filename extension after -o.                                                                         \
     * See: https://graphviz.org/docs/outputs/ for the format options.                                                              \
     *                                                                                                                              \
     * https://en.wikipedia.org/wiki/DOT_(graph_description_language)                                                               \
     */                                                                                                                             \
    int prefix##skip_dot_##decl(FILE *os, decl##_t *slist, size_t nsg, char *msg, skip_sprintf_node_##decl##_t fn)                  \
    {                                                                                                                               \
        int letitgo = 0;                                                                                                            \
        size_t i;                                                                                                                   \
        decl##_node_t *node, *next;                                                                                                 \
                                                                                                                                    \
        if (slist == NULL || fn == NULL)                                                                                            \
            return nsg;                                                                                                             \
        /*if (__skip_integrity_check_##decl(slist, 1) != 0) {                                                                       \
            perror("Skiplist failed integrity checks, impossible to diagram.");                                                     \
            return -1;                                                                                                              \
        }*/                                                                                                                         \
        if (nsg == 0) {                                                                                                             \
            fprintf(os, "digraph Skiplist {\n");                                                                                    \
            fprintf(os, "label = \"Skiplist.\"\n");                                                                                 \
            fprintf(os, "graph [rankdir = \"LR\"];\n");                                                                             \
            fprintf(os, "node [fontsize = \"12\" shape = \"ellipse\"];\n");                                                         \
            fprintf(os, "edge [];\n\n");                                                                                            \
        }                                                                                                                           \
        fprintf(os, "subgraph cluster%lu {\n", nsg);                                                                                \
        fprintf(os, "style=dashed\n");                                                                                              \
        fprintf(os, "label=\"Skiplist [%lu]", nsg);                                                                                 \
        if (msg)                                                                                                                    \
            fprintf(os, " %s", msg);                                                                                                \
        fprintf(os, "\"\n\n");                                                                                                      \
        fprintf(os, "\"HeadNode%lu\" [\n", nsg);                                                                                    \
        fprintf(os, "label = \"");                                                                                                  \
                                                                                                                                    \
        if (slist->slh_head->field.sle_height || slist->slh_head->field.sle_levels[0].next != slist->slh_tail)                      \
            letitgo = 1;                                                                                                            \
                                                                                                                                    \
        /* Write out the fields */                                                                                                  \
        node = slist->slh_head;                                                                                                     \
        if (letitgo) {                                                                                                              \
            __SKIP_ENTRIES_T2B(field, node)                                                                                         \
            {                                                                                                                       \
                next = (node->field.sle_levels[lvl].next == slist->slh_tail) ? NULL : node->field.sle_levels[lvl].next;             \
                /* size_t width = __skip_dot_width_##decl(slist, node, next ? next : slist->slh_tail); */                           \
                fprintf(os, "{ %lu | <f%lu> ", node->field.sle_levels[lvl].hits, lvl);                                              \
                if (next)                                                                                                           \
                    fprintf(os, "%p }", (void *)next);                                                                              \
                else                                                                                                                \
                    fprintf(os, "0x0 }");                                                                                           \
                __SKIP_IS_LAST_ENTRY_T2B() continue;                                                                                \
                fprintf(os, " | ");                                                                                                 \
            }                                                                                                                       \
        } else {                                                                                                                    \
            fprintf(os, "Empty HeadNode");                                                                                          \
        }                                                                                                                           \
        fprintf(os, "\"\n");                                                                                                        \
        fprintf(os, "shape = \"record\"\n");                                                                                        \
        fprintf(os, "];\n");                                                                                                        \
        fflush(os);                                                                                                                 \
                                                                                                                                    \
        /* Edges for head node */                                                                                                   \
        node = slist->slh_head;                                                                                                     \
        if (letitgo) {                                                                                                              \
            node = slist->slh_head;                                                                                                 \
            __SKIP_ENTRIES_B2T(field, node)                                                                                         \
            {                                                                                                                       \
                next = (node->field.sle_levels[lvl].next == slist->slh_tail) ? NULL : node->field.sle_levels[lvl].next;             \
                fprintf(os, "\"HeadNode%lu\":f%lu -> ", nsg, lvl);                                                                  \
                __skip_dot_write_node_##decl(os, nsg, next);                                                                        \
                fprintf(os, ":w%lu [];\n", lvl);                                                                                    \
            }                                                                                                                       \
            fprintf(os, "\n");                                                                                                      \
        }                                                                                                                           \
        fflush(os);                                                                                                                 \
                                                                                                                                    \
        /* Now all nodes via level 0, if non-empty */                                                                               \
        node = slist->slh_head;                                                                                                     \
        if (letitgo) {                                                                                                              \
            SKIPLIST_FOREACH_H2T(decl, prefix, field, slist, next, i)                                                               \
            {                                                                                                                       \
                ((void)i);                                                                                                          \
                __skip_dot_node_##decl(os, slist, next, nsg, fn);                                                                   \
                fflush(os);                                                                                                         \
            }                                                                                                                       \
            fprintf(os, "\n");                                                                                                      \
        }                                                                                                                           \
        fflush(os);                                                                                                                 \
                                                                                                                                    \
        /* The tail, sentinel node */                                                                                               \
        if (letitgo) {                                                                                                              \
            __skip_dot_write_node_##decl(os, nsg, NULL);                                                                            \
            fprintf(os, " [label = \"");                                                                                            \
            node = slist->slh_tail;                                                                                                 \
            size_t th = slist->slh_head->field.sle_height;                                                                          \
            for (size_t lvl = th; lvl != (size_t)-1; lvl--) {                                                                       \
                next = (node->field.sle_levels[lvl].next == slist->slh_tail) ? NULL : node->field.sle_levels[lvl].next;             \
                fprintf(os, "<w%lu> 0x0", lvl);                                                                                     \
                __SKIP_IS_LAST_ENTRY_T2B() continue;                                                                                \
                fprintf(os, " | ");                                                                                                 \
            }                                                                                                                       \
            fprintf(os, "\" shape = \"record\"];\n");                                                                               \
        }                                                                                                                           \
                                                                                                                                    \
        /* End: "subgraph cluster0 {" */                                                                                            \
        fprintf(os, "}\n\n");                                                                                                       \
        nsg += 1;                                                                                                                   \
        fflush(os);                                                                                                                 \
                                                                                                                                    \
        return nsg;                                                                                                                 \
    }

#endif /* _SKIPLIST_H_ */
