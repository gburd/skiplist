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

| Suite                              | Tests | Status |
|------------------------------------|-------|--------|
| Unit (default)                     |    33 | pass   |
| Concurrent (default)               |    13 | pass   |
| TSAN (default)                     |    13 | pass   |
| Property-based (Hegel/hegel-c)     |     9 | pass   |
| Unit (splay rebalance enabled)     |    33 | pass   |
| Concurrent (splay rebalance)       |    13 | pass   |
| TSAN (splay rebalance)             |    13 | pass   |
| Splay-verify (Aksenov height target) |   2 | pass   |
| Single-threaded mode               |     6 | pass   |
| ASan + LSan + UBSan                |    33 | pass   |
| Valgrind                           |    33 | pass   |

Verified on Linux x86_64 with gcc 13/15 and clang 18/21.  The
implementation is C11 with no Linux-specific syscalls; macOS, the BSDs,
and Windows (MSVC) are supported by the portability shims in `sl.h`.

Coverage on the implementation surface (`include/sl.h` plus the
three test translation units that instantiate its macros): 97% line,
99% function, 61% branch.  Branch coverage of lock-free CAS retry and
EBR contention paths is hard to exercise deterministically and is
tracked separately rather than gated.

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

    ex_node_t *n;
    api_skip_alloc_node_ex(&n);
    n->key = 42;
    n->value = strdup("forty-two");
    api_skip_insert_ex(&list, n);

    ex_node_t query = { .key = 42 };
    ex_node_t *found = api_skip_position_eq_ex(&list, &query);
    (void)found;

    api_skip_free_ex(&list);
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
make test              # 33 unit tests under ASan + LSan + UBSan
make test_concurrent   # 13 concurrent tests under ASan + LSan + UBSan
make test_tsan         # 13 concurrent tests under ThreadSanitizer
make test_property     # 9 Hegel property tests (needs hegel-c; see below)
make test_splay        # full suite with -DSKIPLIST_SPLAY_REBALANCE
make test_tsan_splay   # TSAN with splay rebalancing
make test_single       # SKIPLIST_SINGLE_THREADED build (6 tests)
make test_all          # all of the above (except test_property)
make valgrind          # unit tests under valgrind (no sanitizers)
make examples          # build all 10 examples
make bench             # build and run the benchmark suite
make coverage          # gcov + gcovr; gates on >=95% line + function
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
| `SKIPLIST_ARCHIVE_MAX_RECORD` | `1<<28`        | Upper bound (bytes) on a single serialized node record                  |
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

int           api_skip_init_my(my_t *list);
void          api_skip_free_my(my_t *list);
void          api_skip_release_my(my_t *list);
size_t        api_skip_length_my(my_t *list);
int           api_skip_is_empty_my(my_t *list);
int           api_skip_alloc_node_my(my_node_t **out);
void          api_skip_free_node_my(my_t *list, my_node_t *node);
int           api_skip_insert_my(my_t *list, my_node_t *node);
int           api_skip_insert_dup_my(my_t *list, my_node_t *node);
int           api_skip_remove_node_my(my_t *list, my_node_t *query);
my_node_t    *api_skip_position_eq_my(my_t *list, my_node_t *query);
my_node_t    *api_skip_position_lt_my(my_t *list, my_node_t *query);
my_node_t    *api_skip_position_lte_my(my_t *list, my_node_t *query);
my_node_t    *api_skip_position_gt_my(my_t *list, my_node_t *query);
my_node_t    *api_skip_position_gte_my(my_t *list, my_node_t *query);
my_node_t    *api_skip_head_my(my_t *list);
my_node_t    *api_skip_tail_my(my_t *list);
my_node_t    *api_skip_next_node_my(my_t *list, my_node_t *node);
my_node_t    *api_skip_prev_node_my(my_t *list, my_node_t *node);
/* ... */
```

### Core Operations

`init` zeroes the list, allocates the head and tail sentinels, and
seeds the random-height PRNG.

`insert` adds a node to the list.  Returns `0` on success, non-zero if
a node with the same key already exists (use `insert_dup` to allow
duplicates).

`remove_node` searches by key, physically unlinks the matching node
from every level, and reports success.  Under concurrent mode the node
is retired through EBR rather than freed immediately; because it is
fully unlinked before being retired, reclamation never frees a node a
concurrent traversal can still reach.

`free` walks the list and frees every node plus the sentinels.  In
concurrent mode, the caller is responsible for ensuring no other thread
is operating on the list.  `release` is the same but keeps the
sentinels, leaving an empty reusable list.

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
slab allocator with cache-line-aligned slots.  Allocation claims a slot
by CAS-ing that slot's own state word from free to used, guided by a
rotating cursor for O(1)-amortised probing.  Because there is no shared
recyclable free-list head, the allocator is unconditionally free of the
ABA problem -- there is no version tag that could ever wrap.  When the
pool is full, allocations fall back to `malloc`.  Throughput improves
significantly for sequential insert workloads (roughly 2-3x in the
benchmark suite).

```c
SKIPLIST_DECL_POOL(my, api_, entry, /* capacity hint = */ 4096)

