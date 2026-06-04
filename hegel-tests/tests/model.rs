//! Hegel (Hypothesis-backed) property tests for `splaylist`.
//!
//! The centerpiece is a stateful model test: a `SplayMap` is driven through a
//! random sequence of operations alongside a `BTreeMap` oracle, and the two are
//! required to agree after every step. This is the highest-value property for
//! an ordered-map data structure.
//!
//! These tests live in a separate (unpublished) crate because `hegeltest`
//! needs a recent toolchain and a Hypothesis server; keeping them out of the
//! main crate lets `cargo test` there stay dependency-light. Run with:
//! `cd hegel-tests && cargo test`.

use std::collections::BTreeMap;

use hegel::generators;
use hegel::TestCase;
use splaylist::SplayMap;

/// Stateful model: every operation is mirrored against a `BTreeMap`, and the
/// invariant checks full agreement (length, forward and reverse iteration).
struct MapModel {
    subject: SplayMap<u16, u32>,
    model: BTreeMap<u16, u32>,
}

#[hegel::state_machine]
impl MapModel {
    #[rule]
    fn insert(&mut self, tc: TestCase) {
        let k = tc.draw(generators::integers::<u16>());
        let v = tc.draw(generators::integers::<u32>());
        assert_eq!(self.subject.insert(k, v), self.model.insert(k, v));
    }

    #[rule]
    fn remove(&mut self, tc: TestCase) {
        let k = tc.draw(generators::integers::<u16>());
        assert_eq!(self.subject.remove(&k), self.model.remove(&k));
    }

    #[rule]
    fn get(&mut self, tc: TestCase) {
        let k = tc.draw(generators::integers::<u16>());
        assert_eq!(self.subject.get(&k), self.model.get(&k));
    }

    #[rule]
    fn get_mut(&mut self, tc: TestCase) {
        let k = tc.draw(generators::integers::<u16>());
        let v = tc.draw(generators::integers::<u32>());
        match (self.subject.get_mut(&k), self.model.get_mut(&k)) {
            (Some(a), Some(b)) => {
                *a = v;
                *b = v;
            }
            (None, None) => {}
            _ => panic!("get_mut presence disagreed for key {k}"),
        }
    }

    #[rule]
    fn contains(&mut self, tc: TestCase) {
        let k = tc.draw(generators::integers::<u16>());
        assert_eq!(self.subject.contains_key(&k), self.model.contains_key(&k));
    }

    #[rule]
    fn rebalance(&mut self, _: TestCase) {
        // Forcing adaptation must never change the logical contents; the
        // invariant below re-checks full agreement afterward.
        self.subject.rebalance();
    }

    #[invariant]
    fn agrees_with_model(&mut self, _: TestCase) {
        assert_eq!(self.subject.len(), self.model.len());
        assert!(
            self.subject.iter().eq(self.model.iter()),
            "forward iteration diverged from BTreeMap"
        );
        assert!(
            self.subject.iter().rev().eq(self.model.iter().rev()),
            "reverse iteration diverged from BTreeMap"
        );
        assert_eq!(self.subject.first_key_value(), self.model.iter().next());
        assert_eq!(self.subject.last_key_value(), self.model.iter().next_back());
    }
}

#[hegel::test]
fn splaymap_matches_btreemap(tc: TestCase) {
    let machine = MapModel {
        subject: SplayMap::new(),
        model: BTreeMap::new(),
    };
    hegel::stateful::run(machine, tc);
}

/// Range queries must agree with `BTreeMap` over arbitrary bounds. Drawn keys
/// are sorted into `lo <= hi` so the range is always well-formed.
#[hegel::test]
fn range_matches_btreemap(tc: TestCase) {
    let pairs: Vec<(u16, u32)> = {
        let n = tc.draw(generators::integers::<u8>()) as usize;
        (0..n)
            .map(|_| {
                (
                    tc.draw(generators::integers::<u16>()),
                    tc.draw(generators::integers::<u32>()),
                )
            })
            .collect()
    };
    let subject: SplayMap<u16, u32> = pairs.iter().copied().collect();
    let model: BTreeMap<u16, u32> = pairs.into_iter().collect();

    let mut a = tc.draw(generators::integers::<u16>());
    let mut b = tc.draw(generators::integers::<u16>());
    if a > b {
        core::mem::swap(&mut a, &mut b);
    }
    assert!(subject.range(a..b).eq(model.range(a..b)));
    assert!(subject.range(a..=b).eq(model.range(a..=b)));
}
