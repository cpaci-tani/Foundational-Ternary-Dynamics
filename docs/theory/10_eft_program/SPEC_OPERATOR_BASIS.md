# SPEC · Operator Basis for the EFT Recovery Program (Phase 3)

**Tag:** [REFERENCE]
**Version:** 1.0 (Pre-registration)
**Date:** 2026-04-19
**Status:** Committed before Phase-3 measurements run

> **Pre-registration clause.** This spec enumerates the operators whose
> correlators and scaling dimensions Phase 3 will measure. Naive-counting
> dimensions and expected flow classifications are committed *before* any
> measurement. No post-hoc reassignment of classifications.

---

## 0 · What an operator basis does for an EFT

A Wilsonian effective field theory is organised by operator dimension:

- **Relevant** (Δ < D): coefficient grows at IR; must appear in the
  effective Lagrangian; requires renormalisation.
- **Marginal** (Δ = D): stays constant under RG; defines the coupling
  constants of the theory.
- **Irrelevant** (Δ > D): coefficient shrinks at IR; can be dropped
  from the leading Lagrangian; matters only at short scales.

For a D = 4 continuum EFT the relevant/marginal/irrelevant split is
the standard power-counting. On a cubic lattice with D_space = 3 + 1
time, we expect the same structure to emerge in the IR provided the
engine possesses genuine RG flow. Phase 2 established that α_eff *does*
vary under blocking; Phase 3 asks which operators carry that flow.

---

## 1 · Notation & discretisation

Fields on the lattice (voxel-valued):

- **J_μ(x)** ∈ ℝ³ — the flux field, dim ≈ 1 in natural lattice units
  (so that ∫ d³x J² has dim 2 and the kinetic term is marginal at D=4)
- **s(x)** ∈ {−1, 0, +1} — the ternary state / charge density, dim 1
- **Δ J** — Moore-18-point Laplacian of J (see `field_operators.h`), dim 3
- **∇·J** — central-difference divergence, dim 2
- **∇×J** — central-difference curl, dim 2

Operators are products of these fields and their derivatives at a single
voxel. We always evaluate O(x) by central differences (no smoothing),
matching the existing `ftd::divergence_flux_op` / `curl_flux_op`
conventions.

---

## 2 · The Phase-3 Operator Basis

Six operators, all gauge-invariant (no bare J_μ by itself), sorted by
naive lattice dimension. Each has an integer ID used in the CSV output.

| ID | Operator | Lattice discretisation | Naive Δ | Expected class |
|----|----------|------------------------|---------|----------------|
| O1 | `JJ`   | J · J                                   | 2 | **relevant** (mass term) |
| O2 | `divJ2` | (∇·J)²                                 | 4 | **marginal** (gauge-kinetic, Gauss constraint) |
| O3 | `curlJ2` | (∇×J) · (∇×J)                         | 4 | **marginal** (transverse gauge kinetic) |
| O4 | `JdotDivJ` | J · (∇(∇·J))                        | 5 | **irrelevant** (derivative-contact) |
| O5 | `J4` | (J · J)²                                  | 4 | **irrelevant at D=4**; marginal at D_space=3; <br>boundary between classes |
| O6 | `stateSq` | s · s (charge-squared, integer)       | 2 | **relevant** (charge density, always positive) |

**Rationale for the six.** O1 and O6 capture the two-layer ontology of FTD
(flux + state); O2 and O3 are the two standard gauge-kinetic combinations;
O4 tests a derivative-suppressed operator; O5 tests a quartic self-
interaction. Together they span the dim-2 through dim-5 landscape that
dominates the IR physics.

**What we are *not* measuring in Phase 3.**

- Fermion bilinears (no lattice fermions in current engine)
- Chern-Simons term J · (∇×J) (CP-violating; expected zero in parity-
  symmetric vacuum; null-measurement only)
- Composite operators mixing J and s at different voxels (non-local;
  not part of the standard OPE)

These are documented as `[OPEN]` in `TRACKER_OPEN_ITEMS.md` and are
post-Phase-4 extensions.

---

## 3 · Pre-registered expectations

