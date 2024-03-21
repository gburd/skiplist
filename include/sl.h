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

#ifndef _SKIPLIST_H_
#define _SKIPLIST_H_

/*
 * This file defines a skiplist data structure.
 *
 * A skiplist is a way of storing sorted elements in such a way that they can be
 * accessed, inserted and removed, all in O(log(n)) on average.
 *
 * Conceptually, a skiplist is arranged as follows:
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
 * Conceptually, the skiplist can be thought of as a stack of linked lists. At
 * the very bottom is the full linked list with every element, and each layer
 * above corresponds to a linked list containing a random subset of the elements
 * from the layer immediately below it. The probability distribution that
 * determines this random subset can be customized, but typically a layer will
 * contain half the nodes from the layer below.
 *
 * This implementation maintains a doubly-linked list at the bottom layer to
 * support efficient iteration in either direction.  There is also a guard
 * node at the tail rather than simply pointing to NULL.
 *
 * <head> <-> [1] <-> [2] <-> [3] <-> [4] <-> [5] <-> [6] <-> [7] <-> <tail>
 *
 * Safety:
 *
 * The ordered skiplist relies on a well-behaved comparison
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
 *      journal = {Commun. ACM}, month = {jun}, pages = {668–676}, numpages = {9},
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

/*
 * Skiplist declarations.
 */

#ifndef SKIPLIST_MAX_HEIGHT
#define SKIPLIST_MAX_HEIGHT 1
#endif

#define SKIPLIST_ENTRY(type)           \
    struct __skiplist_entry {          \
        struct __skiplist_idx {        \
            struct type *prev, **next; \
            size_t cap, len;           \
        } sle;                         \
    }

/*
 * Skip List access methods.
 */
#define SKIP_FIRST(head) ((head)->slh_head)
#define SKIP_LAST(head) ((head)->slh_tail)
#define SKIP_NEXT(elm, field) ((elm)->field.sle.next[0])
#define SKIP_PREV(elm, field) ((elm)->field.sle.prev)
#define SKIP_EMPTY(head) ((head)->length == 0)

/*
 * Skip List functions.
 */

#define SKIP_COMPARATOR(list, type, fn)                                                 \
    int __skip_cmp_##type(struct list *head, struct type *a, struct type *b, void *aux) \
    {                                                                                   \
        if (a == b)                                                                     \
            return 0;                                                                   \
        if (a == (head)->slh_head || b == (head)->slh_tail)                             \
            return -1;                                                                  \
        if (a == (head)->slh_tail || b == (head)->slh_head)                             \
            return 1;                                                                   \
        fn                                                                              \
    }

