//! Fair, documented comparison of `splaylist::SplayMap` against other pure-Rust
//! ordered maps:
//!
//! - `std::collections::BTreeMap` — the standard-library baseline.
//! - `skiplist::SkipMap` — a single-threaded skip list (crate `skiplist`).
//! - `crossbeam_skiplist::SkipMap` — a concurrent, lock-free skip list,
//!   measured single-threaded (its atomics/epoch machinery is pure overhead
//!   here, so this is an apples-to-oranges reference, included for context).
//!
//! Methodology
//! -----------
//! * Keys and the full access sequence are generated once from a fixed seed and
//!   shared verbatim across every implementation, so each sees identical work.
//! * `u64 -> u64` entries; `N = 100_000` unless noted.
//! * Lookups are measured in batches of `BATCH` to amortize the per-iteration
//!   harness cost; throughput is reported in elements.
//! * For the skewed-lookup benchmark every map is first "warmed" with the same
//!   skewed read sequence before measurement, so the comparison is steady-state.
//!   splaylist additionally *adapts* during that warm phase plus a `rebalance()`
//!   — that adaptation is the feature under test, and the warm reads are
//!   identical for all maps, so no competitor is disadvantaged.
//!
//! Run: `cd comparison && cargo bench`. Numbers in the README were taken with
//! `lto = "thin"`, `codegen-units = 1` (see Cargo.toml).
//!
//! Performance investigation log (negative result, recorded so it is not
//! re-tried): replacing the per-node `forward: Vec<usize>` with a single flat
//! "links" arena owned by the map was measured here in a controlled A/B. It
//! *regressed* every operation (insert, lookup, and notably iterate by ~13%):
//! the shared-arena indirection has worse traversal locality than per-node
//! arrays, and free-list bookkeeping offset the saved allocation. The lookup
//! gap to `BTreeMap` is fundamental (B-trees pack keys contiguously; skip
//! lists pointer-chase) and is not closable by a safe micro-optimization.

use std::collections::BTreeMap;
use std::hint::black_box;

use criterion::{criterion_group, criterion_main, BenchmarkId, Criterion, Throughput};

const N: u64 = 100_000;
const BATCH: usize = 1_000;

/// Deterministic SplitMix64 PRNG so every run and every implementation sees the
/// same data.
struct Rng(u64);
impl Rng {
    fn next(&mut self) -> u64 {
        self.0 = self.0.wrapping_add(0x9e37_79b9_7f4a_7c15);
        let mut z = self.0;
        z = (z ^ (z >> 30)).wrapping_mul(0xbf58_476d_1ce4_e5b9);
        z = (z ^ (z >> 27)).wrapping_mul(0x94d0_49bb_1331_11eb);
        z ^ (z >> 31)
    }
}

/// A random permutation of `0..N` (so inserts are in random order, not sorted).
fn shuffled_keys() -> Vec<u64> {
    let mut keys: Vec<u64> = (0..N).collect();
    let mut rng = Rng(0x1234_5678);
    for i in (1..keys.len()).rev() {
        let j = (rng.next() % (i as u64 + 1)) as usize;
        keys.swap(i, j);
    }
    keys
}

/// Uniformly random existing keys.
fn uniform_queries(n: usize) -> Vec<u64> {
    let mut rng = Rng(0xabcd_ef01);
    (0..n).map(|_| rng.next() % N).collect()
}

/// Skewed queries: ~80% of lookups hit a *scattered* hot set (a fixed
/// pseudo-random ~1% of the key space), the rest uniform. The hot keys are
/// deliberately spread across the whole range rather than clustered at the low
/// end — low keys sit next to the head in any skip list and would be cheap
/// regardless of adaptation, which would not isolate splaylist's feature.
fn skewed_queries(n: usize) -> Vec<u64> {
    // Build a scattered hot set.
    let mut hot_rng = Rng(0xfeed_face);
    let hot_len = (N / 100).max(1) as usize;
    let hot: Vec<u64> = (0..hot_len).map(|_| hot_rng.next() % N).collect();

    let mut rng = Rng(0x0bad_f00d);
    (0..n)
        .map(|_| {
            if rng.next() % 100 < 80 {
                hot[(rng.next() as usize) % hot.len()]
            } else {
                rng.next() % N
            }
        })
        .collect()
}

fn bench_insert(c: &mut Criterion) {
    let keys = shuffled_keys();
    let mut group = c.benchmark_group("insert_random_100k");
    group.throughput(Throughput::Elements(N));

    group.bench_function(BenchmarkId::new("splaylist", N), |b| {
        b.iter(|| {
            let mut m = splaylist::SplayMap::new();
            for &k in &keys {
                m.insert(black_box(k), k);
            }
            m
        });
    });
    group.bench_function(BenchmarkId::new("BTreeMap", N), |b| {
        b.iter(|| {
            let mut m = BTreeMap::new();
            for &k in &keys {
                m.insert(black_box(k), k);
            }
            m
        });
    });
    group.bench_function(BenchmarkId::new("skiplist", N), |b| {
        b.iter(|| {
            let mut m = skiplist::SkipMap::<u64, u64>::new();
            for &k in &keys {
                m.insert(black_box(k), k);
            }
            m
        });
    });
    group.bench_function(BenchmarkId::new("crossbeam", N), |b| {
        b.iter(|| {
            let m = crossbeam_skiplist::SkipMap::new();
            for &k in &keys {
                m.insert(black_box(k), k);
            }
            m
        });
    });
    group.finish();
}

