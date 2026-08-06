# FTD-0576 — Native Hodge Energy and Central-Continuity Obstruction

**Status:** `[THEOREM — EXACT DRIVEN-TICK WORK IDENTITY]` +
`[THEOREM — CONDITIONAL HODGE TOTAL-ENERGY IDENTITY]` +
`[THEOREM — CENTRAL CARDINAL-HOP LOCALITY OBSTRUCTION]` +
`[SCOPED NO-GO — FINITE-RANGE FACE-TO-NATIVE CURRENT PROJECTION]` +
`[OPEN — STAGGERED/NONLINEAR/ENLARGED MOBILE CARRIER]`  
**Date:** 2026-07-26  
**Verdict:**
`NATIVE_HODGE_ENERGY_IDENTITY_CENTRAL_LOCAL_MOBILE_CURRENT_OBSTRUCTED`

## 1. Scope

FTD-0574 derived the exact local free-field action and prescribed-source
functional. FTD-0575 derived its reciprocal Hodge force and proved that its
static pole cancels. This theorem identifies the exact finite-step field-work
coordinate, derives the common-energy identity that a moving carrier would
have to satisfy, and tests whether the existing cardinal ternary hop can
supply the required native current locally.

The production tick is frozen. The conditional energy construction below is
an algebraic target, not a new production phase.

## 2. Exact work of a driven production tick

Let `K` be the symmetric production field operator and add an arbitrary
prescribed kick `S_n`:

\[
 W_1=W_0-KJ_0+S_n,
 \qquad
 J_1=J_0+W_1.
\]

The FTD-0574 tick invariant is

\[
 H(J,W)=\frac12\langle W,W\rangle
       +\frac12\langle J,KJ\rangle
       -\frac12\langle W,KJ\rangle.
\]

Direct expansion gives

\[
 \boxed{H_1-H_0=
 \left\langle S_n,\frac{W_0+W_1}{2}\right\rangle.}
\]

This is exact for every symmetric `K`; it is not a small-step approximation.

## 3. The unique work coordinate

Define

\[
 \boxed{R_n=J_n-\frac12W_n.}
\]

Then

\[
 R_1-R_0=\frac12(W_0+W_1),
\]

so the work theorem becomes

\[
 \boxed{H_1-H_0=\langle S_n,R_1-R_0\rangle.}
\]

Within the linear family `R=J-cW`, demanding
`R_1-R_0=(W_0+W_1)/2` for arbitrary `J_0,W_0,S_n` gives uniquely

\[
 c=\frac12.
\]

For a constant prescribed source, the affine invariant is therefore

\[
 \boxed{H_S=H-\langle S,R\rangle.}
\]

The exact energy-coupling field is the staggered coordinate `R`, not simply a
same-tick visualization of `J`.

## 4. Conditional exact Hodge energy

Let endpoint densities and one integrated site current obey continuity under
the same central divergence `D` as the source action:

\[
 \boxed{\rho_1-\rho_0+DQ=0.}
\]

Write

\[
 \bar\rho=\frac12(\rho_0+\rho_1),\qquad
 \bar R=\frac12(R_0+R_1),\qquad
 \delta R=R_1-R_0,
\]

and use the midpoint Hodge source

\[
 S=-G_CG\bar\rho+G_CCQ.
\]

Because `D^T=-G` and `C^T=C`, the field work is

\[
 \Delta H_f
 =G_C\langle\bar\rho,D\delta R\rangle
  +G_C\langle Q,C\delta R\rangle.
\]

Define the scalar interaction energy

\[
 U_{\rm int}(\rho,R)=-G_C\langle\rho,DR\rangle.
\]

The polarization identity and exact continuity give

\[
 \Delta U_{\rm int}
 =-G_C\langle\bar\rho,D\delta R\rangle
  -G_C\langle Q,GD\bar R\rangle.
\]

Consequently the matter kinetic sector must receive exactly

\[
 \boxed{\Delta H_m
 =G_C\langle Q,GD\bar R-C\delta R\rangle.}
\]

The three terms cancel:

\[
 \boxed{\Delta H_f+\Delta U_{\rm int}+\Delta H_m=0.}
\]

This is the exact discrete Hodge analogue of field work, scalar-potential
energy, and electric matter work. A magnetic impulse proportional to
`v cross C^2 R_bar` is orthogonal to the path velocity and does not change the
scalar ledger.

