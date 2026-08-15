# Preregistration: two-body bion dilation v1.1 (event-map repair)

**Date locked:** 2026-08-15
**Status before execution:** `[PREREGISTERED — MEASURAND REPAIR]`
**Parent:** `PREREG_TWO_BODY_BION_DILATION_v1.md` (lock `7e6840e9`), first
execution **EXECUTION GATES FAILED (14/20)** — nothing claimed, per its own
outcome taxonomy.

## 1. Diagnosis of record (what the v1 gates caught)

The declared cell-level diagnosis found the bion trace is a **modulated
carrier**: beat line + the φ⁴ resonant-energy-exchange envelope
(≈ 0.045–0.05) + combination sidebands (the λ=0.05, u/C=0.40 anomaly
0.2834 = dilated carrier ≈ 0.2328 + envelope 0.0506). The naive global FFT
peak jumps lines under boost. Deeper: the bion is an **anharmonic,
chirping clock** — single-number frequencies conflate dilation with
proper-stage drift. The v1 clean-cell hint (p ≈ −1.7 at u/C = 0.25) is
therefore uninterpretable as recorded and is superseded by this repair.

## 2. The repaired measurand

**Beat-event times.** Events = local maxima of the smoothed, detrended
probe trace (pure time domain — no frequency window anywhere, hence no
expectation-biased search). Dilation requires the entire event sequence to
stretch uniformly: **t_n(u) = γ̂ · t_n(0) at matched cycle number n** —
same proper stage compared to same proper stage, immune to amplitude
dependence, chirp, envelope, and sidebands. Estimand: γ̂ from a
through-origin fit over events 10..200; stretch exponent p̂ from
log γ̂ = p̂ · log γ pooled over fit cells; **the adopted law requires
p̂ = +1 and the same γ̂(u) for every λ.**

## 3. Pins

| artifact | SHA-256 |
|---|---|
| instrument `derive_two_body_bion_dilation_v1_1.py` | `D1E3705064DAB1F2EF574663F82E584A3152E6244889E655DF3AF09586633C31` |
| frozen v1 instrument (physics cells identical) | `2713007899838C9A657C178E510D60420654967D8689E6746689AFF90F8E72AF` |

Physics cells unchanged from v1 (lattice, integrator, preparation,
λ ∈ {0.03, 0.05}, fit u/C ∈ {0.25, 0.40, 0.50}, held-out 0.60, N=8192
volume check). **Selftest pre-lock, disclosed:** synthetic modulated chirp
with true stretch 1.10 recovered to 1.4×10⁻⁷ (R² = 1.000000); rest cells
produce 868/1053 events. No boosted cell run pre-lock under this measurand.

## 4. Gates and outcomes

G1 ≥ 60 matched events/cell; **G2 uniformity R² > 0.999** (non-uniform
stretch is itself a failure of the clean dilation picture and blocks any
verdict); G3 volume < 1%; G4 per-λ γ̂ agreement < 3% at each u; G5 blind
held-out γ̂(0.60) within 3%. Outcomes as in v1: CONSISTENT
(all gates, |p̂ − 1| ≤ 0.05) / DEVIATION CANDIDATE (gates pass, p̂
outside; replication-gated escalation against FC-2's clock clause) /
EXECUTION GATES FAILED (diagnosis only). Scope honesty inherited: 1D
surrogate class of the booked FTD-0814 result; engine replication is S2′.

Artifacts: console log; `results/two_body_bion_dilation_v1_1.json`;
booking shared with the v1 execution record in one LEDGER row; lock tag
`preregister-two-body-bion-dilation-v1-1`.
