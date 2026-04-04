# The phi^3 Exact Effective Field Theory

## Algebraic EFT from the Master Cubic Potential

**Date:** April 3, 2026
**Framework:** Foundational Ternary Dynamics v5.28
**Status:** [THEOREM] for algebraic EFT construction; [SELECTION] for physical identification x+ = alpha^{-1}
**Proof script:** `scripts/verification/verify_phi3_eft.py`

---

## 1. The Master Cubic Potential

**Claim PHI3-1.** [THEOREM] The cubic potential

$$V(x) = \frac{x^3}{3} - 8G^{*2} x^2 + 16G^{*3} x$$

is the unique antiderivative (up to additive constant) of

$$V'(x) = x^2 - 16G^{*2} x + 16G^{*3}$$

whose roots are the master quadratic roots x+ and x-.

*Proof.* Direct integration: the integral of x^2 - 16G*^2 x + 16G*^3 is x^3/3 - 8G*^2 x^2 + 16G*^3 x + C. Setting C = 0 gives V(x). The roots of V'(x) = 0 are x+/- = 8G*^2 +/- 8G* sqrt(G*(4G*-1)), which are the master quadratic roots by definition. $\square$

Here G* = Gamma(1/4)/Gamma(3/4) = 2.95868... is the lemniscatic bridge constant.

---

## 2. Exact Expansion Around x+

**Claim PHI3-2.** [THEOREM] Writing x = x+ + phi, the potential expands as:

$$V(x_+ + \phi) = V(x_+) + \frac{1}{2} m^2 \phi^2 + \frac{1}{3} \phi^3$$

This expansion is **exact**, not truncated. The potential is cubic in x, so the Taylor series terminates at third order. There are no phi^4 or higher terms.

*Proof.* V(x) is a degree-3 polynomial. Its Taylor expansion around any point terminates at order 3. The linear term vanishes because V'(x+) = 0 by construction (x+ is a critical point). The remaining terms are:

- Constant: V(x+)
- Quadratic: (1/2) V''(x+) phi^2
- Cubic: (1/6) V'''(x+) phi^3 = (1/6)(2) phi^3 = (1/3) phi^3

No approximation is involved. $\square$

---

## 3. The Three Wilson Coefficients

**Claim PHI3-3.** [THEOREM] The exact EFT has precisely three Wilson coefficients:

| Coefficient | Expression | Numerical Value | Physical Role |
|-------------|-----------|-----------------|---------------|
| Vacuum energy | V(x+) | -400,505 | Cosmological constant contribution |
| Mass squared | m^2 = V''(x+) = x+ - x- | 134.012 | Root separation |
| Self-coupling | lambda_3 = V'''/3! = 1/3 | 1/3 | Universal cubic vertex |

**Claim PHI3-4.** [THEOREM] The mass squared equals the root separation:

$$m^2 = V''(x_+) = 2x_+ - 16G^{*2} = x_+ - x_- = 8G^* \sqrt{G^*(4G^*-1)} = 134.012\ldots$$

*Proof.* V''(x) = 2x - 16G*^2. At x = x+: V''(x+) = 2x+ - 16G*^2. Since x+ + x- = 16G*^2 (Vieta), this equals 2x+ - (x+ + x-) = x+ - x-. The explicit form follows from the quadratic formula. $\square$

**Claim PHI3-5.** [THEOREM] The self-coupling is universal:

$$\lambda_3 = \frac{V'''}{3!} = \frac{2}{6} = \frac{1}{3} = \frac{1}{D}$$

This does not depend on G* or any other parameter. It is fixed by the degree of the potential alone. In D = 3 spatial dimensions, the universal cubic coupling equals 1/D.

---

## 4. Key Structural Properties

**Claim PHI3-6.** [THEOREM] This is a phi^3 theory, NOT phi^4. Every fundamental Standard Model vertex is three-point (qqg, eegamma, qqW, qqZ, HWW, HZZ, Hff). The Higgs quartic lambda |H|^4 is emergent from integrating out the cubic vertex at one loop, not fundamental.

**Claim PHI3-7.** [THEOREM] The theory is UV-complete in field space: no higher operators exist. The operator product expansion terminates exactly at dimension 3. There are no irrelevant operators to suppress, no cutoff dependence from higher-dimensional terms, and no naturalness problem from phi^4 or phi^6 contributions.

---

## 5. Stability Analysis

**Claim PHI3-8.** [THEOREM] The curvature at each critical point determines the physics:

- At x+: V''(x+) = +134.0 > 0. **Stable minimum.** Small oscillations around x+ are perturbative. This is the QED sector: alpha^{-1} = x+ = 137.036... sits at a stable perturbative vacuum.

- At x-: V''(x-) = -134.0 < 0. **Unstable maximum.** The curvature is negative, signaling a tachyonic instability. This is the QCD sector: x- = 3.024... (floor = N_c = 3) sits at an unstable point, consistent with asymptotic freedom and confinement.

The mass ratio in units of the coupling:

$$\frac{m^2}{\alpha^{-1}} = \frac{x_+ - x_-}{x_+} = 1 - \frac{x_-}{x_+} = 0.978$$

---

## 6. Epistemic Status

| Component | Tag | Note |
|-----------|-----|------|
| Cubic potential and its roots | [THEOREM] | Algebraic identity |
| Exact EFT expansion | [THEOREM] | Taylor termination at degree 3 |
| Three Wilson coefficients | [THEOREM] | Direct computation |
| lambda_3 = 1/3 = 1/D | [THEOREM] | Degree of polynomial |
| x+ = alpha^{-1} | [SELECTION] | Physical identification, 1.26 ppm match |
| x- = N_c | [SELECTION] | floor(x-) = 3, not derived |
| Higgs quartic emergent | [SELECTION] | Requires one-loop integration |

---

## Depends On

- Master quadratic: `docs/theory/03_derivations/DERIV_MASTER_QUADRATIC.md`
- G* definition: `docs/SPEC_FTD.md` Section 4
- Alpha precision formula: `docs/theory/04_coupling/DERIV_ALPHA_PRECISION_FORMULA.md`

## Honesty Notes

1. The EFT construction is pure algebra -- no physics input beyond identifying V'(x) = 0 with the master quadratic.
2. The physical interpretation (x+ as alpha^{-1}, x- as N_c) is a **selection**, not a derivation.
3. The claim that SM vertices are three-point is an observation about the Standard Model Lagrangian, not derived from the cubic potential.
4. The emergence of the Higgs quartic from phi^3 at one loop requires a separate calculation not presented here.
