//! The [`SplayMap`] ordered map and its configuration.

use core::borrow::Borrow;
use core::cell::Cell;
use core::cmp::Ordering;
use core::fmt;
use core::mem;
use core::ops::{Bound, Index, RangeBounds};

use alloc::vec::Vec;

use crate::entry::Entry;
use crate::iter::{IntoIter, Iter, IterMut, Keys, Range, Values, ValuesMut};

/// Maximum tower height. A node is reached at level `l` by roughly `2^-l` of
/// searches, so 32 levels comfortably index `2^32` keys.
pub(crate) const MAX_LEVEL: usize = 32;

/// Sentinel index meaning "no node" (end of a level chain, or the head as a
/// predecessor). Real arena indices are always `< slots.len() <= isize::MAX`.
pub(crate) const NIL: usize = usize::MAX;

/// Tuning knobs for a [`SplayMap`].
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct Config {
    /// Number of mutating operations between adaptive rebalance passes.
    ///
    /// Lower values adapt faster at higher cost; higher values adapt more
    /// lazily. `0` disables adaptation entirely, leaving a plain randomized
    /// skip list.
    pub splay_interval: u64,
    /// Seed for the internal PRNG that chooses tower heights. Fixing the seed
    /// makes a map's structural evolution deterministic, which is convenient
    /// for tests and reproducible benchmarks.
    pub seed: u64,
}

impl Default for Config {
    fn default() -> Self {
        Self {
            splay_interval: 64,
            seed: 0x2545_f491_4f6c_dd1d,
        }
    }
}

#[derive(Clone)]
pub(crate) struct Node<K, V> {
    pub(crate) key: K,
    pub(crate) value: V,
    /// `forward[l]` is the next node at level `l`, or [`NIL`]. Length is the
    /// node's tower height (`>= 1`).
    pub(crate) forward: Vec<usize>,
    /// Level-0 predecessor (or [`NIL`] if this is the first node). Maintains a
    /// doubly-linked base list so iteration is bidirectional.
    pub(crate) prev: usize,
    /// Access counter driving adaptation. Interior-mutable so [`SplayMap::get`]
    /// can record a hit through `&self`.
    pub(crate) hits: Cell<u64>,
}

#[derive(Clone)]
pub(crate) enum Slot<K, V> {
    Occupied(Node<K, V>),
    /// A recycled slot; stores the index of the next free slot, or [`NIL`].
    Vacant(usize),
}

/// An ordered map backed by a self-balancing (splay) skip list.
///
/// Keys are kept in ascending order. Search, insertion, and removal are
/// `O(log n)` on average; under a skewed access pattern, hot keys are promoted
/// to taller towers so their lookups touch fewer nodes.
///
/// See the [crate-level documentation](crate) for when to prefer this over a
/// [`BTreeMap`](std::collections::BTreeMap) or a concurrent skip list.
///
/// # Examples
///
/// ```
/// use splaylist::SplayMap;
///
/// let mut map = SplayMap::new();
/// map.insert("b", 2);
/// map.insert("a", 1);
/// assert_eq!(map.get("a"), Some(&1));
/// assert_eq!(map.remove("b"), Some(2));
/// assert!(map.get("b").is_none());
/// ```
pub struct SplayMap<K, V> {
    slots: Vec<Slot<K, V>>,
    free: usize,
    head: [usize; MAX_LEVEL],
    level: usize,
    last: usize,
    len: usize,
    total_hits: Cell<u64>,
    rng: Cell<u64>,
    ops: u64,
    config: Config,
}

impl<K, V> SplayMap<K, V> {
    /// Creates an empty map with the [default configuration](Config::default).
    #[must_use]
    pub fn new() -> Self {
        Self::with_config(Config::default())
    }

    /// Creates an empty map with the given [`Config`].
    #[must_use]
    pub fn with_config(config: Config) -> Self {
        Self {
            slots: Vec::new(),
            free: NIL,
            head: [NIL; MAX_LEVEL],
            level: 1,
            last: NIL,
            len: 0,
            total_hits: Cell::new(0),
            rng: Cell::new(config.seed | 1),
            ops: 0,
            config,
        }
    }

