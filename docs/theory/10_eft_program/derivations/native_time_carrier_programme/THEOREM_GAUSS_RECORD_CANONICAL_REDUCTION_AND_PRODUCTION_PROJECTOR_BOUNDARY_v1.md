# Theorem — Gauss-record canonical reduction and production-projector boundary v1

**Identifier:** `FTD-0877`  
**Date:** 2026-08-11  
**Status:** `[THEOREM — MATCHED GAUSS CANONICAL REDUCTION]` +
`[THEOREM — STATIC TERNARY RECORD SECTION]` +
`[THEOREM — NO UNIFORMLY LOCAL TRANSLATION-INVARIANT CHARGE CONJUGATE]` +
`[CLOSED NEGATIVE — LIVE CELL-CENTRED GAUSS MAP IS AN EXACT CANONICAL
PROJECTOR]` + `[OPEN — DYNAMIC RECORD PREPARATION, REVERSIBLE ENVIRONMENT,
PRODUCTION ACTUATION, G* SYNCHRONIZATION]`

## 1. Question and verdict

FTD-0876 identified the native phase-complete carrier

\[
  (J,P)=({\tt flux},{\tt wave\_vel})
\]

but did not show how a ternary record is naturally embedded in that carrier.
The obvious candidate is Gauss structure: a manifested polarity is recorded by
the divergence of flux. This document asks whether that candidate supplies an
exact constrained canonical sector and whether the live production Gauss pass
realizes it.

The answers are different:

1. **Exact matched complex:** on the already selected oriented-face incidence
   complex, the canonical phase space splits exactly into transverse and
   charge/conjugate sectors. A neutral ternary record has a unique
   minimum-energy longitudinal representative and a natural static section.
2. **Locality boundary:** the canonical conjugate of the charge is obtained by
   an inverse graph Laplacian. No translation-invariant right inverse of
   uniformly bounded range exists across arbitrarily large periodic probes.
   The charge record is local; its canonical conjugate is relational/global.
3. **Production boundary:** the live cell-centred central-difference
   divergence/gradient and the 18-point SOR Laplacian are not a matched
   complex. Their Fourier symbols differ, and the central derivative has
   extra Nyquist null modes on even probes. The live pass is therefore an
   approximate constraint-relaxation map, not the exact canonical reduction
   proved here.
4. **Loss ledger:** any map that overwrites a longitudinal discrepancy without
   retaining it is noninjective. A reversible actualization/preparation step
   must export that discrepancy to an environment or record it explicitly.

No Hilbert-space recovery, Born rule, Bell mechanism, `G*` gearbox, or
whole-framework completeness result is claimed.

## 2. Matched finite-region complex

Let a connected finite periodic cubic probe have `V=L^3` sites and `3V`
oriented positive-axis faces. Let

\[
  X=\mathbb R^{3V},\qquad
  Q=\{q\in\mathbb R^V:\mathbf 1^Tq=0\}.
\]

The backward incidence divergence `D:X->Q` is

\[
 (DJ)_x=\sum_{a=1}^3\bigl(J_a(x)-J_a(x-e_a)\bigr).
\]

With the Euclidean pairings, its adjoint is

\[
 (D^T\varphi)_a(x)=\varphi(x)-\varphi(x+e_a).
\]

Define the graph Laplacian and mean-zero projector

\[
 L=DD^T,\qquad
 \Pi_Q=I-\frac1V\mathbf1\mathbf1^T.
\]

Connectedness gives

\[
 \ker L=\operatorname{span}\{\mathbf1\},\qquad
 LL^+=L^+L=\Pi_Q,
\]

where `L+` is the Moore--Penrose inverse. This is the abstract version of the
already implemented `MatchedFaceFlux` complex. It is a selected finite probe,
not a claim that production cell-centred flux secretly has face incidence.

## 3. Exact canonical coordinates

Equip `X x X` with the canonical two-form

\[
 \Omega((\delta J_1,\delta P_1),(\delta J_2,\delta P_2))
 =\langle\delta J_1,\delta P_2\rangle
 -\langle\delta P_1,\delta J_2\rangle.
\]

Define the charge coordinate and its conjugate

\[
 q=DJ,\qquad p=L^+DP.
\]

Their canonical bracket is

\[
 \{q,p\}=DD^TL^+=LL^+=\Pi_Q.
\]

Hence the bracket is the identity on the compatible charge space `Q`. The
constant mode is absent because periodic divergence telescopes to zero.

Now define

\[
 \begin{aligned}
 J_L&=D^TL^+q, & J_T&=J-J_L,\\
 P_L&=D^Tp,     & P_T&=P-P_L.
 \end{aligned}
\]

Then

\[
 DJ_T=0,\qquad DP_T=0,
\]

and the reconstruction is exact:

\[
 J=J_T+D^TL^+q,\qquad P=P_T+D^Tp.
\]

