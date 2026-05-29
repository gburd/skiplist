# skiplist -- Design Notes

This document records why the implementation is shaped the way it
is.  The user-facing reference is `README.md`; the man pages
(`man/skiplist.7`, `man/sl.h.3`) describe the API.  This file is for
contributors and reviewers who need to understand the *rationale*
behind the structure.


## Goals (in priority order)

1. **Correctness under concurrent mutation.**  Every operation is
   either lock-free (default) or pure single-threaded; in either
   mode, the structure remains a sorted ordered-set with consistent
   invariants from any observer's point of view.
2. **Header-only, no runtime.**  The implementation lives in
   `include/sl.h` as a set of preprocessor macros that generate
   per-type code.  No `.c` file ships in the library; users include
   the header and instantiate.
3. **Adaptivity to skewed workloads.**  The signature feature is
   the splay-list rebalance from Aksenov et al. 2020, which makes
   search cost proportional to the entropy of the access pattern.
4. **Portability.**  Compiles on GCC, Clang, and MSVC; works on any
   POSIX system plus Windows; no Linux-specific syscalls.


## The macro layer

All public functionality is generated from preprocessor macros
parameterized by a type name (`decl`), function-name prefix
(`prefix`), and the field name where `SKIPLIST_ENTRY` is embedded
in the user's node struct.  The pattern follows the BSD `<sys/queue.h>`
and `<sys/tree.h>` conventions.

The cost of this approach is that the source is dense (every
function definition is one giant macro body with backslash
continuations) and gcov attributes coverage of macro bodies to the
`.c` file that includes the header rather than to `sl.h` itself.
The benefits are:

- **Type safety without `void *`.**  Every generated function takes
  the user's node type directly, no casts.
- **Inlinability.**  All generated code is `static`; the compiler
  can inline aggressively.
- **No runtime dispatch overhead.**  Comparison, free, and other
  user-supplied operations are inlined at every call site, not
  routed through a function pointer.

The dispatch table at the bottom of the list struct
(`slh_fns.compare_entries`, `slh_fns.free_entry`, etc.) is filled
in by `_skip_init_##decl` and used only by the few facilities that
genuinely need it: archive/snapshot deep-copy, validate, and EBR's
retire trampoline.


## The lock-free core (Fraser/Harris)

The default build is lock-free: insert, remove, and search may run
concurrently from any number of threads.  Two well-known techniques
provide correctness:

1. **Marked pointers.**  A logically deleted node is identified by
   a tag bit in its forward pointers (`_SKIP_MARK`).  Readers skip
   marked nodes (the iterators and ordered lookups unmark before
   dereferencing).  A remover marks the node at every level, then
   physically unlinks it from every level by pointer identity --
   retrying until it is unreachable from the head -- *before* it is
   retired, so epoch-based reclamation never frees a node that a
   concurrent traversal can still reach.  Insert cooperates by
   help-unlinking any marked successor it would otherwise link in
   front of, so a marked node never gains a fresh predecessor.
2. **CAS-based linking.**  Inserts splice into level 0 with a
   compare-and-swap on the predecessor's `next[0]` pointer; that
   CAS is the linearization point.  Upper levels are linked
   opportunistically with their own CASes.

The level-0 list is doubly linked.  This costs one extra pointer
per node but makes reverse iteration `O(1)` per step (a regular
skip-list would have to redo the search at level 0 to walk
backwards).  The `sle_prev` pointer is advisory under concurrency:
`api_skip_prev_validated_*` re-derives it via forward traversal if
the hint fails.

Upper-level links are singly linked; the rebalance machinery and
the find/unlink protocols never need to walk a level upward.


## Splay rebalance

### What it does

Every `SKIPLIST_SPLAY_INTERVAL` accesses (default 64), the
rebalance pass walks the locate-recorded path and adjusts heights
based on a per-level hit counter:

- Promote: if the matched node's hit count justifies a higher level
  relative to total accesses, raise the node by one level and
  splice it into that level's chain.
- Demote: if a node along the path has accumulated too few hits at
  its current top level, lower the node by one level.

The math (Aksenov 2020):

- Let `T` be the total accesses, `u` a node's hit count, `K` the
  head's current height.
- A node with hit ratio `p = u/T` should have height
  `K - 1 - log2(1/p)`, placing it at depth `log2(1/p)` below the
  head.
- The rebalance condition `u > T / 2^(K-h-1)` is equivalent to
  `h < K - 1 + log2(p)`, i.e., "promote while we are below the
  target height".

