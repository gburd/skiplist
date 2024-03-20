#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

/* Setting this will do two things:
 * 1) limit our max height across all instances of this datastructure.
 * 2) remove a heap allocation on frequently used paths, insert/remove/etc.
 * so, use it when you need it.
 */
#define SKIPLIST_MAX_HEIGHT 12
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
 */
SKIPLIST_DECL(
  slex, api_, entries,
  /* free node */ { (void)node; },
  /* update node */ { node->value = new->value; })

/*
 * Getter
 * It can be useful to have simple get/put-style API, but to
 * do that you'll have to supply some blocks of code used to
 * extract data from within your nodes.
 */
SKIPLIST_GETTERS(
  slex, api_, int, int, { query.key = key; }, { return node->value; })

/*
 * Now we need a way to compare the nodes you defined above.
 * Let's create a function with four arguments:
 *   - a reference to the Skiplist, `slist`
 *   - the two nodes to compare, `a` and `b`
 *   - and `aux`, which you can use to pass into this function
 *     any additional information required to compare objects.
 *     `aux` is passed from the value in the Skiplist, you can
 *     modify that value at any time to suit your needs.
 *
 * Your function should result in a return statement:
 *   a  < b : return -1
 *   a == b : return 0
 *   a  > b : return 1
 *
 * This result provides the ordering within the Skiplist.  Sometimes
 * your function will not be used when comparing nodes.  This will
 * happen when `a` or `b` are references to the head or tail of the
 * list or when `a == b`.  In those cases the comparison function
 * returns before using the code in your block, don't panic. :)
 */
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
  rc = api_skip_init_slex(list, 12, __slm_key_compare);
  if (rc)
    return rc;

  struct slex_node *n;

  /* Insert 7 key/value pairs into the list. */
  for (int i = -200; i <= 200; i++) {
    int v;
    slex_node_t new;
    rc = api_skip_alloc_node_slex(list, &n);
    if (rc)
      return rc;
    n->key = i;
    n->value = i;
    api_skip_insert_slex(list, n);
    v = api_skip_get_slex(list, i);
    ((void)v);
    new.key = n->key;
    new.value = n->value * 10;
    api_skip_update_slex(list, &new);
  }

  slex_node_t q;
  q.key = 0;
  api_skip_remove_slex(list, &q);

//  assert(api_skip_gte_slex(list, -3000000) == -20);
  assert(api_skip_gte_slex(list, -2) == -20);
  assert(api_skip_gte_slex(list, 0) == 10);
//  assert(api_skip_gte_slex(list, 0) == 0);
  assert(api_skip_gte_slex(list, 2) == 20);
  assert(api_skip_gte_slex(list, 30000000) == 0);

  assert(api_skip_lte_slex(list, -3000000) == 0);
  assert(api_skip_lte_slex(list, -2) == -20);
  assert(api_skip_lte_slex(list, 0) == -10);
//  assert(api_skip_lte_slex(list, 0) == 0);
  assert(api_skip_lte_slex(list, 2) == 20);
//  assert(api_skip_lte_slex(list, 30000000) == 20);

  FILE *of = fopen("/tmp/slm.dot", "w");
  if (!of) {
    perror("Failed to open file /tmp/slm.dot");
    return EXIT_FAILURE;
  }
  api_skip_dot_slex(of, list, 0, sprintf_slex_node);
  fclose(of);

  api_skip_destroy_slex(list);

fail:;
  return rc;
}
