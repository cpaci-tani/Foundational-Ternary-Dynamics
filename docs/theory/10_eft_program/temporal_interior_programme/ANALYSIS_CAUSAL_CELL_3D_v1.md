# ANALYSIS — The Causal Cell in Three Dimensions: Past, Future, and the Region Continuum Relativity Does Not Have

**Status:** `[DERIVED — EXACT GEOMETRY AND CENSUS]` +
`[MEASURED — MAX GROUP VELOCITY AND PRECURSOR PROFILE]` +
`[BOOKED — FTD-0811]`
**Date:** 2026-08-08 · **Artifact:** `scripts/experiments/temporal_interior/toy3d_causal_structure.py`
**Figure:** `dissemination/papers/semantic_ontology/figures/fig10_causal3d.pdf`
**Parents:** `ANALYSIS_CONE_SPEED_CHARACTERIZED_v1.md` (the containment result),
`ANALYSIS_MINIMAL_TEMPORAL_INTERIOR_TOY_v1.md` (panel a, the 2-D version),
FTD-0407 (`C_SPEED = 1/√3`).
**Production impact:** none. No constant is changed; no tag moves.

---

## 1. The question

Continuum relativity partitions spacetime around an event into three
regions — causal past, causal future, and elsewhere — separated by one
surface, the light cone. A discrete substrate does not have three, because
two different structures compete to define causal reach and they do not
coincide:

- **Reach** — the update rule touches a finite neighbour set each tick, so
  after $t$ ticks the field is *exactly* zero outside the $t$-fold
  dilation of the causal polytope. This bound is **unconditional**: it is
  locality, not dynamics, and holds for every initial condition.
- **Light cone** — the effective wave speed is $C = 1/\sqrt3$, so a
  disturbance travels a distance $Ct$. This bound is **dynamical**.

Between them lies a shell that is formally reachable but outside the
effective cone. This document establishes exactly what lives there.

## 2. The geometry at $t=1$, exactly

| neighbourhood | polytope | inradius | circumradius |
|---|---|---|---|
| von Neumann (6) | octahedron | $1/\sqrt3 = 0.577350$ | $1$ |
| M18 face+edge (18) | cuboctahedron | $1$ | $\sqrt2 = 1.414214$ |
| Moore (26) | cube | $1$ | $\sqrt3 = 1.732051$ |
| — | light cone at $t=1$ | $C = 0.577350$ | — |

> **The tangency.** The octahedron's inradius **is** the cone speed. Its
> eight faces are therefore tangent to the light-cone sphere — the sphere
> is inscribed in the octahedron, which is itself inscribed in the
> cuboctahedron and the cube. This is the geometric content of
> $C = \min\{\text{containment bounds}\}$: the cone fits inside every
> candidate causal polytope, and it fits the tightest one *exactly*.

The production stencil is M18, so the reach at $t=1$ is the cuboctahedron
— the convex hull of the 6 face and 12 edge neighbours, equivalently
$\{|x|_\infty \le 1\} \cap \{|x|_1 \le 2\}$. The corresponding lattice
distance is
$$d_{18}(n) = \max\big(|n|_\infty,\ \lceil |n|_1/2 \rceil\big),$$
which is the fewest ticks in which the update rule can touch site $n$.

## 3. The census: the cone is empty at $t=1$

Counting lattice sites by region:

| $t$ | inside the cone | precursor shell | reachable |
|---|---|---|---|
| 1 | **1** | 18 | 19 |
| 2 | 7 | 86 | 93 |
| 3 | **27** | 236 | 263 |
| 4 | 57 | 512 | 569 |
| 5 | 93 | 958 | 1051 |
| 10 | 799 | 6702 | 7501 |

Two exact facts stand out.

**At $t=1$ the light cone contains exactly one lattice site — the origin
itself — while the update rule has already touched eighteen.** Every site
the substrate can influence in its first tick lies outside its own light
cone. The nearest lattice site is at distance $1$; the cone has radius
$0.577$. The cone becomes populated only at $t=2$, when its radius
$2/\sqrt3 = 1.155$ first exceeds $1$ and admits the six face neighbours.

**At $t=3$ the cone contains exactly the 27-site Moore block.** This is
not approximate: $C\cdot 3 = 3/\sqrt3 = \sqrt3$, which is exactly the
body-diagonal distance, so the cone boundary passes precisely through the
eight corners. The $3^3$ cell — the framework's own existential unit — is
the first configuration the light cone closes over, and it closes over it
exactly.

The reading is not that causality fails at one tick. It is that **the
effective light cone is a coarse-grained object with no content below
$t \approx \sqrt3$ ticks.** Asking where the cone is at $t=1$ is asking a
question below the resolution at which the concept was defined.

