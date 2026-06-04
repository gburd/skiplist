//! Differential fuzz target: drive a `SplayMap` and a `BTreeMap` with the same
//! arbitrary operation stream and assert they stay in agreement.
//!
//! Run with: `cargo +nightly fuzz run differential`

#![no_main]

use std::collections::BTreeMap;

use arbitrary::Arbitrary;
use libfuzzer_sys::fuzz_target;
use splaylist::SplayMap;

#[derive(Arbitrary, Debug)]
enum Op {
    Insert(u8, u16),
    Remove(u8),
    Get(u8),
    Rebalance,
}

fuzz_target!(|ops: Vec<Op>| {
    let mut sl: SplayMap<u8, u16> = SplayMap::new();
    let mut bt: BTreeMap<u8, u16> = BTreeMap::new();
    for op in ops {
        match op {
            Op::Insert(k, v) => assert_eq!(sl.insert(k, v), bt.insert(k, v)),
            Op::Remove(k) => assert_eq!(sl.remove(&k), bt.remove(&k)),
            Op::Get(k) => assert_eq!(sl.get(&k), bt.get(&k)),
            Op::Rebalance => sl.rebalance(),
        }
        assert!(sl.iter().eq(bt.iter()));
        assert_eq!(sl.len(), bt.len());
    }
});
