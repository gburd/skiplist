# Changelog

All notable changes to this project are documented here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and this project
adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.1.0]

Initial release: a single-threaded, self-balancing (splay) skip list in 100%
safe Rust, ported from the C `sl.h` implementation.

### Added

- `SplayMap<K, V>` — an ordered map with adaptive tower heights. Supports
  `insert`, `get`, `get_mut`, `get_key_value`, `contains_key`, `remove`,
  `remove_entry`, `first_key_value`, `last_key_value`, `range`, `clear`,
  `with_config`, `with_capacity`, and `rebalance`.
- `SplaySet<T>` — an ordered set built on `SplayMap<T, ()>`.
- A `BTreeMap`-style `entry` API (`Entry`, `OccupiedEntry`, `VacantEntry`).
- Iterators: `iter`, `iter_mut`, `keys`, `values`, `values_mut`, `range`, and
  owning `into_iter`. Map and key/value iterators are double-ended and
  exact-size.
- Standard trait impls: `Debug`, `Clone`, `Default`, `PartialEq`/`Eq`,
  `Index`, `FromIterator`, `Extend`, and `IntoIterator` (by value, `&`, `&mut`).
- `Config` for tuning the rebalance interval and PRNG seed.
- `serde` feature: `Serialize`/`Deserialize` for the map and set.
- `dot` feature: `SplayMap::to_dot` for GraphViz visualization.
- `no_std` support (requires `alloc`) by disabling default features.

[Unreleased]: https://codeberg.org/gregburd/skiplist/compare/v0.1.0...HEAD
[0.1.0]: https://codeberg.org/gregburd/skiplist/releases/tag/v0.1.0
