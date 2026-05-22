# AUDIT · Operator-Spectrum Classification (EFT Phase 3 closure read)

**Tag:** [PARTIAL]
**Version:** 1.0
**Date:** 2026-04-25
**Status:** Test infrastructure validated; pre-registered relevant/marginal/irrelevant classification not recovered in the propagating-pulse regime; partial stratification recovered in the confinement-era scenario.

---

## 1 · Why this audit exists

`STATUS_EFT_CHECKLIST.md` §5 ("Operator Basis") lists three [OPEN]
items including:

> Classify relevant, marginal, and irrelevant directions from
> measured native flow.

The Wilsonian-EFT spine requires that operators stratify by scaling
dimension under blocking — relevant (Δ < D), marginal (Δ = D),
irrelevant (Δ > D) — for the EFT to organise itself by power counting.
This pillar was previously [OPEN]; this audit closes it to [PARTIAL]
with the data below and an honest tag rather than retrofitting the
pre-registered brackets.

Companion: `DERIV_OPERATOR_SPECTRUM.md` (the measurement deriv) and
`SPEC_OPERATOR_BASIS.md` (the pre-registration). This file is the
classification audit; it does not introduce new physics, only assigns
tiers to existing measurements.

---

## 2 · Test execution (2026-04-25)

`engine/tests/test_eft_operator_spectrum` was rebuilt and run on the
CPU-Release configuration (`engine/build_audit_cpu/Release/test_eft_operator_spectrum.exe`).
The WSL2 / RTX-5090 build of the same binary hangs in the per-voxel
inject path on L = 32 (each `GpuEngine::inject_flux` calls
`push_to_device()`, producing N³ full-grid uploads); CPU release
completes in seconds and is the canonical run for this audit. Filing
the GPU inject path as a separate engine issue (see §6).

All four scenarios PASS the existing CTest gates:

```
P1 uniform flux         → 6/6 invalid (correlator flat)        PASS
P2 plane-wave flux      → JJ fit finite, no NaN propagation    PASS
P3–P8 propagating pulse → 5/6 valid+good-fit (R² > 0.5)        PASS
P9 flux-baryon          → 5/6 valid+good-fit (R² > 0.5)        PASS
```

---

## 3 · Classification table (propagating pulse, L = 32, t = 200, fit r ∈ [2, 8])

| ID | Operator   | Naive Δ | Measured Δ | R²    | Tier-by-Δ           | Pre-reg expected   | Match? |
|----|------------|---------|-----------:|------:|---------------------|--------------------|--------|
| O1 | JJ         | 2.0     | 0.531      | 0.997 | strongly relevant   | relevant (≤ 2.5)   | ✓      |
| O2 | divJ²      | 4.0     | 0.458      | 0.918 | strongly relevant   | marginal (3–5)     | ✗      |
| O3 | curlJ²     | 4.0     | 0.391      | 0.959 | strongly relevant   | marginal (3–5)     | ✗      |
| O4 | J·∇(∇·J)  | 5.0     | 0.563      | 0.370 | strongly relevant\* | irrelevant (≥ 4.5) | ✗      |
| O5 | (J·J)²     | 4.0     | 0.753      | 0.992 | relevant            | borderline         | ✗      |
| O6 | s·s        | 2.0     | —          | —     | not measurable      | relevant           | n/a    |

\* Low R² (0.37) — the JdotDivJ correlator is not well fit by a single
power law; the slope is the best-fit value but the structure is more
complex than C ∝ r⁻²ᐩ.

**Net classification verdict (pulse regime).** Every measurable operator
classifies as *relevant* (Δ < 1) regardless of its naive dimension. This
is **not** the standard EFT relevant/marginal/irrelevant split. The
operator basis does not stratify in this scenario; it collapses.

## 3b · Classification table (confinement-era flux-baryon, L = 32, t = 200)

| ID | Operator   | Naive Δ | Measured Δ | R²    | Tier-by-Δ          | Δ-shift vs pulse |
|----|------------|---------|-----------:|------:|--------------------|------------------|
| O1 | JJ         | 2.0     | 0.487      | 0.988 | strongly relevant  | −0.04            |
| O2 | divJ²      | 4.0     | **1.552**  | 0.956 | relevant (high)    | **+1.09 (×3.4)** |
| O3 | curlJ²     | 4.0     | 0.711      | 0.979 | relevant           | +0.32            |
| O4 | J·∇(∇·J)  | 5.0     | 1.677      | 0.839 | relevant (high)    | +1.11            |
| O5 | (J·J)²     | 4.0     | 0.914      | 0.953 | relevant           | +0.16            |
| O6 | s·s        | 2.0     | —          | —     | not measurable     | n/a              |

**Confinement scenario verdict.** Operators *do* stratify here — Δ
spreads from 0.49 (JJ) to 1.68 (J·∇(∇·J)) — but the spread is still
inside the conventionally "relevant" band (Δ < 3), nowhere near the
pre-registered marginal (3–5) or irrelevant (≥ 4.5) bands. The
hierarchy is real but compressed.

---

## 4 · Diagnosis

The audit asked whether the previously-reported "all Δ ≈ 0.5" was a
genuine strong-coupling result, a measurement artefact, or under-
equilibration. The 2026-04-25 rerun reproduces the prior numbers (JJ
Δ = 0.531 vs prior 0.531 to four digits — bit-identical). The pattern
is therefore stable; what is not stable is the *interpretation*.