_skip_pool_my_t pool;
api_skip_pool_init_my(&pool, 4096);
my_t list;
api_skip_init_my(&list);

my_node_t *node;
if (api_skip_pool_alloc_node_my(&pool, &node) == 0) {  /* else ENOMEM */
    node->key = 42;
    api_skip_insert_my(&list, node);
}
/* free a node back to the pool (runs your free block first): */
/* api_skip_pool_free_node_my(&pool, &list, node); */

api_skip_free_my(&list);
api_skip_pool_destroy_my(&pool);
```

### Snapshots (MVCC)

`SKIPLIST_DECL_SNAPSHOTS(decl, prefix, field)` enables point-in-time
snapshots.  Each snapshot captures an immutable era marker; subsequent
mutations preserve the previous version of touched nodes so the
snapshot remains intact.

```c
SKIPLIST_DECL_SNAPSHOTS(my, api_, entry)

api_skip_snapshots_init_my(&list);       /* enable snapshots */
uint64_t era = api_skip_snapshot_my(&list);  /* freeze an era */

api_skip_put_my(&list, 7, "after");      /* mutates current */

api_skip_restore_snapshot_my(&list, era);    /* rewind to era */
api_skip_release_snapshots_my(&list);        /* free preserved state */
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

_skip_ebr_my_t ebr;
api_skip_ebr_init_my(&ebr);
api_skip_ebr_attach_my(&list, &ebr);   /* removals now defer via EBR */

int tid = api_skip_ebr_register_my(&ebr);
api_skip_ebr_pin_my(&ebr, tid);
api_skip_remove_node_my(&list, &q);    /* auto-retires the node via EBR */
api_skip_ebr_unpin_my(&ebr, tid);
api_skip_ebr_unregister_my(&ebr, tid); /* release the slot for reuse */

api_skip_ebr_drain_my(&ebr);           /* at shutdown, no threads active */
```

EBR is mutually exclusive with `SKIPLIST_SINGLE_THREADED` and will
fail to compile if both are defined.

### Archive (Serialization)

`SKIPLIST_DECL_ARCHIVE(decl, prefix, field, write_blk, read_blk)`
adds binary serialization driven by user-supplied per-node blocks.
The library writes a header (magic, length, sizeof-record) and walks
the list invoking `write_blk` for each node; on read it reconstructs
the list invoking `read_blk` to materialize each node.

`write_blk` receives a scratch buffer sized from your `sizeof` block
(`bufsize` gives its capacity) and must not write past it.  `read_blk`
receives the deserialized record and its length `bytes`, and **must
validate `bytes` before reading** -- the length comes from the input
stream and may be hostile.  Records larger than
`SKIPLIST_ARCHIVE_MAX_RECORD` (default 256 MiB, overridable) are
rejected on both read and write so a malformed archive cannot drive an
unbounded allocation.

```c
FILE *fp = fopen("snapshot.bin", "wb");
api_skip_serialize_my(&list, fp);
fclose(fp);

