/*-
 * skiplist - a concurrent skip-list in ansi-c (C99)
 *
 * Copyright (C) 2024- Gregory Burd <greg@burd.me> - All rights reserved.
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <assert.h>
#include <sched.h>
#include <stdlib.h>

#include "skiplist.h"

#define __SL_DEBUG 0

#define __SLD_RT_INS(e, n, t, c) ((void)0)
#define __SLD_NC_INS(n, nn, t, c) ((void)0)
#define __SLD_RT_RMV(e, n, t, c) ((void)0)
#define __SLD_NC_RMV(n, nn, t, c) ((void)0)
#define __SLD_BM(n) ((void)0)
#define __SLD_ASSERT(cond) ((void)0)
#define __SLD_P(...) ((void)0)
#define __SLD_(b) ((void)0)

#define YIELD() sched_yield()

#if 0
#include <stdbool.h>
#endif
typedef uint8_t bool;
#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif

/* Atomic operations:
 * __ATOMIC_RELAXED implies no inter-thread ordering constraints
 */
#define ATM_GET(var) (var)
#define ATM_LOAD(var, val) __atomic_load(&(var), &(val), __ATOMIC_RELAXED)
#define ATM_STORE(var, val) __atomic_store(&(var), &(val), __ATOMIC_RELAXED)
#define ATM_CAS(var, exp, val)                                           \
  __atomic_compare_exchange(&(var), &(exp), &(val), 1, __ATOMIC_RELAXED, \
    __ATOMIC_RELAXED)
#define ATM_FETCH_ADD(var, val) \
  __atomic_fetch_add(&(var), (val), __ATOMIC_RELAXED)
#define ATM_FETCH_SUB(var, val) \
  __atomic_fetch_sub(&(var), (val), __ATOMIC_RELAXED)

/*
 * __sl_node_init --
 *  Erase and reset node information.  Free and allocate a new
 *  next node.
 */
static void
__sl_node_init(sl_node *node, size_t top_layer)
{
  bool bool_val = false;

  if (top_layer > UINT8_MAX)
    top_layer = UINT8_MAX;

  assert(node->is_fully_linked == false);
  assert(node->being_modified == false);

  ATM_STORE(node->is_fully_linked, bool_val);
  ATM_STORE(node->being_modified, bool_val);
  ATM_STORE(node->removed, bool_val);

  if (node->top_layer != top_layer || node->next == NULL) {
    node->top_layer = top_layer;

    if (node->next)
      free(node->next);
    node->next = calloc(top_layer + 1, sizeof(atm_node_ptr));
  }
}

/*
 * sl_init --
 *   Initialize a new skip list, set the comparison function.
 *
 * PUBLIC: void sl_init __P((sl_raw *, sl_cmp_t *));
 */
void
sl_init(sl_raw *slist, sl_cmp_t *cmp_func)
{
  size_t layer;
  bool bool_val = true;

  slist->cmp_func = NULL;
  slist->aux = NULL;

  /* Fanout 4 + layer 12: 4^12 ~= upto 17M items under O(lg n) complexity.
     For +17M items, complexity will grow linearly: O(k lg n). */
  slist->fanout = 4;
  slist->max_layer = 12;
  slist->num_entries = 0;

  slist->layer_entries = calloc(slist->max_layer, sizeof(atm_uint32_t));
  slist->top_layer = 0;

  sl_init_node(&slist->head);
  sl_init_node(&slist->tail);

  __sl_node_init(&slist->head, slist->max_layer);
  __sl_node_init(&slist->tail, slist->max_layer);

  for (layer = 0; layer < slist->max_layer; ++layer) {
    slist->head.next[layer] = &slist->tail;
    slist->tail.next[layer] = NULL;
  }

  ATM_STORE(slist->head.is_fully_linked, bool_val);
  ATM_STORE(slist->tail.is_fully_linked, bool_val);
  slist->cmp_func = cmp_func;
}

/*
 * sl_free_node --
 *  Free and clear the next node in this node.
 *
 * PUBLIC: void sl_free_node __P((sl_node *));
 */
void
sl_free_node(sl_node *node)
{
  free(node->next);
  node->next = NULL;
}

/*
 * __sl_freenode --
 *  Free a node.
 *
 * PUBLIC: sl_free __P((sl_raw *));
 */
