# FTD-0721 — Derived interaction graph and capture threshold

**Status:** `[SELECTION — COMPACT PAIR WELL] + [THEOREM — CONDITIONAL
DISCRETE-GRADIENT IDENTITIES] + [MEASURED — REVERSIBLE GRAPH CHANGE]`  
**Verdict:** `DERIVED_INTERACTION_GRAPH_REVERSIBLE_CAPTURE_REQUIRES_RESERVOIR`  
**Production status:** unchanged

## Statement

Within the selected constituent phase-space branch, a bond or interaction edge
need not be a persistent primitive.  Let

\[
G(x)=\{(i,j):|x_i-x_j|^2<3/2\}.
\]

For the registered two-constituent compact potential

\[
U(d)=
\begin{cases}
-16\epsilon(d-3/2)^2(d-3/4),&d<3/2,\\
0,&d\ge3/2,
\end{cases}
\qquad \epsilon=10^{-2},
\]

the interaction graph is a deterministic observer of current positions.  It
can gain or lose an edge under a reversible, local, energy- and
momentum-preserving transaction without storing a bond bit or event history.

The same construction also proves a formation boundary.  A closed pair
beginning outside the support has nonnegative internal energy, whereas the
registered bound sector has negative internal energy.  Exact conservation
therefore forbids permanent capture into that sector.  Formation requires an
energy receiver: the face/edge field, a third constituent, or an explicitly
open environment.

## Algebra

The well has

\[
U(0)=27\epsilon,\qquad U(1)=-\epsilon,
\qquad U(3/2)=U'(3/2)=0,
\]

and, on the interior,

\[
U'(d)=-48\epsilon(d-3/2)(d-1),\qquad U''(1)=24\epsilon>0.
\]

Thus it has a repulsive coincidence core, a unique minimum at squared
separation one, and a `C1` compact-support boundary.

For `H(p)=sqrt(E_REST^2+C_SPEED^2|p|^2)`, use the exact kinetic discrete
gradient

\[
\bar v(p,p')=
\frac{C_{\rm SPEED}^2(p+p')}{H(p)+H(p')}
\]

and the radial potential discrete gradient

\[
\bar\nabla_1U=
\frac{U(d')-U(d)}{d'-d}(r+r'),
\qquad r=x_1-x_2.
\]

The step equations are

\[
x_a'-x_a=h\bar v_a,
\qquad
p_1'-p_1=-h\bar\nabla_1U,
\qquad
p_2'-p_2=+h\bar\nabla_1U.
\]

The impulses cancel exactly.  Moreover,

\[
\Delta K
=\bar v_1\mathbin\cdot\Delta p_1
 +\bar v_2\mathbin\cdot\Delta p_2
=-(r'-r)\mathbin\cdot\bar\nabla_1U
=-\Delta U.
\]

Replacing `h` by `-h` exchanges the endpoint equations.  Conditional on the
registered unique scalar root, the inverse uses only the later state.

## Engine witness

The fresh locked campaign contains 104 histories: two energy families, 13
unoriented Moore rays, both polarity assignments, and origin/translated
copies.  Every history runs 256 steps and then 256 inverse steps.

```text
arms passing                         104 / 104
maximum root residual                9.971190539914687e-15
maximum energy residual              4.996003610813204e-16
maximum momentum residual            0
maximum impulse-balance residual     0
maximum kinematic residual           3.129441150662160e-15
maximum causal-speed excess          0
maximum inverse recovery             1.376676550535194e-13
maximum scalar-history spread        2.522426711948356e-13
scattering graph transitions         2 in every arm
bound active ticks                   256 in every arm
```

The scattering family starts and ends outside support, entering and leaving
once.  Its internal energy remains approximately `+0.00945776`.  The bound
family stays inside support and remains approximately `-0.00955997`.

## Ontological consequence

This separates three ideas that the fixed-edge prototype had conflated:

1. constituents and their phase-space records are persistent in this branch;
2. connectivity is a derived relation on those records;
3. formation is an energy-transfer process, not merely the appearance of a
   graph edge.

Objects may therefore change relational topology without adding a bond
primitive. FTD-0722 subsequently constructs the required common
matter–field transaction and verifies Gauss, current, total energy, recoil
symmetry, locality, and state-only inversion. Its locked `p=0.07` encounter
exports energy but does not reach the negative sector. The remaining formation
question is therefore quantitative sufficiency of the transfer channel, not
whether a graph-edit primitive is needed.

## Scope

The potential is selected, not derived from the five postulates.  The v1
solver covers an equal-mass, zero-COM, collinear, non-crossing pair sector.  It
does not establish physical matter formation, general scattering,
annihilation, constituent-count change, electromagnetism, or a particle pole.
It proves an existence result for derived connectivity and a conservative
capture obstruction.
