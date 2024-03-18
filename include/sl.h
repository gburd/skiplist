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
 *  - queue.h
 *  - khash.h
 *  - async_nif.h
 */

#ifndef	_SYS_SKIPLIST_H_
#define	_SYS_SKIPLIST_H_

/*
 * This file defines a skiplist data structure with a similar API to those
 * datastructures defined in FreeBSD's <sys/queue.h> header.
 *
 * In 1990 William Pugh published:
 *  - Skiplists: a probabilistic alternative to balanced trees.
 *    https://www.cl.cam.ac.uk/teaching/2005/Algorithms/skiplists.pdf
 * A Skiplist is an ordered data structure providing expected O(Log(n)) lookup,
 * insertion, and deletion complexity.
 *
 *
 * For details on the use of these macros, see the queue(3) manual page.
 *
 *
 *			SKIP		SPLAYED_SKIP
 * _HEAD		+		+
 * _HEAD_INITIALIZER	+		+
 * _ENTRY		+		+
 * _INIT		+		+
 * _DISPOSE		+		+
 * _EMPTY		+		+
 * _FIRST		+		+
 * _NEXT		+		+
 * _PREV		+		+
 * _LAST		-		+
 * _FOREACH		+		+
 * _FOREACH_REVERSE	+		+
 * _INSERT		+		+
 * _UPDATE		+		+
 * _BULK_INSERT		+		+
 * _REMOVE		+		+
 * _LOCATE		+		+
 *
 */

#define	SKIPLIST_MACRO_DEBUG 0
#if SKIPLIST_MACRO_DEBUG
/* Store the last 2 places the element or head was altered */
struct sl_trace {
	char * lastfile;
	int lastline;
	char * prevfile;
	int prevline;
};

#define	TRACEBUF	struct sl_trace trace;
#define	TRASHIT(x)	do {(x) = (void *)-1;} while (0)

#define	SLD_TRACE_HEAD(head) do {					\
	(head)->trace.prevline = (head)->trace.lastline;		\
	(head)->trace.prevfile = (head)->trace.lastfile;		\
	(head)->trace.lastline = __LINE__;				\
	(head)->trace.lastfile = __FILE__;				\
} while (0)

#define	SLD_TRACE_ELEM(elem) do {					\
	(elem)->trace.prevline = (elem)->trace.lastline;		\
	(elem)->trace.prevfile = (elem)->trace.lastfile;		\
	(elem)->trace.lastline = __LINE__;				\
	(elem)->trace.lastfile = __FILE__;				\
} while (0)

#else
#define	SLD_TRACE_ELEM(elem)
#define	SLD_TRACE_HEAD(head)
#define	TRACEBUF
#define	TRASHIT(x)
#endif	/* QUEUE_MACRO_DEBUG */


/*
 * Private, internal API.
 */

/* NOTE:
 * This array is a bit unusual, while values are 0-based the array is allocated
 * with space for two more pointers which are used to store max size the array
 * can grow to (`array[-2]`) and the length, or number of elements used in the
 * array so far (`array[-1]`).
 */
#define ARRAY_ALLOC(var, type, size) do {		\
    (size) = (size == 0 ? 254 : size);			\
    (var) = (type**)calloc(sizeof(type*), size + 2);	\
    if ((var) != NULL) {				\
      *(var)++ = (type *)size;				\
      *(var)++ = 0;					\
    }							\
  } while(0)
#define ARRAY_FREE(var) free((var)-2)
#define ARRAY_SIZE(list) (list)[-2]
#define ARRAY_SET_SIZE(list, size) (list)[-2] = (void *)(uintptr_t)(size)
#define ARRAY_LENGTH(list) (list)[-1]
#define ARRAY_SET_LENGTH(list, len) (list)[-1] = (void *)(uintptr_t)(len)
#define ARRAY_FOREACH(var, list)					\
  for(unsigned int (var) = 0; (var) < (unsigned int)(uintptr_t)ARRAY_LENGTH(list); (var)++)
#define ARRAY_FORALL(var, list)					\
  for(unsigned int (var) = 0; (var) < (unsigned int)(uintptr_t)ARRAY_SIZE(list); (var)++)


/*
 * Skiplist declarations.
 */
#define	SKIP_HEAD(name, type)						\
  struct name {								\
    size_t level, length, max, fanout;					\
    int (*cmp)(struct name *, struct type *, struct type *, void *);	\
    void *aux;								\
    struct type *slh_head;						\
    struct type *slh_tail;						\
    TRACEBUF								\
  }

#define	SKIP_HEAD_DEFAULT_INITIALIZER(cmp)		\
  { 0, 0, 12, 4, cmp, NULL, NULL, NULL }

#define	SKIP_HEAD_INITIALIZER(cmp, max, fanout)			\
  { 0, 0, max, fanout, cmp, NULL, NULL, NULL }

