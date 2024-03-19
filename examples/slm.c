#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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
SKIPLIST_DECL(slex, api_, entries)
int
__slm_key_compare(slex_t *list, slex_node_t *a, slex_node_t *b, void *aux)
{
  (void)list;
  (void)aux;
  if (a->key < b->key)
    return -1;
  if (a->key > b->key)
    return 1;
  return 0;
}

#define DOT
#ifdef DOT
/* Also declare the functions used to visualize a Sliplist (DOT/Graphviz) */
SKIPLIST_DECL_DOT(slex, api_, entries)

void
sprintf_slex_node(slex_node_t *node, char *buf)
{
  sprintf(buf, "%d:%d", node->key, node->value);
}
#endif

int
main()
{
  int rc = 0;
  /* Allocate and initialize a Skiplist. */
  slex_t *list = (slex_t *)malloc(sizeof(slex_t));
  if (list == NULL) {
    rc = ENOMEM;
    goto fail;
  }
  rc = api_skip_init_slex(list, 12, 4, __slm_key_compare);

  struct slex_node *n;

  /* Insert 7 key/value pairs into the list. */
  for (int i = -2; i <= 2; i++) {
    SKIP_ALLOC_NODE(list, n, slex_node, entries);
    n->key = i;
    n->value = i;
    api_skip_insert_slex(list, n);
  }

  FILE *of = fopen("/tmp/slm.dot", "w");
  if (!of) {
    perror("Failed to open file /tmp/slm.dot");
    return EXIT_FAILURE;
  }
  api_skip_dot_slex(of, list, sprintf_slex_node);
  fclose(of);

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
fail:;
  return rc;
}
