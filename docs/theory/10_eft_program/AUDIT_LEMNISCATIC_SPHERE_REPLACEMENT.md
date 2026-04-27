# AUDIT — Lemniscatic Replacement for the 2-Sphere in Einstein and Thermodynamics Formulas

**Tag:** [HYPOTHESIS] (investigation document; conclusions await D1/D2 measurement)
**Date:** 2026-04-27
**LEDGER row:** FTD-0105 (assigned ahead of measurement)
**Plan:** `~/.claude/plans/let-s-plan-a-way-ethereal-sonnet.md` (lemniscatic-replacement investigation)
**Companion (will be written):** `PROTOCOL_LEMNISCATIC_REPLACEMENT.md` (pre-registration), `ANALYSIS_LEMNISCATIC_REPLACEMENT.md` (post-measurement)

---

## 0 · Scope and anti-targets

**This document IS:** a structured catalog of foundational physics formulas (Einstein equations + thermodynamics) where 4π or 8π appears, an analysis of the structural origin of each, a candidate ϖ-native replacement matrix, and a theoretical-analysis section per formula.

**This document IS NOT:** a derivation of any ϖ-native form. Every candidate replacement is tagged at most [CONJECTURE] until either (a) derived from FTD axioms or (b) engine-measured against alternatives.

**Parallel reading:** the existing PF Atlas (`SPEC_FTD_COMPARATIVE_PHYSICS.md`, Feb 2026) decomposes 4π = 16·(π/4) = N_base²·PF as a numerically-exact rearrangement at [SELECTION]. **It makes no different physical predictions.** The investigation here is structurally distinct: it asks whether ϖ should appear *instead of* π in specific formulas, predicting **different numbers** the engine can falsify.

**Pattern-matching is the starting question, never the conclusion.** The pattern "8π in Einstein, 16 = 2·8 in master quadratic" motivates the investigation; the engine arbitrates.

---

## 1 · Phase 1 catalog — formulas in scope

Five foundational formulas (Einstein + thermodynamics, per locked scope) plus three lattice-side anchor identities:

### 1.1 Einstein field equations

$$G_{\mu\nu} = \frac{8\pi G}{c^4} T_{\mu\nu}$$

**FTD doc:** `DERIV_EINSTEIN_FIELD_EQUATIONS.md` §4.3.

**Structural origin of 8π** (per existing FTD derivation):
- π from "solid angle integration (Gauss law on lattice)" — tagged [THEOREM]
- Factor 8 = 2·N_base = 2·4 from "trace-reversal and spinor dimension" — tagged [SELECTION]

The π in the FTD-side derivation enters via Gauss-law integration over a closed surface in the linearised regime. **If that closed surface is implicitly assumed to be a 2-sphere (S² with surface area 4π·r²), the π is replaceable in principle** — a lattice-native closed-surface integral with cubic anisotropy, or with lemniscatic period, would land a different transcendental.

### 1.2 Hawking temperature

$$T_H = \frac{c^3}{8\pi G M k_B}$$

**FTD doc:** `DERIV_BLACK_HOLE_PHYSICS.md` §2 (line 72).

**Structural origin of 8π:** the Euclidean periodicity argument. Near the horizon, the Euclidean metric is a 2D cone in the $(\rho, \tau)$ plane:

$$ds_E^2 = \frac{\rho^2}{4r_s^2}d\tau^2 + d\rho^2 + r_s^2\,d\Omega^2$$

To avoid a conical singularity at $\rho = 0$, $\tau$ must be periodic with $\beta = 4\pi r_s = 8\pi GM/c^3$. The 4π here is the **circumference of a unit circle wrapped around the conical apex** (one full revolution, 2π·1·2 = 4π for the conical-deficit removal).

**Replaceability:** the 4π enters via the ROUND-CIRCLE periodicity. On a CUBIC lattice with cubic-anisotropy near the horizon, the conical periodicity might be a different transcendental. This is engine-measurable through the autocorrelation period of horizon-shell fluctuations.