#define	SKIP_ENTRY(type)			\
  struct {					\
    struct type **sle_next;			\
    struct type *sle_prev;			\
    TRACEBUF					\
  }

/*
 * Skip List access methods.
 */
#define	SKIP_FIRST(head)		((head)->slh_head)
#define	SKIP_LAST(head)			((head)->slh_tail)
#define	SKIP_NEXT(elm, field)		((elm)->field.sle_next[0])
#define	SKIP_PREV(elm, field)		((elm)->field.sle_prev)
#define	SKIP_EMPTY(head)		((head)->length == 0)

#if 0
#define SKIP_FOREACH(var, head, field)					\
	for((var) = SKIP_FIRST(head);					\
	    (var)!= SKIP_END(head);					\
	    (var) = SKIP_NEXT(var, field))

#define	SKIP_FOREACH_SAFE(var, head, field, tvar)			\
  for ((var) = SKIP_FIRST(head);					\
	    (var) && ((tvar) = SKIP_NEXT(var, field), 1);		\
	    (var) = (tvar))

#define TAILQ_FOREACH_REVERSE(var, head, headname, field)		\
	for((var) = TAILQ_LAST(head, headname);				\
	    (var) != TAILQ_END(head);					\
	    (var) = TAILQ_PREV(var, headname, field))

#define	TAILQ_FOREACH_REVERSE_SAFE(var, head, headname, field, tvar)	\
	for ((var) = TAILQ_LAST(head, headname);			\
	    (var) != TAILQ_END(head) &&					\
	    ((tvar) = TAILQ_PREV(var, headname, field), 1);		\
	    (var) = (tvar))
#endif

/*
 * Skip List functions.
 */

#define SKIP_COMPARATOR(list, type, fn)					\
  int __skip_cmp_##type(struct list *head, struct type *a, struct type *b, void *aux) { \
    if (a == b)								\
      return 0;								\
    if (a == (head)->slh_head || b == (head)->slh_tail)			\
      return -1;							\
    if (a == (head)->slh_tail || b == (head)->slh_head)			\
      return 1;								\
    fn }

#define	SKIP_INIT(head, max, fanout, type, field, fn) do {		\
  (head)->level = 0;							\
  (head)->length = 0;							\
  (head)->max = max;							\
  (head)->fanout = fanout;						\
  (head)->cmp = fn;							\
  SKIP_ALLOC_NODE(head, (head)->slh_head, type, field);			\
  SKIP_ALLOC_NODE(head, (head)->slh_tail, type, field);			\
  ARRAY_SET_LENGTH((head)->slh_head->field.sle_next, max);		\
  ARRAY_FORALL(__i, (head)->slh_head->field.sle_next) {			\
    (head)->slh_head->field.sle_next[__i] = (head)->slh_tail;		\
  }									\
  (head)->slh_head->field.sle_prev = NULL;				\
  ARRAY_SET_LENGTH((head)->slh_tail->field.sle_next, max);		\
  ARRAY_FORALL(__i, (head)->slh_tail->field.sle_next) {			\
    (head)->slh_tail->field.sle_next[__i] = NULL;			\
  }									\
  (head)->slh_head->field.sle_prev = (head)->slh_tail;			\
  SLD_TRACE_HEAD(head);							\
  } while (0)

#define	SKIP_DEFAULT_INIT(head, fn, type, field) do {			\
    (head)->level = 0;							\
    (head)->length = 0;							\
    (head)->max = 12;							\
    (head)->fanout = 4;							\
    (head)->cmp = fn;							\
    SKIP_ALLOC_NODE(head, (head)->slh_head, type, field);		\
    SKIP_ALLOC_NODE(head, (head)->slh_tail, type, field);		\
    ARRAY_SET_LENGTH((head)->slh_head->field.sle_next, (head)->max);	\
    ARRAY_FORALL(__i, (head)->slh_head->field.sle_next) {		\
      (head)->slh_head->field.sle_next[__i] = (head)->slh_tail;		\
    }									\
    (head)->slh_head->field.sle_prev = NULL;				\
    ARRAY_SET_LENGTH((head)->slh_tail->field.sle_next,  (head)->max);	\
    ARRAY_FORALL(__i, (head)->slh_tail->field.sle_next) {		\
      (head)->slh_tail->field.sle_next[__i] = NULL;			\
    }									\
    (head)->slh_head->field.sle_prev = (head)->slh_tail;		\
    SLD_TRACE_HEAD(head);						\
  } while (0)


#define SKIP_ALLOC_NODE(head, var, type, field) do {			\
    (var) = (struct type *)calloc(1, sizeof(struct type));		\
    ARRAY_ALLOC((var)->field.sle_next, struct type, (head)->max);	\
    if ((var) && (var)->field.sle_next) {				\
      ARRAY_SET_SIZE((var)->field.sle_next, (head)->max);		\
      ARRAY_SET_LENGTH((var)->field.sle_next, 0);			\
    }									\
  } while (0)

