# FTD-0924 — Ternary-dipole `C4` central-continuity current and hub boundary v1

**Identifier:** `FTD-0924`  
**Date locked:** 2026-08-11  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE CERTIFICATE]`  
**Scope:** exact compact continuity current for the FTD-0923 dipole orbit,
its central curl and conditional Hodge-energy closure, and the obstruction to
compiling that current through the unchanged production tie `j=s v`; no
numerical search, fit, engine mutation, or new ontology adoption

## 1. Question

FTD-0923 constructed a localized exact `C4` field orbit but changed a
two-site ternary dipole while setting its current to zero. A one-site cardinal
hop is obstructed by the central checkerboard zero (FTD-0576), but the present
transition contains two oppositely signed, quarter-rotated arms. Do their
parity defects cancel? If so, where does the exact central current live, what
curl source does it produce, and can it be the unchanged production current
`j=s v`?

The test must separate three claims:

1. existence of a compact algebraic integrated current;
2. conditional exact Hodge source/energy closure using that current; and
3. physical realization by the existing manifested-site velocity variable.

Passing the first two does not imply the third.

## 2. Frozen sources

| Source | SHA-256 |
|---|---|
| `THEOREM_TERNARY_DIPOLE_CORE_EVANESCENT_C4_REFERENCE_ORBIT_AND_AUTONOMY_BOUNDARY_v1.md` | `DB9894C1554422B0BA0C97A991FFF7F714B83EF673DDF5FEDA026B45C55B88AF` |
| `THEOREM_NATIVE_HODGE_ENERGY_CONTINUITY.md` | `7849BFF214225723BFA52EA9034C34B22B94D204A78BE1D6DC6F97D065222868` |
| `AUDIT_NATIVE_HODGE_ENERGY_CONTINUITY.md` | `033985919FAC722F47B09311D51B47E5DDB4E5A3A47D0A3F36B736CFAF481D08` |
| `engine/src/render_bridge_phases/phase_read.cpp` | `D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8` |
| `engine/include/ftd/causal_kinematics.h` | `705501451985333D64128A0896216A137A2D836673AEB02E9ACE6DE4F2E53AA2` |
| `engine/include/ftd/field_operators.h` | `25866EFC8474A2AEF7443C5DA67CBF79BEB352DE3E342A97D1EF0C3C16439E48` |

The certificate fails closed on source drift.

## 3. Frozen dipole orbit and central operators

Let

\[
 s_0=\delta_{e_x}-\delta_{-e_x},
 \qquad s_n=S^n s_0,
\]

where `S` is the active right-handed quarter-turn about `z`. Thus

\[
 s_1=\delta_{e_y}-\delta_{-e_y},
 \qquad s_{n+2}=-s_n.
\]

Use the production central difference

\[
 D_i f(x)={f(x+e_i)-f(x-e_i)\over2},
 \qquad DQ=\sum_iD_iQ_i,
\]

and the matching central curl `C`. For transition `n`, define

\[
 \delta s_n=s_{n+1}-s_n.
\]

The exact continuity equation is

\[
 \boxed{\delta s_n+DQ_n=0.}
\]

## 4. Frozen compact-current candidate

The registered candidate is the point-supported vector current

\[
 \boxed{
 Q_0=2(e_y-e_x)\delta_0,
 \qquad Q_n=S^nQ_0.}
\]

Explicitly,

\[
 Q_1=-2(e_x+e_y)\delta_0,
 \qquad Q_{n+2}=-Q_n.
\]

The certificate must verify all four continuity equations exactly on the
uncontained finite support and on a separate exact `L=4` periodic witness.
It must also verify the cancellation of every one of the eight parity
characters of `delta s_n`. This is the predicted reason the two-arm dipole
transition evades the one-site FTD-0576 checkerboard obstruction.

The current is not a fitted flow and is not inferred from a continuum path.
Its factor `2` is fixed by the half-step normalization of the central
difference.

## 5. Frozen current-curl source

For `Q_n=(a_n,b_n,0) delta_0`, the central curl is

\[
 CQ_n=
 \left(-b_nD_z\delta_0,
        a_nD_z\delta_0,
        b_nD_x\delta_0-a_nD_y\delta_0\right).
\]

The certificate must verify

\[
 \operatorname{supp}_v(CQ_n)=6,
 \qquad
 \|Q_n\|_2^2=8,
 \qquad
 \|CQ_n\|_2^2=8,
\]

plus exact `C4` covariance and `C^T=C`, `DC=0` identities on the registered
witness.

## 6. Frozen midpoint Hodge reference source

Define the arithmetic endpoint midpoint

\[
 \bar s_n={s_n+s_{n+1}\over2}
\]

and the compact Hodge source seed

\[
 q_n=\nabla_c\bar s_n-CQ_n,
 \qquad U_n=-q_n.
\]

`bar s_n` is a bookkeeping midpoint, not an additional actual ternary state.
On the uncontained finite support, the certificate must derive

\[
 \|\nabla_c\bar s_n\|_2^2={7\over4},
 \qquad
 \langle\nabla_c\bar s_n,CQ_n\rangle=0,
 \qquad
 \boxed{\|q_n\|_2^2={39\over4}}.
\]

At `kappa=2`, set

\[
 F_n=(2I-K)^{-1}q_n,
 \qquad P_n=F_n+F_{n+1}.
\]

The exact `L=4` rational witness must verify

\[
 U_n=(K-2I)F_n,
 \quad F_{n+1}=SF_n,
 \quad F_{n+2}=-F_n,
\]

and every kick--drift arm. The FTD-0923 evanescent tail theorem then applies
to this new compact `q_n` without changing its bound.

## 7. Frozen conditional reaction/work ledger

Use the FTD-0576 work coordinate

\[
 R_n=F_n-{1\over2}P_n.
\]

The certificate must check

\[
 R_{n+1}=SR_n,
 \qquad
 \delta R_n=F_{n+1},
 \qquad
 \bar R_n={1\over2}F_n.
\]

The field work is

\[
 \Delta H_f=\langle U_n,\delta R_n\rangle.
\]

The interaction and required matter-reaction changes are

\[
 \Delta U_{\rm int}
 =-\langle\bar s_n,D\delta R_n\rangle
  -\langle Q_n,GD\bar R_n\rangle,
\]

\[
 \Delta H_m
 =\langle Q_n,GD\bar R_n-C\delta R_n\rangle.
\]

The registered prediction is the lossless ideal result

\[
 \boxed{
 \Delta H_f=\Delta U_{\rm int}=\Delta H_m=0}
\]

on each arm. The first zero follows from the orthogonal `C4` return; the
second follows because `(s_n,R_n)` advances by a common rotation, leaving
`-<s_n,DR_n>` invariant; the third then follows from the exact FTD-0576 total
energy identity. Direct exact evaluation must independently confirm all
twelve zeros.

This result would mean that the ideal path requires no *net* maintenance or
reaction work in the registered conditional ledger. It would not provide a
positive carrier Hamiltonian, a switching rule, formation energy, or a
restoring mechanism.

## 8. Frozen production-current discriminators

### 8.1 Endpoint-support no-go

For each transition, allow arbitrary real vector values on either:

1. the two occupied sites of `s_n`; or
2. the union of the occupied sites of `s_n` and `s_{n+1}`.

The certificate must solve the exact coefficient systems and show that no
such current satisfies `delta s_n+Dj=0`. The parity proof must be recorded:
currents on the endpoint parity classes differentiate into classes disjoint
from the nonzero classes of `delta s_n`.

Therefore no unchanged two-site production current

\[
 j_n=s_nv_n
\]

can realize `Q_n`, irrespective of the endpoint velocities.

### 8.2 Void-center fact

The successful current is supported at the rotation center, but

\[
 s_n(0)=0
\]

for all four snapshots. Hence `s_n v_n` vanishes there for every finite
stored velocity. The current is an algebraic bridge variable, not yet a live
production current.

### 8.3 Minimal manifested-hub control

As a diagnostic only, add a fixed ternary hub

\[
 h=\eta\delta_0,
 \qquad \eta\in\{-1,+1\}.
\]

Then `(s_n+h)v_n=Q_n` can be realized by setting the hub velocity to
`v_n=Q_n/eta`. Under the unit-tick identification this requires

\[
 |v_n|=2\sqrt2>{1\over\sqrt3}=C_{\rm SPEED}.
\]

The certificate must verify this exact inequality and the unchanged
production bandwidth projection marker. It must also register that `h` adds
a static electric source `-grad_c h`; therefore the hub control is not the
same source-locked `C4` orbit. It is a diagnostic witness for the missing
carrier, not an admissible production solution.

## 9. Outcome rules

- **Outcome A — compact bridge current with production-carrier boundary:**
  all continuity, curl, Hodge orbit, and conditional zero-work gates pass;
  endpoint-supported `s v` fails; the successful current lies at the void
  center; and the one-site hub control fails the unit-tick bandwidth/source
  criteria. Book the exact bridge-current theorem and the scoped production
  obstruction.
- **Outcome B — existing matter current closes:** an endpoint-supported live
  current solves continuity and respects the production tie and bandwidth.
  Book the explicit current and retire the corresponding obstruction.
- **Outcome C — no compact continuity current:** the parity cancellation or
  point-current identity fails. Retain FTD-0923's autonomy boundary.
- **Outcome D — invalid execution:** any source lock, exact identity,
  production marker, or scope firewall fails. Book no theorem.

## 10. Required certificate gates

The exact certificate must cover:

1. all frozen hashes;
2. ternarity, neutrality, support, rotation, and antipodes of `s_n`;
3. all eight parity characters on all four `delta s_n`;
4. all four exact central-continuity equations;
5. point-current support, norm, curl support/norm, covariance, and Hodge
   identities;
6. exact endpoint and endpoint-union coefficient-system inconsistency;
7. midpoint-source norms and gradient--curl orthogonality;
8. exact rational `L=4` resolvent and four kick--drift arms;
9. twelve exact field/interaction/matter work zeros;
10. void-center and manifested-hub discriminators;
11. production `-grad(s)+curl(sv)`, bandwidth, and kick--drift markers;
12. unchanged engine/type/import status; and
13. no `G*`, gamma, Born/Bell, context, measurement, fit, sweep, near-miss,
    or formula-substitution read.

## 11. Frozen scope ceiling

Success does not derive an autonomous source update, a positive source
Hamiltonian, an admissible local hub/scaffold, multi-tick transport,
formation, reset, perturbation recovery, scale, `G*`, gamma, Born
frequencies, Bell correlations, measurement context, or preferred-tick
hiding. An independent current remains a candidate new type until adopted;
the present test is allowed to expose it but not to purchase it.