fp = fopen("snapshot.bin", "rb");
my_t restored;
api_skip_init_my(&restored);
api_skip_deserialize_my(&restored, fp);
fclose(fp);
```

### Validation and Visualization

`SKIPLIST_DECL_VALIDATE(decl, prefix, field)` adds
`_skip_integrity_check_my(&list, flags)` which walks every level
checking sortedness, height invariants, and forward/backward pointer
consistency.  Returns the number of errors found.  `flags` bit 0
skips concurrent-only checks for single-threaded use; bit 1 exits on
the first error.

`SKIPLIST_DECL_DOT(decl, prefix, field)` adds
`api_skip_dot_my(FILE *os, my_t *list, size_t nsg, char *msg, fn)` and
`api_skip_dot_end_my(FILE *os, size_t nsg)` which write the structure
as a GraphViz DOT graph.  Convert with `dot -Tpdf out.dot -o out.pdf`.


## Concurrency Model

### Lock-Free Algorithm

Concurrent insert, search, and remove follow the Fraser/Harris design:

- **Marked pointers** -- the lowest bit of each `next` pointer flags
  a logically-deleted node.  Readers skip marked nodes, and both the
  forward iterators and the `position_*` scans unmark before
  dereferencing, so a marked pointer is never followed into.
- **Compare-and-swap (CAS) chains** -- inserts splice in bottom-up
  with a CAS at level 0, then opportunistically link upper levels.
  A failure on level >0 leaves a node valid but at lower height; a
  later splay pass corrects the imbalance.
- **Two-phase delete** -- mark the node at every level (level 0 last,
  the linearization point), then physically unlink it.  The remover
  unlinks the node from every level *by pointer identity*, retrying
  until the node is unreachable from the head, before retiring it.
  This preserves the invariant that a retired node is already fully
  unlinked, which epoch-based reclamation relies on -- otherwise a
  thread could pin and then follow a stale link into a reclaimed node.
  Insert cooperates by help-unlinking any logically-deleted successor
  it would otherwise link in front of, so a marked node never gains a
  fresh predecessor.

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

The algorithm is from Aksenov et al. ("The Splay-List", 2020).  A node
with hit ratio `u/T` (where `T` is the total accesses) settles at
height `K - 1 - log2(T/u)` where `K` is the head's current height,
placing it at depth `log2(1/p)` from the top -- exactly matching the
paper's `O(log(1/p))` search-cost target.  Hot keys ride near the top;
uniform random keys see no benefit and a few percent overhead.

The rebalance fires only on read-only access paths (search, contains,
position_*, update, and the duplicate-found path of insert).  It does
not fire from `remove`: promoting a node that is about to be physically
unlinked would create upper-level forward pointers the remove protocol
cannot clean up, leaving dangling references after EBR reclaims the
node.

Under concurrent mode the rebalance never dereferences nodes outside
the locate-recorded path (which are EBR-pinned by the active caller),
so the feature is safe to combine with multiple writer threads and
epoch-based reclamation.  The Aksenov design accepts that splay is a
heuristic and a CAS failure is acceptable; we revert the height bump
and try again on the next access.

### Verification

`tests/test_splay_verify.c` is the canonical empirical check that the
implementation reproduces the paper's predicted behavior.  It builds a
200-key list, issues 20,000 lookups Bernoulli-distributed between a
chosen hot key and uniformly-spread cold keys, and compares the hot
key's final height against the theoretical target.  Sample output:

```
Aksenov 2020 splay-list height verification
  workload: n=200 keys, total accesses=20000
  target:   h = K - 1 - log2(1/p) (+/- 1 level)

  case                       K     h   expect   result    delta
  ---------------------- ----- -----   ------   ------    -----
  p=1.000 (pure hot)        14    13       13     PASS       +0
  p=0.500 (half-hot)         7     6        5     PASS       +1
  p=0.250 (quarter)          7     4        4     PASS       +0
  p=0.125 (eighth)           7     3        3     PASS       +0
  p=0.062 (sixteenth)        7     2        2     PASS       +0
  p=0.005 (cold)             7     0        0     PASS       +0
