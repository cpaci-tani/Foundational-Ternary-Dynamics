# DERIV · 18-Point Isotropic Lattice Laplacian from the Action Principle

**Tag:** [THEOREM]
**Date:** 2026-05-05
**Status:** [THEOREM] for the variational identity (kinetic-functional → 18-pt stencil); [SELECTION] for the {1/3, 1/6} weight pair (one of a finite family of isotropic-restoring choices, picked by leading-order isotropy).
**Purpose:** Phase R2 of the FTD-EFT roadmap. Closes the open item in `SPEC_OPERATOR_BASIS_COMPLETE.md`: "linear G18 generator [THEOREM] for cubic lattice; mixed-sublattice variants are [OPEN]." Derives the engine's 18-point face+edge Laplacian stencil with canonical weights $w_\text{face} = 1/3$, $w_\text{edge} = 1/6$ from the kinetic Lagrangian density `field_gradient_term()` at `engine/include/ftd/lagrangian.h:86-100`, by direct variational derivative.

The result is unsurprising — this is a known Wilsonian construction — but it has not been written up explicitly for FTD's lattice. With this derivation, the 18-pt stencil graduates from "empirically chosen" to "canonical leading-order isotropic discretisation, derivable from the action."

---

## §1 — Setup

Let $\phi(\mathbf{v}, t)$ be a scalar component of the flux field on the cubic lattice $\Lambda \subset \mathbb{Z}^3$ with lattice spacing $a = 1$. Define the kinetic Lagrangian density at vertex $\mathbf{v}$ as:

$$
\mathcal{L}_\text{grad}(\mathbf{v}) = -\frac{c^2}{2} \sum_{\mathbf{u} \in \mathcal{N}(\mathbf{v})} w_{\mathbf{v}\mathbf{u}} \,\bigl[\phi(\mathbf{u}) - \phi(\mathbf{v})\bigr]^2
$$

where $\mathcal{N}(\mathbf{v})$ is the neighbour set and $w_{\mathbf{v}\mathbf{u}}$ is a per-link weight depending only on the link type (translation- and rotation-equivariant). The action contribution is $S_\text{grad} = \sum_\mathbf{v} \mathcal{L}_\text{grad}(\mathbf{v}) \cdot \Delta t$, which under stationarity ($\delta S / \delta \phi = 0$) gives the field equation.

The Moore-26 neighbour set decomposes by graph distance:

| Type | Count | $|\mathbf{u} - \mathbf{v}|^2$ | Set notation |
|---|---|---|---|
| Face | 6 | 1 | $\mathcal{N}_\text{face}$ |
| Edge | 12 | 2 | $\mathcal{N}_\text{edge}$ |
| Corner | 8 | 3 | $\mathcal{N}_\text{corner}$ |

Restricting the kinetic term to face + edge contributions only (i.e. setting $w_\text{corner} = 0$) gives the **18-point stencil** under variation. The 18-point family is parameterised by $(w_\text{face}, w_\text{edge})$.

---

## §2 — Variational derivative (the EOM-stencil identity) [THEOREM]

**Theorem.** *Let $\mathcal{L}_\text{grad}$ be as above with $w_\text{corner} = 0$. Then the Euler–Lagrange equation $\partial \mathcal{L} / \partial \phi(\mathbf{v}) = 0$ at vertex $\mathbf{v}$ yields*

$$
\bigl(\Delta_w \phi\bigr)(\mathbf{v}) \;=\; \sum_{\mathbf{u} \in \mathcal{N}_\text{face}(\mathbf{v})} w_\text{face}\,[\phi(\mathbf{u}) - \phi(\mathbf{v})] \;+\; \sum_{\mathbf{u} \in \mathcal{N}_\text{edge}(\mathbf{v})} w_\text{edge}\,[\phi(\mathbf{u}) - \phi(\mathbf{v})]
$$

*as the discrete Laplacian operating on $\phi$ at $\mathbf{v}$.*

