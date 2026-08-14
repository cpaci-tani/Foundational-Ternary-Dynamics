# Theorem — Moore-coated compact-Hodge preimage and live-current scaffold trilemma v1

**Identifier:** `FTD-0921`  
**Date:** 2026-08-11  
**Status:** `[CLOSED NEGATIVE — COMPACT RELAXED PREIMAGE OF THE COATED SCALAR PLAQUETTE RETURN]` +
`[THEOREM — COMPACT TRANSVERSE RELAXED RETURN]` +
`[CLOSED NEGATIVE — NONZERO COMPACT TRANSVERSE RETURN WITH LIVE j=s v]` +
`[THEOREM — EIGHT-PARITY PERIODIC TERNARY SCAFFOLD CLASSIFICATION]` +
`[OPEN — GLOBAL BACKGROUND, INDEPENDENT CURRENT TYPE, OR LONGITUDINAL TAIL]`

## 1. Result

The FTD-0577 Moore coat removes the eight real zero/Nyquist components found
in FTD-0920, but that is not sufficient to make the coated plaquette return
local. Its longitudinal central-Hodge inverse contains an unavoidable central
Poisson denominator. An exact complex-Laurent witness proves that the coated
scalar-polarized return has no finite-support density/current preimage for
any real body stiffness.

This is not a universal compact-current no-go. A redesigned transverse field

\[
 J=\operatorname{curl}_c A
\]

has the exact compact relaxed return

\[
 (K-\kappa)J
 =\operatorname{curl}_c[(K-\kappa)A].
\]

The obstruction appears when that relaxed current is compiled into the live
production variables. Production ties current to manifested state:

\[
 j=s v.
\]

For a compact transverse source, the longitudinal source equation forces
`s=0`; the live tie then forces `j=0`. Thus no nonzero compact transverse
actuator exists in the current tied source pair.

The exact existing escape is nonlocal. On an even periodic lattice,
central-gradient-null ternary fields are arbitrary constants on the eight
site-parity classes. A scaffold nonzero on every class can gate any local
current while producing no density gradient. On the uncontained lattice,
however, every nonzero such scaffold extends everywhere. It is a global
manifested current medium, not a bounded particle or clock.

The framework therefore faces an explicit trilemma:

1. adopt and price a global two-periodic ternary background;
2. add an independent current type not tied to `s`; or
3. retain the existing ontology and seek a longitudinal ternary core with a
   noncompact localized tail.

The third route is the most conservative next branch. It changes neither the
source types nor the vacuum definition, but exact compact support must be
abandoned.

## 2. Why the eight real blind modes were not the whole locality test

Represent a finite-support lattice field by a Laurent polynomial. Let

\[
 d_i={z_i-z_i^{-1}\over2},
 \qquad
 D=d_x^2+d_y^2+d_z^2.
\]

After dividing out the nonzero constant `G_C`, the production source symbol
is

\[
 U=-d s+d\times j.
\]

The curl is algebraically transverse, so

\[
 \boxed{d\cdot U=-D s.}
\]

For a compact source, all quantities are Laurent polynomials. Hence
`d dot U` must be divisible by `D`. In particular it must vanish at every
complex Laurent point on the hypersurface `D=0`.

FTD-0920 checked only the real unit-torus points where all central
derivatives vanish. Those eight conditions characterize the global range on
a fixed even periodic quotient. The Laurent divisibility condition probes the
whole complex zero hypersurface and is strictly stronger.

The complex point used below is an algebraic proof device. It does not add a
complex physical field or change the actual-layer ontology.

## 3. Exact obstruction for the Moore-coated plaquette

Use

\[
 f=1-z_xz_y,
 \qquad
 B_M=\prod_i{1+c_i\over2},
 \qquad
 c_i={z_i+z_i^{-1}\over2},
\]

and the production stiffness

\[
 K={4\over3}-{2\over9}
 (c_x+c_y+c_z+c_xc_y+c_yc_z+c_zc_x).
\]

For one cardinal flux polarization, the required coated return is

\[
 U=t e_x,
 \qquad
 t=(K-\kappa)B_Mf.
\]

Set