This puts hot keys close to the head and cold keys near the bottom.
The amortized search cost becomes `O(log(1/p))`, which is the
information-theoretic optimum for a comparison-based dictionary
under a known access distribution.

### Where it fires

- **Read-only paths** (search, contains, position_*, update,
  insert-with-duplicate) call `_skip_locate_with_splay_##decl`,
  which invokes the rebalance after `locate` returns.
- **Remove** uses plain `_skip_locate_##decl` and explicitly does
  *not* call the rebalance.  Splay-promoting a node that is about
  to be physically unlinked would create upper-level forward
  pointers the remove protocol cannot clean up, and a subsequent
  EBR reclamation would leave dangling references behind.  This
  separation was an actual fix to a bug: an earlier version put
  the splay call inside `locate` itself, producing a roughly 9%
  flake rate in `tests/test_concurrent_splay /ebr_correctness`
  (heap-use-after-free).  Moving it out and gating per-caller
  fixed that.

### Why only the path

The rebalance dereferences only nodes recorded in the locate path.
Those nodes were just visited within the active EBR pin and are
guaranteed not to be reclaimed before the caller unpins.  An older
version of the rebalance fell back to a backward scan from the
matched node when the path hint failed; that scan dereferenced
arbitrary nodes (potentially retired by other threads) and was the
source of additional concurrent bugs.  The current code drops the
backward scan entirely: if the path-recorded predecessor at
level `new_h` is not viable, the rebalance reverts the height bump
and tries again on the next access.  Splay is a heuristic; CAS
failures and missed promotions are acceptable.

### Lazy semantics

A node's height is re-evaluated **only** when that node is accessed.
Cold nodes that were once hot stay over-promoted until accessed
again and then demoted.  This is by design: the algorithm does no
background work and pays only on the hot path.  Verification tests
must interleave hot and cold accesses with a Bernoulli draw rather
than running them in two phases, otherwise the hot key stays pinned
at its peak height because no later access triggers a demotion.


## Memory reclamation (EBR)

A logically deleted node cannot be freed immediately under
concurrent reads: a peer thread may be in the middle of dereferencing
its `next` pointer.  Two approaches ship in the library:

- **Manual.**  When the caller can guarantee quiescence by some
  other means (e.g., a single mutator thread or a host-provided RCU),
  `prefix##skip_free_node_##decl` may be called directly.
- **Epoch-based reclamation** (`SKIPLIST_DECL_EBR`).  Threads
  register once, pin and unpin around each operation, and retired
  nodes are deferred until every thread has visited a fresh epoch.
  The `slh_ebr_retire` callback is wired into the list at
  `skip_ebr_attach_` time; `remove` dispatches through it
  automatically when EBR is attached.

EBR is mutually exclusive with `SKIPLIST_SINGLE_THREADED`.  Mixing
them produces a compile-time `_Static_assert` failure.


## Single-threaded mode

Defining `SKIPLIST_SINGLE_THREADED` before including `sl.h`:

- Replaces every `_skip_atomic_*` macro with the equivalent plain
  load/store/CAS expansion.  CAS becomes a "compare and assign"
  with no atomicity guarantees.
- Drops the `<stdatomic.h>` and `<stdalign.h>` dependencies.
- Disables `SKIPLIST_DECL_EBR` (compile-time error if both used).
- Reduces code size by roughly 20% and improves throughput by
  roughly 30% on tight insert loops because the compiler can keep
  more state in registers.

Single-threaded mode is exercised by its own test binary
(`tests/test_single.c`) since the flag has global header effect
and cannot coexist with the lock-free build in the same translation
unit.


## Pool allocator

`SKIPLIST_DECL_POOL` adds a fixed-capacity slab allocator with
cache-line-aligned slots.  Each slot is claimed by its own atomic
compare-and-swap on a per-slot state word, guided by a rotating
cursor; there is no shared free-list head, so the allocator is
free of the ABA problem (no version tag that could ever wrap).
The motivation: `malloc`/`free` per node dominates throughput at
small node sizes and creates allocator pressure that interacts
badly with concurrent inserts.  Allocations beyond the configured
capacity fall back to `malloc`, so the pool is a fast path with
graceful degradation rather than a hard cap.

The pool is used explicitly: `skip_pool_alloc_node_` claims a node
(returning `ENOMEM` when exhausted) and `skip_pool_free_node_`
returns it, so callers choose per-allocation whether to draw from
the pool or fall back to `skip_alloc_node_`.


## Snapshots (MVCC)