void
sl_free(sl_raw *slist)
{
  sl_free_node(&slist->head);
  sl_free_node(&slist->tail);

  free(slist->layer_entries);
  slist->layer_entries = NULL;

  slist->aux = NULL;
  slist->cmp_func = NULL;
}

/*
 * sl_init_node --
 *  Initialize a node.
 *
 * PUBLIC: sl_init_node __P((sl_node *));
 */
void
sl_init_node(sl_node *node)
{
  bool bool_false = false;

  node->next = NULL;

  ATM_STORE(node->is_fully_linked, bool_false);
  ATM_STORE(node->being_modified, bool_false);
  ATM_STORE(node->removed, bool_false);

  node->accessing_next = 0;
  node->top_layer = 0;
  node->ref_count = 0;
}

/*
 * sl_get_size --
 *  Returns the number of elements stored in the skip list.
 *
 * PUBLIC: sl_get_size __P((sl_raw *));
 */
size_t
sl_get_size(sl_raw *slist)
{
  uint32_t val;
  ATM_LOAD(slist->num_entries, val);
  return val;
}

/*
 * sl_get_default_config --
 *  Returns a raw config with the default/recommended configuration.
 *
 * PUBLIC: sl_get_default_config __P(());
 */
sl_raw_config
sl_get_default_config(void)
{
  sl_raw_config ret;
  ret.fanout = 4;
  ret.maxLayer = 12;
  ret.aux = NULL;
  return ret;
}

/*
 * sl_get_config --
 *  Returns a copy the config for the `slist` provided.
 *
 * PUBLIC: sl_get_config __P((sl_raw *slist));
 */
sl_raw_config
sl_get_config(sl_raw *slist)
{
  sl_raw_config ret;
  ret.fanout = slist->fanout;
  ret.maxLayer = slist->max_layer;
  ret.aux = slist->aux;
  return ret;
}

/*
 * sl_set_config --
 *  Sets the config for the `slist` provided to `config`.  This will free and
 *  allocate the layer_entries should that already exist.
 *
 * PUBLIC: sl_set_config __P((sl_raw *, sl_raw_config));
 */
void
sl_set_config(sl_raw *slist, sl_raw_config config)
{
  slist->fanout = config.fanout;

  slist->max_layer = config.maxLayer;
  if (slist->layer_entries)
    free(slist->layer_entries);
  slist->layer_entries = calloc(sizeof(atm_uint32_t), slist->max_layer);

  slist->aux = config.aux;
}

/*
 * __sl_cmp --
 *  A comparison function that first covers some boundary cases, and then
 *  calls the user supplied comparison function with the auxiliary data.
 */
static int
__sl_cmp(sl_raw *slist, sl_node *a, sl_node *b)
{
  if (a == b)
    return 0;
  if (a == &slist->head || b == &slist->tail)
    return -1;
  if (a == &slist->tail || b == &slist->head)
    return 1;
  return slist->cmp_func(a, b, slist->aux);
}

/*
 * __sl_valid_node --
 *  Atomically reads the state of a node and returns its validity.
 */
static bool
__sl_valid_node(sl_node *node)
{
  bool is_fully_linked = false;
  ATM_LOAD(node->is_fully_linked, is_fully_linked);
  return is_fully_linked;
}

/*
 * __sl_read_lock_an --
 *  Waits for writers to release locks and then atomically
 *  locks the `node` for reading.
 */
static void
__sl_read_lock_an(sl_node *node)
{
  for (;;) {
    /* Wait for active writer to release the lock. */
    uint32_t accessing_next = 0;
    ATM_LOAD(node->accessing_next, accessing_next);
    while (accessing_next & 0xfff00000) {
      YIELD();
      ATM_LOAD(node->accessing_next, accessing_next);
    }

    ATM_FETCH_ADD(node->accessing_next, 0x1);
    ATM_LOAD(node->accessing_next, accessing_next);
    if ((accessing_next & 0xfff00000) == 0) {
      return;
    }

    ATM_FETCH_SUB(node->accessing_next, 0x1);
  }
}

/*
 * __sl_read_unlock_an --
 *  Atomically releases the read lock on `node`.
 */
static void
__sl_read_unlock_an(sl_node *node)
{
  ATM_FETCH_SUB(node->accessing_next, 0x1);
}

/*
 * __sl_write_lock_an --
 *  Waits for writers to release locks and then atomically
 *  locks the `node` for writing.
 */