#define SKIPLIST_DECL(decl, prefix, field, free_node_blk, update_node_blk, snap_node_blk, sizeof_entry_blk)                                           \
                                                                                                                                                      \
    /* Skiplist node type */                                                                                                                          \
    typedef struct decl##_node decl##_node_t;                                                                                                         \
                                                                                                                                                      \
    /* Skiplist type */                                                                                                                               \
    typedef struct decl {                                                                                                                             \
        size_t level, length, max;                                                                                                                    \
        int (*cmp)(struct decl *, decl##_node_t *, decl##_node_t *, void *);                                                                          \
        void *aux;                                                                                                                                    \
        decl##_node_t *slh_head;                                                                                                                      \
        decl##_node_t *slh_tail;                                                                                                                      \
    } decl##_t;                                                                                                                                       \
                                                                                                                                                      \
    /* Snapshot of a Skiplist */                                                                                                                      \
    typedef struct decl##_snap {                                                                                                                      \
        decl##_t list;                                                                                                                                \
        decl##_node_t *nodes;                                                                                                                         \
        size_t bytes;                                                                                                                                 \
    } decl##_snap_t;                                                                                                                                  \
                                                                                                                                                      \
    /* Path taken to find an element in the Skiplist, path[0] is the match if                                                                         \
       one exists */                                                                                                                                  \
    struct __##decl##_path {                                                                                                                          \
        size_t cap;                                                                                                                                   \
        size_t len;                                                                                                                                   \
        struct decl##_node **nodes;                                                                                                                   \
    };                                                                                                                                                \
                                                                                                                                                      \
    typedef enum { SKIP_EQ = 0, SKIP_LTE = -1, SKIP_LT = -2, SKIP_GTE = 1, SKIP_GT = 2 } skip_pos_##decl_t;                                           \
                                                                                                                                                      \
    /* -- __skip_key_compare_                                                                                                                         \
     *                                                                                                                                                \
     * This function takes four arguments:                                                                                                            \
     *   - a reference to the Skiplist                                                                                                                \
     *   - the two nodes to compare, `a` and `b`                                                                                                      \
     *   - `aux` an additional auxiliary argument                                                                                                     \
     * and returns:                                                                                                                                   \
     *   a  < b : return -1                                                                                                                           \
     *   a == b : return 0                                                                                                                            \
     *   a  > b : return 1                                                                                                                            \
     */                                                                                                                                               \
    static int __skip_key_compare_##decl(decl##_t *slist, decl##_node_t *a, decl##_node_t *b, void *aux)                                              \
    {                                                                                                                                                 \
        if (a == b)                                                                                                                                   \
            return 0;                                                                                                                                 \
        if (a == slist->slh_head || b == slist->slh_tail)                                                                                             \
            return -1;                                                                                                                                \
        if (a == slist->slh_tail || b == slist->slh_head)                                                                                             \
            return 1;                                                                                                                                 \
        return slist->cmp(slist, a, b, aux);                                                                                                          \
    }                                                                                                                                                 \
                                                                                                                                                      \
    /* -- __skip_toss_                                                                                                                                \
     * A "coin toss" function that is critical to the proper operation of the                                                                         \
     * Skiplist.  For example, when `max = 6` this function returns 0 with                                                                            \
     * probability 0.5, 1 with 0.25, 2 with 0.125, etc. until 6 with 0.5^7.                                                                           \
     */                                                                                                                                               \
    static int __skip_toss_##decl(size_t max)                                                                                                         \
    {                                                                                                                                                 \
        size_t level = 0;                                                                                                                             \
        double probability = 0.5;                                                                                                                     \
                                                                                                                                                      \
        double random_value = (double)rand() / RAND_MAX; /* NOLINT(*-msc50-cpp) */                                                                    \
        while (random_value < probability && level < max) {                                                                                           \
            level++;                                                                                                                                  \
            probability *= 0.5;                                                                                                                       \
        }                                                                                                                                             \
        return level;                                                                                                                                 \
    }                                                                                                                                                 \
                                                                                                                                                      \
    /* -- skip_alloc_node_ */                                                                                                                         \
    int prefix##skip_alloc_node_##decl(decl##_t *slist, decl##_node_t **node)                                                                         \
    {                                                                                                                                                 \
        decl##_node_t *n;                                                                                                                             \
        /* Calculate the size of the struct sle within decl##_node_t, multiply                                                                        \
           by array size. (16/24 bytes on 32/64 bit systems) */                                                                                       \
        size_t sle_arr_sz = sizeof(struct __skiplist_idx) * slist->max;                                                                               \
        n = (decl##_node_t *)calloc(1, sizeof(decl##_node_t) + sle_arr_sz);                                                                           \
        if (n == NULL)                                                                                                                                \
            return ENOMEM;                                                                                                                            \
        n->field.sle.cap = slist->max;                                                                                                                \
        n->field.sle.len = 0;                                                                                                                         \
        n->field.sle.next = (decl##_node_t **)((uintptr_t)n + sizeof(decl##_node_t));                                                                 \
        *node = n;                                                                                                                                    \
        return 0;                                                                                                                                     \
    }                                                                                                                                                 \
                                                                                                                                                      \
    /* -- skip_init_                                                                                                                                  \
     *                                                                                                                                                \
     */                                                                                                                                               \
    int prefix##skip_init_##decl(decl##_t *slist, int max, int (*cmp)(struct decl *, decl##_node_t *, decl##_node_t *, void *))                       \
    {                                                                                                                                                 \
        int rc = 0;                                                                                                                                   \
        size_t i;                                                                                                                                     \
                                                                                                                                                      \
        slist->length = 0;                                                                                                                            \
        slist->max = (size_t)(max < 0 ? -max : max);                                                                                                  \
        slist->max = SKIPLIST_MAX_HEIGHT == 1 ? slist->max : SKIPLIST_MAX_HEIGHT;                                                                     \
        if (SKIPLIST_MAX_HEIGHT > 1 && slist->max > SKIPLIST_MAX_HEIGHT)                                                                              \
            return -1;                                                                                                                                \
        slist->cmp = cmp;                                                                                                                             \
        rc = prefix##skip_alloc_node_##decl(slist, &slist->slh_head);                                                                                 \
        if (rc)                                                                                                                                       \
            goto fail;                                                                                                                                \
        rc = prefix##skip_alloc_node_##decl(slist, &slist->slh_tail);                                                                                 \
        if (rc)                                                                                                                                       \
            goto fail;                                                                                                                                \
                                                                                                                                                      \
        slist->slh_head->field.sle.len = 0;                                                                                                           \
        for (i = 0; i < slist->max; i++)                                                                                                              \
            slist->slh_head->field.sle.next[i] = slist->slh_tail;                                                                                     \
        slist->slh_head->field.sle.prev = NULL;                                                                                                       \
                                                                                                                                                      \
        slist->slh_tail->field.sle.len = slist->max;                                                                                                  \
        for (i = 0; i < slist->max; i++)                                                                                                              \
            slist->slh_tail->field.sle.next[i] = NULL;                                                                                                \
        slist->slh_tail->field.sle.prev = slist->slh_head;                                                                                            \
                                                                                                                                                      \
        /* NOTE: Here's a testing aid, simply set `max` to a negative number to                                                                       \
         * seed the PRNG in a predictable way and have reproducible random numbers.                                                                   \
         */                                                                                                                                           \
        if (max < 0)                                                                                                                                  \
            srand(-max);                                                                                                                              \
        else                                                                                                                                          \
            srand(((unsigned int)time(NULL) ^ getpid()));                                                                                             \
    fail:;                                                                                                                                            \
        return rc;                                                                                                                                    \
    }                                                                                                                                                 \
                                                                                                                                                      \
    /* -- skip_free_node_  */                                                                                                                         \
    void prefix##skip_free_node_##decl(decl##_node_t *node)                                                                                           \
    {                                                                                                                                                 \
        free_node_blk;                                                                                                                                \
        free(node);                                                                                                                                   \
    }                                                                                                                                                 \
                                                                                                                                                      \
    /* -- skip_size_ */                                                                                                                               \
    int prefix##skip_size_##decl(decl##_t *slist)                                                                                                     \
    {                                                                                                                                                 \
        return slist->length;                                                                                                                         \
    }                                                                                                                                                 \
                                                                                                                                                      \
    /* -- skip_empty_ */                                                                                                                              \
    int prefix##skip_empty_##decl(decl##_t *slist)                                                                                                    \
    {                                                                                                                                                 \
        return slist->length == 0;                                                                                                                    \
    }                                                                                                                                                 \
                                                                                                                                                      \
    /* -- skip_head_ */                                                                                                                               \
    decl##_node_t *prefix##skip_head_##decl(decl##_t *slist)                                                                                          \
    {                                                                                                                                                 \
        return slist->slh_head->field.sle.next[0] == slist->slh_tail ? NULL : slist->slh_head->field.sle.next[0];                                     \
    }                                                                                                                                                 \
                                                                                                                                                      \
    /* -- skip_tail_ */                                                                                                                               \
    decl##_node_t *prefix##skip_tail_##decl(decl##_t *slist)                                                                                          \
    {                                                                                                                                                 \
        return slist->slh_tail->field.sle.prev == slist->slh_head->field.sle.next[0] ? NULL : slist->slh_tail->field.sle.prev;                        \
    }                                                                                                                                                 \
                                                                                                                                                      \
    /* -- __skip_locate_                                                                                                                              \
     * Locates a node that matches another node updating `path` and then                                                                              \
     * returning the length of that path + 1 to the node and the matching                                                                             \
     * node in path[0], or NULL at path[0] where there wasn't a match.                                                                                \
     * sizeof(path) should be `slist->max + 1`                                                                                                        \
     */                                                                                                                                               \
    static size_t __skip_locate_##decl(decl##_t *slist, decl##_node_t *n, decl##_node_t **path)                                                       \
    {                                                                                                                                                 \
        unsigned int i;                                                                                                                               \
        size_t len = 0;                                                                                                                               \
        decl##_node_t *elm = slist->slh_head;                                                                                                         \
                                                                                                                                                      \
        if (slist == NULL || n == NULL)                                                                                                               \
            return 0;                                                                                                                                 \
                                                                                                                                                      \
        /* Find the node that matches `node` or NULL. */                                                                                              \
        i = slist->slh_head->field.sle.len;                                                                                                           \
        do {                                                                                                                                          \
            while (elm && __skip_key_compare_##decl(slist, elm->field.sle.next[i], n, slist->aux) < 0)                                                \
                elm = elm->field.sle.next[i];                                                                                                         \
            path[i + 1] = elm;                                                                                                                        \
            len++;                                                                                                                                    \
        } while (i--);                                                                                                                                \
        elm = elm->field.sle.next[0];                                                                                                                 \
        if (__skip_key_compare_##decl(slist, elm, n, slist->aux) == 0) {                                                                              \
            path[0] = elm;                                                                                                                            \
        }                                                                                                                                             \
        return len;                                                                                                                                   \
    }                                                                                                                                                 \
                                                                                                                                                      \
    /* -- __skip_insert_ */                                                                                                                           \
    static int __skip_insert_##decl(decl##_t *slist, decl##_node_t *n, int flags)                                                                     \
    {                                                                                                                                                 \
        static decl##_node_t apath[SKIPLIST_MAX_HEIGHT + 1];                                                                                          \
        size_t i, len, level;                                                                                                                         \
        decl##_node_t *node, **path = (decl##_node_t **)&apath;                                                                                       \
                                                                                                                                                      \
        if (slist == NULL || n == NULL)                                                                                                               \
            return ENOENT;                                                                                                                            \
                                                                                                                                                      \
        /* Allocate a buffer */                                                                                                                       \
        if (SKIPLIST_MAX_HEIGHT == 1) {                                                                                                               \
            path = malloc(sizeof(decl##_node_t *) * slist->max + 1);                                                                                  \
            if (path == NULL)                                                                                                                         \
                return ENOMEM;                                                                                                                        \
        }                                                                                                                                             \
                                                                                                                                                      \
        len = __skip_locate_##decl(slist, n, path);                                                                                                   \
        node = path[0];                                                                                                                               \
        if (len > 0) {                                                                                                                                \
            if ((node != NULL) && (flags == 0)) {                                                                                                     \
                /* Don't insert, duplicate flag not set. */                                                                                           \
                return -1;                                                                                                                            \
            }                                                                                                                                         \
            level = __skip_toss_##decl(slist->max - 1);                                                                                               \
            n->field.sle.len = level + 1;                                                                                                             \
            for (i = 0; i <= level; i++) {                                                                                                            \
                if (i <= slist->slh_head->field.sle.len) {                                                                                            \
                    n->field.sle.next[i] = path[i + 1]->field.sle.next[i];                                                                            \
                    path[i + 1]->field.sle.next[i] = n;                                                                                               \
                } else {                                                                                                                              \
                    n->field.sle.next[i] = slist->slh_tail;                                                                                           \
                }                                                                                                                                     \
            }                                                                                                                                         \
            n->field.sle.prev = path[1];                                                                                                              \
            if (n->field.sle.next[0] == slist->slh_tail) {                                                                                            \
                slist->slh_tail->field.sle.prev = n;                                                                                                  \
            }                                                                                                                                         \
            if (level > slist->slh_head->field.sle.len) {                                                                                             \
                slist->slh_head->field.sle.len = level;                                                                                               \
                slist->slh_tail->field.sle.len = level;                                                                                               \
            }                                                                                                                                         \
            slist->length++;                                                                                                                          \
                                                                                                                                                      \
            if (SKIPLIST_MAX_HEIGHT == 1)                                                                                                             \
                free(path);                                                                                                                           \
        }                                                                                                                                             \
        return 0;                                                                                                                                     \
    }                                                                                                                                                 \
                                                                                                                                                      \
    /* -- skip_insert_ */                                                                                                                             \
    int prefix##skip_insert_##decl(decl##_t *slist, decl##_node_t *n)                                                                                 \
    {                                                                                                                                                 \
        return __skip_insert_##decl(slist, n, 0);                                                                                                     \
    }                                                                                                                                                 \
                                                                                                                                                      \
    /* -- skip_insert_dup_ */                                                                                                                         \
    int prefix##skip_insert_dup_##decl(decl##_t *slist, decl##_node_t *n)                                                                             \
    {                                                                                                                                                 \
        return __skip_insert_##decl(slist, n, 1);                                                                                                     \
    }                                                                                                                                                 \
                                                                                                                                                      \
    /* -- skip_position_eq_                                                                                                                           \
     * Find a node that matches the node `n`.  This differs from the locate()                                                                         \
     * API in that it does not return the path to the node, only the match.                                                                           \
     *                                                                                                                                                \
     * NOTE: This differs from _locate() in that it avoids an alloc/free                                                                              \
     * for the path when SKIPLIST_MAX_HEIGHT == 1.                                                                                                    \
     */                                                                                                                                               \
    decl##_node_t *prefix##skip_position_eq_##decl(decl##_t *slist, decl##_node_t *n)                                                                 \
    {                                                                                                                                                 \
        unsigned int i;                                                                                                                               \
        decl##_node_t *elm = slist->slh_head;                                                                                                         \
                                                                                                                                                      \
        if (slist == NULL || n == NULL)                                                                                                               \
            return NULL;                                                                                                                              \
                                                                                                                                                      \
        i = slist->slh_head->field.sle.len;                                                                                                           \
                                                                                                                                                      \
        do {                                                                                                                                          \
            while (elm && __skip_key_compare_##decl(slist, elm->field.sle.next[i], n, slist->aux) < 0)                                                \
                elm = elm->field.sle.next[i];                                                                                                         \
        } while (i--);                                                                                                                                \
        elm = elm->field.sle.next[0];                                                                                                                 \
        if (__skip_key_compare_##decl(slist, elm, n, slist->aux) == 0) {                                                                              \
            return elm;                                                                                                                               \
        }                                                                                                                                             \
        return NULL;                                                                                                                                  \
    }                                                                                                                                                 \
                                                                                                                                                      \
    /* -- skip_position_gte                                                                                                                           \
     * Position and return a cursor at the first node that is equal to                                                                                \
     * or greater than the provided node `n`, otherwise if the largest                                                                                \
     * key is less than the key in `n` return NULL.                                                                                                   \
     *                                                                                                                                                \
     * NOTE: This differs from _locate() in that it avoids an alloc/free                                                                              \
     * for the path when SKIPLIST_MAX_HEIGHT == 1.                                                                                                    \
     */                                                                                                                                               \
    decl##_node_t *prefix##skip_position_gte_##decl(decl##_t *slist, decl##_node_t *n)                                                                \
    {                                                                                                                                                 \
        int cmp;                                                                                                                                      \
        unsigned int i;                                                                                                                               \
        decl##_node_t *elm = slist->slh_head;                                                                                                         \
                                                                                                                                                      \
        if (slist == NULL || n == NULL)                                                                                                               \
            return NULL;                                                                                                                              \
                                                                                                                                                      \
        i = slist->slh_head->field.sle.len;                                                                                                           \
                                                                                                                                                      \
        do {                                                                                                                                          \
            while (elm && __skip_key_compare_##decl(slist, elm->field.sle.next[i], n, slist->aux) < 0)                                                \
                elm = elm->field.sle.next[i];                                                                                                         \
        } while (i--);                                                                                                                                \
        do {                                                                                                                                          \
            elm = elm->field.sle.next[0];                                                                                                             \
            cmp = __skip_key_compare_##decl(slist, elm, n, slist->aux);                                                                               \
        } while (cmp < 0);                                                                                                                            \
        return elm;                                                                                                                                   \
    }                                                                                                                                                 \
                                                                                                                                                      \
    /* -- skip_position_gt TODO                                                                                                                       \
     * Position and return a cursor at the first node that is greater than                                                                            \
     * the provided node `n`. If the largestkey is less than the key in `n`                                                                           \
     * return NULL.                                                                                                                                   \
     *                                                                                                                                                \
     * NOTE: This differs from _locate() in that it avoids an alloc/free                                                                              \
     * for the path when SKIPLIST_MAX_HEIGHT == 1.                                                                                                    \
     */                                                                                                                                               \
    decl##_node_t *prefix##skip_position_gt_##decl(decl##_t *slist, decl##_node_t *n)                                                                 \
    {                                                                                                                                                 \
        int cmp;                                                                                                                                      \
        unsigned int i;                                                                                                                               \
        decl##_node_t *elm = slist->slh_head;                                                                                                         \
                                                                                                                                                      \
        if (slist == NULL || n == NULL)                                                                                                               \
            return NULL;                                                                                                                              \
                                                                                                                                                      \
        i = slist->slh_head->field.sle.len;                                                                                                           \
                                                                                                                                                      \
        do {                                                                                                                                          \
            while (elm && __skip_key_compare_##decl(slist, elm->field.sle.next[i], n, slist->aux) < 0)                                                \
                elm = elm->field.sle.next[i];                                                                                                         \
        } while (i--);                                                                                                                                \
        do {                                                                                                                                          \
            elm = elm->field.sle.next[0];                                                                                                             \
            cmp = __skip_key_compare_##decl(slist, elm, n, slist->aux);                                                                               \
        } while (cmp < 0);                                                                                                                            \
        return elm;                                                                                                                                   \
    }                                                                                                                                                 \
                                                                                                                                                      \
    /* -- skip_position_lte                                                                                                                           \
     * Position and return a cursor at the last node that is less than                                                                                \
     * or equal to node `n`.                                                                                                                          \
     * Return NULL if nothing is less than or equal.                                                                                                  \
     *                                                                                                                                                \
     * NOTE: This differs from _locate() in that it avoids an alloc/free                                                                              \
     * for the path when SKIPLIST_MAX_HEIGHT == 1.                                                                                                    \
     */                                                                                                                                               \
    decl##_node_t *prefix##skip_position_lte_##decl(decl##_t *slist, decl##_node_t *n)                                                                \
    {                                                                                                                                                 \
        int cmp;                                                                                                                                      \
        unsigned int i;                                                                                                                               \
        decl##_node_t *elm = slist->slh_head;                                                                                                         \
                                                                                                                                                      \
        if (slist == NULL || n == NULL)                                                                                                               \
            return NULL;                                                                                                                              \
                                                                                                                                                      \
        i = slist->slh_head->field.sle.len;                                                                                                           \
                                                                                                                                                      \
        do {                                                                                                                                          \
            while (elm && __skip_key_compare_##decl(slist, elm->field.sle.next[i], n, slist->aux) < 0)                                                \
                elm = elm->field.sle.next[i];                                                                                                         \
        } while (i--);                                                                                                                                \
        elm = elm->field.sle.next[0];                                                                                                                 \
        if (__skip_key_compare_##decl(slist, elm, n, slist->aux) == 0) {                                                                              \
            return elm;                                                                                                                               \
        } else {                                                                                                                                      \
            do {                                                                                                                                      \
                elm = elm->field.sle.prev;                                                                                                            \
                cmp = __skip_key_compare_##decl(slist, elm, n, slist->aux);                                                                           \
            } while (cmp > 0);                                                                                                                        \
        }                                                                                                                                             \
        return elm;                                                                                                                                   \
    }                                                                                                                                                 \
                                                                                                                                                      \
    /* -- skip_position_lt TODO                                                                                                                       \
     * Position and return a cursor at the last node that is less than                                                                                \
     * to the node `n`. Return NULL if nothing is less than or equal.                                                                                 \
     *                                                                                                                                                \
     * NOTE: This differs from _locate() in that it avoids an alloc/free                                                                              \
     * for the path when SKIPLIST_MAX_HEIGHT == 1.                                                                                                    \
     */                                                                                                                                               \
    decl##_node_t *prefix##skip_position_lt_##decl(decl##_t *slist, decl##_node_t *n)                                                                 \
    {                                                                                                                                                 \
        int cmp;                                                                                                                                      \
        unsigned int i;                                                                                                                               \
        decl##_node_t *elm = slist->slh_head;                                                                                                         \
                                                                                                                                                      \
        if (slist == NULL || n == NULL)                                                                                                               \
            return NULL;                                                                                                                              \
                                                                                                                                                      \
        i = slist->slh_head->field.sle.len;                                                                                                           \
                                                                                                                                                      \
        do {                                                                                                                                          \
            while (elm && __skip_key_compare_##decl(slist, elm->field.sle.next[i], n, slist->aux) < 0)                                                \
                elm = elm->field.sle.next[i];                                                                                                         \
        } while (i--);                                                                                                                                \
        elm = elm->field.sle.next[0];                                                                                                                 \
        if (__skip_key_compare_##decl(slist, elm, n, slist->aux) == 0) {                                                                              \
            return elm;                                                                                                                               \
        } else {                                                                                                                                      \
            do {                                                                                                                                      \
                elm = elm->field.sle.prev;                                                                                                            \
                cmp = __skip_key_compare_##decl(slist, elm, n, slist->aux);                                                                           \
            } while (cmp > 0);                                                                                                                        \
        }                                                                                                                                             \
        return elm;                                                                                                                                   \
    }                                                                                                                                                 \
                                                                                                                                                      \
    /* -- skip_position_                                                                                                                              \
     * Position a cursor relative to `n`.                                                                                                             \
     * This avoids an alloc/free for the path when SKIPLIST_MAX_HEIGHT == 1.                                                                          \
     */                                                                                                                                               \
    decl##_node_t *prefix##skip_position_##decl(decl##_t *slist, skip_pos_##decl_t op, decl##_node_t *n)                                              \
    {                                                                                                                                                 \
        decl##_node_t *node;                                                                                                                          \
                                                                                                                                                      \
        switch (op) {                                                                                                                                 \
        case (SKIP_LT):                                                                                                                               \
            node = prefix##skip_position_lt_##decl(slist, n);                                                                                         \
            break;                                                                                                                                    \
        case (SKIP_LTE):                                                                                                                              \
            node = prefix##skip_position_lte_##decl(slist, n);                                                                                        \
            break;                                                                                                                                    \
        case (SKIP_GTE):                                                                                                                              \
            node = prefix##skip_position_gte_##decl(slist, n);                                                                                        \
            break;                                                                                                                                    \
        case (SKIP_GT):                                                                                                                               \
            node = prefix##skip_position_gt_##decl(slist, n);                                                                                         \
            break;                                                                                                                                    \
        default:                                                                                                                                      \
        case (SKIP_EQ):                                                                                                                               \
            node = prefix##skip_position_eq_##decl(slist, n);                                                                                         \
            break;                                                                                                                                    \
        }                                                                                                                                             \
        return node;                                                                                                                                  \
    }                                                                                                                                                 \
                                                                                                                                                      \
    /* -- skip_update_                                                                                                                                \
     * Locates a node in the list that equals the `new` node and then                                                                                 \
     * uses the `update_node_blk` to update the contents.                                                                                             \
     *                                                                                                                                                \
     *                                                                                                                                                \
     * WARNING: Do not update the portion of the node used for ordering                                                                               \
     * (e.g. `key`) unless you really know what you're doing.                                                                                         \
     */                                                                                                                                               \
    int prefix##skip_update_##decl(decl##_t *slist, decl##_node_t *new)                                                                               \
    {                                                                                                                                                 \
        decl##_node_t *node;                                                                                                                          \
                                                                                                                                                      \
        if (slist == NULL || new == NULL)                                                                                                             \
            return -1;                                                                                                                                \
                                                                                                                                                      \
        node = prefix##skip_position_eq_##decl(slist, new);                                                                                           \
        if (node) {                                                                                                                                   \
            update_node_blk;                                                                                                                          \
            return 0;                                                                                                                                 \
        }                                                                                                                                             \
        return -1;                                                                                                                                    \
    }                                                                                                                                                 \
                                                                                                                                                      \
    /* -- skip_remove_ */                                                                                                                             \
    int prefix##skip_remove_##decl(decl##_t *slist, decl##_node_t *n)                                                                                 \
    {                                                                                                                                                 \
        static decl##_node_t apath[SKIPLIST_MAX_HEIGHT + 1];                                                                                          \
        size_t i, len, level;                                                                                                                         \
        decl##_node_t *node, **path = (decl##_node_t **)&apath;                                                                                       \
                                                                                                                                                      \
        if (slist == NULL || n == NULL)                                                                                                               \
            return -1;                                                                                                                                \
        if (slist->length == 0)                                                                                                                       \
            return 0;                                                                                                                                 \
                                                                                                                                                      \
        /* Allocate a buffer */                                                                                                                       \
        if (SKIPLIST_MAX_HEIGHT == 1) {                                                                                                               \
            path = malloc(sizeof(decl##_node_t *) * slist->max + 1);                                                                                  \
            if (path == NULL)                                                                                                                         \
                return ENOMEM;                                                                                                                        \
        }                                                                                                                                             \
                                                                                                                                                      \
        len = __skip_locate_##decl(slist, n, path);                                                                                                   \
        node = path[0];                                                                                                                               \
        if (node) {                                                                                                                                   \
            node->field.sle.next[0]->field.sle.prev = node->field.sle.prev;                                                                           \
            for (i = 1; i <= len; i++) {                                                                                                              \
                if (path[i]->field.sle.next[i - 1] != node)                                                                                           \
                    break;                                                                                                                            \
                path[i]->field.sle.next[i - 1] = node->field.sle.next[i - 1];                                                                         \
                if (path[i]->field.sle.next[i - 1] == slist->slh_tail) {                                                                              \
                    level = path[i]->field.sle.len;                                                                                                   \
                    path[i]->field.sle.len = level - 1;                                                                                               \
                }                                                                                                                                     \
            }                                                                                                                                         \
            if (SKIPLIST_MAX_HEIGHT == 1)                                                                                                             \
                free(path);                                                                                                                           \
            free_node_blk;                                                                                                                            \
                                                                                                                                                      \
            /* Find all levels in the first element in the list that point                                                                            \
               at the tail and shrink the level. */                                                                                                   \
            i = 0;                                                                                                                                    \
            node = slist->slh_head;                                                                                                                   \
            while (node->field.sle.next[i] != slist->slh_tail && i++ < slist->slh_head->field.sle.len)                                                \
                ;                                                                                                                                     \
            slist->slh_head->field.sle.len = i;                                                                                                       \
            slist->slh_tail->field.sle.len = i;                                                                                                       \
            slist->length--;                                                                                                                          \
        }                                                                                                                                             \
        return 0;                                                                                                                                     \
    }                                                                                                                                                 \
                                                                                                                                                      \
    /* -- skip_next_node_ */                                                                                                                          \
    decl##_node_t *prefix##skip_next_node_##decl(decl##_t *slist, decl##_node_t *n)                                                                   \
    {                                                                                                                                                 \
        if (slist == NULL || n == NULL)                                                                                                               \
            return NULL;                                                                                                                              \
        if (n->field.sle.next[0] == slist->slh_tail)                                                                                                  \
            return NULL;                                                                                                                              \
        return n->field.sle.next[0];                                                                                                                  \
    }                                                                                                                                                 \
                                                                                                                                                      \
    /* -- skip_prev_node_ */                                                                                                                          \
    decl##_node_t *prefix##skip_prev_node_##decl(decl##_t *slist, decl##_node_t *n)                                                                   \
    {                                                                                                                                                 \
        if (slist == NULL || n == NULL)                                                                                                               \
            return NULL;                                                                                                                              \
        if (n->field.sle.prev == slist->slh_head)                                                                                                     \
            return NULL;                                                                                                                              \
        return n->field.sle.prev;                                                                                                                     \
    }                                                                                                                                                 \
                                                                                                                                                      \
    /* -- skip_destroy_ */                                                                                                                            \
    int prefix##skip_destroy_##decl(decl##_t *slist)                                                                                                  \
    {                                                                                                                                                 \
        decl##_node_t *node, *next;                                                                                                                   \
                                                                                                                                                      \
        if (slist == NULL)                                                                                                                            \
            return 0;                                                                                                                                 \
        if (prefix##skip_size_##decl(slist) == 0)                                                                                                     \
            return 0;                                                                                                                                 \
        node = prefix##skip_head_##decl(slist);                                                                                                       \
        do {                                                                                                                                          \
            next = prefix##skip_next_node_##decl(slist, node);                                                                                        \
            prefix##skip_free_node_##decl(node);                                                                                                      \
            node = next;                                                                                                                              \
        } while (node != NULL);                                                                                                                       \
                                                                                                                                                      \
        free(slist->slh_head);                                                                                                                        \
        free(slist->slh_tail);                                                                                                                        \
        return 0;                                                                                                                                     \
    }                                                                                                                                                 \
                                                                                                                                                      \
    /* -- skip_snapshot_ TODO/WIP                                                                                                                     \
     * A snapshot is a read-only view of a Skiplist at a point in                                                                                     \
     * time.  Once taken, a snapshot must be restored or disposed.                                                                                    \
     * Any number of snapshots can be created.                                                                                                        \
     *                                                                                                                                                \
     * NOTE: There are many fancy algorithms for this, for now                                                                                        \
     * this implementation will simply create a copy of the nodes.                                                                                    \
     */                                                                                                                                               \
    decl##_snap_t *prefix##skip_snapshot_##decl(decl##_t *slist)                                                                                      \
    {                                                                                                                                                 \
        size_t bytes, i;                                                                                                                              \
        decl##_snap_t *snap;                                                                                                                          \
        decl##_node_t *node, *new;                                                                                                                    \
                                                                                                                                                      \
        if (slist == NULL)                                                                                                                            \
            return 0;                                                                                                                                 \
                                                                                                                                                      \
        bytes = sizeof(decl##_snap_t) + (slist->length * sizeof(decl##_node_t));                                                                      \
        snap = (decl##_snap_t *)calloc(1, bytes);                                                                                                     \
        if (snap == NULL)                                                                                                                             \
            return NULL;                                                                                                                              \
                                                                                                                                                      \
        snap->bytes = bytes;                                                                                                                          \
        snap->list.length = slist->length;                                                                                                            \
        snap->list.max = slist->max;                                                                                                                  \
        snap->nodes = (decl##_node_t *)(snap + sizeof(decl##_snap_t));                                                                                \
                                                                                                                                                      \
        i = 0;                                                                                                                                        \
        node = prefix##skip_head_##decl(slist);                                                                                                       \
        while (node) {                                                                                                                                \
            decl##_node_t *n = (decl##_node_t *)snap->nodes + (i++ * sizeof(decl##_node_t));                                                          \
            new = (decl##_node_t *)&n;                                                                                                                \
            snap_node_blk;                                                                                                                            \
            node = prefix##skip_next_node_##decl(slist, node);                                                                                        \
        }                                                                                                                                             \
        return snap;                                                                                                                                  \
    }                                                                                                                                                 \
                                                                                                                                                      \
    /* -- skip_restore_snapshot_ TODO/WIP */                                                                                                          \
    decl##_t *prefix##skip_restore_snapshot_##decl(decl##_snap_t *snap, int (*cmp)(decl##_t * head, decl##_node_t * a, decl##_node_t * b, void *aux)) \
    {                                                                                                                                                 \
        int rc;                                                                                                                                       \
        size_t i;                                                                                                                                     \
        decl##_t *slist;                                                                                                                              \
        decl##_node_t *node, *new;                                                                                                                    \
                                                                                                                                                      \
        if (snap == NULL || cmp == NULL)                                                                                                              \
            return 0;                                                                                                                                 \
        slist = (decl##_t *)calloc(1, sizeof(decl##_t));                                                                                              \
        if (slist == NULL)                                                                                                                            \
            return NULL;                                                                                                                              \
                                                                                                                                                      \
        slist->cmp = cmp;                                                                                                                             \
        slist->max = snap->list.max;                                                                                                                  \
                                                                                                                                                      \
        rc = prefix##skip_alloc_node_##decl(slist, &slist->slh_head);                                                                                 \
        if (rc)                                                                                                                                       \
            goto fail;                                                                                                                                \
        rc = prefix##skip_alloc_node_##decl(slist, &slist->slh_tail);                                                                                 \
        if (rc)                                                                                                                                       \
            goto fail;                                                                                                                                \
                                                                                                                                                      \
        slist->slh_head->field.sle.len = 0;                                                                                                           \
        for (i = 0; i < slist->max; i++)                                                                                                              \
            slist->slh_head->field.sle.next[i] = slist->slh_tail;                                                                                     \
        slist->slh_head->field.sle.prev = NULL;                                                                                                       \
                                                                                                                                                      \
        slist->slh_tail->field.sle.len = slist->max;                                                                                                  \
        for (i = 0; i < slist->max; i++)                                                                                                              \
            slist->slh_tail->field.sle.next[i] = NULL;                                                                                                \
        slist->slh_tail->field.sle.prev = slist->slh_head;                                                                                            \
                                                                                                                                                      \
        i = 0;                                                                                                                                        \
        while (snap->list.length > 0) {                                                                                                               \
            decl##_node_t *n = (decl##_node_t *)snap->nodes + (i++ * sizeof(decl##_node_t));                                                          \
            node = (decl##_node_t *)&n;                                                                                                               \
            rc = prefix##skip_alloc_node_##decl(slist, &new);                                                                                         \
            snap_node_blk;                                                                                                                            \
            __skip_insert_##decl(slist, new, 1);                                                                                                      \
            snap->list.length--;                                                                                                                      \
        }                                                                                                                                             \
        return slist;                                                                                                                                 \
    fail:;                                                                                                                                            \
        if (slist->slh_head)                                                                                                                          \
            free(slist->slh_head);                                                                                                                    \
        if (slist->slh_tail)                                                                                                                          \
            free(slist->slh_tail);                                                                                                                    \
        return NULL;                                                                                                                                  \
    }                                                                                                                                                 \
                                                                                                                                                      \
    /* -- skip_dispose_snapshot_ TODO/WIP */                                                                                                          \
    void prefix##skip_dispose_snapshot_##decl(decl##_snap_t *snap)                                                                                    \
    {                                                                                                                                                 \
        decl##_node_t *node;                                                                                                                          \
                                                                                                                                                      \
        node = (decl##_node_t *)snap->nodes;                                                                                                          \
        while (snap->list.length > 0) {                                                                                                               \
            free_node_blk;                                                                                                                            \
            node += sizeof(decl##_node_t);                                                                                                            \
            snap->list.length--;                                                                                                                      \
        }                                                                                                                                             \
        free(snap);                                                                                                                                   \
    }                                                                                                                                                 \
                                                                                                                                                      \
    /* -- skip_serialize_ TODO/WIP                                                                                                                    \
     * Similar to snapshot, but includes the values and encodes them                                                                                  \
     * in a portable manner.                                                                                                                          \
     */                                                                                                                                               \
    decl##_snap_t *prefix##skip_serialize_##decl(decl##_t *slist)                                                                                     \
    {                                                                                                                                                 \
        size_t size, bytes, i;                                                                                                                        \
        decl##_snap_t *snap;                                                                                                                          \
        decl##_node_t *node, *new;                                                                                                                    \
                                                                                                                                                      \
        if (slist == NULL)                                                                                                                            \
            return 0;                                                                                                                                 \
                                                                                                                                                      \
        bytes = sizeof(decl##_snap_t) + (slist->length * sizeof(decl##_node_t));                                                                      \
        node = prefix##skip_head_##decl(slist);                                                                                                       \
        while (node) {                                                                                                                                \
            sizeof_entry_blk;                                                                                                                         \
            bytes += sizeof(size_t);                                                                                                                  \
            bytes += size;                                                                                                                            \
            node = prefix##skip_next_node_##decl(slist, node);                                                                                        \
        }                                                                                                                                             \
        snap = (decl##_snap_t *)calloc(1, bytes);                                                                                                     \
        if (snap == NULL)                                                                                                                             \
            return NULL;                                                                                                                              \
                                                                                                                                                      \
        snap->bytes = bytes;                                                                                                                          \
        snap->list.length = slist->length;                                                                                                            \
        snap->list.max = slist->max;                                                                                                                  \
        snap->nodes = (decl##_node_t *)(snap + sizeof(decl##_snap_t));                                                                                \
                                                                                                                                                      \
        i = 0;                                                                                                                                        \
        node = prefix##skip_head_##decl(slist);                                                                                                       \
        while (node) {                                                                                                                                \
            decl##_node_t *n = (decl##_node_t *)snap->nodes + (i++ * sizeof(decl##_node_t));                                                          \
            new = (decl##_node_t *)&n;                                                                                                                \
            snap_node_blk;                                                                                                                            \
            node = prefix##skip_next_node_##decl(slist, node);                                                                                        \
        }                                                                                                                                             \
        return snap;                                                                                                                                  \
    }                                                                                                                                                 \
                                                                                                                                                      \
    /* -- skip_deserialize_snapshot_ TODO/WIP */                                                                                                      \
    decl##_t *prefix##skip_deserialize_##decl(decl##_snap_t *snap, int (*cmp)(decl##_t * head, decl##_node_t * a, decl##_node_t * b, void *aux))      \
    {                                                                                                                                                 \
        int rc;                                                                                                                                       \
        size_t i;                                                                                                                                     \
        decl##_t *slist;                                                                                                                              \
        decl##_node_t *node, *new;                                                                                                                    \
                                                                                                                                                      \
        if (snap == NULL || cmp == NULL)                                                                                                              \
            return 0;                                                                                                                                 \
        slist = (decl##_t *)calloc(1, sizeof(decl##_t));                                                                                              \
        if (slist == NULL)                                                                                                                            \
            return NULL;                                                                                                                              \
                                                                                                                                                      \
        slist->cmp = cmp;                                                                                                                             \
        slist->max = snap->list.max;                                                                                                                  \
                                                                                                                                                      \
        rc = prefix##skip_alloc_node_##decl(slist, &slist->slh_head);                                                                                 \
        if (rc)                                                                                                                                       \
            goto fail;                                                                                                                                \
        rc = prefix##skip_alloc_node_##decl(slist, &slist->slh_tail);                                                                                 \
        if (rc)                                                                                                                                       \
            goto fail;                                                                                                                                \
                                                                                                                                                      \
        slist->slh_head->field.sle.len = 0;                                                                                                           \
        for (i = 0; i < slist->max; i++)                                                                                                              \
            slist->slh_head->field.sle.next[i] = slist->slh_tail;                                                                                     \
        slist->slh_head->field.sle.prev = NULL;                                                                                                       \
                                                                                                                                                      \
        slist->slh_tail->field.sle.len = slist->max;                                                                                                  \
        for (i = 0; i < slist->max; i++)                                                                                                              \
            slist->slh_tail->field.sle.next[i] = NULL;                                                                                                \
        slist->slh_tail->field.sle.prev = slist->slh_head;                                                                                            \
                                                                                                                                                      \
        i = 0;                                                                                                                                        \
        while (snap->list.length > 0) {                                                                                                               \
            decl##_node_t *n = (decl##_node_t *)snap->nodes + (i++ * sizeof(decl##_node_t));                                                          \
            node = (decl##_node_t *)&n;                                                                                                               \
            rc = prefix##skip_alloc_node_##decl(slist, &new);                                                                                         \
            snap_node_blk;                                                                                                                            \
            __skip_insert_##decl(slist, new, 1);                                                                                                      \
            snap->list.length--;                                                                                                                      \
        }                                                                                                                                             \
        return slist;                                                                                                                                 \
    fail:;                                                                                                                                            \
        if (slist->slh_head)                                                                                                                          \
            free(slist->slh_head);                                                                                                                    \
        if (slist->slh_tail)                                                                                                                          \
            free(slist->slh_tail);                                                                                                                    \
        return NULL;                                                                                                                                  \
    }                                                                                                                                                 \
                                                                                                                                                      \
    /* -- __skip_integrity_check_ TODO */                                                                                                             \
    static int __skip_integrity_check_##decl(decl##_t *slist)                                                                                         \
    {                                                                                                                                                 \
        ((void)slist);                                                                                                                                \
        return 0;                                                                                                                                     \
    }

#define SKIPLIST_KV_ACCESS(decl, prefix, ktype, vtype, qblk, rblk)                           \
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
    int prefix##skip_put_##decl(decl##_t *slist, ktype key, vtype value)                     \
    {                                                                                        \
        int rc;                                                                              \
        decl##_node_t *node;                                                                 \
        rc = prefix##skip_alloc_node_##decl(slist, &node);                                   \
        if (rc)                                                                              \
            return rc;                                                                       \
        node->key = key;                                                                     \
        node->value = value;                                                                 \
        rc = prefix##skip_insert_##decl(slist, node);                                        \
        if (rc)                                                                              \
            prefix##skip_free_node_##decl(node);                                             \
        return rc;                                                                           \
    }                                                                                        \
                                                                                             \
    int prefix##skip_set_##decl(decl##_t *slist, ktype key, vtype value)                     \
    {                                                                                        \
        decl##_node_t node;                                                                  \
        node.key = key;                                                                      \
        node.value = value;                                                                  \
        return prefix##skip_update_##decl(slist, &node);                                     \
    }                                                                                        \
                                                                                             \
    int prefix##skip_del_##decl(decl##_t *slist, ktype key)                                  \
    {                                                                                        \
        decl##_node_t node;                                                                  \
        node.key = key;                                                                      \
        return prefix##skip_remove_##decl(slist, &node);                                     \
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
    static void __skip_dot_node_##decl(FILE *os, decl##_t *slist, decl##_node_t *node, size_t nsg, skip_sprintf_node_##decl##_t fn) \
    {                                                                                                                               \
        char buf[2048];                                                                                                             \
        size_t level, height = node->field.sle.len;                                                                                 \
        decl##_node_t *next;                                                                                                        \
                                                                                                                                    \
        fprintf(os, "\"node%zu %p\"", nsg, (void *)node);                                                                           \
        fprintf(os, " [label = \"");                                                                                                \
        level = height;                                                                                                             \
        while (level--) {                                                                                                           \
            fprintf(os, " { <w%zu> | <f%zu> %p } |", level, level, (void *)node->field.sle.next[level]);                            \
        }                                                                                                                           \
        if (fn) {                                                                                                                   \
            fn(node, buf);                                                                                                          \
            fprintf(os, " <f0> %s\"\n", buf);                                                                                       \
        } else {                                                                                                                    \
            fprintf(os, " <f0> ?\"\n");                                                                                             \
        }                                                                                                                           \
        fprintf(os, "shape = \"record\"\n");                                                                                        \
        fprintf(os, "];\n");                                                                                                        \
                                                                                                                                    \
        /* Now edges */                                                                                                             \
        level = 0;                                                                                                                  \
        for (level = 0; level < height; level++) {                                                                                  \
            fprintf(os, "\"node%zu %p\"", nsg, (void *)node);                                                                       \
            fprintf(os, ":f%zu -> ", level);                                                                                        \
            fprintf(os, "\"node%zu %p\"", nsg, (void *)node->field.sle.next[level]);                                                \
            fprintf(os, ":w%zu [];\n", level);                                                                                      \
        }                                                                                                                           \
        next = prefix##skip_next_node_##decl(slist, node);                                                                          \
        if (next)                                                                                                                   \
            __skip_dot_node_##decl(os, slist, next, nsg, fn);                                                                       \
    }                                                                                                                               \
                                                                                                                                    \
    /* -- __skip_dot_finish_                                                                                                        \
     * Finalize the DOT file of the internal representation.                                                                        \
     */                                                                                                                             \
    static void __skip_dot_finish_##decl(FILE *os, size_t nsg)                                                                      \
    {                                                                                                                               \
        size_t i;                                                                                                                   \
        if (nsg > 0) {                                                                                                              \
            /* Link the nodes together with an invisible node.                                                                      \
             *    node0 [shape=record, label = "<f0> | <f1> | <f2> | <f3> |                                                         \
             * <f4> | <f5> | <f6> | <f7> | <f8> | ", style=invis, width=0.01];                                                      \
             */                                                                                                                     \
            fprintf(os, "node0 [shape=record, label = \"");                                                                         \
            for (i = 0; i < nsg; ++i) {                                                                                             \
                fprintf(os, "<f%zu> | ", i);                                                                                        \
            }                                                                                                                       \
            fprintf(os, "\", style=invis, width=0.01];\n");                                                                         \
                                                                                                                                    \
            /* Now connect nodes with invisible edges                                                                               \
             *                                                                                                                      \
             *    node0:f0 -> HeadNode [style=invis];                                                                               \
             *    node0:f1 -> HeadNode1 [style=invis];                                                                              \
             */                                                                                                                     \
            for (i = 0; i < nsg; ++i) {                                                                                             \
                fprintf(os, "node0:f%zu -> HeadNode%zu [style=invis];\n", i, i);                                                    \
            }                                                                                                                       \
            nsg = 0;                                                                                                                \
        }                                                                                                                           \
        fprintf(os, "}\n");                                                                                                         \
    }                                                                                                                               \
                                                                                                                                    \
    /* -- skip_dot_start_ */                                                                                                        \
    static int __skip_dot_start_##decl(FILE *os, decl##_t *slist, size_t nsg, skip_sprintf_node_##decl##_t fn)                      \
    {                                                                                                                               \
        size_t level;                                                                                                               \
        decl##_node_t *head, *tail;                                                                                                 \
        if (nsg == 0) {                                                                                                             \
            fprintf(os, "digraph Skiplist {\n");                                                                                    \
            fprintf(os, "label = \"Skiplist.\"\n");                                                                                 \
            fprintf(os, "graph [rankdir = \"LR\"];\n");                                                                             \
            fprintf(os, "node [fontsize = \"12\" shape = \"ellipse\"];\n");                                                         \
            fprintf(os, "edge [];\n\n");                                                                                            \
        }                                                                                                                           \
        fprintf(os, "subgraph cluster%zu {\n", nsg);                                                                                \
        fprintf(os, "style=dashed\n");                                                                                              \
        fprintf(os, "label=\"Skip list iteration %zu\"\n\n", nsg);                                                                  \
        fprintf(os, "\"HeadNode%zu\" [\n", nsg);                                                                                    \
        fprintf(os, "label = \"");                                                                                                  \
                                                                                                                                    \
        /* Write out the fields */                                                                                                  \
        head = slist->slh_head;                                                                                                     \
        if (SKIP_EMPTY(slist))                                                                                                      \
            fprintf(os, "Empty HeadNode");                                                                                          \
        else {                                                                                                                      \
            level = head->field.sle.len - 1;                                                                                        \
            do {                                                                                                                    \
                decl##_node_t *node = head->field.sle.next[level];                                                                  \
                fprintf(os, "{ <f%zu> %p }", level, (void *)node);                                                                  \
                if (level && head->field.sle.next[level] != slist->slh_tail)                                                        \
                    fprintf(os, " | ");                                                                                             \
            } while (level-- && head->field.sle.next[level] != slist->slh_tail);                                                    \
        }                                                                                                                           \
        fprintf(os, "\"\n");                                                                                                        \
        fprintf(os, "shape = \"record\"\n");                                                                                        \
        fprintf(os, "];\n");                                                                                                        \
                                                                                                                                    \
        /* Edges for head node */                                                                                                   \
        decl##_node_t *node = slist->slh_head;                                                                                      \
        for (level = 0; level < slist->slh_head->field.sle.len; level++) {                                                          \
            if (node->field.sle.next[level] == slist->slh_tail)                                                                     \
                break;                                                                                                              \
            fprintf(os, "\"HeadNode%zu\":f%zu -> ", nsg, level);                                                                    \
            fprintf(os, "\"node%zu %p\"", nsg, (void *)node->field.sle.next[level]);                                                \
            fprintf(os, ":w%zu [];\n", level);                                                                                      \
        }                                                                                                                           \
        fprintf(os, "\n");                                                                                                          \
                                                                                                                                    \
        /* Now all nodes via level 0, if non-empty */                                                                               \
        node = prefix##skip_head_##decl(slist);                                                                                     \
        if (node)                                                                                                                   \
            __skip_dot_node_##decl(os, slist, node, nsg, fn);                                                                       \
        fprintf(os, "\n");                                                                                                          \
                                                                                                                                    \
        /* The tail, sentinal node */                                                                                               \
        tail = slist->slh_tail;                                                                                                     \
        if (!SKIP_EMPTY(slist)) {                                                                                                   \
            fprintf(os, "\"node%zu %p\" [label = \"", nsg, (void *)slist->slh_tail);                                                \
            level = tail->field.sle.len - 1;                                                                                        \
            do {                                                                                                                    \
                fprintf(os, "<w%zu> %p", level, (void *)node->field.sle.prev);                                                      \
                if (level && node->field.sle.prev != slist->slh_head)                                                               \
                    fprintf(os, " | ");                                                                                             \
            } while (level-- && node->field.sle.prev != slist->slh_head);                                                           \
            fprintf(os, "\" shape = \"record\"];\n");                                                                               \
        }                                                                                                                           \
                                                                                                                                    \
        /* End: "subgraph cluster0 {" */                                                                                            \
        fprintf(os, "}\n\n");                                                                                                       \
        nsg += 1;                                                                                                                   \
                                                                                                                                    \
        return nsg;                                                                                                                 \
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
    int prefix##skip_dot_##decl(FILE *os, decl##_t *slist, size_t nsg, skip_sprintf_node_##decl##_t fn)                             \
    {                                                                                                                               \
        if (__skip_integrity_check_##decl(slist) != 0) {                                                                            \
            perror("Skiplist failed integrity checks, impossible to diagram.");                                                     \
            return -1;                                                                                                              \
        }                                                                                                                           \
        if (os == NULL)                                                                                                             \
            os = stdout;                                                                                                            \
        if (!os) {                                                                                                                  \
            perror("Failed to open output file, unable to write DOT file.");                                                        \
            return -1;                                                                                                              \
        }                                                                                                                           \
        __skip_dot_start_##decl(os, slist, nsg, fn);                                                                                \
        __skip_dot_finish_##decl(os, nsg);                                                                                          \
        return 0;                                                                                                                   \
    }

#endif /* _SKIPLIST_H_ */
