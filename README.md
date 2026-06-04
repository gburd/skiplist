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
`log n`. On a skewed workload — a hot subset of keys dominating — that is a
measurable win.

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

## When should I use it?

| Use case | Reach for |
|----------|-----------|
| Skewed, single-threaded access (a hot key subset) | **`splaylist`** |
| Roughly uniform access | [`std::collections::BTreeMap`] — better constants |
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
- Differential property tests (`proptest`) check every operation against
  `BTreeMap`/`BTreeSet`, including an aggressive mode that rebalances on every
  mutation, and forward/reverse iteration and range equivalence.
- Internal unit tests assert the structural invariants (level 0 is a complete,
  strictly-ascending, doubly-linked list; every upper level is a sorted
  subsequence) hold under churn, and that hot keys end up taller than cold ones.
- `cargo miri test` is clean (no undefined behaviour), and `cargo deny` guards
  the dependency tree.

## Minimum supported Rust version

The MSRV is **1.70**. Raising it is a minor-version change.

## License

Licensed under either of [Apache-2.0](LICENSE-APACHE) or [MIT](LICENSE-MIT) at
your option. The C ancestor is dual-licensed ISC OR MIT.

[`std::collections::BTreeMap`]: https://doc.rust-lang.org/std/collections/struct.BTreeMap.html
[`crossbeam-skiplist`]: https://docs.rs/crossbeam-skiplist