```

Reading the table:

- `K` is the head's current height.  The structure grows the head
  whenever a node is promoted past the existing top level; with a
  pure-hot workload this drives `K` to roughly `log2(T)`, hence the
  larger value in the first row.
- `h` is the hot key's final height.
- `expect = K - 1 + log2(p)` (where `log2(p) <= 0`, clamped at 0).
- A `+/-1` tolerance is allowed because the rebalance fires only every
  `SKIPLIST_SPLAY_INTERVAL` accesses and adjusts by a single level per
  pass; a setting that has not yet converged can lag the target by one
  rung.  In practice five of the six cases land exactly on target and
  one (`p=0.5`) lands one above.

Splay is intentionally lazy: a node's height is re-evaluated only
when that node is accessed.  The verification test interleaves hot
and cold accesses with a deterministic LCG so the algorithm sees
both regimes during settling.  A naive "all hot first, then all cold"
driver leaves the hot key over-promoted because no later access on
it triggers a demotion.

### What this implies for access cost

A standard skip-list search costs `O(log_2 n)` regardless of access
pattern.  The splay-list converts that to `O(log_2(1/p))` for a key
with hit ratio `p`: a key hit half the time settles at depth 1, a key
hit a quarter of the time at depth 2, and so on.  Under heavily
skewed workloads (Zipfian, time-localized hot sets) the average
search cost approaches the entropy of the access distribution, which
can be much smaller than `log n`.

**This does not make a splay-list as fast as a B+tree.** A B+tree's
search cost is `O(log_b n)` where `b` is the fanout (typically 100 to
200), and each level is a single cache-line read plus an in-node
binary search -- a layout that the splay-list's pointer-chase per
level cannot match on cache locality.  The two structures optimize
orthogonal axes: the splay-list optimizes for the *information
content* of the access pattern, the B+tree for *cache-line layout*.
Splay-list wins on heavily skewed workloads where most accesses hit
a small hot set; the B+tree wins on uniform-random or scan-heavy
workloads where its cache layout dominates.  For the access patterns
where a B+tree shines (fanout-heavy in-memory indexes), reach for an
ART, CSB+, or Masstree instead.


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

The unit and concurrent suites use the vendored
[munit](https://nemequ.github.io/munit/) framework
(`tests/munit.{c,h}`); the property suite uses
[hegel-c](https://github.com/gburd/hegel-c).

- `tests/test.c` -- 33 single-threaded tests covering init, insert,
  duplicate insert, search, remove, the access API, navigation, edge
  cases, splay behavior, memory management, the pool allocator, EBR
  basics, validation, head-height growth/shrink, a 100k-key stress
  test, snapshots, archive, and the breadth of the generated API.
- `tests/test_concurrent.c` -- 13 multi-threaded tests covering
  concurrent insert / search / delete, a mixed workload, EBR
  correctness, pool contention, and a race-validation suite
  (overlapping inserts, shared-key insert/delete churn, pool claim
  exclusivity, EBR register/unregister churn, readers vs writers,
  and iteration during mutation), run under both ASan and TSAN.
- `tests/test_property.c` -- 9 Hegel property tests: a stateful model
  test against a reference map, archive round-trip, `position_*`
  boundary semantics, duplicate keys, deserialize robustness,
  navigation, snapshot/restore MVCC, pool integrity, and EBR slot
  reuse.  Built by `make test_property`, which needs a local hegel-c
  checkout and the hegel-core server (override `HEGEL_DIR` etc.; see
  the Makefile).

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
tests/test_concurrent.c    Multi-threaded + race-validation tests
tests/test_property.c      Hegel (hegel-c) property tests
tests/test_single.c        SKIPLIST_SINGLE_THREADED build exercise
tests/test_splay_verify.c  Aksenov 2020 height-target verification
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
DESIGN.md                  Architectural rationale and design history
LICENSE                    Dual ISC OR MIT
README.md                  This file
```

For the architectural rationale -- why the macro layer, why the
Fraser/Harris variant, why splay rebalance only fires from read-only
paths, the comparison to B+trees and ART, and what changed between
each `archive/*` branch and the current `main` -- see
[`DESIGN.md`](DESIGN.md).


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
