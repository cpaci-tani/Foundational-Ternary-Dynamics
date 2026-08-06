# FTD-0561 — Periodic-Hop Source Multipole Hierarchy v1

**Status:** [PRE-REGISTRATION — LOCKED/RUN; POSITIVE MULTIPOLE THEOREM]
**Date locked:** 2026-07-26
**Scope:** observer-only extension of the FTD-0560 axial `l=1` resonance to
finite rigid manifested source profiles
**Production changes:** none

## 1. Question

Does spatial extension remove the slow-hop resonant source found by FTD-0560,
or does cancellation require microscopic neutrality and higher multipole
conditions?

## 2. Frozen source

Let a finite rigid axial profile have integer weights `s_n` and form factor

\[
 S(u)=\sum_n s_n e^{iun}.
\]

Its axial moments are

\[
 M_r=\sum_n s_n n^r.
\]

At the FTD-0560 `T>=3`, `l=1` resonance `u_T`, the curl source vanishes and
the exact normalized forcing is

\[
 A_T/G_C=\frac{\sqrt3}{T}\sin u_T\,|S(u_T)|.
\]

If `m` is the first index with `M_m != 0`, Taylor expansion gives

\[
 S(u)=\frac{(iu)^m}{m!}M_m+O(u^{m+1}).
\]

Together with

\[
 u_T=2\pi\sqrt3/T+O(T^{-2}),
\]

the locked asymptotic theorem is

\[
 \boxed{
 A_T=G_C\frac{\sqrt3(2\pi\sqrt3)^{m+1}}
 {m!}\frac{|M_m|}{T^{m+2}}
 +O(T^{-(m+3)})}.
\]

In particular:

- nonzero net polarity `Q=M_0` forces
  `A_T=6pi G_C |Q|/T^2+O(T^-3)`;
- a neutral axial dipole forces
  `A_T=12sqrt(3)pi^2 G_C |M_1|/T^3+O(T^-4)`;
- if charge and dipole vanish, the first quadrupole term is
  `A_T=36pi^3 G_C |M_2|/T^4+O(T^-5)`.

Therefore rigid spatial extension alone cannot remove the universal charged
term.  Neutrality raises the suppression order but does not generically make
the source identically zero.

## 3. Exact cancellation condition on the axial witness

Group sites by axial coordinate and define plane sums

\[
 a_n=\sum_{x_i=n}s_i.
\]

Then `S(u)=sum_n a_n exp(iun)` is a finite Laurent polynomial.  It vanishes
for an interval of axial momenta if and only if every `a_n=0`.  Thus exact
cancellation of the complete axial resonance family requires zero net
polarity in every plane normal to the hop direction.

This condition cancels only the axial witness.  A radiationless dressing must
make the full three-dimensional form factor vanish on every resonant surface.

## 4. Locked observer profiles

Use four exact one-dimensional profiles:

1. point: `{0:+1}`, first moment order `m=0`, `M_0=1`;
2. same-sign pair: `{0:+1,1:+1}`, `m=0`, `M_0=2`;
3. dipole: `{0:+1,1:-1}`, `m=1`, `M_1=-1`;
4. quadrupole: `{-2:-1,-1:+1,1:+1,2:-1}`, `m=2`, `M_2=-6`.

For `T in {32,64,128,256}`, all three axial orientations, and both global
polarity mirrors, run exactly `4*4*3*2=96` arms.

Require:

- resonance denominator at or below `1e-12`;
- exact form factor versus direct site sum residual at or below `1e-12`;
- detected first nonzero moment order and coefficient exactly match the
  registered profile;
- polarity and cubic-rotation residuals at or below `1e-12`;
- every forcing amplitude is strictly positive;
- each profile's asymptotically normalized sequence is strictly increasing;
- at `T=256`, relative asymptotic errors are below `0.01` for the point and
  pair, `0.02` for the dipole, and `0.03` for the quadrupole.

Also register the transverse same-plane dipole
`{(0,0,0):+1,(0,1,0):-1}`.  Its axial form factor must vanish exactly, while
its FTD-0560 `T=1` oblique form factor must be nonzero.  This demonstrates
that cancelling one witness is not cancellation of the full resonance
surface.

## 5. Verdicts

- `HOP_SOURCE_MULTIPOLE_HIERARCHY_DERIVED`: all exact identities,
  cardinalities, asymptotic gates, mirrors, rotations, and the same-plane
  counterexample pass.
- `HOP_SOURCE_MULTIPOLE_HIERARCHY_FAILED`: any locked gate fails.

Passing closes “make a charged point wider” as a radiationless linear cure.
It does not close neutral extended, internally deforming, nonlinear,
defect-bound, or topological carriers and licenses no production change.

## 6. Execution record

The protocol was locked before compilation and execution with SHA-256
`166E917FED9DE4F568E06F7DC8F8601C2BE93C9FC244F6A797CA912A3F3AB13F`.
All 96 arms passed.  The point, pair, dipole, and quadrupole normalized
`T=256` errors were `0.00697`, `0.00719`, `0.01366`, and `0.02090` against
their locked gates.  The same-plane dipole cancelled the axial witness
exactly but retained oblique amplitude `0.141480289`.  Verdict:
`HOP_SOURCE_MULTIPOLE_HIERARCHY_DERIVED`.
