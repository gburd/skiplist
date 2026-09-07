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


### Known limitation: contiguous hot ranges degrade without bound

The rebalance promotes any node whose hit ratio justifies a higher
level, evaluated per node in isolation.  When the hot set is a
*contiguous key range* rather than scattered keys, every node in that
range qualifies, so the upper levels fill with the whole range instead
of staying sparse.  Those levels converge on a dense linked list over
the range, and descending them costs more than the level-0 scan the
tower was supposed to skip.

This is not a bounded overhead -- it grows with the length of the run.
Measured with comparisons per lookup (load-independent), N = 100000,
90% of accesses into keys [0, 1000), tower PRNG pinned, splay OFF flat
at 25.8 throughout:

| lookups | cmp/op ON | vs OFF |
|--------:|----------:|-------:|
| 25,000 | 27.7 | +7% |
| 100,000 | 34.5 | +33% |
| 400,000 | 74.8 | +189% |
| 1,600,000 | 91.8 | +256% |

Sign-stable across ten tower seeds.  A long-lived process with a hot
key prefix or a recent-ID range is therefore the worst case for this
feature, and it is a common access pattern.

#### Why it happens, precisely

The target height is `h = K - 1 - log2(1/p)` where `p = u_hits/m_total`.
That is a function of `p` **alone**, so every node sharing an access
probability converges on the *same* height.  For a hot set of size `S`
taking traffic fraction `f`, each member has `p = f/S`, hence one shared
target:

| hot-set size `S` | `p` | target `h` |
|---:|---:|---:|
| 1 | 0.9 | 15.8 |
| 10 | 0.09 | 12.5 |
| 100 | 0.009 | 9.2 |
| 1000 | 0.0009 | 5.9 |

For `S = 1` this is exactly right: one node rises, the tower stays
sparse.  For `S = 1000` the model puts 1000 nodes on level ~6 --
measured 997 of 1000 at height 6, matching the predicted 5.9, so the
implementation is computing the paper's answer correctly.

The global population at that level is not itself alarming (1000 nodes
where a healthy list holds ~1700).  The damage comes from those nodes
being **adjacent**: levels 1..6 each become a near-complete copy of the
base list over the range.  Measured population of hot-range nodes,
levels 1-4: `525 / 254 / 132 / 66` with splay off versus
`1000 / 999 / 995 / 995` on.  A skiplist's bound needs level population
to decay geometrically; here it is flat.

So the defect is in the *model*, not the implementation: it carries no
notion of the hot set's width, nor of where promoted nodes sit relative
to one another.

#### Five fixes attempted and rejected

All five were implemented and measured; none shipped.  The consistent
failure mode is worth naming up front: **every gate that suppressed the
contiguous-range pathology also suppressed the feature in the case it
exists for.**  Two of the five looked like clear wins until the right
thing was measured.

The benchmark to beat, medians over 7 tower seeds, 200k lookups,
comparisons per lookup:

| config | scattered hot (10 keys) | contiguous range, 1.6M lookups |
|---|---:|---:|
| splay OFF | 30.70 | 25.8 |
| splay ON, no gate | 28.72 (-6.4%) | 91.8 (+256%) |

The target is to keep the left column below OFF while bringing the right
column near it.

1. **Fair-coin gate on each promotion** (restore geometric decay by
   making level `h` cost ~`2^h` attempts).  Fails because attempts
   repeat forever: `P(promoted)` after `k` rounds is `1 - 0.5^k`, so the
   collapse is delayed, not prevented.  Measured still 997 nodes at
   height 6 by 1.6M lookups.
2. **Level-0 local-maximum gate** (promote only a node strictly hotter
   than both level-0 neighbours).  The admitted set is a stable ~1/3 of
   nodes -- measured 325 of 998, matching theory -- but *stability is the
   problem*: the same 325 nodes then climb every level together.
   Measured 698 at height 6.
3. **Level-relative gate** (promote only if hotter at the target level
   than the level-`h` predecessor).  Fixed the symptom -- population
   decayed `512/236/120/72/31/13/16`, +23% instead of +256% -- but broke
   the paper's own height verification (5 of 6 cases in
   `tests/test_splay_verify.c`), because `sle_levels[lvl].hits` is never
   *accumulated* for `lvl > 0`: locate increments only level 0,
   promotion zeroes the new level, insert sets it to 1.  The comparison
   was against a counter that is always ~0, so it suppressed essentially
   all promotion.  It fixed the regression by disabling the feature.
4. **Span gate** (refuse a level that points at the same successor as
   the level below, since such a level skips nothing).  Motivated by a
   strong measurement: by 1.6M lookups **100% of promoted hot-range nodes
   spanned exactly one key**, so their towers were pure overhead.  The
   condition is structural rather than statistical, needs no new state,
   and is free (the successor is already loaded at that point).  It cut
   the regression to +38 cmp/op with properly decaying population.  But
   promotion is incremental, and at the bottom rung a lone hot node's
   level-1 successor *is* its level-0 successor, so the gate refused the
   first rung and no node could start climbing -- the pure-hot case sat
   at height 0 against an expected 13.  Exempting `new_h == 1` let it
   climb, but only to height 2, still failing 2 of 6 paper cases.  The
   deeper problem is that a lone hot key among cold neighbours and one
   hot key among 1000 hot neighbours are *locally indistinguishable* from
   the promoting node plus its successor, and the paper legitimately
   wants the first one tall.
5. **Successor-hotness gate** (refuse promotion when the level-0
   successor is comparably hot, i.e. `u_hits <= s_hits * 1.25`, on the
   theory that a hot successor means we are inside a wide hot region and
   the new level would skip a peer rather than a cold node).  This is the
   one that looked correct: the paper's verification **passed all six
   cases**, the lone hot key reached height 13, the contiguous regression
   fell from +256% to +55%, and population decayed cleanly
   (`339/262/143/93/56/32`).  It was rejected only because the scattered
   hot case -- the workload splay exists to serve -- went from -6.4% to
   **+2.3% versus splay off**, sign-stable across all seven seeds.  The
   cause is subtle: with two cold neighbours both at 0 hits the ratio
   test is trivially true, so the gate silently forbade promotion across
   the cold majority of the list.  Requiring the successor to also clear
   `asc_cond` did not recover the benefit.  Full-list height histograms
   were near-identical in all three configurations, so the lost benefit
   was not a visible structural change -- which is precisely why it would
   have passed a review that only checked the pathology.

What this rules out: any gate whose only inputs are the promoting node,
its immediate neighbours, and the existing counters.  Attempts 4 and 5
show that the pathological and intended configurations are
indistinguishable from that information alone.

What a real fix therefore needs is one of:

- **Per-level hit accounting that actually accumulates.**  Locate already
  visits every level; incrementing `sle_levels[lvl].hits` on the way down
  would make "what fraction of level-`L` traffic passes through this
  node" measurable, which is the spatial quantity the model lacks.  The
  demotion path already transfers those counters to the predecessor, so
  the semantics are half-built.  The cost is one relaxed increment per
  level per lookup on the hot path, and a shared counter per level is
  exactly the cache-line contention the existing `slh_head` hit counter
  already suffers -- so this needs measuring under TSAN and concurrency,
  not just single-threaded.
- **An explicit cap on promoted-node density per level**, which needs a
  per-level population counter and a re-derivation of the paper's height
  target under that constraint.

Either is a research change with its own correctness argument, not a
patch.  Until then the flag stays opt-in and the README documents a
contiguous hot range as a contraindication.


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
