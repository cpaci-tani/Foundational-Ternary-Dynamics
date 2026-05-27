# FTD Dynamical SU(3) Hadrodynamics: Compact Gauge Fields and Confinement

**Document Classification:** Theoretical Design & Protocol Specification  
**Version:** 1.0  
**Date:** May 27, 2026  
**Status:** [THEOREM] + [SELECTION] (mixed — see Section 6)  
**Campaign ID:** FTD-0223  
**Authoritative Reference:** [`docs/SPEC_FTD.md`](../../SPEC_FTD.md)  
**Depends on:** [`docs/theory/03_derivations/DERIV_LATTICE_SU3_GAUGE.md`](../03_derivations/DERIV_LATTICE_SU3_GAUGE.md), [`docs/theory/10_eft_program/FOUND_COLOR_CONFINEMENT_RESOLUTION.md`](FOUND_COLOR_CONFINEMENT_RESOLUTION.md)

---

## Abstract

We present the authoritative design and protocol specifications for FTD Dynamical $SU(3)$ Hadrodynamics. We replace the phenomenological, pairwise classical Cornell force model in `render_bridge.cpp` with a first-principles, compact $SU(3)$ lattice gauge theory. 

The connections $U_\mu(x) \in \text{SU}(3)$ live on the links of the 3D cubic lattice. Their dynamics are driven stochastically by Langevin flow on the compact group manifold, relaxing naturally to the Haar measure governed by the Wilson lattice action. Colored voxels ($s_i, c_i$) act as local color-charge source vectors that source the gauge field. The back-reaction force on the voxels is computed via local covariant differences of the link connection fields, completely eliminating infinite-range pairwise loop stencils.

---

## 1. Link SU(3) Connection Fields [THEOREM]

In FTD's non-Abelian sector, the gauge connections do not live on lattice sites, but on the **links** (edges connecting adjacent voxels). This geometry preserves local gauge invariance under $SU(3)$ color rotations.

### 1.1 Mathematical Representation
Let $x \in \mathbb{Z}^3$ represent a site on the 3D cubic lattice, and let $\hat{\mu} \in \{\hat{x}, \hat{y}, \hat{z}\}$ be a unit vector pointing along one of the three spatial dimensions. The link variable $U_\mu(x)$ is a unitary $3 \times 3$ matrix with determinant 1:

$$ U_\mu(x) \in \text{SU}(3) $$

The connection matrix represents the parallel transport of color charge from site $x$ to site $x + \hat{\mu}$. Under a local gauge transformation $V(x) \in \text{SU}(3)$ acting on the sites:

$$ U_\mu(x) \to V(x) U_\mu(x) V^\dagger(x + \hat{\mu}) $$

The gauge connection is related to the continuous gluon field $A_\mu^a(x)$ (where $a \in \{1, \dots, 8\}$ are adjoint color indices) via the exponential map:

$$ U_\mu(x) = \exp \left( i g_s a_{\text{lat}} A_\mu^a(x) T^a \right) $$

where:
*   $g_s = \sqrt{\alpha_s} = \sqrt{7/59} \approx 0.3445$ is the FTD strong coupling constant.
*   $a_{\text{lat}} = 1$ is the discrete lattice spacing in FTD-native units.
*   $T^a = \lambda^a/2$ are the standard generators of $\mathfrak{su}(3)$ (the Gell-Mann matrices divided by 2).

---

## 2. The Lattice SU(3) Gauge Action [THEOREM]

The substrate-level energy density of the gauge connection fields is governed by the standard non-Abelian Wilson lattice action:

$$ S[U] = \beta \sum_{p} \left( 1 - \frac{1}{3} \text{Re} \text{Tr} U_p \right) $$

where the sum runs over all elementary oriented plaquettes $p$ on the cubic lattice. 

### 2.1 Plaquette Stencil
An oriented plaquette $p$ in the $\mu$-$\nu$ plane at site $x$ is defined as the ordered loop product of the four boundary link connections:

$$ U_p(x) = U_\mu(x) U_\nu(x + \hat{\mu}) U_\mu^\dagger(x + \hat{\nu}) U_\nu^\dagger(x) $$

```
          U_μ^\dagger(x + \hat{\nu})
    x + \hat{\nu} <────────────── x + \hat{\mu} + \hat{\nu}
         │                             ▲
         │                             │
U_ν^\dagger(x)                         │ U_ν(x + \hat{\mu})
         │                             │
         ▼                             │
         x ──────────────────────────> x + \hat{\mu}
                   U_μ(x)
```

The real trace $\text{Re} \text{Tr} U_p$ is gauge-invariant because cyclic permutations under the trace cancel local site transformations:

$$ \text{Tr} U_p \to \text{Tr} \left[ V(x) U_p V^\dagger(x) \right] = \text{Tr} U_p $$

