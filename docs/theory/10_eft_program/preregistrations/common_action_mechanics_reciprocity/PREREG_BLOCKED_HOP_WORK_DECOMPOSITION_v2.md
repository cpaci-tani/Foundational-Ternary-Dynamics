# PRE-REGISTRATION — Blocked-hop work decomposition v2

**Date locked:** 2026-07-24  
**Identifier:** `FTD-0460`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Supersedes:** v1 execution only; v1 estimators and classifications unchanged  
**Engine artifacts:** `campaign_blocked_hop_work_decomposition_v2.cpp`,
`coupled_wave_tick_snapshot.h`

**Locked SHA-256:**

- v2 wrapper: `6D03B377870D179E7D94C1AB4A7089DE83AAD08C14FF49D9D8DAD247260F3A15`;
- snapshot observer: `4EDF02D09A24D2B1E97279970524D87FF6D4F3CE105AB5E1A533D83737D7AA3B`;
- inherited v1 campaign body: `216E799FF8424129D2C1F3D6FAE2767E20CC1F6E0403D466819DE1282C290172`.

## Correction boundary

The v1 binary exceeded its 600-second observer timeout before emitting a
record. The cause is implementation-specific: repeated non-const `voxels()`
handouts inside `advance_coupled_wave_tick` mark the ternary cache dirty, so
the next site-level source operator rebuilds the full ternary field. The v2
snapshot observer performs all stencil reads through one const voxel view and
all writes through one later mutable view. Its kick-drift equations, initial
conditions, 42 attempt ticks, estimators, gates, and dominance classification
are byte-inherited from the locked v1 campaign body.

In addition to every v1 gate, the 42 full-work values must reproduce the
FTD-0459 run-of-record series to `1e-12`. Failure is `PROTOCOL_INVALID`.

No physics parameter or outcome threshold changed.

## Result

All validity gates passed. The 42 full-work values reproduce FTD-0459 exactly.
The static polarity source has RMS work `0.008488229634971121`, rescues all 30
otherwise-invalid updates when removed, and exceeds the second-largest RMS by
`94.56x`. Verdict: `STATIC_POLARITY_SOURCE_DOMINATES_BLOCKED_HOP_WORK`.