    /// Creates an empty map that can hold at least `capacity` entries without
    /// reallocating its node arena.
    #[must_use]
    pub fn with_capacity(capacity: usize) -> Self {
        let mut map = Self::new();
        map.slots.reserve(capacity);
        map
    }

    /// Returns the number of entries.
    #[must_use]
    pub fn len(&self) -> usize {
        self.len
    }

    /// Returns `true` if the map contains no entries.
    #[must_use]
    pub fn is_empty(&self) -> bool {
        self.len == 0
    }

    /// Returns the active [`Config`].
    #[must_use]
    pub fn config(&self) -> Config {
        self.config
    }

    /// Removes all entries, keeping allocated capacity for reuse.
    pub fn clear(&mut self) {
        self.slots.clear();
        self.free = NIL;
        self.head = [NIL; MAX_LEVEL];
        self.level = 1;
        self.last = NIL;
        self.len = 0;
        self.total_hits.set(0);
        self.ops = 0;
    }

    /// Returns an iterator over `(&K, &V)` in ascending key order.
    #[must_use]
    pub fn iter(&self) -> Iter<'_, K, V> {
        Iter::new(self, self.head[0], self.last, self.len)
    }

    /// Returns an iterator over `&K` in ascending order.
    #[must_use]
    pub fn keys(&self) -> Keys<'_, K, V> {
        Keys::new(self.iter())
    }

    /// Returns an iterator over `&V` in ascending key order.
    #[must_use]
    pub fn values(&self) -> Values<'_, K, V> {
        Values::new(self.iter())
    }

    pub(crate) fn node(&self, idx: usize) -> &Node<K, V> {
        match &self.slots[idx] {
            Slot::Occupied(n) => n,
            Slot::Vacant(_) => unreachable!("index {idx} points at a vacant slot"),
        }
    }

    fn node_mut(&mut self, idx: usize) -> &mut Node<K, V> {
        match &mut self.slots[idx] {
            Slot::Occupied(n) => n,
            Slot::Vacant(_) => unreachable!("index {idx} points at a vacant slot"),
        }
    }

    /// The next node at level `lvl` starting from predecessor `pred`
    /// ([`NIL`] = head).
    fn forward_from(&self, pred: usize, lvl: usize) -> usize {
        if pred == NIL {
            self.head[lvl]
        } else {
            let fwd = &self.node(pred).forward;
            if lvl < fwd.len() {
                fwd[lvl]
            } else {
                NIL
            }
        }
    }

    /// Repoints predecessor `pred` ([`NIL`] = head) at level `lvl` to `val`.
    fn set_forward(&mut self, pred: usize, lvl: usize, val: usize) {
        if pred == NIL {
            self.head[lvl] = val;
        } else {
            self.node_mut(pred).forward[lvl] = val;
        }
    }

    fn next_rand(&self) -> u64 {
        // xorshift64*; deterministic from the configured seed.
        let mut x = self.rng.get();
        x ^= x >> 12;
        x ^= x << 25;
        x ^= x >> 27;
        self.rng.set(x);
        x.wrapping_mul(0x2545_f491_4f6c_dd1d)
    }

    fn random_level(&self) -> usize {
        let mut lvl = 1;
        while lvl < MAX_LEVEL && lvl <= self.level && (self.next_rand() & 1) == 1 {
            lvl += 1;
        }
        lvl
    }

    fn alloc_node(&mut self, key: K, value: V, levels: usize) -> usize {
        let node = Node {
            key,
            value,
            forward: alloc::vec![NIL; levels],
            prev: NIL,
            hits: Cell::new(1),
        };
        if self.free == NIL {
            self.slots.push(Slot::Occupied(node));
            self.slots.len() - 1
        } else {
            let idx = self.free;
            let next_free = match self.slots[idx] {
                Slot::Vacant(n) => n,
                Slot::Occupied(_) => unreachable!("free list points at an occupied slot"),
            };
            self.free = next_free;
            self.slots[idx] = Slot::Occupied(node);
            idx
        }
    }

    fn free_node(&mut self, idx: usize) -> (K, V) {
        let prev_free = self.free;
        self.free = idx;
        match mem::replace(&mut self.slots[idx], Slot::Vacant(prev_free)) {
            Slot::Occupied(n) => (n.key, n.value),
            Slot::Vacant(_) => unreachable!("double free of slot {idx}"),
        }
    }
}

