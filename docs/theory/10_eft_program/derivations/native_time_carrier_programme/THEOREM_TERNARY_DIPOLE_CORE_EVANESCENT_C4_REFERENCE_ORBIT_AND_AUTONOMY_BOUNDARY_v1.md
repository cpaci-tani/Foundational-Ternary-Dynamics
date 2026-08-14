# Theorem — Ternary-dipole-core evanescent `C4` reference orbit and autonomy boundary v1

**Identifiers:** `FTD-0922`, repaired execution `FTD-0923`  
**Date:** 2026-08-11  
**Status:** `[THEOREM — OUTSIDE-BAND EVANESCENT RESOLVENT AND TAIL BOUND]` +
`[REFERENCE CONSTRUCTION — EXACT SOURCE-LOCKED C4 FIELD ORBIT]` +
`[THEOREM — CONSTANT CIRCULATION AND ZERO FIELD-SIDE MIDPOINT SOURCE WORK]` +
`[CLOSED NEGATIVE — FROZEN ZERO-CURRENT SOURCE SNAPSHOTS SATISFY CONTINUITY]` +
`[OPEN — AUTONOMOUS CORE MOTION, REACTION, AND STORAGE]`

## 1. Result

A compact ternary source can support an exact, spatially localized recursive
field pattern without a new field type or a global ternary background.

Take the exact order-four stiffness

\[
 \kappa=2.
\]

The free C18 stiffness band ends at `16/9`, so `2` lies in a spectral gap.
The resolvent

\[
 R_2=(2I-K)^{-1}
\]

therefore exists and maps every compact source to an `ell^2` evanescent field
with the rigorous graph-distance tail bound

\[
 \boxed{
 \|P_{\ge r}R_2q\|_2
 \le {9\over2}\left({8\over9}\right)^r\|q\|_2.}
\]

Use a neutral two-site ternary dipole and its four quarter-rotated snapshots.
The coded electric source from each snapshot determines an evanescent field
profile. Those four profiles form an exact source-locked `C4` kick--drift
orbit with nonzero constant modal circulation.

The field-side maintenance work is exactly zero on every tick. The return
source is radial in the degenerate field plane while the midpoint field
increment is tangential. This is the first exact realization in this branch
of the proposed “stable recursive” geometry: constraint and circulation are
orthogonal, so the ideal return redirects the field without changing its
free invariant.

The construction is not autonomous. The frozen source uses zero velocity
while its ternary snapshot changes. Its central-continuity residual is
nonzero on every transition. Source motion, source reaction, switching work,
and a positive source reservoir remain missing.

Thus the field gearbox is now explicit; the remaining gearbox debt lies in
the matter/source core.

## 2. Exact spectral gap and resolvent

Let

\[
 K=-{1\over3}\Delta_{18}.
\]

FTD-0919 proved

\[
 K=K^T,
 \qquad
 0\le K\le {16\over9}I.
\]

At `kappa=2`,

\[
 2-{16\over9}={2\over9}>0.
\]

Consequently,

\[
 \left\|{K\over2}\right\|={8\over9}<1
\]

and the norm-convergent Neumann series is

\[
 \boxed{
 R_2=(2I-K)^{-1}
 ={1\over2}\sum_{m=0}^{\infty}\left({K\over2}\right)^m.}
\]

The resolvent norm is bounded sharply from the spectral interval by

\[
 \|R_2\|={1\over2-16/9}={9\over2}.
\]

## 3. Rigorous evanescent tail

The C18 stencil connects one site to its six face and twelve edge neighbors.
It has graph range one. If `q` is supported on `S`, then

\[
 K^m q
\]

is supported within C18 graph distance `m` of `S`. Let `P_{>=r}` project
outside distance `r-1`. Terms with `m<r` vanish after projection. Therefore

\[
\begin{aligned}
 \|P_{\ge r}R_2q\|_2
 &\le {1\over2}\sum_{m=r}^{\infty}
 \left({8\over9}\right)^m\|q\|_2\\
 &={9\over2}\left({8\over9}\right)^r\|q\|_2.
\end{aligned}
\]

This is a volume-independent exponential `ell^2` bound. It is not an optimal
pointwise asymptotic and does not make the field compact. FTD-0919 and
FTD-0921 exclude exact compact support for the relevant unchanged operators.

## 4. Compact ternary source core

Let

\[
 s_0=\delta_{e_x}-\delta_{-e_x}.
\]

