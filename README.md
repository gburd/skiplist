# skiplist

[![CI](https://codeberg.org/gregburd/skiplist/actions/workflows/ci.yml/badge.svg)](https://codeberg.org/gregburd/skiplist/actions)

A header-only C library implementing a concurrent, lock-free skip-list
(specifically, a *splay-list*: a skip-list with optional adaptive
rebalancing).  The entire implementation lives in preprocessor macros
in `include/sl.h` that generate type-specific code at compile time,
similar to C++ templates.

The lock-free path uses the Fraser/Harris algorithm with marked
pointers for logical deletion and epoch-based reclamation (EBR) for
safe concurrent memory reclamation.  A single-threaded mode is also
provided for applications that do not need concurrency, eliminating
all dependencies on `<stdatomic.h>`.

- One file: `include/sl.h`
- C11 (or C99 in single-threaded mode)
- No runtime dependencies beyond `libc`, `libm`, and (for concurrent
  mode) `pthreads`
- Dual-licensed: ISC OR MIT


## Table of Contents

- [Status](#status)
- [Features](#features)
- [Quick Start](#quick-start)
- [Build, Test, Install](#build-test-install)
  - [Plain Make](#plain-make)
  - [Autoconf and Make](#autoconf-and-make)
  - [Meson and Ninja](#meson-and-ninja)
  - [Nix Flake](#nix-flake)
- [Compile-Time Flags](#compile-time-flags)
- [API Overview](#api-overview)
  - [Macro Layers](#macro-layers)
  - [Generated Types and Functions](#generated-types-and-functions)
  - [Core Operations](#core-operations)
  - [Iteration](#iteration)
  - [Key/Value Access API](#keyvalue-access-api)
  - [Pool Allocator](#pool-allocator)
  - [Snapshots (MVCC)](#snapshots-mvcc)
  - [Epoch-Based Reclamation (EBR)](#epoch-based-reclamation-ebr)
  - [Archive (Serialization)](#archive-serialization)
  - [Validation and Visualization](#validation-and-visualization)
- [Concurrency Model](#concurrency-model)
  - [Lock-Free Algorithm](#lock-free-algorithm)
  - [Memory Reclamation](#memory-reclamation)
  - [Single-Threaded Mode](#single-threaded-mode)
- [Splay Rebalancing](#splay-rebalancing)
- [Examples Walkthrough](#examples-walkthrough)
- [Benchmarks](#benchmarks)
- [Performance Notes](#performance-notes)
- [Portability](#portability)
- [Contract for Comparison Functions](#contract-for-comparison-functions)
- [Testing and CI](#testing-and-ci)
- [Project Layout](#project-layout)
- [License](#license)
- [References](#references)
- [Acknowledgments](#acknowledgments)


## Status

Production-ready.  The current `main` branch is the canonical version.
The full test matrix passes locally and in CI:

| Suite                | Tests | Status |
|----------------------|-------|--------|
| Unit                 | 28    | pass   |
| Concurrent           | 6     | pass   |
| TSAN                 | 6     | pass   |
| ASan + LSan + UBSan  | 28    | pass   |
| Valgrind             | 28    | pass   |

Verified on Linux x86_64 with gcc 13/15 and clang 18/21.  The
implementation is C11 with no Linux-specific syscalls; macOS, the BSDs,
and Windows (MSVC) are supported by the portability shims in `sl.h`.

Historical exploration is preserved as git tags under `archive/*`:
`archive/lock-free-first-attempt`, `archive/splay-list-original`,
`archive/splay-list-wip1`, `archive/splay-list-wip2`,
`archive/snapshot-restore`.


## Features

- **Lock-free concurrent operations** by default (Fraser/Harris with
  marked pointers).
- **Single-threaded mode** strips all atomic operations: define
  `SKIPLIST_SINGLE_THREADED` before including the header.
- **Adaptive splay rebalancing** (opt-in via `SKIPLIST_SPLAY_REBALANCE`):
  node heights track access frequency.  Hot keys rise toward the head;
  cold keys sink.  Trades a small amount of work per access for a
  near-optimal structure under skewed workloads.
- **MVCC snapshots** with point-in-time restore (`SKIPLIST_DECL_SNAPSHOTS`).
- **Epoch-based reclamation** (`SKIPLIST_DECL_EBR`) for safe memory
  reclamation under concurrent access without per-node reference counts.
- **Cache-line-aligned pool allocator** (`SKIPLIST_DECL_POOL`) for
  workloads where `malloc/free` overhead dominates.
- **Binary archive** (`SKIPLIST_DECL_ARCHIVE`): user-defined
  serialize/deserialize on a per-node basis.
- **High-level Key/Value API** (`SKIPLIST_DECL_ACCESS`): `get`, `put`,
  `del`, `contains`, `set`, `dup`, `pos`, `pos_lt`, `pos_gt`, etc.
- **Duplicate keys** supported (`insert_dup`, `dup`).
- **Doubly-linked level-zero list** with head and tail sentinels for
  efficient bidirectional iteration.
- **Runtime integrity validation** (`SKIPLIST_DECL_VALIDATE`).
- **GraphViz DOT visualization** (`SKIPLIST_DECL_DOT`) for debugging.


## Quick Start

```c
#include "sl.h"

/* 1. Embed SKIPLIST_ENTRY in your node struct. */
struct ex_node {
    int   key;
    char *value;
    SKIPLIST_ENTRY(ex) entries;
};

/* 2. Generate the skiplist implementation. */
SKIPLIST_DECL(
    ex,            /* type name */
    api_,          /* function prefix */
    entries,       /* SKIPLIST_ENTRY field name */
    /* compare */  { (void)list; (void)aux;
                     return (a->key > b->key) - (a->key < b->key); },
    /* free */     { free(node->value); },
    /* update */   { (void)value; rc = 0; },
    /* archive */  { (void)dest; (void)src; },
    /* sizeof */   { (void)node; bytes = sizeof(struct ex_node); }
);

int main(void)
{
    ex_t list;
    api_skip_init_ex(&list);

    struct ex_node *n = api_skip_alloc_node_ex(&list);
    n->key = 42;
    n->value = strdup("forty-two");
    api_skip_insert_ex(&list, n);

    struct ex_node query = { .key = 42 };
    struct ex_node *found = api_skip_contains_ex(&list, &query);

    api_skip_destroy_ex(&list);
    return 0;
}
```

Compile:

```sh
cc -std=c11 -I/path/to/skiplist/include -O2 -pthread my.c -lm -o my
```


## Build, Test, Install

The repository ships three equivalent build systems.  Pick whichever
fits your workflow.  All three install the public header, the
pkg-config file, and the man pages (`skiplist(7)` and `sl.h(3)`)
under the configured prefix.

### Plain Make

The simplest path; works out of the box without `./configure`.

```sh
make test              # 28 unit tests under ASan + LSan + UBSan
make test_concurrent   # 6 concurrent tests under ASan + LSan + UBSan
make test_tsan         # 6 concurrent tests under ThreadSanitizer
make test_all          # all of the above
make valgrind          # 28 unit tests under valgrind (no sanitizers)
make examples          # build all 10 examples
make bench             # build and run the benchmark suite
make coverage          # gcov coverage over include/sl.h
make format            # clang-format the tree
make install           # install header + pkg-config to /usr/local
make install PREFIX=/opt/skiplist
make uninstall
make clean
make distclean         # clean + remove configure-generated files
```

After `make install`, the man pages are available via `man`:

```sh
man 7 skiplist         # conceptual overview
man 3 sl.h             # API reference (all macros and generated functions)
```

Override the compiler:

```sh
make CC=clang test
```

### Autoconf and Make

For distribution packaging that expects the `./configure && make
&& make install` sequence.

```sh
./autogen.sh                                  # only needed from a git
                                              # checkout; release tarballs
                                              # ship configure
./configure --prefix=/opt/skiplist
./configure --enable-sanitizers               # ASan + UBSan + LSan
./configure --enable-coverage                 # gcov instrumentation
./configure CC=clang CFLAGS="-O3 -march=native"
make
make test
make install
```

`./configure` validates that the C compiler accepts `-std=c11`,
locates pthread (via `-pthread`, `-lpthread`, or implicit support),
and detects `libm`.  Substituted variables land in `config.mk`,
which the Makefile reads via `-include` if present.

### Meson and Ninja

Recommended when integrating skiplist into a larger meson project.

```sh
meson setup builddir
meson setup builddir -Dbuildtype=release
meson setup builddir -Db_sanitize=address     # ASan
meson setup builddir -Db_sanitize=thread      # TSAN
meson setup builddir -Db_sanitize=undefined   # UBSan
ninja -C builddir
meson test -C builddir --print-errorlogs
meson install -C builddir --destdir=/tmp/sl
```

Downstream meson projects pick up the dependency directly:

```meson
skiplist = dependency('skiplist')
my_exe = executable('my', 'my.c', dependencies : skiplist)
```

### Nix Flake

```sh
nix develop                  # dev shell with gcc, clang, meson, ninja,
                             # autoconf, valgrind, gcovr, clang-tools, ...
nix build                    # produce ./result with the installed header
```


## Compile-Time Flags

| Flag                          | Default        | Purpose                                                                 |
|-------------------------------|----------------|-------------------------------------------------------------------------|
| `SKIPLIST_SINGLE_THREADED`    | not defined    | Replace C11 atomics with plain loads/stores; drop `<stdatomic.h>` dep   |
| `SKIPLIST_SPLAY_REBALANCE`    | not defined    | Enable adaptive splay rebalancing                                       |
| `SKIPLIST_MAX_HEIGHT`         | `64`           | Maximum tower height; must be a positive integer <= 64                  |
| `SKIPLIST_SPLAY_INTERVAL`     | `64`           | Accesses between rebalance passes; must be a power of two               |
| `SKIPLIST_EBR_MAX_THREADS`    | `128`          | Maximum threads that may register with EBR concurrently                 |
| `SKIPLIST_DIAGNOSTIC`         | not defined    | Enable runtime invariant checks inside the implementation               |
| `DEBUG`                       | not defined    | Enable internal `assert()` calls                                        |


## API Overview

### Macro Layers

The library is organized as a stack of independent generators.  You
always call `SKIPLIST_DECL`; the others are opt-in.

| Macro                                                                              | Purpose                                                |
|------------------------------------------------------------------------------------|--------------------------------------------------------|
| `SKIPLIST_ENTRY(decl)`                                                             | Embed in your node struct                              |
| `SKIPLIST_DECL(decl, prefix, field, cmp, free, update, archive, sizeof)`           | Core: init, destroy, insert, remove, search, traverse  |
| `SKIPLIST_DECL_ACCESS(decl, prefix, key, ktype, value, vtype, qblk, rblk)`         | High-level key/value API                               |
| `SKIPLIST_DECL_SNAPSHOTS(decl, prefix, field)`                                     | MVCC point-in-time snapshots                           |
| `SKIPLIST_DECL_EBR(decl, prefix)`                                                  | Epoch-based reclamation                                |
| `SKIPLIST_DECL_POOL(decl, prefix, field, capacity)`                                | Fixed-capacity slab allocator                          |
| `SKIPLIST_DECL_ARCHIVE(decl, prefix, field, write_blk, read_blk)`                  | Binary serialize/deserialize                           |
| `SKIPLIST_DECL_VALIDATE(decl, prefix, field)`                                      | Runtime integrity checks                               |
| `SKIPLIST_DECL_DOT(decl, prefix, field)`                                           | GraphViz DOT output                                    |

The arguments to `SKIPLIST_DECL`:

- `decl` -- name of the generated type (creates `decl_t` for the list
  and `decl_node_t` for nodes).
- `prefix` -- function prefix (creates `prefix_skip_init_decl()`,
  `prefix_skip_insert_decl()`, etc.).
- `field` -- the name of the `SKIPLIST_ENTRY` member inside your node
  struct.
- `cmp` -- a code block returning `<0`, `0`, or `>0` given two nodes
  `a` and `b` plus the list pointer and an `aux` pointer.
- `free` -- a code block that releases user-owned resources in `node`.
  `free(node)` itself is called by the library, do not call it yourself.
- `update` -- a code block that updates an existing node in place,
  setting `rc` to 0 on success.
- `archive` -- a code block that performs a deep copy from `src` to
  `dest`, setting `rc` to 0 on success.
- `sizeof` -- a code block that sets `bytes` to the on-disk size of
  the node for the archive subsystem.

### Generated Types and Functions

Given `SKIPLIST_DECL(my, api_, entry, ...)`, the macro emits:

```c
typedef struct my_node {
    SKIPLIST_ENTRY(my) entry;
    /* your fields */
} my_node_t;

typedef struct my {
    /* sentinels, head/tail pointers, comparison and free fn pointers,
       length counter, xorshift32 PRNG state, snapshot/era state,
       function-pointer dispatch table */
} my_t;

void          api_skip_init_my(my_t *list);
void          api_skip_destroy_my(my_t *list);
size_t        api_skip_length_my(const my_t *list);
my_node_t    *api_skip_alloc_node_my(my_t *list);
void          api_skip_free_node_my(my_t *list, my_node_t *node);
int           api_skip_insert_my(my_t *list, my_node_t *node);
int           api_skip_insert_dup_my(my_t *list, my_node_t *node);
my_node_t    *api_skip_remove_my(my_t *list, my_node_t *query);
my_node_t    *api_skip_position_eq_my(my_t *list, my_node_t *query);
my_node_t    *api_skip_position_lt_my(my_t *list, my_node_t *query);
my_node_t    *api_skip_position_gt_my(my_t *list, my_node_t *query);
my_node_t    *api_skip_first_my(my_t *list);
my_node_t    *api_skip_last_my(my_t *list);
my_node_t    *api_skip_next_my(my_t *list, my_node_t *node);
my_node_t    *api_skip_prev_my(my_t *list, my_node_t *node);
/* ... */
```

### Core Operations

`init` zeroes the list, allocates the head and tail sentinels, and
seeds the random-height PRNG.

`insert` adds a node to the list.  Returns `0` on success, non-zero if
a node with the same key already exists (use `insert_dup` to allow
duplicates).

`remove` searches by key, unlinks the matching node, and returns it to
the caller.  Under concurrent mode the node is not freed immediately;
EBR is responsible for retiring it once all readers have moved on.

`destroy` walks the list and frees every node.  In concurrent mode,
the caller is responsible for ensuring no other thread is operating on
the list.

### Iteration

```c
my_node_t *cur;
size_t i;

SKIPLIST_FOREACH_H2T(my, api_, entry, &list, cur, i) {
    printf("[%zu] key=%d\n", i, cur->key);
}

SKIPLIST_FOREACH_T2H(my, api_, entry, &list, cur, i) {
    printf("[%zu] key=%d\n", i, cur->key);
}
```

`H2T` walks head-to-tail (ascending order), `T2H` walks tail-to-head
(descending order).  Iteration uses level-0 forward pointers (`H2T`)
or `sle_prev` back pointers (`T2H`).

**Do not** insert or remove nodes during iteration.  In concurrent
mode, hold an EBR pin around the loop; the structure may shift under
you between increments, but pinned readers see a consistent set of
nodes.

### Key/Value Access API

`SKIPLIST_DECL_ACCESS` adds a higher-level interface that hides node
allocation and feels closer to a hash map:

```c
SKIPLIST_DECL_ACCESS(
    my, api_,
    key,      int,           /* key field, key type */
    value,    char *,        /* value field, value type */
    /* qblk -- build a query node from a key */
    { node->key = k; },
    /* rblk -- replace value on update */
    { free(node->value); node->value = v; }
);

api_skip_put_my(&list, 42, strdup("forty-two"));    /* insert or update */
api_skip_set_my(&list, 42, strdup("the answer"));   /* update only */

char *v = NULL;
if (api_skip_get_my(&list, 42, &v))
    printf("got: %s\n", v);

if (api_skip_contains_my(&list, 42))
    printf("present\n");

api_skip_del_my(&list, 42);
```

### Pool Allocator

`SKIPLIST_DECL_POOL(decl, prefix, field, capacity)` adds a fixed-size
slab allocator with cache-line-aligned slots and a lock-free free
list.  When the pool is full, allocations fall back to `malloc`.
Throughput improves significantly for sequential insert workloads
(roughly 2-3x in the benchmark suite).

```c
SKIPLIST_DECL_POOL(my, api_, entry, /* capacity = */ 4096)

api_pool_my_t pool;
api_pool_init_my(&pool);
my_t list;
api_skip_init_my(&list);
api_skip_set_pool_my(&list, &pool);
/* now alloc_node_my() pulls from the pool */
api_pool_destroy_my(&pool);
```

### Snapshots (MVCC)

`SKIPLIST_DECL_SNAPSHOTS(decl, prefix, field)` enables point-in-time
snapshots.  Each snapshot captures an immutable era marker; subsequent
mutations preserve the previous version of touched nodes so the
snapshot remains intact.

```c
SKIPLIST_DECL_SNAPSHOTS(my, api_, entry)

api_snap_my_t s1;
api_skip_snapshot_my(&list, &s1);    /* freeze an era */

api_skip_put_my(&list, 7, "after");  /* mutates current */

api_skip_restore_my(&list, &s1);     /* rewind to s1 */
api_skip_release_my(&list, &s1);     /* free preserved state */
```

Era counters are 64-bit; wrap-around handling is built in.

### Epoch-Based Reclamation (EBR)

`SKIPLIST_DECL_EBR(decl, prefix)` provides safe memory reclamation
for concurrent workloads: a removed node may still be reachable by
other threads.  Threads register with the EBR machinery and pin/unpin
around each operation; nodes are retired into a per-thread limbo list
and freed only when every thread has advanced past the retirement
epoch.

```c
SKIPLIST_DECL_EBR(my, api_)

api_ebr_my_t ebr;
api_ebr_init_my(&ebr);

int tid = api_ebr_register_my(&ebr);
api_ebr_pin_my(&ebr, tid);
my_node_t *n = api_skip_remove_my(&list, &q);
if (n) api_ebr_retire_my(&ebr, tid, n);
api_ebr_unpin_my(&ebr, tid);

api_ebr_destroy_my(&ebr);
```

EBR is mutually exclusive with `SKIPLIST_SINGLE_THREADED` and will
fail to compile if both are defined.

### Archive (Serialization)

`SKIPLIST_DECL_ARCHIVE(decl, prefix, field, write_blk, read_blk)`
adds binary serialization driven by user-supplied per-node blocks.
The library writes a header (magic, length, sizeof-record) and walks
the list invoking `write_blk` for each node; on read it reconstructs
the list invoking `read_blk` to materialize each node.

```c
FILE *fp = fopen("snapshot.bin", "wb");
api_skip_archive_write_my(&list, fp);
fclose(fp);

fp = fopen("snapshot.bin", "rb");
my_t restored;
api_skip_init_my(&restored);
api_skip_archive_read_my(&restored, fp);
fclose(fp);
```

### Validation and Visualization

`SKIPLIST_DECL_VALIDATE(decl, prefix, field)` adds
`api_skip_validate_my()` which walks every level checking
sortedness, height invariants, and forward/backward pointer
consistency.  Returns the number of errors found.

`SKIPLIST_DECL_DOT(decl, prefix, field)` adds
`api_skip_dot_my(FILE *fp)` and friends which write the structure as
a GraphViz DOT graph.  Convert with `dot -Tpdf out.dot -o out.pdf`.


## Concurrency Model

### Lock-Free Algorithm

Concurrent insert, search, and remove follow the Fraser/Harris design:

- **Marked pointers** -- the lowest bit of each `next` pointer flags
  a logically-deleted node.  Readers skip marked nodes; the next
  writer to traverse the predecessor unlinks the corpse.
- **Compare-and-swap (CAS) chains** -- inserts splice in bottom-up
  with a CAS at level 0, then opportunistically link upper levels.
  A failure on level >0 leaves a node valid but at lower height; a
  later splay pass corrects the imbalance.
- **Two-phase delete** -- mark the node, then physically unlink.

The level-0 list is doubly linked, so iteration in either direction is
O(1) per step.  Upper levels are singly linked.

### Memory Reclamation

Lock-free deletion would race against readers if removed nodes were
freed immediately.  Two options ship in the library:

1. **EBR** (`SKIPLIST_DECL_EBR`) -- the recommended approach.
   Threads register once, pin around each operation, retire removed
   nodes, and the system reclaims them when every thread has visited
   a fresh epoch.  No per-node bookkeeping; constant overhead per
   pin/unpin.
2. **Manual** -- if the application can guarantee quiescence by some
   other means (e.g. a single mutator thread, RCU-style read-side
   critical sections from a host runtime), `api_skip_free_node_*` may
   be called directly.

### Single-Threaded Mode

Defining `SKIPLIST_SINGLE_THREADED` before including `sl.h`:

```c
#define SKIPLIST_SINGLE_THREADED
#include "sl.h"
```

- Replaces every `atomic_load` / `atomic_store` / CAS with the
  equivalent plain operation.
- Drops the dependency on `<stdatomic.h>` and `<stdalign.h>`.
- Reduces code size noticeably; performance improves by roughly 30%
  for tight insert loops because the compiler can keep more state in
  registers.
- Disables `SKIPLIST_DECL_EBR`; using both produces a compile-time
  error.


## Splay Rebalancing

When `SKIPLIST_SPLAY_REBALANCE` is defined, every `SKIPLIST_SPLAY_INTERVAL`
operations the library walks the access path and adjusts node heights
based on a hit counter on each tower level:

- High-traffic nodes get promoted -- their tower grows to lift them
  closer to the head, shortening the search path.
- Low-traffic nodes get demoted -- their tower shrinks so the
  vertical fanout above them stays sparse.

The algorithm is from Aksenov et al. ("The Splay-List", 2020).
Trade-off: a few percent overhead per access in exchange for
near-optimal layout under skewed workloads (Zipf-like).  Uniform
random keys gain little.  The feature is opt-in because the underlying
skip-list is already O(log n) on uniform keys.


## Examples Walkthrough

Ten progressive examples in `examples/`.  Each is a single
self-contained program.

| File         | Topic                              | Key macros                                  |
|--------------|------------------------------------|---------------------------------------------|
| `ex01.c`     | Minimal skiplist                   | `SKIPLIST_ENTRY`, `SKIPLIST_DECL`           |
| `ex02.c`     | Key/value API                      | `SKIPLIST_DECL_ACCESS`                      |
| `ex03.c`     | Custom struct types                | Custom comparison block                     |
| `ex04.c`     | Iteration and positioning          | `SKIPLIST_FOREACH_H2T`, `position_*`        |
| `ex05.c`     | Duplicate keys                     | `insert_dup`, `dup`                         |
| `ex06.c`     | Snapshots                          | `SKIPLIST_DECL_SNAPSHOTS`                   |
| `ex07.c`     | Pool allocator                     | `SKIPLIST_DECL_POOL`                        |
| `ex08.c`     | Serialization                      | `SKIPLIST_DECL_ARCHIVE`                     |
| `ex09.c`     | Validation and DOT output          | `SKIPLIST_DECL_VALIDATE`, `SKIPLIST_DECL_DOT` |
| `ex10.c`     | Concurrent with EBR                | `SKIPLIST_DECL_EBR` + threads               |

Build all and run any example:

```sh
make examples
./examples/ex01
ASAN_OPTIONS=detect_leaks=1 ./examples/ex01      # explicit leak check
./examples/ex09 | dot -Tpdf -o ex09.pdf          # see ex09's structure
```


## Benchmarks

`make bench` builds and runs `bench/bench.c` at `-O2 -DNDEBUG`.  The
suite reports throughput (ops/sec) and per-op latency for:

- Sequential and random insert
- Sequential and random search (hit and miss)
- Sequential and random delete
- Mixed read-heavy (95/5) and write-heavy (50/50)
- Forward iteration
- Splay hotspot (10 hot keys repeatedly accessed)
- Latency distribution: p50, p90, p95, p99, p999, max
- Concurrent insert / search / mixed (2, 4, 8 threads)
- Pool allocator vs `malloc`

Indicative numbers from a recent x86_64 development laptop, N = 100000
keys, gcc 13.3 at `-O2`:

```
Sequential insert                611852 ops/s    1634 ns/op
Random insert                    168695 ops/s    5928 ns/op
Sequential search (hit)         1722553 ops/s     580 ns/op
Random search (hit)              306307 ops/s    3265 ns/op
Forward iteration             22300984 ops/s      45 ns/op
Pool insert (sequential)        1526033 ops/s     655 ns/op
Concurrent insert (8 threads)    650791 ops/s    1537 ns/op
Concurrent mixed 95/5 (8 thr.) 1208381 ops/s     828 ns/op
```

These are illustrative; rerun on your hardware for decisions.


## Performance Notes

- **Single-threaded mode** is roughly 30% faster than the lock-free
  default on tight insert loops.  Use it when concurrency is not
  needed.
- **Pool allocator** doubles to triples sequential insert throughput
  by amortizing `malloc` and improving cache locality.
- **Splay rebalancing** is a per-access overhead; only enable it when
  the access distribution is heavily skewed.
- **Splay interval** trades latency variance for amortized cost.
  Lower intervals keep the structure tighter but spread cost over
  every operation; higher intervals concentrate cost in occasional
  passes.
- **Maximum height** affects stack allocations on the lookup path
  (one pointer + one counter per level).  The default 64 is enough
  for billions of entries; reducing it for small workloads saves
  cache footprint.
- The level-0 list is doubly linked; the `T2H` iterator is therefore
  O(1) per step rather than the O(log n) of a typical skiplist
  reverse walk.


## Portability

Tested on:

- Linux x86_64 / aarch64 with gcc 13/15 and clang 18/21
- Nix / NixOS (the development flake covers x86_64-linux, i686-linux,
  aarch64-linux, riscv64-linux, x86_64-darwin, aarch64-darwin)

Portability shims in `sl.h`:

- **Atomics** -- `<stdatomic.h>` on conforming compilers; no-op stubs
  under `SKIPLIST_SINGLE_THREADED`.
- **Alignment** -- `_Alignas` / `aligned_alloc` on C11; MSVC uses
  `__declspec(align)` and `_aligned_malloc`.
- **`__typeof__`** -- GNU extension where available; `void *` under MSVC
  (assignment converts implicitly).
- **`getpid`** -- `<unistd.h>` on POSIX; `<process.h>` `_getpid()` on
  Windows.

The implementation does not call any system-specific syscalls, signal
APIs, or thread-spawning routines.  Concurrency primitives come
exclusively from `<stdatomic.h>` and (in tests/examples only) the
host's pthread implementation.


## Contract for Comparison Functions

The user-supplied comparison block must implement a strict total order:

1. **Deterministic.**  `cmp(a, b)` returns the same value every time
   given the same inputs.
2. **Antisymmetric.**  `cmp(a, b) > 0` iff `cmp(b, a) < 0`, and
   `cmp(a, b) == 0` iff `cmp(b, a) == 0`.
3. **Transitive.**  If `cmp(a, b) > 0` and `cmp(b, c) > 0`, then
   `cmp(a, c) > 0`.

Violating any of these can corrupt the structure or produce a segfault
under concurrent mutation.  The library does not validate the
comparator at runtime.


## Testing and CI

The test infrastructure uses the vendored
[munit](https://nemequ.github.io/munit/) framework
(`tests/munit.{c,h}`).

- `tests/test.c` -- 28 single-threaded tests covering init, insert,
  duplicate insert, search, remove, the access API, navigation, edge
  cases, splay behavior, memory management, the pool allocator, EBR
  basics, validation, head-height growth/shrink, a 100k-key stress
  test, snapshots (5 sub-tests), and archive (3 sub-tests).
- `tests/test_concurrent.c` -- 6 multi-threaded tests covering
  concurrent insert / search / delete, a mixed workload, EBR
  correctness, and pool contention.

CI runs three independent jobs on every push and pull request:

- **build** -- gcc and clang matrix; runs the Makefile target sequence
  including `test_tsan` on the gcc row.
- **valgrind** -- non-sanitized build under `valgrind --leak-check=full`.
- **meson** -- `{none, address, thread, undefined}` sanitizer matrix.
- **autotools** -- bootstrap, configure, build, `make distcheck`.

Mirror workflow at `.forgejo/workflows/ci.yml` for Codeberg.


## Project Layout

```
include/sl.h               The implementation (single header)
tests/test.c               Single-threaded unit tests
tests/test_concurrent.c    Multi-threaded tests
tests/munit.{c,h}          Vendored test harness
examples/ex01..ex10        Progressive examples
bench/bench.c              Throughput / latency benchmark
man/skiplist.7             Conceptual overview man page
man/sl.h.3                 API reference man page
Makefile                   Plain-make build
configure.ac               Autoconf configure script source
config.mk.in               Make variables substituted by configure
autogen.sh                 Bootstraps configure from configure.ac
meson.build, */meson.build Meson build files
flake.nix                  Nix dev shell + package output
.github/workflows/ci.yml   GitHub Actions CI
.forgejo/workflows/ci.yml  Codeberg / Forgejo Actions mirror
.github/dependabot.yml     CI dependency updates
skiplist.pc.in             pkg-config template
LICENSE                    Dual ISC OR MIT
README.md                  This file
```


## License

```
SPDX-License-Identifier: ISC OR MIT

Copyright (c) 2024 Gregory Burd <greg@burd.me>.  All rights reserved.
```

Dual-licensed under the ISC License and the MIT License.  Pick whichever
is most convenient for your project.  Full license text is in the
`LICENSE` file and at the top of `include/sl.h`.


## References

The implementation draws from the following sources:

- William Pugh.  *Skip lists: a probabilistic alternative to balanced
  trees.*  Communications of the ACM, 33(6):668-676, June 1990.
  [DOI](https://doi.org/10.1145/78973.78977)
  [PDF](https://www.cl.cam.ac.uk/teaching/2005/Algorithms/skiplists.pdf)

- Vitaly Aksenov, Dan Alistarh, Alexandra Drozdova, Amirkeivan Mohtashami.
  *The Splay-List: A Distribution-Adaptive Concurrent Skip-List.*
  arXiv:2008.01009, 2020.
  [PDF](https://arxiv.org/pdf/2008.01009.pdf)

- Keir Fraser.  *Practical Lock-Freedom.*  Ph.D. thesis, University
  of Cambridge, 2004.

- Timothy L. Harris.  *A pragmatic implementation of non-blocking
  linked-lists.*  Distributed Computing, 2001.

- Jeseong Yeon, Leeju Kim, Youil Han, Hyeon Gyu Lee, Eunji Lee,
  Bryan Suk Joon Kim.  *JellyFish: A Fast Skip List with MVCC.*
  Middleware 2020.


## Acknowledgments

Portions of the design are adapted from prior open-source skiplist
implementations.  Each is independently licensed:

- [greensky00/skiplist](https://github.com/greensky00/skiplist) by
  Jung-Sang Ahn (MIT)
- [paulross/skiplist](https://github.com/paulross/skiplist) by
  Paul Ross (MIT)
- [JP-Ellis/rust-skiplist](https://github.com/JP-Ellis/rust-skiplist)
  by Joshua Ellis (MIT)
- [zhpengg/skiplist](https://gist.github.com/zhpengg/2873424) by
  Zhipeng Li (Public Domain)

The vendored test harness `tests/munit.{c,h}` is by Evan Nemerson and
is licensed under the MIT License.
