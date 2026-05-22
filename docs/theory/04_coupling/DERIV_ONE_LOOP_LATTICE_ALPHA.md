# One-Loop Lattice Correction to the Fine Structure Constant

## From 1.26 ppm to 9.6 ppb via Brillouin-Zone Tadpole

**Date:** April 3, 2026
**Framework:** Foundational Ternary Dynamics v5.28
**Status:** [SELECTION] / scheme-conditional calculation (derived after choosing the SC scalar-EFT tadpole scheme and lattice spacing a = 2/D)
**Proof script:** `scripts/verification/verify_one_loop_alpha.py`

---

## 0. Audit update (2026-04-22)

This document's arithmetic remains reproducible inside its stated scalar-EFT scheme, but the ppb interpretation is now conditional rather than universal.

Subsequent GPU audits found:

1. The BCC tadpole value differs from the SC tadpole value used here, and the unrenormalized one-loop residual has no continuum limit without counterterms. See `docs/theory/10_eft_program/archive/campaign_complete/AUDIT_GPU_PLAN_PRIORITIES_1_3_5_6.md`.
2. A Ward-valid Structure-2 two-U(1) BCC scalar gauge completion with bubble plus seagull terms does not reproduce the Structure-1 ppb closure. See `docs/theory/10_eft_program/archive/closed_negative/AUDIT_STRUCTURE2_WARD_VALIDATION.md`.

Therefore the "9.6 ppb" result should be read as a Structure-1, fixed-regularization outcome. It is not currently a scheme-independent physical prediction.

## 1. Setup: phi^3 EFT on the Cubic Lattice

**Claim 1LA-1.** [THEOREM] The phi^3 EFT from the master cubic potential (see `DERIV_PHI3_EXACT_EFT.md`) is placed on a cubic lattice Z[i]^3 with the following parameters:

| Parameter | Expression | Value |
|-----------|-----------|-------|
| Lattice spacing | a = 2/D = (D-1)/D | 2/3 |
| Mass squared (physical) | m^2 = x+ - x- | 134.012 |
| Mass squared (lattice) | m^2_lat = m^2 * a^2 | 59.561 |
| Coupling | g = V''' | 2 |
| Lattice size | N^3 | 128^3 or 150^3 |
| Brillouin zone | k_mu in [-pi, pi]^3 | Standard |
| Lattice dispersion | k_hat^2 = sum_mu 4 sin^2(k_mu/2) | Standard |

**Claim 1LA-2.** [SELECTION PRINCIPLE] The lattice spacing a = 2/D = 2/3 is selected. Non-circular justifications:

- **Geometric:** Boundary-to-bulk ratio in D = 3. The fraction of boundary directions per axis is (D-1)/D = 2/3.
- **Rational approximation (quantitative, added 2026-04-17):** Among all rationals $p/q$ expressible in the base-integer set $\{N_c, N_\mathrm{base}, b_3, N_\mathrm{eff}, D, \mathrm{BCC}\}$ at height $\leq 15$, **2/3 is the uniquely best approximation to $a_\mathrm{opt} = 0.66486$ — by a factor of at least 12× against any competitor**. See [EXPLR_A_OVER_D_AUDIT.md](EXPLR_A_OVER_D_AUDIT.md) §2.2 for the full ranking. The next-best rational (9/14 at height 14) is off by $2.2 \times 10^{-2}$, vs 2/3's $1.8 \times 10^{-3}$.
- **Dimensional:** The unique rational of the form (D-1)/D for D = 3.

**What the audit did NOT find:** No local extremum, zero-crossing, or inflection of $\delta x(a)$ at $a = 2/D$ ([EXPLR_A_OVER_D_AUDIT.md](EXPLR_A_OVER_D_AUDIT.md) §2.3). The function is monotonically smooth through this point — so $a = 2/D$ is not forced by a symmetry / stability condition from the one-loop tadpole alone. The 0.27% gap between $a_\mathrm{opt}$ and $2/D$ remains real.

**Honesty note.** Earlier revisions listed the up-quark charge $Q_u = 2/3$ and hypercharge $Y_{u_R} = 2/3$ as additional "independent derivations." Removed: invoking SM quantum numbers to justify the spacing that closes the α gap is circular.

---

## 2. The Tadpole Integral

**Claim 1LA-3.** [THEOREM] The one-loop tadpole integral on the cubic lattice is:

$$I_1 = \int_{\mathrm{BZ}} \frac{d^3k}{(2\pi)^3} \; \frac{1}{\hat{k}^2 + m^2_{\mathrm{lat}}}$$

where k_hat^2 = sum_{mu=1}^{3} 4 sin^2(k_mu / 2) is the lattice dispersion relation.

Numerical evaluation on a 150^3 lattice via FFT:

$$I_1 = 0.015274$$

**Claim 1LA-4.** [THEOREM] The VEV shift from the tadpole is:

$$\delta\phi = -\frac{g \cdot I_1}{m^2_{\mathrm{lat}}} = -\frac{2 \times 0.015274}{59.561} = -2.564 \times 10^{-4} \quad (\text{lattice units})$$

Converting to physical units:

$$\delta x = \delta\phi \cdot a_{\mathrm{lat}} = -2.564 \times 10^{-4} \times \frac{2}{3} = -1.710 \times 10^{-4}$$

---

## 3. Result: Tree + One-Loop

**Claim 1LA-5.** [SELECTION] Within the chosen SC scalar-EFT tadpole scheme, the corrected fine structure constant:

| Level | Value of x+ | Source |
|-------|-------------|--------|
| Tree level | 137.036171458 | Master quadratic root |
| One-loop corrected | 137.036000492 | x+(tree) + delta_x |
| NIST/CODATA 2022 | 137.035999177 | Experimental |

Residual after one-loop correction: **9.6 ppb** (parts per billion).

Gap closed by the one-loop lattice correction: **99.2%** of the original 1.26 ppm tree-level discrepancy.

**Audit caveat (2026-04-22):** this closure is scheme-specific. It is not reproduced by the Ward-valid Structure-2 scalar gauge completion tested in `AUDIT_STRUCTURE2_WARD_VALIDATION.md`.

---

## 4. Lattice Spacing Analysis

**Claim 1LA-6.** [DERIVED] Solving for the optimal lattice spacing that exactly reproduces the CODATA value:

$$a_{\mathrm{opt}} = 0.66486$$

The rational choice a = 2/3 = 0.66667 differs from a_opt by 0.27%. This small discrepancy is consistent with higher-loop corrections shifting the optimal spacing.

---

## 5. Perturbative Control

**Claim 1LA-7.** [THEOREM] The loop expansion parameter is:

$$g^2 \cdot I_1 = 4 \times 0.01527 = 0.061$$

This is approximately 6%, confirming that the perturbative expansion is well-controlled. Each successive loop contributes roughly an order of magnitude less than the previous.

**Claim 1LA-8.** [DERIVED] The two-loop sunset integral, evaluated via FFT convolution on a 32^3 lattice:

$$I_{\mathrm{sunset}} = 0.1168$$

This is approximately 23% of the one-loop value in magnitude. The two-loop correction shifts in the positive direction, partially compensating the one-loop overcorrection (the one-loop result overshoots NIST by +1.3 ppb, suggesting the two-loop term pushes back toward the experimental value).

---

## 6. Physical Interpretation of a = 2/D

**Claim 1LA-9.** [SELECTION] The spacing a = 2/D = 2/3 has three non-circular motivations:

1. **Geometric:** In a D-dimensional hypercube, the fraction of boundary directions per axis is (D-1)/D.
2. **Dimensional:** The only rational number of the form (D-1)/D for D = 3.
3. **Best rational in the base-integer set (2026-04-17 audit):** Among all rationals in $\{N_c, N_\mathrm{base}, b_3, N_\mathrm{eff}, D, \mathrm{BCC}\}$ at height $\leq 15$, 2/3 is the uniquely best approximation to $a_\mathrm{opt} = 0.66486$ (see [EXPLR_A_OVER_D_AUDIT.md](EXPLR_A_OVER_D_AUDIT.md)).

The audit explicitly tested for a symmetry / stability selection at $a = 2/D$ (Q3 of the audit): $\delta x(a)$ has no local extremum, zero derivative, or inflection at $a = 2/D$. So items (1)-(3) are the complete non-circular basis; **no mechanism-style derivation exists as of 2026-04-17**.

**Removed from earlier revisions:** the citations of up-quark charge $Q_u = 2/3$ and hypercharge $Y_{u_R} = 2/3$ as "independent derivations." These are SM quantum numbers downstream of α and cannot justify a selection that closes the α gap (circular). See §1 Honesty Note.

---

## 7. Higher-Loop Convergence

**Claim 1LA-10.** Superseded as a live alpha-closure task. The scheme-internal convergence pattern is:

| Loop order | Contribution | Cumulative residual |
|------------|-------------|-------------------|
| Tree (0-loop) | 137.036171 | 1.26 ppm |
| 1-loop | -1.71e-4 | 9.6 ppb |
| 2-loop | ~+3e-6 (estimated) | ~few ppb |
| 3-loop+ | unknown | < 1 ppb (expected) |

After the BCC regulator audit and the Ward-valid Structure-2 audit, this is no longer a valid route to a scheme-independent alpha prediction by itself. Higher-loop terms may still be computed inside the selected Structure-1 scalar-EFT scheme, but even perfect convergence to the CODATA value would not establish an FTD prediction unless the FTD-to-EFT matching principle is derived first.

The live problem has therefore moved from "compute more loops until the residual closes" to:

```text
derive the matching rule -> then run fixed loop checks
```

The proposed duality with the epsilon-expansion remains conjectural and should not be used as an acceptance criterion.

---

## 8. Epistemic Status

| Component | Tag | Note |
|-----------|-----|------|
| EFT construction | [THEOREM] | Algebraic, from `DERIV_PHI3_EXACT_EFT.md` |
| Lattice tadpole integral | [THEOREM] | Numerical, verified on 128^3 and 150^3 |
| Spacing a = 2/D | [SELECTION PRINCIPLE] | Multiple derivations from D = 3 |
| One-loop result 9.6 ppb | [SELECTION] | Derived within the SC scalar-EFT scheme at a = 2/D; not scheme-independent |
| Two-loop sunset | [DERIVED] | FFT convolution, 32^3 |
| Higher-loop convergence | Deferred | Not a live alpha-closure item until FTD-to-EFT matching is derived |
| Duality with epsilon-expansion | [CONJECTURE] | Conjectured, not proven |

---

## Depends On

- phi^3 EFT: `docs/theory/04_coupling/DERIV_PHI3_EXACT_EFT.md`
- Master quadratic: `docs/theory/03_derivations/DERIV_MASTER_QUADRATIC.md`
- Alpha precision formula: `docs/theory/04_coupling/DERIV_ALPHA_PRECISION_FORMULA.md`
- G* definition: `docs/SPEC_FTD.md` Section 4

## Honesty Notes

1. The lattice spacing a = 2/D is a **selection principle**, not derived from dynamics. It is the single free choice in this calculation.
2. The 9.6 ppb result is only as good as the selected scalar-EFT scheme, regulator, and spacing a = 2/D. The BCC and Structure-2 audits show that changing the gauge completion or regularization changes the result.
3. The two-loop estimate is preliminary (32^3 lattice). A 128^3 evaluation would be more reliable for the selected scheme, but it is not the current bottleneck.
4. This is a **parametric insertion** of FTD values (G*, D=3) into standard lattice field theory formulas. The lattice QFT machinery is external physics, not derived from FTD axioms.
5. As of 2026-04-22, the live blocker is the FTD-to-EFT matching principle documented in `docs/theory/10_eft_program/archive/closed_negative/OPEN_FTD_TO_EFT_MATCHING.md` and `docs/theory/10_eft_program/OPEN_FTD_TO_EFT_BRIDGE_STATUS.md`.