static void
__sl_write_lock_an(sl_node *node)
{
  for (;;) {
    /* Wait for active writer to release the lock. */
    uint32_t accessing_next = 0;
    ATM_LOAD(node->accessing_next, accessing_next);
    while (accessing_next & 0xfff00000) {
      YIELD();
      ATM_LOAD(node->accessing_next, accessing_next);
    }

    ATM_FETCH_ADD(node->accessing_next, 0x100000);
    ATM_LOAD(node->accessing_next, accessing_next);
    if ((accessing_next & 0xfff00000) == 0x100000) {
      /* Wait until there are no more readers. */
      while (accessing_next & 0x000fffff) {
        YIELD();
        ATM_LOAD(node->accessing_next, accessing_next);
      }
      return;
    }

    ATM_FETCH_SUB(node->accessing_next, 0x100000);
  }
}

/*
 * __sl_write_unlock_an --
 *  Atomically releases the write lock on `node`.
 */
static void
__sl_write_unlock_an(sl_node *node)
{
  ATM_FETCH_SUB(node->accessing_next, 0x100000);
}

/*
 * __sl_fnd_next --
 *  Provides a method to correctly move from node to node traversing the skip
 *  list.
 *
 *  NOTE:
 *   * This increases the `ref_count` of returned node. The caller is
 *     responsible for decreasing it.
 */
static sl_node *
__sl_fnd_next(sl_node *cur_node, size_t layer, sl_node *to_find, bool *found)
{
  size_t i, num_nodes = 0;
  sl_node *nodes[256], *temp, *next_node = NULL;

  /* Turn on `accessing_next`:
   * now `cur_node` is not removable from skiplist,
   * which means that `cur_node->next` will be consistent
   * until clearing `accessing_next`.
   */
  __sl_read_lock_an(cur_node);
  {
    if (!__sl_valid_node(cur_node)) {
      __sl_read_unlock_an(cur_node);
      return NULL;
    }
    ATM_LOAD(cur_node->next[layer], next_node);
    /* Increase ref count of `next_node`:
     * now `next_node` is not destroyable.
     *
     *   << Remaining issue >>
     * 1) initially: A -> B
     * 2) T1: call __sl_next(A):
     *        A.accessing_next := true;
     *        next_node := B;
     * ----- context switch happens here -----
     * 3) T2: insert C:
     *        A -> C -> B
     * 4) T2: and then erase B, and free B.
     *        A -> C    B(freed)
     * ----- context switch back again -----
     * 5) T1: try to do something with B,
     *        but crash happens.
     *
     * ... maybe resolved using RW spinlock (Aug 21, 2017).
     */
    assert(next_node);
    ATM_FETCH_ADD(next_node->ref_count, 1);
    assert(next_node->top_layer >= layer);
  }
  __sl_read_unlock_an(cur_node);

  while ((next_node && !__sl_valid_node(next_node)) || next_node == to_find) {
    if (found && to_find == next_node)
      *found = true;

    temp = next_node;
    __sl_read_lock_an(temp);
    {
      assert(next_node);
      if (!__sl_valid_node(temp)) {
        __sl_read_unlock_an(temp);
        ATM_FETCH_SUB(temp->ref_count, 1);
        next_node = NULL;
        break;
      }
      ATM_LOAD(temp->next[layer], next_node);
      ATM_FETCH_ADD(next_node->ref_count, 1);
      nodes[num_nodes++] = temp;
      assert(next_node->top_layer >= layer);
    }
    __sl_read_unlock_an(temp);
  }

  for (i = 0; i < num_nodes; ++i) {
    ATM_FETCH_SUB(nodes[i]->ref_count, 1);
  }

  return next_node;
}

/*
 * __sl_next --
 *  Provides a method to correctly move from node to node traversing the skip
 *  list.
 *
 *  NOTE:
 *   * This increases the `ref_count` of returned node. The caller is
 *     responsible for decreasing it.
 */
static sl_node *
__sl_next(sl_node *cur_node, size_t layer)
{
  return __sl_fnd_next(cur_node, layer, NULL, NULL);
}

/*
 * __sl_decide_top_layer --
 */
static size_t
__sl_decide_top_layer(sl_raw *slist)
{
  size_t layer = 0;
  while (layer + 1 < slist->max_layer) {
    /* coin flip */
    if (rand() % slist->fanout == 0) { /* NOLINT(*-msc50-cpp) */
      /* grow: 1/fanout probability */
      layer++;
    } else {
      /* stop: 1 - 1/fanout probability */
      break;
    }
  }
  return layer;
}