impl<K: Ord, V> SplayMap<K, V> {
    /// Locates the search path for `q`: `update[l]` is the last node before
    /// `q` at level `l` ([`NIL`] = head). Returns the level-0 candidate and
    /// whether it equals `q`.
    fn find_update<Q>(&self, q: &Q) -> ([usize; MAX_LEVEL], usize, bool)
    where
        K: Borrow<Q>,
        Q: Ord + ?Sized,
    {
        let mut update = [NIL; MAX_LEVEL];
        let mut pred = NIL;
        for lvl in (0..self.level).rev() {
            loop {
                let nxt = self.forward_from(pred, lvl);
                if nxt != NIL && self.node(nxt).key.borrow().cmp(q) == Ordering::Less {
                    pred = nxt;
                } else {
                    break;
                }
            }
            update[lvl] = pred;
        }
        let cand = self.forward_from(update[0], 0);
        let found = cand != NIL && self.node(cand).key.borrow() == q;
        (update, cand, found)
    }

    /// Read-only search: returns the slot index of `q`, or [`NIL`].
    fn find_node<Q>(&self, q: &Q) -> usize
    where
        K: Borrow<Q>,
        Q: Ord + ?Sized,
    {
        let mut pred = NIL;
        for lvl in (0..self.level).rev() {
            loop {
                let nxt = self.forward_from(pred, lvl);
                if nxt != NIL && self.node(nxt).key.borrow().cmp(q) == Ordering::Less {
                    pred = nxt;
                } else {
                    break;
                }
            }
        }
        let cand = self.forward_from(pred, 0);
        if cand != NIL && self.node(cand).key.borrow() == q {
            cand
        } else {
            NIL
        }
    }

    /// Inserts a key/value pair, returning the previous value for `key` if one
    /// was present.
    ///
    /// # Examples
    ///
    /// ```
    /// use splaylist::SplayMap;
    /// let mut map = SplayMap::new();
    /// assert_eq!(map.insert(1, "a"), None);
    /// assert_eq!(map.insert(1, "b"), Some("a"));
    /// ```
    pub fn insert(&mut self, key: K, value: V) -> Option<V> {
        self.insert_inner(key, value).1
    }

    /// Inserts and returns `(slot index, previous value)`.
    fn insert_inner(&mut self, key: K, value: V) -> (usize, Option<V>) {
        let (mut update, cand, found) = self.find_update(&key);
        if found {
            let slot = self.node_mut(cand);
            return (cand, Some(mem::replace(&mut slot.value, value)));
        }

        let levels = self.random_level();
        if levels > self.level {
            for entry in update.iter_mut().take(levels).skip(self.level) {
                *entry = NIL;
            }
            self.level = levels;
        }

        let idx = self.alloc_node(key, value, levels);
        let next0 = self.forward_from(update[0], 0);
        for (lvl, pred) in update.iter().copied().enumerate().take(levels) {
            let nxt = self.forward_from(pred, lvl);
            self.node_mut(idx).forward[lvl] = nxt;
            self.set_forward(pred, lvl, idx);
        }

        // Maintain the doubly-linked base list.
        self.node_mut(idx).prev = update[0];
        if next0 == NIL {
            self.last = idx;
        } else {
            self.node_mut(next0).prev = idx;
        }

        self.len += 1;
        self.total_hits.set(self.total_hits.get() + 1);
        self.maybe_adapt(idx);
        (idx, None)
    }

    /// Returns a reference to the value for `key`, recording an access that
    /// informs later adaptation.
    ///
    /// The key may be any borrowed form of the map's key type, as long as the
    /// ordering on the borrowed form matches that of the key type.
    pub fn get<Q>(&self, key: &Q) -> Option<&V>
    where
        K: Borrow<Q>,
        Q: Ord + ?Sized,
    {
        let idx = self.find_node(key);
        if idx == NIL {
            return None;
        }
        self.touch(idx);
        Some(&self.node(idx).value)
    }

    /// Returns references to the key and value for `key`.
    pub fn get_key_value<Q>(&self, key: &Q) -> Option<(&K, &V)>
    where
        K: Borrow<Q>,
        Q: Ord + ?Sized,
    {
        let idx = self.find_node(key);
        if idx == NIL {
            return None;
        }
        self.touch(idx);
        let n = self.node(idx);
        Some((&n.key, &n.value))
    }

