#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include "../include/sl.h"

/*
 * SKIPLIST EXAMPLE:
 *
 * This example creates a Skiplist keys and values are integers.
 */

/* Create a type of Skiplist that maps int -> int. */
struct entry {
  int key;
  int value;
  SKIP_ENTRY(entry) entries;
};
SKIP_HEAD(skip, entry);

/* Generate a function to compare keys to order our list.
 *
 * The function takes three arguments:
 *   - `a` and `b` of the provided type (`entry` in this case)
 *   - `aux` an additional auxiliary argument
 * and returns:
 *   a  < b : return -1
 *   a == b : return 0
 *   a  > b : return 1
 *
 * This function is only called when a and b are not NULL and valid
 * pointers to the entry type.
 *
 * The function will be named: __skip_cmp_<entry> which should be
 * passed as the 2nd argument in the SKIP_HEAD_INITIALIZER.
 */
  SKIP_COMPARATOR(skip, entry, {
    (void)aux;
    if (a->key < b->key)
      return -1;
    if (a->key > b->key)
      return 1;
    return 0;
  })

typedef struct skip skip_t;
int main() {
/* Allocate and initialize a Skiplist. */
#define STATIC_INIT
#ifdef STATIC_INIT
  skip_t _list = SKIP_HEAD_DEFAULT_INITIALIZER(__skip_cmp_entry);
  _list.slh_tail = (struct entry *)_list.slh_head; // TODO...
  skip_t *list = &_list;
#else /* Dynamic allocation, init. */
  skip_t *list = (skip_t *)malloc(sizeof(skip_t));
  SKIP_DEFAULT_INIT(list, __skip_cmp_entry);
#endif

  /* Insert 10 key/value pairs into the list. */
  for (int i = 0; i < 10; i++) {
    struct entry *n;
    SKIP_ALLOC_NODE(list, n, entry, entries);
    n->key = i;
    n->value = i;
    SKIP_INSERT(list, entry, n, entries);
  }

#if 0
  /* Delete a specific element in the list. */
  struct entry query;
  query.key = 4;
  struct entry *removed = SKIP_REMOVE(list, q, entries);
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
