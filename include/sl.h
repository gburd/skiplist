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
#include <sys/param.h>

#include <assert.h>
#include <errno.h>
#include <math.h>
#include <stdalign.h>
#include <stdarg.h>
#include <stdatomic.h>
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
 * It is a probabilistic datastructure, meaning that it does not guarantee
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

/**
 * -- __skip_diag__
 *
 * Write debug message to stderr with origin of message.
 */
void __attribute__((format(printf, 4, 5)))
__skip_diag_(const char *file, int line, const char *func, const char *format, ...)
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

#ifndef SKIPLIST_SPLAY_INTERVAL
#define SKIPLIST_SPLAY_INTERVAL 64
#endif

/*
 * Every Skiplist node has to have an additional section of data used to manage
 * nodes in the list. The rest of the datastructure is defined by the use case.
 * This housekeeping portion is the SKIPLIST_ENTRY, see below.  It maintains the
 * array of forward pointers to nodes and has a height, this height is a
 * zero-based count of levels, so a height of `0` means one (1) level and a
 * height of `4` means five (5) forward pointers (levels) in the node, [0-4).
 */
#define SKIPLIST_ENTRY(decl)                          \
    struct __skiplist_##decl##_entry {                \
        size_t sle_era;                               \
        _Atomic(size_t) sle_height;                   \
        _Atomic(struct decl##_node *) sle_prev;       \
        struct __skiplist_##decl##_level {            \
            _Atomic(struct decl##_node *) next;       \
            _Atomic(size_t) hits;                     \
        } *sle_levels;                                \
    }

#define SKIPLIST_FOREACH_H2T(decl, prefix, field, list, elm, iter) \
    for (iter = 0, (elm) = (list)->slh_head; \
         ((elm) = atomic_load_explicit(&(elm)->field.sle_levels[0].next, memory_order_acquire)) != (list)->slh_tail; \
         iter++)

/* Iterate from tail to head over the nodes. */
#define SKIPLIST_FOREACH_T2H(decl, prefix, field, list, elm, iter) \
    for (iter = atomic_load_explicit(&(list)->slh_length, memory_order_relaxed), \
         (elm) = (list)->slh_tail; \
         ((elm) = atomic_load_explicit(&(elm)->field.sle_prev, memory_order_acquire)) != (list)->slh_head; \
         iter--)

/* Iterate over the next pointers in a node from bottom to top (B2T) or top to bottom (T2B). */
#define __SKIP_ALL_ENTRIES_T2B(field, elm) for (size_t lvl = slist->slh_head->field.sle_height - 1; lvl != (size_t) - 1; lvl--)
#define __SKIP_ENTRIES_T2B(field, elm) for (size_t lvl = elm->field.sle_height; lvl != (size_t) - 1; lvl--)
#define __SKIP_ENTRIES_T2B_FROM(field, elm, off) for (size_t lvl = off; lvl != (size_t) - 1; lvl--)
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
 * Marked pointer support for lock-free operations.
 * Uses the low bit of node pointers as a logical deletion flag.
 * Nodes are heap-allocated with >= 8-byte alignment, so the low bit
 * is always 0 for valid unmarked pointers.
 */
#define __SKIP_IS_MARKED(p) ((uintptr_t)(p) & 1)
#define __SKIP_MARK(p) ((__typeof__(p))((uintptr_t)(p) | 1))
#define __SKIP_UNMARK(p) ((__typeof__(p))((uintptr_t)(p) & ~(uintptr_t)1))

/*
 * Splay rebalancing: opt-in via -DSKIPLIST_SPLAY_REBALANCE at compile time.
 * When enabled, node heights adapt based on access frequency. When disabled
 * (default), the skiplist uses standard randomized heights with no rebalancing.
 */
#ifdef SKIPLIST_SPLAY_REBALANCE
#define SKIPLIST_SPLAY_IMPL(decl, slist, len, path)                                \
    do {                                                                           \
        uint32_t __splay_cnt = atomic_fetch_add_explicit(                          \
            &(slist)->slh_splay_counter, 1, memory_order_relaxed);                 \
        if ((__splay_cnt & (SKIPLIST_SPLAY_INTERVAL - 1)) == 0) {                  \
            __fix_skip_rebalance_##decl(slist, len, path);                          \
        }                                                                          \
    } while (0)
#else
#define SKIPLIST_SPLAY_IMPL(decl, slist, len, path) \
    do {                                            \
        (void)(slist);                              \
        (void)(len);                                \
        (void)(path);                               \
    } while (0)
