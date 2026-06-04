//! Iterators over a [`SplayMap`](crate::SplayMap).

use core::fmt;
use core::iter::FusedIterator;

use alloc::vec::Vec;

use crate::map::{Node, SplayMap, NIL};

/// An iterator over `(&K, &V)` in ascending key order.
///
/// Created by [`SplayMap::iter`](crate::SplayMap::iter).
pub struct Iter<'a, K, V> {
    map: &'a SplayMap<K, V>,
    front: usize,
    back: usize,
    remaining: usize,
}

impl<K, V> Clone for Iter<'_, K, V> {
    fn clone(&self) -> Self {
        Self {
            map: self.map,
            front: self.front,
            back: self.back,
            remaining: self.remaining,
        }
    }
}

impl<'a, K, V> Iter<'a, K, V> {
    pub(crate) fn new(map: &'a SplayMap<K, V>, front: usize, back: usize, len: usize) -> Self {
        Self {
            map,
            front,
            back,
            remaining: len,
        }
    }
}

impl<'a, K, V> Iterator for Iter<'a, K, V> {
    type Item = (&'a K, &'a V);

    fn next(&mut self) -> Option<Self::Item> {
        if self.remaining == 0 {
            return None;
        }
        let idx = self.front;
        let kv = self.map.node_kv(idx);
        self.front = self.map.node_forward0(idx);
        self.remaining -= 1;
        Some(kv)
    }

    fn size_hint(&self) -> (usize, Option<usize>) {
        (self.remaining, Some(self.remaining))
    }
}

impl<K, V> DoubleEndedIterator for Iter<'_, K, V> {
    fn next_back(&mut self) -> Option<Self::Item> {
        if self.remaining == 0 {
            return None;
        }
        let idx = self.back;
        let kv = self.map.node_kv(idx);
        self.back = self.map.node_prev(idx);
        self.remaining -= 1;
        Some(kv)
    }
}

impl<K, V> ExactSizeIterator for Iter<'_, K, V> {}
impl<K, V> FusedIterator for Iter<'_, K, V> {}

impl<K: fmt::Debug, V: fmt::Debug> fmt::Debug for Iter<'_, K, V> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.debug_list().entries(self.clone()).finish()
    }
}

/// An iterator over `&K` in ascending order.
///
/// Created by [`SplayMap::keys`](crate::SplayMap::keys).
pub struct Keys<'a, K, V> {
    inner: Iter<'a, K, V>,
}

impl<K, V> Clone for Keys<'_, K, V> {
    fn clone(&self) -> Self {
        Self {
            inner: self.inner.clone(),
        }
    }
}

impl<'a, K, V> Keys<'a, K, V> {
    pub(crate) fn new(inner: Iter<'a, K, V>) -> Self {
        Self { inner }
    }
}

impl<'a, K, V> Iterator for Keys<'a, K, V> {
    type Item = &'a K;
    fn next(&mut self) -> Option<Self::Item> {
        self.inner.next().map(|(k, _)| k)
    }
    fn size_hint(&self) -> (usize, Option<usize>) {
        self.inner.size_hint()
    }
}

impl<K, V> DoubleEndedIterator for Keys<'_, K, V> {
    fn next_back(&mut self) -> Option<Self::Item> {
        self.inner.next_back().map(|(k, _)| k)
    }
}

impl<K, V> ExactSizeIterator for Keys<'_, K, V> {}
impl<K, V> FusedIterator for Keys<'_, K, V> {}

/// An iterator over `&V` in ascending key order.
///
/// Created by [`SplayMap::values`](crate::SplayMap::values).
pub struct Values<'a, K, V> {
    inner: Iter<'a, K, V>,
}

impl<K, V> Clone for Values<'_, K, V> {
    fn clone(&self) -> Self {
        Self {
            inner: self.inner.clone(),
        }
    }
}

impl<'a, K, V> Values<'a, K, V> {
    pub(crate) fn new(inner: Iter<'a, K, V>) -> Self {
        Self { inner }
    }
}

impl<'a, K, V> Iterator for Values<'a, K, V> {
    type Item = &'a V;
    fn next(&mut self) -> Option<Self::Item> {
        self.inner.next().map(|(_, v)| v)
    }
    fn size_hint(&self) -> (usize, Option<usize>) {
        self.inner.size_hint()
    }
}

impl<K, V> DoubleEndedIterator for Values<'_, K, V> {
    fn next_back(&mut self) -> Option<Self::Item> {
        self.inner.next_back().map(|(_, v)| v)
    }
}

impl<K, V> ExactSizeIterator for Values<'_, K, V> {}
impl<K, V> FusedIterator for Values<'_, K, V> {}

/// An iterator over the entries in a key range, in ascending order.
///
/// Created by [`SplayMap::range`](crate::SplayMap::range).
pub struct Range<'a, K, V> {
    map: &'a SplayMap<K, V>,
    cur: usize,
    stop: usize,
}

