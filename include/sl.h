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
 * Portions of this code are derived from copyrighted work:
 *
 *  - MIT LICENSE
 *    - https://github.com/greensky00/skiplist
 *      2017-2024 Jung-Sang Ahn <jungsang.ahn@gmail.com>
 *    - https://github.com/paulross/skiplist
 *      Copyright (c) 2017-2023 Paul Ross
 *  - gist skiplist.c
 *  - khash.h
 *  - async_nif.h
 */

#ifndef _SKIPLIST_H_
#define _SKIPLIST_H_

/*
 * This file defines a skiplist data structure.
 *
 * In 1990 William Pugh published:
 *  - Skiplists: a probabilistic alternative to balanced trees.
 *    https://www.cl.cam.ac.uk/teaching/2005/Algorithms/skiplists.pdf
 * A Skiplist is an ordered data structure providing expected O(Log(n)) lookup,
 * insertion, and deletion complexity.
 */

/*
 * Private, internal API.
 */

/* NOTE:
 * This array is a bit unusual, while values are 0-based the array is allocated
 * with space for two more pointers which are used to store max size the array
 * can grow to (`array[-2]`) and the length, or number of elements used in the
 * array so far (`array[-1]`).
 */
#define ARRAY_ALLOC(var, type, size)                   \
  do {                                                 \
    (size) = (size == 0 ? 254 : size);                 \
    (var) = (type **)calloc(sizeof(type *), size + 2); \
    if ((var) != NULL) {                               \
      *(var)++ = (void *)(uintptr_t)size;              \
      *(var)++ = (void *)(uintptr_t)0;                 \
    }                                                  \
  } while (0)
#define ARRAY_FREE(list) free((list)-2)
#define ARRAY_SIZE(list) (unsigned int)(uintptr_t)(list)[-2]
#define ARRAY_SET_SIZE(list, size) (list)[-2] = (void *)(uintptr_t)(size)
#define ARRAY_LENGTH(list) (unsigned int)(uintptr_t)(list)[-1]
#define ARRAY_SET_LENGTH(list, len) (list)[-1] = (void *)(uintptr_t)(len)

/*
 * Skiplist declarations.
 */
#define SKIP_HEAD(name, type)                                        \
  struct name {                                                      \
    size_t level, length, max, fanout;                               \
    int (*cmp)(struct name *, struct type *, struct type *, void *); \
    void *aux;                                                       \
    struct type *slh_head;                                           \
    struct type *slh_tail;                                           \
  }

#define SKIP_ENTRY(type)    \
  struct {                  \
    struct type **sle_next; \
    struct type *sle_prev;  \
  }

/*
 * Skip List access methods.
 */
#define SKIP_FIRST(head) ((head)->slh_head)
#define SKIP_LAST(head) ((head)->slh_tail)
#define SKIP_NEXT(elm, field) ((elm)->field.sle_next[0])
#define SKIP_PREV(elm, field) ((elm)->field.sle_prev)
#define SKIP_EMPTY(head) ((head)->length == 0)

/*
 * Skip List functions.
 */

#define SKIP_COMPARATOR(list, type, fn)                                    \
  int __skip_cmp_##type(struct list *head, struct type *a, struct type *b, \
    void *aux)                                                             \
  {                                                                        \
    if (a == b)                                                            \
      return 0;                                                            \
    if (a == (head)->slh_head || b == (head)->slh_tail)                    \
      return -1;                                                           \
    if (a == (head)->slh_tail || b == (head)->slh_head)                    \
      return 1;                                                            \
    fn                                                                     \
  }

