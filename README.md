# splaylist

[![crates.io](https://img.shields.io/crates/v/splaylist.svg)](https://crates.io/crates/splaylist)
[![docs.rs](https://docs.rs/splaylist/badge.svg)](https://docs.rs/splaylist)
[![CI](https://codeberg.org/gregburd/skiplist/actions/workflows/rust.yml/badge.svg)](https://codeberg.org/gregburd/skiplist/actions)
![MSRV](https://img.shields.io/badge/MSRV-1.70-blue)
![license](https://img.shields.io/crates/l/splaylist.svg)

A **self-balancing skip list** for Rust: an ordered map (`SplayMap`) and set
(`SplaySet`) whose node heights adapt to the access pattern, so frequently
used keys are promoted to taller towers and become cheaper to reach.

A skip list keeps keys in a sorted linked list with stacked "express lane"
forward pointers, giving `O(log n)` search, insert, and remove on average. A
*splay list* (Aksenov et al., 2020) adds adaptivity: each node counts how often
it is accessed, and the structure periodically reshapes itself so the expected
search cost tracks the *entropy* of the access distribution rather than a flat
`log n`. In practice that lowers splaylist's *own* cost on skewed reads; it does
not make it faster than `BTreeMap` (see [Benchmarks](#benchmarks)).

This crate is the safe-Rust descendant of a C `sl.h` splay list. It contains
**no `unsafe` code** (`#![forbid(unsafe_code)]`): nodes live in a single arena
`Vec` and links are array indices, so there are no raw pointers, no manual
lifetimes, and no memory-reclamation hazards.

```toml
[dependencies]
splaylist = "0.1"
```

## Example

```rust
use splaylist::SplayMap;

let mut map = SplayMap::new();
map.insert(3, "three");
map.insert(1, "one");
map.insert(2, "two");

assert_eq!(map.get(&2), Some(&"two"));

// Always iterates in ascending key order, however the towers have adapted.
let keys: Vec<_> = map.keys().copied().collect();
assert_eq!(keys, [1, 2, 3]);

// Entry API, ranges, and the usual collection traits are all available.
*map.entry(1).or_insert("?") = "ONE";
let mid: Vec<_> = map.range(2..).map(|(k, _)| *k).collect();
assert_eq!(mid, [2, 3]);
```

## Benchmarks

Honest summary up front: **`BTreeMap` is faster than every skip list measured
here, including this one.** Skip lists lose to cache-friendly B-trees on modern
hardware — that is a known, expected result, not a defect. Pick `splaylist`
when you want a skip list with access-frequency adaptation, not as a drop-in
speed upgrade over the standard library.

Methodology (see [`comparison/benches/compare.rs`](comparison/benches/compare.rs)):
keys and the full access sequence are generated once from a fixed seed and
replayed verbatim against every implementation; `u64 -> u64`, `N = 100_000`;
Criterion with `lto = "thin"`, `codegen-units = 1`. The skewed workload sends
~80% of lookups to a *scattered* random 1% of keys (clustering them at the low
end would be cheap in any skip list and would not isolate adaptation); every
map is warmed with the identical read sequence first, and splaylist is
additionally allowed to reach adapted steady state (its background rebalance
cost is not charged to the measured lookups). Reproduce with `cd comparison &&
cargo bench`.

Median time per operation on one x86-64 Linux machine (lower is better; your
numbers will differ):

| Operation (per op)        | `BTreeMap` | `splaylist` | `skiplist` | `crossbeam-skiplist` |
|---------------------------|-----------:|------------:|-----------:|---------------------:|
| insert (random order)     |     ~79 ns |     ~328 ns |    ~415 ns |              ~259 ns |
| lookup, uniform           |     ~42 ns |     ~263 ns |    ~218 ns |              ~152 ns |
| lookup, skewed (adapted)  |     ~38 ns |     ~229 ns |    ~196 ns |              ~140 ns |
| iterate (per element)     |    ~1.1 ns |     ~2.6 ns |    ~3.6 ns |               ~26 ns |

`crossbeam-skiplist` is a concurrent, lock-free structure; measuring it
single-threaded charges it atomic/epoch overhead it would amortize across
threads, so treat its column as context, not a head-to-head.

What the numbers show:

- **`BTreeMap` wins across the board** by roughly 5–6x on lookups. If raw
  ordered-map speed is all you need, use it.
- **Among skip lists, `splaylist` is competitive**: fastest insert and
  iteration of the three, slightly slower point lookups.
- **Adaptation helps, modestly.** splaylist's own skewed-lookup latency is
  ~13% better than its uniform latency (~229 ns vs ~263 ns) because hot keys
  climb into taller towers. The benefit grows as the hot set gets more
  concentrated and shrinks as access flattens; it does not close the gap to
  `BTreeMap`.

## When should I use it?

| Use case | Reach for |
|----------|-----------|
| You specifically want a skip list with access-frequency adaptation | **`splaylist`** |
| You just need a fast ordered map/set | [`std::collections::BTreeMap`] — faster here (see [Benchmarks](#benchmarks)) |
| Concurrent / lock-free ordered map | [`crossbeam-skiplist`] |

`splaylist` is deliberately single-threaded. `SplayMap` is `Send` when `K` and
`V` are, but it is **not** `Sync`: reads update interior access counters. For
shared concurrent access use [`crossbeam-skiplist`].

## Features

All optional features are off by default except `std`.

| Feature | Effect |
|---------|--------|
| `std` *(default)* | Link `libstd`. Disable for `no_std` (the crate then needs only `alloc`). |
| `serde`  | `Serialize`/`Deserialize` for `SplayMap` and `SplaySet`. |
| `dot`    | `SplayMap::to_dot` emits a GraphViz description of the level structure. |

```toml
splaylist = { version = "0.1", default-features = false, features = ["serde"] }
```

## How adaptation works

Every `get` records an access against the node it finds (a cheap interior
mutation through a `Cell`). Structural reshaping — promoting hot nodes and
demoting cold ones by one level at a time toward the target height
`K − 1 − log2(total / hits)` — happens during mutating operations on a fixed
interval, and can be forced with `SplayMap::rebalance`. The interval is set via
`SplayMap::with_config`; `splay_interval: 0` disables adaptation entirely,
leaving a plain randomized skip list.

The seed is configurable too, so a map's structural evolution is fully
deterministic for tests and reproducible benchmarks.

## Correctness and testing

- `#![forbid(unsafe_code)]` — the whole crate is safe Rust.
- Property-based tests use [Hegel](https://docs.rs/hegeltest) (Hypothesis
  under the hood). A stateful model test drives `SplayMap`/`SplaySet` through
  random operation sequences against `BTreeMap`/`BTreeSet` oracles and asserts
  agreement after every step (in default, aggressive, and disabled adaptation
  modes), plus range and `into_iter` equivalence. They live in `hegel-tests/`
  (a separate crate, since `hegeltest` needs a Hypothesis server); run with
  `cd hegel-tests && cargo test`.
- Example-based integration and unit tests run on every `cargo test` and are
  self-contained (no server required).
- Internal unit tests assert the structural invariants (level 0 is a complete,
  strictly-ascending, doubly-linked list; every upper level is a sorted
  subsequence) hold under churn, that hot keys end up taller than cold ones,
  and — as a regression guard — that heavy adaptation never collapses search
  cost below `O(log n)`.
- A `cargo fuzz` differential target lives in `fuzz/`.
- `cargo miri test` is clean (no undefined behaviour), and `cargo deny` guards
  the dependency tree.
- Comparison benchmarks against other Rust ordered maps live in `comparison/`
  (see [Benchmarks](#benchmarks)).

## Minimum supported Rust version

The MSRV is **1.70**. Raising it is a minor-version change.

## License

Licensed under either of [Apache-2.0](LICENSE-APACHE) or [MIT](LICENSE-MIT) at
your option. The C ancestor is dual-licensed ISC OR MIT.

[`std::collections::BTreeMap`]: https://doc.rust-lang.org/std/collections/struct.BTreeMap.html
[`crossbeam-skiplist`]: https://docs.rs/crossbeam-skiplist
