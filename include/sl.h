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
 * Skip List declarations.
 */

#ifndef SKIPLIST_MAX_HEIGHT
#define SKIPLIST_MAX_HEIGHT 1
#endif

#if defined(SKIPLIST_DEBUG)
#ifndef SKIP_DEBUGF
#define SKIP_DEBUGF
#if defined(DEBUG) && DEBUG > 0
#define __skip_debugf(...)                                             \
    do {                                                               \
        fprintf(stderr, "%s:%d:%s(): ", __FILE__, __LINE__, __func__); \
        fprintf(stderr, __VA_ARGS__);                                  \
    } while (0)
#else
#define __skip_debugf(fmt, args...) ((void)0)
#endif
#endif
#endif

/*
 * A Skip List contains elements, a portion of which is used to manage those
 * elements while the rest is defined by the use case for this declaration.  The
 * house keeping portion is the SKIPLIST_ENTRY below.  It maintains the array of
 * forward pointers to nodes and has a height (a zero-based count of levels, so
 * a height of `0` means one (1) level and a height of `4` means five (5)
 * levels).
 */
#define SKIPLIST_ENTRY(type)           \
    struct __skiplist_##decl_entry {   \
        struct __skiplist_##decl_idx { \
            struct type *prev, **next; \
            size_t height, gen;        \
        } sle;                         \
    }

/*
 * Skip List node comparison function.  This macro builds a function used when
 * comparing two nodes for equality.  A portion of this function, `fn_blk`, is
 * the code you supply to compare two nodes as a block (starts with `{`, your
 * code, then `}`) that should compare the node referenced by `a` to the node
 * `b` as follows:
 *
 * When:
 *   *a  < *b : return -1
 *   *a == *b : return 0
 *   *a  > *b : return 1
 *
 * As stated earlier, the ordered skiplist relies on a well-behaved comparison
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
 * Example for nodes with keys that are signed integeters (`int`):
 * {
 *   (void)list;
 *   (void)aux;
 *   if (a->key < b->key)
 *       return -1;
 *   if (a->key > b->key)
 *       return 1;
 *   return 0;
 *  }
 *
 * Note that the comparison function can also make use of the reference to the
 * list as well as a reference to a variable `aux` that you can point to
 * anything else required to perform your comparison.  The auxiliary pointer
 * is unused for other purposes.  You could even use it a pointer to a function
 * that chooses the proper comparison function for the two nodes in question.
 *
 * Example where the value of `decl` below is `ex`:
 * {
 *   // Cast `aux` to a function that returns a function that properly compares
 *   // `a` and `b`, for example if they were objects or different structs.
 *   (skip_ex_cmp_t *(fn)(ex_node_t *, ex_node_t *)) =
 *     (skip_ex_cmp_t *()(ex_node_t *, ex_node_t *))aux;
 *
 *   // Use the `fn` pointed to by `aux` to get the comparison function.
 *   skip_ex_cmp_t *cmp = fn(a, b);
 *
 *   // Now call that function and return the proper result.
 *   return cmp(list, a, b, aux);
 *  }
 */
#define SKIP_COMPARATOR(list, decl, fn_blk)                                             \
    int __skip_cmp_##decl(struct list *head, struct decl *a, struct decl *b, void *aux) \
    {                                                                                   \
        if (a == b)                                                                     \
            return 0;                                                                   \
        if (a == (head)->slh_head || b == (head)->slh_tail)                             \
            return -1;                                                                  \
        if (a == (head)->slh_tail || b == (head)->slh_head)                             \
            return 1;                                                                   \
        fn_blk                                                                          \
    }