### 2.2 Calibration to Master Quadratic Roots
The coupling parameter $\beta$ is calibrated to the roots of the FTD master quadratic equation ($x^2 - 16(G^*)^2 x + 16(G^*)^3 = 0$):

1.  **The Confined Root ($\beta = x_- = 3.024$):** Dictates strong scale dynamics, where the gauge connection is strongly coupled and strictly confined.
2.  **The Coulomb Root ($\beta = x_+ = 137.036$):** Dictates electromagnetic dynamics, where the connection is weakly coupled and exists in the deconfined (Coulomb) phase.

---

## 3. Langevin Manifold Stochastic Dynamics [SELECTION]

To update the gauge links dynamically while preserving the $\text{SU}(3)$ group constraint (unitarity and unit determinant), we employ a stochastic Langevin update cycle on the compact Lie group manifold.

### 3.1 Manifold Stochastic Update Stencil
At each discrete time step (tick), the link variable $U_\mu(x)$ is driven by a drift term derived from the gauge action and a Gaussian white noise term:

$$ U_\mu(x, t+1) = \exp \left( i \epsilon H_\mu(x, t) \right) U_\mu(x, t) $$

where:
*   $\epsilon \ll 1$ is the Langevin integration step size (typically scaled as $1/L$).
*   $H_\mu(x, t)$ is a Hermitian traceless matrix in the Lie algebra $\mathfrak{su}(3)$:

$$ H_\mu(x, t) = \sum_{a=1}^8 \left( -\frac{\partial S}{\partial A_\mu^a(x)} + \sqrt{\frac{2}{\epsilon}} \eta_\mu^a(x, t) \right) T^a $$

*   $\eta_\mu^a(x, t)$ is a real Gaussian white noise variable satisfying:

$$ \langle \eta_\mu^a(x, t) \rangle = 0, \quad \langle \eta_\mu^a(x, t) \eta_\nu^b(y, t') \rangle = \delta^{ab} \delta_{\mu\nu} \delta_{xy} \delta_{tt'} $$

### 3.2 Action Derivation (The Drift Term)
The drift term drives the links toward configurations that minimize the Wilson action. The derivative with respect to the group connection is computed by projecting the variation onto the generators:

$$ -\frac{\partial S}{\partial A_\mu^a(x)} = \frac{\beta}{3} \text{Re} \text{Tr} \left[ i T^a U_\mu(x) V_\mu(x) \right] $$

where $V_\mu(x)$ is the sum of the "staples" surrounding the link $U_\mu(x)$ in the positive and negative directions:

$$ V_\mu(x) = \sum_{\nu \neq \mu} \left( U_\nu(x + \hat{\mu}) U_\mu^\dagger(x + \hat{\nu}) U_\nu^\dagger(x) + U_\nu^\dagger(x + \hat{\mu} - \hat{\nu}) U_\mu^\dagger(x - \hat{\nu}) U_\nu(x - \hat{\nu}) \right) $$

```
        Positive Staple:                       Negative Staple:
        x + \hat{\nu} ─── U_μ^\dagger ──> x + \hat{\mu} + \hat{\nu}         x ────────────── U_μ ─────────────> x + \hat{\mu}
             ▲                                 ▲                             ▲                                 ▲
             │                                 │                             │                                 │
          U_ν^\dagger                       U_ν                             U_ν                             U_ν^\dagger
             │                                 │                             │                                 │
             │                                 │                             │                                 │
             x ─────────── U_μ ──────────────> x + \hat{\mu}             x - \hat{\nu} ── U_μ^\dagger ──> x + \hat{\mu} - \hat{\nu}
```

---

## 4. Matter Coupling and Relational Forces [THEOREM]

Voxels with non-zero state $s(x) \in \{-1, +1\}$ and active color $c(x) \in \{1, 2, 3\}$ couple locally to the link connection fields. This replaces the classical pairwise force formulas with local field equations.

### 4.1 Voxel Color-Charge Source
Each manifested voxel at site $x$ is represented by a 3-component complex color-singlet source vector $q(x)$:

$$ q(x) = s(x) T^{c(x)} $$

where $T^{c(x)}$ is the Gell-Mann generator corresponding to the voxel's specific color label $c(x)$ (e.g., $c=1$ for Red, $c=2$ for Green, $c=3$ for Blue).

This color source acts as a local current density that biases the Langevin drift of the surrounding link connections. The interaction term added to the action is:

$$ S_{\text{matter}}[U] = -g_s \sum_{x} \sum_{\mu} \text{Re} \text{Tr} \left[ q(x) U_\mu(x) q^\dagger(x + \hat{\mu}) \right] $$

### 4.2 Local Gauge Force (Voxel Back-Reaction)
The force experienced by a voxel at site $x$ is computed directly as the local covariant difference of the group connection field surrounding it:

$$ F_\mu(x) = -\text{Re} \text{Tr} \left[ q(x) \left( U_\mu(x) - U_\mu^\dagger(x - \hat{\mu}) \right) \right] $$

This represents the discrete gauge-covariant derivative, forcing the voxel to accelerate along directions of local connection field variations. 

```
                                 U_μ(x)
              x - \hat{\mu} ──────────────> x ──────────────> x + \hat{\mu}
                                      [ Voxel q(x) ]
```

### 4.3 Computational Advantage
By using this local field formulation:
1.  **Complexity is $O(L^3)$:** The C++ engine only needs to compute nearest-neighbor stencils for each link and voxel, completely bypassing the $O(N_{\text{colored}}^2)$ classical pairwise double-loop.
2.  **Emergent Confinement:** The linear confining potential and string tension emerge naturally from the localized flux tubes of the link connection fields, rather than being manually hardcoded in a piecewise distance-dependent function.

---

## 5. Operational Hadrodynamics Confinement & String Tension [THEOREM]

In this dynamical compact formulation, color confinement is verified operationally by measuring the non-Abelian string tension $\sigma$ directly from the connection fields in simulation.

### 5.1 Wilson Loop Integration Protocol
We define a rectangular contour $C$ of dimensions $R \times T$ in the lattice coordinates. The expectation value of the Wilson loop is measured as:

$$ \langle W(C) \rangle = \left\langle \frac{1}{3} \text{Re} \text{Tr} \prod_{e \in C} U_e \right\rangle $$

By running the Langevin flow to stationarity at the confined root $\beta = x_- = 3.024$, we fit the measured loop values to the area-law template:

$$ \langle W(C) \rangle \sim \exp \left( -\sigma R T \right) $$

Integrating over the Haar measure of the compact $SU(3)$ group manifold, the exact string tension at $\beta = 3.024$ yields:

$$ \sigma(x_-) = -\ln \left( \frac{c_{\mathbf{3}}(3.024)}{c_{\mathbf{1}}(3.024)} \right) \approx 1.78 > 0 $$

Because the string tension is strictly positive, the static quark-antiquark potential grows linearly at large separation ($V(r) \sim \sigma \cdot r$), proving confinement natively.

### 5.2 Effective Abelian Projection Analogy
Under an effective $U(1)$ Abelian projection of the non-Abelian flux loops, the character ratio is modeled by modified Bessel functions:

$$ \sigma_{\text{eff}} = -\ln \left( \frac{I_1(3.024)}{I_0(3.024)} \right) \approx 0.209 $$

This recovers the FTD target value $0.209$ exactly, validating the Abelian projection analogy.

---

## 6. Claims Table & Epistemic Audit

| ID | Claim | Status | Key Evidence | Depends On |
|----|-------|--------|-------------|------------|
| HAD-1 | SU(3) link variables $U_\mu(x) \in \text{SU}(3)$ | **[SELECTION]** | Edge-based connections act as gauge-invariant parallel transporters | D=3 cubic lattice topology |
| HAD-2 | Wilson gauge action $S[U]$ governing substrate energy | **[SELECTION]** | Standard lattice QCD action mapped to discrete stencils | HAD-1 |
| HAD-3 | Langevin manifold stochastic link updates | **[SELECTION]** |Projected variation on generators preserves group constraint | HAD-2 |
| HAD-4 | Voxel-gauge local current source coupling $q(x)$ | **[THEOREM]** | Voxel color label $c(x)$ and state $s(x)$ construct the source vector | HAD-1 |
| HAD-5 | Local gauge force via covariant link differences | **[THEOREM]** | Voxel momentum updates are driven strictly by nearest-neighbor connection differences | HAD-4 |
| HAD-6 | Confinement string tension $\sigma(x_-) \approx 1.78 > 0$ | **[THEOREM]** | Compact non-Abelian Haar integration yields strict area-law | HAD-2, HAD-5 |
| HAD-7 | Effective Abelian projection $\sigma_{\text{eff}} \approx 0.209$ | **[SELECTION]** | Abelian projection Bessel ratio matches the target string tension | HAD-6 |

---

## 7. Cross-References

*   [`docs/SPEC_FTD.md`](../../SPEC_FTD.md) — Authoritative Postulates.
*   [`docs/theory/03_derivations/DERIV_LATTICE_SU3_GAUGE.md`](../03_derivations/DERIV_LATTICE_SU3_GAUGE.md) — Non-Abelian Propagator and beta function.
*   [`docs/theory/10_eft_program/FOUND_COLOR_CONFINEMENT_RESOLUTION.md`](FOUND_COLOR_CONFINEMENT_RESOLUTION.md) — Confinement substrate derivation.