### 1.3 Hawking β period

$$\beta = 4\pi r_s$$

**FTD doc:** `DERIV_BLACK_HOLE_PHYSICS.md` §2.1 (line 68).

**Structural origin:** same as §1.2 — round-circle periodicity around horizon cone.

### 1.4 Schwarzschild horizon area

$$A = 4\pi r_s^2$$

**FTD doc:** `DERIV_BLACK_HOLE_PHYSICS.md` §3.1 (line 112), and **hardcoded in `engine/tests/benchmark_black_hole_thermo.cpp` line 245** (`hr.horizon_area = 4.0 * ftd::PI * hr.horizon_radius * hr.horizon_radius`).

**Structural origin of 4π:** literal 2-sphere surface area. This is a *geometric* assumption, not a derived FTD result. The benchmark MEASURES r_h on the lattice, then APPLIES A = 4π·r_h² as a closed-form. The lattice itself never directly measures the area — the area is an inferred quantity assuming spherical symmetry.

**Replaceability:** **highest-leverage candidate**. Direct lattice measurement (count voxels at half-max-latency shell × per-voxel area) gives A_actual independent of the 4π assumption. If A_actual / r_h² lands at 4π within stderr, sphere assumption is empirically confirmed on the lattice. If it lands at 4ϖ, 4G*, or another value, structural finding.

### 1.5 Bekenstein-Hawking entropy

$$S_{BH} = \frac{A}{4\ell_P^2}$$

**FTD doc:** `DERIV_BLACK_HOLE_PHYSICS.md` §3.2-3.3 (lines 128, 132).

**Structural origin:** the 1/4 in the entropy is from constraint reduction (Gauss + EOM correlations + parity), tagged [SELECTION]. The 4π enters indirectly via $A = 4\pi r_s^2$, so this formula's 4π is downstream of §1.4.

**Replaceability:** if §1.4's A_actual/r_h² ≠ 4π, then S_BH = A/(4ℓ_P²) is automatically updated; the 1/4 reduction factor is independent.

### Lattice anchors (already on the spine)

#### Anchor 1: Watson identity

$$W_3 = \frac{G^{*2}}{2\pi}$$

**FTD doc:** `DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md`. **[THEOREM].** The BCC-sublattice eigenvalue triple cosine product evaluates to G*²/(2π). This is the cleanest existing structural appearance of ϖ in lattice physics — any other ϖ-native physics formula should reduce to a Watson-style identity at the lattice level.

#### Anchor 2: Phase G geometric Coulomb

$$\alpha_r(r, L) = 2 r \cdot G_L(r) \;\to\; \frac{1}{2\pi} \text{ in continuum limit}$$

**FTD doc:** `DERIV_EMERGENT_COULOMB_GEOMETRIC.md`. **[THEOREM].** The lattice continuum limit of $\alpha_r$ lands at $1/(2\pi)$, NOT $1/(4\pi)$. The 2π comes from the lattice Poisson Green's function asymptotic. This is a structural fact about the FTD lattice that DOES involve a 2π — same 2π that appears in Watson identity.

#### Anchor 3: G* — ϖ — PF identity

$$G^* = \frac{\varpi}{\sqrt{\text{PF}}} = \frac{2\varpi}{\sqrt{\pi}}$$

**FTD doc:** `DERIV_GSTAR_PF_BRIDGE.md`. **[THEOREM].** The bridge between continuous (lemniscate ϖ) and discrete (PF = π/4) geometry. Numerical: ϖ ≈ 2.6221, PF ≈ 0.7854, G* ≈ 2.9587.

---

## 2 · Phase 2 — ϖ-native candidate prediction matrix

For each cataloged formula, generate candidate replacements where standard 4π or 8π is replaced by a lemniscatic-native factor. Numerical separations are computed using the canonical values:

| Constant | Numerical value |
|---|---|
| π | 3.14159 |
| 4π | 12.566 |
| 8π | 25.133 |
| ϖ | 2.62206 |
| 4ϖ | 10.488 |
| 8ϖ | 20.976 |
| G* | 2.95867 |
| 4G* | 11.835 |
| 8G* | 23.669 |
| G*² | 8.7538 |
| G*²·π/2 | 13.749 |
| 16·G*²/π | 44.583 *(was misprinted "35.014" in v1; the intended value 35.015 is 4·G*² = 8π·W₃ on the row above. Corrected per math audit AUDIT_FTD0105_MATH_CHECK.md §3.1.)* |
| 8π·W₃ = 8π·G*²/(2π) = 4·G*² | 35.015 |

### 2.1 Master prediction matrix (Einstein + thermodynamics)

| Formula | Standard | PF Atlas decomp | Candidate ϖ-native A | Candidate ϖ-native B | Candidate ϖ-native C |
|---|---|---|---|---|---|
| §1.4 A / r_s² | **4π = 12.566** | 16·PF = 12.566 | 4ϖ = **10.488** (-16.6%) | 4G* = **11.835** (-5.8%) | G*²·π/2 = **13.749** (+9.4%) |
| §1.3 β / r_s | **4π = 12.566** | (same as above) | 4ϖ = 10.488 | 4G* = 11.835 | G*² = 8.754 |
| §1.2 T·M product (geometrized) | **1/(8π) = 0.0398** | (same) | 1/(8ϖ) = 0.0477 | 1/(8G*) = 0.0423 | 1/(4G*²) = 0.0286 |
| §1.5 S/(N_H·1/4) | (1) | (same) | 1 (downstream of §1.4) | 1 | 1 |
| §1.1 G_μν / (G·T_μν) coefficient | **8π** | (same) | 8ϖ = 20.976 | 8G* = 23.669 | 4·G*² = 35.015 |

The candidates A, B, C are ranked by structural motivation:

- **Candidate A (4ϖ):** "lemniscate replaces circle" — replace π by ϖ wherever a closed 1D curve enters geometrically (perimeter of horizon, periodicity around cone). Most aggressive replacement; assumes ϖ is the lattice's fundamental closed-curve invariant.
- **Candidate B (4G*):** "G* replaces π via the bridge identity G* = 2ϖ/√π" — keeps the structural role of π but rewrites in terms of the lattice's master constant. Conservative.
- **Candidate C (G*²·π/2 or 4·G*²):** "Watson-anchored" — uses the explicit factor G*²/(2π) that already appears in Watson identity. The 4·G*² ≈ 35 candidate would predict horizon areas factor 2.8× larger than standard.

### 2.2 Sanity checks built into the matrix

1. **Standard and PF Atlas always agree** — they're numerically identical rearrangements. If any column of the prediction matrix shows them differing, the matrix is wrong.
2. **The candidates A/B/C are MUTUALLY EXCLUSIVE for any given measurement** — only one can be empirically right per formula.
3. **All three candidates differ from standard by ≥6%** — well above engine bootstrap noise floor (~1-3%) on multi-mass scaling.

### 2.3 Falsifier definition

For each formula, define:

- **PASS for standard sphere:** measured value within ±5% of standard prediction across all mass values. ϖ-native is **closed-negative for this formula**.
- **PASS for one of A/B/C:** measured value within ±5% of one candidate; standard rejected at >5σ. **Structural finding** for that formula.
- **PASS for none:** measured value lands somewhere else (lattice anisotropy, finite-size, parameter regime). Inconclusive; report measured value with stderr.

---

## 3 · Phase 4 — theoretical analysis per formula