    /// Returns `true` if the map contains `key`. Does not record an access.
    pub fn contains_key<Q>(&self, key: &Q) -> bool
    where
        K: Borrow<Q>,
        Q: Ord + ?Sized,
    {
        self.find_node(key) != NIL
    }

    /// Returns a mutable reference to the value for `key`, recording an access
    /// and possibly triggering an adaptive rebalance pass.
    pub fn get_mut<Q>(&mut self, key: &Q) -> Option<&mut V>
    where
        K: Borrow<Q>,
        Q: Ord + ?Sized,
    {
        let idx = self.find_node(key);
        if idx == NIL {
            return None;
        }
        self.touch(idx);
        self.maybe_adapt(idx);
        Some(&mut self.node_mut(idx).value)
    }

    /// Removes `key`, returning its value if it was present.
    pub fn remove<Q>(&mut self, key: &Q) -> Option<V>
    where
        K: Borrow<Q>,
        Q: Ord + ?Sized,
    {
        self.remove_entry(key).map(|(_, v)| v)
    }

    /// Removes `key`, returning the stored key and value if present.
    pub fn remove_entry<Q>(&mut self, key: &Q) -> Option<(K, V)>
    where
        K: Borrow<Q>,
        Q: Ord + ?Sized,
    {
        let (update, idx, found) = self.find_update(key);
        if !found {
            return None;
        }
        Some(self.unlink(&update, idx))
    }

    /// Unlinks node `idx` given its search path `update`, recycles its slot,
    /// and returns the stored key/value.
    fn unlink(&mut self, update: &[usize; MAX_LEVEL], idx: usize) -> (K, V) {
        let node_levels = self.node(idx).forward.len();
        let next0 = self.node(idx).forward[0];
        for (lvl, pred) in update.iter().copied().enumerate().take(self.level) {
            if lvl < node_levels && self.forward_from(pred, lvl) == idx {
                let nxt = self.node(idx).forward[lvl];
                self.set_forward(pred, lvl, nxt);
            }
        }

        // Repair the doubly-linked base list.
        if next0 == NIL {
            self.last = update[0];
        } else {
            self.node_mut(next0).prev = update[0];
        }

        while self.level > 1 && self.head[self.level - 1] == NIL {
            self.level -= 1;
        }

        let node_hits = self.node(idx).hits.get();
        self.total_hits
            .set(self.total_hits.get().saturating_sub(node_hits));
        let kv = self.free_node(idx);
        self.len -= 1;
        kv
    }

    /// Removes the node at slot `idx` (used by the entry API, where the index
    /// is already known). Keys are unique, so locating by key finds exactly
    /// this node.
    pub(crate) fn remove_at(&mut self, idx: usize) -> (K, V) {
        let (update, cand, _found) = self.find_update(self.node(idx).key.borrow());
        debug_assert_eq!(cand, idx, "remove_at located a different node");
        self.unlink(&update, idx)
    }

    /// Inserts `key`/`value` and returns the slot index, replacing any existing
    /// value. Used by the entry API.
    pub(crate) fn insert_and_index(&mut self, key: K, value: V) -> usize {
        self.insert_inner(key, value).0
    }

    /// Read-only search returning a slot index, used by the entry API.
    pub(crate) fn find_index<Q>(&self, key: &Q) -> Option<usize>
    where
        K: Borrow<Q>,
        Q: Ord + ?Sized,
    {
        let idx = self.find_node(key);
        if idx == NIL {
            None
        } else {
            Some(idx)
        }
    }

    /// Returns the first (smallest) key and its value.
    #[must_use]
    pub fn first_key_value(&self) -> Option<(&K, &V)> {
        let idx = self.head[0];
        if idx == NIL {
            None
        } else {
            let n = self.node(idx);
            Some((&n.key, &n.value))
        }
    }

    /// Returns the last (largest) key and its value.
    #[must_use]
    pub fn last_key_value(&self) -> Option<(&K, &V)> {
        if self.last == NIL {
            None
        } else {
            let n = self.node(self.last);
            Some((&n.key, &n.value))
        }
    }

