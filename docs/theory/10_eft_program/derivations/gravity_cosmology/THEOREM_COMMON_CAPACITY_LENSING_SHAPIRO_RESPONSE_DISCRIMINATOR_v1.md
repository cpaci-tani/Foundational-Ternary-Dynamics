# Common-capacity lensing and Shapiro response discriminator v1

**Date:** 2026-08-24

**Status:** **[THEOREM — EXACT WEAK POINT-DEPTH RAY DEFLECTION AND SHAPIRO
RESPONSE]** + **[THEOREM — SOURCE-NORMALIZATION-FREE LENSING/DYNAMICS
DISCRIMINATOR]** + **[THEOREM — SHARED CAPACITY FIELD DOES NOT FIX OPERATOR
RESPONSE RATIOS]** + **[OPEN — NATIVE MAXWELL-CAPACITY
OPERATOR, STATIC POLE, NONZERO LENSING, NONLINEAR GRAVITY]**

**Production status:** unchanged; no refractive-index or metric operator added

**Ledger status:** no row minted

**Exact certificate:**
[proof_common_capacity_lensing_shapiro_discriminator.py](../../../../../scripts/proofs/proof_common_capacity_lensing_shapiro_discriminator.py)
performs 22 exact symbolic checks of the weak clock/optical expansions,
complete-ray integral, finite-endpoint Shapiro integral, response classes,
source-scale cancellation, and coefficient independence. It contains no
physical target fit or parameter search.

---

## 1. Why “the same capacity” is not yet lensing

The existing engine gravity chain already provides a decisive counterexample
to an overly weak unification claim:

- [FTD-1019](../../../03_derivations/gravity_and_cosmology/ANALYSIS_ONE_WELL_REDSHIFT_FALLING_v1.md)
  uses one frozen latency well for material clock rate and selected slow-body
  falling; while
- [FTD-1020](../../../03_derivations/gravity_and_cosmology/ANALYSIS_FROZEN_WELL_CHARACTERISTIC_DEFLECTION_v1.md)
  finds class 0 because the vacuum-wave principal stencil does not read that
  well.

Thus a common named field can affect matter and clocks while leaving light
exactly unaffected. The native-action gate needs four distinct response
coefficients, not one verbal assertion of common capacity.

Let $U(x)$ be the positive weak static capacity depth. Define:

\[
 a_m:\text{ slow-body acceleration response},
 \qquad
 a_t:\text{ material-clock response},             \tag{1}
\]

\[
 a_0:\text{ temporal response of the wave principal symbol},
 \qquad
 a_s:\text{ spatial/Hodge response of that symbol}. \tag{2}
\]

These are blocked response derivatives to be measured from one action. They
are not four new microscopic parameters authorized for insertion.

For the registered latency variable $\mathcal L$,

\[
 U={\mathcal L^2\over2},
 \qquad
 {1\over\sqrt{1-\mathcal L^2}}=1+U+O(U^2),         \tag{3}
\]

and the selected slow-body operator is proportional to

\[
 \mathcal L\nabla\mathcal L=\nabla U.              \tag{4}
\]

Equations (3)--(4) explain the clock-medium/1911 normalization used in the
locked FTD-1020 classifier. They do not install it in the wave update.

---

## 2. Minimum isotropic weak optical response

Parameterize the static isotropic wave principal symbol by the eikonal line
element

\[
 ds_{\rm eik}^2
 =-(1-2a_0U)c_*^2dt^2
 +(1+2a_sU)d\mathbf x^2.                           \tag{5}
\]

Equation (5) is an observable response chart, not a declaration that a
continuum metric is fundamental. It defines what the blocked finite action
would have to produce in its long-wavelength principal symbol.

The coordinate characteristic speed and refractive index are

\[
 {c_{\rm ray}\over c_*}
 =\sqrt{{1-2a_0U\over1+2a_sU}},                    \tag{6}
\]

\[
 \boxed{
 n(U)=1+(a_0+a_s)U+O(U^2).}                       \tag{7}
\]

Only the sum $a_0+a_s$ appears in first-order null propagation. Material
clocks instead obey

\[
 {d\tau\over dt}=1-a_tU+O(U^2),                   \tag{8}
\]

while slow-body motion measures $a_m\nabla U$. Therefore $a_t=a_m$ does not
imply $a_0=a_t$, and neither implies a nonzero $a_s$.

---

## 3. Exact point-depth deflection