/*
 * __sl_clr_flags --
 */
static void
__sl_clr_flags(sl_node **node_arr, unsigned long start_layer,
  unsigned long top_layer)
{
  unsigned long layer;
  for (layer = start_layer; layer <= top_layer; ++layer) {
    if (layer == top_layer || node_arr[layer] != node_arr[layer + 1]) {
      bool exp = true;
      bool bool_false = false;
      if (!ATM_CAS(node_arr[layer]->being_modified, exp, bool_false)) {
        assert(0);
      }
    }
  }
}

/*
 * __sl_valid_prev_next --
 */
static bool
__sl_valid_prev_next(sl_node *prev, sl_node *next)
{
  return __sl_valid_node(prev) && __sl_valid_node(next);
}

/*
 * __sl_insert --
 */
static int
__sl_insert(sl_raw *slist, sl_node *node, bool no_dup)
{
  int i, cmp, error_code, cur_layer;
  size_t layer, top_layer, locked_layer, sl_top_layer;
  bool bool_true = true;
  sl_node *exp, *cur_node, *next_node, *next_node_again,
    *prev[SKIPLIST_MAX_LAYER], *next[SKIPLIST_MAX_LAYER];

  __SLD_(uint64_t tid; pthread_threadid_np(NULL, &tid);
         size_t tid_hash = (size_t)tid % 256; (void)tid_hash;);

  top_layer = __sl_decide_top_layer(slist);

  /* init node before insertion */
  __sl_node_init(node, top_layer);
  __sl_write_lock_an(node);

  __SLD_P("%02x ins %p begin\n", (int)tid_hash, node);

insert_retry:;
  cur_node = &slist->head;
  ATM_FETCH_ADD(cur_node->ref_count, 1);

  __SLD_(size_t nh = 0);
  __SLD_(static __thread sl_node * history[1024]; (void)history);

  sl_top_layer = slist->top_layer;
  if (top_layer > sl_top_layer)
    sl_top_layer = top_layer;
  for (cur_layer = sl_top_layer; cur_layer >= 0; --cur_layer) {
    do {
      __SLD_(history[nh++] = cur_node);

      next_node = __sl_next(cur_node, cur_layer);
      if (!next_node) {
        __sl_clr_flags(prev, cur_layer + 1, top_layer);
        ATM_FETCH_SUB(cur_node->ref_count, 1);
        YIELD();
        goto insert_retry;
      }
      cmp = __sl_cmp(slist, node, next_node);
      if (cmp > 0) {
        /* cur_node < next_node < node
           => move to next node */
        sl_node *temp = cur_node;
        cur_node = next_node;
        ATM_FETCH_SUB(temp->ref_count, 1);
        continue;
      } else {
        /* otherwise: cur_node < node <= next_node */
        ATM_FETCH_SUB(next_node->ref_count, 1);
      }

      if (no_dup && (cmp == 0)) {
        /* duplicate key is not allowed */
        __sl_clr_flags(prev, cur_layer + 1, top_layer);
        ATM_FETCH_SUB(cur_node->ref_count, 1);
        return -1;
      }

      if (cur_layer <= top_layer) {
        prev[cur_layer] = cur_node;
        next[cur_layer] = next_node;

        /* Both 'prev' and 'next' should be fully linked
           before insertion, and no other thread should
           not modify 'prev' at the same time. */
        error_code = 0;
        locked_layer = cur_layer + 1;

        /* check if prev node is duplicated with upper
         * layer */
        if (cur_layer < top_layer && prev[cur_layer] == prev[cur_layer + 1]) {
          /* duplicate
               => which means that
             'being_modified' flag is already true
               => do nothing */
        } else {
          bool expected = false;
          if (ATM_CAS(prev[cur_layer]->being_modified, expected, bool_true)) {
            locked_layer = cur_layer;
          } else {
            error_code = -1;
          }
        }

        if (error_code == 0 &&
          !__sl_valid_prev_next(prev[cur_layer], next[cur_layer])) {
          error_code = -2;
        }

        if (error_code != 0) {
          __SLD_RT_INS(error_code, node, top_layer, cur_layer);
          __sl_clr_flags(prev, locked_layer, top_layer);
          ATM_FETCH_SUB(cur_node->ref_count, 1);
          YIELD();
          goto insert_retry;
        }

        /* set current node's pointers */
        ATM_STORE(node->next[cur_layer], next[cur_layer]);

        /* check if `cur_node->next` has been changed
         * from `next_node` */
        next_node_again = __sl_next(cur_node, cur_layer);
        ATM_FETCH_SUB(next_node_again->ref_count, 1);
        if (next_node_again != next_node) {
          __SLD_NC_INS(cur_node, next_node, top_layer, cur_layer);
          /* clear including the current layer as
             we already set modification flag
             above */
          __sl_clr_flags(prev, cur_layer, top_layer);
          ATM_FETCH_SUB(cur_node->ref_count, 1);
          YIELD();
          goto insert_retry;
        }
      }

      /* non-bottom layer => go down */
      if (cur_layer)
        break;

      /* bottom layer => insertion succeeded
         change prev/next nodes' prev/next pointers from 0 ~
         top_layer */
      for (layer = 0; layer <= top_layer; ++layer) {
        // `accessing_next` works as a spin-lock.
        __sl_write_lock_an(prev[layer]);
        exp = next[layer];
        if (!ATM_CAS(prev[layer]->next[layer], exp, node)) {
          __SLD_P("%02x ASSERT ins %p[%d] -> %p (expected %p)\n", (int)tid_hash,
            prev[layer], cur_layer, ATM_GET(prev[layer]->next[layer]),
            next[layer]);
          assert(0);
        }
        __SLD_P("%02x ins %p[%d] -> %p -> %p\n", (int)tid_hash, prev[layer],
          layer, node, ATM_GET(node->next[layer]));
        __sl_write_unlock_an(prev[layer]);
      }

      /* now this node is fully linked */
      ATM_STORE(node->is_fully_linked, bool_true);

      /* allow removing next nodes */
      __sl_write_unlock_an(node);

      __SLD_P("%02x ins %p done\n", (int)tid_hash, node);

      ATM_FETCH_ADD(slist->num_entries, 1);
      ATM_FETCH_ADD(slist->layer_entries[node->top_layer], 1);
      for (i = slist->max_layer - 1; i >= 0; --i) {
        if (slist->layer_entries[i] > 0) {
          slist->top_layer = i;
          break;
        }
      }

      /* modification is done for all layers */
      __sl_clr_flags(prev, 0, top_layer);
      ATM_FETCH_SUB(cur_node->ref_count, 1);

      return 0;
    } while (cur_node != &slist->tail);
  }
  return 0;
}