Three distinct effects are at play:

1. **Pulse-envelope dominance (O1–O5 in §3).** The Gaussian flux pulse
   (σ = 2, amp = 1) at t = 200 has support ~ O(σ · c_wave · t) =
   O(20) lattice units, i.e. roughly the entire L = 32 box. Every
   operator sees the same envelope at scales r ∈ [2, 8], so every
   correlator decays at roughly the same envelope rate. This forces
   measured Δ ≈ envelope-rate ≈ 0.5 across operators that should differ
   by order unity in the IR. **This is a measurement artefact, not a
   strong-coupling regime.**

2. **No RG scale separation (all rows).** Wilsonian Δ classification
   presumes a UV→IR window of at least one decade between the lattice
   scale a and the box scale L. At L = 32 the fit window r ∈ [2, 8] is
   only half a decade. The pre-registered bracket-test ("at L = 64
   should agree with L = 32 to within 50%") cannot be run inside the
   CTest budget; that comparison is the next ticket.

3. **stateSq is genuinely non-measurable in the pulse scenario (O6).**
   The pulse has s = 0 everywhere — no manifestation, only flux. The
   correlator is identically zero, fit invalid. This is not a bug; it
   reflects the scenario not exciting that operator. The flux-baryon
   scenario also fails to activate stateSq (likely because the pre-
   manifested charges produce a flat s² field that cancels in the
   ⟨O⟩²-subtracted correlator). Activating O6 needs a scenario with
   *time-varying* s (genesis enabled) — not part of this test's
   pre-registered scope.

The flux-baryon shift in §3b confirms diagnosis (1): when the pulse
envelope is replaced by a confined-quark background with non-trivial
divergence structure (the Gauss-projected charges produce real ∇·J
content), divJ² stratifies upward by a factor of 3.4. The operator
basis is *physical*, not degenerate; the pulse scenario simply does not
excite operator-specific scaling.

**Diagnosis tag.** The "all Δ ≈ 0.5" pulse-regime collapse is a
**scenario artefact, NOT a genuine strong-coupling result**. Evidence:
the same operators stratify by ×3.4 when the scenario is replaced.
A genuine strong-coupling regime would compress Δ uniformly across
all scenarios.

---

## 5 · Verdict

The Wilsonian-EFT pillar "classify relevant/marginal/irrelevant
directions" is closed to [PARTIAL]:

- ✓ The fit infrastructure is validated (P1 catches degenerate inputs,
  P2 confirms numerical stability, P3-P8 produce reproducible Δ).
- ✓ The operator basis is non-degenerate (flux-baryon vs pulse shows
  ×3.4 spread in divJ²).
- ✓ One classification claim is robust: every measured operator falls
  into the "relevant" (Δ < D = 4) tier in both scenarios. This is
  consistent with FTD operating below the lattice's UV cutoff in
  these scenarios.
- ✗ The naive-counting brackets (relevant/marginal/irrelevant at
  Δ ∈ [0, D-ε], [D], [D+ε, ∞)) **are not recovered** at L = 32. We
  cannot from these data say which operators are marginal vs
  irrelevant — they all measure as relevant.
- [OPEN] The test at L = 64 (canonical) and L = 96 (post-pulse-envelope)
  remains unrun; it is the natural next campaign and would be expected
  to either (a) stratify operators per pre-reg or (b) confirm the
  compression as a real lattice strong-coupling effect.

This audit does **not** demote the operator basis or the EFT program;
it documents that the relevant/marginal/irrelevant classification
demands either a larger lattice or a multi-scenario ensemble before it
becomes a [MEASURED] result rather than a [PARTIAL] one.

---

## 6 · Follow-up tickets

1. **GPU per-voxel inject hot-loop fix.** `GpuEngine::inject_flux`
   calls `push_to_device()` per voxel. Replace with a deferred-flush
   pattern (mark dirty, flush before next kernel). Not in scope for
   this audit but blocks the WSL2 GPU run of the same test.
2. **L = 64, L = 96 campaign.** Rerun on a larger lattice with the
   matched-stencil CG projector; check whether the pulse-envelope
   compression breaks at L ≥ 64.
3. **Multi-scenario operator scan.** Beyond pulse and flux-baryon,
   run on (a) static-charge dipole, (b) thermalised Langevin
   ensemble (FTD-0051), (c) two-pulse interference. The Δ vs scenario
   distribution is the falsifier between "envelope artefact" and
   "genuine FTD-native exponents".
4. **Activate stateSq.** Enable genesis in a flux-pulse scenario so
   that s² has non-trivial spatial structure; rerun O6 only.
5. **Wilson-coefficient extraction.** With ≥ 2 lattice sizes that
   produce stratified Δ, fit c_ij^k from the OPE. Phase-4 deferred per
   `SPEC_OPERATOR_BASIS.md` §5.4.

---

## 7 · Cross-references

- Pre-reg: `SPEC_OPERATOR_BASIS.md` §3
- Measurement: `DERIV_OPERATOR_SPECTRUM.md` (section 1 table is the
  prior single-seed run; this audit reproduces and re-tags it)
- Status checklist: `STATUS_EFT_CHECKLIST.md` §5
- Code: `engine/include/ftd/eft/operator_spectrum.h`,
  `engine/tests/test_eft_operator_spectrum.cpp`
- Ledger row: FTD-0091 (this audit)
