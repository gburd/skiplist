# Skiplist

A header-only C library implementing a splay-list -- a skiplist with adaptive
rebalancing.  The entire implementation lives in preprocessor macros
(`include/sl.h`) that generate type-specific code at compile time, similar to
C++ templates.

Lock-free concurrent operations are supported by default using C11 atomics.
Define `SKIPLIST_SINGLE_THREADED` to strip all atomic overhead for
single-threaded use.

## Features

- **Lock-free concurrent operations** -- default mode using C11 atomics and
  marked pointers for logical deletion
- **Single-threaded mode** -- define `SKIPLIST_SINGLE_THREADED` to eliminate
  all atomic overhead
- **Splay rebalancing** -- node heights adapt based on access frequency;
  popular elements are promoted, rarely-accessed elements are demoted
  (opt-in via `-DSKIPLIST_SPLAY_REBALANCE`)
- **MVCC point-in-time snapshots** with restore (`SKIPLIST_DECL_SNAPSHOTS`)
- **Epoch-based reclamation (EBR)** for safe concurrent memory reclamation
  (`SKIPLIST_DECL_EBR`)
- **Fixed-capacity pool allocator** with cache-line-aligned slots
  (`SKIPLIST_DECL_POOL`)
- **Binary serialization/deserialization** (`SKIPLIST_DECL_ARCHIVE`)
- **Key/value access API** -- `get`, `put`, `del`, `contains`
  (`SKIPLIST_DECL_ACCESS`)
- **Duplicate key support** -- `insert_dup` and `dup` operations
- **Runtime integrity validation** (`SKIPLIST_DECL_VALIDATE`)
- **GraphViz DOT visualization** (`SKIPLIST_DECL_DOT`)
- **Doubly-linked bottom layer** with head/tail guard nodes for efficient
  bidirectional iteration
- **Portable** -- compiles with GCC, Clang, and MSVC; supports C11 (C99 in
  single-threaded mode)

## Quick Start

```c
#include "sl.h"

/* 1. Define your node type */
struct ex_node {
    int key;
    char *value;
    SKIPLIST_ENTRY(ex) entries;
};

/* 2. Generate the skiplist implementation */
SKIPLIST_DECL(ex, api_, entries,
    /* compare */  { (void)list; (void)aux; return (a->key > b->key) - (a->key < b->key); },
    /* free */     { free(node->value); },
    /* update */   { (void)value; rc = 0; },
    /* archive */  { (void)node; (void)buf; *bytes = 0; },
    /* sizeof */   { (void)node; *s = sizeof(struct ex_node); }
);

int main(void)
{
    ex_t list;
    api_skip_init_ex(&list);

    /* Allocate and insert a node */
    struct ex_node *n = api_skip_alloc_node_ex(&list);
    n->key = 42;
    n->value = strdup("forty-two");
    api_skip_insert_ex(&list, n);

    /* Search */
    struct ex_node query = { .key = 42 };
    struct ex_node *found = api_skip_contains_ex(&list, &query);

    /* Cleanup */
    api_skip_destroy_ex(&list);
    return 0;
}
```

## Installation

This is a header-only library.  Copy `include/sl.h` into your project, or
install system-wide:

```bash
make install              # installs to /usr/local/include by default
make install PREFIX=/opt  # custom prefix
```

A `pkg-config` file (`skiplist.pc`) is generated and installed alongside the
header.

To uninstall:

```bash
make uninstall
```

## Examples

| Example | Description | Key macros |
|---------|-------------|------------|
| `ex01` | Minimal skiplist | `SKIPLIST_ENTRY`, `SKIPLIST_DECL` |
| `ex02` | Key/value API | `SKIPLIST_DECL_ACCESS` |
| `ex03` | Custom struct types | Custom comparison |
| `ex04` | Iteration and positioning | `SKIPLIST_FOREACH_H2T`, `position_*` |
| `ex05` | Duplicate keys | `insert_dup`, `dup` |
| `ex06` | Snapshots | `SKIPLIST_DECL_SNAPSHOTS` |
| `ex07` | Pool allocator | `SKIPLIST_DECL_POOL` |
| `ex08` | Serialization | `SKIPLIST_DECL_ARCHIVE` |
| `ex09` | Validation and DOT output | `SKIPLIST_DECL_VALIDATE`, `SKIPLIST_DECL_DOT` |
| `ex10` | Concurrent with EBR | `SKIPLIST_DECL_EBR` |

Build and run all examples:

```bash
make examples
./examples/ex01
```

## Build

```bash
make test              # Build and run the test suite
make test_concurrent   # Build and run concurrency tests
make test_tsan         # Build and run ThreadSanitizer tests
make test_all          # Run all test suites
make examples          # Build all examples (ex01 through ex10)
make bench             # Build and run benchmarks
make coverage          # Build with gcov and report coverage
make format            # Run clang-format on all source files
make clean             # Remove build artifacts
```

Running examples with leak detection:

```bash
env ASAN_OPTIONS=detect_leaks=1 ./examples/ex01
```

Generating a DOT visualization:

```bash
dot -Tpdf /tmp/ex09.dot -o /tmp/ex09.pdf
```

## Single-Threaded Mode

By default the library uses C11 `<stdatomic.h>` for lock-free concurrent
access.  If your application is single-threaded, define
`SKIPLIST_SINGLE_THREADED` before including `sl.h` to replace all atomic
operations with plain loads and stores:

```c
#define SKIPLIST_SINGLE_THREADED
#include "sl.h"
```

