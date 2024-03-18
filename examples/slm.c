#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
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
  int value;
  SKIP_ENTRY(slex_node) entries;
};

/*
 * Generate all the access functions for our type of Skiplist.
 * The last argument to this macro is a block of code used to
 * compare the nodes you defined above.
 * This block can expect four arguments:
 *   - a reference to the Skiplist, `slist`
 *   - the two nodes to compare, `a` and `b`
 *   - and `aux`, which you can use to pass into this function
 *     any additional information required to compare objects.
 *     `aux` is passed from the value in the Skiplist, you can
 *     modify that value at any time to suit your needs.
 *
 * Your block should result in a return statement:
 *   a  < b : return -1
 *   a == b : return 0
 *   a  > b : return 1
 *
 * This result provides the ordering within the Skiplist.  Sometimes
 * your block of code will not be used when comparing nodes.  This
 * happens when `a` or `b` are references to the head or tail of the
 * list or when `a == b`.  In those cases the comparison function
 * returns before using the code in your block, don't panic. :)
 */
SKIPLIST_DECL(slex, api_, entries, {
    (void)aux;
    if (a->key < b->key)
        return -1;
    if (a->key > b->key)
        return 1;
    return 0;
})

int main() {
/* Allocate and initialize a Skiplist. */
    slex_t _list = SKIP_HEAD_DEFAULT_INITIALIZER(__skip_key_compare_slex);
  _list.slh_tail = (struct slex_node *)&_list.slh_head; // TODO...
    /* Dynamic allocation, init. */
  slex_t *list = (slex_t *)malloc(sizeof(slex_t));
  SKIP_DEFAULT_INIT(list, __skip_key_compare_slex, slex_node, entries);
#ifdef STATIC_INIT
  free(list);
  slex_t *list = &_list;
#else
#endif

  struct slex_node *n;
  SKIP_ALLOC_NODE(list, n, slex_node, entries);
  n->key = -1;
  n->value = -1;
  api_skip_insert_slex(list, n);

  /* Insert 10 key/value pairs into the list. */
  for (int i = 0; i < 10; i++) {
      SKIP_ALLOC_NODE(list, n, slex_node, entries);
      n->key = i;
      n->value = i;
      api_skip_insert_slex(list, n);
  }

#if 0
  /* Delete a specific element in the list. */
  struct slex_node query;
  query.key = 4;
  struct slex_node *removed = SKIP_REMOVE(list, q, entries);
  free(removed);
  /* Forward traversal. */
  SKIP_FOREACH(np, &head, entries)
    np-> ...
    /* Reverse traversal. */
    SKIP_FOREACH_REVERSE(np, &head, skiphead, entries)
    np-> ...
    /* Skiplist Deletion. */
    while (!SKIP_EMPTY(&head)) {
      n1 = SKIP_FIRST(&head);
      SKIP_REMOVE(&head, n1, entries);
      free(n1);
    }
  /* Faster Skiplist Deletion. */
  n1 = SKIP_FIRST(&head);
  while (n1 != NULL) {
    n2 = SKIP_NEXT(n1, entries);
    free(n1);
    n1 = n2;
  }
  SKIP_INIT(&head);
#endif
  return 0;
}
