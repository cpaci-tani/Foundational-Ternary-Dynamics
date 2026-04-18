# Audit: Is `a = 2/D` Forced from Lattice First Principles?

**Date:** 2026-04-17
**Status:** [EMERGENT] — audit result, not a derivation
**Method:** Numerical BZ triple integration via `mpmath.quad` on the 3D cubic lattice
**Script:** [`scripts/exploration/audit_lattice_spacing_a.py`](../../../scripts/exploration/audit_lattice_spacing_a.py)
**Related:** [DERIV_ONE_LOOP_LATTICE_ALPHA.md](DERIV_ONE_LOOP_LATTICE_ALPHA.md) (the primary doc defining the spacing selection)

---

## Abstract

The one-loop lattice correction to the fine-structure constant closes 99.2% of the tree-level 1.26 ppm gap, conditional on selecting the lattice spacing $a = 2/D = 2/3$. The primary doc tags this selection [SELECTION PRINCIPLE] with the geometric motivation "$(D-1)/D$ is the fraction of boundary directions per axis in $D = 3$." This audit tests whether that selection is **forced** by lattice first principles or merely **close to** the value that closes the CODATA gap.

Three questions were addressed:

1. **(Q1)** Find $a_{\mathrm{opt}}$: the value of $a$ that makes the one-loop correction exactly reproduce CODATA. If $a_{\mathrm{opt}} = 2/D$ exactly (modulo higher-loop effects), the selection is justified empirically; if $a_{\mathrm{opt}} \ne 2/D$ meaningfully, the match is approximate.
2. **(Q2)** Among low-height rationals expressible in the base-integer set $\{N_c, N_{\mathrm{base}}, b_3, N_{\mathrm{eff}}, D, \mathrm{BCC}\}$, is $2/3$ the unique closest approximation to $a_{\mathrm{opt}}$, or are there competitors?
3. **(Q3)** Does the correction $\delta x(a)$ have a **special point** (local extremum, inflection) at $a = 2/D$ that would motivate the choice from a symmetry / stability argument?

---

## §1. Setup

The one-loop tadpole correction to the master-quadratic root is

$$\delta x(a) \;=\; -\frac{g \cdot I_1(m^2 a^2)}{m^2 a}, \qquad I_1(m^2_\mathrm{lat}) = \int_\mathrm{BZ} \frac{d^3 k}{(2\pi)^3} \; \frac{1}{\hat k^2 + m^2_\mathrm{lat}}$$