impl<K, V> Clone for Range<'_, K, V> {
    fn clone(&self) -> Self {
        Self {
            map: self.map,
            cur: self.cur,
            stop: self.stop,
        }
    }
}

impl<'a, K, V> Range<'a, K, V> {
    pub(crate) fn new(map: &'a SplayMap<K, V>, start: usize, stop: usize) -> Self {
        Self {
            map,
            cur: start,
            stop,
        }
    }
}

impl<'a, K, V> Iterator for Range<'a, K, V> {
    type Item = (&'a K, &'a V);
    fn next(&mut self) -> Option<Self::Item> {
        if self.cur == NIL || self.cur == self.stop {
            return None;
        }
        let idx = self.cur;
        let kv = self.map.node_kv(idx);
        self.cur = self.map.node_forward0(idx);
        Some(kv)
    }
}

impl<K, V> FusedIterator for Range<'_, K, V> {}

/// An iterator over `(&K, &mut V)` in ascending key order.
///
/// Created by [`SplayMap::iter_mut`](crate::SplayMap::iter_mut).
pub struct IterMut<'a, K, V> {
    inner: alloc::vec::IntoIter<(&'a K, &'a mut V)>,
}

impl<'a, K, V> IterMut<'a, K, V> {
    pub(crate) fn new(pairs: Vec<(&'a K, &'a mut V)>) -> Self {
        Self {
            inner: pairs.into_iter(),
        }
    }
}

impl<'a, K, V> Iterator for IterMut<'a, K, V> {
    type Item = (&'a K, &'a mut V);
    fn next(&mut self) -> Option<Self::Item> {
        self.inner.next()
    }
    fn size_hint(&self) -> (usize, Option<usize>) {
        self.inner.size_hint()
    }
}

impl<K, V> DoubleEndedIterator for IterMut<'_, K, V> {
    fn next_back(&mut self) -> Option<Self::Item> {
        self.inner.next_back()
    }
}

impl<K, V> ExactSizeIterator for IterMut<'_, K, V> {}
impl<K, V> FusedIterator for IterMut<'_, K, V> {}

/// An iterator over `&mut V` in ascending key order.
///
/// Created by [`SplayMap::values_mut`](crate::SplayMap::values_mut).
pub struct ValuesMut<'a, K, V> {
    inner: alloc::vec::IntoIter<(&'a K, &'a mut V)>,
}

impl<'a, K, V> ValuesMut<'a, K, V> {
    pub(crate) fn new(pairs: Vec<(&'a K, &'a mut V)>) -> Self {
        Self {
            inner: pairs.into_iter(),
        }
    }
}

impl<'a, K, V> Iterator for ValuesMut<'a, K, V> {
    type Item = &'a mut V;
    fn next(&mut self) -> Option<Self::Item> {
        self.inner.next().map(|(_, v)| v)
    }
    fn size_hint(&self) -> (usize, Option<usize>) {
        self.inner.size_hint()
    }
}

impl<K, V> DoubleEndedIterator for ValuesMut<'_, K, V> {
    fn next_back(&mut self) -> Option<Self::Item> {
        self.inner.next_back().map(|(_, v)| v)
    }
}

impl<K, V> ExactSizeIterator for ValuesMut<'_, K, V> {}
impl<K, V> FusedIterator for ValuesMut<'_, K, V> {}

/// An owning iterator over `(K, V)` in ascending key order.
///
/// Created by [`IntoIterator::into_iter`] on a [`SplayMap`](crate::SplayMap).
pub struct IntoIter<K, V> {
    slots: Vec<crate::map::Slot<K, V>>,
    cur: usize,
    remaining: usize,
}

impl<K, V> IntoIter<K, V> {
    pub(crate) fn new(slots: Vec<crate::map::Slot<K, V>>, head: usize, len: usize) -> Self {
        Self {
            slots,
            cur: head,
            remaining: len,
        }
    }
}

impl<K, V> Iterator for IntoIter<K, V> {
    type Item = (K, V);
    fn next(&mut self) -> Option<Self::Item> {
        if self.remaining == 0 || self.cur == NIL {
            return None;
        }
        let idx = self.cur;
        let taken = core::mem::replace(&mut self.slots[idx], crate::map::Slot::Vacant(NIL));
        let Node {
            key,
            value,
            forward,
            ..
        } = taken
            .take_occupied()
            .expect("into_iter visited a vacant slot");
        self.cur = forward[0];
        self.remaining -= 1;
        Some((key, value))
    }

    fn size_hint(&self) -> (usize, Option<usize>) {
        (self.remaining, Some(self.remaining))
    }
}

impl<K, V> ExactSizeIterator for IntoIter<K, V> {}
impl<K, V> FusedIterator for IntoIter<K, V> {}