/*
 * sl_insert --
 *  Insert the provided `node` into the `slist`, duplicates of the same key are
 *  allowed.
 *
 * PUBLIC: int sl_insert __P((sl_raw *, sl_node *));
 */
int
sl_insert(sl_raw *slist, sl_node *node)
{
  return __sl_insert(slist, node, false);
}

/*
 * sl_insert_nodup --
 *  Insert the provided `node` into the `slist`, duplicates are not allowed.
 *
 * PUBLIC: int sl_insert_nodup __P((sl_raw *, sl_node *));
 */
int
sl_insert_nodup(sl_raw *slist, sl_node *node)
{
  return __sl_insert(slist, node, true);
}

typedef enum {
  SM = -2,   /*   <   less than (smaller)    */
  SMEQ = -1, /*   <=  less than or equal     */
  EQ = 0,    /*   ==  equal                  */
  GTEQ = 1,  /*   >=  greater than or equal  */
  GT = 2     /*   >   greater than           */
} __sl_find_mode;

/*
 * __sl_find --
 *  Look for the key in the `query` within the `slist` and return
 *  a cursor to that node if found with mode `EQ`.
 *
 * NOTE:
 *   * This increases the `ref_count` of returned node. The caller is
 *     responsible for decreasing it.
 */
static sl_node *
_sl_find(sl_raw *slist, sl_node *query, __sl_find_mode mode)
{
  int cmp, cur_layer;
  sl_node *cur_node, *next_node, *temp;
  uint8_t sl_top_layer;

find_retry:;
  cur_node = &slist->head;
  ATM_FETCH_ADD(cur_node->ref_count, 1);

  __SLD_(size_t nh = 0);
  __SLD_(static __thread sl_node * history[1024]; (void)history);

  sl_top_layer = slist->top_layer;
  for (cur_layer = sl_top_layer; cur_layer >= 0; --cur_layer) {
    do {
      __SLD_(history[nh++] = cur_node);

      next_node = __sl_next(cur_node, cur_layer);
      if (!next_node) {
        ATM_FETCH_SUB(cur_node->ref_count, 1);
        YIELD();
        goto find_retry;
      }
      cmp = __sl_cmp(slist, query, next_node);
      if (cmp > 0) {
        /* cur_node < next_node < query
           => move to next node */
        temp = cur_node;
        cur_node = next_node;
        ATM_FETCH_SUB(temp->ref_count, 1);
        continue;
      } else if (-1 <= mode && mode <= 1 && cmp == 0) {
        /* cur_node < query == next_node ... return */
        ATM_FETCH_SUB(cur_node->ref_count, 1);
        return next_node;
      }

      /* otherwise: cur_node < query < next_node */
      if (cur_layer) {
        /* non-bottom layer => go down */
        ATM_FETCH_SUB(next_node->ref_count, 1);
        break;
      }

      /* bottom layer */
      if (mode < 0 && cur_node != &slist->head) {
        /* smaller mode */
        ATM_FETCH_SUB(next_node->ref_count, 1);
        return cur_node;
      } else if (mode > 0 && next_node != &slist->tail) {
        /* greater mode */
        ATM_FETCH_SUB(cur_node->ref_count, 1);
        return next_node;
      }
      /* otherwise: exact match mode OR not found */
      ATM_FETCH_SUB(cur_node->ref_count, 1);
      ATM_FETCH_SUB(next_node->ref_count, 1);
      return NULL;
    } while (cur_node != &slist->tail);
  }

  return NULL;
}

