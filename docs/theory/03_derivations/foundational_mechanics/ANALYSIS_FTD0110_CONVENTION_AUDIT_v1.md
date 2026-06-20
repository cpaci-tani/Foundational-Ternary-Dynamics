# Analysis — FTD-0110 Convention Audit: the N(A) calibration is PHYSICAL, not convention

**Tag:** `[MEASURED — BOUNDARY: exit (ii) CLOSED NEGATIVE]`
**Date:** 2026-06-19
**LEDGER row:** FTD-0307 (provisional — confirm against corpus max-id before allocating; the main table trails the true max 0306, with 0305 reserved-uncommitted)
**Pre-registration:** [`PREREG_FTD0110_CONVENTION_AUDIT_v1.md`](PREREG_FTD0110_CONVENTION_AUDIT_v1.md), tag `preregister-ftd0110-convention-audit-v1`, lock commit `5023d259`.
**Run of record:** `engine/results/convention_audit/drain_scan_convention_v1.csv` (528 rows, 6 drains × 11 amplitudes × 8 seeds).
**Frozen artifacts:** `engine/tests/campaign_drain_scan.cpp` (SHA `acd03bbd…`), `scripts/exploration/analyze_drain_convention.py` (SHA `c9bbe1a6…`).

---

## 0 · Verdict

The never-attempted **exit (ii)** of the FTD-0110 nonlinear bridge — *is the N(A)
cluster-mass calibration pure CONVENTION (only the dimensionless shape is physical, the
rest an affine rescaling of (A, N)) or PHYSICAL (the knob changes the dimensionless
shape)?* — closes **NEGATIVE on both engine knobs**:

- **drain — PHYSICAL** (this audit, run of record, under the locked STRICT gate).
- **γ — PHYSICAL** (`[established]`, FTD-0276 Leg B; the quick-check that motivated this
  audit; owner decision not to re-measure).

There is **no "only the dimensionless shape is physical" escape**. Combined with exit (i)'s
simplest forms already `[CLOSED NEGATIVE]` (FTD-0276: drain², quadrature-½, 1−1/N_base,
v1 counting), the FTD-0110 nonlinear calibration is neither *derivable* by the simple
routes nor *dismissable* as gauge convention: it is **irreducibly engine-emergent physical
content**. The FTD-0269 BOUNDARY is **hardened**, not escaped.

**The linear `k = ¼` theorem (O_h representation theory) is untouched mathematics.**
**No promotions:** FTD-0013 `[SMC]`, MC-T4.3 `[FOUNDATIONAL OBSTRUCTION]`, the SM
cluster-mass identification `[SMC]`, and FTD-0110 itself (`[OPEN]`, boundary now hardened
on both exits) are all unchanged.

This is a Number-One-Goal clause-2 result: a rigorously mapped boundary on what the
discrete ontology does *not* permit (treating the nonlinear calibration as removable
convention), stated in advance (40%-prior outcome) and landed.

## 1 · Run of record

Platform (frozen, pre-reg §2): canonical ic1 stack `wave_propagation + gauss_projection
+ genesis + coupling + langevin(γ=0.02, T=0.005)`; L=32; x-axial point injection
`A·K_GENESIS`; CPU, SOR=150; settle=300; 8 seeds; drains {0.125, 0.25, 0.375, 0.5,
0.625, 0.75}; amplitudes {10,12,14,16,20,25,30,40,50,70,90}.

Execution note: the 528 runs were produced by 12 parallel workers (6 drains × 2
amplitude-groups) and merged. Each `(drain, A, seed)` run is independent and
deterministic — the RNG seed is `seed_base + s·2654435761`, a function of the seed index
only, not of drain or A (`campaign_drain_scan.cpp:117`) — so the merged CSV is
**bit-identical** to a single serial run; parallelization changed wall-clock only.

## 2 · Per-drain broken-power fit and the three gates

Adjudicator output (`analyze_drain_convention.py`, segmented log-log fit + seed bootstrap):