    /// Gets the [`Entry`] for `key` for in-place insertion or mutation.
    ///
    /// # Examples
    ///
    /// ```
    /// use splaylist::SplayMap;
    /// let mut map: SplayMap<&str, i32> = SplayMap::new();
    /// *map.entry("a").or_insert(0) += 1;
    /// *map.entry("a").or_insert(0) += 1;
    /// assert_eq!(map.get("a"), Some(&2));
    /// ```
    pub fn entry(&mut self, key: K) -> Entry<'_, K, V> {
        Entry::new(self, key)
    }

    /// Returns an iterator over `(&K, &mut V)` in ascending key order.
    pub fn iter_mut(&mut self) -> IterMut<'_, K, V> {
        IterMut::new(self.collect_mut_sorted())
    }

    /// Returns an iterator over `&mut V` in ascending key order.
    pub fn values_mut(&mut self) -> ValuesMut<'_, K, V> {
        ValuesMut::new(self.collect_mut_sorted())
    }

    /// Returns an iterator over the entries whose keys fall in `range`, in
    /// ascending order.
    ///
    /// # Panics
    ///
    /// Panics if the start bound is greater than the end bound, matching
    /// [`BTreeMap::range`](std::collections::BTreeMap::range).
    ///
    /// # Examples
    ///
    /// ```
    /// use splaylist::SplayMap;
    /// let map: SplayMap<i32, i32> = (0..10).map(|i| (i, i * i)).collect();
    /// let got: Vec<_> = map.range(3..6).map(|(k, _)| *k).collect();
    /// assert_eq!(got, [3, 4, 5]);
    /// ```
    pub fn range<Q, R>(&self, range: R) -> Range<'_, K, V>
    where
        K: Borrow<Q>,
        Q: Ord + ?Sized,
        R: RangeBounds<Q>,
    {
        let start_bound = range.start_bound();
        let end_bound = range.end_bound();
        if let (Bound::Included(s) | Bound::Excluded(s), Bound::Included(e) | Bound::Excluded(e)) =
            (start_bound, end_bound)
        {
            assert!(
                s.cmp(e) != Ordering::Greater,
                "range start is greater than range end"
            );
        }
        let start = self.lower_bound_index(start_bound);
        let stop = self.upper_bound_index(end_bound);
        Range::new(self, start, stop)
    }

    /// Index of the first node whose key satisfies the lower `bound`.
    fn lower_bound_index<Q>(&self, bound: Bound<&Q>) -> usize
    where
        K: Borrow<Q>,
        Q: Ord + ?Sized,
    {
        let mut pred = NIL;
        for lvl in (0..self.level).rev() {
            loop {
                let nxt = self.forward_from(pred, lvl);
                if nxt == NIL {
                    break;
                }
                let k = self.node(nxt).key.borrow();
                let before = match bound {
                    Bound::Unbounded => false,
                    Bound::Included(b) => k.cmp(b) == Ordering::Less,
                    Bound::Excluded(b) => k.cmp(b) != Ordering::Greater,
                };
                if before {
                    pred = nxt;
                } else {
                    break;
                }
            }
        }
        self.forward_from(pred, 0)
    }

    /// Index of the first node that lies past the upper `bound` (the stop
    /// sentinel for [`Range`]), or [`NIL`].
    fn upper_bound_index<Q>(&self, bound: Bound<&Q>) -> usize
    where
        K: Borrow<Q>,
        Q: Ord + ?Sized,
    {
        let mut pred = NIL;
        for lvl in (0..self.level).rev() {
            loop {
                let nxt = self.forward_from(pred, lvl);
                if nxt == NIL {
                    break;
                }
                let k = self.node(nxt).key.borrow();
                let still_in = match bound {
                    Bound::Unbounded => true,
                    Bound::Included(b) => k.cmp(b) != Ordering::Greater,
                    Bound::Excluded(b) => k.cmp(b) == Ordering::Less,
                };
                if still_in {
                    pred = nxt;
                } else {
                    break;
                }
            }
        }
        self.forward_from(pred, 0)
    }

    /// Forces a full adaptive rebalance pass over every node, reshaping the
    /// towers to match the access pattern recorded so far.
    pub fn rebalance(&mut self) {
        if self.is_empty() {
            return;
        }
        let mut idx = self.head[0];
        let mut order = Vec::with_capacity(self.len);
        while idx != NIL {
            order.push(idx);
            idx = self.node(idx).forward[0];
        }
        for idx in order {
            self.adapt_node(idx);
        }
    }

    /// Records an access against node `idx`.
    fn touch(&self, idx: usize) {
        let n = self.node(idx);
        n.hits.set(n.hits.get() + 1);
        self.total_hits.set(self.total_hits.get() + 1);
    }

    /// Advances the operation clock and, on the configured interval, adapts the
    /// node just touched.
    fn maybe_adapt(&mut self, idx: usize) {
        if self.config.splay_interval == 0 {
            return;
        }
        self.ops += 1;
        if self.ops % self.config.splay_interval == 0 {
            self.adapt_node(idx);
        }
    }

    /// Desired tower height (number of levels, in `1..=self.level`) for a node
    /// with `hits` accesses, following the splay-list target
    /// `height ≈ K - 1 - log2(total / hits)`.
    fn desired_levels(&self, hits: u64) -> usize {
        let total = self.total_hits.get();
        if total == 0 || hits == 0 {
            return 1;
        }
        let ratio = (total / hits).max(1);
        let log2 = ratio.ilog2() as usize;
        let height = (self.level - 1).saturating_sub(log2);
        height + 1
    }

    /// Promotes or demotes node `idx` by at most one level toward its desired
    /// height.
    fn adapt_node(&mut self, idx: usize) {
        let cur = self.node(idx).forward.len();
        let desired = self.desired_levels(self.node(idx).hits.get());
        if desired > cur {
            self.promote(idx, cur);
        } else if desired < cur && cur > 1 {
            self.demote(idx, cur);
        }
    }

    /// Adds level `new_lvl` (`= cur` levels) to node `idx`.
    fn promote(&mut self, idx: usize, new_lvl: usize) {
        if new_lvl >= self.level {
            return;
        }
        let key_ptr = idx;
        let (update, _, _) = self.find_update(self.node(key_ptr).key.borrow());
        let pred = update[new_lvl];
        let nxt = self.forward_from(pred, new_lvl);
        self.node_mut(idx).forward.push(nxt);
        self.set_forward(pred, new_lvl, idx);
    }

    /// Removes the top level (`top = cur - 1`) from node `idx`.
    fn demote(&mut self, idx: usize, cur: usize) {
        let top = cur - 1;
        let (update, _, _) = self.find_update(self.node(idx).key.borrow());
        let pred = update[top];
        if self.forward_from(pred, top) == idx {
            let nxt = self.node(idx).forward[top];
            self.set_forward(pred, top, nxt);
        }
        self.node_mut(idx).forward.pop();
        while self.level > 1 && self.head[self.level - 1] == NIL {
            self.level -= 1;
        }
    }

    /// Collects `(&K, &mut V)` for every entry, sorted by key. Used to build
    /// the mutable iterators safely without aliasing.
    fn collect_mut_sorted(&mut self) -> Vec<(&K, &mut V)> {
        let mut pairs: Vec<(&K, &mut V)> = self
            .slots
            .iter_mut()
            .filter_map(|slot| match slot {
                Slot::Occupied(n) => Some((&n.key, &mut n.value)),
                Slot::Vacant(_) => None,
            })
            .collect();
        pairs.sort_by(|a, b| a.0.cmp(b.0));
        pairs
    }
}