## 4. The group-velocity cone is effective, not the strict front

For the leapfrog
dispersion $\Omega(k) = 2\arcsin\!\big(\tfrac{C}{2}\sqrt{-L(k)}\big)$ the
group velocity was differentiated analytically,
$$\frac{\partial\Omega}{\partial k_i}
= \frac{C\,\sin k_i\,(1+\cos k_j+\cos k_l)}
       {3\sqrt{1-S^2}\,\sqrt{-L}}, \qquad S = \tfrac{C}{2}\sqrt{-L},$$
and scanned over a $181^3$ grid of the irreducible zone. The result:
$$\max_{k\neq0} |\nabla_k\Omega| = 0.999975\,C .$$
The grid finds no sampled value above $C$ and approaches $C$ as $k\to0$,
where the expression has a removable limit. This is evidence about the
effective packet/group-velocity cone, not a proof of the global supremum.
More importantly, even an exact group-velocity supremum would not be a
strict support theorem: the Green function below is nonzero outside $Ct$.
The exact signal/support front is the reach boundary of the finite update.

## 5. What is actually in the shell

The Green's function settles it. A point source is the only unambiguous
probe: its initial support is a single site, so its strict support at time
$t$ is exactly the reach polytope and no padding convention is required.
At $t=10$ (cone radius $5.774$, support radius $\sqrt2\cdot10 = 14.142$):

| $r$ | region | $\max|\phi|$ |
|---|---|---|
| 0.5 | signal | $2.48\times10^{-1}$ |
| 4.5 | signal | $2.41\times10^{-2}$ |
| 6.5 | precursor | $5.78\times10^{-3}$ |
| 8.5 | precursor | $4.06\times10^{-4}$ |
| 10.5 | precursor | $2.73\times10^{-6}$ |
| 12.5 | precursor | $2.69\times10^{-9}$ |
| 14.1 | precursor | $2.80\times10^{-13}$ |
| $>14.142$ | beyond reach | $0$ **exactly** |

The amplitude falls **10.3 decades across the shell**, roughly $1.3$
decades per lattice cell, and then terminates in a machine-exact zero that
the artifact asserts rather than reports. This is precursor-like:
present, calculable, and strongly suppressed in the reported finite-time
profile, outside the effective group-velocity cone but inside exact support.

So the bookkeeping has four regions: effective-cone interior, precursor
shell, exact support boundary and unreached exterior. The run shows about
ten decades of suppression across the shell and then an exact zero. It
does not establish a uniform exponential bound in time or physical
harmlessness.

> **A measurement error worth recording.** The first attempt probed the
> shell with a *Gaussian* source and reported a flat $\approx 3\%$ of
> amplitude "beyond the cone", independent of source width — which would
> have been a serious result had it been true. It was an artifact: a
> Gaussian has infinite tails, and the padding used to define "beyond"
> ($2\sigma$) sat inside the front's own shoulder (the $99\%$ energy
> radius of the initial profile is $\approx 2.1\sigma$), so the
> measurement was clipping the signal and counting it as leakage. The
> front position was independently confirmed to track $Ct$ to within the
> source extent before the probe was replaced. The point-source
> formulation has no such convention and is the one reported above.

## 6. What this settles, and what it does not

It settles, as exact geometry: the tangency $C = $ octahedral inradius;
the $t=1$ census; the exact closure of the cone over the $3^3$ block at
$t=3$; the strict support bound at every $t$.

It establishes, as measurement on this stencil: that a $181^3$ scan found
no group velocity above $C$, supporting an effective cone; and that the
precursor amplitude falls by ten decades across the shell in the reported
run. Neither statement makes $Ct$ the strict front.

It does not bear on the constitutional question left open by
`ANALYSIS_CONE_SPEED_CHARACTERIZED_v1.md` — which neighbourhood *defines*
causal reach. The tangency makes the octahedral reading geometrically
distinguished, but P4 commits the framework to Moore causality, and the
production wave operator is M18. The value $1/\sqrt3$ remains the
strictest containment bound over all three readings, which is why it needs
no such commitment; it does not thereby become forced by any one of them.

Nor does it license any claim about Lorentz invariance. A contained cone
with a subluminal spectrum is a *necessary* condition for the free-sector
result, not a sufficient one; the outstanding obligation is the two-body
one recorded in `DERIV_TWO_OWED_PROOFS_v1.md`.

## 7. Reproduction

```
python scripts/experiments/temporal_interior/toy3d_causal_structure.py
```

Roughly one minute. Deterministic throughout — the only stochastic
element would be the source, and it is a point. The run asserts locality
(`beyond the reach polytope: identically zero`) and fails loudly if the
stencil is ever changed in a way that breaks it.
