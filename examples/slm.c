#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include "../include/sl.h"

/*
 * SKIPLIST EXAMPLE:
 *
 * This example creates a Skiplist keys and values are integers.
 * 'slex' - meaning: SkipList EXample
 */

/* Create a type of Skiplist that maps int -> int. */
struct slex_node {
  int key;
  int value;
  SKIP_ENTRY(slex_node) entries;
};
SKIP_HEAD(slex, slex_node);

/* Generate all the access functions for our type of Skiplist. */
SKIP_DECL(slex, api_, entries, {
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