Suppose a future native static-pole theorem produces, in its weak exterior,

\[
 U(r)={\mu\over r}.                                \tag{9}
\]

This section is conditional on equation (9); it does not derive the pole or
its normalization.

For an unperturbed ray along $z$ at impact parameter $b>0$,

\[
 r=\sqrt{b^2+z^2},
 \qquad
 \partial_b n
 =-(a_0+a_s)\mu{b\over(b^2+z^2)^{3/2}}.           \tag{10}
\]

The exact integral is

\[
 \int_{-\infty}^{\infty}
 {b\,dz\over(b^2+z^2)^{3/2}}={2\over b}.          \tag{11}
\]

With the sign chosen so negative deflection points toward the source,

\[
 \boxed{
 \theta=-{2(a_0+a_s)\mu\over b}.}                 \tag{12}
\]

Slow-body dynamics independently measures

\[
 \mu_m=a_m\mu                                      \tag{13}
\]

from the exterior acceleration. Hence the blind optical/dynamical ratio is

\[
 \boxed{
 \mathscr D
 =-{b\theta\over2\mu_m}
 ={a_0+a_s\over a_m}.}                            \tag{14}
\]

The unknown source strength, absolute gravity normalization, and any common
rescaling of $U$ cancel from equation (14).

---

## 4. Exact finite-endpoint Shapiro response

Let the emitter and receiver lie distances $z_L,z_R>0$ from closest approach.
The first-order excess travel time, in units with $c_*=1$, is

\[
 \Delta t
 =(a_0+a_s)\mu
 \left[
 \operatorname{asinh}{z_L\over b}
 +\operatorname{asinh}{z_R\over b}
 \right].                                         \tag{15}
\]

Writing the bracket as $\mathcal G(b,z_L,z_R)$ gives a second blind ratio,

\[
 \boxed{
 \mathscr S
 ={\Delta t\over\mu_m\mathcal G}
 ={a_0+a_s\over a_m}
 =\mathscr D.}                                     \tag{16}
\]

A proposed native optical operator must give the same coefficient in
deflection and delay. Disagreement is a direct falsifier of the weak common
principal symbol, independent of comparison with GR.

---

## 5. Blind response classes

The separately measurable clock/fall ratio is

\[
 \mathscr R_{tm}={a_t\over a_m}.                   \tag{17}
\]

The relevant classes are therefore:

| Class | Response tuple | $\mathscr R_{tm}$ | $\mathscr D=\mathscr S$ |
|---|---:|---:|---:|
| wave unread / FTD-1020 class 0 | $a_t=a_m$, $a_0=a_s=0$ | 1 | 0 |
| clock-medium / 1911-half | $a_t=a_0=a_m$, $a_s=0$ | 1 | 1 |
| equal temporal+spatial response | $a_t=a_0=a_s=a_m$ | 1 | 2 |

The labels “1911-half” and “equal temporal+spatial” are classifier references,
not adopted physical results. A native calculation must determine equations
(14), (16), and (17) before any comparison with those classes.

FTD-1019 plus FTD-1020 occupies the first row at the tested engine fixture:
clock/fall coherence does not rescue an unread wave operator.

---

## 6. Exact coefficient obstruction

At isotropic weak order, the four operators

\[
 a_mU,\qquad a_tU,\qquad a_0U,\qquad a_sU         \tag{18}
\]

are linearly independent symmetry-allowed responses. Their coefficient
Jacobian has rank four. Naming their source field $U$ does not impose any
equality among them.

This is the lensing analogue of the
[native-alpha action-scale obstruction](../charge_gauss_native_em/THEOREM_COTANGENT_NATIVE_ALPHA_ACTION_SCALE_OBSTRUCTION_v1.md),
with one favorable difference: the overall static source normalization
cancels from equation (14). The unified action can therefore be tested for
lensing before it has derived the absolute value of $G_N$ or the
electromagnetic action scale.

---

## 7. Relation to the tensor-curl target

The
[parity-correct spin-2 curl target](THEOREM_COTANGENT_STF_PARITY_PRICE_AND_SPIN2_CURL_TARGET_v1.md)
supplies a possible radiative even/odd tensor pair. It does not determine
$a_0$ or $a_s$. A free tensor wave can coexist with

\[
 \mathscr D=0                                      \tag{19}
\]

if the sourced static capacity solution never enters the Maxwell principal
symbol. Conversely, weak static lensing can be tested through equation (14)
before the nonlinear radiative tensor completion is known. Both must
ultimately descend from the same finite action, but they are distinct gates.