with:
- $g = V''' = 2$ (coupling from the cubic potential)
- $m^2 = x_+ - x_- = 134.012$ (physical mass²)
- $m^2_\mathrm{lat} = m^2 \cdot a^2$ (lattice mass²)
- $\hat k^2 = 4 \sum_\mu \sin^2(k_\mu/2)$ (lattice dispersion)

The **required** $\delta x$ to close the CODATA gap is

$$\delta x_\mathrm{req} = \alpha^{-1}_\mathrm{CODATA} - x_+ \approx -1.72 \times 10^{-4}$$

---

## §2. Findings

### 2.1 Q1 — $a_{\mathrm{opt}}$ vs $2/D$

Numerical bisection on $\delta x(a) = \delta x_{\mathrm{req}}$ with `scipy.integrate.tplquad` at relative tolerance $10^{-10}$:

$$a_{\mathrm{opt}} = 0.66485838\ldots, \qquad \frac{2}{D} = \frac{2}{3} = 0.66666667\ldots$$

**Discrepancy:** $|a_{\mathrm{opt}} - 2/D| / (2/D) = 0.2712\%$.

This reproduces the value $a_{\mathrm{opt}} = 0.66486$ reported in [DERIV_ONE_LOOP_LATTICE_ALPHA.md](DERIV_ONE_LOOP_LATTICE_ALPHA.md) §4, confirming the 0.27% gap is real and not an artifact of numerical precision. The gap implies that substituting $a = 2/D$ into the one-loop formula leaves a residual of $\sim 1.3 \times 10^{-6}$ in $x_+$ (approximately $9.4$ ppb against CODATA) — matching the "9.6 ppb residual after one-loop" claim in the primary doc.

**Reading 1:** $a_{\mathrm{opt}} \ne 2/D$ exactly. $a = 2/D$ is a *nearby rational*, not the optimum.
**Reading 2 (primary doc's position):** the 0.27% gap is absorbed by higher-loop corrections. At two loops, shifts of order $g^2 I_1 \sim 0.06 \times \delta x_{\mathrm{1-loop}}$ are expected — roughly $10^{-5}$, which *is* comparable to the $10^{-6}$ residual. So the claim is plausible but not numerically pinned.

### 2.2 Q2 — Nearest base-integer rational

Enumeration of all rationals $p/q$ with $p, q$ drawn from low-complexity combinations of $\{N_c, N_{\mathrm{base}}, b_3, N_{\mathrm{eff}}, D, \mathrm{BCC}\}$, in the range $[0.55, 0.75]$:

| Rational | Height | $\|r - a_{\mathrm{opt}}\|$ | Note |
|----------|--------|---------------------------|------|
| **2/3** | **3** | **$1.81 \times 10^{-3}$** | **2/D** (claimed) |
| 3/4 | 4 | $8.51 \times 10^{-2}$ | $N_c / N_{\mathrm{base}}$ |
| 3/5 | 5 | $6.49 \times 10^{-2}$ | |
| 5/7 | 7 | $4.94 \times 10^{-2}$ | |
| 4/7 | 7 | $9.34 \times 10^{-2}$ | $N_{\mathrm{base}} / b_3$ |
| 9/13 | 13 | $2.74 \times 10^{-2}$ | |
| 9/14 | 14 | $2.20 \times 10^{-2}$ | |

**Finding:** $2/3$ is uniquely the best rational approximation to $a_{\mathrm{opt}}$ at height $\leq 15$ by a factor of at least **12×** (next-best is $9/14$ at diff $2.2 \times 10^{-2}$). This is a non-trivial rigidity: the closest rational in the base-integer set is the claimed one, and it beats all higher-height alternatives by more than an order of magnitude.

### 2.3 Q3 — Special-point check

Numerical derivative $\partial \delta x / \partial a$ evaluated by central finite difference at $h = 10^{-4}$, sampled near $a = 2/D$:

| $a$ | $\delta x$ | $\partial \delta x / \partial a$ |
|-----|------------|-----------------------------------|
| 0.6167 | $-2.128 \times 10^{-4}$ | $+9.65 \times 10^{-4}$ |
| 0.6467 | $-1.863 \times 10^{-4}$ | $+8.10 \times 10^{-4}$ |
| 0.6617 | $-1.746 \times 10^{-4}$ | $+7.44 \times 10^{-4}$ |
| **0.6667** ($= 2/D$) | $-1.710 \times 10^{-4}$ | $+7.24 \times 10^{-4}$ |
| 0.6717 | $-1.674 \times 10^{-4}$ | $+7.04 \times 10^{-4}$ |
| 0.6867 | $-1.573 \times 10^{-4}$ | $+6.48 \times 10^{-4}$ |
| 0.7167 | $-1.393 \times 10^{-4}$ | $+5.53 \times 10^{-4}$ |

**Finding:** $\delta x(a)$ is monotonically increasing in $a$ over this range, with positive derivative ~$7 \times 10^{-4}$ at $a = 2/D$. **There is no local extremum, zero-crossing, or inflection at $a = 2/D$.** The function is smooth and non-special at this point.

---

## §3. Verdict: **PARTIAL**

Measured against the three scenarios listed at the top of §2:

- **(a) $a_{\mathrm{opt}} = 2/D$ exactly?** NO. There is a real 0.27% gap.
- **(b) Uniquely best base-integer rational?** YES. Among low-height rationals in $\{N_c, N_{\mathrm{base}}, b_3, N_{\mathrm{eff}}, D, \mathrm{BCC}\}$, $2/3$ beats every competitor up to height 15 by more than 12×.
- **(c) Special point of $\delta x(a)$ at $a = 2/D$?** NO. No derivative zero, no extremum, no inflection.

The "forced" hypothesis is **partially supported and partially refuted**. The strongest defensible framing:

> **$a = 2/D$ is the uniquely best low-height base-integer rational approximation to $a_{\mathrm{opt}}$ — but it is not $a_{\mathrm{opt}}$ itself.** The 0.27% residual is real and requires higher-loop corrections (or another mechanism) to absorb. There is no symmetry / stability argument from the one-loop formula alone.

This upgrades $a = 2/D$ from the original "[SELECTION], motivated by the (D−1)/D geometric ratio" to:

> **[SELECTION], uniquely best rational in the base-integer set at height ≤ 15.**

It does **not** upgrade to [THEOREM]. A [THEOREM] tag would require either (i) $a_{\mathrm{opt}} = 2/D$ exactly modulo a quantified higher-loop correction, or (ii) an independent mechanism (symmetry, stability, topology) that selects $a = 2/D$ without referring to CODATA.

---

## §4. Interpretation

**What this does say:** Among all rationals expressible in the base-integer set with height $\leq 15$, $2/3$ is best by a large margin (≥ 12×). If one accepts SP5 (the base-integer set), then $a = 2/D = 2/3$ is the natural rational selection for the one-loop lattice spacing. The [SELECTION] tag is justified — and now *quantitatively* justified, rather than resting only on the "(D−1)/D boundary-to-bulk ratio" argument.

**What this does NOT say:** It does not say $a = 2/D$ is forced by first principles. The 0.27% gap between $a = 2/D$ and $a_{\mathrm{opt}}$ is real, and there is no special point in $\delta x(a)$ at $a = 2/D$. The claim in [DERIV_ONE_LOOP_LATTICE_ALPHA.md](DERIV_ONE_LOOP_LATTICE_ALPHA.md) §4 that the gap "is consistent with higher-loop corrections shifting the optimal spacing" is plausible but speculative until a two-loop calculation actually produces a $\sim 10^{-3}$ shift back toward $2/D$ from $a_{\mathrm{opt}}$.

**What would close the gap to [THEOREM]:**

1. **Two-loop calculation.** Compute $\delta x^{(2)}(a)$ explicitly. If $\delta x^{(1)}(a_{\mathrm{opt}}^{(1)}) + \delta x^{(2)}(2/D) \approx \delta x_{\mathrm{req}}$ (i.e., a_opt shifts toward 2/D when two-loop is included), this would support the higher-loop absorption claim.
2. **Lattice RG fixed-point argument.** Show that $a = 2/D$ is a fixed point of some Wilsonian flow on the cubic lattice.
3. **Derive $\{N_c, N_{\mathrm{base}}, b_3, N_{\mathrm{eff}}, D, \mathrm{BCC}\}$ from first principles.** This closes not just this gap but the broader SP5 circularity (see [BRIDGE_QUADRATIC_PHYSICS.md](../01_reference/BRIDGE_QUADRATIC_PHYSICS.md) §5.4 and [AUDIT_EPISTEMIC_AUDIT.md](../07_assessment/AUDIT_EPISTEMIC_AUDIT.md) Gap 5.5).

---

## §5. Downstream Documentation Updates

This audit's result justifies the following tag/wording updates:

1. [DERIV_ONE_LOOP_LATTICE_ALPHA.md](DERIV_ONE_LOOP_LATTICE_ALPHA.md) Claim 1LA-2 and §6: strengthen the [SELECTION] justification to include "uniquely best rational in the base-integer set at height ≤ 15 (see EXPLR_A_OVER_D_AUDIT.md §2.2)."
2. [SPEC_FTD_COMPLETE_CHAIN.md](../01_reference/SPEC_FTD_COMPLETE_CHAIN.md) §2.1 (a=2/D caveat): note that the selection is the uniquely best base-integer rational, not just a geometric motivation.
3. No change to the top-level status of `a = 2/D` — it remains [SELECTION]. But the *grounds* for that [SELECTION] tag are now stronger.

---

## §6. What the Audit Rules Out (Negative Results)

- **Not a symmetry-point selection.** $\delta x(a)$ has no local extremum at $a = 2/D$. Ruling out at least one class of first-principles derivations (those that would identify $a = 2/D$ via a stationarity condition on the one-loop tadpole).
- **Not a lower-height competitor from a different base.** $2/3$ is best at height 3. Any rational beating it would need height ≥ 16 — and such rationals, if they exist in the base-integer set at all, would be harder to motivate.
- **Not "$a_{\mathrm{opt}}$ happens to equal $2/D$ to numerical precision."** The 0.27% gap is robust at `tplquad` relative tolerance $10^{-10}$.

---

## Document History

- **2026-04-17:** Created with full audit results. Verdict: PARTIAL (b) positive, (a,c) negative. Upgrade to "uniquely best base-integer rational at height ≤ 15." Not upgraded to [THEOREM].

---

## §5. What Would Force a = 2/D Independently

A clean derivation would need one of the following structural arguments:

1. **Symmetry / stability:** an argument that $a = (D-1)/D$ is the unique spacing at which some natural lattice quantity (BZ volume, spectral gap, commutator structure) is stationary or extremal.
2. **Renormalization fixed point:** an argument that the Wilsonian RG flow on the cubic lattice has a fixed point at $a = 2/D$.
3. **Topological:** an argument tying $2/D$ to the Moore-neighborhood fraction structure (SC:FCC:BCC = 6:12:8 → some ratio equals $2/D$) — though the obvious ratios $\{6/26, 8/26, 12/26\}$ do not give $2/3$.
4. **Dimensional analysis:** a dimensional argument that $2/D$ is the unique scale-free combination of lattice primitives.

None of these is currently in place. This audit's role is to narrow the search by ruling out (3) if the specific ratio $2/3$ is not structurally singled out.

---

## Document History

- **2026-04-17:** Created. Results to be filled after running `scripts/exploration/audit_lattice_spacing_a.py`.
