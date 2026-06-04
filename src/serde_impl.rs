//! `serde` support for [`SplayMap`] and [`SplaySet`], gated behind the
//! `serde` feature.
//!
//! A map serializes as a serde map and a set as a serde sequence. Tower
//! heights and access counters are not part of the serialized form; they are
//! reconstructed as entries are inserted on deserialization.

use core::fmt;
use core::marker::PhantomData;

use serde::de::{MapAccess, SeqAccess, Visitor};
use serde::ser::{SerializeMap, SerializeSeq};
use serde::{Deserialize, Deserializer, Serialize, Serializer};

use crate::{SplayMap, SplaySet};

impl<K, V> Serialize for SplayMap<K, V>
where
    K: Ord + Serialize,
    V: Serialize,
{
    fn serialize<S: Serializer>(&self, serializer: S) -> Result<S::Ok, S::Error> {
        let mut map = serializer.serialize_map(Some(self.len()))?;
        for (k, v) in self {
            map.serialize_entry(k, v)?;
        }
        map.end()
    }
}

struct MapVisitor<K, V>(PhantomData<fn() -> (K, V)>);

impl<'de, K, V> Visitor<'de> for MapVisitor<K, V>
where
    K: Ord + Deserialize<'de>,
    V: Deserialize<'de>,
{
    type Value = SplayMap<K, V>;

    fn expecting(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.write_str("a map of key/value pairs")
    }

    fn visit_map<A: MapAccess<'de>>(self, mut access: A) -> Result<Self::Value, A::Error> {
        let mut out = SplayMap::with_capacity(access.size_hint().unwrap_or(0));
        while let Some((k, v)) = access.next_entry()? {
            out.insert(k, v);
        }
        Ok(out)
    }
}

impl<'de, K, V> Deserialize<'de> for SplayMap<K, V>
where
    K: Ord + Deserialize<'de>,
    V: Deserialize<'de>,
{
    fn deserialize<D: Deserializer<'de>>(deserializer: D) -> Result<Self, D::Error> {
        deserializer.deserialize_map(MapVisitor(PhantomData))
    }
}

impl<T> Serialize for SplaySet<T>
where
    T: Ord + Serialize,
{
    fn serialize<S: Serializer>(&self, serializer: S) -> Result<S::Ok, S::Error> {
        let mut seq = serializer.serialize_seq(Some(self.len()))?;
        for v in self {
            seq.serialize_element(v)?;
        }
        seq.end()
    }
}

struct SetVisitor<T>(PhantomData<fn() -> T>);

impl<'de, T> Visitor<'de> for SetVisitor<T>
where
    T: Ord + Deserialize<'de>,
{
    type Value = SplaySet<T>;

    fn expecting(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.write_str("a sequence of values")
    }

    fn visit_seq<A: SeqAccess<'de>>(self, mut access: A) -> Result<Self::Value, A::Error> {
        let mut out = SplaySet::with_capacity(access.size_hint().unwrap_or(0));
        while let Some(v) = access.next_element()? {
            out.insert(v);
        }
        Ok(out)
    }
}

impl<'de, T> Deserialize<'de> for SplaySet<T>
where
    T: Ord + Deserialize<'de>,
{
    fn deserialize<D: Deserializer<'de>>(deserializer: D) -> Result<Self, D::Error> {
        deserializer.deserialize_seq(SetVisitor(PhantomData))
    }
}
