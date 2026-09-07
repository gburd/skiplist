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


### Contiguous hot ranges: the failure mode, and the fix

The paper's target height `h = K - 1 - log2(1/p)` depends only on
`p = u_hits/m_total`, where `u_hits` counts exact matches.  That is a pure
per-key popularity with no dependence on position, so every node of equal
popularity earns an equal height however many neighbours share it.  For a
hot set of size `S` taking traffic fraction `f`, every member has
`p = f/S` and so targets one shared height:

| hot-set size `S` | `p` | target `h` |
|---:|---:|---:|
| 1 | 0.9 | 15.8 |
| 10 | 0.09 | 12.5 |
| 100 | 0.009 | 9.2 |
| 1000 | 0.0009 | 5.9 |

For `S = 1` this is exactly right: one node rises, the tower stays sparse.
For `S = 1000` the model puts 1000 nodes on level ~6 -- measured 997 of
1000 at height 6, matching the predicted 5.9, so the implementation was
computing the paper's answer correctly.  The *model* is what cannot
express hot-set width.

The global population at that level is not itself alarming (1000 nodes
where a healthy list holds ~1700).  The damage comes from those nodes
being **adjacent**: levels 1..6 each became a near-complete copy of the
base list over the range.  Measured population of hot-range nodes at
levels 1-4: `525 / 254 / 132 / 66` with splay off against
`1000 / 999 / 995 / 995` on.  A skiplist's bound needs level population
to decay geometrically; there it was flat.  By 1.6M lookups **100% of
promoted hot-range nodes spanned exactly one key** -- their towers skipped
nothing at all.

#### The fix

