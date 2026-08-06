# FTD-0719 — Polarity-snapshot current non-uniqueness theorem

**Status:** `[THEOREM — SELECTED FACE-CURRENT COMPLEX]`  
**Numerical witness:** `[EXACT FINITE-LATTICE CONSTRUCTION]`  
**Production status:** unchanged

## Statement

Let `rho_0` and `rho_1` be two signed quadratic-coat densities on the sites of
a periodic cubic lattice.  Suppose two charge-preserving collections of causal
straight segments connect the same unordered signed endpoint sets and deposit
oriented face currents `I_1` and `I_2`.  Exact continuity gives

\[
\rho_1-\rho_0+D I_1=0,
\qquad
\rho_1-\rho_0+D I_2=0,
\]

where `D` is the backward face divergence.  Therefore

\[
D(I_1-I_2)=0.
\]

The endpoint snapshots determine the divergence of the current, but not its
component in `ker D`.  In the registered quadratic-coat construction this
ambiguity is nontrivial: there exist causal, neutral histories with the same
endpoint densities for which

\[
I_1-I_2\ne0,
\qquad
C^T(I_1-I_2)\ne0,
\]

where `C^T` is the matched face-to-edge curl adjoint.

Consequently an unordered polarity snapshot is not, by itself, a dynamically
complete Markov state for the face/edge matter–field theory.

## Proof

The divergence result follows by subtracting the two exact continuity
identities.  The remaining claim requires only one nonzero witness.

FTD-0719 uses a neutral four-constituent configuration.  The positive pair has
two admissible correspondences between the same start and endpoint sets:
direct and crossed.  The negative pair follows the same direct paths in both
histories.  Every segment has length `sqrt(1/8)<1/sqrt(3)`.

The engine result is:

```text
start-density difference              0
endpoint-density difference           0
maximum continuity residual           1.3877787807814457e-17
divergence of current difference       0
current-difference L2                  0.065713296254924697
curl-adjoint-difference L2             0.13503426576127817
total-current-moment difference        0
<Delta I, Delta I>                     0.0043182373046875
reversal residual                      0
24-rotation covariance residual        0
integer-translation covariance         0
```

Thus the difference is a pure cycle current: it changes neither endpoint
density nor total transport moment, but it is nonzero, has nonzero curl, and
couples to an oriented-face connection.  This proves the claim. `QED`

## Exact consequence for the field update

For a common pre-current field, the matched update has

\[
E_1=E_{\rm pre}-I.
\]

The two histories therefore produce fields differing by `-(I_1-I_2)`.  Their
Gauss divergences agree because `D(I_1-I_2)=0`, while their transverse field
content differs because `C^T(I_1-I_2)\ne0`.  Gauss and endpoint polarity do
not erase the physical distinction.

## Ontological corollary

Persistent constituent labels are not forced, but bare endpoint sets are
insufficient.  A complete dynamics must contain exactly one of the following
equivalent kinds of information:

1. a deterministic correspondence/current selected uniquely by the common
   action;
2. an explicit oriented current/history variable;
3. a connection or holonomy variable carrying the same cycle information.

This is a trilemma, not a selection among the three. FTD-0503 already
constructs route 1 in the free distinct-endpoint phase-space sector, and
FTD-0720 witnesses one interacting root basin for the smallest connected
neutral composite. Neither result proves global action uniqueness, and the
coincident-target collision sector remains open.

## Scope

The theorem is conditional on the selected quadratic-coat face-current
representation.  Its linear-algebraic core—continuity fixes current only up to
`ker D`—holds for any discrete continuity complex with nontrivial cycle space.

The theorem does not derive electromagnetism, particle identity, spin,
statistics, mass, or a stable matter orbit.  It does not license a new
primitive before uniqueness of a common-action selector has been tested.