It is neutral, ternary, and supported on two sites. Let `S` denote the active
right-handed quarter-turn about the `z` axis, including the rotation of vector
components, and define

\[
 s_n=S^n s_0.
\]

Then

\[
 s_{n+2}=-s_n,
 \qquad
 s_{n+4}=s_n.
\]

For the uncontained source,

\[
 \nabla_c s_0
\]

has vector support on exactly eleven sites and

\[
 \|\nabla_c s_0\|_2^2={7\over2}.
\]

These values are exact finite sums. On the separate `L=4` periodic witness,
the `+2` and `-2` axial lobes identify, so the corresponding periodic support
and norm are ten and four. FTD-0922's first verifier accidentally used these
periodic values for the uncontained checks; FTD-0923 repairs only that domain
mix-up.

Set the reference snapshot velocity to zero. The coded source is then

\[
 U_n=-G_C\nabla_c s_n.
\]

## 5. Evanescent field profiles

Define

\[
 q_n=G_C\nabla_c s_n,
 \qquad
 F_n=R_2q_n.
\]

The resolvent equation gives

\[
 (2I-K)F_n=q_n.
\]

Hence the actual coded source satisfies

\[
 \boxed{U_n=(K-2I)F_n.}
\]

The scalar convolution `K`, central gradient, and `R_2` commute with cubic
rotations. Therefore

\[
 F_{n+1}=SF_n,
 \qquad
 F_{n+2}=-F_n,
 \qquad
 F_{n+4}=F_n.
\]

The profiles are nonzero. Since `S` is orthogonal and `S^2=-I` on their
two-plane,

\[
 \|F_0\|=\|F_1\|,
 \qquad
 \langle F_0,F_1\rangle=0.
\]

An exact rational Fourier construction on `(Z/4Z)^3` verifies all these
relations and the C18 resolvent equation component by component. The periodic
witness checks consistency; the spectral-series argument supplies the
uncontained localization theorem.

## 6. Exact source-locked quarter-turn

Define the pre-kick conjugate field

\[
 P_n=F_n+F_{n+1}.
\]

The driven production-order kick is

\[
\begin{aligned}
 P_n-KF_n+U_n
 &=P_n-KF_n+(K-2I)F_n\\
 &=P_n-2F_n\\
 &=F_{n+1}-F_n\\
 &=F_{n+1}+F_{n+2}\\
 &=P_{n+1}.
\end{aligned}
\]

The following drift is

\[
 F_n+P_{n+1}=F_{n+1}.
\]

Thus

\[
 \boxed{(F_n,P_n)\mapsto(F_{n+1},P_{n+1})}
\]

exactly, with period four. This is the same unit kick--drift order-four value
identified in FTD-0918/0919, now realized by a driven localized profile rather
than an impossible free compact eigenmode.

The construction does not violate the free-mode obstruction because the
compact source changes on each tick and supplies the exact boundary return.

## 7. Constant handed circulation

Normalize the orthogonal field profiles:

\[
 a={F_0\over\|F_0\|},
 \qquad
 b={F_1\over\|F_0\|}.
\]

Let

\[
 A=ab^T-ba^T.
\]

The modal circulation is

\[
 \mathcal L=F_n^TAP_n.
\]

At `n=0`, the configuration coefficients are `(1,0)` and the momentum
coefficients `(1,1)`, after factoring out the common norm. Hence

\[
 \mathcal L=\|F_0\|^2>0.
\]

The exact four arms give the same value:

\[
 \boxed{\mathcal L_n=\|F_0\|^2\quad(n=0,1,2,3).}
\]

The reverse source sequence changes its sign. The substrate fields therefore
retain clockwise/counterclockwise information through the native phase-space
charge, not through a symmetric-square snapshot.

## 8. Zero field-side maintenance work

FTD-0576 gives the exact driven field-invariant change from source `U_n`:

\[
 W_n=\left\langle U_n,{P_n+P_{n+1}\over2}\right\rangle.
\]

The momentum midpoint is

\[
 {P_n+P_{n+1}\over2}=F_{n+1}=SF_n.
\]

Let

\[
 A_K=K-2I.
\]

It is self-adjoint and commutes with `S`. On the field doublet `S^T=-S`, so
`A_KS` is skew. Therefore

\[
\begin{aligned}
 W_n
 &=\langle A_KF_n,SF_n\rangle\\
 &=0.
\end{aligned}
\]

Hence