/*
 * sl_find --
 *  Position and return a cursor at the first node with a key
 *  equal to that provided in the `query`, otherwise NULL.
 *
 * PUBLIC: sl_node *sl_find __P((sl_raw *, sl_node *));
 */
sl_node *
sl_find(sl_raw *slist, sl_node *query)
{
  return _sl_find(slist, query, EQ);
}

/*
 * sl_find_smaller_or_equal --
 *  Position and return a cursor at the last node with a key less
 *  than or the first key equal to the key provided in the `query`,
 *  otherwise if nothing is less than or equal to the key, NULL.
 *
 * PUBLIC: sl_node *sl_find_smaller_or_equal __P((sl_raw *, sl_node *));
 */
sl_node *
sl_find_smaller_or_equal(sl_raw *slist, sl_node *query)
{
  return _sl_find(slist, query, SMEQ);
}

/*
 * __sl_find_greater_or_equal --
 *  Position and return a cursor at the first node with a key
 *  greater than, or the first node containing a key equal to,
 *  the key provided in `query`, otherwise if the largest key
 *  is less than the key in `query` return NULL.
 *
 * PUBLIC: sl_node *sl_find_greater_or_equal __P((sl_raw *, sl_node *));
 */
sl_node *
sl_find_greater_or_equal(sl_raw *slist, sl_node *query)
{
  return _sl_find(slist, query, GTEQ);
}