For each of the five formulas, classify the structural origin of 4π/8π as **replaceable** (the π enters via an assumption that could in principle be different on the lattice), **locked-to-π** (the π enters via a derivation that doesn't depend on spherical symmetry), or **unclear**.

### 3.1 §1.4 Schwarzschild horizon area A = 4π·r_s² → REPLACEABLE

The 4π here is the literal surface area of a 2-sphere of radius r_s. On the FTD lattice, a "horizon" is the surface where latency $\mathcal{L} \to 1$. The lattice itself does not enforce sphericity — only the COARSE-GRAINED interpretation does. The benchmark already measures r_h directly; the area can be measured directly too without the 4π assumption.

**Verdict: REPLACEABLE.** D1 distinguishes 4π from candidates A/B/C.

### 3.2 §1.3 Hawking β period β = 4π·r_s → UNCLEAR

The 4π here comes from the round-circle Euclidean-time periodicity (avoiding conical singularity). The conical-deficit argument relies on Riemannian geometry, not on the embedding being a 2-sphere — it's about a 2D cone in the (ρ, τ) plane. So strictly the 4π here is "circle circumference 2π" doubled, not "sphere area 4π."

**However**, on a cubic lattice the near-horizon geometry has discrete cubic-anisotropy effects. The "thermal circle" might not be a circle at all — it could be a discretised period that follows lemniscatic or BCC-symmetric structure.

**Verdict: UNCLEAR.** D2 is the test, but interpretation requires care.

### 3.3 §1.2 Hawking T·M = 1/(8π) → DOWNSTREAM of §1.3

T = 1/β, so T·M = 1/(8π·G) in geometrized units (G=c=1). Direct downstream of §1.3.

**Verdict: same as §1.3 (UNCLEAR).** D2 is the same test.

### 3.4 §1.5 Bekenstein-Hawking S = A/(4ℓ_P²) → DOWNSTREAM of §1.4

The 1/4 reduction factor is independent of 4π; the 4π enters only via A. If §1.4 is closed-negative for ϖ-native, S formula remains S = A_measured/(4ℓ_P²) with whatever A the lattice produces.

**Verdict: REPLACEABLE downstream of §1.4.** No independent test needed.

### 3.5 §1.1 Einstein equations 8πG → LOCKED-TO-π via Lovelock

The 8π in the Einstein field equations enters via a chain:
1. Linearised flux-wave-equation derivation (FTD §3): produces a $\Box \bar h_{\mu\nu} = -(16\pi G/c^4) T_{\mu\nu}$ equation
2. Lovelock's theorem (D=4 spacetime): the unique symmetric, divergence-free, second-rank tensor in 4D is $G_{\mu\nu} + \Lambda g_{\mu\nu}$
3. Match: linearised $G_{\mu\nu}^{(1)} = -\frac{1}{2}\Box \bar h_{\mu\nu}$, giving the factor 1/2 from G_μν construction
4. Combine: $G_{\mu\nu} = (8\pi G/c^4) T_{\mu\nu}$

**The 16π in step 1 is the load-bearing place** for π. Step 1's 16π comes from Gauss-law solid-angle integration in the lattice linearisation — i.e., it routes through §1.4 (the 4π in horizon area is the same 4π that appears in solid-angle integration). The 8 in step 1 then becomes part of the 16 = 2·8 = 2·N_base² pattern.

**However, Lovelock's theorem itself is locked to D=4 spacetime geometry; it doesn't depend on lemniscatic substructure.** The Einstein-equation form $G_{\mu\nu} = \kappa·T_{\mu\nu}$ is forced by Lovelock; only the coefficient κ is lattice-dependent.

**Verdict: REPLACEABLE only insofar as §1.4 is** — if A_actual / r_h² ≠ 4π on the lattice, then the Gauss-law solid-angle integration in step 1 also uses a non-4π factor, and the 8π in §1.1 propagates accordingly. **No independent test for §1.1; closure follows from §1.4.**

### 3.6 Theoretical summary

The investigation reduces to **one engine-measurable question**: does the lattice horizon have area 4π·r_h² (sphere) or some other coefficient × r_h²?

If 4π → ϖ-native, the cascade is automatic across §§1.1, 1.4, 1.5; §§1.2, 1.3 require independent confirmation via D2 (autocorrelation period of horizon-shell fluctuations).

This is a much tighter investigation than the catalog initially suggested. **D1 (horizon area) is the load-bearing measurement.**

---

## 4 · What this document allows you to claim (now, before D1/D2)

In order from most to least defensible:

1. "FTD's existing recovered Einstein and thermodynamics formulas all carry factors of 4π or 8π that, in the standard derivation, trace structurally to the 2-sphere geometry of the horizon (literal surface area or Euclidean conical periodicity around the horizon)." — Phase 1 catalog.

2. "The PF Atlas (`SPEC_FTD_COMPARATIVE_PHYSICS.md`) decomposes these factors as 4π = 16·(π/4) = N_base²·PF at [SELECTION], a numerically-exact rearrangement that makes no different prediction. A separate ϖ-native candidate matrix (Phase 2 here) would predict factor differences of 5-30% from standard, distinguishable by engine measurement at L=64." — §2.

3. "The cleanest engine-measurable distinguisher is the lattice horizon area: the existing benchmark measures r_h (horizon radius) and APPLIES A = 4π·r_h² as an assumption (line 245 of `benchmark_black_hole_thermo.cpp`). Direct measurement of A from voxel-counting at the half-max-latency shell would test whether A/r_h² → 4π (sphere) or another factor (lemniscatic candidates)." — §3.1.

4. "The investigation is pre-registered in `PROTOCOL_LEMNISCATIC_REPLACEMENT.md` (forthcoming) with explicit candidate predictions {4π, 4ϖ, 4G*, G*²·π/2} and falsifier criteria. Engine measurement D1 + D2 will close the question one way or the other." — §2.3 + plan §3.

What this document explicitly does NOT allow you to claim:

- That ϖ replaces π in any formula (no measurement landed yet)
- That the existing PF Atlas is wrong (it's a parallel reading at [SELECTION])
- That the master quadratic coefficients 16G*² ≈ 110 OR 16G*³ ≈ 289 must appear in physics formulas (no derivation route established for this)
- That any of the candidates A/B/C is structurally privileged (the engine arbitrates)

---

## 5 · Cross-references

| Section | Primary doc | Status |
|---|---|---|
| §1.1 Einstein eqs | `DERIV_EINSTEIN_FIELD_EQUATIONS.md` §4.3 | [THEOREM]/[SELECTION] mix |
| §1.2-1.5 BH thermo | `DERIV_BLACK_HOLE_PHYSICS.md` §2-§3 | [THEOREM]/[SELECTION] mix |
| Anchor 1 Watson | `DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md` | [THEOREM] |
| Anchor 2 Phase G | `DERIV_EMERGENT_COULOMB_GEOMETRIC.md` | [THEOREM] |
| Anchor 3 G* bridge | `DERIV_GSTAR_PF_BRIDGE.md` | [THEOREM] |
| PF Atlas (parallel) | `SPEC_FTD_COMPARATIVE_PHYSICS.md` | [SELECTION] |
| Algebraic spine | `SPEC_ALGEBRAIC_SPINE.md` | [REFERENCE] |
| Engine D1/D2 infrastructure | `engine/tests/benchmark_black_hole_thermo.cpp` | (extension target) |
| LEDGER row | FTD-0105 [HYPOTHESIS] (this doc) | this audit |

---

## 6 · Single-line summary

**Pre-investigation catalog of 4π/8π in Einstein equations + Hawking T + β-period + Schwarzschild horizon area + Bekenstein-Hawking entropy. Phase 4 theoretical analysis reduces the investigation to ONE load-bearing engine measurement: D1 lattice horizon area A_actual / r_h². Standard prediction 4π = 12.566; three ϖ-native candidates {4ϖ, 4G*, G*²·π/2} predict 10.49, 11.84, 13.75 (5-30% separated from standard, distinguishable at engine bootstrap noise floor). All other formulas in scope (§1.1, §1.2, §1.3, §1.5) are downstream of §1.4 OR require D2 (autocorrelation period). Existing benchmark hardcodes A = 4π·r_h² assumption at line 245, ready for `--lemniscatic-mode` extension that measures A directly from voxel-counting. Engine arbitrates; pre-registered protocol pending.**
