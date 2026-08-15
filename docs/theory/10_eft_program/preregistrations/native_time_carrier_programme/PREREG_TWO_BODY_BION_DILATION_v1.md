# Preregistration: two-body bion dilation v1 (the two-body Lorentz campaign, S2)

**Date locked:** 2026-08-15
**Status before execution:** `[PREREGISTERED]`
**Campaign:** `SCOPE_TWO_BODY_LORENTZ_CAMPAIGN_v1.md` (S2; S1 calibration
completed pre-lock and disclosed below).
**Parents:** FTD-0814 (one-body soliton shape-mode dilation, p = −1.000 ±
0.002, one-energy universality; instrument conventions inherited verbatim),
FTD-1000 (the fold: the campaign tests the adopted clock law's consistency,
it does not derive it), the clock-gate analysis (the Newtonian-node error is
fixed by substitution — here the constituents ARE the field's own kinks).

## 1. Question

Does a genuinely **two-body bound state** — two φ⁴ kinks captured into a
bion, binding and kinetics from **one** energy functional — dilate as the
adopted law requires: Ω(u) = Ω(0)/γ, exponent **p = −1**, the same for
every λ? One-body universality is booked; this is the composite's turn.
A clean deviation is a falsifier run against FC-2's clock clause.

## 2. Instrument (pinned)

`scripts/experiments/temporal_interior/derive_two_body_bion_dilation.py`
SHA-256 `2713007899838C9A657C178E510D60420654967D8689E6746689AFF90F8E72AF`.
Inherits the FTD-0814 lattice (axial, C = 1/√3, leapfrog dt = 1, N = 4096),
boost preparation (contracted profiles), and translation-invariant probe
max|∂ₓφ|; adds: bion preparation (half-separation 3 kink widths, rest-start
capture), sub-bin FFT peak interpolation, split-window drift diagnostic.

**S1 calibration (pre-lock, disclosed):** pure-tone extractor error
1.8×10⁻⁴; rest-frame capture confirmed at both λ (Ω(0) ≈ 0.2001, 0.2616;
drift 1.0–1.7%; beat alive at T = 30000). **No boosted cell was run
pre-lock.**

## 3. Declared design

λ ∈ {0.03, 0.05} (moderate, below the FTD-0814 discreteness limit);
fit grid u/C ∈ {0.25, 0.40, 0.50}; **held-out u/C = 0.60** predicted
blind from the pooled p̂ before its cells are read; N = 8192 volume spot
check at (λ=0.03, u/C=0.40); windows: drop 30%, split-window drift gate.
Estimands: pooled p̂ (primary), per-λ means (universality), C_eff from the
two-parameter law fit (secondary — a two-body data point for FTD-0814's
open ~6% cone-mismatch item).

## 4. Gates and outcomes (all declared)

G1 capture (beat alive every cell); G2 drift < 2% per cell; G3 volume
< 1%; G4 universality: per-λ p spread < 0.10; G5 held-out error < 3%.

- **CONSISTENT** — all gates pass and |p̂ + 1| ≤ max(0.05, 2σ_p): the
  adopted clock law survives its first two-body substrate test
  (conditional pass; no tag moves, FC-2 unchanged).
- **DEVIATION CANDIDATE** — gates pass, p̂ outside the band: escalates to
  the owner as a falsifier candidate against FC-2's clock clause; no
  booking of a deviation without an independent replication lock.
- **EXECUTION GATES FAILED** — any G fails: cell-level diagnosis, nothing
  claimed.

Scope honesty: this is a 1D scalar-field surrogate on the substrate's
axial lattice with the substrate's C — the same surrogate class as the
booked one-body result, not the production engine; an engine-side
replication is the campaign's S2′ if S2 lands. Nothing here touches C3,
G\*, or any α-sector claim.

Artifacts: console log; `scripts/experiments/temporal_interior/results/
two_body_bion_dilation.json`; LEDGER row post-run; lock tag
`preregister-two-body-bion-dilation-v1`.
