//! Public-API integration tests for `splaylist`.

use splaylist::{Config, SplayMap, SplaySet};

#[test]
fn insert_get_remove() {
    let mut m = SplayMap::new();
    assert_eq!(m.insert(2, "two"), None);
    assert_eq!(m.insert(1, "one"), None);
    assert_eq!(m.insert(2, "TWO"), Some("two"));
    assert_eq!(m.get(&1), Some(&"one"));
    assert_eq!(m.get(&2), Some(&"TWO"));
    assert_eq!(m.get(&3), None);
    assert_eq!(m.len(), 2);
    assert_eq!(m.remove(&1), Some("one"));
    assert_eq!(m.remove(&1), None);
    assert_eq!(m.len(), 1);
}

#[test]
fn ordering_is_ascending_regardless_of_insertion_order() {
    let mut m = SplayMap::new();
    for k in [5, 3, 9, 1, 7, 2, 8, 4, 6, 0] {
        m.insert(k, k * 10);
    }
    let keys: Vec<_> = m.keys().copied().collect();
    assert_eq!(keys, (0..=9).collect::<Vec<_>>());
    let rev: Vec<_> = m.keys().rev().copied().collect();
    assert_eq!(rev, (0..=9).rev().collect::<Vec<_>>());
    assert_eq!(m.first_key_value(), Some((&0, &0)));
    assert_eq!(m.last_key_value(), Some((&9, &90)));
}

#[test]
fn borrowed_key_lookup() {
    let mut m: SplayMap<String, i32> = SplayMap::new();
    m.insert("alpha".to_string(), 1);
    // Look up with &str, not &String.
    assert_eq!(m.get("alpha"), Some(&1));
    assert!(m.contains_key("alpha"));
    assert_eq!(m.remove("alpha"), Some(1));
}

#[test]
fn get_mut_mutates() {
    let mut m = SplayMap::new();
    m.insert(1, 10);
    *m.get_mut(&1).unwrap() += 5;
    assert_eq!(m[&1], 15);
    for (_, v) in &mut m {
        *v *= 2;
    }
    assert_eq!(m[&1], 30);
}

#[test]
fn entry_api() {
    let mut m: SplayMap<&str, i32> = SplayMap::new();
    *m.entry("a").or_insert(0) += 1;
    *m.entry("a").or_insert(0) += 1;
    m.entry("b").or_insert_with(|| 41);
    m.entry("b").and_modify(|v| *v += 1).or_insert(0);
    m.entry("c").or_default();
    assert_eq!(m.get("a"), Some(&2));
    assert_eq!(m.get("b"), Some(&42));
    assert_eq!(m.get("c"), Some(&0));

    // Occupied removal through the entry API.
    if let splaylist::Entry::Occupied(e) = m.entry("c") {
        assert_eq!(e.remove(), 0);
    }
    assert!(!m.contains_key("c"));
}

#[test]
fn range_queries() {
    let m: SplayMap<i32, i32> = (0..20).map(|i| (i, i)).collect();
    let got: Vec<_> = m.range(5..10).map(|(k, _)| *k).collect();
    assert_eq!(got, [5, 6, 7, 8, 9]);
    let inc: Vec<_> = m.range(5..=7).map(|(k, _)| *k).collect();
    assert_eq!(inc, [5, 6, 7]);
    let tail: Vec<_> = m.range(18..).map(|(k, _)| *k).collect();
    assert_eq!(tail, [18, 19]);
    let head: Vec<_> = m.range(..3).map(|(k, _)| *k).collect();
    assert_eq!(head, [0, 1, 2]);
}

#[test]
#[should_panic(expected = "range start is greater than range end")]
fn range_inverted_panics() {
    let m: SplayMap<i32, i32> = (0..10).map(|i| (i, i)).collect();
    let (lo, hi) = (8, 2);
    let _ = m.range(lo..hi).count();
}

#[test]
fn clone_and_eq() {
    let a: SplayMap<i32, i32> = (0..50).map(|i| (i, i * i)).collect();
    let b = a.clone();
    assert_eq!(a, b);
    let mut c = a.clone();
    c.insert(999, 0);
    assert_ne!(a, c);
}

#[test]
fn clear_then_reuse() {
    let mut m: SplayMap<i32, i32> = (0..100).map(|i| (i, i)).collect();
    m.clear();
    assert!(m.is_empty());
    assert_eq!(m.get(&5), None);
    m.insert(7, 7);
    assert_eq!(m.get(&7), Some(&7));
    assert_eq!(m.len(), 1);
}

#[test]
fn set_basics() {
    let mut s: SplaySet<i32> = SplaySet::new();
    assert!(s.insert(3));
    assert!(s.insert(1));
    assert!(!s.insert(1));
    assert!(s.contains(&1));
    assert_eq!(s.first(), Some(&1));
    assert_eq!(s.last(), Some(&3));
    assert_eq!(s.take(&3), Some(3));
    let v: Vec<_> = s.into_iter().collect();
    assert_eq!(v, [1]);
}

#[test]
fn empty_map_behaviour() {
    let mut m: SplayMap<i32, i32> = SplayMap::new();
    assert!(m.is_empty());
    assert_eq!(m.get(&0), None);
    assert_eq!(m.remove(&0), None);
    assert_eq!(m.first_key_value(), None);
    assert_eq!(m.iter().next(), None);
    assert_eq!(m.iter().next_back(), None);
    assert_eq!(m.range(..).count(), 0);
}

#[test]
fn debug_format() {
    let mut m = SplayMap::new();
    m.insert(1, "a");
    m.insert(2, "b");
    assert_eq!(format!("{m:?}"), r#"{1: "a", 2: "b"}"#);
}

#[test]
fn stress_insert_remove_stays_consistent() {
    // Aggressive adaptation across a churny workload; the structure must
    // remain a valid sorted map throughout.
    let mut m = SplayMap::with_config(Config {
        splay_interval: 1,
        seed: 7,
    });
    let mut state = 0x1234_5678_u64;
    let mut next = || {
        state ^= state << 13;
        state ^= state >> 7;
        state ^= state << 17;
        state
    };
    let mut reference = std::collections::BTreeMap::new();
    for _ in 0..20_000 {
        let k = (next() % 500) as u32;
        if next() & 1 == 0 {
            m.insert(k, k);
            reference.insert(k, k);
        } else {
            assert_eq!(m.remove(&k), reference.remove(&k));
        }
    }
    assert!(m.iter().eq(reference.iter()));
}

#[cfg(feature = "serde")]
#[test]
fn serde_round_trip() {
    let m: SplayMap<i32, String> = (0..25).map(|i| (i, format!("v{i}"))).collect();
    let json = serde_json::to_string(&m).unwrap();
    let back: SplayMap<i32, String> = serde_json::from_str(&json).unwrap();
    assert_eq!(m, back);

    let s: SplaySet<i32> = (0..25).collect();
    let json = serde_json::to_string(&s).unwrap();
    let back: SplaySet<i32> = serde_json::from_str(&json).unwrap();
    assert_eq!(s, back);
}

#[cfg(feature = "dot")]
#[test]
fn dot_output_is_wellformed() {
    let mut m = SplayMap::new();
    for i in 0..5 {
        m.insert(i, i);
    }
    let dot = m.to_dot();
    assert!(dot.starts_with("digraph splaylist {"));
    assert!(dot.trim_end().ends_with('}'));
    assert!(dot.contains("head"));
}