\[
 \boxed{W_n=0\text{ on every tick},
 \qquad\sum_{n=0}^3W_n=0.}
\]

This is the exact self-dual geometry suggested by the earlier intuition:
the maintained return and the circulating displacement occupy conjugate
orthogonal directions. It does **not** mean the whole clock is free. It is
only the field-side midpoint source-work channel on the exact orbit.

## 9. Exact autonomy boundary

The reference actuator froze

\[
 v_n=0,
 \qquad
 j_n=s_nv_n=0.
\]

But the source snapshot changes:

\[
 s_{n+1}-s_n\ne0.
\]

The exact `L=4` control gives

\[
 \|s_{n+1}-s_n\|_2^2=4
\]

on every arm. Therefore

\[
 s_{n+1}-s_n+\operatorname{div}_c j_n
 =s_{n+1}-s_n\ne0.
\]

The core sequence is imposed. It is not generated by a continuity-compatible
worldline, production movement, genesis/unactualization transaction, or
reversible source Hamiltonian.

This cleanly locates the remaining dynamics:

1. produce the four ternary core transitions locally;
2. include their nonzero `s v` curl contribution;
3. preserve or reconstruct the evanescent return profile;
4. react the field impulse onto the source variables;
5. pay switching/formation work and close it with positive storage; and
6. recover the exact orbit after perturbation without reading its outcome or
   a Born target.

## 10. Meaning for the clock programme

The substrate now has a complete **reference field clock body**:

- a compact ternary source core;
- a localized rather than compact field body;
- exact clockwise/counterclockwise circulation;
- exact four-tick recurrence; and
- zero ideal field-side maintenance work.

What it does not yet have is the autonomous core that turns the reference
actuator into physical hardware.

`G*` remains downstream. The present period is exactly four production ticks
because `kappa=2`; no lemniscatic factor was read or derived. A later
critical-quartic envelope or gate may compare its cadence against this local
carrier only after source autonomy and reaction close.

## 11. Epistemic boundary

This theorem does not derive source continuity, reciprocal core dynamics,
switching energy, a positive battery, formation, perturbation recovery,
mobility, physical scale, finite total source energy, `G*`, gamma, Born
frequencies, Bell correlations, measurement context, or preferred-tick
hiding. It changes no engine source, type, import, toggle, or default.

The tail estimate is a rigorous upper bound, not a measured decay law. The
periodic rational profile is a consistency witness, not substrate evidence
for spontaneous formation.

## 12. Verification and repair provenance

The FTD-0922 locked preregistration has SHA-256
`59B061102D498727E8099F6109464A0B8A9439FD014BC8176888524D40AD9BC7`.

The first immutable certificate has SHA-256
`2FEC105772F6396E49C3E2C47ADA2F2792438C7ADACF64D68AC4BE38C73CECEE`
and is preserved execution-invalid at `71/74`. It evaluated two uncontained
source-count checks on the periodic witness.

FTD-0923 locks the verifier-only repair in
`PREREG_TERNARY_DIPOLE_CORE_EVANESCENT_C4_CERTIFICATE_REPAIR_v2.md`,
SHA-256
`E4C8AD09EAEC580D6BD5C34588F293AAE8E8762D17331756252E5138CA371637`.

The exact one-block repair wrapper is
`scripts/proofs/proof_ternary_dipole_core_evanescent_c4_reference_orbit_v2.py`,
SHA-256
`2A3D8013F7D75B6DD5D49017A5F0CF021628D31DD5CC159E22C3DA159D43BD0D`.

It passes the inherited `74/74` gates and reports

```text
OUTCOME=A_EVANESCENT_REFERENCE_ORBIT_WITH_AUTONOMY_BOUNDARY
KAPPA_TWO_OUTSIDE_FREE_BAND=TRUE
TAIL_L2_BOUND=(9/2)*(8/9)^r
TERNARY_CORE_SUPPORT=2
ELECTRIC_SOURCE_VECTOR_SUPPORT=11
SOURCE_LOCKED_C4_ORBIT=EXACT
MODAL_CIRCULATION=NONZERO_CONSTANT
FIELD_SIDE_MIDPOINT_SOURCE_WORK=ZERO_EACH_TICK
SOURCE_CONTINUITY=FAILED_BY_FROZEN_REFERENCE
PRODUCTION_CHANGED=FALSE
GSTAR_USED=FALSE
BORN_BELL_CONTEXT_USED=FALSE
```
