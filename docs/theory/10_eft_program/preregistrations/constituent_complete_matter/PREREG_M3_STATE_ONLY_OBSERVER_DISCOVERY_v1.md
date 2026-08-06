# FTD-0754 — M3 state-only observer discovery replay v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE REGISTERED DISCOVERY REPLAY]`  
**Date:** 2026-07-30  
**Parents:** FTD-0743 state-only predicate contract; FTD-0753 explicit-rounding
causal-horizon witness  
**Scope:** observer discovery on already-observed histories only; no held-out
validation, M3 family, particle, radiation, charge, pole, or production claim

## 1. Question

Can one deterministic instantaneous observer split the registered centered
field readout into a selected local Gauss dressing, an outgoing Maxwell
characteristic, and a complementary incoming-plus-radial background while
preserving exact reconstruction, the quadratic norm, Gauss compatibility,
translation/cubic/polarity covariance, and the prior FTD-0753 trajectory?

This is discovery. It may expose useful margins and failure modes but may not
use them as validation evidence.

## 2. Frozen state-only observer

The input is only the complete instantaneous connected-pair state and frozen
action parameters. Tick, arm name, preparation name, history, future state,
periodic-return information, and source route are not passed to the observer.

1. `F_b` is the unique finite-support minimum-energy face field satisfying the
   instantaneous pair's Gauss source on support radius four with zero support
   boundary crossing.
2. The integer-time magnetic readout is reconstructed from
   `(E_n,B_{n-1/2})` by the existing matched half-step.
3. Face `E` is centered from the two adjacent normal faces; edge `B` is
   centered from the four adjacent parallel edges.
4. At noncentral sample direction `n`, residual tangential fields give
   `E_o=(E_t-n cross B_t)/2`, `B_o=n cross E_o`,
   `E_i=(E_t+n cross B_t)/2`, and `B_i=-n cross E_i`.
5. `F_o=(E_o,B_o)`. `F_bg` is the incoming characteristic plus both radial
   components. The central residual is assigned wholly to background because
   the state supplies no radial direction there.
6. Shell radius is used only for reporting. It never selects outgoing content.

The observer must satisfy pointwise reconstruction, quadratic energy
partition, and `S_r=E_o^2-E_i^2` to `1e-12` relative scale. Actual-minus-bound
Gauss residual must be at most `1e-12`. All volumes are odd, excluding the
even-volume Nyquist null mode of the centered readout. Exact reconstruction is
claimed only for this registered readout, not for primitive cochains.

Frozen implementation hashes before registered replay:

- interface:
  `CF6D1803EE3C7907945D81F55F310869308B0E8F0597A97730F594745C9742B1`;
- implementation:
  `7E52EC380748729A1D596FA474803A3227029A6C0397CC5288D2FE1E69022EFE`;
- unlocked runner candidate:
  `F2167D473AE80D8678E8918ECE3D3502CD671CAD18572C0FDC879C7EB2DBE43D`.

## 3. Frozen discovery corpus and replay

Replay exactly the already-observed FTD-0753 face, edge, and body histories:

- directions `(0,0,1)`, `(0,1,-1)`, `(1,1,1)`;
- plus-minus pair, separation `1.30`, inward momentum `0.0120`;
- periodic `L=321`, support radius 4, ticks `0..312`;
- `dt=1/4`, existing compact-pair action, ordered explicit-rounding WSL2 CUDA
  backend, and every FTD-0753 tolerance unchanged.

Observe only ticks `{0,80,96,115,160,240,297,312}` and shells
`{8,12,16,24,32,48}`. These ticks were selected from the already-published
initial state, three ray-class core-onset times, late-core interval,
radius-48 first passage, and pre-contact endpoint. No new state is thereby
made held out.

The prior scalar CSV rows must reproduce byte-for-byte before the new observer
record is accepted. Frozen baseline hashes are:

- face: `A66AE90177B8D11B1B182BD57D6476B8331E41220BE618D4BF4D8D09CD8C6E08`;
- edge: `DB7CD7106C0FF2589E764A4CFEF28C24978A6B7ADBC036091A7EC09BF55B6DF1`;
- body: `2A2591B0C1B2F08D0AC9B44AA9CD5FE9901E8416BD33215D22B8A7D16431A064`.

Qualification may execute at most eight ticks, writes no artifact, and cannot
count as discovery evidence. Registered output goes only to
`engine/results/ftd_0754/`.

## 4. Frozen algebraic controls

The unit gate fixes, before replay:

- pure outgoing, pure incoming, radial-static, standing-wave, and central
  no-direction samples;
- proper cubic rotation and polarity conjugation;
- a bound-only finite-support pair;
- a Gauss-incompatible negative control.

Incoming and standing content must not be relabelled outgoing by distance.
The bound-only control must have zero residual. Gauss-incompatible input must
fail closed.

## 5. Discovery verdict

The replay passes only if all three arms:

1. execute every original row with valid canonical aggregation;
2. reproduce the registered scalar row strings exactly;
3. return a valid observer at every registered discovery tick;
4. pass the frozen algebraic unit gate.

Passing licenses analysis of discovery margins and drafting FTD-0755. It does
not establish a state-space basin, choose predicate thresholds, prove detached
radiation, or validate the observer on an unseen state.

Failure closes this centered-characteristic separator for the frozen
candidate unless the failure is independently identified as instrumentation
error. No trajectory, tick, tolerance, shell, or formula may be changed after
seeing registered output.