impl<K, V> Default for SplayMap<K, V> {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(feature = "dot")]
#[cfg_attr(docsrs, doc(cfg(feature = "dot")))]
impl<K: Ord + fmt::Display, V: fmt::Display> SplayMap<K, V> {
    /// Renders the level structure as a `GraphViz` DOT digraph.
    ///
    /// Each node shows its key, value, tower height, and access count; one
    /// edge set is drawn per level, so the diagram makes the adaptive towers
    /// visible. Render with, e.g., `dot -Tpng out.dot -o out.png`.
    ///
    /// Requires the `dot` feature.
    #[must_use]
    pub fn to_dot(&self) -> alloc::string::String {
        use core::fmt::Write as _;
        let mut s = alloc::string::String::new();
        let _ = writeln!(s, "digraph splaylist {{");
        let _ = writeln!(s, "  rankdir=LR;");
        let _ = writeln!(s, "  node [shape=record];");
        let _ = writeln!(s, "  head [label=\"head|levels={}\"];", self.level);

        let mut idx = self.head[0];
        while idx != NIL {
            let n = self.node(idx);
            let _ = writeln!(
                s,
                "  n{idx} [label=\"{}={}|h={}|hits={}\"];",
                n.key,
                n.value,
                n.forward.len(),
                n.hits.get()
            );
            idx = n.forward[0];
        }

        for lvl in 0..self.level {
            let mut prev = NIL;
            let mut cur = self.head[lvl];
            while cur != NIL {
                if prev == NIL {
                    let _ = writeln!(s, "  head -> n{cur} [label=\"L{lvl}\"];");
                } else {
                    let _ = writeln!(s, "  n{prev} -> n{cur} [label=\"L{lvl}\"];");
                }
                prev = cur;
                cur = self.node(cur).forward[lvl];
            }
        }

        let _ = writeln!(s, "}}");
        s
    }
}