`SKIPLIST_DECL_SNAPSHOTS` provides point-in-time snapshots: take a
snapshot at era `e`, mutate, and `restore` to roll back to era
`e`'s view.  The implementation is single-threaded only --
snapshots use the same node header (`sle_era`, `slh_snap.cur_era`,
etc.) without atomics.

Each mutation that *would* lose information about the snapshot's
view (overwrite, remove of a snapshot-visible node) preserves the
old node into a per-snapshot list (`slh_snap.pres`).  Restore
walks the preserved list and re-installs the old nodes.

The 64-bit era counter is wide enough to be wrap-around-immune
for any realistic workload; the wrap path exists but is dead code
in practice.


## Validate / DOT

These two are diagnostics, not load-bearing features.

- `SKIPLIST_DECL_VALIDATE` -- adds `_skip_integrity_check_##decl`
  which walks every level checking sortedness, height invariants,
  and forward/backward pointer consistency.  Returns the number of
  errors found.  Not safe to call concurrently with mutation but
  useful in tests under `SKIPLIST_DIAGNOSTIC`.
- `SKIPLIST_DECL_DOT` -- emits the structure as a GraphViz
  multi-subgraph DOT file.  Useful for debugging shape pathologies.


## Test architecture

Five test translation units cover the implementation surface:

| File | What it exercises |
|---|---|
| `tests/test.c` | Single-threaded API breadth: 33 tests covering every macro. |
| `tests/test_concurrent.c` | Multi-threaded behavior: 7 tests under heavy contention. |
| `tests/test_single.c` | The `SKIPLIST_SINGLE_THREADED` build path: 6 tests. |
| `tests/test_splay_verify.c` | Empirical Aksenov 2020 height verification: 2 tests, only meaningful with `-DSKIPLIST_SPLAY_REBALANCE`. |
| (none -- a separate target) | TSAN, ASan, UBSan, valgrind variants of the above. |

The CI matrix runs every translation unit twice -- once with the
default flags and once with `SKIPLIST_SPLAY_REBALANCE` defined --
so any regression in either code path fails fast.

Coverage is measured against the *union* of `include/sl.h` plus the
test files (because gcov attributes macro expansions to the file
that includes the header, not to the header itself).  The current
gates: 95% line, 95% function (both passing at 97% / 99%).  Branch
coverage is reported at 59% but not gated -- many branches are
lock-free CAS retry paths and EBR contention paths that require
fault injection or scheduled interleaving to exercise
deterministically.


## Comparison to other in-memory ordered indexes

| | Skip-list | Splay-list (this) | B+tree | ART / radix | CSB+/Masstree |
|---|---|---|---|---|---|
| Search asymptotic | `O(log_2 n)` | `O(log_2(1/p))` for hot keys | `O(log_b n)` | `O(k)` k=key length | `O(log_b n)` |
| Adapts to access pattern | No | Yes (lazy) | No | No | No |
| Cache locality | Poor (pointer chase) | Same as skip-list | Excellent | Variable | Excellent |
| Lock-free option | Yes (Fraser/Harris) | Yes (this library) | Hard; usually mutex | Yes (some impls) | Optimistic |
| Range scan | O(1) per step | O(1) per step | O(1) per step | Depth-first walk | O(1) per step |
| Memory overhead | One pointer per level | Same | One per child + keys | Variable | Same as B+tree |

The splay-list's niche is **skewed workloads under concurrent
mutation**: the amortized search cost approaches the entropy of
the access distribution, the structure is lock-free, and the
deletion path is straightforward.  For uniform-random workloads on
modern hardware, a B+tree or ART will beat any skip-list variant on
wall-clock time because cache-line layout dominates pointer-chase
depth.


## What changed between earlier WIP and the current main

The repo's `archive/*` tags preserve five earlier branches.  The
current `main` is a substantial rework over each of them:

- `archive/splay-list-original`, `splay-list-wip1`, `splay-list-wip2`:
  these contain the early splay rebalance design.  The current
  rewrite fixed two compounding bugs (loop index excluded `path[0]`,
  `_skip_locate_` invoked rebalance even from inside `remove`) and
  replaced the racy backward-scan predecessor finder with a
  strict path-recorded one.
- `archive/lock-free-first-attempt`: an earlier lock-free attempt
  using a different protocol.  Superseded by the Fraser/Harris
  implementation now in `main`.
- `archive/snapshot-restore`: an iteration of the snapshot machinery
  with subtle era-tracking bugs.  Fixed and merged.

Anyone interested in the design evolution can `git log
archive/splay-list-original..main -- include/sl.h`.