#define SKIPLIST_DECL(decl, prefix, field, free_node_blk, update_node_blk)   \
                                                                             \
  /* Skiplist node type */                                                   \
  typedef struct decl##_node decl##_node_t;                                  \
                                                                             \
  /* Skiplist type */                                                        \
  typedef struct decl {                                                      \
    size_t level, length, max, fanout;                                       \
    int (*cmp)(struct decl *, decl##_node_t *, decl##_node_t *, void *);     \
    void *aux;                                                               \
    decl##_node_t *slh_head;                                                 \
    decl##_node_t *slh_tail;                                                 \
  } decl##_t;                                                                \
                                                                             \
  /* -- __skip_key_compare_                                                  \
   *                                                                         \
   * This function takes four arguments:                                     \
   *   - a reference to the Skiplist                                         \
   *   - the two nodes to compare, `a` and `b`                               \
   *   - `aux` an additional auxiliary argument                              \
   * and returns:                                                            \
   *   a  < b : return -1                                                    \
   *   a == b : return 0                                                     \
   *   a  > b : return 1                                                     \
   */                                                                        \
  static int __skip_key_compare_##decl(decl##_t *slist, decl##_node_t *a,    \
    decl##_node_t *b, void *aux)                                             \
  {                                                                          \
    if (a == b)                                                              \
      return 0;                                                              \
    if (a == slist->slh_head || b == slist->slh_tail)                        \
      return -1;                                                             \
    if (a == slist->slh_tail || b == slist->slh_head)                        \
      return 1;                                                              \
    return slist->cmp(slist, a, b, aux);                                     \
  }                                                                          \
                                                                             \
  /* -- __skip_toss_ */                                                      \
  static int __skip_toss_##decl(size_t max, size_t fanout)                   \
  {                                                                          \
    size_t level = 0;                                                        \
    while (level + 1 < max) {                                                \
      if (rand() % fanout == 0) /* NOLINT(*-msc50-cpp) */                    \
        level++;                                                             \
      else                                                                   \
        break;                                                               \
    }                                                                        \
    return level;                                                            \
  }                                                                          \
                                                                             \
  /* -- skip_alloc_node_ */                                                  \
  int prefix##skip_alloc_node_##decl(decl##_t *slist, decl##_node_t **node)  \
  {                                                                          \
    decl##_node_t *n;                                                        \
    n = (decl##_node_t *)calloc(1, sizeof(decl##_node_t));                   \
    ARRAY_ALLOC(n->field.sle_next, struct decl##_node, slist->max);          \
    if (n && n->field.sle_next) {                                            \
      ARRAY_SET_SIZE(n->field.sle_next, slist->max);                         \
      ARRAY_SET_LENGTH(n->field.sle_next, 0);                                \
      *node = n;                                                             \
      return 0;                                                              \
    }                                                                        \
    return ENOMEM;                                                           \
  }                                                                          \
                                                                             \
  /* -- skip_init_                                                           \
   * max: 12, fanout: 4 are good defaults.                                   \
   */                                                                        \
  int prefix##skip_init_##decl(decl##_t *slist, size_t max, size_t fanout,   \
    int (*cmp)(struct decl *, decl##_node_t *, decl##_node_t *, void *))     \
  {                                                                          \
    int rc = 0;                                                              \
    slist->level = 0;                                                        \
    slist->length = 0;                                                       \
    slist->max = max;                                                        \
    slist->fanout = fanout;                                                  \
    slist->cmp = cmp;                                                        \
    rc = prefix##skip_alloc_node_##decl(slist, &slist->slh_head);            \
    if (rc)                                                                  \
      goto fail;                                                             \
    rc = prefix##skip_alloc_node_##decl(slist, &slist->slh_tail);            \
    if (rc)                                                                  \
      goto fail;                                                             \
    ARRAY_SET_LENGTH(slist->slh_head->field.sle_next, max);                  \
    for (size_t __i = 0; __i < max; __i++)                                   \
      slist->slh_head->field.sle_next[__i] = slist->slh_tail;                \
    slist->slh_head->field.sle_prev = NULL;                                  \
    ARRAY_SET_LENGTH(slist->slh_tail->field.sle_next, max);                  \
    for (size_t __i = 0; __i < max; __i++)                                   \
      slist->slh_tail->field.sle_next[__i] = NULL;                           \
    slist->slh_head->field.sle_prev = slist->slh_tail;                       \
  fail:;                                                                     \
    return rc;                                                               \
  }                                                                          \
                                                                             \
  /* -- skip_free_node_  */                                                  \
  void prefix##skip_free_node_##decl(decl##_node_t *node)                    \
  {                                                                          \
    free_node_blk;                                                           \
    ARRAY_FREE(node->field.sle_next);                                        \
    free(node);                                                              \
  }                                                                          \
                                                                             \
  /* -- skip_size_ */                                                        \
  int prefix##skip_size_##decl(decl##_t *slist)                              \
  {                                                                          \
    return slist->length;                                                    \
  }                                                                          \
                                                                             \
  /* -- skip_empty_ */                                                       \
  int prefix##skip_empty_##decl(decl##_t *slist)                             \
  {                                                                          \
    return slist->length == 0;                                               \
  }                                                                          \
                                                                             \
  /* -- skip_head_ */                                                        \
  decl##_node_t *prefix##skip_head_##decl(decl##_t *slist)                   \
  {                                                                          \
    return slist->slh_head->field.sle_next[0];                               \
  }                                                                          \
                                                                             \
  /* -- skip_tail_ */                                                        \
  decl##_node_t *prefix##skip_tail_##decl(decl##_t *slist)                   \
  {                                                                          \
    return slist->slh_tail->field.sle_prev;                                  \
  }                                                                          \
                                                                             \
  /* -- skip_locate_                                                         \
   * Locates a node that matches another node returning the path to that     \
   * node and the match in path[0].                                          \
   */                                                                        \
  decl##_node_t **prefix##skip_locate_##decl(decl##_t *slist,                \
    decl##_node_t *n)                                                        \
  {                                                                          \
    unsigned int i;                                                          \
    decl##_node_t **path;                                                    \
    decl##_node_t *elm = slist->slh_head;                                    \
                                                                             \
    if (slist == NULL || n == NULL)                                          \
      return NULL;                                                           \
                                                                             \
    i = slist->max + 1;                                                      \
    ARRAY_ALLOC(path, decl##_node_t, i);                                     \
    ARRAY_SET_LENGTH(path, 1);                                               \
    i = slist->level;                                                        \
    if (path == NULL)                                                        \
      return NULL;                                                           \
                                                                             \
    /* Find the node that matches `node` or NULL. */                         \
    do {                                                                     \
      while (elm &&                                                          \
        __skip_key_compare_##decl(slist, elm->field.sle_next[i], n,          \
          slist->aux) < 0)                                                   \
        elm = elm->field.sle_next[i];                                        \
      path[i + 1] = elm;                                                     \
      ARRAY_SET_LENGTH(path, ARRAY_LENGTH(path) + 1);                        \
    } while (i--);                                                           \
    elm = elm->field.sle_next[0];                                            \
    if (__skip_key_compare_##decl(slist, elm, n, slist->aux) == 0) {         \
      path[0] = elm;                                                         \
      return path;                                                           \
    }                                                                        \
    ARRAY_FREE(path);                                                        \
    return NULL;                                                             \
  }                                                                          \
                                                                             \
  /* -- __skip_insert_ */                                                    \
  static int __skip_insert_##decl(decl##_t *slist, decl##_node_t *n,         \
    int flags)                                                               \
  {                                                                          \
    unsigned int i;                                                          \
    decl##_node_t *prev, *elm = slist->slh_head;                             \
    decl##_node_t **path;                                                    \
                                                                             \
    if (n == NULL)                                                           \
      return ENOENT;                                                         \
                                                                             \
    i = slist->level;                                                        \
    ARRAY_ALLOC(path, decl##_node_t, slist->max);                            \
    if (path == NULL)                                                        \
      return ENOMEM;                                                         \
                                                                             \
    /* Find the position in the list where this element belongs. */          \
    do {                                                                     \
      while (elm &&                                                          \
        __skip_key_compare_##decl(slist, elm->field.sle_next[i], n,          \
          slist->aux) < 0)                                                   \
        elm = elm->field.sle_next[i];                                        \
      path[i] = elm;                                                     \
      ARRAY_SET_LENGTH(path, ARRAY_LENGTH(path) + 1);                        \
    } while (i--);                                                           \
    i = 0;                                                                   \
    prev = elm;                                                              \
    elm = elm->field.sle_next[0];                                            \
    if (__skip_key_compare_##decl(slist, elm, n, slist->aux) == 0) {         \
      if (flags == 0) {                                                      \
        /* Don't overwrite, to do that use _REPLACE not _INSERT */           \
        ARRAY_FREE(path);                                                    \
        return -1;                                                           \
      }                                                                      \
    }                                                                        \
    size_t level = __skip_toss_##decl(slist->max, slist->fanout);            \
    ARRAY_SET_LENGTH(n->field.sle_next, level);                              \
    if (level > slist->level) {                                              \
      for (i = slist->level + 1; i <= level; i++) {                              \
        path[i] = slist->slh_tail;                                           \
      }                                                                      \
      slist->level = level;                                                  \
    }                                                                        \
    for (i = 0; i <= level; i++) {                                            \
      n->field.sle_next[i] = path[i]->field.sle_next[i];                     \
      path[i]->field.sle_next[i] = n;                                        \
    }                                                                        \
    n->field.sle_prev = prev;                                                \
    if (n->field.sle_next[0] == slist->slh_tail) {                           \
      slist->slh_tail->field.sle_prev = n;                                   \
    }                                                                        \
    slist->length++;                                                         \
    ARRAY_FREE(path);                                                        \
    return 0;                                                                \
  }                                                                          \
                                                                             \
  /* -- skip_insert_ */                                                      \
  int prefix##skip_insert_##decl(decl##_t *slist, decl##_node_t *n)          \
  {                                                                          \
    return __skip_insert_##decl(slist, n, 0);                                \
  }                                                                          \
                                                                             \
  /* -- skip_insert_dup_ */                                                  \
  int prefix##skip_insert_dup_##decl(decl##_t *slist, decl##_node_t *n)      \
  {                                                                          \
    return __skip_insert_##decl(slist, n, 1);                                \
  }                                                                          \
                                                                             \
  /* -- skip_find_                                                           \
   * Find a node that matches another node.  This differs from the locate()  \
   * API in that it does not return the path to the node, only the match.    \
   * This avoids an alloc/free for the path.                                 \
   */                                                                        \
  decl##_node_t *prefix##skip_find_##decl(decl##_t *slist, decl##_node_t *n) \
  {                                                                          \
    unsigned int i;                                                          \
    decl##_node_t *elm = slist->slh_head;                                    \
                                                                             \
    if (slist == NULL || n == NULL)                                          \
      return NULL;                                                           \
                                                                             \
    i = slist->level;                                                        \
                                                                             \
    do {                                                                     \
      while (elm &&                                                          \
        __skip_key_compare_##decl(slist, elm->field.sle_next[i], n,          \
          slist->aux) < 0)                                                   \
        elm = elm->field.sle_next[i];                                        \
    } while (i--);                                                           \
    elm = elm->field.sle_next[0];                                            \
    if (__skip_key_compare_##decl(slist, elm, n, slist->aux) == 0) {         \
      return elm;                                                            \
    }                                                                        \
    return NULL;                                                             \
  }                                                                          \
                                                                             \
  /* -- skip_find_gte                                                        \
   * Return the matching node or the next greater node after that.           \
   */                                                                        \
  decl##_node_t *prefix##skip_find_gte_##decl(decl##_t *slist,               \
    decl##_node_t *n)                                                        \
  {                                                                          \
    int cmp;                                                                 \
    unsigned int i;                                                          \
    decl##_node_t *elm = slist->slh_head;                                    \
                                                                             \
    if (slist == NULL || n == NULL)                                          \
      return NULL;                                                           \
                                                                             \
    i = slist->level;                                                        \
                                                                             \
    do {                                                                     \
      while (elm &&                                                          \
        __skip_key_compare_##decl(slist, elm->field.sle_next[i], n,          \
          slist->aux) < 0)                                                   \
        elm = elm->field.sle_next[i];                                        \
    } while (i--);                                                           \
    do {                                                                     \
      elm = elm->field.sle_next[0];                                          \
      cmp = __skip_key_compare_##decl(slist, elm, n, slist->aux);            \
    } while (cmp < 0);                                                       \
    return elm;                                                              \
  }                                                                          \
                                                                             \
  /* -- skip_find_lte                                                        \
   * Return the matching node or the last one before it.                     \
   */                                                                        \
  decl##_node_t *prefix##skip_find_lte_##decl(decl##_t *slist,               \
    decl##_node_t *n)                                                        \
  {                                                                          \
    int cmp;                                                                 \
    unsigned int i;                                                          \
    decl##_node_t *elm = slist->slh_head;                                    \
                                                                             \
    if (slist == NULL || n == NULL)                                          \
      return NULL;                                                           \
                                                                             \
    i = slist->level;                                                        \
                                                                             \
    do {                                                                     \
      while (elm &&                                                          \
        __skip_key_compare_##decl(slist, elm->field.sle_next[i], n,          \
          slist->aux) < 0)                                                   \
        elm = elm->field.sle_next[i];                                        \
    } while (i--);                                                           \
    elm = elm->field.sle_next[0];                                            \
    if (__skip_key_compare_##decl(slist, elm, n, slist->aux) == 0) {         \
      return elm;                                                            \
    } else {                                                                 \
      do {                                                                   \
        elm = elm->field.sle_prev;                                           \
        cmp = __skip_key_compare_##decl(slist, elm, n, slist->aux);          \
      } while (cmp > 0);                                                     \
    }                                                                        \
    return elm;                                                              \
  }                                                                          \
                                                                             \
  /* -- skip_update_                                                         \
   * Locates a node in the list that equals the `new` node and then          \
   * uses the `update_node_blk` to update the contents.                      \
   * WARNING: Do not update the portion of the node used for ordering        \
   * (e.g. `key`) unless you really know what you're doing.                  \
   */                                                                        \
  int prefix##skip_update_##decl(decl##_t *slist, decl##_node_t *new)        \
  {                                                                          \
    decl##_node_t *node;                                                     \
                                                                             \
    if (slist == NULL || new == NULL)                                        \
      return -1;                                                             \
                                                                             \
    node = prefix##skip_find_##decl(slist, new);                             \
    if (node) {                                                              \
      update_node_blk;                                                       \
      return 0;                                                              \
    }                                                                        \
    return -1;                                                               \
  }                                                                          \
                                                                             \
  /* -- skip_remove_ */                                                      \
  int prefix##skip_remove_##decl(decl##_t *slist, decl##_node_t *n)          \
  {                                                                          \
    size_t i, s;                                                             \
    decl##_node_t **path, *node;                                             \
                                                                             \
    if (slist == NULL || n == NULL)                                          \
      return -1;                                                             \
    if (slist->length == 0)                                                  \
      return 0;                                                              \
                                                                             \
    path = prefix##skip_locate_##decl(slist, n);                             \
    s = ARRAY_LENGTH(path);                                                  \
    node = path[0];                                                          \
    if (s > 0) {                                                             \
      node->field.sle_next[0]->field.sle_prev = node->field.sle_prev;        \
      for (i = 1; i < s; i++) {                                              \
        if (path[i]->field.sle_next[i - 1] != node)                          \
          break;                                                             \
        path[i]->field.sle_next[i - 1] = node->field.sle_next[i - 1];        \
        if (path[i]->field.sle_next[i - 1] == slist->slh_tail) {             \
          size_t h = ARRAY_LENGTH(path[i]->field.sle_next);                  \
          ARRAY_SET_LENGTH(path[i]->field.sle_next, h - 1);                  \
        }                                                                    \
      }                                                                      \
      ARRAY_FREE(path);                                                      \
      free_node_blk;                                                         \
      /* Find all levels in the first element in the list that point         \
         at the tail and shrink the level. */                                \
      while (slist->level > 0 &&                                             \
        slist->slh_head->field.sle_next[slist->level] == slist->slh_tail) {  \
        slist->level--;                                                      \
      }                                                                      \
      slist->length--;                                                       \
    }                                                                        \
    return 0;                                                                \
  }                                                                          \
                                                                             \
  /* -- skip_next_node_ */                                                   \
  decl##_node_t *prefix##skip_next_node_##decl(decl##_t *slist,              \
    decl##_node_t *n)                                                        \
  {                                                                          \
    if (slist == NULL || n == NULL)                                          \
      return NULL;                                                           \
    if (n->field.sle_next[0] == slist->slh_tail)                             \
      return NULL;                                                           \
    return n->field.sle_next[0];                                             \
  }                                                                          \
                                                                             \
  /* -- skip_prev_node_ */                                                   \
  decl##_node_t *prefix##skip_prev_node_##decl(decl##_t *slist,              \
    decl##_node_t *n)                                                        \
  {                                                                          \
    if (slist == NULL || n == NULL)                                          \
      return NULL;                                                           \
    if (n->field.sle_prev == slist->slh_head)                                \
      return NULL;                                                           \
    return n->field.sle_prev;                                                \
  }                                                                          \
                                                                             \
  /* -- skip_destroy_ */                                                     \
  int prefix##skip_destroy_##decl(decl##_t *slist)                           \
  {                                                                          \
    decl##_node_t *node, *next;                                              \
    if (slist == NULL)                                                       \
      return 0;                                                              \
    if (prefix##skip_size_##decl(slist) == 0)                                \
      return 0;                                                              \
    node = prefix##skip_head_##decl(slist);                                  \
    do {                                                                     \
      next = prefix##skip_next_node_##decl(slist, node);                     \
      prefix##skip_free_node_##decl(node);                                   \
      node = next;                                                           \
    } while (node != NULL);                                                  \
                                                                             \
    ARRAY_FREE(slist->slh_head->field.sle_next);                             \
    free(slist->slh_head);                                                   \
    ARRAY_FREE(slist->slh_tail->field.sle_next);                             \
    free(slist->slh_tail);                                                   \
    return 0;                                                                \
  }                                                                          \
                                                                             \
  /* -- skip_snapshot_                                                       \
   * A snapshot is a read-only view of a Skiplist at a point in              \
   * time.  Once taken, a snapshot must be restored or disposed.             \
   * Any number of snapshots can be created.                                 \
   */                                                                        \
  int prefix##skip_snapshot_##decl(decl##_t *slist)                          \
  {                                                                          \
    ((void)slist); /* TODO */                                                \
    return 0;                                                                \
  }                                                                          \
                                                                             \
  /* -- skip_restore_snapshot_ */                                            \
  int prefix##skip_restore_snapshot_##decl(decl##_t *slist)                  \
  {                                                                          \
    ((void)slist); /* TODO */                                                \
    return 0;                                                                \
  }                                                                          \
                                                                             \
  /* -- skip_dispose_snapshot_ */                                            \
  int prefix##skip_dispose_snapshot_##decl(decl##_t *slist)                  \
  {                                                                          \
    ((void)slist); /* TODO */                                                \
    return 0;                                                                \
  }                                                                          \
                                                                             \
  /* -- __skip_integrity_check_ */                                           \
  static int __skip_integrity_check_##decl(decl##_t *slist)                  \
  {                                                                          \
    ((void)slist); /* TODO */                                                \
    return 0;                                                                \
  }

#define SKIPLIST_GETTERS(decl, prefix, ktype, vtype, qblk, rblk) \
  vtype prefix##skip_get_##decl(decl##_t *slist, ktype key)      \
  {                                                              \
    decl##_node_t *node, query;                                  \
                                                                 \
    qblk;                                                        \
    node = prefix##skip_find_##decl(slist, &query);              \
    if (node) {                                                  \
      rblk;                                                      \
    }                                                            \
    return (vtype)0;                                             \
  }                                                              \
  vtype prefix##skip_gte_##decl(decl##_t *slist, ktype key)      \
  {                                                              \
    decl##_node_t *node, query;                                  \
                                                                 \
    qblk;                                                        \
    node = prefix##skip_find_gte_##decl(slist, &query);          \
    if (node != slist->slh_tail) {                               \
      rblk;                                                      \
    }                                                            \
    return (vtype)0;                                             \
  }                                                              \
  vtype prefix##skip_lte_##decl(decl##_t *slist, ktype key)      \
  {                                                              \
    decl##_node_t *node, query;                                  \
                                                                 \
    qblk;                                                        \
    node = prefix##skip_find_lte_##decl(slist, &query);          \
    if (node != slist->slh_head) {                               \
      rblk;                                                      \
    }                                                            \
    return (vtype)0;                                             \
  }

#define SKIPLIST_DECL_DOT(decl, prefix, field)                                 \
                                                                               \
  /* A type for a function that writes into a char[2048] buffer                \
   * a description of the value within the node. */                            \
  typedef void (*skip_sprintf_node_##decl##_t)(decl##_node_t *, char *);       \
                                                                               \
  /* -- __skip_dot_node_                                                       \
   * Writes out a fragment of a DOT file representing a node.                  \
   */                                                                          \
  static void __skip_dot_node_##decl(FILE *os, decl##_t *slist,                \
    decl##_node_t *node, size_t nsg, skip_sprintf_node_##decl##_t fn)          \
  {                                                                            \
    char buf[2048];                                                            \
    size_t level, height = ARRAY_LENGTH(node->field.sle_next) - 1;             \
    decl##_node_t *next;                                                       \
                                                                               \
    fprintf(os, "\"node%zu %p\"", nsg, (void *)node);                          \
    fprintf(os, " [label = \"");                                               \
    level = height;                                                            \
    do {                                                                       \
      fprintf(os, " { <w%zu> | <f%zu> %p } |", level, level,                   \
        (void *)node->field.sle_next[level]);                                  \
    } while (level--);                                                         \
    if (fn) {                                                                  \
      fn(node, buf);                                                           \
      fprintf(os, " <f0> %s\"\n", buf);                                        \
    } else {                                                                   \
      fprintf(os, " <f0> ?\"\n");                                              \
    }                                                                          \
    fprintf(os, "shape = \"record\"\n");                                       \
    fprintf(os, "];\n");                                                       \
                                                                               \
    /* Now edges */                                                            \
    level = 0;                                                                 \
    for (level = 0; level < height; level++) {                                 \
      fprintf(os, "\"node%zu %p\"", nsg, (void *)node);                        \
      fprintf(os, ":f%zu -> ", level);                                         \
      fprintf(os, "\"node%zu %p\"", nsg, (void *)node->field.sle_next[level]); \
      fprintf(os, ":w%zu [];\n", level);                                       \
    }                                                                          \
    next = prefix##skip_next_node_##decl(slist, node);                         \
    if (next)                                                                  \
      __skip_dot_node_##decl(os, slist, next, nsg, fn);                        \
  }                                                                            \
                                                                               \
  /* -- __skip_dot_finish_                                                     \
   * Finalize the DOT file of the internal representation.                     \
   */                                                                          \
  static void __skip_dot_finish_##decl(FILE *os, size_t nsg)                   \
  {                                                                            \
    size_t i;                                                                  \
    if (nsg > 0) {                                                             \
      /* Link the nodes together with an invisible node.                       \
       *    node0 [shape=record, label = "<f0> | <f1> | <f2> | <f3> |          \
       * <f4> | <f5> | <f6> | <f7> | <f8> | ", style=invis, width=0.01];       \
       */                                                                      \
      fprintf(os, "node0 [shape=record, label = \"");                          \
      for (i = 0; i < nsg; ++i) {                                              \
        fprintf(os, "<f%zu> | ", i);                                           \
      }                                                                        \
      fprintf(os, "\", style=invis, width=0.01];\n");                          \
                                                                               \
      /* Now connect nodes with invisible edges                                \
       *                                                                       \
       *    node0:f0 -> HeadNode [style=invis];                                \
       *    node0:f1 -> HeadNode1 [style=invis];                               \
       */                                                                      \
      for (i = 0; i < nsg; ++i) {                                              \
        fprintf(os, "node0:f%zu -> HeadNode%zu [style=invis];\n", i, i);       \
      }                                                                        \
      nsg = 0;                                                                 \
    }                                                                          \
  }                                                                            \
                                                                               \
  /* -- skip_dot_start_ */                                                     \
  static int __skip_dot_start_##decl(FILE *os, decl##_t *slist, size_t nsg,    \
    skip_sprintf_node_##decl##_t fn)                                           \
  {                                                                            \
    size_t level;                                                              \
    decl##_node_t *head;                                                       \
    if (nsg == 0) {                                                            \
      fprintf(os, "digraph Skiplist {\n");                                     \
      fprintf(os, "label = \"Skiplist.\"\n");                                  \
      fprintf(os, "graph [rankdir = \"LR\"];\n");                              \
      fprintf(os, "node [fontsize = \"12\" shape = \"ellipse\"];\n");          \
      fprintf(os, "edge [];\n\n");                                             \
    }                                                                          \
    fprintf(os, "subgraph cluster%zu {\n", nsg);                               \
    fprintf(os, "style=dashed\n");                                             \
    fprintf(os, "label=\"Skip list iteration %zu\"\n\n", nsg);                 \
    fprintf(os, "\"HeadNode%zu\" [\n", nsg);                                   \
    fprintf(os, "label = \"");                                                 \
                                                                               \
    /* Write out the fields */                                                 \
    head = slist->slh_head;                                                    \
    if (SKIP_EMPTY(slist))                                                     \
      fprintf(os, "Empty HeadNode");                                           \
    else {                                                                     \
      level = ARRAY_LENGTH(head->field.sle_next) - 1;                          \
      do {                                                                     \
        decl##_node_t *node = head->field.sle_next[level];                     \
        fprintf(os, "{ <f%zu> %p }", level, (void *)node);                     \
        if (level)                                                             \
          fprintf(os, " | ");                                                  \
      } while (level--);                                                       \
    }                                                                          \
    fprintf(os, "\"\n");                                                       \
    fprintf(os, "shape = \"record\"\n");                                       \
    fprintf(os, "];\n");                                                       \
                                                                               \
    /* Edges for head node */                                                  \
    decl##_node_t *node = slist->slh_head;                                     \
    level = 0;                                                                 \
    do {                                                                       \
      fprintf(os, "\"HeadNode%zu\":f%zu -> ", nsg, level);                     \
      fprintf(os, "\"node%zu %p\"", nsg, (void *)node->field.sle_next[level]); \
      fprintf(os, ":w%zu [];\n", level);                                       \
    } while (++level < slist->level);                                          \
    fprintf(os, "}\n\n");                                                      \
                                                                               \
    /* Now all nodes via level 0, if non-empty */                              \
    node = prefix##skip_head_##decl(slist);                                    \
    if (node)                                                                  \
      __skip_dot_node_##decl(os, slist, node->field.sle_next[0], nsg, fn);     \
    fprintf(os, "\n");                                                         \
                                                                               \
    /* The tail, sentinal node */                                              \
    if (!SKIP_EMPTY(slist)) {                                                  \
      fprintf(os, "\"node%zu 0x0\" [label = \"", nsg);                         \
      level = slist->level;                                                    \
      do {                                                                     \
        fprintf(os, "<w%zu> NULL", level);                                     \
        if (level)                                                             \
          fprintf(os, " | ");                                                  \
      } while (--level);                                                       \
      fprintf(os, "\" shape = \"record\"];\n");                                \
    }                                                                          \
                                                                               \
    /* End: "subgraph cluster0 {" */                                           \
    fprintf(os, "}\n\n");                                                      \
    nsg += 1;                                                                  \
                                                                               \
    return nsg;                                                                \
  }                                                                            \
                                                                               \
  /* -- skip_dot_                                                              \
   * Create a DOT file of the internal representation of the                   \
   * Skiplist on the provided file descriptor (default: STDOUT).               \
   *                                                                           \
   * To view the output:                                                       \
   * $ dot -Tps filename.dot -o outfile.ps                                     \
   * You can change the output format by varying the value after -T and        \
   * choosing an appropriate filename extension after -o.                      \
   * See: https://graphviz.org/docs/outputs/ for the format options.           \
   *                                                                           \
   * https://en.wikipedia.org/wiki/DOT_(graph_description_language)            \
   */                                                                          \
  int prefix##skip_dot_##decl(FILE *os, decl##_t *slist, size_t nsg,           \
    skip_sprintf_node_##decl##_t fn)                                           \
  {                                                                            \
    if (__skip_integrity_check_##decl(slist) != 0) {                           \
      perror("Skiplist failed integrity checks, impossible to diagram.");      \
      return -1;                                                               \
    }                                                                          \
    if (os == NULL)                                                            \
      os = stdout;                                                             \
    if (!os) {                                                                 \
      perror("Failed to open output file, unable to write DOT file.");         \
      return -1;                                                               \
    }                                                                          \
    __skip_dot_start_##decl(os, slist, nsg, fn);                               \
    __skip_dot_finish_##decl(os, nsg);                                         \
    return 0;                                                                  \
  }

#endif /* _SKIPLIST_H_ */