\[
 a=\sqrt2,
 \qquad
 z_x=z_y=1+a,
 \qquad
 z_z=i(1+a).
\]

The inverse Laurent coordinates exist and give

\[
 c=(a,a,i),
 \qquad
 d=(1,1,ia).
\]

Therefore

\[
 D=1+1+(ia)^2=0.
\]

At the same point,

\[
 B_Mf=-{(1+a)^3(1+i)\over4}\ne0,
\]

while

\[
 K={4\over3}-{2\over9}
 \left(2+2a+i(1+2a)\right).
\]

Its imaginary part is

\[
 \operatorname{Im}K=-{2\over9}(1+2a)\ne0.
\]

For every real `kappa`, `K-kappa` is nonzero. Hence `t!=0`. But

\[
 d\cdot U=d_x t=t\ne0
\]

at a point where `D=0`, contradicting `d dot U=-Ds` for every Laurent
polynomial `s`. Thus

\[
 \boxed{
 (K-\kappa)B_Mf e_x
 \notin\operatorname{Ran}_{\rm compact}[-d,d\times]
 \quad\text{for every real }\kappa.}
\]

Cubic symmetry makes one failed cardinal polarization sufficient to exclude
the proposed isotropic body, which must support all three flux components.

This does not contradict FTD-0920. On a finite periodic box the inverse may
use a box-spanning central-Poisson solve. The theorem excludes an exact
finite-support preimage on the uncontained lattice.

## 4. Compact transverse return exists in the relaxed algebra

Let `A` be any compact vector potential and define

\[
 J=d\times A.
\]

Then `d dot J=0`. Because `K` is a scalar translation-invariant convolution,
it commutes with `d cross`:

\[
\begin{aligned}
 U_{\rm ret}
 &=(K-\kappa)J\\
 &=(K-\kappa)(d\times A)\\
 &=d\times[(K-\kappa)A].
\end{aligned}
\]

The relaxed source choice

\[
 s=0,
 \qquad
 j=(K-\kappa)A
\]

is exact and compact because both `K` and central curl are finite-range.

This is the constructive half of the theorem. Compact local recirculation is
compatible with the central operators themselves. It is not compatible with
the current production identification of the source current.

No `C4` body, stable orbit, formation law, or energy reservoir follows merely
from choosing a vector potential.

## 5. Why the live source tie kills the compact transverse construction

Production imposes

\[
 s\in\{-1,0,+1\},
 \qquad
 j=s v.
\]

Suppose a compact live source is transverse. Then

\[
 0=d\cdot U=-Ds.
\]

The finite-support Laurent ring is an integral domain and `D` is a nonzero
Laurent polynomial. Therefore

\[
 s=0.
\]

The live current is consequently

\[
 j=sv=0,
\]

and the whole source vanishes. Hence

\[
 \boxed{
 U\text{ compact},\ d\cdot U=0,\ U=-ds+d\times(sv)
 \Longrightarrow U=0.}
\]

This proof does not use ternary quantization. It remains true if compact `s`
is allowed arbitrary real values. The decisive constraint is that the same
field both generates the longitudinal source and gates the current.

## 6. Exact periodic scaffold escape

On a real even periodic lattice, the Fourier symbol corresponding to `-D` is

\[
 \sin^2k_x+\sin^2k_y+\sin^2k_z.
\]

It vanishes only when every coordinate is `0` or `pi`. Thus `ker D` has eight
dimensions. In real space,

\[
 \boxed{
 \ker D
 =\{s:s(x+2e_i)=s(x)\text{ for all }i\}.}
\]

These are precisely the fields constant on each of the eight parity classes
`x mod 2`. The exact `L=4` matrix has nullity eight and the eight parity-class
indicators are an independent basis; the same zero/Nyquist count holds on
`L=6`.

Ternary restriction gives

\[
 3^8=6561
\]

gradient-null parity scaffolds. Of these,

\[
 2^8=256
\]

are nonzero on all eight classes. For such a scaffold `s^2=1` everywhere.
Given a desired compact relaxed current `j`, choose

\[
 v={s j\over G_C}.
\]

Then

\[
 G_Csv=j,
 \qquad
 \nabla_c s=0,
\]

