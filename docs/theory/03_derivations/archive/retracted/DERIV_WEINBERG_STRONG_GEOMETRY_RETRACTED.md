# Geometric Reading of Electroweak and Strong Mixing Parameters (ARCHIVED — merged 2026-08-06)

> **Merged into `DERIV_GEOMETRIC_MASS_RATIO_READINGS.md` §2 (2026-08-06),**
> alongside three sibling documents from the same 2026-06-18 batch applying
> the same technique to different targets. Content preserved verbatim there;
> kept here for provenance per the doc-cleanup skill.

**Status:** `[STRUCTURALLY MOTIVATED PARAMETRIC]` **(corrected 2026-08-06 — was `[THEOREM: LATTICE PROJECTION]`)**

> **Correction notice.** This document's 2026-06-18 `[THEOREM: LATTICE PROJECTION]`
> upgrade (commit `24b31016`) is **RETRACTED**, per LEDGER.md FTD-0018/FTD-0020
> (correction of record 2026-06-19, adjudicated): it is a substitution identity, not
> a forcing chain. It fails the FTD-0097/0189 look-elsewhere bar — a competitor
> ratio fits sin²θ_W better (2/9 at 0.31% vs this document's 3/13 at 3.5%) and
> another fits α_s better (2/17 at 0.29% vs 7/59 at 0.63%) — and the standing
> zero-promotion discipline. The document's own citation of "per FTD-0259" as
> justification was bogus: it collided with the real FTD-0259 (Mechanism-α), which
> says nothing about this claim. Two sibling documents from the same 2026-06-18
> batch, `DERIV_LEPTON_MASS_GEOMETRY.md` and `DERIV_BARYON_AND_QUARK_GEOMETRY.md`,
> received this same correction on 2026-07-01; this document was missed by that
> sweep until the 2026-08-06 docs audit caught it. The Moore-layer geometric
> reading of $N_{eff}=13$, the Cartesian/face-diagonal/body-diagonal decomposition,
> and $b_3=7$ from the QCD beta function are real structural content and are
> retained below as *motivation*, not derivation — the canonical tags are
> `[STRUCTURALLY MOTIVATED PARAMETRIC]` for both sin²θ_W = 3/13 (FTD-0018) and
> α_s(M_Z) = 7/59 (FTD-0020).

---

## 1. The Weinberg Angle: 13-Axis Moore Projection

In the standard formulation of FTD, the Weinberg angle was posited as $\sin^2\theta_W = N_c/N_{eff} = 3/13 \approx 0.230769$.

### 1.1 The Degrees of Freedom (The Denominator)
FTD operates on a 3D discrete lattice with a 26-connected Moore neighborhood. Every vector $\vec{v}$ to a neighbor has an antipodal counterpart $-\vec{v}$. Therefore, the number of independent spatial axes (effective degrees of freedom, $N_{eff}$) available for information propagation is exactly:
$$N_{eff} = \frac{26}{2} = 13 \text{ axes}$$

### 1.2 The Cartesian Basis (The Numerator)
The 13 spatial axes uniquely decompose into:
- **3 orthogonal Cartesian axes** (face-centers of the bounding cube: $\pm x, \pm y, \pm z$)
- **6 2D face-diagonal axes** (edge-centers)
- **4 3D body-diagonal axes** (corners)
Total: $3 + 6 + 4 = 13$.

The $SU(3)$ strong force (Color, $N_c = 3$) operates exclusively on the 3 orthogonal Cartesian axes. This forms the baseline continuum geometry of macroscopic space.

### 1.3 Electroweak Unification Geometry
The weak mixing angle defines the projection between the electromagnetic $U(1)_Y$ and the weak $SU(2)_L$ forces. In FTD, the weak force (mediated by chirality flux) propagates across the *entire* 13-axis Moore stencil. Electromagnetism, as the Coulomb limit, is bound by the macroscopic orthogonal Cartesian geometry.

Therefore, the weak mixing angle is the exact geometric projection of the orthogonal Cartesian sub-lattice onto the full Moore neighborhood:
$$\sin^2\theta_W = \frac{\text{Cartesian Axes}}{\text{Total Moore Axes}} = \frac{3}{13} \approx 0.230769$$

*(Standard Model experimental value: 0.2312. Error: 0.19%)*

**Reading:** The factor $3/13$ has a geometric motivation as a projection of lattice anisotropy — but per the correction notice above, a competitor ratio (2/9) fits the CODATA value more closely (0.31% vs 3.5%), so this is not a forcing derivation and the tag is `[STRUCTURALLY MOTIVATED PARAMETRIC]`, not `[THEOREM]`.

---

## 2. The Strong Coupling: Dirac-Moore Fixed Point

The strong coupling at the Z-pole was posited as $\alpha_s(M_Z) = b_3 / (b_3 + 4N_{eff}) = 7/59 \approx 0.1186$. The denominator 59 was heavily criticized in prior audits ("59 is not structural; 2/17 fits better").

### 2.1 The Gluon Anti-Screening Term ($b_3$)
From the standard QCD beta function, $b_3 = \frac{11 N_c - 2 n_f}{3}$. For FTD parameters ($N_c = 3, n_f = 6$), $b_3 = 7$. This integer structurally represents the net anti-screening effect of gluon self-interactions minus quark vacuum polarization.

### 2.2 The Fermionic Vacuum Polarization ($4N_{eff}$)
In a discrete quantum field theory, fermions propagate across the spatial axes. As established, the lattice possesses $N_{eff} = 13$ spatial axes. 
A discrete Dirac spinor fundamentally requires **4 complex components** to support parity and matter-antimatter symmetry. 

Therefore, the total number of fundamental fermionic degrees of freedom available for vacuum polarization across the entire spatial neighborhood is:
$$\text{Dirac Components} \times \text{Spatial Axes} = 4 \times 13 = 52$$

### 2.3 The Topological Fixed Point
At the electroweak unification scale ($M_Z$), the lattice symmetry is fully active. The strong coupling strength represents the thermodynamic partition of the strong force's intrinsic charge ($b_3$) against the total possible vacuum screening pathways available on the lattice.

These pathways consist of the gluon contribution ($b_3$) plus the total fermionic contribution ($4N_{eff}$):
$$\alpha_s(M_Z) = \frac{\text{Gluon Anti-Screening}}{\text{Gluon Anti-Screening} + \text{Total Lattice Fermion Screening}} = \frac{b_3}{b_3 + 52} = \frac{7}{59}$$

**Reading:** The arithmetic 7 + 52 = 59 is exact given the stated inputs, and the numerator $b_3=7$ is structurally motivated by the standard QCD beta function — but per the correction notice above, a competitor ratio (2/17) fits α_s(M_Z) more closely (0.29% vs 0.63%), and the 52 = 4×13 denominator term was not independently forced before the target was known. The tag is `[STRUCTURALLY MOTIVATED PARAMETRIC]`, not `[THEOREM]`.

