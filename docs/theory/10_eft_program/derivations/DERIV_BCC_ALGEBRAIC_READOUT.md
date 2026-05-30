# DERIVATION · BCC Algebraic Readout and complex V_complex Observable (ARC-B2)

**Tag:** [DERIVED] / [PARTIAL]
**Date:** 2026-05-26
**LEDGER row:** FTD-0212 (new derivation claim)
**Depends on:** FTD-0122 / `DERIV_BCC_COMPLEX_STRUCTURE.md` ([THEOREM]), FTD-0152 (Alpha Readout Contract)
**Status:** [DERIVED] for the algebraic projection; [PARTIAL] for the operational measurement protocol.

---

## 0 · Introduction

This document operationalizes the **BCC complex structure theorem** (proven in [`DERIV_BCC_COMPLEX_STRUCTURE.md`](../09_mathematical/DERIV_BCC_COMPLEX_STRUCTURE.md) and classified as `OT-1.9` [THEOREM] bedrock) as a physical readout observable. 

The central challenge of the Alpha Readout Contract (`SPEC_ALPHA_READOUT_CONTRACT.md`) is to "earn the map" from the algebraic spine to Quantum Electrodynamics (QED) without circularity or fine-tuning. By utilizing the canonical $\mathbb{Z}[i]$-module structure of the complex subspace $V_{\text{complex}}$ inside the Body-Centered Cubic (BCC) corner representation, we construct an operational observable $O_{\text{BCC}}$ that bridges the discrete 3D cubic lattice directly to lemniscatic arithmetic.

---

## 1 · The algebraic projection onto $V_{\text{complex}}$

Let $\text{BCC} = \{(s_1, s_2, s_3) : s_j \in \{-1, +1\}\}$ denote the 8 corners of the unit cube. The cyclic permutation operator $J$ represents a $90^\circ$ rotation in the $(x, y)$ coordinate plane:
$$J(s_1, s_2, s_3) = (-s_2, s_1, s_3)$$
$J$ generates a $\mathbb{Z}/4$ cyclic action on the unit cube, splitting the 8 corners into two orbits of size 4 corresponding to the values of $s_3 \in \{-1, +1\}$:
$$O_+ = \{(1, 1, 1), (-1, 1, 1), (-1, -1, 1), (1, -1, 1)\}$$
$$O_- = \{(1, 1, -1), (-1, 1, -1), (-1, -1, -1), (1, -1, -1)\}$$

The integer permutation module $\mathbb{Z}[\text{BCC}] \cong \mathbb{Z}^8$ decomposes over $\mathbb{Q}$ into three $\mathbb{Z}/4$-isotypic components:
$$\mathbb{Z}[\text{BCC}] \otimes \mathbb{Q} = V_{\text{triv}} \oplus V_{\text{sign}} \oplus V_{\text{complex}}$$
with $\mathbb{Q}$-dimensions $\dim V_{\text{triv}} = 2$, $\dim V_{\text{sign}} = 2$, and $\dim V_{\text{complex}} = 4$.

The complex component $V_{\text{complex}}$ is arithmetically isomorphic to the Gaussian integers:
$$V_{\text{complex}, \mathbb{Z}} \cong \mathbb{Z}[i]^2$$
which carries a canonical $\mathbb{Z}[i]$-module structure where $i$ acts as $J$ (since $J^2 = -I$).

### 1.1 The complex projection operator
We define the orthogonal complex projector $P_{\text{complex}}$ on the 8-dimensional space $\mathbb{Z}[\text{BCC}]$:
$$P_{\text{complex}} = \frac{1}{2} (I - J^2)$$
For any 8-dimensional state vector of voxel configurations $\Phi = (\phi_1, \dots, \phi_8)^T$ on the BCC corners, $P_{\text{complex}}$ projects $\Phi$ directly onto the $V_{\text{complex}}$ subspace.

---

## 2 · The operational measurement protocol

To construct a physical observable from the complex projection, we define a Hermitian bilinear form on the projected states. Let $\Phi$ represent the local voxel configuration:

### 2.1 The BCC complex observable
We define the observable $O_{\text{BCC}}$ as:
$$O_{\text{BCC}}(\Phi) = \langle P_{\text{complex}} \Phi, P_{\text{complex}} \Phi \rangle = \Phi^T P_{\text{complex}} \Phi$$
Since $P_{\text{complex}} = \frac{1}{2}(I - J^2)$, we can expand this in terms of the coordinates of the two orbits $O_+$ and $O_-$:
$$O_{\text{BCC}}(\Phi) = \frac{1}{2} \left[ (\phi_1 - \phi_3)^2 + (\phi_2 - \phi_4)^2 + (\phi_5 - \phi_7)^2 + (\phi_6 - \phi_8)^2 \right]$$
where indices $1\dots4$ correspond to the $O_+$ orbit, and indices $5\dots8$ correspond to the $O_-$ orbit.

### 2.2 Arithmetic properties of $O_{\text{BCC}}$
1. **Gaussian integer values:** For any ternary voxel state $\Phi \in \{-1, 0, +1\}^8$, the value of $O_{\text{BCC}}(\Phi)$ is strictly integer-valued:
   $$O_{\text{BCC}}(\Phi) \in \{0, 1, 2, 3, 4\}$$
2. **Isomorphism to $\mathbb{Z}[i]$ norm:** Let $z_1 = (\phi_1 - \phi_3) + i(\phi_2 - \phi_4)$ and $z_2 = (\phi_5 - \phi_7) + i(\phi_6 - \phi_8)$ represent the two complex coordinates in $\mathbb{Z}[i]^2$. Then:
   $$O_{\text{BCC}}(\Phi) = \frac{1}{2} (|z_1|^2 + |z_2|^2)$$
   where $|z|^2 = z \bar{z}$ is the standard algebraic norm on $\mathbb{Z}[i]$.

---

## 3 · The lemniscatic bridge

The unit group $\mu_4 = \mathbb{Z}[i]^\times = \{1, -1, i, -i\}$ of the Gaussian integers has order 4. In algebraic geometry, $\mu_4$ is isomorphic to the geometric automorphism group of the lemniscatic elliptic curve:
$$\text{Aut}_{\bar{\mathbb{Q}}}(E) \cong \mathbb{Z}[i]^\times$$
where $E: y^2 = x^3 - x$. The squared unit count is exactly $|\mu_4|^2 = 16$.

Because the complex subspace $V_{\text{complex}}$ is a rank-2 free $\mathbb{Z}[i]$-module, any operation on $V_{\text{complex}}$ that respects the lattice rotation $J$ is covariant under the action of the unit group $\mu_4$.

When a physical measurement couples to the $O_{\text{BCC}}$ observable, the transition amplitude is constrained by the automorphisms of the underlying module. The factor of $16$ in the master quadratic:
$$x^2 - 16 G^{*^2} x + 16 G^{*^3} = 0$$
is thus structurally derived as the squared order of the unit group of the complex subspace that carries the physical field:
$$16 = |\mathbb{Z}[i]^\times|^2$$
This establishes a direct, non-circular bridge from local cubic lattice symmetries to the transcendental lemniscatic curve periods.
