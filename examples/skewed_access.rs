//! Demonstrates how `SplayMap` adapts to a skewed access pattern: a small set
//! of hot keys is queried far more often than the rest, and after the map
//! rebalances those keys end up in taller towers.
//!
//! Run with: `cargo run --example skewed_access`

use splaylist::SplayMap;

fn main() {
    let mut map: SplayMap<u32, u32> = (0..1_000).map(|k| (k, k)).collect();

    // A deterministic PRNG so the demo is reproducible.
    let mut state = 0x1234_5678_9abc_def0_u64;
    let mut next = || {
        state ^= state << 13;
        state ^= state >> 7;
        state ^= state << 17;
        state
    };

    // 90% of lookups hit one of ten hot keys; the rest are uniform.
    let hot: [u32; 10] = [7, 42, 99, 128, 256, 333, 512, 700, 815, 999];
    for _ in 0..200_000 {
        let key = if next() % 10 < 9 {
            hot[(next() % hot.len() as u64) as usize]
        } else {
            (next() % 1_000) as u32
        };
        let _ = map.get(&key);
    }

    // A few rebalance passes let the hot keys climb toward their target height.
    for _ in 0..8 {
        map.rebalance();
    }

    println!("after a 90/10 skewed workload over 1,000 keys:");
    println!("  total entries : {}", map.len());
    println!("  hot keys are still found in ascending-order iteration,");
    println!("  but now sit in taller towers so their lookups are cheaper.");
    println!();
    println!("  sample lookups: {:?}", hot.map(|k| (k, map.get(&k).copied())));
}