so the live production formula reproduces the compact curl source at the
algebraic level.

The price is global. A nonzero parity-class value repeats without bound.
Uniform `s=+1`, a full checkerboard sign choice, and all other fully
supporting assignments manifest the state field throughout the lattice. The
current may be localized through `v`, but its gating medium is not.

Calling this medium “vacuum” would alter the current actual/void ontology and
requires a declared background-energy subtraction, genesis/record semantics,
boundary treatment, and operational tests. None is supplied or selected by
this theorem.

## 7. The exact trilemma

The result can now be stated without ambiguity.

### Branch I — coated scalar body

The Moore coat satisfies the finite-periodic parity budgets but its
longitudinal return requires a central-Poisson inverse. Exact compact support
is closed negative.

### Branch II — independent transverse current

A curl carrier has an exact compact curl-current return. This branch is
mathematically constructive but requires current to be independently
available when density is zero.

### Branch III — live tied current

A nonzero compact transverse source cannot be compiled from `(s,sv)`. It can
be realized only by:

- a noncompact two-periodic ternary scaffold;
- a newly independent current/source type; or
- abandoning the pure-transverse/compact premise in favor of a longitudinal
  ternary core and/or a noncompact localized tail.

These alternatives are ontically distinct and must not be blended into one
unpriced “self-dual” mechanism.

## 8. Recommended next route

The minimal route preserving the existing ontology is the third option's
tail branch:

1. choose a compact live ternary core source;
2. solve the exact driven eigenmode equation

   \[
   (K-\kappa)J=U(s,sv);
   \]

3. take `kappa` outside the free band so the response is evanescent rather
   than radiative;
4. audit tail decay, source work, reaction, and reversal; and
5. test whether the source and tail can form one autonomous positive-energy
   recursive system.

FTD-0919 proved the free band ends at `16/9`. The exact one-tick quarter-turn
value `kappa=2` lies above it, so it is a natural first evanescent control.
This is a structural consequence, not a numerical near-match and not yet a
physical clock.

The global-scaffold and independent-current branches remain admissible only
after explicit type/selection accounting.

## 9. Epistemic boundary

This theorem does not derive a physical global background, a new current
type, a compact transverse `C4` body, formation, stability, reciprocal source
reaction, positive storage, finite total energy, mobility, `G*`, gamma, Born
frequencies, Bell correlations, or preferred-tick hiding. It changes no
production source or default.

The positive compact-transverse result is a relaxed source theorem. The
periodic scaffold is an exact implementation identity with an unaccepted
global ontic cost. Neither is substrate evidence for a clock.

## 10. Verification

The locked preregistration is
`PREREG_MOORE_COATED_COMPACT_HODGE_PREIMAGE_AND_LIVE_CURRENT_SCAFFOLD_TRILEMMA_v1.md`
with SHA-256
`8E29F7F667F3A96AC550CC30276D7E1B6AC119D7207C4CD3E11BA73A430ABC54`.

The independent exact certificate is
`scripts/proofs/proof_moore_coated_compact_hodge_preimage_live_current_scaffold_trilemma.py`
with SHA-256
`AC4F15BBC9F819C0B6AE7CB3D53E9BD62307C448E32825BCDBF085A06889913E`.

It passes `69/69` exact gates and reports

```text
OUTCOME=A_COMPACT_SOURCE_LIVE_TIE_TRILEMMA
COATED_SCALAR_PLAQUETTE_COMPACT_RELAXED_PREIMAGE=FALSE
TRANSVERSE_CARRIER_COMPACT_RELAXED_RETURN=TRUE
TRANSVERSE_CARRIER_COMPACT_LIVE_TIED_RETURN=FALSE
PERIODIC_TERNARY_SCAFFOLDS=6561
FULLY_SUPPORTING_TERNARY_SCAFFOLDS=256
NONZERO_LIVE_TRANSVERSE_ESCAPE=GLOBAL_SCAFFOLD_OR_NEW_TYPE_OR_TAIL
PRODUCTION_CHANGED=FALSE
GSTAR_USED=FALSE
BORN_BELL_CONTEXT_USED=FALSE
```
