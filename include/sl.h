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
#define ARRAY_ALLOC(var, type, size) do {				\
    (size) = (size == 0 ? 254 : size);					\
    (var) = (struct type**)calloc(sizeof(struct type*), size + 2);	\
    if ((var) != NULL) {						\
      *(var)++ = (struct type *)size;					\
      *(var)++ = 0;							\
    }									\
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
    struct type **slh_head;						\
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
#define	SKIP_FIRST(head)		((head)->slh_head ? (head)->slh_head[0] : NULL)
#define	SKIP_LAST(head)			((head)->slh_tail == (head)->slh_head ? NULL : (head)->slh_tail)
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
    if (a == (struct type *)(head)->slh_head || b == (head)->slh_tail)	\
      return -1;							\
    if (a == (head)->slh_tail || b == (struct type *)(head)->slh_head)	\
      return 1;								\
    fn }

#define	SKIP_INIT(head, max, fanout, fn) do {	\
    (head)->level = 0;				\
    (head)->length = 0;				\
    (head)->max = max;				\
    (head)->fanout = fanout;			\
    (head)->cmp = fn;				\
    (head)->slh_head = NULL;			\
    (head)->slh_tail = (head)->slh_head;	\
    SLD_TRACE_HEAD(head);			\
  } while (0)

#define	SKIP_DEFAULT_INIT(head, fn) do {	\
    (head)->level = 0;				\
    (head)->length = 0;				\
    (head)->max = 12;				\
    (head)->fanout = 4;				\
    (head)->cmp = fn;				\
    (head)->slh_head = NULL;			\
    (head)->slh_tail = (head)->slh_head;	\
    SLD_TRACE_HEAD(head);			\
  } while (0)


#define SKIP_ALLOC_NODE(head, var, type, field) do {			\
    size_t amt = sizeof(struct type *) * (head)->max;			\
    (var) = (struct type *)calloc(1, sizeof(struct type) + amt + 3);	\
    if ((var) != NULL) {						\
      (var)->field.sle_prev = (struct type *)(var) + sizeof(struct type); \
      (var)->field.sle_next = (struct type **)((var)->field.sle_prev + (sizeof(void *) * 3)); \
      ARRAY_SET_SIZE((var)->field.sle_next, (head)->max);		\
      ARRAY_SET_LENGTH((var)->field.sle_next, 0);			\
    }									\
  } while (0)

#define __SKIP_TOSS(var, max, fanout) do {			\
    (var) = 0;							\
    while ((var) + 1 < (max)) {					\
      if (rand() % (fanout) == 0) /* NOLINT(*-msc50-cpp) */	\
	(var)++;						\
      else							\
	break;							\
    }								\
  } while(0)

#define SKIP_INSERT(head, type, listelm, field) do {			\
    struct type *__elm = SKIP_FIRST(head);				\
    unsigned int __i;							\
    struct type **__path;						\
    if ((listelm) == NULL) break;					\
    if (__elm == NULL) { \
      /* Empty list, setup header and add first element. */		\
      ARRAY_ALLOC((head)->slh_head, type, (head)->max);			\
      ARRAY_FORALL(__i, (head)->slh_head) {				\
	(head)->slh_head[__i] = (head)->slh_tail;			\
      }									\
      (head)->slh_head[0] = (listelm);					\
      (head)->slh_tail = (listelm);					\
      (listelm)->field.sle_next[0] = (head)->slh_tail;			\
      (listelm)->field.sle_prev = (struct type *)(head)->slh_head;	\
      (head)->length = 1;						\
      break;								\
    }									\
    __i = (head)->level;						\
    ARRAY_ALLOC(__path, type, (head)->max);				\
    if (__path == NULL) break; /* ENOMEM */				\
    /* Find the position in the list where this element belongs. */	\
    do {								\
      while(__elm && (head)->cmp((head), __elm->field.sle_next[__i], (listelm), (head)->aux) < 0) \
	__elm = __elm->field.sle_next[__i];				\
      __path[__i] = __elm;						\
    } while(__i--);							\
    __i = 0;								\
    __elm = __elm->field.sle_next[0];					\
    if ((head)->cmp((head), __elm, listelm, (head)->aux) == 0) {	\
      ARRAY_FREE(__path);						\
      break; /* Don't overwrite, to do that use _REPLACE not _INSERT */	\
    }									\
    unsigned int __level;						\
    __SKIP_TOSS(__level, (head)->max, (head)->fanout);			\
    ARRAY_SET_LENGTH((listelm)->field.sle_next, __level);		\
    if (__level > (head)->level) {					\
      for (__i = (head)->level; __i <= __level; __i++) {		\
	__path[__i] = (list)->slh_tail;					\
      }									\
      (head)->level = __level;						\
    }									\
    for (__i = 0; __i <= __level; __i++) {				\
      (listelm)->field.sle_next[__i] = __path[__i]->field.sle_next[__i];\
      __path[__i]->field.sle_next[__i] = (listelm);			\
    }									\
    (listelm)->field.sle_prev = __elm;					\
    __elm->field.sle_next[0]->field.sle_prev = (listelm);		\
    (head)->length++;							\
    ARRAY_FREE(__path);							\
  } while (0)

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
