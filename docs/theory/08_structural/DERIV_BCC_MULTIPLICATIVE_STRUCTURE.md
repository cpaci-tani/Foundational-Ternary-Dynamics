# The BCC Multiplicative Structure

## Why the gap equation coefficient and the color gauge group share a single origin

**Date:** April 11, 2026
**Framework:** Foundational Ternary Dynamics v5.29
**Status:** Structural theorem + physical identification [SELECTION]
**Dependencies:** THEOREM_MOORE_LAYER_DECOMPOSITION.md, DERIV_WATSON_GSTAR_IDENTITY.md, DERIV_MOORE_GAUGE_STRUCTURE.md, FOUND_POTENTIAL_CORE_AND_GENERATIVE_INTERIOR.md

---

## Abstract

The 8 BCC (body-centered cubic) neighbors at offsets `(+-1, +-1, +-1)` in the 26-neighbor Moore neighborhood have a unique structural property: their Laplacian eigenvalue is a **product** of cosines, `sigma_BCC(k) = 1 - cos k_1 cos k_2 cos k_3`, not a sum. This single fact has two consequences that were previously treated as separate results:

1. **Propagator:** The BCC Watson integral equals `Gamma(1/4)^4 / (4 pi^3) = G*^2/(2 pi)`, providing the gap equation coefficient `16 G*^2` that yields `alpha` and `N_c` (Part I of this document)
2. **Gauge structure:** The BCC sublattice couples all 3 flux components `(J_x, J_y, J_z)` simultaneously, generating the color gauge group SU(3) (Part II)

These are not two independent properties of BCC. They are the **same property** (the multiplicative cosine product) seen from two perspectives: momentum-space (propagator) and position-space (flux coupling).

---

## Part I: The BCC Propagator and the Watson Identity

### 1.1 The multiplicative eigenvalue [THEOREM — algebraic]

The 8 BCC offsets `(+-1, +-1, +-1)` generate a structure factor:

```text
S_BCC(k) = sum_{delta in BCC} exp(i k . delta)
         = 8 cos(k_1) cos(k_2) cos(k_3)
```

The normalized Laplacian eigenvalue is:

```text
sigma_BCC(k) = 1 - S_BCC(k) / 8 = 1 - cos(k_1) cos(k_2) cos(k_3)
```

This is a **product** of cosines across the three axes. By contrast:

- SC: `sigma_SC(k) = 1 - (cos k_1 + cos k_2 + cos k_3) / 3` — a **sum**
- FCC: `sigma_FCC(k) = 1 - (cos k_1 cos k_2 + cos k_1 cos k_3 + cos k_2 cos k_3) / 3` — a sum of **pairwise** products

Only BCC has the full triple product.

### 1.2 The geometric series factorization [THEOREM — algebraic]

The BCC Green's function at the origin is:

```text
G_BCC(0) = (1/(2pi)^3) integral dk / (1 - cos k_1 cos k_2 cos k_3)
```

The key step: expand as a geometric series:

```text
1 / (1 - x) = sum_{n=0}^{inf} x^n   for |x| < 1
```

with `x = cos k_1 cos k_2 cos k_3`. This gives:

```text
G_BCC(0) = sum_{n=0}^{inf} [(1/(2pi)) integral_0^{2pi} (cos k)^n dk]^3
```

The integral **factors across the three axes** because the product `(cos k_1 cos k_2 cos k_3)^n = (cos k_1)^n (cos k_2)^n (cos k_3)^n` is separable.

Each 1D integral evaluates to:

```text
(1/(2pi)) integral_0^{2pi} (cos k)^{2m} dk = C(2m, m) / 4^m
```

where `C(2m, m)` is the central binomial coefficient. (Odd powers integrate to zero.)

Therefore:

```text
G_BCC(0) = sum_{m=0}^{inf} [C(2m, m) / 4^m]^3
```

### 1.3 The Gamma function identity [THEOREM — number theory]

The sum of cubed central binomial coefficients is a known identity:

```text
sum_{m=0}^{inf} [C(2m, m) / 4^m]^3 = Gamma(1/4)^4 / (4 pi^3)
```

This is Watson's integral `I_1` (Watson, 1939). The connection to `Gamma(1/4)` arises because the central binomial coefficients are ratios of factorials, and `Gamma(1/4)` is the analytic continuation of the factorial to quarter-integers.

### 1.4 The Watson-G* identity [THEOREM]

Since `G* = Gamma(1/4)^2 / (sqrt(2) pi)`:

```text
G*^2 / (2 pi) = Gamma(1/4)^4 / (2 pi^2 * 2 pi)
              = Gamma(1/4)^4 / (4 pi^3)
              = G_BCC(0)
```

Therefore **the BCC Watson integral IS `G*^2/(2 pi)`**. This is exact and algebraic.

### 1.5 The gap equation coefficient [THEOREM given n_DOF = 16]

The gap equation `x^2 = K (x - G*)` with `K = n_DOF * 2pi * G_BCC(0)`:

```text
K = 16 * 2pi * G*^2 / (2pi) = 16 G*^2
```

This is the master quadratic coefficient. With `K = 16 G*^2`, the roots are `x_+ = 1/alpha = 137.036` and `x_- = N_c = 3.024`.

No other sublattice's Watson integral produces this coefficient.

---

## Part II: The BCC Gauge Structure and SU(3)

### 2.1 Flux component excitation by sublattice [THEOREM — combinatorial]

From the Moore Layer Theorem (THEOREM_MOORE_LAYER_DECOMPOSITION.md, Theorem MGS-2):

A neighbor at offset `delta = (delta_x, delta_y, delta_z)` excites flux component `J_mu` if and only if `delta_mu != 0`. The number of nonzero offset components determines how many flux directions are coupled:

| Sublattice | Offsets | Nonzero components | Flux coupling | Gauge group |
|------------|---------|-------------------|---------------|-------------|
| SC | `(+-1, 0, 0)` etc. | 1 | `J_mu` alone | U(1) |
| FCC | `(+-1, +-1, 0)` etc. | 2 | `J_mu J_nu` pair | SU(2) |
| BCC | `(+-1, +-1, +-1)` | 3 | `J_x J_y J_z` all | SU(3) |

BCC is the **unique** sublattice where all three flux components are excited simultaneously. This generates SU(3) because three independent color charges (mapped to `J_x, J_y, J_z`) require three generators — the defining representation of SU(3).

### 2.2 Position-space meaning of the product [THEOREM — structural]

In position space, the BCC structure factor `S_BCC(k) = 8 cos k_1 cos k_2 cos k_3` encodes the fact that a BCC neighbor `(+-1, +-1, +-1)` is displaced along **all three axes simultaneously**. The triple product of cosines in momentum space is the Fourier transform of this simultaneous 3-axis displacement.

The multiplicative structure is not an accident of the coordinates. It reflects the physical fact that a corner neighbor of a cube requires motion in all three spatial directions at once. No axis can be zero.

---

## Part III: The Unification

### 3.1 One structure, two consequences [THEOREM for structure, SELECTION for physical identification]

The BCC multiplicative eigenvalue `1 - cos k_1 cos k_2 cos k_3` produces:

1. **In momentum space (propagator perspective):** The geometric series factorization yields `G_BCC(0) = sum [C(2m,m)/4^m]^3 = Gamma(1/4)^4/(4 pi^3) = G*^2/(2pi)`. This determines the gap equation coefficient `16 G*^2`.

2. **In position space (coupling perspective):** The triple-axis displacement excites all 3 flux components simultaneously, generating the SU(3) color gauge group.

These are **not independent facts**. They both follow from the single property: **BCC offsets have all three components nonzero**, which in Fourier space gives a triple cosine product.

### 3.2 The lemniscatic connection

`G*` enters through `Gamma(1/4)^4`, which enters through the central binomial cube, which enters through the geometric series, which enters through the **multiplicative** structure of the BCC eigenvalue. If the eigenvalue were additive (like SC), the factorization would not work, `Gamma(1/4)` would not appear, and `G*` would not be the bridge constant.

The lemniscate constant `varpi = Gamma(1/4)^2 / (2 sqrt(2 pi))` and its bridge ratio `G* = 2 varpi / sqrt(pi)` are BCC facts. They emerge from the lattice Green's function at the BCC sublattice specifically, because that is the unique sublattice whose eigenvalue is fully multiplicative.

### 3.3 Implications for the observer formalism

From FOUND_POTENTIAL_CORE_AND_GENERATIVE_INTERIOR.md (Section 18): the self-referential loop (sLoop) requires BCC because self-referential closure requires the gap equation to close, which requires the BCC Watson integral.

This can now be stated more precisely: **the sLoop requires the multiplicative 3-axis coupling** because:

- Self-consistency demands the gap equation coefficient = `16 G*^2`
- This coefficient comes from the BCC propagator's factorized geometric series
- The factorization requires the eigenvalue to be a product of cosines
- Only BCC has this structure

A cluster engaging only SC (electromagnetic channel) or SC+FCC (electromagnetic + weak) cannot close the sLoop because their propagators do not generate `Gamma(1/4)^4`, and hence cannot produce `G*^2/(2pi)` as the self-energy coefficient.

---

## Part IV: The Zero Mode Topology

### 4.1 Zero modes by sublattice [THEOREM — algebraic]

The eigenvalue `sigma_S(k) = 0` determines the zero modes of each sublattice Laplacian:

- **SC:** `cos k_1 + cos k_2 + cos k_3 = 3` only at `k = (0, 0, 0)`. **1 zero mode.**
- **FCC:** `cos k_1 cos k_2 + cos k_1 cos k_3 + cos k_2 cos k_3 = 3` only at `k = (0,0,0)` and `k = (pi, pi, pi)`. **2 zero modes.**
- **BCC:** `cos k_1 cos k_2 cos k_3 = 1` at `k = (0,0,0)`, `(pi, pi, 0)`, `(pi, 0, pi)`, `(0, pi, pi)`. **4 zero modes.**