#endif

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
        _Atomic(size_t) slh_length;                                                                                                      \
        void *slh_aux;                                                                                                                   \
        void *slh_ebr;                                         /* EBR state (NULL for single-threaded use) */                             \
        void (*slh_ebr_retire)(void *, struct decl *, struct decl##_node *); /* EBR retire callback */                                   \
        _Atomic(uint32_t) slh_prng_state;                                                                                                 \
        _Atomic(uint32_t) slh_splay_counter;                                                                                             \
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
        decl##_node_t *node; /* predecessor node traversed during location */                                                             \
        decl##_node_t *succ; /* successor node at this level (for lock-free CAS) */                                                       \
        size_t in;           /* level at which the node was intersected */                                                               \
        size_t pu;           /* see "partial sums trick" */                                                                              \
    } __skiplist_path_##decl##_t;                                                                                                        \
                                                                                                                                         \
    /* Xorshift algorithm for PRNG (lock-free via CAS loop) */                                                                            \
    static uint32_t __##decl##_xorshift32(_Atomic(uint32_t) *state)                                                                      \
    {                                                                                                                                    \
        uint32_t old_state, new_state;                                                                                                   \
        do {                                                                                                                             \
            old_state = atomic_load_explicit(state, memory_order_relaxed);                                                               \
            if (old_state == 0)                                                                                                          \
                old_state = 123456789;                                                                                                   \
            new_state = old_state;                                                                                                       \
            new_state ^= new_state << 13;                                                                                                \
            new_state ^= new_state >> 17;                                                                                                \
            new_state ^= new_state << 5;                                                                                                 \
        } while (!atomic_compare_exchange_weak_explicit(state, &old_state, new_state,                                                    \
                     memory_order_relaxed, memory_order_relaxed));                                                                        \
        return new_state;                                                                                                                \
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
        double random_value = (double)__##decl##_xorshift32(&slist->slh_prng_state) / UINT32_MAX;                                        \
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
           by array size. (16/24 bytes on 32/64 bit systems) */                                                                          \
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
        slist->slh_ebr = NULL;                                                                                                           \
        slist->slh_ebr_retire = NULL;                                                                                                    \
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
        /* Initial height is 1 (level 0 active). Initialize ALL levels so that                                                             \
           head->next[i] = tail for every i. This ensures the delete shrinkage                                                            \
           loop (which scans for head->next[i] != tail) terminates correctly. */                                                          \
        slist->slh_head->field.sle_height = 1;                                                                                           \
        for (i = 0; i < SKIPLIST_MAX_HEIGHT; i++)                                                                                        \
            slist->slh_head->field.sle_levels[i].next = slist->slh_tail;                                                                 \
        slist->slh_head->field.sle_prev = NULL;                                                                                          \
                                                                                                                                         \
        /* Tail: all next pointers are NULL */                                                                                           \
        slist->slh_tail->field.sle_height = slist->slh_head->field.sle_height;                                                           \
        for (i = 0; i < SKIPLIST_MAX_HEIGHT; i++)                                                                                        \
            slist->slh_tail->field.sle_levels[i].next = NULL;                                                                            \
        slist->slh_tail->field.sle_prev = slist->slh_head;                                                                               \
        slist->slh_prng_state = ((uint32_t)time(NULL) ^ (uint32_t)getpid());                                                              \
        slist->slh_splay_counter = 0;                                                                                                    \
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
        return atomic_load_explicit(&slist->slh_length, memory_order_relaxed);                                                           \
    }                                                                                                                                    \
                                                                                                                                         \
    /**                                                                                                                                  \
     * -- skip_is_empty_                                                                                                                 \
     *                                                                                                                                   \
     * Returns non-zero when the list is empty.                                                                                          \
     */                                                                                                                                  \
    int prefix##skip_is_empty_##decl(decl##_t *slist)                                                                                    \
    {                                                                                                                                    \
        return atomic_load_explicit(&slist->slh_length, memory_order_relaxed) == 0;                                                      \
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
        decl##_node_t *first = atomic_load_explicit(&slist->slh_head->field.sle_levels[0].next, memory_order_acquire);                   \
        return first == slist->slh_tail ? NULL : first;                                                                                  \
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
        decl##_node_t *last = atomic_load_explicit(&slist->slh_tail->field.sle_prev, memory_order_acquire);                              \
        return last == slist->slh_head ? NULL : last;                                                                                    \
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
        decl##_node_t *next = atomic_load_explicit(&n->field.sle_levels[0].next, memory_order_acquire);                                  \
        if (next == slist->slh_tail)                                                                                                     \
            return NULL;                                                                                                                 \
        return next;                                                                                                                     \
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
        decl##_node_t *prev = atomic_load_explicit(&n->field.sle_prev, memory_order_acquire);                                            \
        if (prev == slist->slh_head)                                                                                                     \
            return NULL;                                                                                                                 \
        return prev;                                                                                                                     \
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
        node = atomic_load_explicit(&slist->slh_head->field.sle_levels[0].next, memory_order_acquire);                                    \
        while (node != slist->slh_tail) {                                                                                                \
            next = atomic_load_explicit(&node->field.sle_levels[0].next, memory_order_acquire);                                          \
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
     * -- __skip_adjust_hit_counts_                                                                                                      \
     *                                                                                                                                   \
     * When the total hit count (stored at slh_head's top+1 level) exceeds                                                               \
     * SIZE_MAX / 2, halve all hit counters across all nodes to prevent                                                                  \
     * overflow while preserving relative ordering.                                                                                      \
     */                                                                                                                                  \
    static void __skip_adjust_hit_counts_##decl(decl##_t *slist)                                                                         \
    {                                                                                                                                    \
        size_t total_hits, lvl, nth;                                                                                                     \
        decl##_node_t *node;                                                                                                             \
                                                                                                                                         \
        if (slist == NULL)                                                                                                               \
            return;                                                                                                                      \
                                                                                                                                         \
        total_hits = atomic_load_explicit(                                                                                                \
            &slist->slh_head->field.sle_levels[atomic_load_explicit(&slist->slh_head->field.sle_height, memory_order_relaxed)].hits,      \
            memory_order_acquire);                                                                                                       \
        if (total_hits < SIZE_MAX / 2)                                                                                                   \
            return;                                                                                                                      \
                                                                                                                                         \
        /* Halve all hit counters on every node at every level using CAS.                                                                  \
           CAS ensures correctness under concurrency: if another thread                                                                  \
           modifies a counter between our load and store, the CAS fails                                                                  \
           and we retry with the updated value. */                                                                                       \
        node = slist->slh_head;                                                                                                          \
        for (lvl = 0; lvl <= atomic_load_explicit(&node->field.sle_height, memory_order_relaxed); lvl++) {                               \
            size_t old_val = atomic_load_explicit(&node->field.sle_levels[lvl].hits, memory_order_relaxed);                              \
            size_t new_val;                                                                                                              \
            do {                                                                                                                         \
                new_val = (old_val + 1) / 2;                                                                                             \
            } while (!atomic_compare_exchange_weak_explicit(                                                                             \
                &node->field.sle_levels[lvl].hits, &old_val, new_val,                                                                    \
                memory_order_relaxed, memory_order_relaxed));                                                                            \
        }                                                                                                                                \
                                                                                                                                         \
        SKIPLIST_FOREACH_H2T(decl, prefix, field, slist, node, nth)                                                                      \
        {                                                                                                                                \
            (void)nth;                                                                                                                   \
            for (lvl = 0; lvl <= atomic_load_explicit(&node->field.sle_height, memory_order_relaxed); lvl++) {                           \
                size_t old_val = atomic_load_explicit(&node->field.sle_levels[lvl].hits, memory_order_relaxed);                          \
                size_t new_val;                                                                                                          \
                do {                                                                                                                     \
                    new_val = (old_val + 1) / 2;                                                                                         \
                } while (!atomic_compare_exchange_weak_explicit(                                                                         \
                    &node->field.sle_levels[lvl].hits, &old_val, new_val,                                                                \
                    memory_order_relaxed, memory_order_relaxed));                                                                        \
            }                                                                                                                            \
        }                                                                                                                                \
    }                                                                                                                                    \
                                                                                                                                         \
    static decl##_node_t *__skip_splay_find_pred_at_level_##decl(                                                                        \
        decl##_t *slist, decl##_node_t *target, size_t level)                                                                            \
    {                                                                                                                                    \
        decl##_node_t *scan, *fwd;                                                                                                       \
        size_t steps = 0;                                                                                                                \
        const size_t MAX_BACK_SCAN = 128;                                                                                                \
                                                                                                                                         \
        scan = atomic_load_explicit(&target->field.sle_prev, memory_order_acquire);                                                      \
        while (scan != slist->slh_head && steps < MAX_BACK_SCAN) {                                                                       \
            if (__SKIP_IS_MARKED(scan)) {                                                                                                \
                scan = __SKIP_UNMARK(scan);                                                                                              \
                scan = atomic_load_explicit(&scan->field.sle_prev, memory_order_acquire);                                                \
                steps++;                                                                                                                 \
                continue;                                                                                                                \
            }                                                                                                                            \
            size_t scan_h = atomic_load_explicit(&scan->field.sle_height, memory_order_acquire);                                         \
            if (scan_h >= level) {                                                                                                       \
                fwd = atomic_load_explicit(&scan->field.sle_levels[level].next, memory_order_acquire);                                   \
                if (fwd == target) {                                                                                                     \
                    return scan;                                                                                                         \
                }                                                                                                                        \
            }                                                                                                                            \
            scan = atomic_load_explicit(&scan->field.sle_prev, memory_order_acquire);                                                    \
            steps++;                                                                                                                     \
        }                                                                                                                                \
                                                                                                                                         \
        /* Check head as last resort. */                                                                                                 \
        if (atomic_load_explicit(&slist->slh_head->field.sle_height, memory_order_relaxed) >= level) {                                   \
            fwd = atomic_load_explicit(&slist->slh_head->field.sle_levels[level].next, memory_order_acquire);                            \
            if (fwd == target)                                                                                                           \
                return slist->slh_head;                                                                                                  \
        }                                                                                                                                \
                                                                                                                                         \
        return NULL;                                                                                                                     \
    }                                                                                                                                    \
                                                                                                                                         \
    static void __fix_skip_rebalance_##decl(decl##_t *slist, size_t len, __skiplist_path_##decl##_t path[])                               \
    {                                                                                                                                    \
        size_t i, node_height, delta_height;                                                                                             \
        size_t k_threshold, m_total_hits;                                                                                                \
        double asc_cond, dsc_cond;                                                                                                       \
        decl##_node_t *node, *pred, *succ, *expected;                                                                                    \
                                                                                                                                         \
        if (len < 2)                                                                                                                     \
            return;                                                                                                                      \
                                                                                                                                         \
        /* Read global counters. These are approximate under concurrency,                                                                \
         * which is fine: splay rebalancing is a heuristic optimization,                                                                 \
         * not a correctness requirement. */                                                                                             \
        k_threshold = atomic_load_explicit(                                                                                              \
            &slist->slh_head->field.sle_height, memory_order_acquire);                                                                   \
        m_total_hits = atomic_load_explicit(                                                                                             \
            &slist->slh_head->field.sle_levels[k_threshold].hits,                                                                        \
            memory_order_relaxed);                                                                                                       \
                                                                                                                                         \
        /* Need at least some history to make meaningful decisions. */                                                                   \
        if (m_total_hits < 4 || k_threshold < 1)                                                                                         \
            return;                                                                                                                      \
                                                                                                                                         \
        /* Process each node in the search path. path[1..len] are the                                                                    \
         * predecessors recorded during locate; path[0] is the match.                                                                    \
         * We only rebalance the actual nodes on the path, skipping                                                                      \
         * head and tail sentinels. */                                                                                                   \
        for (i = 1; i <= len; i++) {                                                                                                     \
            node = path[i].node;                                                                                                         \
            if (node == NULL || node == slist->slh_head || node == slist->slh_tail)                                                      \
                continue;                                                                                                                \
                                                                                                                                         \
            /* Skip marked (logically deleted) nodes. */                                                                                 \
            {                                                                                                                            \
                decl##_node_t *lvl0_next = atomic_load_explicit(                                                                         \
                    &node->field.sle_levels[0].next, memory_order_acquire);                                                              \
                if (__SKIP_IS_MARKED(lvl0_next))                                                                                         \
                    continue;                                                                                                            \
            }                                                                                                                            \
                                                                                                                                         \
            node_height = atomic_load_explicit(                                                                                          \
                &node->field.sle_height, memory_order_acquire);                                                                          \
                                                                                                                                         \
            /* Read this node's hit count at level 0 (total accesses). */                                                                \
            size_t u_hits = atomic_load_explicit(                                                                                        \
                &node->field.sle_levels[0].hits, memory_order_relaxed);                                                                  \
                                                                                                                                         \
            if (node_height >= k_threshold)                                                                                              \
                delta_height = 0;                                                                                                        \
            else                                                                                                                         \
                delta_height = k_threshold - node_height;                                                                                \
                                                                                                                                         \
            /* ---- DEMOTION CHECK ----                                                                                                  \
             *                                                                                                                           \
             * Condition: u_hits <= m_total_hits / 2^delta_height                                                                        \
             *                                                                                                                           \
             * A node with few hits relative to its height is over-promoted.                                                             \
             * We remove it from its top level to push it down. */                                                                       \
            dsc_cond = (double)m_total_hits / pow(2.0, (double)delta_height);                                                            \
            if (u_hits <= (size_t)dsc_cond && node_height > 0) {                                                                         \
                size_t top = node_height;                                                                                                \
                                                                                                                                         \
                /* Step 1: Find predecessor at the top level. */                                                                         \
                pred = __skip_splay_find_pred_at_level_##decl(slist, node, top);                                                         \
                if (pred == NULL) {                                                                                                      \
                    /* Cannot find predecessor; skip demotion this round.                                                                \
                     * This is safe: the node just stays at its current                                                                  \
                     * height until the next rebalance finds the pred. */                                                                \
                    goto __splay_check_ascent_##decl;                                                                                    \
                }                                                                                                                        \
                                                                                                                                         \
                /* Step 2: CAS predecessor's next[top] to skip this node.                                                                \
                 *                                                                                                                       \
                 * We expect: pred->levels[top].next == node                                                                             \
                 * We want:   pred->levels[top].next == node->levels[top].next                                                           \
                 *                                                                                                                       \
                 * Release ordering ensures the pointer update is visible                                                                \
                 * to any thread that subsequently reads this level. */                                                                  \
                succ = atomic_load_explicit(                                                                                             \
                    &node->field.sle_levels[top].next, memory_order_acquire);                                                            \
                                                                                                                                         \
                /* Don't demote if the successor is marked (concurrent delete). */                                                       \
                if (__SKIP_IS_MARKED(succ))                                                                                              \
                    goto __splay_check_ascent_##decl;                                                                                    \
                                                                                                                                         \
                expected = node;                                                                                                         \
                if (atomic_compare_exchange_strong_explicit(                                                                             \
                        &pred->field.sle_levels[top].next,                                                                               \
                        &expected, succ,                                                                                                 \
                        memory_order_release, memory_order_relaxed)) {                                                                   \
                                                                                                                                         \
                    /* Step 3: Atomically decrement the node's height.                                                                   \
                     *                                                                                                                   \
                     * We use a CAS rather than fetch_sub to ensure we                                                                   \
                     * only decrement if the height hasn't changed since                                                                 \
                     * we read it (another thread might have promoted or                                                                 \
                     * demoted concurrently). */                                                                                         \
                    size_t expected_h = node_height;                                                                                     \
                    atomic_compare_exchange_strong_explicit(                                                                             \
                        &node->field.sle_height,                                                                                         \
                        &expected_h, node_height - 1,                                                                                    \
                        memory_order_release, memory_order_relaxed);                                                                     \
                                                                                                                                         \
                    /* Transfer hits from the demoted level to the predecessor.                                                          \
                     * This preserves hit count totals approximately. */                                                                 \
                    size_t demoted_hits = atomic_load_explicit(                                                                           \
                        &node->field.sle_levels[top].hits, memory_order_relaxed);                                                        \
                    atomic_fetch_add_explicit(                                                                                           \
                        &pred->field.sle_levels[top].hits,                                                                               \
                        demoted_hits, memory_order_relaxed);                                                                             \
                    atomic_store_explicit(                                                                                                \
                        &node->field.sle_levels[top].hits, 0, memory_order_relaxed);                                                     \
                }                                                                                                                        \
                /* If CAS fails, another thread modified the link. That's                                                                \
                 * fine: we just skip demotion this round. */                                                                            \
                                                                                                                                         \
                /* Re-read height after potential demotion for ascent check. */                                                          \
                node_height = atomic_load_explicit(                                                                                      \
                    &node->field.sle_height, memory_order_acquire);                                                                      \
                if (node_height >= k_threshold)                                                                                          \
                    delta_height = 0;                                                                                                    \
                else                                                                                                                     \
                    delta_height = k_threshold - node_height;                                                                            \
            }                                                                                                                            \
                                                                                                                                         \
        __splay_check_ascent_##decl:                                                                                                     \
            /* ---- PROMOTION CHECK ----                                                                                                 \
             *                                                                                                                           \
             * Condition: u_hits > m_total_hits / 2^(delta_height - 1)                                                                   \
             *                                                                                                                           \
             * A node with many hits relative to its height is under-promoted.                                                           \
             * We add a new level to bring it higher in the structure. */                                                                \
            if (delta_height < 1)                                                                                                        \
                continue;                                                                                                                \
            asc_cond = (double)m_total_hits / pow(2.0, (double)(delta_height - 1));                                                      \
            if (u_hits <= (size_t)asc_cond)                                                                                              \
                continue;                                                                                                                \
            if (node_height >= SKIPLIST_MAX_HEIGHT - 1)                                                                                  \
                continue;                                                                                                                \
            if (node_height >= k_threshold)                                                                                              \
                continue;                                                                                                                \
                                                                                                                                         \
            /* Step 1: Grow list height if needed.                                                                                       \
             *                                                                                                                           \
             * If promoting this node would exceed the current list height,                                                              \
             * grow the head/tail first. This mirrors the logic in insert. */                                                            \
            {                                                                                                                            \
                size_t new_node_h = node_height + 1;                                                                                     \
                size_t cur_head_h = atomic_load_explicit(                                                                                \
                    &slist->slh_head->field.sle_height, memory_order_acquire);                                                           \
                if (new_node_h >= cur_head_h) {                                                                                          \
                    /* Check if total hits justify growing the list. */                                                                  \
                    size_t expected_h = (size_t)floor(log2((double)m_total_hits));                                                        \
                    if (expected_h > cur_head_h && expected_h < SKIPLIST_MAX_HEIGHT) {                                                    \
                        size_t old_h = cur_head_h;                                                                                       \
                        if (atomic_compare_exchange_strong_explicit(                                                                     \
                                &slist->slh_head->field.sle_height,                                                                      \
                                &old_h, expected_h,                                                                                      \
                                memory_order_release, memory_order_acquire)) {                                                           \
                            /* Initialize the new head levels. */                                                                        \
                            for (size_t h = old_h + 1; h <= expected_h; h++) {                                                           \
                                atomic_store_explicit(                                                                                   \
                                    &slist->slh_head->field.sle_levels[h].next,                                                          \
                                    slist->slh_tail, memory_order_relaxed);                                                              \
                                atomic_store_explicit(                                                                                   \
                                    &slist->slh_head->field.sle_levels[h].hits,                                                          \
                                    atomic_load_explicit(                                                                                \
                                        &slist->slh_head->field.sle_levels[old_h].hits,                                                 \
                                        memory_order_relaxed),                                                                          \
                                    memory_order_relaxed);                                                                               \
                            }                                                                                                            \
                            atomic_store_explicit(                                                                                       \
                                &slist->slh_tail->field.sle_height,                                                                      \
                                expected_h, memory_order_release);                                                                       \
                            k_threshold = expected_h;                                                                                    \
                        }                                                                                                                \
                    } else {                                                                                                             \
                        /* Hits don't justify growing; skip promotion. */                                                                \
                        continue;                                                                                                        \
                    }                                                                                                                    \
                }                                                                                                                        \
            }                                                                                                                            \
                                                                                                                                         \
            /* Step 2: Atomically increment node height.                                                                                 \
             *                                                                                                                           \
             * Use CAS to ensure no concurrent height change happened.                                                                   \
             * If it did, just skip -- another thread handled it. */                                                                     \
            {                                                                                                                            \
                size_t expected_h = node_height;                                                                                         \
                size_t new_h = node_height + 1;                                                                                          \
                if (!atomic_compare_exchange_strong_explicit(                                                                            \
                        &node->field.sle_height,                                                                                         \
                        &expected_h, new_h,                                                                                              \
                        memory_order_release, memory_order_relaxed)) {                                                                   \
                    /* Height was changed by another thread. Skip. */                                                                    \
                    continue;                                                                                                            \
                }                                                                                                                        \
                                                                                                                                         \
                /* Step 3: Link the new level into the skip chain.                                                                       \
                 *                                                                                                                       \
                 * We need to find a predecessor at level new_h and CAS                                                                  \
                 * ourselves into the chain:                                                                                             \
                 *   pred->levels[new_h].next: old_succ -> node                                                                          \
                 *   node->levels[new_h].next: (set to) old_succ                                                                         \
                 *                                                                                                                       \
                 * Use path[i].node as a hint for the predecessor since                                                                  \
                 * path[i+1] (if it exists) would be the predecessor at                                                                  \
                 * the next higher level from the locate traversal. */                                                                   \
                pred = NULL;                                                                                                             \
                                                                                                                                         \
                /* Try to use path information first. path[i+1] if valid                                                                 \
                 * might be the predecessor at a higher level. However,                                                                  \
                 * paths can be stale, so we fall back to backward scan. */                                                              \
                if (i + 1 <= len && path[i + 1].node != NULL) {                                                                          \
                    decl##_node_t *cand = path[i + 1].node;                                                                              \
                    size_t cand_h = atomic_load_explicit(                                                                                \
                        &cand->field.sle_height, memory_order_acquire);                                                                  \
                    if (cand_h >= new_h) {                                                                                               \
                        /* Validate: cand's next at new_h should come                                                                    \
                         * after node in sort order (or be tail). */                                                                     \
                        decl##_node_t *cand_next = atomic_load_explicit(                                                                 \
                            &cand->field.sle_levels[new_h].next,                                                                         \
                            memory_order_acquire);                                                                                       \
                        if (cand_next != NULL && !__SKIP_IS_MARKED(cand_next)) {                                                         \
                            int c = (cand_next == slist->slh_tail) ? 1                                                                   \
                                : __skip_compare_nodes_##decl(slist, cand_next, node, slist->slh_aux);                                   \
                            if (c >= 0) {                                                                                                \
                                pred = cand;                                                                                             \
                            }                                                                                                            \
                        }                                                                                                                \
                    }                                                                                                                    \
                }                                                                                                                        \
                                                                                                                                         \
                /* Fall back to backward scan if path hint didn't work. */                                                               \
                if (pred == NULL) {                                                                                                      \
                    pred = __skip_splay_find_pred_at_level_##decl(slist, node, new_h);                                                   \
                }                                                                                                                        \
                                                                                                                                         \
                /* If no predecessor found, try head. */                                                                                 \
                if (pred == NULL) {                                                                                                      \
                    size_t head_h = atomic_load_explicit(                                                                                \
                        &slist->slh_head->field.sle_height, memory_order_acquire);                                                       \
                    if (head_h >= new_h) {                                                                                               \
                        pred = slist->slh_head;                                                                                          \
                    }                                                                                                                    \
                }                                                                                                                        \
                                                                                                                                         \
                if (pred == NULL) {                                                                                                      \
                    /* Cannot link: revert the height increment.                                                                         \
                     * This is safe because no pointer references new_h yet. */                                                          \
                    size_t revert_exp = new_h;                                                                                           \
                    atomic_compare_exchange_strong_explicit(                                                                             \
                        &node->field.sle_height,                                                                                         \
                        &revert_exp, node_height,                                                                                        \
                        memory_order_release, memory_order_relaxed);                                                                     \
                    continue;                                                                                                            \
                }                                                                                                                        \
                                                                                                                                         \
                /* Read what pred currently points to at new_h. */                                                                       \
                succ = atomic_load_explicit(                                                                                             \
                    &pred->field.sle_levels[new_h].next, memory_order_acquire);                                                          \
                                                                                                                                         \
                if (__SKIP_IS_MARKED(succ)) {                                                                                            \
                    /* Pred is being deleted; revert height. */                                                                          \
                    size_t revert_exp = new_h;                                                                                           \
                    atomic_compare_exchange_strong_explicit(                                                                             \
                        &node->field.sle_height,                                                                                         \
                        &revert_exp, node_height,                                                                                        \
                        memory_order_release, memory_order_relaxed);                                                                     \
                    continue;                                                                                                            \
                }                                                                                                                        \
                                                                                                                                         \
                /* Set our new level's next to succ before linking in.                                                                   \
                 * Use relaxed: this is not yet visible to other threads                                                                 \
                 * (they'll see it only after the CAS on pred succeeds). */                                                              \
                atomic_store_explicit(                                                                                                   \
                    &node->field.sle_levels[new_h].next, succ,                                                                           \
                    memory_order_relaxed);                                                                                               \
                atomic_store_explicit(                                                                                                   \
                    &node->field.sle_levels[new_h].hits, 0,                                                                              \
                    memory_order_relaxed);                                                                                               \
                                                                                                                                         \
                /* CAS: pred->levels[new_h].next = succ -> node.                                                                         \
                 * Release ordering publishes our new level's next pointer. */                                                           \
                expected = succ;                                                                                                         \
                if (!atomic_compare_exchange_strong_explicit(                                                                            \
                        &pred->field.sle_levels[new_h].next,                                                                             \
                        &expected, node,                                                                                                 \
                        memory_order_release, memory_order_relaxed)) {                                                                   \
                    /* CAS failed: revert height. The node remains correct                                                               \
                     * at its old height; the stale next pointer at new_h                                                                \
                     * is harmless since no one links to it. */                                                                          \
                    size_t revert_exp = new_h;                                                                                           \
                    atomic_compare_exchange_strong_explicit(                                                                             \
                        &node->field.sle_height,                                                                                         \
                        &revert_exp, node_height,                                                                                        \
                        memory_order_release, memory_order_relaxed);                                                                     \
                }                                                                                                                        \
            }                                                                                                                            \
        }                                                                                                                                \
    }                                                                                                                                    \
                                                                                                                                         \
    static void __skip_rebalance_##decl(decl##_t *slist, size_t len, __skiplist_path_##decl##_t path[])                                  \
    {                                                                                                                                    \
        SKIPLIST_SPLAY_IMPL(decl, slist, len, path);                                                                                     \
    }                                                                                                                                    \
                                                                                                                                         \
    /**                                                                                                                                  \
     * -- __skip_locate_                                                                                                                 \
     *                                                                                                                                   \
     * Lock-free search: locates a node in the skiplist using Fraser's                                                                   \
     * algorithm.  Helps physically unlink marked (logically deleted)                                                                    \
     * nodes encountered during traversal.  Fills `path` with predecessors                                                               \
     * and successors, returning the path length and a match in path[0].                                                                 \
     */                                                                                                                                  \
    static size_t __skip_locate_##decl(decl##_t *slist, decl##_node_t *n, __skiplist_path_##decl##_t path[])                             \
    {                                                                                                                                    \
        size_t len = 0;                                                                                                                  \
        int cmp;                                                                                                                         \
        decl##_node_t *pred, *curr = NULL, *succ;                                                                                          \
                                                                                                                                         \
        if (slist == NULL || n == NULL)                                                                                                  \
            return 0;                                                                                                                    \
                                                                                                                                         \
    __skip_locate_retry_##decl:                                                                                                          \
        pred = slist->slh_head;                                                                                                          \
        len = 0;                                                                                                                         \
                                                                                                                                         \
        /* Traverse from the highest active level down to level 0.                                                                       \
           Head's sle_height is the number of active levels (1 means only                                                                \
           level 0 is active; search starts at level sle_height - 1). */                                                                 \
        for (size_t __lvl = atomic_load_explicit(                                                                                        \
                 &slist->slh_head->field.sle_height, memory_order_acquire);                                                              \
             __lvl > 0; __lvl--) {                                                                                                       \
            size_t i = __lvl - 1; /* current level index */                                                                              \
                                                                                                                                         \
            /* Read pred's next pointer at this level. */                                                                                \
            curr = atomic_load_explicit(                                                                                                 \
                &pred->field.sle_levels[i].next, memory_order_acquire);                                                                  \
                                                                                                                                         \
            for (;;) {                                                                                                                   \
                /* Read curr's next pointer.  Tail's next is always NULL. */                                                             \
                if (curr == slist->slh_tail) {                                                                                           \
                    succ = NULL;                                                                                                         \
                } else {                                                                                                                 \
                    succ = atomic_load_explicit(                                                                                          \
                        &curr->field.sle_levels[i].next, memory_order_acquire);                                                          \
                }                                                                                                                        \
                                                                                                                                         \
                /* Help unlink any marked (logically deleted) nodes. */                                                                  \
                while (succ != NULL && __SKIP_IS_MARKED(succ)) {                                                                         \
                    decl##_node_t *unmarked_succ = __SKIP_UNMARK(succ);                                                                  \
                    decl##_node_t *expected = curr;                                                                                      \
                    if (!atomic_compare_exchange_strong_explicit(                                                                         \
                            &pred->field.sle_levels[i].next,                                                                             \
                            &expected, unmarked_succ,                                                                                    \
                            memory_order_release, memory_order_relaxed)) {                                                               \
                        /* CAS failed: restart from the top. */                                                                          \
                        goto __skip_locate_retry_##decl;                                                                                 \
                    }                                                                                                                    \
                    curr = unmarked_succ;                                                                                                \
                    if (curr == slist->slh_tail) {                                                                                       \
                        succ = NULL;                                                                                                     \
                    } else {                                                                                                             \
                        succ = atomic_load_explicit(                                                                                      \
                            &curr->field.sle_levels[i].next, memory_order_acquire);                                                      \
                    }                                                                                                                    \
                }                                                                                                                        \
                                                                                                                                         \
                /* Both curr and succ are unmarked.  Compare curr against key. */                                                        \
                cmp = __skip_compare_nodes_##decl(slist, curr, n, slist->slh_aux);                                                       \
                if (cmp < 0) {                                                                                                           \
                    pred = curr;                                                                                                         \
                    curr = (succ == NULL) ? slist->slh_tail : succ;                                                                      \
                } else {                                                                                                                 \
                    break;                                                                                                               \
                }                                                                                                                        \
            }                                                                                                                            \
                                                                                                                                         \
            /* Record predecessor and successor at level i. */                                                                           \
            path[i + 1].node = pred;                                                                                                     \
            path[i + 1].succ = curr;                                                                                                     \
            path[i + 1].pu = 0;                                                                                                          \
            len++;                                                                                                                       \
        }                                                                                                                                \
                                                                                                                                         \
        /* Check for an exact match at level 0. */                                                                                       \
        if (curr != slist->slh_tail &&                                                                                                   \
            __skip_compare_nodes_##decl(slist, curr, n, slist->slh_aux) == 0) {                                                          \
            path[0].node = curr;                                                                                                         \
            /* Hit counting with relaxed atomics. */                                                                                     \
            atomic_fetch_add_explicit(                                                                                                   \
                &curr->field.sle_levels[0].hits, 1, memory_order_relaxed);                                                               \
            atomic_fetch_add_explicit(                                                                                                   \
                &slist->slh_head->field.sle_levels[                                                                                      \
                    atomic_load_explicit(&slist->slh_head->field.sle_height,                                                             \
                                         memory_order_relaxed)].hits,                                                                    \
                1, memory_order_relaxed);                                                                                                \
            __skip_rebalance_##decl(slist, len, path);                                                                                   \
        } else {                                                                                                                         \
            path[0].node = NULL;                                                                                                         \
        }                                                                                                                                \
                                                                                                                                         \
        return len;                                                                                                                      \
    }                                                                                                                                    \
                                                                                                                                         \
    /**                                                                                                                                  \
     * -- __skip_insert_                                                                                                                 \
     *                                                                                                                                   \
     * Lock-free insert: atomically links `new` into the skiplist at                                                                     \
     * all appropriate levels.  Level-0 CAS is the linearization point.                                                                  \
     * When `flags` is 0, duplicates are rejected; when non-zero, they                                                                   \
     * are allowed.                                                                                                                      \
     */                                                                                                                                  \
    static int __skip_insert_##decl(decl##_t *slist, decl##_node_t *new, int flags)                                                      \
    {                                                                                                                                    \
        __skiplist_path_##decl##_t apath[SKIPLIST_MAX_HEIGHT + 1];                                                                       \
        int rc = 0;                                                                                                                      \
        size_t i, len, current_height, new_height, old_head_height;                                                                      \
        __skiplist_path_##decl##_t *path = apath;                                                                                        \
                                                                                                                                         \
        if (slist == NULL || new == NULL)                                                                                                \
            return ENOENT;                                                                                                               \
                                                                                                                                         \
    __skip_insert_retry_##decl:                                                                                                          \
        memset(path, 0, sizeof(__skiplist_path_##decl##_t) * (SKIPLIST_MAX_HEIGHT + 1));                                                  \
                                                                                                                                         \
        /* Phase 1: Find the insertion point. */                                                                                         \
        len = __skip_locate_##decl(slist, new, path);                                                                                    \
        if (len == 0)                                                                                                                    \
            return ENOENT;                                                                                                               \
                                                                                                                                         \
        /* Reject duplicates unless flags is set. */                                                                                     \
        if (path[0].node != NULL && flags == 0) {                                                                                        \
            return -1;                                                                                                                   \
        }                                                                                                                                \
                                                                                                                                         \
        /* Phase 2: Determine the new node's height via coin toss. */                                                                    \
        old_head_height = atomic_load_explicit(                                                                                          \
            &slist->slh_head->field.sle_height, memory_order_acquire);                                                                   \
        current_height = old_head_height - 1;                                                                                            \
                                                                                                                                         \
        {                                                                                                                                \
            size_t toss_max = current_height + 1;                                                                                        \
            if (toss_max > SKIPLIST_MAX_HEIGHT - 2)                                                                                      \
                toss_max = SKIPLIST_MAX_HEIGHT - 2;                                                                                      \
            new_height = __skip_toss_##decl(slist, toss_max);                                                                            \
        }                                                                                                                                \
        atomic_store_explicit(&new->field.sle_height, new_height, memory_order_relaxed);                                                 \
                                                                                                                                         \
        /* Phase 3: Grow the head height if needed (CAS loop). */                                                                        \
        if (new_height > current_height) {                                                                                               \
            size_t desired_head_h = new_height + 1;                                                                                      \
            size_t cur_h = atomic_load_explicit(                                                                                         \
                &slist->slh_head->field.sle_height, memory_order_acquire);                                                               \
            while (cur_h < desired_head_h) {                                                                                             \
                if (atomic_compare_exchange_weak_explicit(                                                                                \
                        &slist->slh_head->field.sle_height,                                                                              \
                        &cur_h, desired_head_h,                                                                                          \
                        memory_order_release, memory_order_acquire)) {                                                                   \
                    atomic_store_explicit(                                                                                                \
                        &slist->slh_tail->field.sle_height,                                                                              \
                        desired_head_h, memory_order_release);                                                                           \
                    break;                                                                                                               \
                }                                                                                                                        \
            }                                                                                                                            \
        }                                                                                                                                \
                                                                                                                                         \
        /* Fill path entries for levels above what locate saw. */                                                                        \
        for (i = old_head_height; i <= new_height; i++) {                                                                                \
            path[i + 1].node = slist->slh_head;                                                                                          \
            path[i + 1].succ = slist->slh_tail;                                                                                          \
        }                                                                                                                                \
                                                                                                                                         \
        /* Phase 4: Pre-fill the new node's next pointers. */                                                                            \
        for (i = 0; i <= new_height; i++) {                                                                                              \
            atomic_store_explicit(                                                                                                       \
                &new->field.sle_levels[i].next,                                                                                          \
                path[i + 1].succ, memory_order_relaxed);                                                                                 \
        }                                                                                                                                \
                                                                                                                                         \
        /* Phase 5: CAS at level 0 -- LINEARIZATION POINT. */                                                                           \
        {                                                                                                                                \
            decl##_node_t *expected = path[1].succ;                                                                                      \
            if (!atomic_compare_exchange_strong_explicit(                                                                                 \
                    &path[1].node->field.sle_levels[0].next,                                                                             \
                    &expected, new,                                                                                                      \
                    memory_order_release, memory_order_relaxed)) {                                                                       \
                goto __skip_insert_retry_##decl;                                                                                         \
            }                                                                                                                            \
        }                                                                                                                                \
                                                                                                                                         \
        /* Phase 6: Link at higher levels (1 .. new_height). */                                                                          \
        for (i = 1; i <= new_height; i++) {                                                                                              \
            for (;;) {                                                                                                                   \
                decl##_node_t *pred_at_i = path[i + 1].node;                                                                             \
                decl##_node_t *succ_at_i = path[i + 1].succ;                                                                             \
                                                                                                                                         \
                {                                                                                                                        \
                    decl##_node_t *cur_next = atomic_load_explicit(                                                                       \
                        &new->field.sle_levels[i].next, memory_order_acquire);                                                           \
                    if (__SKIP_IS_MARKED(cur_next)) {                                                                                    \
                        goto __skip_insert_done_##decl;                                                                                  \
                    }                                                                                                                    \
                    if (cur_next != succ_at_i) {                                                                                         \
                        atomic_store_explicit(                                                                                           \
                            &new->field.sle_levels[i].next,                                                                              \
                            succ_at_i, memory_order_relaxed);                                                                            \
                    }                                                                                                                    \
                }                                                                                                                        \
                                                                                                                                         \
                {                                                                                                                        \
                    decl##_node_t *expected = succ_at_i;                                                                                  \
                    if (atomic_compare_exchange_strong_explicit(                                                                          \
                            &pred_at_i->field.sle_levels[i].next,                                                                        \
                            &expected, new,                                                                                              \
                            memory_order_release, memory_order_relaxed)) {                                                               \
                        break;                                                                                                           \
                    }                                                                                                                    \
                }                                                                                                                        \
                                                                                                                                         \
                /* CAS failed; re-find to get fresh preds/succs. */                                                                      \
                memset(path, 0, sizeof(__skiplist_path_##decl##_t) * (SKIPLIST_MAX_HEIGHT + 1));                                          \
                __skip_locate_##decl(slist, new, path);                                                                                  \
                                                                                                                                         \
                {                                                                                                                        \
                    decl##_node_t *lvl0_next = atomic_load_explicit(                                                                      \
                        &new->field.sle_levels[0].next, memory_order_acquire);                                                           \
                    if (__SKIP_IS_MARKED(lvl0_next)) {                                                                                   \
                        goto __skip_insert_done_##decl;                                                                                  \
                    }                                                                                                                    \
                }                                                                                                                        \
            }                                                                                                                            \
        }                                                                                                                                \
                                                                                                                                         \
    __skip_insert_done_##decl:                                                                                                           \
        /* Phase 7: Set backward pointer (advisory, best-effort). */                                                                     \
        atomic_store_explicit(                                                                                                           \
            &new->field.sle_prev, path[1].node, memory_order_relaxed);                                                                   \
        {                                                                                                                                \
            decl##_node_t *succ_at_0 = atomic_load_explicit(                                                                             \
                &new->field.sle_levels[0].next, memory_order_acquire);                                                                   \
            if (!__SKIP_IS_MARKED(succ_at_0) && succ_at_0 != slist->slh_tail) {                                                          \
                decl##_node_t *old_prev = path[1].node;                                                                                  \
                atomic_compare_exchange_strong_explicit(                                                                                  \
                    &succ_at_0->field.sle_prev,                                                                                          \
                    &old_prev, new,                                                                                                      \
                    memory_order_relaxed, memory_order_relaxed);                                                                         \
            } else if (!__SKIP_IS_MARKED(succ_at_0) && succ_at_0 == slist->slh_tail) {                                                   \
                decl##_node_t *old_prev = path[1].node;                                                                                  \
                atomic_compare_exchange_strong_explicit(                                                                                  \
                    &slist->slh_tail->field.sle_prev,                                                                                    \
                    &old_prev, new,                                                                                                      \
                    memory_order_relaxed, memory_order_relaxed);                                                                         \
            }                                                                                                                            \
        }                                                                                                                                \
                                                                                                                                         \
        /* Record era for snapshot support. */                                                                                           \
        if (slist->slh_snap.pres_era > 0) {                                                                                              \
            new->field.sle_era = slist->slh_snap.cur_era++;                                                                              \
        }                                                                                                                                \
                                                                                                                                         \
        /* Initial hit count for splay rebalancing. */                                                                                   \
        atomic_store_explicit(                                                                                                           \
            &new->field.sle_levels[new_height].hits, 1, memory_order_relaxed);                                                           \
                                                                                                                                         \
        /* Increment list length. */                                                                                                     \
        atomic_fetch_add_explicit(                                                                                                       \
            &slist->slh_length, 1, memory_order_relaxed);                                                                                \
                                                                                                                                         \
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
     * Find a node that matches the node `n`.  This differs from the locate()                                                            \
     * API in that it does not return the path to the node, only the match.                                                              \
     */                                                                                                                                  \
    decl##_node_t *prefix##skip_position_eq_##decl(decl##_t *slist, decl##_node_t *query)                                                \
    {                                                                                                                                    \
        __skiplist_path_##decl##_t apath[SKIPLIST_MAX_HEIGHT + 1];                                                                \
        decl##_node_t *node = NULL;                                                                                                      \
        __skiplist_path_##decl##_t *path = apath;                                                                                        \
                                                                                                                                         \
        memset(path, 0, sizeof(__skiplist_path_##decl##_t) * (SKIPLIST_MAX_HEIGHT + 1));                                                   \
                                                                                                                                         \
        /* Find a `path` to `new` in the list and a match (`path[0]`) if it exists. */                                                   \
        __skip_locate_##decl(slist, query, path);                                                                                        \
        node = path[0].node;                                                                                                             \
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
        __skiplist_path_##decl##_t apath[SKIPLIST_MAX_HEIGHT + 1];                                                                \
        int cmp;                                                                                                                         \
        decl##_node_t *node;                                                                                                             \
        __skiplist_path_##decl##_t *path = apath;                                                                                        \
                                                                                                                                         \
        memset(path, 0, sizeof(__skiplist_path_##decl##_t) * (SKIPLIST_MAX_HEIGHT + 1));                                                   \
                                                                                                                                         \
        /* Find a `path` to `new` in the list and a match (`path[0]`) if it exists. */                                                   \
        __skip_locate_##decl(slist, query, path);                                                                                        \
        node = path[1].node;                                                                                                             \
        do {                                                                                                                             \
            node = atomic_load_explicit(&node->field.sle_levels[0].next, memory_order_acquire);                                          \
            cmp = __skip_compare_nodes_##decl(slist, node, query, slist->slh_aux);                                                       \
        } while (cmp < 0);                                                                                                               \
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
        __skiplist_path_##decl##_t apath[SKIPLIST_MAX_HEIGHT + 1];                                                                       \
        int cmp;                                                                                                                         \
        decl##_node_t *node;                                                                                                             \
        __skiplist_path_##decl##_t *path = apath;                                                                                        \
                                                                                                                                         \
        memset(path, 0, sizeof(__skiplist_path_##decl##_t) * (SKIPLIST_MAX_HEIGHT + 1));                                                  \
                                                                                                                                         \
        /* Find a `path` to `new` in the list and a match (`path[0]`) if it exists. */                                                   \
        __skip_locate_##decl(slist, query, path);                                                                                        \
        node = path[1].node;                                                                                                             \
        if (node == slist->slh_tail)                                                                                                     \
            goto done;                                                                                                                   \
        do {                                                                                                                             \
            node = atomic_load_explicit(&node->field.sle_levels[0].next, memory_order_acquire);                                          \
            cmp = __skip_compare_nodes_##decl(slist, node, query, slist->slh_aux);                                                       \
        } while (cmp <= 0 && node != slist->slh_tail);                                                                                   \
    done:;                                                                                                                               \
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
        __skiplist_path_##decl##_t apath[SKIPLIST_MAX_HEIGHT + 1];                                                                \
        decl##_node_t *node;                                                                                                             \
        __skiplist_path_##decl##_t *path = apath;                                                                                        \
                                                                                                                                         \
        memset(path, 0, sizeof(__skiplist_path_##decl##_t) * (SKIPLIST_MAX_HEIGHT + 1));                                                   \
                                                                                                                                         \
        /* Find a `path` to `new` in the list and a match (`path[0]`) if it exists. */                                                   \
        __skip_locate_##decl(slist, query, path);                                                                                        \
        node = path[0].node;                                                                                                             \
        if (node)                                                                                                                        \
            goto done;                                                                                                                   \
        node = path[1].node;                                                                                                             \
    done:;                                                                                                                               \
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
        __skiplist_path_##decl##_t apath[SKIPLIST_MAX_HEIGHT + 1];                                                                \
        decl##_node_t *node;                                                                                                             \
        __skiplist_path_##decl##_t *path = apath;                                                                                        \
                                                                                                                                         \
        memset(path, 0, sizeof(__skiplist_path_##decl##_t) * (SKIPLIST_MAX_HEIGHT + 1));                                                   \
                                                                                                                                         \
        /* Find a `path` to `new` in the list and a match (`path[0]`) if it exists. */                                                   \
        __skip_locate_##decl(slist, query, path);                                                                                        \
        node = path[1].node;                                                                                                             \
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
        __skiplist_path_##decl##_t apath[SKIPLIST_MAX_HEIGHT + 1];                                                                \
        int rc = 0, np;                                                                                                                  \
        decl##_node_t *node;                                                                                                             \
        __skiplist_path_##decl##_t *path = apath;                                                                                        \
                                                                                                                                         \
        if (slist == NULL)                                                                                                               \
            return -1;                                                                                                                   \
                                                                                                                                         \
        memset(path, 0, sizeof(__skiplist_path_##decl##_t) * (SKIPLIST_MAX_HEIGHT + 1));                                                   \
                                                                                                                                         \
        __skip_locate_##decl(slist, query, path);                                                                                        \
        node = path[0].node;                                                                                                             \
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
     * Lock-free delete: logically removes a node by marking its next                                                                    \
     * pointers (top-down), then physically unlinks by re-traversing.                                                                    \
     * The level-0 mark is the linearization point.                                                                                      \
     */                                                                                                                                  \
    int prefix##skip_remove_node_##decl(decl##_t *slist, decl##_node_t *query)                                                           \
    {                                                                                                                                    \
        __skiplist_path_##decl##_t apath[SKIPLIST_MAX_HEIGHT + 1];                                                                       \
        int np = 0;                                                                                                                      \
        size_t height;                                                                                                                   \
        decl##_node_t *node, *succ, *expected;                                                                                           \
        __skiplist_path_##decl##_t *path = apath;                                                                                        \
        int ok;                                                                                                                          \
                                                                                                                                         \
        if (slist == NULL || query == NULL)                                                                                              \
            return -1;                                                                                                                   \
                                                                                                                                         \
        memset(path, 0, sizeof(__skiplist_path_##decl##_t) * (SKIPLIST_MAX_HEIGHT + 1));                                                  \
                                                                                                                                         \
        /* Locate the node to be removed. */                                                                                             \
        __skip_locate_##decl(slist, query, path);                                                                                        \
        node = path[0].node;                                                                                                             \
        if (node == NULL) {                                                                                                              \
            return 0;                                                                                                                    \
        }                                                                                                                                \
                                                                                                                                         \
        /* Snapshot preservation (single-threaded feature). */                                                                           \
        if (slist->slh_snap.pres_era > 0) {                                                                                              \
            np = slist->slh_fns.snapshot_preserve_node(slist, node, NULL);                                                               \
            if (np > 0)                                                                                                                  \
                return np;                                                                                                               \
            slist->slh_snap.cur_era++;                                                                                                   \
        }                                                                                                                                \
                                                                                                                                         \
        height = atomic_load_explicit(                                                                                                   \
            &node->field.sle_height, memory_order_acquire);                                                                              \
                                                                                                                                         \
        /* Phase 1: Mark next pointers from top level down to level 1. */                                                                \
        {                                                                                                                                \
            size_t lvl = height;                                                                                                         \
            while (lvl >= 1) {                                                                                                           \
                succ = atomic_load_explicit(                                                                                              \
                    &node->field.sle_levels[lvl].next, memory_order_acquire);                                                            \
                while (!__SKIP_IS_MARKED(succ)) {                                                                                        \
                    expected = succ;                                                                                                     \
                    atomic_compare_exchange_strong_explicit(                                                                              \
                        &node->field.sle_levels[lvl].next,                                                                               \
                        &expected, __SKIP_MARK(succ),                                                                                    \
                        memory_order_release, memory_order_relaxed);                                                                     \
                    succ = atomic_load_explicit(                                                                                          \
                        &node->field.sle_levels[lvl].next, memory_order_acquire);                                                        \
                }                                                                                                                        \
                lvl--;                                                                                                                   \
            }                                                                                                                            \
        }                                                                                                                                \
                                                                                                                                         \
        /* Phase 2: Mark level 0 -- LINEARIZATION POINT. */                                                                              \
        succ = atomic_load_explicit(                                                                                                     \
            &node->field.sle_levels[0].next, memory_order_acquire);                                                                      \
        for (;;) {                                                                                                                       \
            if (__SKIP_IS_MARKED(succ)) {                                                                                                \
                /* Another thread already marked level 0. */                                                                             \
                return 0;                                                                                                                \
            }                                                                                                                            \
            expected = succ;                                                                                                             \
            ok = atomic_compare_exchange_strong_explicit(                                                                                 \
                &node->field.sle_levels[0].next,                                                                                         \
                &expected, __SKIP_MARK(succ),                                                                                            \
                memory_order_acq_rel, memory_order_acquire);                                                                             \
            if (ok) {                                                                                                                    \
                break;                                                                                                                   \
            }                                                                                                                            \
            succ = expected;                                                                                                             \
        }                                                                                                                                \
                                                                                                                                         \
        /* Phase 3: Physical unlinking via find. */                                                                                      \
        memset(path, 0, sizeof(__skiplist_path_##decl##_t) * (SKIPLIST_MAX_HEIGHT + 1));                                                  \
        __skip_locate_##decl(slist, query, path);                                                                                        \
                                                                                                                                         \
        /* Update backward pointer hint (best-effort). */                                                                                \
        {                                                                                                                                \
            decl##_node_t *unmarked_succ = __SKIP_UNMARK(                                                                                \
                atomic_load_explicit(                                                                                                    \
                    &node->field.sle_levels[0].next, memory_order_acquire));                                                             \
            if (unmarked_succ != slist->slh_tail && unmarked_succ != NULL) {                                                             \
                decl##_node_t *exp_prev = node;                                                                                          \
                decl##_node_t *new_prev = atomic_load_explicit(                                                                          \
                    &node->field.sle_prev, memory_order_relaxed);                                                                        \
                atomic_compare_exchange_strong_explicit(                                                                                  \
                    &unmarked_succ->field.sle_prev,                                                                                      \
                    &exp_prev, new_prev,                                                                                                 \
                    memory_order_relaxed, memory_order_relaxed);                                                                         \
            }                                                                                                                            \
            if (unmarked_succ == slist->slh_tail) {                                                                                      \
                decl##_node_t *exp_prev = node;                                                                                          \
                decl##_node_t *new_prev = atomic_load_explicit(                                                                          \
                    &node->field.sle_prev, memory_order_relaxed);                                                                        \
                atomic_compare_exchange_strong_explicit(                                                                                  \
                    &slist->slh_tail->field.sle_prev,                                                                                    \
                    &exp_prev, new_prev,                                                                                                 \
                    memory_order_relaxed, memory_order_relaxed);                                                                         \
            }                                                                                                                            \
        }                                                                                                                                \
                                                                                                                                         \
        /* Decrement list length. */                                                                                                     \
        atomic_fetch_sub_explicit(                                                                                                       \
            &slist->slh_length, 1, memory_order_relaxed);                                                                                \
                                                                                                                                         \
        /* Free the node.  When EBR is attached, defer freeing via the                                                                    \
           retire list; otherwise free immediately (single-threaded). */                                                                 \
        if (slist->slh_ebr != NULL && slist->slh_ebr_retire != NULL) {                                                                   \
            slist->slh_ebr_retire(slist->slh_ebr, slist, node);                                                                          \
        } else {                                                                                                                         \
            slist->slh_fns.free_entry(node);                                                                                             \
            free(node);                                                                                                                  \
        }                                                                                                                      \
                                                                                                                                         \
        __skip_adjust_hit_counts_##decl(slist);                                                                                          \
                                                                                                                                         \
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

/*
 * Epoch-Based Reclamation (EBR) for safe memory reclamation in lock-free
 * skip lists.  When multiple threads perform concurrent operations, deleted
 * nodes cannot be freed immediately because other threads may still hold
 * references.  EBR defers freeing until it is safe: a global epoch advances
 * when all active threads have observed the current epoch, and nodes retired
 * two epochs ago can then be reclaimed.
 *
 * Usage:
 *   1. Declare EBR with SKIPLIST_DECL_EBR(decl, prefix) after SKIPLIST_DECL.
 *   2. Allocate and init an EBR state: __skip_ebr_##decl##_t ebr;
 *      prefix##skip_ebr_init_##decl(&ebr);
 *   3. Attach it to a list: prefix##skip_ebr_attach_##decl(&slist, &ebr);
 *   4. Each thread registers: int tid = prefix##skip_ebr_register_##decl(&ebr);
 *   5. Before accessing the list: prefix##skip_ebr_pin_##decl(&ebr, tid);
 *   6. After done: prefix##skip_ebr_unpin_##decl(&ebr, tid);
 *   7. Deletions automatically retire nodes through the EBR retire lists.
 */

#ifndef SKIPLIST_EBR_MAX_THREADS
#define SKIPLIST_EBR_MAX_THREADS 128
#endif

#define SKIPLIST_DECL_EBR(decl, prefix)                                                                                                  \
                                                                                                                                         \
    /* Per-thread EBR state. */                                                                                                          \
    typedef struct __skip_ebr_thread_##decl {                                                                                            \
        _Atomic(uint64_t) local_epoch;                                                                                                   \
        _Atomic(int) active;                                                                                                             \
    } __skip_ebr_thread_##decl##_t;                                                                                                      \
                                                                                                                                         \
    /* A retired node waiting to be freed. */                                                                                            \
    typedef struct __skip_ebr_retired_##decl {                                                                                           \
        decl##_node_t *node;                                                                                                             \
        decl##_t *slist;                                                                                                                 \
        struct __skip_ebr_retired_##decl *next;                                                                                          \
    } __skip_ebr_retired_##decl##_t;                                                                                                     \
                                                                                                                                         \
    /* The EBR state. */                                                                                                                 \
    typedef struct __skip_ebr_##decl {                                                                                                   \
        _Atomic(uint64_t) global_epoch;                                                                                                  \
        __skip_ebr_thread_##decl##_t threads[SKIPLIST_EBR_MAX_THREADS];                                                                  \
        _Atomic(int) thread_count;                                                                                                       \
        /* Three retire lists, one per epoch bucket (epoch % 3). */                                                                      \
        __skip_ebr_retired_##decl##_t *retire_lists[3];                                                                                  \
        _Atomic(int) retire_locks[3];                                                                                                    \
    } __skip_ebr_##decl##_t;                                                                                                             \
                                                                                                                                         \
    /* Spinlock helpers for retire list access. */                                                                                       \
    static void __skip_ebr_lock_##decl(_Atomic(int) *lock)                                                                               \
    {                                                                                                                                    \
        while (atomic_exchange_explicit(lock, 1, memory_order_acquire) != 0) {                                                           \
            /* spin */                                                                                                                   \
        }                                                                                                                                \
    }                                                                                                                                    \
                                                                                                                                         \
    static void __skip_ebr_unlock_##decl(_Atomic(int) *lock)                                                                             \
    {                                                                                                                                    \
        atomic_store_explicit(lock, 0, memory_order_release);                                                                            \
    }                                                                                                                                    \
                                                                                                                                         \
    /* Forward declaration for try_advance. */                                                                                           \
    static void __skip_ebr_try_advance_##decl(                                                                                           \
        __skip_ebr_##decl##_t *ebr);                                                                                                     \
                                                                                                                                         \
    /**                                                                                                                                  \
     * -- skip_ebr_init_                                                                                                                 \
     *                                                                                                                                   \
     * Initialize EBR state.  Must be called before any other EBR operation.                                                             \
     */                                                                                                                                  \
    void prefix##skip_ebr_init_##decl(__skip_ebr_##decl##_t *ebr)                                                                        \
    {                                                                                                                                    \
        memset(ebr, 0, sizeof(*ebr));                                                                                                    \
        atomic_store_explicit(&ebr->global_epoch, 1, memory_order_relaxed);                                                              \
    }                                                                                                                                    \
                                                                                                                                         \
    /**                                                                                                                                  \
     * -- skip_ebr_register_                                                                                                             \
     *                                                                                                                                   \
     * Register a thread for EBR participation.  Returns a thread ID                                                                     \
     * (0-based).  Must be called once per thread before pin/unpin.                                                                      \
     * Returns -1 if the maximum number of threads has been reached.                                                                     \
     */                                                                                                                                  \
    int prefix##skip_ebr_register_##decl(__skip_ebr_##decl##_t *ebr)                                                                     \
    {                                                                                                                                    \
        int tid = atomic_fetch_add_explicit(                                                                                             \
            &ebr->thread_count, 1, memory_order_relaxed);                                                                                \
        if (tid >= SKIPLIST_EBR_MAX_THREADS) {                                                                                           \
            atomic_fetch_sub_explicit(                                                                                                   \
                &ebr->thread_count, 1, memory_order_relaxed);                                                                            \
            return -1;                                                                                                                   \
        }                                                                                                                                \
        atomic_store_explicit(                                                                                                           \
            &ebr->threads[tid].local_epoch, 0, memory_order_relaxed);                                                                    \
        atomic_store_explicit(                                                                                                           \
            &ebr->threads[tid].active, 0, memory_order_relaxed);                                                                         \
        return tid;                                                                                                                      \
    }                                                                                                                                    \
                                                                                                                                         \
    /**                                                                                                                                  \
     * -- skip_ebr_pin_                                                                                                                  \
     *                                                                                                                                   \
     * Enter a critical section.  The calling thread announces that it                                                                   \
     * is reading the data structure and nodes must not be freed until                                                                    \
     * it unpins.                                                                                                                        \
     */                                                                                                                                  \
    void prefix##skip_ebr_pin_##decl(__skip_ebr_##decl##_t *ebr, int tid)                                                                \
    {                                                                                                                                    \
        uint64_t ge = atomic_load_explicit(                                                                                              \
            &ebr->global_epoch, memory_order_acquire);                                                                                   \
        atomic_store_explicit(                                                                                                           \
            &ebr->threads[tid].local_epoch, ge, memory_order_relaxed);                                                                   \
        atomic_store_explicit(                                                                                                           \
            &ebr->threads[tid].active, 1, memory_order_release);                                                                         \
        /* Re-read global epoch after publishing active, ensuring we                                                                     \
           observe any epoch advance that happened concurrently. */                                                                      \
        ge = atomic_load_explicit(                                                                                                       \
            &ebr->global_epoch, memory_order_acquire);                                                                                   \
        atomic_store_explicit(                                                                                                           \
            &ebr->threads[tid].local_epoch, ge, memory_order_release);                                                                   \
    }                                                                                                                                    \
                                                                                                                                         \
    /**                                                                                                                                  \
     * -- skip_ebr_unpin_                                                                                                                \
     *                                                                                                                                   \
     * Exit a critical section.  The calling thread is no longer reading                                                                 \
     * the data structure.                                                                                                               \
     */                                                                                                                                  \
    void prefix##skip_ebr_unpin_##decl(__skip_ebr_##decl##_t *ebr, int tid)                                                              \
    {                                                                                                                                    \
        atomic_store_explicit(                                                                                                           \
            &ebr->threads[tid].active, 0, memory_order_release);                                                                         \
    }                                                                                                                                    \
                                                                                                                                         \
    /**                                                                                                                                  \
     * -- skip_ebr_retire_                                                                                                               \
     *                                                                                                                                   \
     * Defer freeing a node.  The node is placed on a retire list tagged                                                                 \
     * with the current epoch.  After all active threads have advanced                                                                   \
     * past this epoch, the node will be reclaimed.                                                                                      \
     */                                                                                                                                  \
    void prefix##skip_ebr_retire_##decl(                                                                                                 \
        __skip_ebr_##decl##_t *ebr, decl##_t *slist, decl##_node_t *node)                                                                \
    {                                                                                                                                    \
        uint64_t epoch = atomic_load_explicit(                                                                                           \
            &ebr->global_epoch, memory_order_acquire);                                                                                   \
        int bucket = (int)(epoch % 3);                                                                                                   \
                                                                                                                                         \
        __skip_ebr_retired_##decl##_t *entry =                                                                                           \
            (__skip_ebr_retired_##decl##_t *)malloc(                                                                                     \
                sizeof(__skip_ebr_retired_##decl##_t));                                                                                  \
        if (entry == NULL)                                                                                                               \
            return; /* best-effort; leak rather than crash */                                                                            \
        entry->node = node;                                                                                                              \
        entry->slist = slist;                                                                                                            \
                                                                                                                                         \
        __skip_ebr_lock_##decl(&ebr->retire_locks[bucket]);                                                                              \
        entry->next = ebr->retire_lists[bucket];                                                                                         \
        ebr->retire_lists[bucket] = entry;                                                                                               \
        __skip_ebr_unlock_##decl(&ebr->retire_locks[bucket]);                                                                            \
                                                                                                                                         \
        /* Attempt to advance the epoch and reclaim old nodes. */                                                                        \
        __skip_ebr_try_advance_##decl(ebr);                                                                                              \
    }                                                                                                                                    \
                                                                                                                                         \
    /**                                                                                                                                  \
     * -- __skip_ebr_try_advance_                                                                                                        \
     *                                                                                                                                   \
     * Check whether all active threads have observed the current global                                                                 \
     * epoch.  If so, advance the epoch and free all nodes retired two                                                                   \
     * epochs ago.                                                                                                                       \
     */                                                                                                                                  \
    static void __skip_ebr_try_advance_##decl(                                                                                           \
        __skip_ebr_##decl##_t *ebr)                                                                                                      \
    {                                                                                                                                    \
        uint64_t cur_epoch = atomic_load_explicit(                                                                                       \
            &ebr->global_epoch, memory_order_acquire);                                                                                   \
        int tc = atomic_load_explicit(                                                                                                   \
            &ebr->thread_count, memory_order_acquire);                                                                                   \
                                                                                                                                         \
        /* Check: every active thread must have local_epoch >= cur_epoch. */                                                             \
        for (int i = 0; i < tc; i++) {                                                                                                   \
            if (atomic_load_explicit(                                                                                                    \
                    &ebr->threads[i].active, memory_order_acquire)) {                                                                    \
                uint64_t le = atomic_load_explicit(                                                                                      \
                    &ebr->threads[i].local_epoch, memory_order_acquire);                                                                 \
                if (le < cur_epoch)                                                                                                      \
                    return; /* at least one thread hasn't caught up */                                                                   \
            }                                                                                                                            \
        }                                                                                                                                \
                                                                                                                                         \
        /* All active threads are up to date; try to bump the epoch. */                                                                  \
        uint64_t new_epoch = cur_epoch + 1;                                                                                              \
        if (!atomic_compare_exchange_strong_explicit(                                                                                    \
                &ebr->global_epoch, &cur_epoch, new_epoch,                                                                               \
                memory_order_acq_rel, memory_order_relaxed))                                                                             \
            return; /* another thread advanced it first */                                                                               \
                                                                                                                                         \
        /* Reclaim the bucket that is now 2 epochs behind.                                                                               \
           new_epoch - 2 is the epoch whose retire list is safe to free                                                                  \
           because all threads have since observed at least cur_epoch. */                                                                \
        if (new_epoch < 2)                                                                                                               \
            return; /* not enough epochs have passed yet */                                                                              \
        int old_bucket = (int)((new_epoch - 2) % 3);                                                                                     \
                                                                                                                                         \
        __skip_ebr_lock_##decl(&ebr->retire_locks[old_bucket]);                                                                          \
        __skip_ebr_retired_##decl##_t *list = ebr->retire_lists[old_bucket];                                                             \
        ebr->retire_lists[old_bucket] = NULL;                                                                                            \
        __skip_ebr_unlock_##decl(&ebr->retire_locks[old_bucket]);                                                                        \
                                                                                                                                         \
        while (list != NULL) {                                                                                                           \
            __skip_ebr_retired_##decl##_t *cur = list;                                                                                   \
            list = cur->next;                                                                                                            \
            cur->slist->slh_fns.free_entry(cur->node);                                                                                   \
            free(cur->node);                                                                                                             \
            free(cur);                                                                                                                   \
        }                                                                                                                                \
    }                                                                                                                                    \
                                                                                                                                         \
    /**                                                                                                                                  \
     * -- __skip_ebr_retire_cb_                                                                                                          \
     *                                                                                                                                   \
     * Type-erased callback that bridges the void* function pointer                                                                      \
     * stored in slh_ebr_retire to the typed retire function.                                                                            \
     */                                                                                                                                  \
    static void __skip_ebr_retire_cb_##decl(                                                                                             \
        void *ebr_opaque, decl##_t *slist, decl##_node_t *node)                                                                          \
    {                                                                                                                                    \
        prefix##skip_ebr_retire_##decl(                                                                                                  \
            (__skip_ebr_##decl##_t *)ebr_opaque, slist, node);                                                                           \
    }                                                                                                                                    \
                                                                                                                                         \
    /**                                                                                                                                  \
     * -- skip_ebr_attach_                                                                                                               \
     *                                                                                                                                   \
     * Attach an initialized EBR state to a skiplist.  After this call,                                                                  \
     * skip_remove_node_ will defer node freeing through EBR rather                                                                      \
     * than calling free() immediately.                                                                                                  \
     */                                                                                                                                  \
    void prefix##skip_ebr_attach_##decl(                                                                                                 \
        decl##_t *slist, __skip_ebr_##decl##_t *ebr)                                                                                     \
    {                                                                                                                                    \
        slist->slh_ebr = (void *)ebr;                                                                                                   \
        slist->slh_ebr_retire = __skip_ebr_retire_cb_##decl;                                                                             \
    }                                                                                                                                    \
                                                                                                                                         \
    /**                                                                                                                                  \
     * -- skip_ebr_drain_                                                                                                                \
     *                                                                                                                                   \
     * Force-drain all retire lists, freeing every deferred node                                                                         \
     * regardless of epoch.  Call only when no threads are accessing                                                                     \
     * the data structure (e.g., during shutdown).                                                                                       \
     */                                                                                                                                  \
    void prefix##skip_ebr_drain_##decl(__skip_ebr_##decl##_t *ebr)                                                                       \
    {                                                                                                                                    \
        for (int b = 0; b < 3; b++) {                                                                                                    \
            __skip_ebr_lock_##decl(&ebr->retire_locks[b]);                                                                               \
            __skip_ebr_retired_##decl##_t *list = ebr->retire_lists[b];                                                                  \
            ebr->retire_lists[b] = NULL;                                                                                                 \
            __skip_ebr_unlock_##decl(&ebr->retire_locks[b]);                                                                             \
                                                                                                                                         \
            while (list != NULL) {                                                                                                       \
                __skip_ebr_retired_##decl##_t *cur = list;                                                                               \
                list = cur->next;                                                                                                        \
                cur->slist->slh_fns.free_entry(cur->node);                                                                               \
                free(cur->node);                                                                                                         \
                free(cur);                                                                                                               \
            }                                                                                                                            \
        }                                                                                                                                \
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
        size_t i, cur_era, n_remove = 0, n_discard = 0, n_restore = 0;                                          \
        decl##_node_t *node, *next_node;                                                                         \
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
        /* (a) Collect nodes to remove from the active list (era > target).                                      \
           We decouple iteration from mutation to avoid use-after-free. */                                       \
        decl##_node_t **to_remove = NULL;                                                                        \
        size_t cap_remove = 16;                                                                                  \
        to_remove = (decl##_node_t **)malloc(sizeof(decl##_node_t *) * cap_remove);                              \
        if (to_remove == NULL)                                                                                   \
            return NULL;                                                                                         \
                                                                                                                 \
        SKIPLIST_FOREACH_H2T(decl, prefix, field, slist, node, i)                                                \
        {                                                                                                        \
            (void)i;                                                                                             \
            if (node->field.sle_era > era) {                                                                     \
                if (n_remove >= cap_remove) {                                                                    \
                    cap_remove *= 2;                                                                             \
                    decl##_node_t **tmp = (decl##_node_t **)realloc(                                             \
                        to_remove, sizeof(decl##_node_t *) * cap_remove);                                        \
                    if (tmp == NULL) {                                                                           \
                        free(to_remove);                                                                         \
                        return NULL;                                                                             \
                    }                                                                                            \
                    to_remove = tmp;                                                                             \
                }                                                                                                \
                to_remove[n_remove++] = node;                                                                    \
            }                                                                                                    \
        }                                                                                                        \
                                                                                                                 \
        /* Now remove them. */                                                                                   \
        for (i = 0; i < n_remove; i++)                                                                           \
            prefix##skip_remove_node_##decl(slist, to_remove[i]);                                                \
        free(to_remove);                                                                                         \
                                                                                                                 \
        /* (b) & (c) Walk the preserved list, collecting nodes to discard                                        \
           (era > target) and nodes to restore (era == target). */                                               \
        decl##_node_t **to_discard = NULL;                                                                       \
        decl##_node_t **to_restore = NULL;                                                                       \
        size_t cap_discard = 16, cap_restore = 16;                                                               \
        to_discard = (decl##_node_t **)malloc(sizeof(decl##_node_t *) * cap_discard);                            \
        to_restore = (decl##_node_t **)malloc(sizeof(decl##_node_t *) * cap_restore);                            \
        if (to_discard == NULL || to_restore == NULL) {                                                          \
            free(to_discard);                                                                                    \
            free(to_restore);                                                                                    \
            return NULL;                                                                                         \
        }                                                                                                        \
                                                                                                                 \
        node = slist->slh_snap.pres;                                                                             \
        while (node) {                                                                                           \
            next_node = node->field.sle_levels[0].next;                                                          \
            if (node->field.sle_era > era) {                                                                     \
                if (n_discard >= cap_discard) {                                                                  \
                    cap_discard *= 2;                                                                            \
                    decl##_node_t **tmp = (decl##_node_t **)realloc(                                             \
                        to_discard, sizeof(decl##_node_t *) * cap_discard);                                      \
                    if (tmp == NULL) {                                                                           \
                        free(to_discard);                                                                        \
                        free(to_restore);                                                                        \
                        return NULL;                                                                             \
                    }                                                                                            \
                    to_discard = tmp;                                                                            \
                }                                                                                                \
                to_discard[n_discard++] = node;                                                                  \
            } else if (node->field.sle_era == era) {                                                             \
                if (n_restore >= cap_restore) {                                                                  \
                    cap_restore *= 2;                                                                            \
                    decl##_node_t **tmp = (decl##_node_t **)realloc(                                             \
                        to_restore, sizeof(decl##_node_t *) * cap_restore);                                      \
                    if (tmp == NULL) {                                                                           \
                        free(to_discard);                                                                        \
                        free(to_restore);                                                                        \
                        return NULL;                                                                             \
                    }                                                                                            \
                    to_restore = tmp;                                                                            \
                }                                                                                                \
                to_restore[n_restore++] = node;                                                                  \
            }                                                                                                    \
            node = next_node;                                                                                    \
        }                                                                                                        \
                                                                                                                 \
        /* (b) Remove and free preserved nodes newer than era. */                                                \
        for (i = 0; i < n_discard; i++) {                                                                        \
            /* Unlink from the preserved singly-linked list. */                                                  \
            decl##_node_t **pp = &slist->slh_snap.pres;                                                          \
            while (*pp && *pp != to_discard[i])                                                                  \
                pp = (decl##_node_t **)&(*pp)->field.sle_levels[0].next;                                         \
            if (*pp)                                                                                             \
                *pp = to_discard[i]->field.sle_levels[0].next;                                                   \
            prefix##skip_free_node_##decl(slist, to_discard[i]);                                                 \
        }                                                                                                        \
        free(to_discard);                                                                                        \
                                                                                                                 \
        /* (c) Restore preserved nodes matching era. */                                                          \
        for (i = 0; i < n_restore; i++) {                                                                        \
            /* Unlink from the preserved singly-linked list. */                                                  \
            decl##_node_t **pp = &slist->slh_snap.pres;                                                          \
            while (*pp && *pp != to_restore[i])                                                                  \
                pp = (decl##_node_t **)&(*pp)->field.sle_levels[0].next;                                         \
            if (*pp)                                                                                             \
                *pp = to_restore[i]->field.sle_levels[0].next;                                                   \
                                                                                                                 \
            node = to_restore[i];                                                                                \
            node->field.sle_prev = NULL;                                                                         \
            if (node->field.sle_levels[1].next != 0) {                                                           \
                node->field.sle_levels[1].next = NULL;                                                           \
                prefix##skip_insert_dup_##decl(slist, node);                                                     \
            } else {                                                                                             \
                prefix##skip_insert_##decl(slist, node);                                                         \
            }                                                                                                    \
        }                                                                                                        \
        free(to_restore);                                                                                        \
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

/* ----------------------------------------------------------------
 * SKIPLIST_DECL_VALIDATE
 * ---------------------------------------------------------------- */
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
     *                                                                                                                                                       \
     * Validate the internal consistency of a skiplist.                                                                                                      \
     *                                                                                                                                                       \
     * flags:                                                                                                                                                \
     *   bit 0 (& 1): skip concurrent-specific checks (marked pointers,                                                                                     \
     *                 forward-chain-to-tail) for single-threaded use.                                                                                       \
     *   bit 1 (& 2): early-exit on first error.                                                                                                            \
     */                                                                                                                                                      \
    static int __skip_integrity_check_##decl(decl##_t *slist, int flags)                                                                                     \
    {                                                                                                                                                        \
        size_t n = 0;                                                                                                                                        \
        unsigned long nth, n_err = 0;                                                                                                                        \
        decl##_node_t *node, *prev, *next;                                                                                                                   \
        struct __skiplist_##decl##_entry *this;                                                                                                              \
        int early_exit = (flags & 2);                                                                                                                        \
        int skip_concurrent = (flags & 1);                                                                                                                   \
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
        /* Read head/tail heights atomically */                                                                                                              \
        size_t head_height = atomic_load_explicit(&slist->slh_head->field.sle_height, memory_order_acquire);                                                 \
        size_t tail_height = atomic_load_explicit(&slist->slh_tail->field.sle_height, memory_order_acquire);                                                 \
                                                                                                                                                             \
        if (head_height > SKIPLIST_MAX_HEIGHT) {                                                                                                             \
            __skip_integrity_failure_##decl("skiplist head height > SKIPLIST_MAX_HEIGHT\n");                                                                 \
            n_err++;                                                                                                                                         \
            if (early_exit)                                                                                                                                  \
                return n_err;                                                                                                                                \
        }                                                                                                                                                    \
                                                                                                                                                             \
        if (tail_height > SKIPLIST_MAX_HEIGHT) {                                                                                                             \
            __skip_integrity_failure_##decl("skiplist tail height > SKIPLIST_MAX_HEIGHT\n");                                                                 \
            n_err++;                                                                                                                                         \
            if (early_exit)                                                                                                                                  \
                return n_err;                                                                                                                                \
        }                                                                                                                                                    \
                                                                                                                                                             \
        if (head_height != tail_height) {                                                                                                                    \
            __skip_integrity_failure_##decl("skiplist head & tail height are not equal\n");                                                                  \
            n_err++;                                                                                                                                         \
            if (early_exit)                                                                                                                                  \
                return n_err;                                                                                                                                \
        }                                                                                                                                                    \
                                                                                                                                                             \
        /* TODO: slh_head->field.sle_height should == log(m) where m is the sum of all hits on all nodes */                                                  \
                                                                                                                                                             \
        if (SKIPLIST_MAX_HEIGHT < 1) {                                                                                                                       \
            __skip_integrity_failure_##decl("SKIPLIST_MAX_HEIGHT cannot be less than 1\n");                                                                  \
            n_err++;                                                                                                                                         \
            if (early_exit)                                                                                                                                  \
                return n_err;                                                                                                                                \
        }                                                                                                                                                    \
                                                                                                                                                             \
        /* Validate head node forward pointers */                                                                                                            \
        node = slist->slh_head;                                                                                                                              \
        for (size_t lvl = 0; lvl <= head_height; lvl++) {                                                                                                    \
            decl##_node_t *head_next = atomic_load_explicit(&node->field.sle_levels[lvl].next, memory_order_acquire);                                        \
            decl##_node_t *head_next_unmarked = __SKIP_UNMARK(head_next);                                                                                    \
            if (head_next_unmarked == NULL) {                                                                                                                \
                __skip_integrity_failure_##decl("the head's %lu next node should not be NULL\n", lvl);                                                       \
                n_err++;                                                                                                                                     \
                if (early_exit)                                                                                                                              \
                    return n_err;                                                                                                                            \
            }                                                                                                                                                \
            /* Head node next pointers should never be marked */                                                                                             \
            if (!skip_concurrent && __SKIP_IS_MARKED(head_next)) {                                                                                           \
                __skip_integrity_failure_##decl("the head's %lu next pointer is marked (should never be)\n", lvl);                                           \
                n_err++;                                                                                                                                     \
                if (early_exit)                                                                                                                              \
                    return n_err;                                                                                                                            \
            }                                                                                                                                                \
            n = lvl;                                                                                                                                         \
            if (head_next_unmarked == slist->slh_tail)                                                                                                       \
                break;                                                                                                                                       \
        }                                                                                                                                                    \
        n++;                                                                                                                                                 \
        for (size_t lvl = n; lvl <= head_height; lvl++) {                                                                                                    \
            decl##_node_t *head_next = atomic_load_explicit(&node->field.sle_levels[lvl].next, memory_order_acquire);                                        \
            decl##_node_t *head_next_unmarked = __SKIP_UNMARK(head_next);                                                                                    \
            if (head_next_unmarked == NULL) {                                                                                                                \
                __skip_integrity_failure_##decl("the head's %lu next node should not be NULL\n", lvl);                                                       \
                n_err++;                                                                                                                                     \
                if (early_exit)                                                                                                                              \
                    return n_err;                                                                                                                            \
            }                                                                                                                                                \
        }                                                                                                                                                    \
                                                                                                                                                             \
        /* Check: tail->prev should not be head when list is non-empty */                                                                                    \
        size_t list_len = atomic_load_explicit(&slist->slh_length, memory_order_relaxed);                                                                    \
        decl##_node_t *tail_prev = atomic_load_explicit(&slist->slh_tail->field.sle_prev, memory_order_acquire);                                             \
        if (list_len > 0 && tail_prev == slist->slh_head) {                                                                                                  \
            __skip_integrity_failure_##decl("slist->slh_length is %lu, but tail->prev == head, not an internal node\n", list_len);                           \
            n_err++;                                                                                                                                         \
            if (early_exit)                                                                                                                                  \
                return n_err;                                                                                                                                \
        }                                                                                                                                                    \
                                                                                                                                                             \
        /* Forward pointer consistency: at each level, following next pointers                                                                               \
         * from head should eventually reach tail. Only checked in concurrent                                                                                \
         * mode (when !(flags & 1)) to verify no dangling chains exist. */                                                                                   \
        if (!skip_concurrent) {                                                                                                                              \
            for (size_t lvl = 0; lvl <= head_height; lvl++) {                                                                                                \
                decl##_node_t *walk = slist->slh_head;                                                                                                       \
                size_t steps = 0;                                                                                                                            \
                size_t max_steps = list_len + 2; /* head + nodes + tail */                                                                                   \
                while (walk != NULL && walk != slist->slh_tail && steps <= max_steps) {                                                                            \
                    decl##_node_t *walk_next = atomic_load_explicit(&walk->field.sle_levels[lvl].next, memory_order_acquire);                                 \
                    walk = __SKIP_UNMARK(walk_next);                                                                                                         \
                    steps++;                                                                                                                                 \
                }                                                                                                                                            \
                if (walk != slist->slh_tail) {                                                                                                               \
                    __skip_integrity_failure_##decl("forward chain at level %lu does not reach tail (cycle or NULL after %lu steps)\n", lvl, steps);          \
                    n_err++;                                                                                                                                 \
                    if (early_exit)                                                                                                                          \
                        return n_err;                                                                                                                        \
                }                                                                                                                                            \
            }                                                                                                                                                \
        }                                                                                                                                                    \
                                                                                                                                                             \
        /* Validate each node */                                                                                                                             \
        SKIPLIST_FOREACH_H2T(decl, prefix, field, slist, node, nth)                                                                                          \
        {                                                                                                                                                    \
            this = &node->field;                                                                                                                             \
            size_t node_height = atomic_load_explicit(&this->sle_height, memory_order_acquire);                                                              \
                                                                                                                                                             \
            if (node_height > head_height) {                                                                                                                 \
                __skip_integrity_failure_##decl("the %lu node's [%p] height %lu is > head %lu\n", nth, (void *)node, node_height,                            \
                    head_height);                                                                                                                            \
                n_err++;                                                                                                                                     \
                if (early_exit)                                                                                                                              \
                    return n_err;                                                                                                                            \
            }                                                                                                                                                \
                                                                                                                                                             \
            if (this->sle_levels == NULL) {                                                                                                                  \
                __skip_integrity_failure_##decl("the %lu node's [%p] next field should never be NULL\n", nth, (void *)node);                                 \
                n_err++;                                                                                                                                     \
                if (early_exit)                                                                                                                              \
                    return n_err;                                                                                                                            \
            }                                                                                                                                                \
                                                                                                                                                             \
            decl##_node_t *node_prev = atomic_load_explicit(&this->sle_prev, memory_order_acquire);                                                          \
            if (node_prev == NULL) {                                                                                                                         \
                __skip_integrity_failure_##decl("the %lu node [%p] prev field should never be NULL\n", nth, (void *)node);                                   \
                n_err++;                                                                                                                                     \
                if (early_exit)                                                                                                                              \
                    return n_err;                                                                                                                            \
            }                                                                                                                                                \
                                                                                                                                                             \
            /* Check forward pointers at each level of this node */                                                                                          \
            for (size_t lvl = 0; lvl <= node_height; lvl++) {                                                                                                \
                decl##_node_t *lvl_next = atomic_load_explicit(&this->sle_levels[lvl].next, memory_order_acquire);                                           \
                decl##_node_t *lvl_next_unmarked = __SKIP_UNMARK(lvl_next);                                                                                  \
                                                                                                                                                             \
                /* No next pointer should be NULL (should at least point to tail) */                                                                         \
                if (lvl_next_unmarked == NULL) {                                                                                                             \
                    __skip_integrity_failure_##decl("the %lu node's next[%lu] should not be NULL\n", nth, lvl);                                              \
                    n_err++;                                                                                                                                 \
                    if (early_exit)                                                                                                                          \
                        return n_err;                                                                                                                        \
                }                                                                                                                                            \
                                                                                                                                                             \
                /* In a quiescent list, no reachable next pointer should be marked */                                                                        \
                if (!skip_concurrent && __SKIP_IS_MARKED(lvl_next)) {                                                                                        \
                    __skip_integrity_failure_##decl("the %lu node's next[%lu] is marked in a quiescent list\n", nth, lvl);                                   \
                    n_err++;                                                                                                                                 \
                    if (early_exit)                                                                                                                          \
                        return n_err;                                                                                                                        \
                }                                                                                                                                            \
                                                                                                                                                             \
                n = lvl;                                                                                                                                     \
                if (lvl_next_unmarked == slist->slh_tail)                                                                                                    \
                    break;                                                                                                                                   \
            }                                                                                                                                                \
            n++;                                                                                                                                             \
            for (size_t lvl = n; lvl <= node_height; lvl++) {                                                                                                \
                decl##_node_t *lvl_next = atomic_load_explicit(&this->sle_levels[lvl].next, memory_order_acquire);                                           \
                decl##_node_t *lvl_next_unmarked = __SKIP_UNMARK(lvl_next);                                                                                  \
                if (lvl_next_unmarked == NULL) {                                                                                                             \
                    __skip_integrity_failure_##decl("after the %lunth the %lu node's next[%lu] should not be NULL\n", n, nth, lvl);                          \
                    n_err++;                                                                                                                                 \
                    if (early_exit)                                                                                                                          \
                        return n_err;                                                                                                                        \
                } else if (lvl_next_unmarked != slist->slh_tail) {                                                                                           \
                    __skip_integrity_failure_##decl("after the %lunth the %lu node's next[%lu] should point to the tail\n", n, nth, lvl);                    \
                    n_err++;                                                                                                                                 \
                    if (early_exit)                                                                                                                          \
                        return n_err;                                                                                                                        \
                }                                                                                                                                            \
                /* Check for marked pointers in upper levels too */                                                                                          \
                if (!skip_concurrent && __SKIP_IS_MARKED(lvl_next)) {                                                                                        \
                    __skip_integrity_failure_##decl("the %lu node's next[%lu] is marked in a quiescent list\n", nth, lvl);                                   \
                    n_err++;                                                                                                                                 \
                    if (early_exit)                                                                                                                          \
                        return n_err;                                                                                                                        \
                }                                                                                                                                            \
            }                                                                                                                                                \
                                                                                                                                                             \
            decl##_node_t *a = (decl##_node_t *)(uintptr_t)this->sle_levels;                                                                                 \
            decl##_node_t *b = (decl##_node_t *)(intptr_t)((uintptr_t)node + sizeof(decl##_node_t));                                                         \
            if (a != b) {                                                                                                                                    \
                __skip_integrity_failure_##decl("the %lu node's [%p] next field isn't at the proper offset relative to the node\n", nth, (void *)node);      \
                n_err++;                                                                                                                                     \
                if (early_exit)                                                                                                                              \
                    return n_err;                                                                                                                            \
            }                                                                                                                                                \
                                                                                                                                                             \
            next = __SKIP_UNMARK(atomic_load_explicit(&this->sle_levels[0].next, memory_order_acquire));                                                     \
            prev = __SKIP_UNMARK(atomic_load_explicit(&this->sle_prev, memory_order_acquire));                                                               \
            if (__skip_compare_nodes_##decl(slist, node, node, slist->slh_aux) != 0) {                                                                       \
                __skip_integrity_failure_##decl("the %lu node [%p] is not equal to itself\n", nth, (void *)node);                                            \
                n_err++;                                                                                                                                     \
                if (early_exit)                                                                                                                              \
                    return n_err;                                                                                                                            \
            }                                                                                                                                                \
                                                                                                                                                             \
            if (__skip_compare_nodes_##decl(slist, node, prev, slist->slh_aux) < 0) {                                                                        \
                __skip_integrity_failure_##decl("the %lu node [%p] is not greater than the prev node [%p]\n", nth, (void *)node, (void *)prev);              \
                n_err++;                                                                                                                                     \
                if (early_exit)                                                                                                                              \
                    return n_err;                                                                                                                            \
            }                                                                                                                                                \
                                                                                                                                                             \
            if (__skip_compare_nodes_##decl(slist, node, next, slist->slh_aux) > 0) {                                                                        \
                __skip_integrity_failure_##decl("the %lu node [%p] is not less than the next node [%p]\n", nth, (void *)node, (void *)next);                 \
                n_err++;                                                                                                                                     \
                if (early_exit)                                                                                                                              \
                    return n_err;                                                                                                                            \
            }                                                                                                                                                \
                                                                                                                                                             \
            if (__skip_compare_nodes_##decl(slist, prev, node, slist->slh_aux) > 0) {                                                                        \
                __skip_integrity_failure_##decl("the prev node [%p] is not less than the %lu node [%p]\n", (void *)prev, nth, (void *)node);                 \
                n_err++;                                                                                                                                     \
                if (early_exit)                                                                                                                              \
                    return n_err;                                                                                                                            \
            }                                                                                                                                                \
                                                                                                                                                             \
            if (__skip_compare_nodes_##decl(slist, next, node, slist->slh_aux) < 0) {                                                                        \
                __skip_integrity_failure_##decl("the next node [%p] is not greater than the %lu node [%p]\n", (void *)next, nth, (void *)node);              \
                n_err++;                                                                                                                                     \
                if (early_exit)                                                                                                                              \
                    return n_err;                                                                                                                            \
            }                                                                                                                                                \
        }                                                                                                                                                    \
                                                                                                                                                             \
        if (list_len != nth) {                                                                                                                               \
            __skip_integrity_failure_##decl("slist->slh_length (%lu) doesn't match the count (%lu) of nodes between the head and tail\n", list_len,          \
                nth);                                                                                                                                        \
            n_err++;                                                                                                                                         \
            if (early_exit)                                                                                                                                  \
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

/* ----------------------------------------------------------------
 * SKIPLIST_DECL_DOT
 *
 * NOTE: Under concurrency, the DOT output is a point-in-time snapshot.
 * Field reads are performed with atomic_load_explicit but the overall
 * picture may not be globally consistent if concurrent mutations are
 * in progress.  For a consistent diagram, quiesce all writers first.
 * ---------------------------------------------------------------- */
#define SKIPLIST_DECL_DOT(decl, prefix, field)                                                                                      \
                                                                                                                                    \
    /* A type for a function that writes into a char[2048] buffer                                                                   \
     * a description of the value within the node. */                                                                               \
    typedef void (*skip_sprintf_node_##decl##_t)(decl##_node_t *, char *);                                                          \
                                                                                                                                    \
    /* -- __skip_dot_width_                                                                                                         \
     * Counts how many nodes lie between `from` and `to` via sle_prev.                                                              \
     */                                                                                                                             \
    static size_t __skip_dot_width_##decl(decl##_t *slist, decl##_node_t *from, decl##_node_t *to)                                  \
    {                                                                                                                               \
        size_t w = 1;                                                                                                               \
        decl##_node_t *n = to;                                                                                                      \
                                                                                                                                    \
        if (from == NULL || to == NULL)                                                                                             \
            return 0;                                                                                                               \
                                                                                                                                    \
        while (__SKIP_UNMARK(atomic_load_explicit(&n->field.sle_prev, memory_order_acquire)) != from) {                              \
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
     * Marked (logically deleted) nodes are shown with dashed red border.                                                           \
     * Marked next-pointer edges are shown as dashed red lines.                                                                     \
     */                                                                                                                             \
    static void __skip_dot_node_##decl(FILE *os, decl##_t *slist, decl##_node_t *node, size_t nsg, skip_sprintf_node_##decl##_t fn) \
    {                                                                                                                               \
        char buf[2048];                                                                                                             \
        decl##_node_t *raw_next, *next;                                                                                             \
        size_t node_height = atomic_load_explicit(&node->field.sle_height, memory_order_acquire);                                   \
                                                                                                                                    \
        /* Check if this node is logically deleted (level 0 next is marked) */                                                      \
        decl##_node_t *lvl0_raw = atomic_load_explicit(&node->field.sle_levels[0].next, memory_order_acquire);                      \
        int node_is_marked = __SKIP_IS_MARKED(lvl0_raw);                                                                            \
                                                                                                                                    \
        __skip_dot_write_node_##decl(os, nsg, node);                                                                                \
        fprintf(os, " [label = \"");                                                                                                \
        fflush(os);                                                                                                                 \
        for (size_t lvl = node_height; lvl != (size_t)-1; lvl--) {                                                                  \
            raw_next = atomic_load_explicit(&node->field.sle_levels[lvl].next, memory_order_acquire);                                \
            next = __SKIP_UNMARK(raw_next);                                                                                          \
            next = (next == slist->slh_tail) ? NULL : next;                                                                         \
            (void)__skip_dot_width_##decl;                                                                                          \
            size_t hits = atomic_load_explicit(&node->field.sle_levels[lvl].hits, memory_order_relaxed);                             \
            fprintf(os, " { <w%lu> %lu | <f%lu> ", lvl, hits, lvl);                                                                 \
            if (__SKIP_IS_MARKED(raw_next))                                                                                          \
                fprintf(os, "X ");                                                                                                   \
            if (next)                                                                                                                \
                fprintf(os, "%p } |", (void *)next);                                                                                \
            else                                                                                                                    \
                fprintf(os, "0x0 } |");                                                                                             \
            fflush(os);                                                                                                             \
        }                                                                                                                           \
        if (fn) {                                                                                                                   \
            fn(node, buf);                                                                                                          \
            fprintf(os, " <f0> \u219F %lu \u226B %s \"\n", node_height, buf);                                                       \
        } else {                                                                                                                    \
            fprintf(os, " <f0> \u219F %lu \"\n", node_height);                                                                      \
        }                                                                                                                           \
        fprintf(os, "shape = \"record\"\n");                                                                                        \
        /* Render marked (logically deleted) nodes with dashed red border */                                                        \
        if (node_is_marked) {                                                                                                        \
            fprintf(os, "style = \"dashed\"\n");                                                                                     \
            fprintf(os, "color = \"red\"\n");                                                                                        \
            fprintf(os, "fontcolor = \"red\"\n");                                                                                    \
        }                                                                                                                           \
        fprintf(os, "];\n");                                                                                                        \
        fflush(os);                                                                                                                 \
                                                                                                                                    \
        /* Now edges */                                                                                                             \
        for (size_t lvl = 0; lvl <= node_height; lvl++) {                                                                           \
            raw_next = atomic_load_explicit(&node->field.sle_levels[lvl].next, memory_order_acquire);                                \
            int edge_marked = __SKIP_IS_MARKED(raw_next);                                                                            \
            next = __SKIP_UNMARK(raw_next);                                                                                          \
            next = (next == slist->slh_tail) ? NULL : next;                                                                         \
            __skip_dot_write_node_##decl(os, nsg, node);                                                                            \
            fprintf(os, ":f%lu -> ", lvl);                                                                                          \
            __skip_dot_write_node_##decl(os, nsg, next);                                                                            \
            /* Render marked edges as dashed red lines */                                                                           \
            if (edge_marked)                                                                                                         \
                fprintf(os, ":w%lu [style=dashed, color=red];\n", lvl);                                                              \
            else                                                                                                                    \
                fprintf(os, ":w%lu [];\n", lvl);                                                                                    \
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
            fprintf(os, "node0 [shape=record, label = \"");                                                                         \
            for (i = 0; i < nsg; ++i) {                                                                                             \
                fprintf(os, "<f%lu> | ", i);                                                                                        \
            }                                                                                                                       \
            fprintf(os, "\", style=invis, width=0.01];\n");                                                                         \
                                                                                                                                    \
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
     * NOTE: Under concurrency, the DOT output represents a point-in-time                                                          \
     * snapshot. Atomic loads are used for individual field reads, but the                                                          \
     * overall diagram may not be globally consistent if concurrent                                                                 \
     * mutations are in progress. Quiesce all writers for a consistent view.                                                        \
     *                                                                                                                              \
     * To view the output:                                                                                                          \
     * $ dot -Tps filename.dot -o outfile.ps                                                                                        \
     */                                                                                                                             \
    int prefix##skip_dot_##decl(FILE *os, decl##_t *slist, size_t nsg, char *msg, skip_sprintf_node_##decl##_t fn)                  \
    {                                                                                                                               \
        int letitgo = 0;                                                                                                            \
        size_t i;                                                                                                                   \
        decl##_node_t *node, *next;                                                                                                 \
                                                                                                                                    \
        if (slist == NULL || fn == NULL)                                                                                            \
            return nsg;                                                                                                             \
                                                                                                                                    \
        size_t dot_head_height = atomic_load_explicit(&slist->slh_head->field.sle_height, memory_order_acquire);                     \
                                                                                                                                    \
        if (nsg == 0) {                                                                                                             \
            fprintf(os, "digraph Skiplist {\n");                                                                                    \
            fprintf(os, "label = \"Skiplist (point-in-time snapshot).\"\n");                                                        \
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
        decl##_node_t *head_lvl0_next = __SKIP_UNMARK(                                                                               \
            atomic_load_explicit(&slist->slh_head->field.sle_levels[0].next, memory_order_acquire));                                 \
        if (dot_head_height || head_lvl0_next != slist->slh_tail)                                                                    \
            letitgo = 1;                                                                                                            \
                                                                                                                                    \
        /* Write out the head node fields */                                                                                        \
        node = slist->slh_head;                                                                                                     \
        if (letitgo) {                                                                                                              \
            for (size_t lvl = dot_head_height; lvl != (size_t)-1; lvl--) {                                                          \
                decl##_node_t *raw = atomic_load_explicit(&node->field.sle_levels[lvl].next, memory_order_acquire);                  \
                next = __SKIP_UNMARK(raw);                                                                                           \
                next = (next == slist->slh_tail) ? NULL : next;                                                                     \
                size_t hits = atomic_load_explicit(&node->field.sle_levels[lvl].hits, memory_order_relaxed);                         \
                fprintf(os, "{ %lu | <f%lu> ", hits, lvl);                                                                          \
                if (next)                                                                                                           \
                    fprintf(os, "%p }", (void *)next);                                                                              \
                else                                                                                                                \
                    fprintf(os, "0x0 }");                                                                                           \
                if (lvl == 0)                                                                                                       \
                    continue;                                                                                                       \
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
            for (size_t lvl = 0; lvl <= dot_head_height; lvl++) {                                                                   \
                decl##_node_t *raw = atomic_load_explicit(&node->field.sle_levels[lvl].next, memory_order_acquire);                  \
                int edge_marked = __SKIP_IS_MARKED(raw);                                                                             \
                next = __SKIP_UNMARK(raw);                                                                                           \
                next = (next == slist->slh_tail) ? NULL : next;                                                                     \
                fprintf(os, "\"HeadNode%lu\":f%lu -> ", nsg, lvl);                                                                  \
                __skip_dot_write_node_##decl(os, nsg, next);                                                                        \
                if (edge_marked)                                                                                                     \
                    fprintf(os, ":w%lu [style=dashed, color=red];\n", lvl);                                                          \
                else                                                                                                                \
                    fprintf(os, ":w%lu [];\n", lvl);                                                                                \
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
            size_t th = dot_head_height;                                                                                            \
            for (size_t lvl = th; lvl != (size_t)-1; lvl--) {                                                                       \
                decl##_node_t *raw = atomic_load_explicit(&node->field.sle_levels[lvl].next, memory_order_acquire);                  \
                (void)raw; /* tail next pointers are unused in display */                                                           \
                fprintf(os, "<w%lu> 0x0", lvl);                                                                                     \
                if (lvl == 0)                                                                                                       \
                    continue;                                                                                                       \
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

/*
 * SKIPLIST_DECL_POOL -- Fixed-capacity pre-allocation pool for skiplist nodes.
 *
 * This macro generates a lock-free pool allocator that pre-allocates a
 * contiguous block of memory for `capacity` node slots.  Each slot is
 * cache-line aligned (64 bytes) to prevent false sharing.  The free list
 * is managed via atomic CAS on an index (int32_t), avoiding the ABA
 * problem that plagues pointer-based lock-free stacks.
 *
 * Usage:
 *   SKIPLIST_DECL_POOL(ex, api_, entries, 1024)
 *
 * Then:
 *   __skip_pool_ex_t pool;
 *   api_skip_pool_init_ex(&pool, 1024);
 *   api_skip_pool_attach_ex(&list, &pool);
 *   // ... use the list normally; alloc/free now use the pool ...
 *   api_skip_pool_destroy_ex(&pool);
 */

#define SKIPLIST_DECL_POOL(decl, prefix, field, capacity_hint)                                                       \
                                                                                                                     \
    /* ------------------------------------------------------------------ */                                         \
    /* Pool type definition                                                */                                         \
    /* ------------------------------------------------------------------ */                                         \
    typedef struct __skip_pool_##decl {                                                                               \
        size_t capacity;          /* total number of slots */                                                         \
        size_t slot_size;         /* bytes per slot (aligned to 64) */                                                \
        _Alignas(64) char *slots; /* contiguous allocation for all slots */                                           \
        _Atomic(int32_t) free_head; /* index of first free slot, -1 = empty */                                       \
    } __skip_pool_##decl##_t;                                                                                        \
                                                                                                                     \
    /* ------------------------------------------------------------------ */                                         \
    /* __skip_pool_slot_ptr_ -- Return a pointer to the start of slot `i` */                                         \
    /* ------------------------------------------------------------------ */                                         \
    static inline char *__skip_pool_slot_ptr_##decl(__skip_pool_##decl##_t *pool, int32_t i)                          \
    {                                                                                                                \
        return pool->slots + ((size_t)i * pool->slot_size);                                                          \
    }                                                                                                                \
                                                                                                                     \
    /* ------------------------------------------------------------------ */                                         \
    /* __skip_pool_next_free_ -- Read/write the next-free index stored in */                                         \
    /*   the first bytes of a free slot.                                   */                                         \
    /* ------------------------------------------------------------------ */                                         \
    static inline int32_t __skip_pool_get_next_free_##decl(__skip_pool_##decl##_t *pool, int32_t i)                   \
    {                                                                                                                \
        int32_t next;                                                                                                \
        memcpy(&next, __skip_pool_slot_ptr_##decl(pool, i), sizeof(int32_t));                                        \
        return next;                                                                                                 \
    }                                                                                                                \
                                                                                                                     \
    static inline void __skip_pool_set_next_free_##decl(__skip_pool_##decl##_t *pool, int32_t i, int32_t next)       \
    {                                                                                                                \
        memcpy(__skip_pool_slot_ptr_##decl(pool, i), &next, sizeof(int32_t));                                        \
    }                                                                                                                \
                                                                                                                     \
    /* ------------------------------------------------------------------ */                                         \
    /* __skip_pool_index_of_ -- Given a node pointer, return its slot     */                                         \
    /*   index (or -1 if the pointer is outside the pool).                */                                         \
    /* ------------------------------------------------------------------ */                                         \
    static inline int32_t __skip_pool_index_of_##decl(__skip_pool_##decl##_t *pool, decl##_node_t *node)             \
    {                                                                                                                \
        char *p = (char *)node;                                                                                      \
        if (p < pool->slots || p >= pool->slots + (pool->capacity * pool->slot_size))                                \
            return -1;                                                                                               \
        size_t offset = (size_t)(p - pool->slots);                                                                   \
        if (offset % pool->slot_size != 0)                                                                           \
            return -1;                                                                                               \
        return (int32_t)(offset / pool->slot_size);                                                                  \
    }                                                                                                                \
                                                                                                                     \
    /* ------------------------------------------------------------------ */                                         \
    /* skip_pool_init_ -- Initialize the pool with `capacity` slots.      */                                         \
    /*                                                                     */                                         \
    /* Each slot is sized to hold one decl##_node_t plus the sle_levels   */                                         \
    /* array for SKIPLIST_MAX_HEIGHT levels, rounded up to a multiple of  */                                         \
    /* 64 bytes for cache-line alignment.                                  */                                         \
    /* ------------------------------------------------------------------ */                                         \
    int prefix##skip_pool_init_##decl(__skip_pool_##decl##_t *pool, size_t capacity)                                 \
    {                                                                                                                \
        if (pool == NULL || capacity == 0)                                                                           \
            return EINVAL;                                                                                           \
                                                                                                                     \
        /* Compute raw slot size: node struct + levels array */                                                      \
        size_t raw_size = sizeof(decl##_node_t) +                                                                    \
                          sizeof(struct __skiplist_##decl##_level) * SKIPLIST_MAX_HEIGHT;                             \
                                                                                                                     \
        /* Ensure each slot is at least large enough to hold an int32_t   */                                         \
        /* for the free-list link (it always will be, but be safe).       */                                         \
        if (raw_size < sizeof(int32_t))                                                                              \
            raw_size = sizeof(int32_t);                                                                              \
                                                                                                                     \
        /* Round up to next multiple of 64 for cache-line alignment */                                               \
        size_t slot_size = (raw_size + 63u) & ~(size_t)63u;                                                          \
                                                                                                                     \
        pool->capacity = capacity;                                                                                   \
        pool->slot_size = slot_size;                                                                                 \
                                                                                                                     \
        /* Allocate the contiguous slab, aligned to 64 bytes */                                                      \
        pool->slots = (char *)aligned_alloc(64, slot_size * capacity);                                               \
        if (pool->slots == NULL)                                                                                     \
            return ENOMEM;                                                                                           \
                                                                                                                     \
        /* Zero the entire slab */                                                                                   \
        memset(pool->slots, 0, slot_size * capacity);                                                                \
                                                                                                                     \
        /* Build the free list: slot[0]->1, slot[1]->2, ..., slot[n-1]->-1 */                                       \
        for (size_t i = 0; i < capacity - 1; i++) {                                                                  \
            __skip_pool_set_next_free_##decl(pool, (int32_t)i, (int32_t)(i + 1));                                    \
        }                                                                                                            \
        __skip_pool_set_next_free_##decl(pool, (int32_t)(capacity - 1), -1);                                         \
                                                                                                                     \
        atomic_store_explicit(&pool->free_head, 0, memory_order_release);                                            \
                                                                                                                     \
        return 0;                                                                                                    \
    }                                                                                                                \
                                                                                                                     \
    /* ------------------------------------------------------------------ */                                         \
    /* skip_pool_alloc_ -- Pop a slot from the free list (lock-free).     */                                         \
    /*                                                                     */                                         \
    /* Returns a fully zeroed node with sle_levels pointing into the      */                                         \
    /* trailing portion of the same slot.  Returns NULL when the pool     */                                         \
    /* is exhausted.                                                       */                                         \
    /* ------------------------------------------------------------------ */                                         \
    decl##_node_t *prefix##skip_pool_alloc_##decl(__skip_pool_##decl##_t *pool)                                      \
    {                                                                                                                \
        int32_t head, next;                                                                                          \
        do {                                                                                                         \
            head = atomic_load_explicit(&pool->free_head, memory_order_acquire);                                     \
            if (head < 0)                                                                                            \
                return NULL; /* pool exhausted */                                                                     \
            next = __skip_pool_get_next_free_##decl(pool, head);                                                     \
        } while (!atomic_compare_exchange_weak_explicit(                                                             \
            &pool->free_head, &head, next,                                                                           \
            memory_order_acq_rel, memory_order_acquire));                                                            \
                                                                                                                     \
        /* Zero the slot and initialize the node */                                                                  \
        char *slot = __skip_pool_slot_ptr_##decl(pool, head);                                                        \
        memset(slot, 0, pool->slot_size);                                                                            \
                                                                                                                     \
        decl##_node_t *node = (decl##_node_t *)slot;                                                                 \
        node->field.sle_height = 0;                                                                                  \
        node->field.sle_levels =                                                                                     \
            (struct __skiplist_##decl##_level *)((uintptr_t)node + sizeof(decl##_node_t));                           \
                                                                                                                     \
        return node;                                                                                                 \
    }                                                                                                                \
                                                                                                                     \
    /* ------------------------------------------------------------------ */                                         \
    /* skip_pool_free_ -- Push a slot back onto the free list (lock-free).*/                                         \
    /* ------------------------------------------------------------------ */                                         \
    void prefix##skip_pool_free_##decl(__skip_pool_##decl##_t *pool, decl##_node_t *node)                            \
    {                                                                                                                \
        int32_t idx = __skip_pool_index_of_##decl(pool, node);                                                       \
        if (idx < 0)                                                                                                 \
            return; /* not from this pool, ignore */                                                                  \
                                                                                                                     \
        int32_t head;                                                                                                \
        do {                                                                                                         \
            head = atomic_load_explicit(&pool->free_head, memory_order_acquire);                                     \
            __skip_pool_set_next_free_##decl(pool, idx, head);                                                       \
        } while (!atomic_compare_exchange_weak_explicit(                                                             \
            &pool->free_head, &head, idx,                                                                            \
            memory_order_acq_rel, memory_order_acquire));                                                            \
    }                                                                                                                \
                                                                                                                     \
    /* ------------------------------------------------------------------ */                                         \
    /* skip_pool_is_from_ -- Check if a node belongs to this pool.        */                                         \
    /* ------------------------------------------------------------------ */                                         \
    int prefix##skip_pool_is_from_##decl(__skip_pool_##decl##_t *pool, decl##_node_t *node)                          \
    {                                                                                                                \
        return __skip_pool_index_of_##decl(pool, node) >= 0;                                                         \
    }                                                                                                                \
                                                                                                                     \
    /* ------------------------------------------------------------------ */                                         \
    /* skip_pool_destroy_ -- Free the contiguous slab.                    */                                         \
    /* ------------------------------------------------------------------ */                                         \
    void prefix##skip_pool_destroy_##decl(__skip_pool_##decl##_t *pool)                                              \
    {                                                                                                                \
        if (pool == NULL)                                                                                            \
            return;                                                                                                  \
        free(pool->slots);                                                                                           \
        pool->slots = NULL;                                                                                          \
        pool->capacity = 0;                                                                                          \
        atomic_store_explicit(&pool->free_head, -1, memory_order_release);                                           \
    }                                                                                                                \
                                                                                                                     \
    /* ------------------------------------------------------------------ */                                         \
    /* Pool-aware alloc/free wrappers                                      */                                         \
    /*                                                                     */                                         \
    /* These replace skip_alloc_node_ and skip_free_node_ when a pool     */                                         \
    /* is attached to the skiplist.  They check the pool first, falling   */                                         \
    /* back to malloc/free when the pool is exhausted or the node is not  */                                         \
    /* from the pool.                                                      */                                         \
    /* ------------------------------------------------------------------ */                                         \
    int prefix##skip_pool_alloc_node_##decl(__skip_pool_##decl##_t *pool, decl##_node_t **node)                      \
    {                                                                                                                \
        decl##_node_t *n = prefix##skip_pool_alloc_##decl(pool);                                                     \
        if (n != NULL) {                                                                                             \
            *node = n;                                                                                               \
            return 0;                                                                                                \
        }                                                                                                            \
        /* Pool exhausted -- return ENOMEM.                                */                                         \
        /* If fallback-to-malloc is desired, the caller can try            */                                         \
        /* prefix##skip_alloc_node_##decl() instead.                       */                                         \
        return ENOMEM;                                                                                               \
    }                                                                                                                \
                                                                                                                     \
    void prefix##skip_pool_free_node_##decl(                                                                         \
        __skip_pool_##decl##_t *pool, decl##_t *slist, decl##_node_t *node)                                          \
    {                                                                                                                \
        /* Always call the user's free_entry to release user-held resources */                                       \
        slist->slh_fns.free_entry(node);                                                                             \
                                                                                                                     \
        /* If the node came from the pool, return it there; otherwise free */                                        \
        if (prefix##skip_pool_is_from_##decl(pool, node)) {                                                          \
            prefix##skip_pool_free_##decl(pool, node);                                                               \
        } else {                                                                                                     \
            free(node);                                                                                              \
        }                                                                                                            \
    }

#endif /* _SKIPLIST_H_ */