A tower level from `A` to `B` earns its keep only if the traffic wanting
to land strictly between them is small relative to the traffic landing on
`A`.  That is what "skipping" means, and crucially neither side of it
depends on any height, so it introduces no feedback loop.  (Per-level
*traversal* traffic does: it is itself a function of height, so using it
to set height is self-reinforcing.  Measured, that floats cold nodes up
two levels and fails the paper's own verification.)

Summing matches across the interval would be an `O(span)` level-0 walk,
far too slow for a heuristic on the access path, so the promotion path
compares against the single node that will follow it **at the target
level** -- the `B` in "a level from `A` to `B`".  That successor is already
recorded in the locate path (`path[h+1].succ`) and is covered by the
caller's EBR pin, so the check costs nothing extra.  The promotion is
refused when that successor is within 25% of the node's own traffic:
inside a hot region it is a peer, and beside a lone hot key it is cold.

Getting the *level* right matters more than it appears.  v1.1.5 compared
against the immediate **level-0** successor, which only approximates the
spanned interval when that interval is a single node.  Any hot set with
cold keys interleaved defeated it -- with every second key hot, each hot
node's level-0 neighbour is cold, so the test passed and 972 of 1000 hot
nodes still converged on one level, measuring 2.02x splay off.  That was
*worse* than the contiguous case the gate had been written for.

The guard on `u_hits` is the other load-bearing part.  Testing the ratio
unconditionally is trivially true for two cold neighbours both at zero
matches, which silently forbids promotion across the cold majority of the
list -- measured, that turned the scattered-hot case from -6.0% into
+2.3% against splay off while leaving the height histogram visually
unchanged.  Requiring the node to carry a meaningful share of total
traffic first (`m_total / 2^12`) confines the gate to the dense-hot-region
case it is for.

#### Measured result

Steady-state search cost, comparisons per lookup, N = 100000, medians over
three pinned tower seeds.  Costs are measured **after** warming, which
matters: a cumulative average over the whole run keeps climbing long after
the structure has settled, because early cheap lookups are progressively
diluted by later dear ones.  Measuring that way is how this regression was
initially mis-diagnosed as unbounded when it in fact converges.

| workload | OFF | v1.1.4 | v1.1.5 | v1.1.6 |
|---|---:|---:|---:|---:|
| uniform random | 31.4 | 31.4 | 31.4 | 31.4 |
| 10 scattered hot keys | 31.0 | 28.8 | 28.8 | 28.5 |
| contiguous 100 | 23.0 | -- | 26.3 | 24.6 |
| contiguous 1000 | 26.6 | 467.9* | 40.5 | 38.7 |
| contiguous 10000 | 29.7 | -- | 36.9 | 36.9 |
| 1000 hot keys, every 2nd | 26.9 | -- | 54.3 | 34.3 |
| 1000 hot keys, every 4th | 28.5 | -- | 42.6 | 32.4 |
| two hot blocks of 500 | 27.8 | -- | 55.8 | 44.0 |

\* measured at a longer warm-up; the v1.1.5 and v1.1.6 columns share a
common 800k-lookup warm-up so they are comparable to each other.

The intended case keeps its benefit, uniform access is untouched, and
every skewed shape improves.  Worst remaining case is two separated hot
blocks at 1.59x.

#### What is still not fixed

The gate consults one successor, not the whole spanned interval, so a hot
region dense enough that the target-level successor is *also* hot still
promotes part of itself.  That is why two hot blocks (1.59x) and a
contiguous 1000-key range (1.45x) remain above parity.  Closing the gap
needs either interval match-sums -- too slow on the access path as
written -- or per-level population accounting.

The upside also stays small even when the heuristic works, for a reason
unrelated to any of this: see the next section.

#### Approaches that did not work

Recorded because each looks plausible and costs a day to rediscover.

1. **Fair-coin gate on each promotion.**  Attempts repeat forever, so
   `P(promoted)` after `k` rounds is `1 - 0.5^k`: the collapse is delayed,
   not prevented.  Still 997 nodes at height 6 by 1.6M lookups.
2. **Level-0 local-maximum gate.**  Admits a stable ~1/3 of nodes
   (measured 325 of 998, matching theory) -- and stability is the problem:
   the same 325 then climb every level together.  698 at height 6.
3. **Level-relative gate** (compare per-level hit counters).  Broke 5 of 6
   Aksenov cases, because `sle_levels[lvl].hits` is never *accumulated*
   for `lvl > 0`: locate increments only level 0, promotion zeroes the new
   level, insert sets it to 1.  It was comparing against a counter that is
   always ~0, so it suppressed nearly all promotion -- fixing the
   regression by disabling the feature.
4. **Span gate** (refuse a level pointing at the same successor as the
   level below).  Well motivated by the "100% span one key" measurement,
   but promotion is incremental and at the bottom rung a lone hot node's
   level-1 successor *is* its level-0 successor, so nothing could start
   climbing -- pure-hot sat at height 0 against an expected 13.
5. **Populating per-level traversal counters in locate.**  Makes the ratio
   dimensionally consistent, and the demotion path already transfers those
   counters, so the accounting was half-built.  But traversal traffic is a
   function of height, so feeding it back into the height decision is
   self-reinforcing: cold nodes floated up two levels.  Using traversal
   traffic *alone* cannot bootstrap either, since a height-0 node has no
   upper level to have accumulated any.

### Why the upside is small even when it works

`_skip_locate_` always descends from the head's height to level 0 with
no early exit when the target is matched at an upper level.  Promoting
a hot node to height 12 therefore does not shorten the traversal that
finds it -- it only reduces how many nodes are compared on the way
down.  That is why a height adaptation which provably matches the
paper's target (see `tests/test_splay_verify.c`) converts into only
single-digit percent fewer comparisons.  Adding an early exit on an
upper-level match would change the linearization argument for the
lock-free path and has not been attempted.


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

Six test translation units cover the implementation surface:

| File | What it exercises |
|---|---|
| `tests/test.c` | Single-threaded API breadth: 47 tests covering every macro, including validator corruption injection and archive/DOT failure paths. |
| `tests/test_concurrent.c` | Multi-threaded behavior: 13 tests under heavy contention. |
| `tests/test_single.c` | The `SKIPLIST_SINGLE_THREADED` build path: 17 tests. |
| `tests/test_splay_verify.c` | Empirical Aksenov 2020 height verification: 2 tests, only meaningful with `-DSKIPLIST_SPLAY_REBALANCE`. |
| `tests/test_property.c` | Hegel (hegel-c) property tests: 9 properties driven against an independent reference model.  Opt-in (`make test_property`); needs a local hegel-c checkout, so it is not in CI. |
| (no separate source) | TSAN, ASan, UBSan, and valgrind variants of the above, built as distinct targets from the same sources. |
| `examples/ex01..ex10` | Doubling as smoke tests: `make run_examples` runs every example under ASan/LSan and fails on a non-zero exit.  Building them is not enough -- an out-of-bounds write inside an example's own archive callbacks only shows up when it runs. |

The CI matrix runs every translation unit twice -- once with the
default flags and once with `SKIPLIST_SPLAY_REBALANCE` defined --
so any regression in either code path fails fast.

Coverage is measured against the *union* of `include/sl.h` plus the
test files (because gcov attributes macro expansions to the file
that includes the header, not to the header itself).  The current
gates: 95% line, 95% function (both passing at 98% / 99%), and 72%
branch.

The branch figure needs two caveats to be meaningful.  It is the union
of seven builds whose branch sets are mutually exclusive (a branch
guarded by `SKIPLIST_SPLAY_REBALANCE` is uncoverable in the default
build and vice versa), so the merged number sits below any individual
build -- measured per build the same suites are 70-76%.  And
assertion-failure arms are excluded: `assert_*()` compiles to a branch
whose failure arm cannot run in a passing suite, so counting them
merely scales the denominator with test volume.  What genuinely remains
is CAS-retry machinery, marked-pointer help-unlink paths needing a peer
thread mid-delete, and allocation-failure arms -- all requiring fault
injection or scheduled interleaving.


## Bugs found by raising branch coverage

Three defects surfaced while writing tests for previously unexercised
branches.  All three were invisible to the existing suite because
nothing drove the relevant path.

1. **Infinite loop in `_skip_unlink_fully_` when deleting a key with
   duplicates.**  The function carries a `pred` hint down between
   levels as a scan-start optimisation.  It advanced that hint onto any
   node comparing `<=` the target, which includes an equal-key
   duplicate that sorts *after* the target at level 0.  Lower levels
   then began scanning past the node, so it was never unlinked, while
   the level-0 reachability check at the bottom of the loop kept
   finding it -- `remove()` spun forever.  Reproducing it needs enough
   keys for the head to grow past level 0 plus duplicates on the low
   keys, which is why ad-hoc small cases never hit it.  Fixed by
   separating the carried hint (`pred`, only ever moved to strictly
   smaller nodes) from the within-level walk (`scan`, which steps over
   equal keys).  Regression tests in both `tests/test.c` and
   `tests/test_single.c`.

2. **`SKIPLIST_SINGLE_THREADED` + `SKIPLIST_SPLAY_REBALANCE` did not
   compile.**  The single-threaded `_skip_atomic_fetch_add` shim was a
   fixed-signature helper taking `size_t *`, but `slh_splay_counter` is
   `uint32_t` -- and it is the only `uint32_t` user, reached only from
   the splay path.  No build combined the two flags, so the
   incompatible-pointer error never appeared.  Fixed by making the
   shims type-generic statement expressions.  The coverage target now
   builds this combination.

3. **The validator crashed or hung on the corruption it exists to
   diagnose.**  `_skip_integrity_check_` reported several classes of
   error and then continued using the corrupt value: a height at or
   above `SKIPLIST_MAX_HEIGHT` read past the level array (nodes
   allocate `[0, MAX-1]` and every level loop is inclusive, so the
   bound had to be `>=`, not `>`); a `NULL` `sle_levels` was reported
   and then dereferenced; a `NULL` `next[0]` was reported and then
   passed to the comparator; and a cycle in the level-0 chain was
   correctly detected but the subsequent node walk was unbounded and
   spun forever.  Fixed by clamping bad heights, returning early where
   the list is genuinely unwalkable (the iteration macro advances
   through `next[0]`, so there is no way to skip a node), and bounding
   the node walk at the claimed length plus slack.

The general lesson: a diagnostic that is only ever fed valid input is
untested code.  Feeding the validator deliberately corrupted lists,
one invariant at a time, is what found defect 3 -- and defects 1 and 2
came from building and running configurations that no target covered.


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
