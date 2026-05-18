# K_comp Baryon Composition Correction: Circle-to-Sphere Attempt

## [CLOSED NEGATIVE] — The Numerical Premise Does Not Survive Verification

**Version:** 1.0
**Date:** 2026-04-23
**Framework:** Foundational Ternary Dynamics v5.32
**Status:** [CLOSED NEGATIVE]. The manuscript's claim that $K_{\text{comp}} = m_e/\pi$ brings the proton/neutron/delta formulas to 0.4 keV / <1 eV / 0.03% agreement with experiment is **numerically false at the stated precision**. The quantity the formulas *actually* need to match experiment differs from $m_e/\pi$ by ~0.95% (~1.5 keV). Because the target itself is not really $1/\pi$, the geometric "circle-to-sphere" intuition has nothing firm to land on, and the derivation program as posed is ill-posed.

**Naming distinction enforced.** This $K_{\text{comp}}$ (baryon mass offset, MeV-scale) is a *different quantity* from the $K_{\text{comp}} = K_B = 0.511$ of `DERIV_KCOMP_VOLUMETRIC_SHELL.md` (manifestation threshold / shell energy budget). The manuscript overloaded the symbol. This document concerns *only* the baryon offset; the volumetric-shell $K_{\text{comp}}$ is unaffected.

**Depends on:** Master-quadratic identification ($x_+ = 1/\alpha$, $x_- = N_c$), Moore-layer decomposition, baryon-mass formulas in `dissemination/manuscript/src/chapters/1.10b-master-quadratic-derivation.qmd:660-716`.

---

## 1. Inputs declared (Calibration Hygiene Rule 1)

| Symbol | Value | Units | Source | Epistemic tag |
|--------|-------|-------|--------|---------------|
| $m_e$ | 0.51099895000 | MeV/$c^2$ | CODATA 2022 | External anchor |
| $\alpha$ | 1/137.035999177 | dimensionless | Master quadratic $x_+$ | [STRONGLY MOTIVATED CONJECTURE] |
| $\varphi$ | $(1+\sqrt 5)/2$ | dimensionless | Golden ratio | Pure math |
| $M_p^{\text{exp}}$ | 938.27208816 | MeV/$c^2$ | CODATA 2022 | External anchor |
| $M_n^{\text{exp}}$ | 939.56542052 | MeV/$c^2$ | CODATA 2022 | External anchor |
| $M_\Delta^{\text{exp}}$ | 1232.0 ± 2 | MeV/$c^2$ | PDG | External anchor |

Target: *dimensionless* ratio $K_{\text{comp}}/m_e$. Compute it in MeV, then divide. No $m_e$-in-other-units smuggling.

---

## 2. The numerical audit (pre-derivation sanity check)

The request specification explicitly demands: "is this really $1/\pi$, or is it e.g. (something)/π or $1/(\pi + \text{small})$?" I computed this *first*, before attempting any geometric derivation.

### 2.1 Proton

$$M_p^{\text{geo}} \equiv \left(\tfrac{13}{\alpha} + 55\right) m_e = 938.433214\ \text{MeV}.$$

To reproduce the experimental $938.27208816$ MeV, the correction required is

$$K_p^{\text{needed}} = M_p^{\text{geo}} - M_p^{\text{exp}} = 0.161126\ \text{MeV}, \qquad \frac{K_p^{\text{needed}}}{m_e} = 0.315316.$$

By comparison, $1/\pi = 0.318310$. The ratio

$$\frac{K_p^{\text{needed}}/m_e}{1/\pi} = 0.99059.$$

That is a **0.95 % gap**, not the ppm-level match the chapter implies.

Propagated to the final mass: using $K_{\text{comp}} = m_e/\pi$ predicts $938.270558$ MeV, so the error is $-1.530$ keV — **four times larger** than the chapter's stated 0.4 keV.

### 2.2 Neutron

Same computation for the neutron with $(\varphi^2 - 12\alpha)m_e$ correction:

$$K_n^{\text{needed}}/m_e = 0.314793, \qquad M_n^{\text{pred}} - M_n^{\text{exp}} = -1797\ \text{eV}.$$

The chapter claims "<1 eV". **The true error is three orders of magnitude larger.**

### 2.3 Delta

$$K_\Delta^{\text{needed}} = M_\Delta^{\text{geo}} - M_\Delta^{\text{exp}} = -0.17981\ \text{MeV}.$$

**The sign is opposite** to the proton/neutron case. Using $K_{\text{comp}} = +m_e/\pi$ gives a correction in the wrong direction. Within the PDG error band ±2 MeV this hides, but any derivation claiming a *single* universal $K_{\text{comp}}$ to be subtracted from all three formulas is contradicted: the Δ wants a +$m_e/\pi$-sized *addition*, not a subtraction.

### 2.4 Summary of the pre-derivation audit