Scaling dimension Δ is extracted from the asymptotic behaviour of the
operator two-point function:

  C_O(r) ≡ ⟨O(x) · O(x+r)⟩  −  ⟨O⟩²   ∝   r^(−2·Δ)   for large r

Measured Δ will be fitted as slope of ln |C_O(r)| vs ln r over r ∈ [4, L/4]
using the `fit_exponential`-style regression already validated in Phase 1A.

**Pre-registered dimensions** (naive counting, before any measurement):

| ID | Expected Δ | Classification test |
|----|-----------|---------------------|
| O1 | 2 | Relevant if Δ_meas ≤ 2.5 |
| O2 | 4 | Marginal if 3 ≤ Δ_meas ≤ 5 |
| O3 | 4 | Marginal if 3 ≤ Δ_meas ≤ 5 |
| O4 | 5 | Irrelevant if Δ_meas ≥ 4.5 |
| O5 | 4 | Borderline if 3.5 ≤ Δ_meas ≤ 5.5 |
| O6 | 2 | Relevant if Δ_meas ≤ 2.5 |

**Pass criterion for Phase 3:** at least 4 of 6 operators classify
as expected (within the generous brackets above). The brackets are
deliberately wide — this is an initial measurement, not a precision
spectroscopy, and continuum scaling dimensions can shift by O(1) under
RG flow in the presence of interactions.

**Blocking stability criterion:** for each operator, the measured Δ at
L = 32 (after one blocking of L = 64) should agree with the Δ at L = 64
to within 50% (engineering tolerance). This tests whether the dimension
extraction is itself scale-stable. Operators failing this test are
reported as "flow-dependent" rather than having a fixed Δ.

**Anomalous dimensions.** If the Δ measured at L = 64 differs from the
naive value by > 0.2, we report the difference as an *anomalous
dimension* γ. No pre-reg numerical threshold on γ — it is itself a
measurement output.

---

## 4 · Canonical regime for Phase 3

- **Lattice size:** L = 64 (fine) + L = 32 (blocked, produced by Phase 2A
  `block_full()`)
- **Scenario:** `flux-pulse` at tick 2000 (canonical regime from SPEC §3).
  A single propagating Gaussian pulse gives non-trivial fluctuations in
  J and ∇J *without* manifested charges, which is the cleanest
  configuration for measuring operator correlators unperturbed by
  particle contact singularities.
- **Seeds:** 42 (canonical) + 43 for statistical spread. Two-seed average
  quoted in the Phase-3 theory doc; single-seed results also tabulated.
- **r-range:** [4, L/4] = [4, 16] at L = 64; [4, 8] at L = 32.

---

## 5 · What Phase 3 ships

- `engine/include/ftd/eft/operator_spectrum.h` — six operator evaluators
  and a correlator builder.
- `engine/tests/test_eft_operator_spectrum.cpp` — CTest with per-operator
  scaling-dimension extraction + classification against this spec.
- `scripts/benchmarks/measure_anomalous_dimensions.py` — batch runner
  producing CSV of Δ vs L, anomalous-dimension table, and a markdown
  report.
- `DERIV_OPERATOR_SPECTRUM.md` — the theory-doc output comparing measured
  Δ's against the pre-registrations in §3 above.

No changes to existing modules. No new toggles. No dependencies added.

---

## 6 · What Phase 3 does not claim

1. **It is not a complete operator basis.** Six operators span a small
   corner of the dim-2-through-dim-5 space. Missing: operators coupling
   J and s at the same voxel (e.g., s · ∇·J — the Gauss-constraint
   operator itself), fermion-sector operators, CP-violating operators.
2. **It does not test the OPE sum-rule directly.** Wilson-coefficient
   extraction (c_ij from the short-distance expansion
   O_i(x)O_j(0) = Σ_k c_ij^k r^(Δ_k−Δ_i−Δ_j) O_k(0) + …) is a Phase-4
   extension, deferred because it requires more statistics than a
   single-seed measurement can provide.
3. **It does not derive anomalous dimensions from diagrams.** We measure
   them; we do not *predict* them from a perturbative expansion.