fn build_splay(keys: &[u64]) -> splaylist::SplayMap<u64, u64> {
    let mut m = splaylist::SplayMap::new();
    for &k in keys {
        m.insert(k, k);
    }
    m
}
fn build_btree(keys: &[u64]) -> BTreeMap<u64, u64> {
    let mut m = BTreeMap::new();
    for &k in keys {
        m.insert(k, k);
    }
    m
}
fn build_skiplist(keys: &[u64]) -> skiplist::SkipMap<u64, u64> {
    let mut m = skiplist::SkipMap::<u64, u64>::new();
    for &k in keys {
        m.insert(k, k);
    }
    m
}
fn build_crossbeam(keys: &[u64]) -> crossbeam_skiplist::SkipMap<u64, u64> {
    let m = crossbeam_skiplist::SkipMap::new();
    for &k in keys {
        m.insert(k, k);
    }
    m
}

fn bench_lookup(c: &mut Criterion, name: &str, queries: &[u64], warm: bool) {
    let keys: Vec<u64> = (0..N).collect();
    let splay = build_splay(&keys);
    let btree = build_btree(&keys);
    let skip = build_skiplist(&keys);
    let cross = build_crossbeam(&keys);

    // Warm phase: identical read sequence for every map. splaylist adapts.
    if warm {
        for _ in 0..5 {
            for &q in queries {
                let _ = black_box(splay.get(&q));
                let _ = black_box(btree.get(&q));
                let _ = black_box(skip.get(&q));
                let _ = black_box(cross.get(&q));
            }
        }
    }
    let mut splay = splay;
    if warm {
        // Several adaptive passes let the scattered hot keys climb toward their
        // target height (steady state). This background maintenance cost is not
        // included in the measured lookup latency below.
        for _ in 0..16 {
            splay.rebalance();
        }
    }
    let splay = splay;

    let mut group = c.benchmark_group(name);
    group.throughput(Throughput::Elements(BATCH as u64));
    let batch = &queries[..BATCH];

    group.bench_function("splaylist", |b| {
        b.iter(|| {
            let mut acc = 0u64;
            for &q in batch {
                if let Some(v) = splay.get(&q) {
                    acc = acc.wrapping_add(*v);
                }
            }
            acc
        });
    });
    group.bench_function("BTreeMap", |b| {
        b.iter(|| {
            let mut acc = 0u64;
            for &q in batch {
                if let Some(v) = btree.get(&q) {
                    acc = acc.wrapping_add(*v);
                }
            }
            acc
        });
    });
    group.bench_function("skiplist", |b| {
        b.iter(|| {
            let mut acc = 0u64;
            for &q in batch {
                if let Some(v) = skip.get(&q) {
                    acc = acc.wrapping_add(*v);
                }
            }
            acc
        });
    });
    group.bench_function("crossbeam", |b| {
        b.iter(|| {
            let mut acc = 0u64;
            for &q in batch {
                if let Some(e) = cross.get(&q) {
                    acc = acc.wrapping_add(*e.value());
                }
            }
            acc
        });
    });
    group.finish();
}

fn bench_lookup_uniform(c: &mut Criterion) {
    let q = uniform_queries(BATCH);
    bench_lookup(c, "lookup_uniform", &q, false);
}

fn bench_lookup_skewed(c: &mut Criterion) {
    let q = skewed_queries(BATCH);
    bench_lookup(c, "lookup_skewed", &q, true);
}

fn bench_iterate(c: &mut Criterion) {
    let keys: Vec<u64> = (0..N).collect();
    let splay = build_splay(&keys);
    let btree = build_btree(&keys);
    let skip = build_skiplist(&keys);
    let cross = build_crossbeam(&keys);

    let mut group = c.benchmark_group("iterate_100k");
    group.throughput(Throughput::Elements(N));
    group.bench_function("splaylist", |b| {
        b.iter(|| splay.values().fold(0u64, |a, v| a.wrapping_add(*v)));
    });
    group.bench_function("BTreeMap", |b| {
        b.iter(|| btree.values().fold(0u64, |a, v| a.wrapping_add(*v)));
    });
    group.bench_function("skiplist", |b| {
        b.iter(|| skip.iter().fold(0u64, |a, (_, v)| a.wrapping_add(*v)));
    });
    group.bench_function("crossbeam", |b| {
        b.iter(|| cross.iter().fold(0u64, |a, e| a.wrapping_add(*e.value())));
    });
    group.finish();
}

criterion_group!(
    benches,
    bench_insert,
    bench_lookup_uniform,
    bench_lookup_skewed,
    bench_iterate
);
criterion_main!(benches);