| Particle | True $K/m_e$ | $1/\pi$ | Deviation | Chapter's claimed accuracy | Actual error |
|----------|--------------|---------|-----------|---------------------------|--------------|
| $p$ | $+0.315316$ | $0.318310$ | $-0.95\%$ | 0.4 keV | $-1.53$ keV |
| $n$ | $+0.314793$ | $0.318310$ | $-1.11\%$ | <1 eV | $-1.80$ keV |
| $\Delta$ | $\mathbf{-0.351872}$ | $0.318310$ | wrong sign | 0.03% | 0.19% |

**Conclusion of §2:** The premise "$K_{\text{comp}} = m_e/\pi$ fits with the precision claimed" is false. The baryon mass formulas reported in the manuscript chapter do not reach the precision the chapter claims, and the universal constant $K_{\text{comp}}$ required is not $m_e/\pi$ for any of the three particles.

**Secondary check — the CLAUDE.md-level claim.** The text says $m_p/m_e = N_{\text{eff}}/\alpha + N_{\text{base}}\cdot N_{\text{eff}} + N_c = 1836.47$, and subtracting $1/\pi = 0.3183$ gives 1836.15, matching 1836.15267 "exactly". Checking: $13/\alpha + 52 + 3 = 1781.47 + 55 = 1836.47$. True. $1836.47 - 0.318 = 1836.152$ vs experiment $1836.15267$. The gap closes to the ppm level at *dimensionless-ratio precision* — but that is precisely because $m_p/m_e$ is reported to only 5 digits ($N_{\text{eff}}/\alpha$ itself carries only ~1 ppm precision from $\alpha$). At MeV precision the formula fails by keV, as §2.1 shows. The ppm match is load-bearing only at the level of the first 5 sig-figs of the ratio, where $1/\pi$ and the true offset $0.315316$ **disagree in the 3rd sig fig**.

---

## 3. Why the "circle-to-sphere" mechanism cannot rescue this

Having shown the target is not $1/\pi$, I document briefly why none of the six suggested mechanisms was pursued to a full derivation.

### (1) Moore-shell self-field integration
Would need to produce the dimensionless number $0.31532$ from an angular/volumetric integral on the 26-site Moore neighborhood. The only $\pi$-dependent ratios that arise naturally on the cubic lattice are integer-coefficient combinations of Watson integrals ($W_{SC}, W_{FCC}, W_{BCC}$). $W_{BCC} = G^{*2}/(2\pi)$ carries a $1/\pi$ by theorem. But the self-field energy integral over the BCC shell does not produce $1/\pi$ bare; it produces $G^{*2}/(2\pi) \cdot (\text{shell-structure factor})$, and that factor is *not* $m_e^2 G^{*2\, -1}$. **No clean path.**

### (2) Three-quark worldline loop closure
The intuition (1D triangular worldline → 3D compact ball) could plausibly carry a $1/\pi$ factor from comparing perimeter $2\pi r$ against surface $4\pi r^2$ or volume $\tfrac{4}{3}\pi r^3$. But:
- A path-integral loop correction for *three* identical confined worldlines on the FTD lattice is gauge-sum-over-26 at each tick. Its leading $O(\alpha^0)$ contribution is the bare color-singlet energy, already counted in $N_c$ and $N_{\text{base}}$.
- The sub-leading contribution is $O(\alpha)$, i.e. $\sim \alpha m_e \approx 0.0073\, m_e \approx 3.7\, \text{keV}$ — two orders of magnitude *too small* to be the 161 keV correction needed.
- Generating a *bare* dimensionless $1/\pi$-sized correction (no $\alpha$ suppression) from a loop path integral would require the loop to be topologically *non-trivial* on the BCC sub-lattice. There is no such invariant with the right size.

### (3) BCC-shell angular average
BCC has 8 corner directions; angular average over the 8-corner set is a sum of 8 terms, not a $4\pi$ steradian integral. A spherical harmonic $Y_{\ell m}$ averaged over the cube corners vanishes for odd $\ell$ by symmetry and gives rational numbers (with no $\pi$) for even $\ell$. **Cannot produce $1/\pi$.**

### (4) Watson-integral residue projection
$G^{*2} = 2\pi W_{BCC}$ gives $W_{BCC}/G^{*2} = 1/(2\pi)$. Doubling to get $1/\pi$ requires a factor of 2 from somewhere structural. None of the documented projections (onto BCC, onto stella-octangula parity, onto any generation plane) produce that factor while also producing the right *overall* $m_e$ scale without introducing $\alpha$. The scale-setting is fundamentally wrong: Watson ratios are pure numbers of order 1, and multiplying by $m_e$ produces corrections at the $m_e$ scale, not at $m_e/\pi \approx 0.163$ MeV. Even if the factor were obtained, it would collide with the fact (§2) that $1/\pi$ is not the target.

### (5) One-loop Lagrangian correction
Same objection as (2): bare-coupling QED/QCD loop corrections to bound states are $O(\alpha) m_e$ or $O(\alpha_s) m_e$, wrong size.