impl<K: Clone, V: Clone> Clone for SplayMap<K, V> {
    fn clone(&self) -> Self {
        Self {
            slots: self.slots.clone(),
            free: self.free,
            head: self.head,
            level: self.level,
            last: self.last,
            len: self.len,
            total_hits: self.total_hits.clone(),
            rng: self.rng.clone(),
            ops: self.ops,
            config: self.config,
        }
    }
}

impl<K: fmt::Debug, V: fmt::Debug> fmt::Debug for SplayMap<K, V> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.debug_map().entries(self.iter()).finish()
    }
}

impl<K: PartialEq, V: PartialEq> PartialEq for SplayMap<K, V> {
    fn eq(&self, other: &Self) -> bool {
        self.len == other.len && self.iter().eq(other.iter())
    }
}

impl<K: Eq, V: Eq> Eq for SplayMap<K, V> {}

impl<K: Ord, V> FromIterator<(K, V)> for SplayMap<K, V> {
    fn from_iter<I: IntoIterator<Item = (K, V)>>(iter: I) -> Self {
        let mut map = Self::new();
        map.extend(iter);
        map
    }
}

impl<K: Ord, V> Extend<(K, V)> for SplayMap<K, V> {
    fn extend<I: IntoIterator<Item = (K, V)>>(&mut self, iter: I) {
        for (k, v) in iter {
            self.insert(k, v);
        }
    }
}

impl<'a, K: Ord + Copy, V: Copy> Extend<(&'a K, &'a V)> for SplayMap<K, V> {
    fn extend<I: IntoIterator<Item = (&'a K, &'a V)>>(&mut self, iter: I) {
        for (&k, &v) in iter {
            self.insert(k, v);
        }
    }
}

impl<K: Ord, Q, V> Index<&Q> for SplayMap<K, V>
where
    K: Borrow<Q>,
    Q: Ord + ?Sized,
{
    type Output = V;

    fn index(&self, key: &Q) -> &V {
        self.get(key).expect("no entry found for key")
    }
}

impl<'a, K, V> IntoIterator for &'a SplayMap<K, V> {
    type Item = (&'a K, &'a V);
    type IntoIter = Iter<'a, K, V>;
    fn into_iter(self) -> Self::IntoIter {
        self.iter()
    }
}

impl<'a, K: Ord, V> IntoIterator for &'a mut SplayMap<K, V> {
    type Item = (&'a K, &'a mut V);
    type IntoIter = IterMut<'a, K, V>;
    fn into_iter(self) -> Self::IntoIter {
        self.iter_mut()
    }
}

impl<K, V> IntoIterator for SplayMap<K, V> {
    type Item = (K, V);
    type IntoIter = IntoIter<K, V>;
    fn into_iter(self) -> Self::IntoIter {
        let head = self.head[0];
        let len = self.len;
        IntoIter::new(self.slots, head, len)
    }
}

// Internal accessors for sibling modules.
impl<K, V> SplayMap<K, V> {
    pub(crate) fn node_forward0(&self, idx: usize) -> usize {
        self.node(idx).forward[0]
    }

