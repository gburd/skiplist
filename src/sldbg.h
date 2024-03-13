#if __SL_DEBUG > 0
#include <sys/types.h>

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#endif
#if __SL_DEBUG >= 1
#define __SLD_ASSERT(cond) assert(cond)
#define __SLD_(b) b
#elif __SL_DEBUG >= 2
#define __SLD_P(args...) printf(args)
#elif __SL_DEBUG >= 3
typedef struct dbg_node {
	sl_node snode;
	int value;
} dbg_node_t;

inline void
__sld_rt_ins(int error_code, sl_node *node, int top_layer, int cur_layer)
{
	dbg_node_t *ddd = sl_get_entry(node, dbg_node_t, snode);
	printf("[INS] retry (code %d) "
	       "%p (top %d, cur %d) %d\n",
	    error_code, node, top_layer, cur_layer, ddd->value);
}

inline void
__sld_nc_ins(sl_node *node, sl_node *next_node, int top_layer, int cur_layer)
{
	dbg_node_t *ddd = sl_get_entry(node, dbg_node_t, snode);
	dbg_node_t *ddd_next = sl_get_entry(next_node, dbg_node_t, snode);

	printf("[INS] next node changed, "
	       "%p %p (top %d, cur %d) %d %d\n",
	    node, next_node, top_layer, cur_layer, ddd->value, ddd_next->value);
}

inline void
__sld_rt_rmv(int error_code, sl_node *node, int top_layer, int cur_layer)
{
	dbg_node_t *ddd = sl_get_entry(node, dbg_node_t, snode);
	printf("[RMV] retry (code %d) "
	       "%p (top %d, cur %d) %d\n",
	    error_code, node, top_layer, cur_layer, ddd->value);
}

inline void
__sld_nc_rmv(sl_node *node, sl_node *next_node, int top_layer, int cur_layer)
{
	dbg_node_t *ddd = sl_get_entry(node, dbg_node_t, snode);
	dbg_node_t *ddd_next = sl_get_entry(next_node, dbg_node_t, snode);

	printf("[RMV] next node changed, "
	       "%p %p (top %d, cur %d) %d %d\n",
	    node, next_node, top_layer, cur_layer, ddd->value, ddd_next->value);
}

inline void
__sld_bm(sl_node *node)
{
	dbg_node_t *ddd = sl_get_entry(node, dbg_node_t, snode);
	printf("[RMV] node is being modified %d\n", ddd->value);
}

#define __SLD_RT_INS(e, n, t, c) __sld_rt_ins(e, n, t, c)
#define __SLD_NC_INS(n, nn, t, c) __sld_nc_ins(n, nn, t, c)
#define __SLD_RT_RMV(e, n, t, c) __sld_rt_rmv(e, n, t, c)
#define __SLD_NC_RMV(n, nn, t, c) __sld_nc_rmv(n, nn, t, c)
#define __SLD_BM(n) __sld_bm(n)
#else
#define __SLD_RT_INS(e, n, t, c) \
	do {                     \
	} while (0)
#define __SLD_NC_INS(n, nn, t, c) \
	do {                      \
	} while (0)
#define __SLD_RT_RMV(e, n, t, c) \
	do {                     \
	} while (0)
#define __SLD_NC_RMV(n, nn, t, c) \
	do {                      \
	} while (0)
#define __SLD_BM(n) \
	do {        \
	} while (0)
#define __SLD_ASSERT(cond) \
	do {               \
	} while (0)
#define __SLD_P(args...) \
	do {             \
	} while (0)
#define __SLD_(b) \
	do {      \
	} while (0)
#endif
