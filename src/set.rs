//! The [`SplaySet`] ordered set.

use core::borrow::Borrow;
use core::fmt;
use core::iter::FusedIterator;
use core::ops::RangeBounds;

use crate::map::{Config, SplayMap};

/// An ordered set backed by a self-balancing (splay) skip list.
///
/// This is a thin wrapper around a [`SplayMap`] with `()` values, so it shares
/// the same adaptivity: frequently-looked-up members are promoted to taller
/// towers. Iteration is always in ascending order.
///
/// # Examples
///
/// ```
/// use splaylist::SplaySet;
///
/// let mut set = SplaySet::new();
/// assert!(set.insert(2));
/// assert!(set.insert(1));
/// assert!(!set.insert(1)); // already present
///
/// assert!(set.contains(&1));
/// let sorted: Vec<_> = set.iter().copied().collect();
/// assert_eq!(sorted, [1, 2]);
/// ```
#[derive(Clone)]
pub struct SplaySet<T> {
    map: SplayMap<T, ()>,
}

impl<T> SplaySet<T> {
    /// Creates an empty set with the default configuration.
    #[must_use]
    pub fn new() -> Self {
        Self {
            map: SplayMap::new(),
        }
    }

    /// Creates an empty set with the given [`Config`].
    #[must_use]
    pub fn with_config(config: Config) -> Self {
        Self {
            map: SplayMap::with_config(config),
        }
    }

    /// Creates an empty set that can hold at least `capacity` members without
    /// reallocating.
    #[must_use]
    pub fn with_capacity(capacity: usize) -> Self {
        Self {
            map: SplayMap::with_capacity(capacity),
        }
    }

    /// Returns the number of members.
    #[must_use]
    pub fn len(&self) -> usize {
        self.map.len()
    }

    /// Returns `true` if the set is empty.
    #[must_use]
    pub fn is_empty(&self) -> bool {
        self.map.is_empty()
    }

    /// Removes all members, keeping allocated capacity.
    pub fn clear(&mut self) {
        self.map.clear();
    }

    /// Returns an iterator over the members in ascending order.
    #[must_use]
    pub fn iter(&self) -> Iter<'_, T> {
        Iter {
            inner: self.map.keys(),
        }
    }
}

impl<T: Ord> SplaySet<T> {
    /// Adds `value`; returns `true` if it was not already present.
    pub fn insert(&mut self, value: T) -> bool {
        self.map.insert(value, ()).is_none()
    }

    /// Removes `value`; returns `true` if it was present.
    pub fn remove<Q>(&mut self, value: &Q) -> bool
    where
        T: Borrow<Q>,
        Q: Ord + ?Sized,
    {
        self.map.remove(value).is_some()
    }

    /// Removes and returns the stored member equal to `value`, if present.
    pub fn take<Q>(&mut self, value: &Q) -> Option<T>
    where
        T: Borrow<Q>,
        Q: Ord + ?Sized,
    {
        self.map.remove_entry(value).map(|(k, ())| k)
    }

    /// Returns `true` if the set contains `value`.
    pub fn contains<Q>(&self, value: &Q) -> bool
    where
        T: Borrow<Q>,
        Q: Ord + ?Sized,
    {
        self.map.contains_key(value)
    }

    /// Returns a reference to the stored member equal to `value`, recording an
    /// access that informs later adaptation.
    pub fn get<Q>(&self, value: &Q) -> Option<&T>
    where
        T: Borrow<Q>,
        Q: Ord + ?Sized,
    {
        self.map.get_key_value(value).map(|(k, ())| k)
    }

    /// Returns the smallest member.
    #[must_use]
    pub fn first(&self) -> Option<&T> {
        self.map.first_key_value().map(|(k, ())| k)
    }

    /// Returns the largest member.
    #[must_use]
    pub fn last(&self) -> Option<&T> {
        self.map.last_key_value().map(|(k, ())| k)
    }

    /// Returns an iterator over the members in `range`, in ascending order.
    pub fn range<Q, R>(&self, range: R) -> Range<'_, T>
    where
        T: Borrow<Q>,
        Q: Ord + ?Sized,
        R: RangeBounds<Q>,
    {
        Range {
            inner: self.map.range(range),
        }
    }