    pub(crate) fn node_prev(&self, idx: usize) -> usize {
        self.node(idx).prev
    }

    pub(crate) fn node_kv(&self, idx: usize) -> (&K, &V) {
        let n = self.node(idx);
        (&n.key, &n.value)
    }

    pub(crate) fn node_key(&self, idx: usize) -> &K {
        &self.node(idx).key
    }

    pub(crate) fn node_value(&self, idx: usize) -> &V {
        &self.node(idx).value
    }

    pub(crate) fn node_value_mut(&mut self, idx: usize) -> &mut V {
        &mut self.node_mut(idx).value
    }
}

// `Slot` accessor used by `IntoIter`.
impl<K, V> Slot<K, V> {
    pub(crate) fn take_occupied(self) -> Option<Node<K, V>> {
        match self {
            Slot::Occupied(n) => Some(n),
            Slot::Vacant(_) => None,
        }
    }
}

#[cfg(test)]
mod tests {
    use super::{Config, SplayMap, NIL};

    impl<K: Ord, V> SplayMap<K, V> {
        /// Tower height (number of levels) for `key`, or 0 if absent. Test-only.
        fn height_of(&self, key: &K) -> usize {
            let idx = self.find_node(key);
            if idx == NIL {
                0
            } else {
                self.node(idx).forward.len()
            }
        }

        /// Checks the structural invariants and panics on any violation.
        /// Test-only; exercised by the unit tests below.
        fn check_invariants(&self) {
            // Level 0 is the complete, sorted, doubly-linked base list.
            let mut count = 0usize;
            let mut prev = NIL;
            let mut cur = self.head[0];
            while cur != NIL {
                assert_eq!(self.node(cur).prev, prev, "prev pointer broken");
                if prev != NIL {
                    assert!(
                        self.node(prev).key < self.node(cur).key,
                        "level 0 not strictly ascending"
                    );
                }
                prev = cur;
                cur = self.node(cur).forward[0];
                count += 1;
            }
            assert_eq!(count, self.len, "len disagrees with level-0 chain");
            assert_eq!(self.last, prev, "last pointer wrong");

            // Every upper level is a sorted subsequence of level 0.
            for lvl in 1..self.level {
                let mut prev_key: Option<&K> = None;
                let mut c = self.head[lvl];
                while c != NIL {
                    let n = self.node(c);
                    assert!(n.forward.len() > lvl, "node on level {lvl} too short");
                    if let Some(pk) = prev_key {
                        assert!(*pk < n.key, "level {lvl} not ascending");
                    }
                    prev_key = Some(&n.key);
                    c = n.forward[lvl];
                }
            }
        }
    }

    #[test]
    fn invariants_hold_under_churn() {
        let mut m = SplayMap::with_config(Config {
            splay_interval: 1,
            seed: 99,
        });
        let mut s = 0xabcd_1234u64;
        let mut rng = || {
            s ^= s << 13;
            s ^= s >> 7;
            s ^= s << 17;
            s
        };
        for _ in 0..5_000 {
            let k = (rng() % 300) as u32;
            if rng() & 1 == 0 {
                m.insert(k, k);
            } else {
                m.remove(&k);
            }
            let _ = m.get(&((rng() % 300) as u32));
        }
        m.check_invariants();
        m.rebalance();
        m.check_invariants();
    }

    #[test]
    fn hot_keys_grow_taller_than_cold() {
        let mut m = SplayMap::with_config(Config {
            splay_interval: 64,
            seed: 5,
        });
        for k in 0..400u32 {
            m.insert(k, k);
        }
        // Hammer a hot key; never touch a cold one.
        let hot = 123u32;
        let cold = 321u32;
        for _ in 0..50_000 {
            let _ = m.get(&hot);
        }
        // Several adaptive passes let the hot key climb toward its target.
        for _ in 0..MAX_PASSES {
            m.rebalance();
        }
        m.check_invariants();
        assert!(
            m.height_of(&hot) > m.height_of(&cold),
            "hot height {} should exceed cold height {}",
            m.height_of(&hot),
            m.height_of(&cold)
        );
        assert!(m.height_of(&hot) >= 3, "hot key should be promoted");
    }

    const MAX_PASSES: usize = 12;
}
