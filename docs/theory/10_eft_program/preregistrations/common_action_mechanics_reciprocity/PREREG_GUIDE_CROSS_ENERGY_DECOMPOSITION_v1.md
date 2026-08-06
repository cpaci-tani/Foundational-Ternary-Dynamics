# PRE-REGISTRATION — Guide cross-energy decomposition v1

**Date locked:** 2026-07-24  
**Identifier:** `FTD-0463`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parent:** `FTD-0462`  
**Engine artifact:** `engine/tests/campaign_guide_cross_energy_decomposition.cpp`

**Locked campaign SHA-256:**
`27F9A81D045C3A6E7F4BFDE838A195BDBD8A0C1AF212B34E1A279448F2FF022C`

## 1. Question

In FTD-0462's rigid dressed-event work, is the changing wave cross energy
dominated by interference of the source history with the transverse packet or
with the small longitudinal initial dressing?

## 2. Frozen protocol

Evolve the exact FTD-0459 packet, initial dressing, and zero-field moving
polarity source histories separately for 48 ticks at `L=33`. At the same 42
attempt ticks, rigidly translate the source history by `+x` and use the
quadratic wave-energy observer to define

`X(A,S)=H_wave(A+S)-H_wave(A)-H_wave(S)`.

Measure before/after changes `Delta X_packet`, `Delta X_dressing`, and
`Delta X_external` for `external=packet+dressing`. Require

`Delta X_external=Delta X_packet+Delta X_dressing`.

Also decompose external endpoint work and reconstruct FTD-0462's rigid required
work

`W_rigid = W_packet+W_dressing-Delta X_packet-Delta X_dressing`.

No amplitude, phase, offset, speed, or tolerance is scanned.

## 3. Gates and classification

- exactly 42 attempts; all finite;
- cross-energy additivity and endpoint-work additivity residuals `<=1e-12`;
- all reconstructed rigid required works reproduce FTD-0462 to `1e-12`;
- if one component's `Delta X` RMS is at least twice the other's, classify it
  as `PACKET_SOURCE_CROSS_DOMINATES` or
  `DRESSING_SOURCE_CROSS_DOMINATES`; otherwise classify
  `MIXED_GUIDE_CROSS_ENERGY`;
- any validity failure returns `PROTOCOL_INVALID`.

## 4. Interpretation boundary

Dominant packet-source cross energy would establish a classical interference
channel by which a transverse packet changes dressed-event work despite zero
direct scalar endpoint work. It would not establish a Bohmian pilot wave,
quantization, Born statistics, autonomous packet creation, or a local bound
dressing. FTD-0462's nonlocal-event failure remains controlling.

## 5. Execution record

The locked Windows/MSVC run completed all 42 attempts. Exact quadratic
additivity closed to `3.59e-16`, endpoint-work additivity to `2.71e-20`, and
the reconstructed rigid required-work series reproduced FTD-0462 to
`1.13e-16`. The component RMS values were

- packet-source: `1.2775930643866692e-5`;
- dressing-source: `2.3856515736440676e-4`.

The dressing-source term is `18.6730x` larger. Locked verdict:
`DRESSING_SOURCE_CROSS_DOMINATES`.
