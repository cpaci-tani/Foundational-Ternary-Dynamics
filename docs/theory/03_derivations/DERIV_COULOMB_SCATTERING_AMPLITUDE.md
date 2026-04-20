# Tree-Level Coulomb Scattering on the FTD Lattice

## The First Scattering Amplitude from the FTD Lagrangian

**Date:** March 16, 2026
**Status:** [THEOREM] — derived from the FTD Lagrangian with no external input
**Dependencies:** DERIV_QFT_GRT_BRIDGE.md, DERIV_PATH_INTEGRAL_CONSTRUCTION.md

---

## Abstract

We compute the tree-level Coulomb scattering amplitude for two ternary charges on the cubic lattice with no defined boundary, using the Feynman rules derived from the FTD Lagrangian. The result recovers Rutherford scattering in the long-wavelength regime ($|\mathbf{q}| \ll \pi$) and predicts cubic-anisotropic corrections at the Planck scale.

---

## 1. Feynman Rules [THEOREM]

From DERIV_QFT_GRT_BRIDGE.md (Theorems 1.1-1.5):

| Element | Expression | Origin |
|---------|-----------|--------|
| Photon propagator | $G_L(\mathbf{k}) = 1/\lambda(\mathbf{k})$ | Lattice Green's function |
| $\lambda(\mathbf{k})$ | $2(3 - \cos k_x - \cos k_y - \cos k_z)$ | Lattice Laplacian eigenvalue |
| Vertex factor | $g_c = \sqrt{\alpha}$ | State-flux coupling |
| External legs | $s = \pm 1$ | Ternary state of manifested charge |
| Momentum domain | $\text{BZ} = [-\pi, \pi]^3$ | Compact Brillouin zone (UV-finite) |

where $\alpha = 1/x_+ = 1/137.036$ from the master quadratic.

---

## 2. The Amplitude [THEOREM]

Two ternary charges $q_1 = +1$ and $q_2 = -1$ exchange one virtual flux quantum with momentum transfer $\mathbf{q} = \mathbf{p}_1 - \mathbf{p}_1'$. The tree-level amplitude is:

$$\boxed{\mathcal{M}(\mathbf{q}) = \frac{-\alpha}{2(3 - \cos q_x - \cos q_y - \cos q_z)}}$$

This is exact on the lattice. No approximations, no regularization, no renormalization needed. The compact Brillouin zone provides automatic UV finiteness.

---

## 3. The Differential Cross-Section [THEOREM]

In the non-relativistic Born approximation:

$$\frac{d\sigma}{d\Omega} = \frac{m^2 \alpha^2}{4\pi^2\,\lambda(\mathbf{q})^2}$$

where $\lambda(\mathbf{q}) = 2(3 - \cos q_x - \cos q_y - \cos q_z)$ and $|\mathbf{q}| = 2|\mathbf{p}|\sin(\theta/2)$.

---

## 4. Long-Wavelength Regime: Rutherford Recovery [THEOREM]

For $|\mathbf{q}| \ll \pi$ (energies far below the Planck scale):

$$\lambda(\mathbf{q}) \approx q_x^2 + q_y^2 + q_z^2 = q^2$$

$$\mathcal{M}(\mathbf{q}) \to \frac{-\alpha}{q^2}$$

$$\frac{d\sigma}{d\Omega} \to \frac{\alpha^2}{16E^2 \sin^4(\theta/2)}$$

This is the **Rutherford scattering formula**, exactly as derived in standard QED. The FTD lattice reproduces classical electrodynamics in the long-wavelength regime ($|\mathbf{q}| \ll \pi$).

---

## 5. Lattice Corrections: Planck-Scale Predictions [THEOREM]

At finite momentum, expanding $\lambda(\mathbf{q})$ to next order:

$$\lambda(\mathbf{q}) = q^2 + \frac{1}{12}(q_x^4 + q_y^4 + q_z^4) + O(q^6)$$

The lattice correction to the cross-section:

$$\frac{d\sigma_{\text{lattice}}}{d\sigma_{\text{Rutherford}}} = \left[1 + \frac{q_x^4 + q_y^4 + q_z^4}{12\,q^2} + \ldots\right]^{-2}$$

For isotropic scattering ($q_x = q_y = q_z = q/\sqrt{3}$):

$$\frac{d\sigma_{\text{lattice}}}{d\sigma_{\text{Rutherford}}} \approx 1 - \frac{q^2}{18} + O(q^4)$$

**Novel predictions:**

1. **Cubic anisotropy:** The correction depends on the direction of $\mathbf{q}$ relative to the lattice axes. This breaks SO(3) rotational symmetry to the cubic group O_h at the Planck scale.

2. **Suppression at high momentum:** The lattice propagator falls off faster than $1/q^2$ at large $q$, suppressing the cross-section relative to Rutherford. At $q \sim 1$ (Planck energy), the suppression is $\sim 5\%$.

3. **At the BZ corner** ($\mathbf{q} = (\pi,\pi,\pi)$): $\lambda = 12$, giving $\mathcal{M} = -\alpha/12 \approx -6 \times 10^{-4}$. The amplitude is finite and nonzero at maximum momentum transfer — no ultraviolet divergence.

**Observability:** At accessible energies ($q \sim 10^{-19}$ in lattice units), the lattice correction is $\sim 10^{-38}$ — utterly undetectable. The predictions are structural, not phenomenological.

---

## 6. Ward Identity [THEOREM]

The lattice Ward identity (DERIV_QFT_GRT_BRIDGE.md, Theorem 1.5) guarantees:

$$\hat{q}_\mu \cdot \mathcal{M}^{\mu} = 0$$

where $\hat{q}_\mu = \sin(q_\mu)$. This ensures gauge invariance and current conservation at every lattice vertex, exactly as in continuum QED.

---

## 7. What This Establishes

This is a **genuine scattering amplitude computed from the FTD Lagrangian:**

- The coupling $\alpha$ comes from the master quadratic (not from experiment)
- The propagator comes from the lattice Laplacian (not from continuum field theory)
- The Ward identity holds exactly (not as an approximation)
- The Rutherford formula is recovered in the long-wavelength regime $|\mathbf{q}| \ll \pi$ (not assumed)
- The UV finiteness is automatic (not imposed by a regulator)

Every ingredient traces back to Axiom Zero: state $s \in \{-1,0,+1\}$ and integer-coordinate lattice site $x$ on the cubic lattice with no defined boundary.

---

## References

- DERIV_QFT_GRT_BRIDGE.md — Feynman rules from lattice (03_derivations)
- DERIV_PATH_INTEGRAL_CONSTRUCTION.md — Partition function (03_derivations)
- DERIV_MASTER_QUADRATIC_GAP_EQUATION.md — $\alpha$ from the gap equation (03_derivations)
- Creutz, M. *Quarks, Gluons and Lattices*, Cambridge University Press, 1983
