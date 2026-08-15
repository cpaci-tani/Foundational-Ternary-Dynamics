# Preregistration: two-body bion dilation v2 (interpolated-peak probe)

**Date locked:** 2026-08-15
**Status before execution:** `[PREREGISTERED — PROBE REPAIR]`
**Parents:** v1 (`7e6840e9`, GATES FAILED 14/20, spectral line-jumping),
v1.1 (`8c97446d`, GATES FAILED 9/20, preparation covariance), v1.2
(`84a61db8`, GATES FAILED 9/20, event-stream contamination). Nothing
claimed by any execution.

## 1. Diagnosis of record (what the v1.2 gates caught)

γ̂ < 1 persisted (0.72–0.94) after the proper-covariant preparation fix,
acquitting the physics and indicting the detector: the rest stream carried
~30% spurious events (mean period 24 ticks vs carrier 31 — envelope
wiggles, radiation ripples), and the raw site-max probe adds Peierls
sampling wobble under boost at the site-crossing rate. More spurious
events per true cycle in boosted streams ⇒ matched indices arrive early ⇒
γ̂ < 1, worsening with u — exactly the recorded pattern. The v1.1/v1.2
selftest validated the machinery on a clean synthetic and never checked
stream purity on the real trace: a calibration gap, closed below.

## 2. The repair

**Probe:** the parabolically **interpolated interior maximum of φ**
(sub-lattice refinement kills the sampling wobble at the source; the
interior peak height is the bion's cleanest scalar breathing coordinate).
**Calibration gains a purity gate** (would have caught both prior
contaminations pre-lock): the rest event stream must have median period
within 10% of the FFT carrier period and IQR/median < 0.30. Measured
pre-lock: purity error 7.1%/6.4%, **IQR/median 0.06/0.05** (v1.2's raw
stream: 0.88). Measurand, matched-event fit, grid, gates, blind held-out,
volume check: byte-identical to v1.1/v1.2. No boosted cell run pre-lock
under this probe.

## 3. Pins

| artifact | SHA-256 |
|---|---|
| instrument `derive_two_body_bion_dilation_v2.py` | `47DD26FC52ACD1050FD44A411F3003F560E3A6FDEA8338330871CEE9EDB18DBD` |
| frozen v1.2 | `95758114894E57F8D1C6179338FE2DD6D3AE5EF80EE51DD79ACE368FB994F1E9` |
| frozen v1.1 | `D1E3705064DAB1F2EF574663F82E584A3152E6244889E655DF3AF09586633C31` |
| frozen v1 | `2713007899838C9A657C178E510D60420654967D8689E6746689AFF90F8E72AF` |

## 4. Gates, outcomes, stopping rule

Gates inherited verbatim (G1 events; G2 uniformity R² > 0.999; G3 volume;
G4 universality 3%; G5 blind held-out 3%). Outcomes: CONSISTENT
(all gates, |p̂ − 1| ≤ 0.05) / DEVIATION CANDIDATE (replication-gated) /
EXECUTION GATES FAILED. **Declared stopping rule: this is the final
repair of the chain in this session** — a fourth gate failure is booked
as the campaign's honest state (instrument requirements diagnosed, chain
handed off), not repaired again under momentum. One LEDGER row for the
full chain. Lock tag `preregister-two-body-bion-dilation-v2`.