    /// Forces a full adaptive rebalance pass over the structure.
    pub fn rebalance(&mut self) {
        self.map.rebalance();
    }
}

impl<T> Default for SplaySet<T> {
    fn default() -> Self {
        Self::new()
    }
}

impl<T: fmt::Debug> fmt::Debug for SplaySet<T> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.debug_set().entries(self.iter()).finish()
    }
}

impl<T: PartialEq> PartialEq for SplaySet<T> {
    fn eq(&self, other: &Self) -> bool {
        self.map == other.map
    }
}

impl<T: Eq> Eq for SplaySet<T> {}

impl<T: PartialOrd> PartialOrd for SplaySet<T> {
    fn partial_cmp(&self, other: &Self) -> Option<core::cmp::Ordering> {
        self.map.partial_cmp(&other.map)
    }
}

impl<T: Ord> Ord for SplaySet<T> {
    fn cmp(&self, other: &Self) -> core::cmp::Ordering {
        self.map.cmp(&other.map)
    }
}

impl<T: core::hash::Hash> core::hash::Hash for SplaySet<T> {
    fn hash<H: core::hash::Hasher>(&self, state: &mut H) {
        self.map.hash(state);
    }
}

impl<T: Ord> FromIterator<T> for SplaySet<T> {
    fn from_iter<I: IntoIterator<Item = T>>(iter: I) -> Self {
        let mut set = Self::new();
        set.extend(iter);
        set
    }
}

impl<T: Ord> Extend<T> for SplaySet<T> {
    fn extend<I: IntoIterator<Item = T>>(&mut self, iter: I) {
        for value in iter {
            self.insert(value);
        }
    }
}

/// An iterator over a [`SplaySet`]'s members in ascending order.
///
/// Created by [`SplaySet::iter`].
pub struct Iter<'a, T> {
    inner: crate::iter::Keys<'a, T, ()>,
}

impl<T> Clone for Iter<'_, T> {
    fn clone(&self) -> Self {
        Self {
            inner: self.inner.clone(),
        }
    }
}

impl<'a, T> Iterator for Iter<'a, T> {
    type Item = &'a T;
    fn next(&mut self) -> Option<Self::Item> {
        self.inner.next()
    }
    fn size_hint(&self) -> (usize, Option<usize>) {
        self.inner.size_hint()
    }
}

impl<T> DoubleEndedIterator for Iter<'_, T> {
    fn next_back(&mut self) -> Option<Self::Item> {
        self.inner.next_back()
    }
}

impl<T> ExactSizeIterator for Iter<'_, T> {}
impl<T> FusedIterator for Iter<'_, T> {}

/// An iterator over a range of a [`SplaySet`]'s members.
///
/// Created by [`SplaySet::range`].
pub struct Range<'a, T> {
    inner: crate::iter::Range<'a, T, ()>,
}

impl<T> Clone for Range<'_, T> {
    fn clone(&self) -> Self {
        Self {
            inner: self.inner.clone(),
        }
    }
}

impl<'a, T> Iterator for Range<'a, T> {
    type Item = &'a T;
    fn next(&mut self) -> Option<Self::Item> {
        self.inner.next().map(|(k, ())| k)
    }
}

impl<T> FusedIterator for Range<'_, T> {}

/// An owning iterator over a [`SplaySet`]'s members in ascending order.
pub struct IntoIter<T> {
    inner: crate::iter::IntoIter<T, ()>,
}

impl<T> Iterator for IntoIter<T> {
    type Item = T;
    fn next(&mut self) -> Option<Self::Item> {
        self.inner.next().map(|(k, ())| k)
    }
    fn size_hint(&self) -> (usize, Option<usize>) {
        self.inner.size_hint()
    }
}

impl<T> ExactSizeIterator for IntoIter<T> {}
impl<T> FusedIterator for IntoIter<T> {}

impl<'a, T> IntoIterator for &'a SplaySet<T> {
    type Item = &'a T;
    type IntoIter = Iter<'a, T>;
    fn into_iter(self) -> Self::IntoIter {
        self.iter()
    }
}

impl<T> IntoIterator for SplaySet<T> {
    type Item = T;
    type IntoIter = IntoIter<T>;
    fn into_iter(self) -> Self::IntoIter {
        IntoIter {
            inner: self.map.into_iter(),
        }
    }
}