#define SKIP_FREE_NODE(node, field) do {			\
    free((node)->field.sle_next);				\
    free((node));						\
  } while (0)

#define SKIP_DECL(decl, prefix, field, key_cmp_logic)			\
  /* Skiplist type */							\
  typedef struct decl decl##_t;						\
									\
  /* Skiplist node type */						\
  typedef struct decl##_node decl##_node_t;				\
									\
  /* -- __skip_key_compare_decl						\
   *									\
   * This function takes four arguments:				\
   *   - a reference to the Skiplist					\
   *   - the two nodes to compare, `a` and `b`				\
   *   - `aux` an additional auxiliary argument				\
   * and returns:							\
   *   a  < b : return -1						\
   *   a == b : return 0						\
   *   a  > b : return 1						\
   */									\
  static int __skip_key_compare_##decl(decl##_t *slist, decl##_node_t *a, decl##_node_t *b, void *aux) { \
    if (a == b)								\
      return 0;								\
    if (a == slist->slh_head || b == slist->slh_tail)			\
      return -1;							\
    if (a == slist->slh_tail || b == slist->slh_head)			\
      return 1;								\
    do { key_cmp_logic } while(0);					\
  }									\
									\
  /* -- __skip_toss_decl */						\
  static int __skip_toss_##decl(size_t max, size_t fanout) {		\
    size_t level = 0;							\
    while (level + 1 < max) {						\
      if (rand() % fanout == 0) /* NOLINT(*-msc50-cpp) */		\
	level++;							\
      else								\
	break;								\
    }									\
    return level;							\
  }									\
									\
  /* -- prefixskip_insert_decl */					\
  int prefix##skip_insert_##decl(decl##_t *slist, decl##_node_t *n) {	\
    if (n == NULL)							\
      return ENOMEM;							\
    decl##_node_t *prev, *elm = slist->slh_head;			\
    unsigned int i;							\
    decl##_node_t **path;						\
    i = slist->level;							\
    ARRAY_ALLOC(path, decl##_node_t, slist->max);			\
    if (path == NULL)							\
      return ENOMEM;							\
    /* Find the position in the list where this element belongs. */	\
    do {								\
      while(elm && slist->cmp(slist, elm->field.sle_next[i], n, slist->aux) < 0) \
	elm = elm->field.sle_next[i];					\
      path[i] = elm;							\
      ARRAY_SET_LENGTH(path, ARRAY_LENGTH(path)+1);			\
    } while(i--);							\
    i = 0;								\
    prev = elm;								\
    elm = elm->field.sle_next[0];					\
    if (slist->cmp(slist, elm, n, slist->aux) == 0) {			\
      ARRAY_FREE(path);							\
      /* Don't overwrite, to do that use _REPLACE not _INSERT */	\
      return -1;							\
    }									\
    size_t level = __skip_toss_##decl(slist->max, slist->fanout); \
    ARRAY_SET_LENGTH(n->field.sle_next, level);				\
    if (level > slist->level) {						\
      for (i = slist->level + 1; i <= level; i++) {			\
	path[i] = slist->slh_tail;					\
      }									\
      slist->level = level;						\
    }									\
    for (i = 0; i <= level; i++) {					\
      n->field.sle_next[i] = path[i]->field.sle_next[i];		\
      path[i]->field.sle_next[i] = n;					\
    }									\
    n->field.sle_prev = prev;						\
    if (n->field.sle_next[0] == slist->slh_tail) {			\
      slist->slh_tail->field.sle_prev = n;				\
    }									\
    slist->length++;							\
    ARRAY_FREE(path);							\
    return 0;								\
  }									\
  /* END */

#if 0
#define SKIP_REMOVE(head, elm, field) do {				\
	if ((elm)->field.le_next != NULL)				\
		(elm)->field.le_next->field.le_prev =			\
		    (elm)->field.le_prev;				\
	*(elm)->field.le_prev = (elm)->field.le_next;			\
	_Q_INVALIDATE((elm)->field.le_prev);				\
	_Q_INVALIDATE((elm)->field.le_next);				\
} while (0)

#define SKIP_REPLACE(elm, elm2, field) do {				\
	if (((elm2)->field.le_next = (elm)->field.le_next) != NULL)	\
		(elm2)->field.le_next->field.le_prev =			\
		    &(elm2)->field.le_next;				\
	(elm2)->field.le_prev = (elm)->field.le_prev;			\
	*(elm2)->field.le_prev = (elm2);				\
	_Q_INVALIDATE((elm)->field.le_prev);				\
	_Q_INVALIDATE((elm)->field.le_next);				\
} while (0)
#endif

#endif /* _SYS_SKIPLIST_H_ */
