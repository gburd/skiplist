//! `splaylist` — a self-balancing skip list.
//!
//! A skip list is a probabilistic, sorted, linked structure that supports
//! `O(log n)` search, insertion, and removal on average by stacking
//! "express lane" forward pointers on top of an ordinary sorted list. A
//! *splay list* adds adaptivity: each node tracks how often it is accessed,
//! and the structure periodically reshapes itself so that frequently-used
//! keys are promoted to taller towers and become cheaper to reach, while
//! rarely-used keys sink. Under a skewed access pattern the expected search
//! cost approaches the entropy of the access distribution rather than a flat
//! `log n` — the property proven for the splay list of Aksenov et al. (2020).
//!
//! This crate is the safe-Rust descendant of the C `sl.h` splay list. It
//! contains **no `unsafe` code** (`#![forbid(unsafe_code)]`): nodes live in a
//! single arena `Vec` and links are array indices, so there are no raw
//! pointers, no manual lifetimes, and no reclamation hazards.
//!
//! # Which collection should I use?
//!
//! Note up front: on raw speed [`BTreeMap`](std::collections::BTreeMap) beats
//! this and every other skip list — cache-friendly B-trees win on modern
//! hardware. Adaptation lowers splaylist's cost on skewed reads relative to
//! itself, but does not close that gap. Choose `splaylist` when you want a skip
//! list with access-frequency adaptation specifically.
//!
//! - [`SplayMap`] / [`SplaySet`] — when your workload is **skewed** (a hot
//!   subset of keys is accessed far more often than the rest) and
//!   **single-threaded**. The adaptive towers pay off precisely when some
//!   keys are much hotter than others.
//! - [`std::collections::BTreeMap`] — when access is roughly uniform; a
//!   B-tree has better constant factors and cache behaviour.
//! - [`crossbeam_skiplist`](https://docs.rs/crossbeam-skiplist) — when you
//!   need a **concurrent, lock-free** ordered map. `splaylist` is
//!   deliberately single-threaded; see [Concurrency](#concurrency).
//!
//! # Examples
//!
//! ```
//! use splaylist::SplayMap;
//!
//! let mut map = SplayMap::new();
//! map.insert(3, "three");
//! map.insert(1, "one");
//! map.insert(2, "two");
//!
//! assert_eq!(map.get(&2), Some(&"two"));
//! assert_eq!(map.len(), 3);
//!
//! // Iteration is always in ascending key order, regardless of how the
//! // internal towers have adapted.
//! let keys: Vec<_> = map.keys().copied().collect();
//! assert_eq!(keys, [1, 2, 3]);
//! ```
//!
//! # Adaptivity
//!
//! Every [`SplayMap::get`] records an access against the node (an interior
//! mutation through a [`Cell`](core::cell::Cell), so it is cheap and needs
//! only `&self`). Structural reshaping happens during mutating operations
//! ([`insert`](SplayMap::insert), [`remove`](SplayMap::remove),
//! [`get_mut`](SplayMap::get_mut)) on a fixed interval, and can be forced with
//! [`SplayMap::rebalance`]. The interval is configurable via
//! [`SplayMap::with_config`]; setting it to `0` disables adaptation, leaving a
//! plain randomized skip list.
//!
//! # Concurrency
//!
//! [`SplayMap`] is [`Send`] when `K` and `V` are, but it is **not** [`Sync`]:
//! reads mutate interior access counters, so shared concurrent access is not
//! permitted. For lock-free concurrent use reach for
//! [`crossbeam_skiplist`](https://docs.rs/crossbeam-skiplist) instead.
//!
//! # Feature flags
//!
//! - `std` *(default)* — link `libstd`. Disable for `no_std`; the crate then
//!   relies only on `alloc`.
//! - `serde` — derive-free [`Serialize`]/[`Deserialize`] for [`SplayMap`] and
//!   [`SplaySet`]. Heights and access counters are not persisted.
//! - `dot` — [`SplayMap::to_dot`] emits a `GraphViz` description of the level
//!   structure (requires `K: Display` and `V: Display`).
//!
//! [`Serialize`]: https://docs.rs/serde/latest/serde/trait.Serialize.html
//! [`Deserialize`]: https://docs.rs/serde/latest/serde/trait.Deserialize.html

#![cfg_attr(not(feature = "std"), no_std)]
#![cfg_attr(docsrs, feature(doc_cfg))]
#![forbid(unsafe_code)]
#![warn(missing_docs)]
#![warn(clippy::pedantic)]

extern crate alloc;

mod entry;
mod iter;
mod map;
mod set;

#[cfg(feature = "serde")]
mod serde_impl;

pub use entry::{Entry, OccupiedEntry, VacantEntry};
pub use iter::{IntoIter, Iter, IterMut, Keys, Range, Values, ValuesMut};
pub use map::{Config, SplayMap};
pub use set::{IntoIter as SetIntoIter, Iter as SetIter, Range as SetRange, SplaySet};
