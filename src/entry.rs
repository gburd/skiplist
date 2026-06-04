//! The entry API for [`SplayMap`](crate::SplayMap).

use crate::map::SplayMap;

/// A view into a single entry in a [`SplayMap`], which may be vacant or
/// occupied.
///
/// Constructed by [`SplayMap::entry`](crate::SplayMap::entry).
pub enum Entry<'a, K: Ord, V> {
    /// An occupied entry.
    Occupied(OccupiedEntry<'a, K, V>),
    /// A vacant entry.
    Vacant(VacantEntry<'a, K, V>),
}

impl<'a, K: Ord, V> Entry<'a, K, V> {
    pub(crate) fn new(map: &'a mut SplayMap<K, V>, key: K) -> Self {
        match map.find_index(&key) {
            Some(idx) => Entry::Occupied(OccupiedEntry { map, idx }),
            None => Entry::Vacant(VacantEntry { map, key }),
        }
    }

    /// Returns a reference to this entry's key.
    #[must_use]
    pub fn key(&self) -> &K {
        match self {
            Entry::Occupied(e) => e.key(),
            Entry::Vacant(e) => e.key(),
        }
    }

    /// Ensures a value is in the entry by inserting `default` if vacant, then
    /// returns a mutable reference to the value.
    pub fn or_insert(self, default: V) -> &'a mut V {
        match self {
            Entry::Occupied(e) => e.into_mut(),
            Entry::Vacant(e) => e.insert(default),
        }
    }

    /// Ensures a value is in the entry by inserting the result of `default` if
    /// vacant, then returns a mutable reference to the value.
    pub fn or_insert_with<F: FnOnce() -> V>(self, default: F) -> &'a mut V {
        match self {
            Entry::Occupied(e) => e.into_mut(),
            Entry::Vacant(e) => e.insert(default()),
        }
    }

    /// Ensures a value is in the entry by inserting the result of `default`
    /// (which receives the key by reference) if vacant.
    pub fn or_insert_with_key<F: FnOnce(&K) -> V>(self, default: F) -> &'a mut V {
        match self {
            Entry::Occupied(e) => e.into_mut(),
            Entry::Vacant(e) => {
                let value = default(&e.key);
                e.insert(value)
            }
        }
    }

    /// Runs `f` on the value if the entry is occupied, then returns the entry.
    #[must_use]
    pub fn and_modify<F: FnOnce(&mut V)>(mut self, f: F) -> Self {
        if let Entry::Occupied(e) = &mut self {
            f(e.get_mut());
        }
        self
    }
}

impl<'a, K: Ord, V: Default> Entry<'a, K, V> {
    /// Ensures a value is in the entry by inserting [`Default::default`] if
    /// vacant.
    pub fn or_default(self) -> &'a mut V {
        self.or_insert_with(V::default)
    }
}

/// An occupied [`Entry`].
pub struct OccupiedEntry<'a, K: Ord, V> {
    map: &'a mut SplayMap<K, V>,
    idx: usize,
}

impl<'a, K: Ord, V> OccupiedEntry<'a, K, V> {
    /// Returns a reference to this entry's key.
    #[must_use]
    pub fn key(&self) -> &K {
        self.map.node_key(self.idx)
    }

    /// Returns a reference to the value.
    #[must_use]
    pub fn get(&self) -> &V {
        self.map.node_value(self.idx)
    }

    /// Returns a mutable reference to the value.
    pub fn get_mut(&mut self) -> &mut V {
        self.map.node_value_mut(self.idx)
    }

    /// Converts the entry into a mutable reference to the value with the map's
    /// lifetime.
    #[must_use]
    pub fn into_mut(self) -> &'a mut V {
        self.map.node_value_mut(self.idx)
    }

    /// Replaces the value, returning the old one.
    pub fn insert(&mut self, value: V) -> V {
        core::mem::replace(self.map.node_value_mut(self.idx), value)
    }

    /// Removes the entry, returning the value.
    #[must_use]
    pub fn remove(self) -> V {
        self.map.remove_at(self.idx).1
    }

    /// Removes the entry, returning the stored key and value.
    #[must_use]
    pub fn remove_entry(self) -> (K, V) {
        self.map.remove_at(self.idx)
    }
}

/// A vacant [`Entry`].
pub struct VacantEntry<'a, K: Ord, V> {
    map: &'a mut SplayMap<K, V>,
    key: K,
}

impl<'a, K: Ord, V> VacantEntry<'a, K, V> {
    /// Returns a reference to the key that would be inserted.
    #[must_use]
    pub fn key(&self) -> &K {
        &self.key
    }

    /// Takes ownership of the key.
    #[must_use]
    pub fn into_key(self) -> K {
        self.key
    }

    /// Inserts `value` and returns a mutable reference to it.
    pub fn insert(self, value: V) -> &'a mut V {
        let idx = self.map.insert_and_index(self.key, value);
        self.map.node_value_mut(idx)
    }
}