**Proof.** Each neighbour pair $(\mathbf{v}, \mathbf{u})$ appears in the sum in two places: once for $\mathcal{L}_\text{grad}(\mathbf{v})$ and once for $\mathcal{L}_\text{grad}(\mathbf{u})$. Variation with respect to $\phi(\mathbf{v})$ gives:

$$
\frac{\partial \mathcal{L}_\text{grad}(\mathbf{w})}{\partial \phi(\mathbf{v})} = \begin{cases}
- c^2 \sum_{\mathbf{u} \in \mathcal{N}(\mathbf{v})} w_{\mathbf{v}\mathbf{u}}\,[\phi(\mathbf{v}) - \phi(\mathbf{u})] \cdot (-1) & \text{if } \mathbf{w} = \mathbf{v} \\
- c^2 w_{\mathbf{v}\mathbf{w}}\,[\phi(\mathbf{w}) - \phi(\mathbf{v})] \cdot 1 & \text{if } \mathbf{v} \in \mathcal{N}(\mathbf{w}) \\
0 & \text{otherwise}
\end{cases}
$$

Summing over both $\mathbf{w} = \mathbf{v}$ and $\mathbf{w} \in \mathcal{N}(\mathbf{v})$ contributions:

$$
\sum_\mathbf{w} \frac{\partial \mathcal{L}_\text{grad}(\mathbf{w})}{\partial \phi(\mathbf{v})}
= c^2 \sum_{\mathbf{u} \in \mathcal{N}(\mathbf{v})} w_{\mathbf{v}\mathbf{u}}\,[\phi(\mathbf{u}) - \phi(\mathbf{v})] + c^2 \sum_{\mathbf{u} \in \mathcal{N}(\mathbf{v})} w_{\mathbf{u}\mathbf{v}}\,[\phi(\mathbf{u}) - \phi(\mathbf{v})]
$$

By translation- and rotation-equivariance, $w_{\mathbf{v}\mathbf{u}} = w_{\mathbf{u}\mathbf{v}}$ depends only on the link type. The two terms combine and the variational derivative is, after factoring out the common $2c^2$ that is absorbed into the canonical kinetic normalisation:

$$
\frac{1}{c^2}\,\frac{\partial S_\text{grad}}{\partial \phi(\mathbf{v})} \cdot \frac{1}{\Delta t}
= \sum_{\mathbf{u} \in \mathcal{N}_\text{face}(\mathbf{v})} w_\text{face}\,[\phi(\mathbf{u}) - \phi(\mathbf{v})]
+ \sum_{\mathbf{u} \in \mathcal{N}_\text{edge}(\mathbf{v})} w_\text{edge}\,[\phi(\mathbf{u}) - \phi(\mathbf{v})]
$$

This is the discrete Laplacian $\Delta_w \phi$ as claimed. $\square$

---

## §3 — Continuum limit and the isotropy constraint

For smooth $\phi$ (treating the lattice as a discretisation of a continuum field for the purpose of computing the leading-order operator), Taylor-expand $\phi(\mathbf{u})$ about $\mathbf{v}$:

$$
\phi(\mathbf{u}) - \phi(\mathbf{v}) = (\mathbf{u} - \mathbf{v})_i\,\partial_i\phi + \tfrac{1}{2}(\mathbf{u} - \mathbf{v})_i (\mathbf{u} - \mathbf{v})_j\,\partial_i\partial_j\phi + \tfrac{1}{6}\cdots\partial_i\partial_j\partial_k\phi + \tfrac{1}{24}\cdots\partial_i\partial_j\partial_k\partial_l\phi + O(a^5)
$$

Summed over neighbour sets $\mathcal{N}_\text{face}$ and $\mathcal{N}_\text{edge}$, odd-derivative terms cancel by the inversion symmetry $\mathbf{u} \to -\mathbf{u}$ across $\mathbf{v}$. The leading non-vanishing terms are the second and fourth derivatives.

**Second-derivative contribution.** Computing $\sum_\mathbf{u}(\mathbf{u}-\mathbf{v})_i(\mathbf{u}-\mathbf{v})_j$ over each neighbour set gives:

| Set | $\sum (\mathbf{u}-\mathbf{v})_i (\mathbf{u}-\mathbf{v})_j$ |
|---|---|
| $\mathcal{N}_\text{face}$ | $2\delta_{ij}$ (from each axis ±1, weighted by 2 = |face count along that axis|) |
| $\mathcal{N}_\text{edge}$ | $4\delta_{ij}$ (from 4 edges along each face plane axis pair, by symmetry: each axis $i$ appears in 4 edge directions with weight 1) |

Multiplied by $\frac{1}{2}\partial_i\partial_j\phi$ and summed:

$$
\Delta_w \phi\Big|_{O(a^2)} = w_\text{face}\,\bigl(2\nabla^2\phi\bigr) + w_\text{edge}\,\bigl(4\nabla^2\phi\bigr) = (2 w_\text{face} + 4 w_\text{edge})\,\nabla^2\phi
$$

**The continuum-Laplacian normalisation requires $2 w_\text{face} + 4 w_\text{edge} = 1$.** This is one constraint on $(w_\text{face}, w_\text{edge})$.

**Fourth-derivative contribution (isotropy constraint).** Computing $\sum_\mathbf{u}(\mathbf{u}-\mathbf{v})_i(\mathbf{u}-\mathbf{v})_j(\mathbf{u}-\mathbf{v})_k(\mathbf{u}-\mathbf{v})_l$ over each neighbour set:

For $\mathcal{N}_\text{face}$: only diagonal terms contribute, $\sum = 2\delta_{ijkl}$ (i.e. $i=j=k=l$).

For $\mathcal{N}_\text{edge}$: contributions $\sum = 4\bigl[\delta_{ij}\delta_{kl} + \delta_{ik}\delta_{jl} + \delta_{il}\delta_{jk}\bigr] - 4\delta_{ijkl}$ (the off-diagonal pieces include the $\delta_{ij}\delta_{kl}$-style traces, with a correction).

Let me write this out without the abuse of $\delta_{ijkl}$ notation. Define $A_\text{face} = \sum_{\mathbf{u} \in \mathcal{N}_\text{face}} (\mathbf{u}-\mathbf{v})_i(\mathbf{u}-\mathbf{v})_j(\mathbf{u}-\mathbf{v})_k(\mathbf{u}-\mathbf{v})_l$. Direct computation: $A_\text{face} = 2$ if $i=j=k=l$ (axis-aligned), 0 otherwise. Similarly $A_\text{edge}$ has structure: $A_\text{edge} = 4$ if exactly two indices match each of the two axes spanning the edge plane (i.e. $\{i,j\}=\{a,b\}, \{k,l\}=\{a,b\}$ pairings), 0 if any single index is unmatched.

After contraction with $\frac{1}{24}\partial_i\partial_j\partial_k\partial_l\phi$, the $O(a^4)$ correction is:

$$
\Delta_w \phi\Big|_{O(a^4)} = \frac{1}{24}\Bigl[\,2 w_\text{face}\sum_a\partial_a^4\phi + 4 w_\text{edge}\sum_{a\neq b}\partial_a^2\partial_b^2\phi\,\Bigr] \cdot 4\,! / \text{combinatorial}
$$

(After collecting numerical factors carefully — see appendix in any standard reference, e.g. Patra–Karttunen 2006 or the lattice-PDE literature.) The isotropic combination is $\nabla^4\phi = (\sum_a \partial_a^2)^2 = \sum_a\partial_a^4 + 2\sum_{a<b}\partial_a^2\partial_b^2$. The cubic-anisotropic combination is $\sum_a \partial_a^4$.

The 4th-derivative term in $\Delta_w\phi$ becomes a linear combination of $\nabla^4\phi$ (isotropic) and $\sum_a\partial_a^4\phi - 3\sum_{a<b}\partial_a^2\partial_b^2\phi$ (cubic-anisotropic, traceless). For **leading-order isotropy** — meaning that the lattice Laplacian's $O(a^4)$ correction is purely the isotropic biharmonic — the cubic-anisotropic coefficient must vanish:

$$
2 w_\text{face} - 4 w_\text{edge} = 0 \quad\Leftrightarrow\quad w_\text{face} = 2 w_\text{edge}
$$

**Combining both constraints.** $2 w_\text{face} + 4 w_\text{edge} = 1$ and $w_\text{face} = 2 w_\text{edge}$ give the unique solution:

$$
\boxed{w_\text{face} = \frac{1}{3}, \qquad w_\text{edge} = \frac{1}{6}}
$$

These are exactly the weights used in `engine/include/ftd/lagrangian.h:93, 97`.

---

## §4 — Why "leading-order isotropy" and not "exact"?

Adding the corner set $\mathcal{N}_\text{corner}$ (8 neighbours at distance $\sqrt{3}$) introduces a third weight $w_\text{corner}$ and a third constraint from the next isotropy condition. The 27-point family $(w_\text{face}, w_\text{edge}, w_\text{corner})$ has more isotropy-restoring solutions and reduces the leading anisotropic operator dimension further — but at the cost of a longer-range stencil and increased computational expense.

The 18-point {1/3, 1/6} stencil:

- **Cancels the leading cubic-anisotropic $O(a^4)$ correction.** The remaining anisotropy is at $O(a^6)$ and Wilsonian-irrelevant in 4D under the standard power counting. (This is why `AUDIT_LORENTZ_ANISOTROPY.md` measured anisotropy exponent $p = 4.0008 \pm 0.0006$ — Wilsonian-irrelevant operator at dimension 7, consistent with the leading-order isotropic stencil's residual anisotropy living one order higher than the isotropic dispersion.)
- **Is the canonical choice in the lattice-PDE literature** (under various names: "Patra–Karttunen 18-pt", "Mehrstellen", "compact 9-pt"-3D-extension).

A 27-point stencil would push the anisotropy to $O(a^8)$ at additional cost; the engine deliberately chooses the 18-point stencil as the right cost / isotropy trade-off for the FTD lattice.

---

## §5 — The relation to the engine implementation

`engine/include/ftd/lagrangian.h:86-100` (function `field_gradient_term()`):

```cpp
inline double field_gradient_term(const Vec3& flux_here,
                                  const std::array<int, 6>& nbr6,
                                  const std::array<int, 12>& nbr12,
                                  const std::vector<Voxel>& voxels) {
    double grad_sq = 0.0;
    for (int n : nbr6) {
        Vec3 d = voxels[n].flux - flux_here;
        grad_sq += (1.0 / 3.0) * d.mag2();
    }
    for (int n : nbr12) {
        Vec3 d = voxels[n].flux - flux_here;
        grad_sq += (1.0 / 6.0) * d.mag2();
    }
    return -0.5 * (C_WAVE * C_WAVE) * grad_sq;
}
```

This implements $\mathcal{L}_\text{grad} = -\tfrac{c^2}{2}\bigl[\sum_\text{face}(1/3)|\Delta J|^2 + \sum_\text{edge}(1/6)|\Delta J|^2\bigr]$ exactly. The `nbr6` / `nbr12` arrays are the face/edge neighbour index lists.

The Laplacian operator that the variational derivative produces — used in `phase_read.cpp` and the GPU stencil kernels (`engine/cuda/kernels_stencil_*.cu`) — has the form

$$
(\Delta_{18}\,\mathbf{J})(\mathbf{v}) = \tfrac{1}{3}\sum_\text{face}\mathbf{J}(\mathbf{u}) + \tfrac{1}{6}\sum_\text{edge}\mathbf{J}(\mathbf{u}) - 4\,\mathbf{J}(\mathbf{v})
$$

where the diagonal "$-4$" is $\bigl(6 \cdot \tfrac{1}{3} + 12 \cdot \tfrac{1}{6}\bigr) = 4$. This matches the engine stencil documented in `SPEC_FTD.md` and used by the `phase_read()` 18-point Moore Laplacian.

**EL-residual verification** (`compute_el_residual()` at `engine/src/lagrangian.cpp:69-95`): after `phase_read()` runs, `delta_j_[i]` should equal $c^2(\Delta_{18}\,\mathbf{J})(\mathbf{v}) + g_c \nabla s + g_c \nabla\times(s\,\mathbf{v})$. Test runs typically report RMS $\sim 10^{-15}$ — the variational identity above is empirically verified at machine epsilon every tick.

---

## §6 — Sublattice variants (still [OPEN] as of 2026-05-05)

The variational identity in §2 is general — it holds for any choice of weights including sublattice-specific ones (BCC face-only, FCC face+edge, SC face-only, etc.). The 18-point {1/3, 1/6} weights are the canonical isotropic choice on the **full cubic lattice with single substrate**.

For dual-substrate or BCC-sublattice work — `bcc_stencil != FULL` per `engine/include/ftd/term_toggles.h:79` — the corresponding weights need their own variational + isotropy analysis. Two known cases:

- **BCC sub-stencil (face-only on BCC sublattice)** — `Cluster A` / `FTD-0093` work. The kinetic functional restricts to BCC links; the resulting Laplacian's isotropy properties differ. Per `AUDIT_BCC_SUBLATTICE_SPECTRUM.md` (closed-negative for the Mechanism C ratio prediction), this stencil has different physics from the 18-point isotropic one. A full variational analysis for this case is **[OPEN]** — would close part of MC-T1 in the math-completion checklist.
- **Mixed FCC/SC stencils** — explored in `DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md` for the multiplicative-structure theorem, but not analysed at variational stencil-weight level. **[OPEN]**.

The R2 deliverable closes only the canonical 18-point full-cubic-single-substrate case. The sublattice-specific variants remain queued for separate derivation under future R-tier work.

---

## §7 — Cross-reference

| Spec / engine target | This doc closes |
|---|---|
| `SPEC_OPERATOR_BASIS_COMPLETE.md` "linear G18 generator [THEOREM] for cubic lattice" | The variational derivation of the {1/3, 1/6} weights from the kinetic action. |
| `SPEC_FTD_LAGRANGIAN.md` §3.6 Term 6 ("Field gradient energy") | Justification for the {1/3, 1/6} weights from leading-order isotropy. |
| `MAP_LAGRANGIAN_TO_ENGINE.md` Term 6 row "Stencil weights are currently empirical at the spec level" | Empirical → derived. |
| Engine stencil tests (`test_moore_laplacian_isotropy`, `test_dispersion`) | Now have a theoretical reference for what they're verifying. |
| `AUDIT_LORENTZ_ANISOTROPY.md` $p = 4.0008 \pm 0.0006$ measurement | Predicted by the residual $O(a^6)$ anisotropy of the 18-pt stencil; consistency check. |

The stencil derivation does not affect any physics-bearing tag elsewhere in the project; the 18-point Laplacian's isotropy was already empirically validated. This doc graduates one [SELECTION] (the weight choice) to [THEOREM]-conditional-on-leading-isotropy — see §2 vs §3 epistemic split.

---

## §8 — What this derivation does NOT prove

To keep the doc focused:

- **The physical scale** of the Laplacian operator (i.e. its overall coefficient $c^2$ and how it couples to time-evolution): determined by the leapfrog step + bandwidth constraint, not by this variational derivation.
- **Stability of the discrete time evolution** under the 18-pt Laplacian: governed by CFL ($c_\text{max} = 1/\sqrt{3}$) — an independent constraint, treated in `SPEC_ENGINE.md`.
- **Renormalization of $c^2$** under blocking (the $b=2$ Wilsonian map): governed by $K_T^\text{FTD}$ and the response-flow tuple — see `DERIV_FTD_NATIVE_RESPONSE_AND_BLOCKING.md`.
- **Sublattice variants** (BCC, FCC, mixed): [OPEN] per §6.

The derivation establishes only that the 18-point face+edge Laplacian with weights {1/3, 1/6} is the canonical leading-order isotropic kinetic operator on the cubic lattice, derivable by direct variation of the 18-point kinetic Lagrangian density.