### 4.2 BCC zero modes are FCC reciprocal points [THEOREM — lattice theory]

The 3 extra BCC zero modes at `(pi, pi, 0)`, `(pi, 0, pi)`, `(0, pi, pi)` are exactly the **FCC reciprocal lattice vectors** of the BCC real-space lattice. This is a standard result in crystallography: the reciprocal lattice of BCC is FCC, and vice versa.

The 4 zero modes cause slow convergence of the BCC Green's function on finite tori (each zero mode contributes an IR contribution whose finite-L correction is O(1/L) and shrinks for arbitrarily large L). This is why numerical simulations at moderate lattice sizes (L <= 48) initially appeared to show SC as closer to the target — SC has only 1 zero mode and converges faster, but to the **wrong** value.

### 4.3 Physical interpretation [RESOLVED — combinatorial, not arithmetic-geometric]

**Simulation result** (`scripts/exploration/verify_zero_modes.py`): The zero mode counts follow a **2^k pattern** related to coupling depth, not a connection to elliptic curve automorphism groups.

| Sublattice | Zero modes | Pattern | |Aut(E)| of associated CM curve |
|------------|-----------|---------|-------------------------------|
| SC | 1 = 2^0 | sum of 0-fold products | no single CM curve |
| FCC | 2 = 2^1 | sum of 2-fold products | 6 (j=0, E: y^2=x^3-1) |
| BCC | 4 = 2^2 | 3-fold product | 4 (j=1728, E: y^2=x^3-x) |

The `|zero modes| = |Aut(E)|` match for BCC (4 = 4) is a **coincidence** that breaks at FCC (2 != 6). The actual pattern is combinatorial:

- **SC eigenvalue** `1 - (cos k_1 + cos k_2 + cos k_3)/3` has no inter-axis coupling. Only k = 0 gives sigma = 0. Zero modes: 2^0 = 1.
- **FCC eigenvalue** `1 - (cos k_1 cos k_2 + ...)/3` couples axes pairwise. The zone-boundary point (pi, pi, pi) satisfies all three pair-products = 1. Zero modes: 2^1 = 2.
- **BCC eigenvalue** `1 - cos k_1 cos k_2 cos k_3` couples all three axes multiplicatively. Points where an even number of k_mu = pi satisfy the product = 1. Zero modes: C(3,0) + C(3,2) = 1 + 3 = 4 = 2^2.

The zero mode count is `2^(floor(coupling_order/1.5))` or more precisely: the number of k-points in {0, pi}^3 where the structure factor equals its maximum (the coordination number). This is a topological property of the Brillouin zone, not an arithmetic property of elliptic curves.

---

## Claims Summary

| ID | Claim | Status |
|----|-------|--------|
| BMS-1 | BCC eigenvalue is `1 - cos k_1 cos k_2 cos k_3` (product) | [THEOREM] |
| BMS-2 | Geometric series factors across axes giving `[C(2m,m)/4^m]^3` | [THEOREM] |
| BMS-3 | Sum = `Gamma(1/4)^4/(4 pi^3) = G*^2/(2 pi)` | [THEOREM] |
| BMS-4 | BCC excites all 3 J-components -> SU(3) | [THEOREM] for counting; [SELECTION] for SU(3) identification |
| BMS-5 | Propagator and gauge structure share the multiplicative origin | [THEOREM] for structure |
| BMS-6 | sLoop requires BCC multiplicative coupling | [SELECTION] (depends on PI-C15) |
| BMS-7 | BCC has 4 zero modes at FCC reciprocal points | [THEOREM] |
| BMS-8 | Zero mode count is combinatorial (2^k pattern), not arithmetic-geometric | [THEOREM] — verified by simulation, |Aut(E)| match breaks at FCC |

---

## Cross-References

- [THEOREM_MOORE_LAYER_DECOMPOSITION.md](THEOREM_MOORE_LAYER_DECOMPOSITION.md) — Moore layer decomposition, gauge groups from J-component counting
- [../04_coupling/DERIV_WATSON_GSTAR_IDENTITY.md](../04_coupling/DERIV_WATSON_GSTAR_IDENTITY.md) — Watson's three integrals, BCC attribution, numerical confirmation
- [../03_derivations/DERIV_MOORE_GAUGE_STRUCTURE.md](../03_derivations/DERIV_MOORE_GAUGE_STRUCTURE.md) — Gauge group assignment by sublattice
- [../02_foundations/FOUND_POTENTIAL_CORE_AND_GENERATIVE_INTERIOR.md](../02_foundations/FOUND_POTENTIAL_CORE_AND_GENERATIVE_INTERIOR.md) — Observer formalism, sLoop requires BCC
- [../02_foundations/FOUND_SELF_REFERENTIAL_CLOSURE.md](../02_foundations/FOUND_SELF_REFERENTIAL_CLOSURE.md) — Gap equation as self-referential fixed point