#define SKIPLIST_EACH_H2T(decl, prefix, list, elm, iter) \
    for (iter = 0, (elm) = prefix##skip_head_##decl(list); (elm) != NULL; iter++, (elm) = prefix##skip_next_node_##decl(list, elm))

#define SKIPLIST_EACH_T2H(decl, prefix, list, elm, iter) \
    for (iter = prefix##skip_size_##decl(list), (elm) = prefix##skip_tail_##decl(list); (elm) != NULL; iter--, (elm) = prefix##skip_prev_node_##decl(list, elm))

#define __SKIP_NEXT_ENTRIES_T2B(field, elm) for (size_t lvl = elm->field.sle.height; lvl != (size_t)-1; lvl--)
#define __SKIP_IS_LAST_ENTRY_T2B() if (lvl == 0)

#define __SKIP_NEXT_ENTRIES_B2T(field, elm) for (size_t lvl = 0; lvl <= elm->field.sle.height; lvl++)
#define __SKIP_IS_LAST_ENTRY_B2T() if (lvl + 1 == elm->field.sle.height)

/*
 * Skip List declarations and access methods.
 */
#define SKIPLIST_DECL(decl, prefix, field, free_node_blk, update_node_blk, archive_node_blk, sizeof_entry_blk)                                        \
                                                                                                                                                      \
    /* Skip List node type */                                                                                                                         \
    typedef struct decl##_node decl##_node_t;                                                                                                         \
                                                                                                                                                      \
    /* Skip List structure and type */                                                                                                                \
    typedef struct decl {                                                                                                                             \
        size_t level, length, max, gen, max_gen;                                                                                                      \
        int (*cmp)(struct decl *, decl##_node_t *, decl##_node_t *, void *);                                                                          \
        void *aux;                                                                                                                                    \
        decl##_node_t *slh_head;                                                                                                                      \
        decl##_node_t *slh_tail;                                                                                                                      \
        decl##_node_t *slh_pres;                                                                                                                      \
    } decl##_t;                                                                                                                                       \
                                                                                                                                                      \
    /* Skip List comparison function type */                                                                                                          \
    typedef int (*skip_##decl##_cmp_t)(decl##_t *, decl##_node_t *, decl##_node_t *, void *);                                                         \
                                                                                                                                                      \
    /* Used when positioning a cursor within a Skip List. */                                                                                          \
    typedef enum { SKIP_EQ = 0, SKIP_LTE = -1, SKIP_LT = -2, SKIP_GTE = 1, SKIP_GT = 2 } skip_pos_##decl_t;                                           \
                                                                                                                                                      \
    /* -- __skip_key_compare_                                                                                                                         \
     *                                                                                                                                                \
     * This function takes four arguments:                                                                                                            \
     *   - a reference to the Skip List                                                                                                               \
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
     * Skip List.  For example, when `max = 6` this function returns 0 with                                                                           \
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
        size_t sle_arr_sz = sizeof(struct __skiplist_##decl_idx) * slist->max;                                                                        \
        n = (decl##_node_t *)calloc(1, sizeof(decl##_node_t) + sle_arr_sz);                                                                           \
        if (n == NULL)                                                                                                                                \
            return ENOMEM;                                                                                                                            \
        n->field.sle.gen = slist->gen;                                                                                                                \
        n->field.sle.height = 0;                                                                                                                      \
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
        slist->gen = 1;                                                                                                                               \
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
        slist->slh_head->field.sle.height = 0;                                                                                                        \
        for (i = 0; i < slist->max; i++)                                                                                                              \
            slist->slh_head->field.sle.next[i] = slist->slh_tail;                                                                                     \
        slist->slh_head->field.sle.prev = NULL;                                                                                                       \
                                                                                                                                                      \
        slist->slh_tail->field.sle.height = slist->max;                                                                                               \
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
        i = slist->slh_head->field.sle.height;                                                                                                        \
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
    /* -- __skip_preserve_                                                                                                                            \
     * Preserve nodes for snapshots if necessary. Returns > 0 are                                                                                     \
     * errors, 0 means nothing preserved, negative number represents                                                                                  \
     * the number of nodes preserved.                                                                                                                 \
     *                                                                                                                                                \
     * ALGORITHM:                                                                                                                                     \
     * Foreach node in `path`, if the generation in that element                                                                                      \
     * is less than the current generation for the list then                                                                                          \
     * that node must be preserved before being mutated for this                                                                                      \
     * insert. So we:                                                                                                                                 \
     *   a) allocate a new node and copy this node into it without                                                                                    \
     *      copying the user-supplied additional memory (that will                                                                                    \
     *      happen iff an _update() is called on the node).                                                                                           \
     *   b) zero out the next sle.prev/next[] pointers                                                                                                \
     *   c) determine if this is a duplicate, if so we set the                                                                                        \
     *      sle.next[1] field to 0x1 as a reminder to re-insert                                                                                       \
     *      this element as a duplicate in the restore function.                                                                                      \
     *   d) insert the node's copy into the slh_pres singly-linked                                                                                    \
     *      list.                                                                                                                                     \
     * Meanwhile, don't duplicate head and the tail nodes if they                                                                                     \
     * are in the path[].                                                                                                                             \
     */                                                                                                                                               \
    static int __skip_preserve_##decl(decl##_t *slist, decl##_node_t **path, size_t len)                                                              \
    {                                                                                                                                                 \
        int rc = 0, n = 0;                                                                                                                            \
        size_t i;                                                                                                                                     \
        decl##_node_t *node = path[0];                                                                                                                \
                                                                                                                                                      \
        for (i = 0; i < len; i++) {                                                                                                                   \
            if (path[i]->field.sle.gen < slist->gen) {                                                                                                \
                if (path[i] == slist->slh_head || path[i] == slist->slh_tail)                                                                         \
                    continue;                                                                                                                         \
                decl##_node_t *src = node, *dest, *this;                                                                                              \
                size_t amt = sizeof(src);                                                                                                             \
                char *d = NULL;                                                                                                                       \
                const char *s = (const char *)src;                                                                                                    \
                rc = prefix##skip_alloc_node_##decl(slist, &dest);                                                                                    \
                if (rc)                                                                                                                               \
                    return rc;                                                                                                                        \
                d = (char *)dest;                                                                                                                     \
                for (size_t i = 0; i < amt; i++)                                                                                                      \
                    d[i] = s[i];                                                                                                                      \
                                                                                                                                                      \
                this = dest;                                                                                                                          \
                this->field.sle.prev = NULL;                                                                                                          \
                __SKIP_NEXT_ENTRIES_B2T(field, this)                                                                                                  \
                {                                                                                                                                     \
                    this->field.sle.next[lvl] = NULL;                                                                                                 \
                }                                                                                                                                     \
                                                                                                                                                      \
                if (__skip_key_compare_##decl(slist, dest, dest->field.sle.next[0], slist->aux) == 0 ||                                               \
                    __skip_key_compare_##decl(slist, dest, dest->field.sle.prev, slist->aux) == 0)                                                    \
                    dest->field.sle.next[0] = (decl##_node_t *)0x1;                                                                                   \
                                                                                                                                                      \
                if (slist->slh_pres == NULL)                                                                                                          \
                    slist->slh_pres = dest;                                                                                                           \
                else {                                                                                                                                \
                    dest->field.sle.next[0] = slist->slh_pres;                                                                                        \
                    slist->slh_pres = dest;                                                                                                           \
                }                                                                                                                                     \
                n++;                                                                                                                                  \
            }                                                                                                                                         \
        }                                                                                                                                             \
        return -n;                                                                                                                                    \
    }                                                                                                                                                 \
                                                                                                                                                      \
    /* -- __skip_insert_ */                                                                                                                           \
    static int __skip_insert_##decl(decl##_t *slist, decl##_node_t *n, int flags)                                                                     \
    {                                                                                                                                                 \
        int rc = 0;                                                                                                                                   \
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
            n->field.sle.gen = slist->gen;                                                                                                            \
            n->field.sle.height = level;                                                                                                              \
            rc = __skip_preserve_##decl(slist, path, len);                                                                                            \
            if (rc > 0)                                                                                                                               \
                return rc;                                                                                                                            \
            for (i = slist->slh_head->field.sle.height + 1; i < n->field.sle.height + 1; i++) {                                                       \
                path[i + 1] = slist->slh_tail;                                                                                                        \
            }                                                                                                                                         \
            for (i = 0; i < n->field.sle.height + 1; i++) {                                                                                           \
                n->field.sle.next[i] = path[i + 1]->field.sle.next[i];                                                                                \
                path[i + 1]->field.sle.next[i] = n;                                                                                                   \
            }                                                                                                                                         \
            n->field.sle.prev = path[1];                                                                                                              \
            n->field.sle.next[0]->field.sle.prev = n;                                                                                                 \
            if (n->field.sle.next[0] == slist->slh_tail) {                                                                                            \
                slist->slh_tail->field.sle.prev = n;                                                                                                  \
            }                                                                                                                                         \
            if (level > slist->slh_head->field.sle.height) {                                                                                          \
                slist->slh_head->field.sle.height = level;                                                                                            \
                slist->slh_tail->field.sle.height = level;                                                                                            \
            }                                                                                                                                         \
            slist->length++;                                                                                                                          \
                                                                                                                                                      \
            if (SKIPLIST_MAX_HEIGHT == 1)                                                                                                             \
                free(path);                                                                                                                           \
        }                                                                                                                                             \
        return rc;                                                                                                                                    \
    }                                                                                                                                                 \
                                                                                                                                                      \
    /* -- skip_insert_ */                                                                                                                             \
    int prefix##skip_insert_##decl(decl##_t *slist, decl##_node_t *n)                                                                                 \
    {                                                                                                                                                 \
        return __skip_insert_##decl(slist, n, 0);                                                                                                     \
    }                                                                                                                                                 \
                                                                                                                                                      \
    /* -- skip_merge_lists_ TODO                                                                                                                      \
     * Merge two lists together into one.  The value of `flags` determines if:                                                                        \
     *  - duplicates are preserved                                                                                                                    \
     *  - duplicates from dest are preferred over those from src                                                                                      \
     *  - duplicates from src are preferred over those from dest                                                                                      \
     */                                                                                                                                               \
    int prefix##skip_merge_lists_##decl(decl##_t *dest, decl##_t *src, int flags)                                                                     \
    {                                                                                                                                                 \
        ((void)src);                                                                                                                                  \
        ((void)dest);                                                                                                                                 \
        ((void)flags);                                                                                                                                \
        return 0;                                                                                                                                     \
    }                                                                                                                                                 \
                                                                                                                                                      \
    /* -- skip_bulk_insert_ TODO */                                                                                                                   \
    int prefix##skip_bulk_insert_##decl(decl##_t *slist)                                                                                              \
    {                                                                                                                                                 \
        ((void)slist);                                                                                                                                \
        return 0;                                                                                                                                     \
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
        i = slist->slh_head->field.sle.height;                                                                                                        \
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
        i = slist->slh_head->field.sle.height;                                                                                                        \
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
    /* -- skip_position_gt_                                                                                                                           \
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
        i = slist->slh_head->field.sle.height;                                                                                                        \
                                                                                                                                                      \
        do {                                                                                                                                          \
            while (elm && __skip_key_compare_##decl(slist, elm->field.sle.next[i], n, slist->aux) < 0)                                                \
                elm = elm->field.sle.next[i];                                                                                                         \
        } while (i--);                                                                                                                                \
        do {                                                                                                                                          \
            elm = elm->field.sle.next[0];                                                                                                             \
            cmp = __skip_key_compare_##decl(slist, elm, n, slist->aux);                                                                               \
        } while (cmp <= 0);                                                                                                                           \
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
        i = slist->slh_head->field.sle.height;                                                                                                        \
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
            } while (cmp >= 0);                                                                                                                       \
        }                                                                                                                                             \
        return elm;                                                                                                                                   \
    }                                                                                                                                                 \
                                                                                                                                                      \
    /* -- skip_position_lt_                                                                                                                           \
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
        i = slist->slh_head->field.sle.height;                                                                                                        \
                                                                                                                                                      \
        do {                                                                                                                                          \
            while (elm && __skip_key_compare_##decl(slist, elm->field.sle.next[i], n, slist->aux) < 0)                                                \
                elm = elm->field.sle.next[i];                                                                                                         \
        } while (i--);                                                                                                                                \
        elm = elm->field.sle.next[0];                                                                                                                 \
        do {                                                                                                                                          \
            elm = elm->field.sle.prev;                                                                                                                \
            cmp = __skip_key_compare_##decl(slist, elm, n, slist->aux);                                                                               \
        } while (cmp >= 0);                                                                                                                           \
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
    /* -- skip_remove_node_ */                                                                                                                        \
    int prefix##skip_remove_node_##decl(decl##_t *slist, decl##_node_t *n)                                                                            \
    {                                                                                                                                                 \
        int np = 0;                                                                                                                                   \
        size_t i, len, level;                                                                                                                         \
        static decl##_node_t apath[SKIPLIST_MAX_HEIGHT + 1];                                                                                          \
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
            np = __skip_preserve_##decl(slist, path, len);                                                                                            \
            if (np > 0)                                                                                                                               \
                return np;                                                                                                                            \
            node->field.sle.next[0]->field.sle.prev = node->field.sle.prev;                                                                           \
            for (i = 1; i <= len; i++) {                                                                                                              \
                if (path[i]->field.sle.next[i - 1] != node)                                                                                           \
                    break;                                                                                                                            \
                path[i]->field.sle.next[i - 1] = node->field.sle.next[i - 1];                                                                         \
                if (path[i]->field.sle.next[i - 1] == slist->slh_tail) {                                                                              \
                    level = path[i]->field.sle.height;                                                                                                \
                    path[i]->field.sle.height = level - 1;                                                                                            \
                }                                                                                                                                     \
            }                                                                                                                                         \
            if (node->field.sle.next[0] == slist->slh_tail) {                                                                                         \
                slist->slh_tail->field.sle.prev = n->field.sle.prev;                                                                                  \
            }                                                                                                                                         \
            if (SKIPLIST_MAX_HEIGHT == 1)                                                                                                             \
                free(path);                                                                                                                           \
            if (np == 0)                                                                                                                              \
                free_node_blk;                                                                                                                        \
                                                                                                                                                      \
            /* Find all levels in the first element in the list that point                                                                            \
               at the tail and shrink the level. */                                                                                                   \
            i = 0;                                                                                                                                    \
            node = slist->slh_head;                                                                                                                   \
            while (node->field.sle.next[i] != slist->slh_tail && i++ < slist->slh_head->field.sle.height)                                             \
                ;                                                                                                                                     \
            slist->slh_head->field.sle.height = i;                                                                                                    \
            slist->slh_tail->field.sle.height = i;                                                                                                    \
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
        if (prefix##skip_empty_##decl(slist))                                                                                                         \
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
    /* -- skip_snapshot_                                                                                                                              \
     * A snapshot is a read-only view of a Skip List at a point in time.  Once                                                                        \
     * taken, a snapshot must be restored or disposed. Any number of snapshots                                                                        \
     * can be created.                                                                                                                                \
     */                                                                                                                                               \
    size_t prefix##skip_snapshot_##decl(decl##_t *slist)                                                                                              \
    {                                                                                                                                                 \
        if (slist == NULL)                                                                                                                            \
            return 0;                                                                                                                                 \
                                                                                                                                                      \
        slist->gen++;                                                                                                                                 \
        if (slist->gen > slist->max_gen)                                                                                                              \
            slist->max_gen = slist->gen;                                                                                                              \
        return slist->gen;                                                                                                                            \
    }                                                                                                                                                 \
                                                                                                                                                      \
    /* -- skip_restore_snapshot_                                                                                                                      \
     * Restores the Skiplist to generation `gen`.  Once you restore `gen` you                                                                         \
     * can no longer access any generations > `gen`.                                                                                                  \
     */                                                                                                                                               \
    decl##_t *prefix##skip_restore_snapshot_##decl(decl##_t *slist, unsigned gen)                                                                     \
    {                                                                                                                                                 \
        size_t i;                                                                                                                                     \
        decl##_node_t *node, *next, *prev;                                                                                                            \
                                                                                                                                                      \
        if (slist == NULL)                                                                                                                            \
            return NULL;                                                                                                                              \
                                                                                                                                                      \
        if (gen >= slist->gen || slist->slh_pres == NULL)                                                                                             \
            return slist;                                                                                                                             \
                                                                                                                                                      \
        /* ALGORITHM:                                                                                                                                 \
         * iterate over the preserved nodes (slist->slh_pres)                                                                                         \
         *  a) remove nodes with node->gen > gen from slist                                                                                           \
         *  b) remove nodes > gen from slh_pres and _free_node()                                                                                      \
         *  c) restore nodes == gen by...                                                                                                             \
         *     i) remove node from slh_pres list                                                                                                      \
         *     ii) _insert(node) or                                                                                                                   \
         *         _insert_dup() if node->field.sle_next[1] != 0 (clear that)                                                                         \
         *  d) set slist's gen to `gen`                                                                                                               \
         *                                                                                                                                            \
         * NOTES:                                                                                                                                     \
         * - the `node->field.sle.next[0]` forms a singly-linked list.                                                                                \
         */                                                                                                                                           \
                                                                                                                                                      \
        SKIPLIST_EACH_H2T(decl, prefix, slist, node, i)                                                                                               \
        {                                                                                                                                             \
            ((void)i);                                                                                                                                \
            if (node->field.sle.gen > gen)                                                                                                            \
                prefix##skip_remove_node_##decl(slist, node);                                                                                         \
        }                                                                                                                                             \
                                                                                                                                                      \
        prev = NULL;                                                                                                                                  \
        node = slist->slh_pres;                                                                                                                       \
        while (node) {                                                                                                                                \
            next = node->field.sle.next[0];                                                                                                           \
            if (node->field.sle.gen > gen) {                                                                                                          \
                if (prev == NULL)                                                                                                                     \
                    slist->slh_pres = next;                                                                                                           \
                else                                                                                                                                  \
                    prev->field.sle.next[0] = next;                                                                                                   \
                prefix##skip_free_node_##decl(node);                                                                                                  \
            }                                                                                                                                         \
            if (node->field.sle.gen == gen) {                                                                                                         \
                if (node->field.sle.next[1] != 0) {                                                                                                   \
                    node->field.sle.next[1] = NULL;                                                                                                   \
                    prefix##skip_insert_dup_##decl(slist, node);                                                                                      \
                } else {                                                                                                                              \
                    prefix##skip_insert_##decl(slist, node);                                                                                          \
                }                                                                                                                                     \
            }                                                                                                                                         \
            prev = node;                                                                                                                              \
            node = next;                                                                                                                              \
        }                                                                                                                                             \
        slist->max_gen = gen;                                                                                                                         \
        return slist;                                                                                                                                 \
    }                                                                                                                                                 \
                                                                                                                                                      \
    /* -- skip_dispose_snapshot_                                                                                                                      \
     * Removes from history all snapshots equal to or newer than (>=)                                                                                 \
     * `gen`.                                                                                                                                         \
     */                                                                                                                                               \
    void prefix##skip_dispose_snapshot_##decl(decl##_t *slist, unsigned gen)                                                                          \
    {                                                                                                                                                 \
        decl##_node_t *node, *next, *prev;                                                                                                            \
                                                                                                                                                      \
        if (slist == NULL)                                                                                                                            \
            return;                                                                                                                                   \
                                                                                                                                                      \
        if (slist->slh_pres == NULL)                                                                                                                  \
            return;                                                                                                                                   \
                                                                                                                                                      \
        prev = NULL;                                                                                                                                  \
        node = slist->slh_pres;                                                                                                                       \
        while (node) {                                                                                                                                \
            next = node->field.sle.next[0];                                                                                                           \
            if (node->field.sle.gen >= gen) {                                                                                                         \
                if (prev == NULL)                                                                                                                     \
                    slist->slh_pres = next;                                                                                                           \
                else                                                                                                                                  \
                    prev->field.sle.next[0] = next;                                                                                                   \
                prefix##skip_free_node_##decl(node);                                                                                                  \
            }                                                                                                                                         \
            prev = node;                                                                                                                              \
            node = next;                                                                                                                              \
        }                                                                                                                                             \
        slist->max_gen = (gen == 0 ? 0 : gen - 1);                                                                                                    \
    }                                                                                                                                                 \
                                                                                                                                                      \
    /* Archive of a Skip List */                                                                                                                      \
    typedef struct decl##_archive {                                                                                                                   \
        decl##_t list;                                                                                                                                \
        decl##_node_t *nodes;                                                                                                                         \
        size_t bytes;                                                                                                                                 \
    } decl##_archive_t;                                                                                                                               \
                                                                                                                                                      \
    /* -- skip_to_bytes_ TODO/WIP                                                                                                                     \
     * Similar to snapshot, but includes the values and encodes them                                                                                  \
     * in a portable manner.                                                                                                                          \
     */                                                                                                                                               \
    decl##_archive_t *prefix##skip_to_bytes_##decl(decl##_t *slist)                                                                                   \
    {                                                                                                                                                 \
        size_t size, bytes, i;                                                                                                                        \
        decl##_archive_t *archive;                                                                                                                    \
        decl##_node_t *node, *new;                                                                                                                    \
                                                                                                                                                      \
        if (slist == NULL)                                                                                                                            \
            return 0;                                                                                                                                 \
                                                                                                                                                      \
        bytes = sizeof(decl##_archive_t) + (slist->length * sizeof(decl##_node_t));                                                                   \
        node = prefix##skip_head_##decl(slist);                                                                                                       \
        while (node) {                                                                                                                                \
            sizeof_entry_blk;                                                                                                                         \
            bytes += sizeof(size_t);                                                                                                                  \
            bytes += size;                                                                                                                            \
            node = prefix##skip_next_node_##decl(slist, node);                                                                                        \
        }                                                                                                                                             \
        archive = (decl##_archive_t *)calloc(1, bytes);                                                                                               \
        if (archive == NULL)                                                                                                                          \
            return NULL;                                                                                                                              \
                                                                                                                                                      \
        archive->bytes = bytes;                                                                                                                       \
        archive->list.length = slist->length;                                                                                                         \
        archive->list.max = slist->max;                                                                                                               \
        archive->nodes = (decl##_node_t *)(archive + sizeof(decl##_archive_t));                                                                       \
                                                                                                                                                      \
        i = 0;                                                                                                                                        \
        node = prefix##skip_head_##decl(slist);                                                                                                       \
        while (node) {                                                                                                                                \
            decl##_node_t *n = (decl##_node_t *)archive->nodes + (i++ * sizeof(decl##_node_t));                                                       \
            new = (decl##_node_t *)&n;                                                                                                                \
            archive_node_blk;                                                                                                                         \
            node = prefix##skip_next_node_##decl(slist, node);                                                                                        \
        }                                                                                                                                             \
        return archive;                                                                                                                               \
    }                                                                                                                                                 \
                                                                                                                                                      \
    /* -- skip_from_bytes_ TODO/WIP */                                                                                                                \
    decl##_t *prefix##skip_from_bytes_##decl(decl##_archive_t *archive, int (*cmp)(decl##_t * head, decl##_node_t * a, decl##_node_t * b, void *aux)) \
    {                                                                                                                                                 \
        int rc;                                                                                                                                       \
        size_t i;                                                                                                                                     \
        decl##_t *slist;                                                                                                                              \
        decl##_node_t *node, *new;                                                                                                                    \
                                                                                                                                                      \
        if (archive == NULL || cmp == NULL)                                                                                                           \
            return 0;                                                                                                                                 \
        slist = (decl##_t *)calloc(1, sizeof(decl##_t));                                                                                              \
        if (slist == NULL)                                                                                                                            \
            return NULL;                                                                                                                              \
                                                                                                                                                      \
        slist->cmp = cmp;                                                                                                                             \
        slist->max = archive->list.max;                                                                                                               \
                                                                                                                                                      \
        rc = prefix##skip_alloc_node_##decl(slist, &slist->slh_head);                                                                                 \
        if (rc)                                                                                                                                       \
            goto fail;                                                                                                                                \
        rc = prefix##skip_alloc_node_##decl(slist, &slist->slh_tail);                                                                                 \
        if (rc)                                                                                                                                       \
            goto fail;                                                                                                                                \
                                                                                                                                                      \
        slist->slh_head->field.sle.height = 0;                                                                                                        \
        for (i = 0; i < slist->max; i++)                                                                                                              \
            slist->slh_head->field.sle.next[i] = slist->slh_tail;                                                                                     \
        slist->slh_head->field.sle.prev = NULL;                                                                                                       \
                                                                                                                                                      \
        slist->slh_tail->field.sle.height = slist->max;                                                                                               \
        for (i = 0; i < slist->max; i++)                                                                                                              \
            slist->slh_tail->field.sle.next[i] = NULL;                                                                                                \
        slist->slh_tail->field.sle.prev = slist->slh_head;                                                                                            \
                                                                                                                                                      \
        i = 0;                                                                                                                                        \
        while (archive->list.length > 0) {                                                                                                            \
            decl##_node_t *n = (decl##_node_t *)archive->nodes + (i++ * sizeof(decl##_node_t));                                                       \
            node = (decl##_node_t *)&n;                                                                                                               \
            rc = prefix##skip_alloc_node_##decl(slist, &new);                                                                                         \
            archive_node_blk;                                                                                                                         \
            __skip_insert_##decl(slist, new, 1);                                                                                                      \
            archive->list.length--;                                                                                                                   \
        }                                                                                                                                             \
        return slist;                                                                                                                                 \
    fail:;                                                                                                                                            \
        if (slist->slh_head)                                                                                                                          \
            free(slist->slh_head);                                                                                                                    \
        if (slist->slh_tail)                                                                                                                          \
            free(slist->slh_tail);                                                                                                                    \
        return NULL;                                                                                                                                  \
    }

#define SKIPLIST_INTEGRITY_CHECK(decl, prefix, field)                                                                                                  \
    /* -- __skip_integrity_failure_ */                                                                                                                 \
    static void __attribute__((format(printf, 1, 2))) __skip_integrity_failure_##decl(const char *fmt, ...)                                            \
    {                                                                                                                                                  \
        va_list args;                                                                                                                                  \
        __skip_debugf(fmt, args);                                                                                                                      \
    }                                                                                                                                                  \
                                                                                                                                                       \
    /* -- __skip_integrity_check_ */                                                                                                                   \
    static int __skip_integrity_check_##decl(decl##_t *slist, int flags)                                                                               \
    {                                                                                                                                                  \
        unsigned nth, n_err = 0;                                                                                                                       \
        decl##_node_t *node, *prev, *next;                                                                                                             \
        struct __skiplist_##decl_idx *this;                                                                                                            \
                                                                                                                                                       \
        if (slist == NULL) {                                                                                                                           \
            __skip_integrity_failure_##decl("slist was NULL, nothing to check");                                                                       \
            n_err++;                                                                                                                                   \
            return n_err;                                                                                                                              \
        }                                                                                                                                              \
                                                                                                                                                       \
        /* Check the Skiplist header (slh) */                                                                                                          \
                                                                                                                                                       \
        if (slist->slh_head == NULL) {                                                                                                                 \
            __skip_integrity_failure_##decl("skiplist slh_head is NULL");                                                                              \
            n_err++;                                                                                                                                   \
            return n_err;                                                                                                                              \
        }                                                                                                                                              \
                                                                                                                                                       \
        if (slist->slh_tail == NULL) {                                                                                                                 \
            __skip_integrity_failure_##decl("skiplist slh_tail is NULL");                                                                              \
            n_err++;                                                                                                                                   \
            return n_err;                                                                                                                              \
        }                                                                                                                                              \
                                                                                                                                                       \
        if (slist->cmp == NULL) {                                                                                                                      \
            __skip_integrity_failure_##decl("skiplist comparison function (cmp) is NULL");                                                             \
            n_err++;                                                                                                                                   \
            return n_err;                                                                                                                              \
        }                                                                                                                                              \
                                                                                                                                                       \
        if (slist->max < 2) {                                                                                                                          \
            __skip_integrity_failure_##decl("skiplist max level must be 1 at minimum");                                                                \
            n_err++;                                                                                                                                   \
            if (flags)                                                                                                                                 \
                return n_err;                                                                                                                          \
        }                                                                                                                                              \
                                                                                                                                                       \
        if (slist->level >= slist->max) {                                                                                                              \
            /* level is 0-based, max of 12 means level cannot be > 11 */                                                                               \
            __skip_integrity_failure_##decl("skiplist level in header was >= max");                                                                    \
            n_err++;                                                                                                                                   \
            if (flags)                                                                                                                                 \
                return n_err;                                                                                                                          \
        }                                                                                                                                              \
                                                                                                                                                       \
        if (SKIPLIST_MAX_HEIGHT < 1) {                                                                                                                 \
            __skip_integrity_failure_##decl("SKIPLIST_MAX_HEIGHT cannot be less than 1");                                                              \
            n_err++;                                                                                                                                   \
            if (flags)                                                                                                                                 \
                return n_err;                                                                                                                          \
        }                                                                                                                                              \
                                                                                                                                                       \
        if (SKIPLIST_MAX_HEIGHT > 0 && slist->max > SKIPLIST_MAX_HEIGHT) {                                                                             \
            __skip_integrity_failure_##decl("slist->max cannot be greater than SKIPLIST_MAX_HEIGHT");                                                  \
            n_err++;                                                                                                                                   \
            if (flags)                                                                                                                                 \
                return n_err;                                                                                                                          \
        }                                                                                                                                              \
                                                                                                                                                       \
        node = slist->slh_head;                                                                                                                        \
        for (nth = 0; nth < node->field.sle.height; nth++) {                                                                                           \
            if (node->field.sle.next[nth] == NULL) {                                                                                                   \
                __skip_integrity_failure_##decl("the head's %u next node should not be NULL", nth);                                                    \
                n_err++;                                                                                                                               \
                if (flags)                                                                                                                             \
                    return n_err;                                                                                                                      \
            }                                                                                                                                          \
            if (node->field.sle.next[nth] == slist->slh_tail)                                                                                          \
                break;                                                                                                                                 \
        }                                                                                                                                              \
        for (; nth < node->field.sle.height; nth++) {                                                                                                  \
            if (node->field.sle.next[nth] == NULL) {                                                                                                   \
                __skip_integrity_failure_##decl("the head's %u next node should not be NULL", nth);                                                    \
                n_err++;                                                                                                                               \
                if (flags)                                                                                                                             \
                    return n_err;                                                                                                                      \
            }                                                                                                                                          \
            if (node->field.sle.next[nth] != slist->slh_tail) {                                                                                        \
                __skip_integrity_failure_##decl("after internal nodes, the head's %u next node should always be the tail", nth);                       \
                n_err++;                                                                                                                               \
                if (flags)                                                                                                                             \
                    return n_err;                                                                                                                      \
            }                                                                                                                                          \
        }                                                                                                                                              \
                                                                                                                                                       \
        if (slist->length > 0 && slist->slh_tail->field.sle.prev == slist->slh_head) {                                                                 \
            __skip_integrity_failure_##decl("slist->length is 0, but tail->prev == head, not an internal node");                                       \
            n_err++;                                                                                                                                   \
            if (flags)                                                                                                                                 \
                return n_err;                                                                                                                          \
        }                                                                                                                                              \
                                                                                                                                                       \
        /* Validate the head node */                                                                                                                   \
                                                                                                                                                       \
        /* Validate the tail node */                                                                                                                   \
                                                                                                                                                       \
        /* Validate each node */                                                                                                                       \
        nth = 0;                                                                                                                                       \
        node = prefix##skip_head_##decl(slist);                                                                                                        \
        while (node) {                                                                                                                                 \
            this = &node->field.sle;                                                                                                                   \
                                                                                                                                                       \
            if (this->next == NULL) {                                                                                                                  \
                __skip_integrity_failure_##decl("the %uth node's [%p] next field should never NULL", nth, (void *)node);                               \
                n_err++;                                                                                                                               \
                if (flags)                                                                                                                             \
                    return n_err;                                                                                                                      \
            }                                                                                                                                          \
                                                                                                                                                       \
            if (this->prev == NULL) {                                                                                                                  \
                __skip_integrity_failure_##decl("the %u node [%p] prev field should never NULL", nth, (void *)node);                                   \
                n_err++;                                                                                                                               \
                if (flags)                                                                                                                             \
                    return n_err;                                                                                                                      \
            }                                                                                                                                          \
                                                                                                                                                       \
            uintptr_t a = (uintptr_t)this->next;                                                                                                       \
            uintptr_t b = (intptr_t)((uintptr_t)node + sizeof(decl##_node_t));                                                                         \
            if (a != b) {                                                                                                                              \
                __skip_integrity_failure_##decl("the %uth node's [%p] next field isn't at the proper offset relative to the node", nth, (void *)node); \
                n_err++;                                                                                                                               \
                if (flags)                                                                                                                             \
                    return n_err;                                                                                                                      \
            }                                                                                                                                          \
                                                                                                                                                       \
            next = this->next[0];                                                                                                                      \
            prev = this->prev;                                                                                                                         \
            if (__skip_key_compare_##decl(slist, node, node, slist->aux) != 0) {                                                                       \
                __skip_integrity_failure_##decl("the %uth node [%p] is not equal to itself", nth, (void *)node);                                       \
                n_err++;                                                                                                                               \
                if (flags)                                                                                                                             \
                    return n_err;                                                                                                                      \
            }                                                                                                                                          \
                                                                                                                                                       \
            if (__skip_key_compare_##decl(slist, node, prev, slist->aux) < 0) {                                                                        \
                __skip_integrity_failure_##decl("the %uth node [%p] is not greater than the prev node [%p]", nth, (void *)node, (void *)prev);         \
                n_err++;                                                                                                                               \
                if (flags)                                                                                                                             \
                    return n_err;                                                                                                                      \
            }                                                                                                                                          \
                                                                                                                                                       \
            if (__skip_key_compare_##decl(slist, node, next, slist->aux) > 0) {                                                                        \
                __skip_integrity_failure_##decl("the %uth node [%p] is not less than the next node [%p]", nth, (void *)node, (void *)next);            \
                n_err++;                                                                                                                               \
                if (flags)                                                                                                                             \
                    return n_err;                                                                                                                      \
            }                                                                                                                                          \
                                                                                                                                                       \
            if (__skip_key_compare_##decl(slist, prev, node, slist->aux) > 0) {                                                                        \
                __skip_integrity_failure_##decl("the prev node [%p] is not less than the %uth node [%p]", (void *)prev, nth, (void *)node);            \
                n_err++;                                                                                                                               \
                if (flags)                                                                                                                             \
                    return n_err;                                                                                                                      \
            }                                                                                                                                          \
                                                                                                                                                       \
            if (__skip_key_compare_##decl(slist, next, node, slist->aux) < 0) {                                                                        \
                __skip_integrity_failure_##decl("the next node [%p] is not greater than the %uth node [%p]", (void *)next, nth, (void *)node);         \
                n_err++;                                                                                                                               \
                if (flags)                                                                                                                             \
                    return n_err;                                                                                                                      \
            }                                                                                                                                          \
                                                                                                                                                       \
            node = prefix##skip_next_node_##decl(slist, node);                                                                                         \
            nth++;                                                                                                                                     \
        }                                                                                                                                              \
                                                                                                                                                       \
        if (slist->length != nth) {                                                                                                                    \
            __skip_integrity_failure_##decl("slist->length doesn't match the count of nodes between the head and tail");                               \
            n_err++;                                                                                                                                   \
            if (flags)                                                                                                                                 \
                return n_err;                                                                                                                          \
        }                                                                                                                                              \
                                                                                                                                                       \
        return 0;                                                                                                                                      \
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
    int prefix##skip_dup_##decl(decl##_t *slist, ktype key, vtype value)                     \
    {                                                                                        \
        int rc;                                                                              \
        decl##_node_t *node;                                                                 \
        rc = prefix##skip_alloc_node_##decl(slist, &node);                                   \
        if (rc)                                                                              \
            return rc;                                                                       \
        node->key = key;                                                                     \
        node->value = value;                                                                 \
        rc = prefix##skip_insert_dup_##decl(slist, node);                                    \
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
    static size_t __skip_dot_width_##decl(decl##_t *slist, decl##_node_t *node, size_t level)                                       \
    {                                                                                                                               \
        size_t w = 1;                                                                                                               \
        decl##_node_t *n;                                                                                                           \
                                                                                                                                    \
        if (node == slist->slh_tail)                                                                                                \
            return 0;                                                                                                               \
                                                                                                                                    \
        n = node->field.sle.next[level] == slist->slh_tail ? NULL : node->field.sle.next[level];                                    \
        while (n && n->field.sle.prev != node) {                                                                                    \
            w++;                                                                                                                    \
            n = prefix##skip_prev_node_##decl(slist, n);                                                                            \
        }                                                                                                                           \
                                                                                                                                    \
        return w;                                                                                                                   \
    }                                                                                                                               \
                                                                                                                                    \
    static inline void __skip_dot_write_node_##decl(FILE *os, size_t nsg, decl##_node_t *node)                                      \
    {                                                                                                                               \
        fprintf(os, "\"node%zu %p\"", nsg, (void *)node);                                                                           \
    }                                                                                                                               \
                                                                                                                                    \
    /* -- __skip_dot_node_                                                                                                          \
     * Writes out a fragment of a DOT file representing a node.                                                                     \
     */                                                                                                                             \
    static void __skip_dot_node_##decl(FILE *os, decl##_t *slist, decl##_node_t *node, size_t nsg, skip_sprintf_node_##decl##_t fn) \
    {                                                                                                                               \
        char buf[2048];                                                                                                             \
        size_t width;                                                                                                               \
        decl##_node_t *this;                                                                                                        \
                                                                                                                                    \
        __skip_dot_write_node_##decl(os, nsg, node);                                                                                \
        fprintf(os, " [label = \"");                                                                                                \
        fflush(os);                                                                                                                 \
        this = node;                                                                                                                \
        __SKIP_NEXT_ENTRIES_T2B(field, this)                                                                                        \
        {                                                                                                                           \
            width = __skip_dot_width_##decl(slist, this, lvl);                                                                      \
            fprintf(os, " { <w%zu> %zu | <f%zu> %p } |", lvl, width, lvl, (void *)this->field.sle.next[lvl]);                       \
            fflush(os);                                                                                                             \
        }                                                                                                                           \
        if (fn) {                                                                                                                   \
            fn(node, buf);                                                                                                          \
            fprintf(os, " <f0> h:%zu - %s\"\n", node->field.sle.height, buf);                                                       \
        } else {                                                                                                                    \
            fprintf(os, " <f0> h:%zu \"\n", node->field.sle.height);                                                                \
        }                                                                                                                           \
        fprintf(os, "shape = \"record\"\n");                                                                                        \
        fprintf(os, "];\n");                                                                                                        \
        fflush(os);                                                                                                                 \
                                                                                                                                    \
        /* Now edges */                                                                                                             \
        this = node;                                                                                                                \
        __SKIP_NEXT_ENTRIES_B2T(field, this)                                                                                        \
        {                                                                                                                           \
            __skip_dot_write_node_##decl(os, nsg, this);                                                                            \
            fprintf(os, ":f%zu -> ", lvl);                                                                                          \
            __skip_dot_write_node_##decl(os, nsg, this->field.sle.next[lvl]);                                                       \
            fprintf(os, ":w%zu [];\n", lvl);                                                                                        \
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
    /* -- skip_dot_                                                                                                                 \
     * Create a DOT file of the internal representation of the                                                                      \
     * Skip List on the provided file descriptor (default: STDOUT).                                                                 \
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
        int letitgo = 0;                                                                                                            \
        size_t width, i;                                                                                                            \
        decl##_node_t *node, *next;                                                                                                 \
                                                                                                                                    \
        if (slist == NULL || fn == NULL)                                                                                            \
            return nsg;                                                                                                             \
        if (__skip_integrity_check_##decl(slist, 1) != 0) {                                                                         \
            perror("Skiplist failed integrity checks, impossible to diagram.");                                                     \
            return -1;                                                                                                              \
        }                                                                                                                           \
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
        if (slist->slh_head->field.sle.height || slist->slh_head->field.sle.next[0] != slist->slh_tail)                             \
            letitgo = 1;                                                                                                            \
                                                                                                                                    \
        /* Write out the fields */                                                                                                  \
        node = slist->slh_head;                                                                                                     \
        if (letitgo) {                                                                                                              \
            __SKIP_NEXT_ENTRIES_T2B(field, node)                                                                                    \
            {                                                                                                                       \
                width = __skip_dot_width_##decl(slist, node, lvl);                                                                  \
                fprintf(os, "{ %zu | <f%zu> %p }", width, lvl, (void *)node);                                                       \
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
            __SKIP_NEXT_ENTRIES_B2T(field, node)                                                                                    \
            {                                                                                                                       \
                fprintf(os, "\"HeadNode%zu\":f%zu -> ", nsg, lvl);                                                                  \
                __skip_dot_write_node_##decl(os, nsg, node->field.sle.next[lvl]);                                                   \
                fprintf(os, ":w%zu [];\n", lvl);                                                                                    \
            }                                                                                                                       \
            fprintf(os, "\n");                                                                                                      \
        }                                                                                                                           \
        fflush(os);                                                                                                                 \
                                                                                                                                    \
        /* Now all nodes via level 0, if non-empty */                                                                               \
        node = slist->slh_head;                                                                                                     \
        if (letitgo) {                                                                                                              \
            SKIPLIST_EACH_H2T(decl, prefix, slist, next, i)                                                                         \
            {                                                                                                                       \
                ((void)i);                                                                                                          \
                __skip_dot_node_##decl(os, slist, next, nsg, fn);                                                                   \
                fflush(os);                                                                                                         \
            }                                                                                                                       \
            fprintf(os, "\n");                                                                                                      \
        }                                                                                                                           \
        fflush(os);                                                                                                                 \
                                                                                                                                    \
        /* The tail, sentinal node */                                                                                               \
        node = slist->slh_head;                                                                                                     \
        if (letitgo) {                                                                                                              \
            __skip_dot_write_node_##decl(os, nsg, slist->slh_tail);                                                                 \
            fprintf(os, "[label = \"");                                                                                             \
            node = slist->slh_tail;                                                                                                 \
            __SKIP_NEXT_ENTRIES_T2B(field, node)                                                                                    \
            {                                                                                                                       \
                fprintf(os, "<w%zu> %p", lvl, (void *)node);                                                                        \
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