This eliminates the dependency on `<stdatomic.h>` and `<stdalign.h>`, reduces
code size, and improves performance when concurrency is not needed.

Note: `SKIPLIST_DECL_EBR` is not available in single-threaded mode and will
produce a compile-time error if both are used together.

## Compile-Time Flags

| Flag | Default | Description |
|------|---------|-------------|
| `SKIPLIST_SINGLE_THREADED` | *(not defined)* | Replace C11 atomics with plain operations |
| `SKIPLIST_SPLAY_REBALANCE` | *(not defined)* | Enable adaptive splay rebalancing |
| `SKIPLIST_MAX_HEIGHT` | `64` | Maximum skiplist height (must be <= 64) |
| `SKIPLIST_SPLAY_INTERVAL` | `64` | Number of accesses between rebalance passes |
| `SKIPLIST_EBR_MAX_THREADS` | `128` | Maximum threads that can register with EBR |
| `SKIPLIST_DIAGNOSTIC` | *(not defined)* | Enable diagnostic/validation macros |
| `DEBUG` | *(not defined)* | Enable debug output |

## API Overview

The library is organized as a set of macro layers.  Each macro generates a
family of functions named `prefix_skip_<operation>_decl()`.

| Macro | Purpose |
|-------|---------|
| `SKIPLIST_ENTRY(decl)` | Embed in your node struct to add skiplist metadata |
| `SKIPLIST_DECL(decl, prefix, field, cmp, free, update, archive, sizeof)` | Core skiplist: init, destroy, insert, remove, search, traversal |
| `SKIPLIST_DECL_ACCESS(decl, prefix, key, ktype, value, vtype, qblk, rblk)` | High-level key/value API: `get`, `put`, `del`, `contains` |
| `SKIPLIST_DECL_SNAPSHOTS(decl, prefix, field)` | MVCC point-in-time snapshots with restore |
| `SKIPLIST_DECL_EBR(decl, prefix)` | Epoch-based reclamation for concurrent memory safety |
| `SKIPLIST_DECL_POOL(decl, prefix, field, capacity)` | Fixed-capacity pool allocator |
| `SKIPLIST_DECL_ARCHIVE(decl, prefix, field, write_blk, read_blk)` | Binary serialization/deserialization |
| `SKIPLIST_DECL_VALIDATE(decl, prefix, field)` | Runtime integrity checking |
| `SKIPLIST_DECL_DOT(decl, prefix, field)` | GraphViz DOT visualization output |

Iteration macros:

| Macro | Description |
|-------|-------------|
| `SKIPLIST_FOREACH_H2T(decl, prefix, field, list, elm, iter)` | Iterate head to tail |
| `SKIPLIST_FOREACH_T2H(decl, prefix, field, list, elm, iter)` | Iterate tail to head |

## License

SPDX-License-Identifier: `ISC OR MIT`

Copyright (c) 2024 Gregory Burd <greg@burd.me>.  All rights reserved.

Dual-licensed under the ISC License and the MIT License.  See the header of
`include/sl.h` for the full license text.

## Safety

The ordered skiplist relies on a well-behaved comparison function.
Specifically, given some ordering function `f(a, b)`, it must satisfy the
following properties:

1. **Well-defined**: `f(a, b)` should always return the same value.
2. **Antisymmetric**: `f(a, b) == Greater` if and only if `f(b, a) == Less`,
   and `f(a, b) == Equal == f(b, a)`.
3. **Transitive**: If `f(a, b) == Greater` and `f(b, c) == Greater` then
   `f(a, c) == Greater`.

Failure to satisfy these properties can result in unexpected behavior at best,
and at worst will cause a segfault, null dereference, or other undefined
behavior.

## References

Sources of information most helpful for this implementation include, but are
not limited to:

- **Skip lists: a probabilistic alternative to balanced trees**
  William Pugh, 1990.
  *Communications of the ACM*, 33(6), 668--676.
  [DOI](https://doi.org/10.1145/78973.78977) |
  [PDF](https://www.cl.cam.ac.uk/teaching/2005/Algorithms/skiplists.pdf)

- **Tutorial: The Ubiquitous Skiplist, its Variants, and Applications in Modern Big Data Systems**
  Venkata Sai Pavan Kumar Vadrevu, Lu Xing, Walid G. Aref, 2023.
  *arXiv:2304.09983*.
  [PDF](https://arxiv.org/pdf/2304.09983.pdf)

- **The Splay-List: A Distribution-Adaptive Concurrent Skip-List**
  Vitaly Aksenov, Dan Alistarh, Alexandra Drozdova, Amirkeivan Mohtashami, 2020.
  *arXiv:2008.01009*.
  [PDF](https://arxiv.org/pdf/2008.01009.pdf)

- **JellyFish: A Fast Skip List with MVCC**
  Jeseong Yeon, Leeju Kim, Youil Han, Hyeon Gyu Lee, Eunji Lee, Bryan Suk Joon Kim, 2020.
  *Proceedings of the 21st International Middleware Conference*.

## Acknowledgments

Portions of this code are derived from other copyrighted works:

- *MIT License*
  - [greensky00/skiplist](https://github.com/greensky00/skiplist) -- Jung-Sang Ahn
  - [paulross/skiplist](https://github.com/paulross/skiplist) -- Paul Ross
  - [JP-Ellis/rust-skiplist](https://github.com/JP-Ellis/rust-skiplist) -- Joshua Ellis
- *Public Domain*
  - [zhpengg/skiplist](https://gist.github.com/zhpengg/2873424) -- Zhipeng Li