The orthogonality of `ker D` and `im D^T` eliminates all cross terms. The
canonical form therefore splits as

\[
 \boxed{\Omega=\Omega_T+dq^T\wedge dp},
\]

where `Omega_T` is the canonical form restricted to
`ker D x ker D`. This is an exact symplectic change of variables after the
constant charge mode is removed.

## 4. Ternary records and the static section

For a ternary configuration `s_x in {-1,0,+1}` and declared coupling `g`, the
periodic compatible charge is

\[
 q_s=g(s-\bar s\mathbf1),\qquad
 \bar s=\frac1V\sum_xs_x.
\]

If the probe is neutral, `bar s=0` and `q_s=gs`. If it is not neutral, the
mean-subtracted background is part of the finite-probe ledger; the probe does
not represent an isolated net charge without that background.

The minimum-energy representative subject to `DJ=q_s` is

\[
 J_s=D^TL^+q_s.
\]

Indeed, every other representative is `J_s+J_T` with `DJ_T=0`, and

\[
 \|J_s+J_T\|^2=\|J_s\|^2+\|J_T\|^2.
\]

The natural static record section is therefore

\[
 \mathcal S_s:
 (q,p,J_T,P_T)=(q_s,0,0,0),
 \quad (J,P)=(J_s,0).
\]

More generally, fixing `q=q_s` gives a presymplectic constraint surface: the
`p` direction is null when `dq=0`. Quotienting that gauge direction leaves the
transverse symplectic phase space. Choosing `p=0` is a static section/gauge
choice, not a newly derived material degree of freedom.

This section is axis-free and polarity-covariant:

\[
 s\mapsto-s\quad\Longrightarrow\quad q_s\mapsto-q_s,
 \quad J_s\mapsto-J_s.
\]

It supplies an exact record representation. It does **not** supply a native
dynamical process that forms the section from arbitrary production data.

## 5. Constraint-preserving recursion

If an update has the staggered canonical form

\[
 P^+=P+hF,\qquad J^+=J+hP^+,
\]

then a fixed charge record is preserved whenever

\[
 DP=0,\qquad DF=0.
\]

The matched complex already has the algebraic mechanism: if `C` is its curl,

\[
 DC=0.
\]

Therefore curl-generated forces and drifts remain in the transverse sector
and leave `q=DJ` unchanged. Conservative source motion can instead update
`q` through an explicit continuity current. This is the simplest stable
recursive form:

```text
longitudinal sector:  q = ternary record, p = 0 for a static record
transverse sector:    reversible curl/canonical dynamics
record change:         explicit continuity current or explicit reaction ledger
```

The existing `MatchedGaussDynamics` is evidence that this matched recursion is
implementable in the isolated EFT namespace. It remains a selected sidecar and
is not promoted to the production tick by this theorem.

## 6. Why the conjugate is not uniformly local

Suppose a translation-invariant right inverse `B` of `D` had fixed range `R`
on every sufficiently large periodic probe. Its Fourier symbol would satisfy

\[
 D(k)B(k)=1\qquad(k\ne0).
\]

Restrict to the one-dimensional line `k=(k_x,0,0)` and write `z=e^{ik_x}`.
Then

\[
 (1-z^{-1})B_x(z)=1
\]

at every nontrivial `L`-th root of unity. Since a range-`R` stencil makes
`B_x(z)` a Laurent polynomial with exponents from `-R` to `R`,

\[
 f(z)=z^{R+1}\big((1-z^{-1})B_x(z)-1\big)
\]

is a polynomial of degree at most `2R+1`. For `L>=2R+3`, it has `L-1` roots,
more than its degree, so it would vanish identically. But at `z=1`,

\[
 f(1)=-1,
\]

a contradiction. Thus:

\[
 \boxed{\text{No uniformly finite-range translation-invariant right inverse
 of }D\text{ exists across arbitrarily large periodic probes.}}
\]

Equivalently, `p=L+DP` and the minimum-energy representative `J_s` require a
relational solve whose support/range grows with the probe. This is not a
failure of locality of substrate propagation. It says that a global
constraint coordinate cannot have an onsite canonical conjugate without
additional local link/environment structure.

## 7. Exact audit of the live cell-centred projector

The production divergence and gradient are central differences. Their
composition has Fourier symbol

\[
 \widehat{D_cG_c}(k)
 =-\big(\sin^2k_x+\sin^2k_y+\sin^2k_z\big).
\]

The SOR solver instead uses the 18-point symbol

\[
 \widehat{\Delta_{18}}(k)
 =\frac23(c_x+c_y+c_z)
  +\frac23(c_xc_y+c_xc_z+c_yc_z)-4,
 \qquad c_a=\cos k_a.
\]

They agree to leading long-wavelength order but are not the same operator. Two
exact witnesses are

