# Serialization compatibility review

Date: 2026-09-01

## Established goldens

The compact/four-space-pretty migration fixtures and all pre-existing
`SerializeTests` goldens remain byte-exact. No golden fixture was rewritten to
hide a serializer change. The private writer normalizes exponent leading
zeroes so established output such as `1e-7` remains stable.

## Reviewed non-golden differences

Two changes outside established byte snapshots are intentional:

1. Raw floating values use the exact binary value produced by the strict
   nlohmann parser. For decimal ties where more than one shortest round-trip
   spelling exists, output can differ from RapidJSON. The observed migration
   example is `0.4980392158031464` becoming
   `0.49803921580314636`. The numeric value round-trips exactly, repeated
   serialization is stable, and no asserted golden changes.
2. Collections represented by SDK `unordered_map`/`unordered_set` fields are
   emitted in lexicographic order. This makes mesh attributes, registered and
   unregistered extension names, and `extensionsUsed`/`extensionsRequired`
   deterministic instead of depending on hash iteration order. Core member
   order and raw extension/extras subtree insertion order remain unchanged.

These differences are accepted for the approved 2.0 serialization policy:
they improve determinism, preserve JSON number categories and semantic values,
and do not alter the established compatibility fixtures.

## Edge evidence

Tests cover:

- floating `1.0`, negative zero, exponent output, and float precision;
- signed and unsigned 64-bit limits through raw extras;
- target-width `size_t` output;
- compact and pretty output;
- JSON escaping and unescaped valid UTF-8;
- invalid UTF-8 and non-finite rejection through `GLTFException`;
- registered/unregistered extensions and extras ordering;
- repeated byte-identical serialization; and
- public deserialize/serialize semantic round trips.
