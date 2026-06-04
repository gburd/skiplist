//! Differential property tests: every operation is applied to both a
//! `SplayMap`/`SplaySet` and the corresponding `std` collection, and the two
//! are required to agree. `BTreeMap`/`BTreeSet` are the oracle.

use std::collections::{BTreeMap, BTreeSet};

use proptest::prelude::*;
use splaylist::{Config, SplayMap, SplaySet};

#[derive(Debug, Clone)]
enum Op {
    Insert(u8, u16),
    Remove(u8),
    Get(u8),
    GetMut(u8, u16),
    Contains(u8),
}

fn op_strategy() -> impl Strategy<Value = Op> {
    prop_oneof![
        (any::<u8>(), any::<u16>()).prop_map(|(k, v)| Op::Insert(k, v)),
        any::<u8>().prop_map(Op::Remove),
        any::<u8>().prop_map(Op::Get),
        (any::<u8>(), any::<u16>()).prop_map(|(k, v)| Op::GetMut(k, v)),
        any::<u8>().prop_map(Op::Contains),
    ]
}

fn run_map_ops(config: Config, ops: &[Op]) -> Result<(), TestCaseError> {
    let mut sl = SplayMap::with_config(config);
    let mut bt = BTreeMap::new();
    for op in ops {
        match op {
            Op::Insert(k, v) => prop_assert_eq!(sl.insert(*k, *v), bt.insert(*k, *v)),
            Op::Remove(k) => prop_assert_eq!(sl.remove(k), bt.remove(k)),
            Op::Get(k) => prop_assert_eq!(sl.get(k), bt.get(k)),
            Op::Contains(k) => prop_assert_eq!(sl.contains_key(k), bt.contains_key(k)),
            Op::GetMut(k, v) => match (sl.get_mut(k), bt.get_mut(k)) {
                (Some(a), Some(b)) => {
                    *a = *v;
                    *b = *v;
                }
                (None, None) => {}
                _ => prop_assert!(false, "get_mut presence disagreed for {k}"),
            },
        }
        prop_assert_eq!(sl.len(), bt.len());
        prop_assert!(sl.iter().eq(bt.iter()), "forward iteration diverged");
        prop_assert!(
            sl.iter().rev().eq(bt.iter().rev()),
            "reverse iteration diverged"
        );
        prop_assert_eq!(sl.first_key_value(), bt.iter().next());
        prop_assert_eq!(sl.last_key_value(), bt.iter().next_back());
    }
    // Range queries must agree across several windows.
    for (lo, hi) in [(0u8, 255u8), (40, 200), (100, 101), (0, 1)] {
        prop_assert!(
            sl.range(lo..hi).eq(bt.range(lo..hi)),
            "range {lo}..{hi} diverged"
        );
        prop_assert!(sl.range(lo..=hi).eq(bt.range(lo..=hi)));
    }
    prop_assert!(sl.range(..).eq(bt.range(..)));
    Ok(())
}

proptest! {
    /// Default (adaptive) configuration.
    #[test]
    fn map_matches_btreemap(ops in prop::collection::vec(op_strategy(), 0..600)) {
        run_map_ops(Config::default(), &ops)?;
    }

    /// Aggressive adaptation: rebalance on every mutating op. Stresses the
    /// promote/demote paths far harder than production settings.
    #[test]
    fn map_matches_btreemap_aggressive(ops in prop::collection::vec(op_strategy(), 0..600)) {
        run_map_ops(Config { splay_interval: 1, seed: 0xdead_beef }, &ops)?;
    }

    /// Adaptation disabled: a plain randomized skip list must also agree.
    #[test]
    fn map_matches_btreemap_no_adapt(ops in prop::collection::vec(op_strategy(), 0..600)) {
        run_map_ops(Config { splay_interval: 0, seed: 1 }, &ops)?;
    }

    /// An explicit full rebalance never changes the logical contents.
    #[test]
    fn rebalance_preserves_contents(pairs in prop::collection::vec((any::<u16>(), any::<u16>()), 0..400)) {
        let mut sl: SplayMap<u16, u16> = pairs.iter().copied().collect();
        let before: Vec<_> = sl.iter().map(|(k, v)| (*k, *v)).collect();
        sl.rebalance();
        let after: Vec<_> = sl.iter().map(|(k, v)| (*k, *v)).collect();
        prop_assert_eq!(before, after);
    }

    /// `SplaySet` agrees with `BTreeSet`.
    #[test]
    fn set_matches_btreeset(vals in prop::collection::vec(any::<u8>(), 0..400)) {
        let mut ss = SplaySet::new();
        let mut bs = BTreeSet::new();
        for v in &vals {
            prop_assert_eq!(ss.insert(*v), bs.insert(*v));
        }
        prop_assert!(ss.iter().eq(bs.iter()));
        for v in &vals {
            prop_assert_eq!(ss.remove(v), bs.remove(v));
            prop_assert!(ss.iter().eq(bs.iter()));
        }
        prop_assert!(ss.is_empty());
    }

    /// Round-trip through `into_iter` yields ascending, complete contents.
    #[test]
    fn into_iter_sorted_and_complete(pairs in prop::collection::vec((any::<u16>(), any::<u16>()), 0..300)) {
        let sl: SplayMap<u16, u16> = pairs.iter().copied().collect();
        let mut bt: BTreeMap<u16, u16> = BTreeMap::new();
        bt.extend(pairs);
        let collected: Vec<_> = sl.into_iter().collect();
        let expected: Vec<_> = bt.into_iter().collect();
        prop_assert_eq!(collected, expected);
    }
}
