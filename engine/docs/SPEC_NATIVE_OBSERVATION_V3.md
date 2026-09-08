# Native reference observation protocol v3

Status: implementation contract, frozen before implementation tests. This
extends the production reference transport. It neither changes a finite
microscopic law nor certifies complete-state replay or physical recovery.

`info` advertises `nativeProtocolVersion:3`, `nativeBinaryVersions:[2,3]`,
`nativeInstanceId` (32 lowercase hexadecimal digits), and
`exactIntegerEncoding:"safe-number-or-decimal-string"`. Every unsigned 64-bit
JSON identity/version uses a number through `2^53-1`, then a canonical unsigned
decimal string. A receiver rejects already-rounded numeric identities. Signed
32-bit ticks and particle identifiers retain their exact numeric representation.
The process namespace is external transport identity; its nonce never seeds or
modifies simulation state. It persists across reconnects; a new process
generates a new random namespace to disambiguate reused source epochs.
Uniqueness is probabilistic, not a collision-free mathematical guarantee.

Binary requests opt in with `_binaryVersion:3` and a positive safe-integer
`_requestId`. Missing version keeps the original v2 frame; unsupported versions
or missing/invalid v3 IDs reject before observation. Since v2 has no correlation
field, a binary request combining v2 (explicit or default) with `_requestId`
receives a correlated error before observation; it never emits an uncorrelated
terminal binary reply. This admission rule was clarified by independent review
before the final candidate freeze. A v3 frame has a common
FTN3 prefix followed by the original FTP2, FTV2 or FTS2 payload. Both server and
client retain compact sampling and avoid copying the payload to add/remove the
prefix. The legacy field token remains a uint32 cache/request key.

| Offset | Type, little endian | Meaning |
|---|---|---|
| 0 | uint32 | Magic `0x334e5446` (`FTN3`) |
| 4 | uint32 | Header bytes, exactly 88 |
| 8 | uint64 | Request identity, `1..2^53-1` |
| 16 | uint64 | Actual represented native tick |
| 24 | uint64 | Source epoch |
| 32 | uint64 | Publisher state epoch, including direct edits |
| 40 | uint64 | Exact nested payload bytes |
| 48 | float64 | Reference physical time; nonfinite becomes unavailable in JS |
| 56 | float64 | Reference dt; nonfinite becomes unavailable in JS |
| 64 | uint32 | Actual lattice size |
| 68 | uint32 | Flags, exactly 1: sampled floating reference observation |
| 72 | uint64 | Process nonce low word |
| 80 | uint64 | Process nonce high word |
| 88 | original frame | FTP2, FTV2 or FTS2; exact original layout |

The printable instance ID is high word then low word, each zero-padded to
16 hexadecimal digits. Unknown flags, nested types, mismatched lengths and
unsupported versions reject. Native capture records tick, source and publisher
epoch before sampling and checks that they and the represented size/time/dt
remain unchanged afterward. The single owner dispatches no mutation during
sampling. This does not claim separate requested fields form an atomic batch.

Client acceptance requires the expected socket, instance, source, local scenario
generation, request ID and response kind. Fields additionally match token/kind.
Late, duplicate or unknown replies cannot update caches or clear another
request. Scientific combinations must share instance, source, epoch and sample
tick; legacy frames keep unavailable provenance. The immutable metadata object
declares model `production-reference`, units `reference-lattice`, and status
`approximate`. Exact identifiers do not make float32 payloads exact physics.

Every valid command supplied with `_requestId` produces a correlated reply,
including tick/run, flux slices and formerly silent mutators. Existing mutators
without an ID remain fire-and-forget. Unsolicited telemetry has no request ID.
Request IDs are never recycled into an unresolved request; exhaustion rejects.

Acceptance requires native layout/precision/capture tests, independent real-wire
checks, actual JS codec and ordering tests at `2^53-1`, `2^53`, `2^53+1` and
`2^64-1`, corrupt headers, reversed/duplicate/late completions, source reset at
equal ticks, and legacy compatibility. Registered performance and lifecycle
gates remain separate. Native finite counter exhaustion and backend guarantees
must remain explicit; no unbounded-identity or global replay claim follows.