\[
 \begin{array}{c|cc}
 k & \widehat{D_cG_c}(k) & \widehat{\Delta_{18}}(k)\\ \hline
 (\pi/2,0,0)&-1&-2\\
 (\pi,0,0)&0&-4.
 \end{array}
\]

The second row is decisive on even periodic probes: the central gradient and
divergence cannot see the Nyquist checkerboard mode, while the SOR operator
does. Consequently solving

\[
 \Delta_{18}\phi=D_cJ-q_s
\]

and applying `J <- J-G_c phi` does not in general yield `D_cJ=q_s`, even with
an exact infinite-iteration solve. The live code adds two further departures:

- it executes only a finite number of SOR iterations; and
- unless `exact_dual_gauss` is set, it skips correction at manifested sites.

Therefore the statement “the live Gauss pass is the exact orthogonal/canonical
projection onto `D_cJ=q_s`” is closed negative. The honest status is
**approximate production constraint relaxation**. This finding does not alter
the exact matched-incidence results of Sections 2--5.

## 8. Loss, unactualization, and reversibility

For the matched complex, the affine minimum-change map toward a compatible
record is

\[
 \mathcal P_s(J)=J-D^TL^+(DJ-q_s).
\]

Its linear part is the transverse projector

\[
 T=I-D^TL^+D,
 \qquad T^2=T.
\]

`T` annihilates `im D^T`, so `P_s` is noninjective. Distinct prehistories that
differ only by a longitudinal discrepancy produce the same prepared record.
That discarded quantity is

\[
 \ell=J-\mathcal P_s(J)=D^TL^+(DJ-q_s).
\]

If the output is enlarged to `(P_s(J),ell)`, the input is recovered exactly by
addition. Thus the minimum reversible completion is not mysterious:

```text
prepared record + exported longitudinal discrepancy = recoverable input.
```

If `ell` is dissipated into unresolved substrate/environment variables, the
effective record map is lossy. This is a precise ledgerable model of the
user's “unactualization”: irrelevant detail may disappear from the retained
record, but it cannot disappear from a globally reversible account without an
environmental carrier.

## 9. Epistemic accounting

### Closed positive

- `[THEOREM]` exact matched-incidence transverse/longitudinal decomposition;
- `[THEOREM]` exact charge bracket `{q,p}=Pi_Q` and canonical split on `Q`;
- `[THEOREM]` neutral ternary configurations admit a unique minimum-energy
  longitudinal record representative;
- `[THEOREM]` the static section `(q,p,J_T,P_T)=(q_s,0,0,0)` is axis-free and
  polarity-covariant;
- `[THEOREM]` matched curl dynamics preserves fixed charge because `DC=0`;
- `[THEOREM]` no uniformly finite-range translation-invariant right inverse
  of the incidence divergence exists across arbitrarily large probes; and
- `[THEOREM]` retaining the discarded longitudinal discrepancy makes the
  affine record-preparation map exactly reversible.

### Closed negative

- the production central-difference/18-point-SOR combination is an exact
  matched Gauss projector;
- changing flux at a source cell necessarily repairs the source-cell
  divergence in the collocated central stencil; and
- an unledgered nonidentity Gauss overwrite is a symplectic automorphism of
  the full unconstrained phase space.

### Still open

- native dynamic formation of the minimum-energy ternary record section;
- a local environment-complete reversible preparation mechanism;
- production migration, if ever justified, to a matched face/link complex;
- coupling the record leaf to the FTD-0875 parity actuator without target
  coding or nonlocal control;
- finite-support/uncontained-substrate formulation beyond periodic probes;
- physical scale, dissipation, routing, collisions, and reciprocal boundary;
- `G*` synchronization and non-rescalability;
- Born recovery, Bell laboratory recovery, and operational hiding.

No new selected type is booked. The theorem uses the already selected
`MatchedFaceFlux` representation as a consistency witness and sharpens, but
does not retire, the dynamic-preparation portion of `OPEN-CA-TRANSDUCER`.

## 10. Verification contract

The locked certificate is
`scripts/proofs/proof_gauss_record_canonical_reduction.py`. The isolated C++
witness is under `ftd::eft` and must not modify production `Voxel`, Gauss
toggles, or tick phases.

Required terminal markers are:

```text
GAUSS_RECORD_CANONICAL_REDUCTION_THEOREM
MATCHED_CHARGE_BRACKET=IDENTITY_ON_MEAN_ZERO_SPACE
STATIC_TERNARY_RECORD_SECTION=EXACT
UNIFORMLY_LOCAL_CHARGE_CONJUGATE=NO
PRODUCTION_GAUSS_EXACT_PROJECTOR=NO
REVERSIBLE_PREPARATION_REQUIRES=DISCREPANCY_LEDGER
GSTAR_ROLE=SEPARATE_CALENDAR
```