The identity is conditional: a local carrier must still provide `Q` satisfying
the native central continuity equation, and a matched gather must turn its
lattice pairing into the production-dispersion energy change.

## 5. Cardinal one-site hop obstruction

Consider an axial hop of the cardinal endpoint shape. In one translation
variable `z`, its endpoint-density difference contains the factor `z-1`. The
central divergence symbol is

\[
 d_c(z)=\frac{z-z^{-1}}2
       =\frac{(z-1)(z+1)}{2z}.
\]

Continuity forces the current symbol

\[
 \boxed{Q(z)=-\frac{2z}{z+1}.}
\]

It has a pole at `z=-1` and is not a finite Laurent polynomial. Therefore no
fixed finite-range translation-covariant site current transports a cardinal
polarity by one site under the native central divergence.

The finite-volume distinction is exact:

- On even periodic volumes, `z=-1` is an allowed checkerboard mode. The
  density hop has nonzero projection on that left null mode while `d_c(-1)=0`;
  no current solution exists.
- On odd periodic volumes, the checkerboard point is absent and a solution
  exists, but it is global. The registered zero-mean solution is

  \[
  Q_0=Q_{2m+1}=q\frac{L-1}{L},\qquad
  Q_{2m}=-q\frac{L+1}{L}\quad(m\ge1),
  \]

  which is nonzero at every site and reaches radius `(L-1)/2`. Adding the
  constant null mode can zero one parity class but cannot produce support
  bounded independently of `L`.

Thus odd boxes hide the algebraic nonexistence behind a box-spanning
alternating current; they do not restore locality.

## 6. No local face-to-native current bridge

The oriented-face divergence has the axial symbol

\[
 d_f(z)=1-z^{-1}.
\]

A finite-range translation-invariant projection `A` from the exact FTD-0478
face current to a native site current would have to commute with divergence:

\[
 d_c(z)A(z)=d_f(z).
\]

This fixes

\[
 \boxed{A(z)=\frac{2}{z+1}.}
\]

The same checkerboard pole proves that no finite-range commuting projection
exists. The local face-current theorem and the native central-source action
are individually valid but cannot be combined by a local compatibility
projection.

## 7. Consequence

The exact-energy problem is now separated into two statements:

1. **Positive:** the native field tick has an exact source-work theorem, and
   continuity supplies an exact conditional Hodge total-energy identity.
2. **Negative:** the cardinal ternary hop cannot provide that continuity
   locally under the frozen central operators, and the exact face current
   cannot be projected into them by a finite-range map.

Therefore exact energy, frozen native central operators, cardinal site
mobility, and finite-range locality cannot all hold in the minimal variable
set. At least one of the following is required:

- face/link fields with their matched divergence as a separate selected
  dynamics;
- a staggered current or connection primitive;
- a nonlocal box-dependent current;
- a derived non-cardinal or nonlinear carrier whose endpoint symbol cancels
  the checkerboard factor while retaining the ternary manifestation contract.

**Successor correction (FTD-0577):** the last route now has one exact local
witness. The unique normalized symmetric radius-one separable filter
`B=(z^-1+2+z)/4` cancels the pole and maps the FTD-0478 face current to a
finite-range native central current. This does not weaken the cardinal-hop
no-go proved here; it abandons cardinal coupling while retaining ternary
primitive manifestation. Force selection, self-force, and mobile closure
remain open.

No production toggle or mobile scenario is licensed. The result does not
close all nonlinear or enlarged matter models.

## 8. Verification

The native observer runs 36 modal work arms, four full-field work arms, four
conditional energy arms, 18 volume/axis hop arms with 36 polarity checks, and
24 proper-cubic rotations. Worst native energy residual is `1.36e-15`; odd
current and even checkerboard identities are exact at binary64 resolution.

The independent SymPy/NumPy proof derives the unique half-step coefficient,
the two rational symbols, the poles at `z=-1`, and the same finite-volume
currents. The run of record is
`engine/results/ftd_0576/windows_msvc_cpu.json`.

The locked preregistration SHA-256 is
`98B3F8D13E6FBAAD26931C6DD7EC37C9377BD054899012B109C63A0512C26E78`.
Production state and defaults are unchanged.
