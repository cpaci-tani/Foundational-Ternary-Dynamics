# FTD-0721 — Derived interaction-graph transaction v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE VALIDATION RUN]`  
**Identifier:** `FTD-0721`  
**Date:** 2026-07-28  
**Parents:** `FTD-0503`, `FTD-0504`, `FTD-0516`, `FTD-0526`,
`FTD-0551`, `FTD-0598`, `FTD-0669`, `FTD-0719`, `FTD-0720`  
**Scope:** observer-only matter-sector discriminator; no production state,
default, toggle, scenario, particle identity, or electromagnetic claim changes.

## 1. Question locked before the validation campaign

Can matter connectivity change reversibly without storing a bond bit or event
history, if the interaction graph is instead derived from the instantaneous
constituent positions?  If so, what additional channel is still required for
an initially unbound two-constituent encounter to become permanently bound?

This protocol does not claim that the selected pair potential is derived from
the five postulates.  It tests a minimal existence construction inside the
already selected constituent phase-space branch.

## 2. Frozen candidate

For two equal-mass constituents with positions `x_1,x_2`, momenta `p_1,p_2`,
and opposite ternary polarities, set

\[
d=|x_1-x_2|^2,\qquad
H(p)=\sqrt{E_{\rm REST}^2+C_{\rm SPEED}^2|p|^2}.
\]

The interaction edge is an observer, not persistent state:

\[
(1,2)\in G(x)\quad\Longleftrightarrow\quad d<d_c,
\qquad d_c=\frac32.
\]

The selected compact radial well is

\[
U(d)=
\begin{cases}
-16\epsilon(d-\frac32)^2(d-\frac34),&d<\frac32,\\
0,&d\ge\frac32,
\end{cases}
\qquad \epsilon=10^{-2}.
\]

Thus `U(1)=-epsilon`, `U(0)=27 epsilon`, and both `U` and `dU/dd`
vanish at the support boundary.  The force is strictly finite-range and the
graph can change where the physical interaction and its first derivative are
already zero.

One step uses the production dispersion's exact kinetic discrete gradient and
the radial potential discrete gradient:

\[
\frac{x_a'-x_a}{h}
=\frac{C_{\rm SPEED}^2(p_a+p_a')}{H(p_a)+H(p_a')},
\]

\[
p_1'-p_1=-h\,\bar\nabla_1U,\qquad
p_2'-p_2=+h\,\bar\nabla_1U,
\]

\[
\bar\nabla_1U=
\frac{U(d')-U(d)}{d'-d}
\big[(x_1-x_2)+(x_1'-x_2')\big],
\]

with the analytic derivative used at `d'=d`.  The registered step is
`h=1/4`.  Only the zero-centre-momentum collinear sector is solved in v1; the
scalar root is part of the locked scope, not a general-scattering claim.

## 3. Analytic statements to check

1. The potential is `C1` at `d=d_c`, has its unique interior minimum at
   `d=1`, and is repulsive toward coincidence.
2. The discrete gradients imply exact pair-momentum conservation and
   `Delta(K+U)=0` conditional on a converged root.
3. Replacing `h` by `-h` gives the state-only inverse equation.
4. Because `G(x)` is a deterministic function of current positions, an edge
   appearance/disappearance requires neither a stored topology bit nor event
   history.
5. An encounter beginning outside support has nonnegative internal energy.
   Exact conservation forbids it from ending in the negative-energy bound
   sector.  Permanent capture therefore requires an energy receiver: the
   matched field, a third constituent, or an explicitly open environment.

The fifth item is an energy-threshold statement.  It does not assert that
every positive-energy trajectory escapes in arbitrary dimensions or under an
arbitrary potential.

## 4. Fresh validation arms

The exploratory construction values used before this lock are not validation
arms.  The locked campaign uses fresh momenta:

- scattering family: separation `R_0=1.30`, inward momentum magnitude
  `p_0=0.07`, 256 forward steps;
- bound family: separation `R_0=1`, momentum magnitude `p_0=0.015`, 256
  forward steps;
- all 13 unoriented Moore rays, represented by integer directions whose first
  nonzero component is positive;
- both polarity assignments `(+,-)` and `(-,+)`;
- origin and translated-center copies with translation `(4,-3,2)`;
- 256 state-only inverse steps using `h=-1/4` from every final state.

This gives `13 x 2 x 2 x 2 = 104` forward histories and 104 inverse histories.
No parameter, tolerance, arm, or classifier may be changed after the first
validation result is emitted.

## 5. Acceptance gates

For every step and arm:

- scalar root residual `<1e-13`;
- total-energy residual `<1e-12`;
- total-momentum residual `<1e-12`;
- equal-and-opposite impulse residual `<1e-12`;
- causal speed excess `<=1e-12`.

For every complete history:

- inverse state recovery `<1e-10`;
- translation covariance `<1e-12`;
- cubic-ray scalar-history spread `<1e-12`;
- scattering family begins outside the graph, enters it, leaves it, has
  exactly two graph transitions, and ends outside after 256 steps;
- bound family begins with negative internal energy and remains inside the
  graph for all 256 steps;
- scattering internal energy remains strictly positive and bound internal
  energy remains strictly negative, each with a sign margin `>1e-6`.

## 6. Locked verdict map

- All gates pass:
  `DERIVED_INTERACTION_GRAPH_REVERSIBLE_CAPTURE_REQUIRES_RESERVOIR`.
- Algebraic gates pass but any topology/inverse/covariance gate fails:
  `DERIVED_GRAPH_TRANSACTION_NUMERICALLY_UNRESOLVED`.
- Energy or momentum identities fail at a converged root:
  `DERIVED_GRAPH_DISCRETE_GRADIENT_CLOSED_NEGATIVE`.

Even the positive verdict licenses only a selected matter-only existence
construction.  It does not derive the potential, demonstrate physical bound
matter, permit annihilation, or establish that the current matched field can
actually absorb capture energy.  That field-capture transaction is the
separately registered successor if this campaign passes.
