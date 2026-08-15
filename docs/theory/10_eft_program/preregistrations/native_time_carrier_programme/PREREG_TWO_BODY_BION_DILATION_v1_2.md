# Preregistration: two-body bion dilation v1.2 (proper-covariant preparation)

**Date locked:** 2026-08-15
**Status before execution:** `[PREREGISTERED — PREPARATION REPAIR]`
**Parents:** v1 (lock `7e6840e9`, EXECUTION GATES FAILED 14/20 — spectral
line-jumping), v1.1 (lock `8c97446d`, EXECUTION GATES FAILED 9/20 — see
below). Nothing has been claimed by either execution, per their own
taxonomies.

## 1. Diagnosis of record (what the v1.1 gates caught)

Every boosted γ̂ came back **below 1** (0.43–0.97 of γ) with R² 0.83–0.94
and universality shattered — the boosted bions beat *faster* than rest.
Cause: the v1/v1.1 preparation fixed the **lab** separation at 2D for every
boost, so the **proper** separation was 2Dγ — each boosted cell prepared a
*different, wider, hotter* co-moving initial state, whose deeper capture
beats faster. The event map compared different proper histories; γ̂ < 1 is
the recorded signature of that preparation error, not of the clock.

## 2. The repair (one line of physics)

Lab centers at **±D/γ**, so the proper separation is 2·(3 kink widths) at
every boost — the identical co-moving initial configuration. In the
continuum the model is exactly Lorentz invariant, so the whole event
sequence must then stretch uniformly by γ; what the instrument reads
beyond that is the lattice's genuine discreteness deviation. Everything
else — measurand, event machinery, grid, gates, blind held-out, volume
check — is byte-identical to v1.1.

## 3. Pins

| artifact | SHA-256 |
|---|---|
| instrument `derive_two_body_bion_dilation_v1_2.py` | `95758114894E57F8D1C6179338FE2DD6D3AE5EF80EE51DD79ACE368FB994F1E9` |
| frozen v1.1 instrument | `D1E3705064DAB1F2EF574663F82E584A3152E6244889E655DF3AF09586633C31` |
| frozen v1 instrument | `2713007899838C9A657C178E510D60420654967D8689E6746689AFF90F8E72AF` |

Selftest pre-lock (disclosed): machinery unchanged, synthetic stretch
recovered to 1.4×10⁻⁷; rest cells unchanged (preparation at u = 0 is
identical, D/γ = D). No boosted cell run pre-lock under this preparation.

## 4. Gates and outcomes

Inherited verbatim from v1.1: G1 events ≥ 60; G2 uniformity R² > 0.999;
G3 volume < 1%; G4 per-λ γ̂ within 3%; G5 blind held-out within 3%.
CONSISTENT (all gates, |p̂ − 1| ≤ 0.05) / DEVIATION CANDIDATE
(replication-gated) / EXECUTION GATES FAILED (diagnosis only). Booking:
one LEDGER row for the v1 → v1.1 → v1.2 chain. Lock tag
`preregister-two-body-bion-dilation-v1-2`.
