//! Breadth tests that exercise the full public surface (entry variants, every
//! iterator form and direction, and the set wrapper) to keep coverage honest.

use splaylist::{Config, Entry, SplayMap, SplaySet};

#[test]
fn entry_all_variants() {
    let mut m: SplayMap<i32, i32> = SplayMap::new();

    // Vacant: key(), into_key via or_insert_with_key.
    match m.entry(1) {
        Entry::Vacant(e) => {
            assert_eq!(*e.key(), 1);
            assert_eq!(e.into_key(), 1);
        }
        Entry::Occupied(_) => unreachable!(),
    }
    assert!(m.is_empty());

    m.entry(1).or_insert_with_key(|k| k * 10);
    assert_eq!(m[&1], 10);

    // Occupied: key/get/get_mut/into_mut/insert/remove_entry.
    match m.entry(1) {
        Entry::Occupied(mut e) => {
            assert_eq!(*e.key(), 1);
            assert_eq!(*e.get(), 10);
            *e.get_mut() += 1;
            assert_eq!(e.insert(99), 11);
            assert_eq!(e.remove_entry(), (1, 99));
        }
        Entry::Vacant(_) => unreachable!(),
    }
    assert!(m.is_empty());

    // and_modify on a vacant entry leaves it vacant; key() works on both arms.
    let e = m.entry(7).and_modify(|v| *v += 1);
    assert_eq!(*e.key(), 7);
    e.or_insert(0);
    assert_eq!(m[&7], 0);

    *m.entry(7).or_insert_with(|| 100) += 5;
    assert_eq!(m[&7], 5);
    *m.entry(8).or_default() += 2;
    assert_eq!(m[&8], 2);

    if let Entry::Occupied(e) = m.entry(7) {
        assert_eq!(e.remove(), 5);
    }
    assert!(!m.contains_key(&7));
}

#[test]
fn every_iterator_form() {
    let mut m: SplayMap<i32, i32> = (0..10).map(|i| (i, i * i)).collect();

    // keys / values forward and reverse.
    assert_eq!(m.keys().copied().collect::<Vec<_>>(), (0..10).collect::<Vec<_>>());
    assert_eq!(
        m.keys().rev().copied().collect::<Vec<_>>(),
        (0..10).rev().collect::<Vec<_>>()
    );
    assert_eq!(m.values().copied().collect::<Vec<_>>(), (0..10).map(|i| i * i).collect::<Vec<_>>());
    assert_eq!(
        m.values().rev().next().copied(),
        Some(81)
    );

    // ExactSize.
    assert_eq!(m.iter().len(), 10);
    assert_eq!(m.keys().len(), 10);
    assert_eq!(m.values().len(), 10);

    // iter_mut forward + reverse, values_mut.
    for (k, v) in m.iter_mut() {
        *v += k;
    }
    assert_eq!(m[&3], 9 + 3);
    let last_mut = m.iter_mut().next_back().map(|(k, _)| *k);
    assert_eq!(last_mut, Some(9));
    for v in m.values_mut() {
        *v = 0;
    }
    assert!(m.values().all(|&v| v == 0));
    let vm_back = m.values_mut().rev().next();
    assert_eq!(vm_back, Some(&mut 0));

    // owning iterator with size_hint.
    let it = m.into_iter();
    assert_eq!(it.size_hint(), (10, Some(10)));
    assert_eq!(it.count(), 10);
}

#[test]
fn map_into_iter_by_ref() {
    let m: SplayMap<i32, i32> = (0..5).map(|i| (i, i)).collect();
    let mut total = 0;
    for (k, v) in &m {
        total += k + v;
    }
    assert_eq!(total, 2 * (0 + 1 + 2 + 3 + 4));
}

#[test]
fn extend_from_refs() {
    let mut m: SplayMap<i32, i32> = SplayMap::new();
    let src = [(1, 10), (2, 20)];
    m.extend(src.iter().map(|(k, v)| (k, v)));
    assert_eq!(m[&1], 10);
    assert_eq!(m[&2], 20);
}

#[test]
fn set_full_surface() {
    let mut s = SplaySet::with_config(Config::default());
    s.extend([5, 3, 1, 4, 2]);
    assert_eq!(s.len(), 5);
    assert_eq!(s.get(&3), Some(&3));
    assert_eq!(s.range(2..5).copied().collect::<Vec<_>>(), [2, 3, 4]);
    s.rebalance();
    assert!(s.contains(&4));

    // &set iteration, Debug, Eq, clone.
    let collected: Vec<_> = (&s).into_iter().copied().collect();
    assert_eq!(collected, [1, 2, 3, 4, 5]);
    assert_eq!(s.iter().rev().next(), Some(&5));
    assert_eq!(s.iter().len(), 5);
    let s2 = s.clone();
    assert_eq!(s, s2);
    assert!(format!("{s:?}").starts_with('{'));

    let from_iter: SplaySet<i32> = (0..3).collect();
    assert_eq!(from_iter.first(), Some(&0));

    let with_cap: SplaySet<i32> = SplaySet::with_capacity(16);
    assert!(with_cap.is_empty());
}

#[test]
fn map_with_capacity_and_config() {
    let m: SplayMap<i32, i32> = SplayMap::with_capacity(128);
    assert!(m.is_empty());
    assert_eq!(m.config().splay_interval, Config::default().splay_interval);

    let custom = Config {
        splay_interval: 8,
        seed: 42,
    };
    let m2: SplayMap<i32, i32> = SplayMap::with_config(custom);
    assert_eq!(m2.config(), custom);
}

#[test]
fn get_key_value_and_clears() {
    let mut m: SplayMap<String, i32> = SplayMap::new();
    m.insert("k".into(), 1);
    let (k, v) = m.get_key_value("k").unwrap();
    assert_eq!((k.as_str(), *v), ("k", 1));
    assert_eq!(m.get_key_value("missing"), None);
}

#[test]
fn set_take_and_remove_paths() {
    let mut s: SplaySet<i32> = (0..10).collect();
    assert_eq!(s.take(&5), Some(5));
    assert_eq!(s.take(&5), None);
    assert!(s.remove(&6));
    assert!(!s.remove(&6));
    assert_eq!(s.last(), Some(&9));
}
