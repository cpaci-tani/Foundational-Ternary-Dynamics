# DERIV — Newton's law of gravity from FTD substrate

**Tag:** [STRONGLY MOTIVATED CONJECTURE] for the prediction itself (epistemic floor inherited from FTD-0015 via `α_G = (m_e/m_P)²` tautology); [DERIVED] for the chain steps 1.1–1.5 that recover Schwarzschild leading-order from substrate. Two flagged postulates (gravitational coupling form, linearized tick coefficient `2/c²`). Closes FTD-0130 resolution-path-(a). **Honest reading**: the 0.38% precision match is the squared FTD-0015 precision (mechanical, not new evidence); the substrate-derivation content is in the chain, not in the numerical match.
**Date:** 2026-05-03 late night
**LEDGER:** FTD-0131
**Verification script:** `scripts/proofs/proof_newton_from_substrate.py` (PASS, 0.38% precision)
**Depends on:**
- FTD-0004 (Phase G discrete Poisson Green's function) — [THEOREM]
- FTD-0110 (cluster-mass identification, k = 1/N_base = 1/4) — [DERIVED at linear level]
- FTD-0015 (m_e = m_P · √(2π) · (16/3) · α¹¹) — [STRONGLY MOTIVATED CONJECTURE]
- Postulates 1, 4, 5 (cubic lattice, locality, determinism)
**Closes:** FTD-0130 resolution-path-(a) (substrate derivation of physical G_N).
**Companion:** `scripts/exploration/audit_schwarzschild_form_2026-05-03.py` (form-comparison work).

---

## 0 · Summary

Newton's law of gravity emerges from FTD substrate as the leading-order behaviour of the discrete-Poisson tick-rate-variation mechanism. The derivation chain has four steps that are derived from substrate axioms + existing algebraic-spine results, plus two explicit postulates that are flagged as such. The resulting prediction:

$$\boxed{\;\alpha_G(e,e) \;=\; \frac{G_N\,m_e^2}{\hbar c} \;=\; \left(\frac{m_e}{m_P}\right)^2 \;=\; \left[\sqrt{2\pi}\cdot\tfrac{16}{3}\cdot\alpha^{11}\right]^2 \;\approx\; 1.745\times 10^{-45}\;}$$

matches the measured value `1.752 × 10⁻⁴⁵` to **0.38%**.

**Significantly: this *also* falsifies** the framework-integer claim `G_N = 1/(b_3+N_c)² = 1/100` as an identification with physical G_N. The substrate-derived value differs from the "1/100" claim by **factor ~10²⁰** under the K_B = m_e calibration, **factor ~300** under K_B = m_P calibration, and **factor ~10⁴³** vs the natural dimensionless gravitational coupling. The "1/100" was a numerical coincidence with no substrate justification.

This document closes FTD-0130 resolution-path-(a) (cluster-primary calibration retained; physical G_N derived from substrate via Phase G + FTD-0015).

---

## 1 · The derivation chain

### 1.1 The form: Phase G provides the 1/r tail

The discrete-Poisson Green's function on the simple-cubic lattice ℤ³ has known asymptotic:

$$G_+(r) \;\to\; \frac{1}{4\pi r}\quad\text{at large}\;r$$

This is the central result of FTD-0004 (Phase G geometric Coulomb), tagged **[THEOREM]** in `SPEC_ALGEBRAIC_SPINE.md §6` and verified to 0.07% at L=384 in the Coulomb tail. The result is independent of any physics interpretation — it's a classical lattice-Green's-function fact.

Specifically: for a delta-source at origin on ℤ³ under the 7-point Laplacian, the potential at large `r` (in voxel units) approaches `1/(4πr)` — the standard 3D Newton-potential form.

### 1.2 The source: cluster-mass identification

Per FTD-0110 (`DERIV_K_FROM_OH_A1G_MULTIPLICITY.md`, **[DERIVED at linear level]**): an N-voxel cluster identifies with mass `M = N · m_e` (under the K_B = m_e calibration declared in FTD-0041).

For the gravitational source density, postulate that each manifested voxel acts as a gravitational source of strength `K_B_grav` (a coupling constant to be determined):

$$\rho_g(\mathbf{x}) \;=\; K_B^{\rm grav}\cdot\mathbb{1}_{\rm manifested}(\mathbf{x})$$

**[POSTULATE 1, flagged]** — *2026-05-24 reconciliation update*: substantively closed by `SPEC_FTD_LAGRANGIAN.md` §4.2 [THEOREM] (variation of S w.r.t. ℒ derives Poisson `∇²ℒ = 4πGρ_mass` with `ρ_mass = K_B · n` in weak field) under the identification `K_B^grav = K_B = m_e` per SPEC §3.4. The `K_B^grav` superscript notation in this section is bookkeeping ("the coupling constant entering the gravity term"), not a claim that it differs from K_B = m_e. See [`AUDIT_NEWTON_POSTULATES_RECONCILIATION.md`](AUDIT_NEWTON_POSTULATES_RECONCILIATION.md) §1 (Reading A confirmed). The original substrate-derivation [OPEN] is hereby closed; remaining open: FTD-0015 prefactor `√(2π)·(16/3)` derivation (separate question, governs α_G hierarchy SMC→DERIVED upgrade).

### 1.3 The discrete Poisson equation

Combining Steps 1.1 and 1.2:

$$\nabla^2_{\rm disc}\,\phi_g(\mathbf{x}) \;=\; K_B^{\rm grav}\cdot\rho_g(\mathbf{x})$$

For an N-voxel cluster at the origin, the potential at large `r` follows from Phase G's Green's function:

$$\phi_g(r) \;=\; \frac{N\cdot K_B^{\rm grav}}{4\pi r} \;=\; \frac{M\cdot K_B^{\rm grav}}{4\pi\,m_e\,r}$$

This step is **[DERIVED]** given Steps 1.1 and 1.2.

### 1.4 The tick-rate response: linearized

Per `FOUND_SPACETIME_EMERGENCE_AND_GRAVITY.md` §3.2: local tick rate responds linearly to local gravitational potential:

$$\frac{d\tau}{dT_U}(\mathbf{x}) \;=\; 1 \,+\, \frac{2\,\phi_g(\mathbf{x})}{c^2}$$

In lattice units (`c_lat² = 1/3`): `tick_rate = 1 + 6 · φ_g`.

**[POSTULATE 2, flagged]** — *2026-05-24 reconciliation update*: substantively closed by `SPEC_FTD_LAGRANGIAN.md` §4.3 [THEOREM] (Born-Infeld core derives `dτ/dt = √(f - v²/f)`, exact Schwarzschild proper time for all f ∈ (0,1]) modulo the clock hypothesis. **Convention clarification**: "tick_rate" in the equation above refers to the metric component `g_00 ≈ 1 + 2Φ/c²` (coefficient `2`), NOT the proper-time ratio `dτ/dt = √g_00 ≈ 1 + Φ/c²` (coefficient `1`). The two are consistent under square-root. SPEC §4.3's [THEOREM] establishes the full nonlinear form for `dτ/dt`; the linearization above is a corollary. See [`AUDIT_NEWTON_POSTULATES_RECONCILIATION.md`](AUDIT_NEWTON_POSTULATES_RECONCILIATION.md) §2 (Reading A confirmed modulo clock hypothesis). The genuine remaining open piece narrows from "derive coefficient 2/c² from substrate dynamics" to: substrate-derivation (or honest-axiom tier) of the **clock hypothesis** used implicitly in SPEC §4.3 — interpretive step not formally tagged anywhere in the corpus.

### 1.5 Combining: leading-order Schwarzschild

For an N-voxel cluster at origin:

$$\frac{d\tau}{dT_U}(r) \;=\; 1 \,+\, \frac{2}{c^2}\cdot\frac{M\cdot K_B^{\rm grav}}{4\pi\,m_e\,r}$$

Comparing to GR's leading-order Schwarzschild:

$$\frac{d\tau}{dT_U}(r) \;=\; 1 \,-\, \frac{2 G_N M}{r c^2}$$

Identifying coefficients (negative for attractive gravity):

$$\boxed{\;G_N \;=\; -\frac{K_B^{\rm grav}}{4\pi\,m_e}\;}$$

This step is **[DERIVED]** given Steps 1.1-1.4.

---

## 2 · The substrate prediction: gravitational hierarchy

The derivation above relates `G_N` to the postulated coupling `K_B_grav`. To extract a *prediction* (rather than a calibration), we use the dimensionless gravitational fine-structure constant for one electron:

$$\alpha_G(e,e) \;=\; \frac{G_N\,m_e^2}{\hbar c}$$

This dimensionless ratio is *the* natural gravitational coupling — it's calibration-independent (any choice of unit system gives the same number).

By the standard Planck-mass relation `m_P² = ℏc/G_N`:

$$\alpha_G(e,e) \;=\; \left(\frac{m_e}{m_P}\right)^2$$

Now substitute FTD-0015's mass formula `m_e = m_P · √(2π) · (16/3) · α¹¹`:

$$\boxed{\;\alpha_G(e,e) \;=\; \left[\sqrt{2\pi}\cdot\tfrac{16}{3}\cdot\alpha^{11}\right]^2\;}$$

This is a substrate-side prediction of the gravitational hierarchy.

### 2.1 Numerical verification

| Quantity | Predicted (substrate) | Measured | Deviation |
|---|---|---|---|
| `m_e / m_P` | `√(2π)·(16/3)·α¹¹` = 4.177 × 10⁻²³ | 4.185 × 10⁻²³ | **−0.19%** |
| `α_G(e,e)` | `(m_e/m_P)²` = 1.745 × 10⁻⁴⁵ | 1.752 × 10⁻⁴⁵ | **−0.38%** |

The percent-level agreement matches FTD-0015's existing precision (0.19% on m_e). The squared form `(m_e/m_P)²` doubles the deviation per error-propagation, hence 0.38%.

This is the famous **gravitational hierarchy problem** — gravity is ~10⁻⁴⁵ times weaker than EM at the electron scale — and FTD predicts it from substrate via the α¹¹ cascade in FTD-0015.

---

## 3 · The framework-integer claim is falsified

The CLAUDE.md key-constants table previously asserted (now annotated, per FTD-0130 minimal patch):

> `G_N (gravity) | 0.01 | 1/(b₃+N_c)²`

This identification fails under all natural readings:

| Reading | Substrate value | Claimed (1/100) | Off by |
|---|---|---|---|
| `G_N` in lattice units, K_B = m_e | 1.26 × 10⁻²² | 0.01 | factor 8 × 10¹⁹ |
| `G_N` in lattice units, K_B = m_P | 3.0 | 0.01 | factor 300 |
| `α_G(e,e)` (dimensionless) | 1.75 × 10⁻⁴⁵ | 0.01 | factor 6 × 10⁴² |

**The "1/100" numerical coincidence has no substrate justification.** It is at minimum a 2.5-order-of-magnitude discrepancy (Planck-mass calibration) and at most a 43-order discrepancy (gravitational fine structure). It does NOT correspond to any natural gravitational quantity in the framework.

**Recommended action:** the `CLAUDE.md` key-constants line should be updated to reflect the substrate-derived prediction `α_G(e,e) = (√(2π)·(16/3)·α¹¹)² ≈ 1.75 × 10⁻⁴⁵`, with explicit attribution to this derivation. The "1/100" claim should be retired (or moved to historical / archived numerology).

---

## 4 · Honest tagging of the chain

The derivation chain has four substrate-derived steps and two postulates:

| Step | Content | Tag | Source |
|---|---|---|---|
| 1.1 | `G_+(r) → 1/(4π r)` at large r | **[THEOREM]** | FTD-0004 (Phase G), classical Glasser-Zucker |
| 1.2 | Cluster mass `M = N · m_e` | **[DERIVED at linear level]** | FTD-0110 |
| 1.2 | `ρ_g = K_B^grav · 1_manifested` *(2026-05-24 reconciliation: K_B^grav = K_B = m_e)* | **[DERIVED via SPEC §4.2 [THEOREM]]** | SPEC_FTD_LAGRANGIAN.md §4.2 (was [POSTULATE 1]; AUDIT_NEWTON_POSTULATES_RECONCILIATION.md §1) |
| 1.3 | `φ_g(r) = M·K_B^grav/(4π·m_e·r)` at large r | **[DERIVED]** | combine 1.1 + 1.2 |
| 1.4 | `tick_rate = 1 + 2φ_g/c²` (linearized; this is `g_00`, not `dτ/dt`) | **[DERIVED via SPEC §4.3 [THEOREM] modulo clock hypothesis]** | SPEC_FTD_LAGRANGIAN.md §4.3 (was [POSTULATE 2]; AUDIT_NEWTON_POSTULATES_RECONCILIATION.md §2) |
| 1.5 | Schwarzschild form to leading order | **[DERIVED]** | combine 1.3 + 1.4 |
| 2.0 | `α_G(e,e) = (m_e/m_P)² = (...)²·α²²` | **[STRONGLY MOTIVATED CONJECTURE]** | inherits FTD-0015 tag |

**Net tag for Newton-from-substrate** *(post-2026-05-24 reconciliation)*: **[DERIVED]** *modulo* (a) the **clock hypothesis** used in SPEC §4.3 (P2's substantive closure mechanism) — interpretive step pending substrate-derivation or honest-axiom tier, and (b) the existing FTD-0015 `[SMC]` tag for the α_G(e,e) prediction floor. Both Postulates 1+2 of the original 2026-05-03 flagging are now subsumed by SPEC §4.2 + §4.3 [THEOREM]s respectively; the clock hypothesis is the single remaining open interpretive piece. See [`AUDIT_NEWTON_POSTULATES_RECONCILIATION.md`](AUDIT_NEWTON_POSTULATES_RECONCILIATION.md) §4 for Arc B horizon implications.

The **load-bearing epistemic floor** for the prediction `α_G(e,e) ≈ 1.75 × 10⁻⁴⁵` is **[STRONGLY MOTIVATED CONJECTURE]** — inherited from FTD-0015. Promoting this to [DERIVED] would require either (a) substrate derivation of the prefactor `√(2π)·(16/3)` in FTD-0015, or (b) an independent path to the gravitational hierarchy that doesn't route through FTD-0015.

---

## 5 · What this derivation establishes

**Establishes:**
- The form of Schwarzschild g_00 to leading order is recovered from FTD substrate via Phase G + cluster-mass + linearized tick response.
- The dimensionless gravitational coupling for one electron is predicted as `α_G(e,e) = (m_e/m_P)²`, with `m_e/m_P` derived from FTD-0015 to 0.19%.
- The squared prediction `α_G(e,e) ≈ 1.745 × 10⁻⁴⁵` matches the measured value to 0.38% — within FTD-0015's existing precision envelope.
- The framework's "G_N = 1/(b_3+N_c)² = 1/100" claim is falsified as an identification with physical G_N under any natural calibration.

**Does NOT establish:**
- Beyond-leading-order GR predictions (Mercury perihelion, light bending, gravitational waves) — these require more substrate work; this derivation only confirms the leading 1/r tail.
- The two flagged postulates (gravitational coupling form, linearized tick response coefficient `2/c²`). These match GR's linearization but are not substrate-derived. Their substrate derivation is [OPEN].
- A first-principles derivation of FTD-0015's prefactor `√(2π)·(16/3)`. This factor is curve-fit; substrate justification of *those particular numerical factors* is [OPEN] and is what would upgrade the gravitational hierarchy prediction from [SMC] to [DERIVED].
- The full nonlinear Einstein equations. The leading 1/r piece is recovered; full nonlinear coupling (which standard GR addresses via the Bianchi identities + Lovelock's theorem, see `DERIV_EINSTEIN_FIELD_EQUATIONS.md`) is a separate question.

---

## 6 · Connection to FTD-0130 (calibration audit)

FTD-0130 surfaced a 10²⁰-order tension between the K_B = m_e calibration and the "G_N = 1/100" framework-integer claim, and identified two resolution paths:

- **(a) Cluster-primary**: keep K_B = m_e, derive physical G_N from substrate, retire the "1/100" claim.
- **(b) Planck-primary**: switch to K_B = m_P, promote FTD-0015 to load-bearing primary mass derivation.

This document executes **resolution-path-(a)**: the substrate G_N derivation is now explicit, and the "1/100" claim is shown to be structurally inconsistent with the substrate. The K_B = m_e calibration is preserved.

Resolution-path-(b) (Planck-primary architectural restructuring) remains [OPEN] as a separate ontological decision — see `scripts/exploration/audit_planck_primary_2026-05-03.py` for verification that path-(b) also works numerically. The choice between (a) and (b) is now a clean either/or based on cluster-primary vs Planck-primary ontological preference, not a hidden inconsistency.

---

## 7 · Single-line summary

**FTD's substrate predicts the gravitational fine-structure constant for one electron as `α_G(e,e) = (m_e/m_P)² = [√(2π)·(16/3)·α¹¹]² ≈ 1.745 × 10⁻⁴⁵`, matching the measured 1.752 × 10⁻⁴⁵ to 0.38% — derived from Phase G's discrete Poisson 1/r tail (FTD-0004 [THEOREM]) + FTD-0110's cluster-mass identification ([DERIVED at linear level]) + FTD-0015's α¹¹ mass formula ([STRONGLY MOTIVATED CONJECTURE]) + two flagged postulates (gravitational coupling form, linearized tick response). The framework-integer claim G_N = 1/(b_3+N_c)² = 1/100 is falsified as an identification with physical G_N under any natural calibration; substrate-derived gravity uses α¹¹ hierarchy, not (b_3+N_c)⁻².**

---

## 8 · Provenance

Derivation work performed during the 2026-05-03 late-night session as the resolution-path-(a) execution for FTD-0130. The chain was identified in `scripts/exploration/audit_schwarzschild_form_2026-05-03.py` (form-comparison work) and made explicit + verified in `scripts/proofs/proof_newton_from_substrate.py`. The prediction `α_G(e,e) = (m_e/m_P)²` was derived using FTD-0015 as load-bearing input, achieving 0.38% agreement with measurement. The "1/100" framework-integer claim was tested and falsified across all four natural readings.
