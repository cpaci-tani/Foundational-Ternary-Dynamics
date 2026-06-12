# PRE-REGISTRATION — Kinetic-drain scaling of cluster efficiency (FTD-0276 Leg A)

**Status:** `[PRE-REGISTRATION]` — design lock; run of record follows the hash-lock.
**Date:** 2026-06-12
**LEDGER id (reserved):** FTD-0276 (Leg A)
**Git tag (to be applied at lock):** `preregister-drain-scaling-v1`
**Executes:** the FTD-0269 §4 queued follow-up — "is the kinetic drain 0.5 forced by
`1 − 1/N_base` or similar?" — in its falsifiable scaling form.

---

## §1 · Purpose and narrow target

The kinetic drain (`v.wave_vel *= (1 − kinetic_drain)` at each genesis manifestation)
is the engine-tuning constant FTD-0269 found **decisively load-bearing** for the
N(A) law's calibration (the knee shifts ~16 grid-units across drain ∈ {0.25, 0.5, 0.75}).
`FOUND_LATTICE_SPACING_GAUGE_FREEDOM.md` §12 floats — untested — the hypothesis that
the cluster efficiency `k = drain² = 0.5² = 0.25 = 1/N_base`, i.e. that the famous ¼
coefficient (FTD-0110 linear theorem) is the square of the physical drain. This
pre-registration tests that hypothesis in two falsifiable forms.

**Single narrow question:** does the sub-knee cluster efficiency `k_eff(drain) = N̄/A²`
scale as `drain²`, and does it equal `0.25` at the physical drain `0.5`?

## §2 · Frozen definitions

- **k_eff(drain)** `[DEFINITION]` = mean over the sub-knee A-window of `N̄(A, drain)/A²`,
  where `N̄` is the seed-mean settled manifested count.
- **Sub-knee A-window** `[FROZEN]`: `A ≤ 16` (the FTD-0261 knee). Declared before the run.
- **R1 (scaling)**: log-log least-squares exponent `p` of `k_eff(drain) ∝ drain^p`.
- **R2 (value coincidence)**: `k_eff` at `drain = 0.5` vs `drain² = 0.25`.

## §3 · Frozen artifacts

| Artifact | SHA256 |
|---|---|
| `engine/tests/campaign_drain_scan.cpp` | `acd03bbd72a428b1d0ef2ff7f934881057e7db9467fa02742dfb9a003f1d92fd` |
| `scripts/exploration/analyze_drain_scan.py` | `59df53fe0307b19d26f4ea6e2509b344626e504b700b4ac54a82b46a278a25b9` |

The instrument uses the FTD-0276 runtime `kinetic_drain` toggle (CPU + GPU single
path); golden-neutral at the default 0.5 (`test_render_bridge_golden` = `0x56fa28acb5b9fe88`,
verified green on this build with the toggle wired). The analyzer encodes the §5
verdict logic and was frozen before the run of record.

## §4 · Prior information (disclosed for integrity)

This is **not a blind test**. The *direction* is already known:
- **FTD-0269** measured N decreasing with drain (knee {25, 14, 30}; N(10) {23, 3.9, 2.4}
  for drain {0.25, 0.5, 0.75}). The drain² hypothesis predicts k_eff *increasing* as
  `drain²` (exponent +2) — opposite sign to the known trend.
- An **instrument-validation smoke run** (L = 24, 2 seeds, settle = 150, NOT the run of
  record) was executed before this lock to confirm the new toggle changes N and the exe
  runs; it reproduced the FTD-0269 direction (k_eff(0.5) ≈ 0.05, consistent with FTD-0261,
  and decreasing in drain). It set no verdict thresholds.

**Prior-favoured outcome: CLOSED-NEGATIVE.** The value of the pre-registered run of record
is to *quantify* the scaling exponent and the drain=0.5 value on the canonical FTD-0261
stack (L = 32, 5 seeds, fine drain grid), making the boundary rigorous rather than
directional. The verdict thresholds in §5 are symmetric and fixed independent of the smoke
magnitudes.

## §5 · Frozen verdict logic (analyzer-encoded)

- **R1 — drain²-CONFIRMED** iff the fitted exponent `p ∈ [1.8, 2.2]`; else **CLOSED-NEGATIVE**.
- **R2 — COINCIDENCE-HOLDS** iff `|k_eff(0.5) − 0.25| / 0.25 < 0.20`; else **COINCIDENCE-FAILS**.
- **Overall Leg-A verdict = drain²-CONFIRMED** iff BOTH R1 and R2 hold; otherwise
  **CLOSED-NEGATIVE** (the kinetic drain does not explain the ¼ coefficient via a square law).

## §6 · Run of record (frozen invocation)

```
campaign_drain_scan --L=32 --drains=0.125,0.25,0.375,0.5,0.625,0.75 \
    --As=10,12,14,16,20,25,30,40 --seeds=5 --settle=300 --cpu --tag=v1
python scripts/exploration/analyze_drain_scan.py --csv engine/results/drain_scan/drain_scan_v1.csv
```

Canonical ic1 stack: wave_propagation + gauss_projection + genesis + coupling + langevin
(γ = 0.02, T = 0.005), CPU forced, SOR 150, x-axial point injection A·K_GENESIS at center,
settle 300 ticks, settled `manifested_count`. (drain = 0.5 row reproduces the FTD-0261 N(A)
law as a built-in sanity check.)

## §7 · Pre-declared outcomes

- **OUTCOME A:** R1 ∈ {CONFIRMED, CLOSED-NEGATIVE} and R2 ∈ {HOLDS, FAILS} — any combination
  is an informative, publishable boundary result. drain²-CONFIRMED would be a partial
  derivation of the ¼ coefficient (feeding Leg C); CLOSED-NEGATIVE sharpens the FTD-0269
  engine-emergent boundary.
- **OUTCOME B (partial):** the fit is rank-deficient or k_eff non-monotone such that the
  exponent is ill-defined — report the raw k_eff(drain) table, no scaling claim.

## §8 · Pre-declared exclusions (banned moves)

1. The sub-knee window (A ≤ 16), the exponent band [1.8, 2.2], and the coincidence tolerance
   (0.20) are frozen — no post-hoc adjustment to move a verdict.
2. No re-fit over a hand-picked drain sub-range to manufacture an exponent near +2.
3. No claim that drain² holds "in some regime" without it passing the frozen R1 over the full
   declared grid.
4. CLOSED-NEGATIVE here does **not** demote FTD-0110's linear k = ¼ theorem (O_h
   representation theory is mathematics, independent of the engine drain) — it only falsifies
   the *engine-side* drain² *origin* of that number.
5. Zero promotions: FTD-0013 [SMC], MC-T4.3, FTD-0110/0261/0269 statuses unchanged regardless
   of outcome.

## §9 · Hash-lock declaration

This document, the campaign instrument, and the analyzer are committed together and tagged
`preregister-drain-scaling-v1` BEFORE the §6 run executes. The §3 SHA256 hashes bind the
instrument and analyzer versions. Any post-lock edit to §§2, 5, 6, 8 or to either artifact
invalidates the lock and requires a v2. Leg B (forward-model γ knob) and Leg C (drain
derivation attempt) are separate deliverables under the same FTD-0276 row, not gated by this
lock.