### (6) Solid-angle reduction
No construction of solid angles / perimeters on the D=3 Moore lattice yields $1/\pi$ cleanly — all such ratios pick up the Watson-integral structure (which routes via $1/(2\pi)$) or polyhedral rational fractions (no $\pi$).

**Pattern:** the successful FTD derivations of baryon-scale corrections go via $\alpha$ ($\sim 0.0073$) or via integer/rational factors (no $\pi$). A *bare* $1/\pi$ at the $\sim 0.3\, m_e$ level is not natural in the lattice structure. This is consistent with the audit's finding that the true correction is *not* $1/\pi$.

---

## 4. What the manuscript chapter was actually fitting

The baryon formulas

$$M_p = (13/\alpha + 55) m_e - K, \qquad M_n = M_p^{\text{geo}} + (\varphi^2 - 12\alpha) m_e - K$$

contain adjustable integer coefficients ($13$, $55$, ($-12$ on $\alpha$)) and a free $\varphi^2$ (chosen post-hoc). With that many handles, fitting a single "correction" $K \approx 0.16$ MeV is not surprising. The identification $K = m_e/\pi$ is a post-hoc *labeling* of a number that happens to be within 1% of $1/\pi$, not a genuine derivation target.

The 174 ppm residual in $m_p/m_e = N_{\text{eff}}/\alpha + N_{\text{base}} N_{\text{eff}} + N_c$ really is a load-bearing gap that needs closure. But it is not closed by $1/\pi$; the true offset is $0.31532$, which has no obvious FTD-primitive origin.

---

## 5. Recommended disposition

1. **Strike $K_{\text{comp}} = m_e/\pi$ from the manuscript.** Replace with: "The residual 174-ppm gap between $N_{\text{eff}}/\alpha + N_{\text{base}} N_{\text{eff}} + N_c$ and $m_p/m_e^{\text{exp}}$ is presently [OPEN]. A $0.315\, m_e$ correction fits the proton within keV precision but no first-principles derivation has been found."
2. **Correct the chapter's accuracy claims** (0.4 keV → 1.5 keV for the proton; <1 eV → 1.8 keV for the neutron; flag that $\Delta$ wants an *additive* correction of opposite sign).
3. **Rename the volumetric-shell quantity** to $K_{\text{shell}}$ or $K_B$ to eliminate the symbol collision with the defunct baryon offset.
4. **Remove the claim from CLAUDE.md** that "$-1/\pi$ closes a 174-ppm gap" — it does not, at the precision claimed. The ppm number there relies on truncation of $m_p/m_e$ to 5 sig figs.
5. Add LEDGER row FTD-0053 [CLOSED NEGATIVE]: "Baryon $K_{\text{comp}} = m_e/\pi$ conjecture." Point to this document.

---

## 6. What would a real derivation need?

For a future attempt to be worth pursuing, three conditions must hold:

1. **Fix the target first.** The actual needed correction is $0.31532\, m_e$ ± (CODATA uncertainty on $\alpha$, $m_e$). Determine whether this number is the *true* target or whether the mass formula itself should be adjusted (different integers, different $\alpha$-power, etc.) so that an integer or $\pi$-involving ratio emerges at experimental precision.
2. **Consistency across $p$, $n$, $\Delta$.** A universal $K$ must apply with the *same sign and magnitude* to all three. The sign flip for $\Delta$ rules out "$-K$ subtracted from every baryon."
3. **FTD-primitive origin.** The correction must be expressible as a Moore-shell polyhedral invariant, a Watson-integral ratio, or an $O(\alpha^n)$ loop correction with correct scale. A bare $1/\pi$ has no such origin.

Condition (1) alone invalidates the premise of this derivation task; conditions (2–3) kill the mechanism even if (1) could be finessed.

---

## 7. Verification artifacts

- Numerical audit script output above; independently reproducible via
  ```python
  m_e = 0.51099895; alpha = 1/137.035999177
  M_p_geo = (13/alpha + 55)*m_e  # 938.433214
  (M_p_geo - 938.27208816)/m_e   # 0.315316, NOT 1/pi = 0.318310
  ```
- Manuscript claim location: `dissemination/manuscript/src/chapters/1.10b-master-quadratic-derivation.qmd:660-716`.
- Naming collision: compare `docs/theory/03_derivations/DERIV_KCOMP_VOLUMETRIC_SHELL.md` (different object: $K_B = 0.511$ MeV, self-field energy budget).

---

## 8. One-line summary

The proposed $K_{\text{comp}} = m_e/\pi$ baryon-mass correction is numerically a **~1% approximation** to the true offset, not a tight identity, and no FTD-primitive mechanism (shell integration, worldline path-integral, Watson projection, polyhedral average, loop correction, solid-angle reduction) naturally produces either the approximate or the exact value. The circle-to-sphere intuition cannot be rigorized because the target it was supposed to reproduce is not, in fact, $1/\pi$.
