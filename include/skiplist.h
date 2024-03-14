/**
 * Copyright 2024-present Gregory Burd <greg@burd.me> All rights reserved.
 *
 * Portions of this code are derived from work copyrighted by Jung-Sang Ahn
 * 2017-2024 Jung-Sang Ahn <jungsang.ahn@gmail.com> and made available under
 * the MIT License. (see: https://github.com/greensky00 Skiplist version:
 * 0.2.9)
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

#ifndef SKIPLIST_H__
#define SKIPLIST_H__ (1)

#include <stddef.h>
#include <stdint.h>

#define SKIPLIST_MAX_LAYER (64)

#define _STL_ATOMIC (1)
#ifdef __APPLE__
#define _STL_ATOMIC (1)
#endif
typedef struct _sl_node *atm_node_ptr;
typedef uint8_t atm_bool;
typedef uint8_t atm_uint8_t;
typedef uint16_t atm_uint16_t;
typedef uint32_t atm_uint32_t;

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _sl_node {
    atm_node_ptr *next;
    atm_bool is_fully_linked;
    atm_bool being_modified;
    atm_bool removed;
    uint8_t top_layer; /* 0: bottom */
    atm_uint16_t ref_count;
    atm_uint32_t accessing_next;
} sl_node;

/*
 * *a  < *b : return neg
 * *a == *b : return 0
 * *a  > *b : return pos
 */
typedef int sl_cmp_t(sl_node *a, sl_node *b, void *aux);

typedef struct {
    size_t fanout;
    size_t maxLayer;
    void *aux;
} sl_raw_config;

typedef struct {
    sl_node head;
    sl_node tail;
    sl_cmp_t *cmp_func;
    void *aux;
    atm_uint32_t num_entries;
    atm_uint32_t *layer_entries;
    atm_uint8_t top_layer;
    uint8_t fanout;
    uint8_t max_layer;
} sl_raw;

#ifndef sl_get_entry
#define sl_get_entry(ELEM, STRUCT, MEMBER) \
    ((STRUCT *)((uint8_t *)(ELEM)-offsetof(STRUCT, MEMBER)))
#endif

void sl_init(sl_raw *slist, sl_cmp_t *cmp_func);
void sl_free(sl_raw *slist);

void sl_init_node(sl_node *node);
void sl_free_node(sl_node *node);

size_t sl_get_size(sl_raw *slist);

sl_raw_config sl_get_default_config();
sl_raw_config sl_get_config(sl_raw *slist);

void sl_set_config(sl_raw *slist, sl_raw_config config);

int sl_insert(sl_raw *slist, sl_node *node);
int sl_insert_nodup(sl_raw *slist, sl_node *node);

sl_node *sl_find(sl_raw *slist, sl_node *query);
sl_node *sl_find_smaller_or_equal(sl_raw *slist, sl_node *query);
sl_node *sl_find_greater_or_equal(sl_raw *slist, sl_node *query);

int sl_erase_node_passive(sl_raw *slist, sl_node *node);
int sl_erase_node(sl_raw *slist, sl_node *node);
int sl_erase(sl_raw *slist, sl_node *query);

int sl_is_valid_node(sl_node *node);
int sl_is_safe_to_free(sl_node *node);
void sl_wait_for_free(sl_node *node);

void sl_grab_node(sl_node *node);
void sl_release_node(sl_node *node);

sl_node *sl_next(sl_raw *slist, sl_node *node);
sl_node *sl_prev(sl_raw *slist, sl_node *node);
sl_node *sl_begin(sl_raw *slist);
sl_node *sl_end(sl_raw *slist);

#ifdef __cplusplus
}
#endif

#endif /* SKIPLIST_H__ */