int
sl_erase_node_passive(sl_raw *slist, sl_node *node)
{
  int i, error_code, cmp, cur_layer, top_layer, locked_layer;
  bool bool_true = true, bool_false = false;
  bool removed = false, is_fully_linked = false, expected = false;
  bool found_node_to_erase = false;
  bool node_found;
  sl_node *prev[SKIPLIST_MAX_LAYER], *next[SKIPLIST_MAX_LAYER];
  sl_node *cur_node, *next_node, *next_node_again, *exp;

  __SLD_(uint64_t tid; pthread_threadid_np(NULL, &tid);
         size_t tid_hash = (size_t)tid % 256; (void)tid_hash;);

  top_layer = node->top_layer;

  ATM_LOAD(node->removed, removed);
  if (removed)
    return -1; /* already removed */

  if (!ATM_CAS(node->being_modified, expected, bool_true)) {
    /* already being modified ... cannot work on this node for now */
    __SLD_BM(node);
    return -2;
  }

  /* set removed flag first, so that reader cannot read this node */
  ATM_STORE(node->removed, bool_true);

  __SLD_P("%02x rmv %p begin\n", (int)tid_hash, node);

erase_node_retry:
  ATM_LOAD(node->is_fully_linked, is_fully_linked);
  if (!is_fully_linked) {
    /* already unlinked ... remove is done by other thread */
    ATM_STORE(node->removed, bool_false);
    ATM_STORE(node->being_modified, bool_false);
    return -3;
  }

  found_node_to_erase = false;
  (void)found_node_to_erase;
  cur_node = &slist->head;
  ATM_FETCH_ADD(cur_node->ref_count, 1);

  __SLD_(size_t nh = 0);
  __SLD_(static __thread sl_node * history[1024]; (void)history);

  cur_layer = slist->top_layer;
  for (; cur_layer >= 0; --cur_layer) {
    do {
      __SLD_(history[nh++] = cur_node);

      node_found = false;
      next_node = __sl_fnd_next(cur_node, cur_layer, node, &node_found);
      if (!next_node) {
        __sl_clr_flags(prev, cur_layer + 1, top_layer);
        ATM_FETCH_SUB(cur_node->ref_count, 1);
        YIELD();
        goto erase_node_retry;
      }

      /* unlike insert(), we should find exact position of `node` */
      cmp = __sl_cmp(slist, node, next_node);
      if (cmp > 0 || (cur_layer <= top_layer && !node_found)) {
        /* cur_node <= next_node < node
           => move to next node */
        sl_node *temp = cur_node;
        cur_node = next_node;
        __SLD_(if (cmp > 0) {
          int cmp2 = __sl_cmp(slist, cur_node, node);
          if (cmp2 > 0) {
            /* node < cur_node <= next_node:
               not found. */
            __sl_clr_flags(prev, cur_layer + 1, top_layer);
            ATM_FETCH_SUB(temp->ref_count, 1);
            ATM_FETCH_SUB(next_node->ref_count, 1);
            assert(0);
          }
        });
        ATM_FETCH_SUB(temp->ref_count, 1);
        continue;
      } else {
        /* otherwise: cur_node <= node <= next_node */
        ATM_FETCH_SUB(next_node->ref_count, 1);
      }

      if (cur_layer <= top_layer) {
        prev[cur_layer] = cur_node;
        /* 'next_node' and 'node' should not be
           the same, as 'removed' flag is already set. */
        assert(next_node != node);
        next[cur_layer] = next_node;

        /* check if prev node duplicates with upper layer */
        error_code = 0;
        locked_layer = cur_layer + 1;
        if (cur_layer < top_layer && prev[cur_layer] == prev[cur_layer + 1]) {
          /* duplicate with upper layer
             => which means that 'being_modified'
             flag is already true
             => do nothing. */
        } else {
          expected = false;
          if (ATM_CAS(prev[cur_layer]->being_modified, expected, bool_true)) {
            locked_layer = cur_layer;
          } else {
            error_code = -1;
          }
        }

        if (error_code == 0 &&
          !__sl_valid_prev_next(prev[cur_layer], next[cur_layer])) {
          error_code = -2;
        }

        if (error_code != 0) {
          __SLD_RT_RMV(error_code, node, top_layer, cur_layer);
          __sl_clr_flags(prev, locked_layer, top_layer);
          ATM_FETCH_SUB(cur_node->ref_count, 1);
          YIELD();
          goto erase_node_retry;
        }

        next_node_again = __sl_fnd_next(cur_node, cur_layer, node, NULL);
        ATM_FETCH_SUB(next_node_again->ref_count, 1);
        if (next_node_again != next[cur_layer]) {
          /* `next` pointer has been changed, retry */
          __SLD_NC_RMV(cur_node, next[cur_layer], top_layer, cur_layer);
          __sl_clr_flags(prev, cur_layer, top_layer);
          ATM_FETCH_SUB(cur_node->ref_count, 1);
          YIELD();
          goto erase_node_retry;
        }
      }
      if (cur_layer == 0)
        found_node_to_erase = true;
      /* go down */
      break;
    } while (cur_node != &slist->tail);
  }
  /* not exist in the skiplist, should not happen */
  assert(found_node_to_erase);
  /* bottom layer => removal succeeded, mark this node unlinked */
  __sl_write_lock_an(node);
  {
    ATM_STORE(node->is_fully_linked, bool_false);
  }
  __sl_write_unlock_an(node);

  /* change prev nodes' next pointer from 0 ~ top_layer */
  for (cur_layer = 0; cur_layer <= top_layer; ++cur_layer) {
    __sl_write_lock_an(prev[cur_layer]);
    exp = node;
    assert(exp != next[cur_layer]);
    assert(next[cur_layer]->is_fully_linked);
    if (!ATM_CAS(prev[cur_layer]->next[cur_layer], exp, next[cur_layer])) {
      __SLD_P("%02x ASSERT rmv %p[%d] -> %p (node %p)\n", (int)tid_hash,
        prev[cur_layer], cur_layer, ATM_GET(prev[cur_layer]->next[cur_layer]),
        node);
      assert(0);
    }
    assert(next[cur_layer]->top_layer >= cur_layer);
    __SLD_P("%02x rmv %p[%d] -> %p (node %p)\n", (int)tid_hash, prev[cur_layer],
      cur_layer, next[cur_layer], node);
    __sl_write_unlock_an(prev[cur_layer]);
  }

  __SLD_P("%02x rmv %p done\n", (int)tid_hash, node);

  ATM_FETCH_SUB(slist->num_entries, 1);
  ATM_FETCH_SUB(slist->layer_entries[node->top_layer], 1);
  for (i = slist->max_layer - 1; i >= 0; --i) {
    if (slist->layer_entries[i] > 0) {
      slist->top_layer = i;
      break;
    }
  }

  /* modification is done for all layers */
  __sl_clr_flags(prev, 0, top_layer);
  ATM_FETCH_SUB(cur_node->ref_count, 1);

  ATM_STORE(node->being_modified, bool_false);

  return 0;
}