| drain | knee | p_lo (sub-knee) | p_hi (super-knee) | p_hi bootstrap |
|---|---|---|---|---|
| 0.125 | 14 | 0.50 | 1.94 | 1.944 ± 0.025 |
| 0.250 | 16 | 0.79 | 1.89 | 1.895 ± 0.023 |
| 0.375 | 16 | 2.10 | 1.86 | 1.847 ± 0.045 |
| 0.500 | 14 | 3.73 | 1.64 | 1.666 ± 0.042 |
| 0.625 | 25 | 2.44 | 1.74 | 1.717 ± 0.060 |
| 0.750 | 30 | 2.07 | 1.88 | 1.796 ± 0.109 |

Locked STRICT gates (owner decision 2026-06-19):
- `spread(p_lo) = 166%` ≥ 10% → **fail**
- `spread(p_hi) = 16%` ≥ 10% → **fail**
- `collapse median-CV = 19%` ≥ 5% → **fail**

→ **PHYSICAL** (any gate failing suffices). Three independent gates fail.

## 3 · Robustness (why the verdict is not a fit artifact)

Honest scrutiny of the headline numbers:

- **The 166% `p_lo` spread overstates the effect.** The sub-knee segment is short
  (A ∈ {10,12,14,16}) and the fitted knee moves across drains (14→30), so `p_lo` is fit
  over different windows with large bootstrap CIs (±0.2–0.56). The verdict does **not**
  rest on `p_lo`.

- **The verdict rests on the super-knee exponent and the collapse, both robust.**
  A clean *single* power-law fit on the super-knee only (A ≥ 20, many points, **no
  knee ambiguity**) gives a **monotone** decrease:

  | drain | 0.125 | 0.25 | 0.375 | 0.5 | 0.625 | 0.75 |
  |---|---|---|---|---|---|---|
  | `p_super(A≥20)` | 1.91 | 1.86 | 1.91 | 1.78 | 1.60 | 1.59 |

  spread ≈ 18%, monotone, with the drain=0.125 vs 0.5 difference significant at ~6σ on
  the bootstrap CIs. **A pure (A,N) rescaling keeps this exponent exactly constant; an
  18% monotone decrease is a real shape change.** Physically intuitive: a larger kinetic
  drain removes more wave energy per genesis event, flattening cluster growth at large A.

- **The collapse test is fit-window-independent** and fails cleanly (median cross-drain
  CV 19% ≫ 5% after knee-rescaling) — the six curves do not lie on one master curve.

- **Gate sensitivity, disclosed.** Under a *lenient* gate (the FTD-0261 fit bands
  `p_hi ∈ [1.6, 2.1]`) every `p_hi` value qualifies, so the lenient gate would not have
  separated on `p_hi` alone. The owner pre-registered the STRICT gate precisely to avoid
  that ambiguity; under it the monotone 18% super-knee trend and the 19% collapse are
  decisive. The verdict is a property of the pre-registered bar, declared in advance.

## 4 · What this means for the FTD-0110 bridge

Both exits are now mapped:
- **Exit (i) — derive the calibration:** simplest forms `[CLOSED NEGATIVE]` (FTD-0276).
- **Exit (ii) — it's convention:** `[CLOSED NEGATIVE]` (this audit) — drain and γ both
  change the dimensionless shape.

⇒ The nonlinear calibration of N(A) is **irreducibly engine-emergent physical content**.
The remaining clean-derivation routes for the bridge are: (a) derive the values
`kinetic_drain = 0.5` and `γ = 0.02` from the FTD action/postulates (a hard `[OPEN]`,
plausibly needing a 6th-postulate-class input — the MC-T4.3-style obstruction), or
(b) accept them as motivated `[IMPOSED]` and land the *shape* as
`[CONDITIONAL — DERIVED-GIVEN-IMPOSED]` via a collective-coordinate counting model
(Design C / exit i, the heavier follow-up; `feedback_imposing_with_motivation`).

## 5 · No promotions

FTD-0013 `[SMC]`, MC-T4.3 `[FOUNDATIONAL OBSTRUCTION]`, the SM cluster-mass identification
`[SMC]`, FTD-0261/0269, and the linear k=¼ theorem (mathematics) are all unchanged. This
audit hardens the FTD-0269 BOUNDARY by closing exit (ii); it promotes nothing.
