//! Criterion benchmarks for `splaylist`, including a skewed-access scenario
//! that compares `SplayMap` against `std::collections::BTreeMap` — the
//! workload where adaptive towers are meant to pay off.

#![allow(
    missing_docs,
    clippy::cast_precision_loss,
    clippy::cast_possible_truncation,
    clippy::cast_sign_loss
)]

use std::collections::BTreeMap;
use std::hint::black_box;

use criterion::{criterion_group, criterion_main, BenchmarkId, Criterion, Throughput};
use splaylist::SplayMap;

/// A small deterministic PRNG so benchmarks are reproducible.
struct Rng(u64);
impl Rng {
    fn next(&mut self) -> u64 {
        let mut x = self.0;
        x ^= x >> 12;
        x ^= x << 25;
        x ^= x >> 27;
        self.0 = x;
        x.wrapping_mul(0x2545_f491_4f6c_dd1d)
    }
    /// A Zipf-ish skewed index in `0..n`: squares a uniform draw so small
    /// indices dominate.
    fn skewed(&mut self, n: u64) -> u64 {
        let u = (self.next() % n) as f64 / n as f64;
        ((u * u) * n as f64) as u64 % n
    }
}

fn bench_insert(c: &mut Criterion) {
    let mut group = c.benchmark_group("insert_sequential");
    for &n in &[1_000_u64, 10_000] {
        group.throughput(Throughput::Elements(n));
        group.bench_with_input(BenchmarkId::new("SplayMap", n), &n, |b, &n| {
            b.iter(|| {
                let mut m = SplayMap::new();
                for i in 0..n {
                    m.insert(black_box(i), i);
                }
                m
            });
        });
        group.bench_with_input(BenchmarkId::new("BTreeMap", n), &n, |b, &n| {
            b.iter(|| {
                let mut m = BTreeMap::new();
                for i in 0..n {
                    m.insert(black_box(i), i);
                }
                m
            });
        });
    }
    group.finish();
}

fn bench_skewed_lookup(c: &mut Criterion) {
    let n = 10_000_u64;
    let mut splay = SplayMap::new();
    let mut btree = BTreeMap::new();
    for i in 0..n {
        splay.insert(i, i);
        btree.insert(i, i);
    }
    // Warm the splay map so it adapts to the skewed pattern.
    let mut warm = Rng(1);
    for _ in 0..(n * 4) {
        let k = warm.skewed(n);
        let _ = splay.get(&k);
    }

    let mut group = c.benchmark_group("skewed_lookup");
    group.throughput(Throughput::Elements(1));
    group.bench_function("SplayMap", |b| {
        let mut rng = Rng(42);
        b.iter(|| {
            let k = rng.skewed(n);
            black_box(splay.get(&k))
        });
    });
    group.bench_function("BTreeMap", |b| {
        let mut rng = Rng(42);
        b.iter(|| {
            let k = rng.skewed(n);
            black_box(btree.get(&k))
        });
    });
    group.finish();
}

criterion_group!(benches, bench_insert, bench_skewed_lookup);
criterion_main!(benches);
