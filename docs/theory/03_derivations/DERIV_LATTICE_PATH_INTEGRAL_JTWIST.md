# DERIV - Lattice Path Integral and J-Twisted Screening Self-Energy

**Tag:** [DERIVATION] / [OPEN]
**Date:** 2026-05-30
**LEDGER:** FTD-0245 [Mechanism B: Lattice-to-Continuum Matching]

## 1. The Physics Objective

The core objective of Mechanism B is to extract the continuous physical coupling (the master quadratic root $x$) by performing a rigorous lattice-to-continuum matching. 

In standard Quantum Field Theory, the fully dressed observable coupling $x$ is given by the Schwinger-Dyson (gap) equation, which resums the 1-particle irreducible (1PI) vacuum polarization (the screening self-energy $\Pi(x)$):

$$ x_{\text{dressed}} = x_{\text{bare}} + \Pi(x_{\text{dressed}}) $$

Our goal is to compute the screening self-energy $\Pi(x)$ directly from the actual $J$-twisted lattice update on the complex vector space $V_{\text{complex}} \cong \mathbb{Z}[i]^2$.

## 2. The Master Quadratic as a Schwinger-Dyson Equation

Consider the FTD master quadratic:
$$ x^2 - 16(G^*)^2 x + 16(G^*)^3 = 0 $$

If we divide by $x$, we can rewrite this algebraically as:
$$ x = 16(G^*)^2 - \frac{16(G^*)^3}{x} $$
$$ x = 16(G^*)^2 \left( 1 - \frac{G^*}{x} \right) $$

This is precisely the structure of a **Schwinger-Dyson gap equation**! 
By matching the terms:
1. **Bare Lattice Coupling:** $x_{\text{bare}} = 16(G^*)^2$. This is the raw geometric/topological transfer scale of the bare lattice (derived from the $\mathbb{Z}[i]^2$ automorphism order and Watson integral).
2. **Screening Self-Energy:** $\Pi(x) = - \frac{16(G^*)^3}{x}$. This represents the 1-loop (and resummed) vacuum polarization that screens the bare charge.

Therefore, deriving the master quadratic from first principles (Mechanism B) is exactly equivalent to proving that the 1-loop screening self-energy of the $J$-twisted lattice evaluates to $\Pi(x) = -16(G^*)^3 / x$.

## 3. The Path Integral on $V_{\text{complex}}$

We define the partition function over the FTD lattice flux field $J$ and state field $s$:
$$ Z = \int \mathcal{D}J \mathcal{D}s \, e^{-S_E[J, s]} $$

Because the classical action $S_E$ is ultralocal in the state field $s$ (depending only on the count of manifested voxels), the classical variation cannot fix the coupling. The dynamics only emerge when we compute the **quantum fluctuation determinants** over the $J$-twisted boundary conditions:

$$ \psi(\phi + 2\pi) = J\psi(\phi) $$

This restricts the internal loop momenta $p$ in the Feynman diagrams to the quarter-integer spectrum $D_{1/4}$ and $D_{3/4}$.

## 4. The Loop Integral (Target Computation)

The self-energy $\Pi(q)$ is given by integrating the lattice propagator over the J-twisted Brillouin zone. 
In a discrete theory, the propagator is $\Delta(p) = \frac{1}{4 \sin^2(p/2)}$. 

Because the vertices are dressed by the state-field genesis rule, the loop integral is:
$$ \Pi(x) = \sum_{p \in D_{1/4}, D_{3/4}} V(p, x) \Delta(p) \Delta(p+q) V(p, x) $$

Our strict proof obligation is to rigorously evaluate this sum over the quarter-twisted spectrum and show that it natively converges to $-16(G^*)^3 / x$, utilizing the established Lerch identity $\det_\zeta \{n + a\} = \sqrt{2\pi}/\Gamma(a)$.

If this integral dynamically yields the target without external parameter insertion, Mechanism B is solved, and the operational readout is elevated to a `[THEOREM]`.