---

## 8. One-action pass criterion

The common finite action must generate one sourced static background and four
blind functional responses:

\[
 \begin{aligned}
 a_m&:\text{ slow-body force},\\
 a_t&:\text{ material-clock rate},\\
 a_0&:\text{ temporal Maxwell characteristic},\\
 a_s&:\text{ spatial/Hodge Maxwell characteristic}.
 \end{aligned}                                     \tag{20}
\]

A pass requires:

1. the same source preparation and static solution in all four measurements;
2. no inserted refractive index, metric coefficient, or physical lensing
   target;
3. nonzero $\mathscr D=\mathscr S$ from independently measured rays and
   delays;
4. agreement of $a_0$ with the material lapse response if universal local time
   is claimed;
5. polarization-independent Maxwell response in the isotropic fixture;
6. stability under finite volume, impact parameter, and weak-source scaling;
   and
7. only after the blind result, comparison with the three reference classes.

---

## 9. Next locked gate

Derive a capacity-weighted cotangent Maxwell principal operator from the same
finite parity-staggered transaction law that carries the tensor sector. Freeze
its source and coefficients before evaluating a ray. The desk certificate must
first compute $a_0/a_m$ and $a_s/a_m$ from the blocked action; only then may a
pre-registered engine fixture measure equations (14) and (16).

Until that operator exists, FTD's lensing status remains **OUT** and FTD-1020
class 0 remains the authoritative production result.

The
[common-admission clock/Maxwell theorem](../common_action_mechanics_reciprocity/THEOREM_COMMON_ADMISSION_CLOCK_MAXWELL_AND_SPATIAL_LENSING_PRICE_v1.md)
now derives one conditional part of that operator: one retained permission
history shared by the material clock and complete Maxwell advance forces
$a_0=a_t$. If $a_t=a_m$, this gives $\mathscr D=1$ and no more. The remaining
equal-response half is precisely the spatial/Hodge coefficient $a_s$, whose
finite primal/dual transaction remains open.

The successor
[primal/dual permission theorem](../common_action_mechanics_reciprocity/THEOREM_PRIMAL_DUAL_PERMISSION_IDEMPOTENCE_AND_LENSING_FACTOR_PRICE_v1.md)
proves that one binary permission cannot supply that half by a duplicated
read. A separately retained primal/dual pair admits exact reversible
factorization and would give $c_{\rm ray}=\nu^2/6$ under equal marginals. The
action must still generate the pair, its equality, and its physical
inhomogeneous Hodge operator before the blind fixture is licensed.

The later
[dual-capacity mixing theorem](../common_action_mechanics_reciprocity/THEOREM_DUAL_CAPACITY_CORRELATION_OBSTRUCTION_AND_CYCLIC_MIXING_RESPONSE_v1.md)
shows why this factorization is dynamical rather than automatic. Collocated
source deficits give a $3/2$ boundary, while a reversible dual-layer mixing
cycle gives exact $\bar j=\nu_t\nu_s$ for arbitrary finite counts. The source
ledger and $M(U)$ remain prerequisites for the blind fixture. The
[3D $O_h$ mixing successor](../common_action_mechanics_reciprocity/THEOREM_OH_MOORE_LOCAL_DUAL_CAPACITY_MIXING_AND_ISOTROPIC_FACTOR_PASS_v1.md)
now closes isotropic zero-drift factorization as a reference construction;
native schedule selection, the C18/cotangent composition, and the sourced
inhomogeneous operator remain open.

The
[self-dual trace-capacity successor](THEOREM_SELF_DUAL_TRACE_CAPACITY_STATIC_POLE_AND_EQUAL_RESPONSE_LENSING_BOUNDARY_v1.md)
now supplies a complete conditional response witness. The trace of the same
actualization moment sources a selected primal/dual action whose symmetric
mode has pole \(1/(\kappa\Lambda)\) and whose solution obeys \(U_t=U_s\).
Under explicitly selected normalized matter, clock, Maxwell-time, and
Hodge-space readouts, it gives

\[
 (a_m,a_t,a_0,a_s)=(1,1,1,1),\qquad
 \mathscr D=\mathscr S=2.
\]

This passes the algebraic discriminator but not the native-action gate: the
equal source coupling, readouts, finite ownership, inhomogeneous Maxwell
operator, and vector constraint sector remain to be derived. FTD-1020 class
zero remains the production result.