int
sl_erase_node(sl_raw *slist, sl_node *node)
{
  int ret;
  do {
    ret = sl_erase_node_passive(slist, node);
    /* if ret == -2, other thread is accessing the same node
       at the same time, so try again */
  } while (ret == -2);
  return ret;
}

int
sl_erase(sl_raw *slist, sl_node *query)
{
  int ret;
  sl_node *found = sl_find(slist, query);

  if (!found)
    return -4; /* key not found */

  /* if ret == -2, other thread is accessing the same node
     at the same time, so try again */
  do
    ret = sl_erase_node_passive(slist, found);
  while (ret == -2);

  ATM_FETCH_SUB(found->ref_count, 1);
  return ret;
}

int
sl_is_valid_node(sl_node *node)
{
  return __sl_valid_node(node);
}

int
sl_is_safe_to_free(sl_node *node)
{
  uint16_t ref_count;

  if (node->accessing_next)
    return 0;
  if (node->being_modified)
    return 0;
  if (!node->removed)
    return 0;

  ref_count = 0;
  ATM_LOAD(node->ref_count, ref_count);
  if (ref_count)
    return 0;
  return 1;
}

void
sl_wait_for_free(sl_node *node)
{
  while (!sl_is_safe_to_free(node)) {
    YIELD();
  }
}

void
sl_grab_node(sl_node *node)
{
  ATM_FETCH_ADD(node->ref_count, 1);
}

void
sl_release_node(sl_node *node)
{
  assert(node->ref_count);
  ATM_FETCH_SUB(node->ref_count, 1);
}

sl_node *
sl_next(sl_raw *slist, sl_node *node)
{
  /*
   * << Issue >>
   * If `node` is already removed and its next node is also removed
   * and then released, the link update will not be applied to `node`
   * as it is already unreachable from skiplist. `node` still points to
   * the released node so that `__sl_next(node)` may return corrupted
   * memory region.
   *
   * 0) initial:
   *    A -> B -> C -> D
   *
   * 1) B is `node`, which is removed but not yet released:
   *    B --+-> C -> D
   *        |
   *    A --+
   *
   * 2) remove C, and then release:
   *    B -> !C!  +-> D
   *              |
   *    A --------+
   *
   * 3) sl_next(B):
   *    will fetch C, which is already released so that
   *    may contain garbage data.
   *
   * In this case, start over from the top layer,
   * to find valid link (same as in prev()).
   */

  sl_node *next = __sl_next(node, 0);
  if (!next)
    next = _sl_find(slist, node, GT);

  if (next == &slist->tail)
    return NULL;
  return next;
}

sl_node *
sl_prev(sl_raw *slist, sl_node *node)
{
  sl_node *prev = _sl_find(slist, node, SM);
  if (prev == &slist->head)
    return NULL;
  return prev;
}

sl_node *
sl_begin(sl_raw *slist)
{
  sl_node *next = NULL;
  while (!next) {
    next = __sl_next(&slist->head, 0);
  }
  if (next == &slist->tail)
    return NULL;
  return next;
}

sl_node *
sl_end(sl_raw *slist)
{
  return sl_prev(slist, &slist->tail);
}
