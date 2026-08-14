# Theorem — Oriented even self-pair rectifier and G* gear-ratio boundary v1

**Identifier:** `FTD-0904`  
**Date:** 2026-08-11  
**Status:** `[THEOREM — INVERSION-EQUIVARIANT EVEN-POLAR RECTIFIER NO-GO]` +
`[THEOREM — EXACT CONDITIONAL ORIENTED REST-SECTOR RECTIFIER]` +
`[THEOREM — EXACT G* INVERSE GEAR RATIOS]` +
`[BOUNDARY — RETAINED POLAR AXIS AND TIME-ODD CHIRALITY REQUIRED]` +
`[REFERENCE IMPLEMENTATION — ISOLATED ftd::eft]`

## 1. Symmetry price of rectification

Let `F:R^3 -> R^3` be a polar-vector rectifier depending only on one polar
clock vector `D`. If it is even,

\[
F(-D)=F(D),                                               \tag{1}
\]

and equivariant under spatial inversion `Q=-I`, then

\[
F(-D)=QF(D)=-F(D).                                       \tag{2}
\]

Equations (1)--(2) imply `F=0`. Thus a nonzero even polar rectifier cannot be
both inversion-equivariant and a function of `D` alone. Within this
registered class it must retain auxiliary inversion-odd polar data, break
inversion symmetry, or produce a non-polar output.

The reference construction retains a local unit polar axis `e` and a
clockwise/counterclockwise branch `chi in {-1,+1}`. It adopts the
**[IMPOSED reference coupling law]**

\[
D=q e,
\qquad
A(q,e,\chi)=\chi\gamma q^2e.                            \tag{3}
\]

Under every signed cubic transformation `Q`, `e -> Qe` and

\[
A(q,Qe,\chi)=QA(q,e,\chi).                              \tag{4}
\]

The law therefore introduces no global preferred direction. Each prepared
branch nevertheless carries a local oriented axis. The formation,
maintenance, and erasure of `e` and `chi` are not derived here.

## 2. Positive action and exact rest clock

Adopt

\[
L=\frac M2|\dot C|^2+\frac m2\dot q^2
  +\chi\gamma q^2 e\cdot\dot C-\lambda q^4,
\qquad M,m,\lambda>0.                                   \tag{5}
\]

The canonical momenta and Hamiltonian are

\[
P=M\dot C+\chi\gamma q^2e,
\qquad \pi=m\dot q,                                    \tag{6}
\]

\[
H=\frac{|P-\chi\gamma q^2e|^2}{2M}
 +\frac{\pi^2}{2m}+\lambda q^4.                        \tag{7}
\]

The mechanical common momentum

\[
K=P-\chi\gamma q^2e                                    \tag{8}
\]

obeys the exact endpoint identity

\[
\boxed{\Delta K=-\chi\gamma\Delta(q^2)e.}              \tag{9}
\]

At `P=0`, define

\[
\Lambda=\lambda+\frac{\gamma^2}{2M}>0.                \tag{10}
\]

Then

\[
\boxed{H_{P=0}=\frac{\pi^2}{2m}+\Lambda q^4.}          \tag{11}
\]

The even connection has zero linearization at the origin and contributes
only to the positive quartic coefficient in the rest sector. The critical
clock and its exact continuum period-amplitude law survive:

\[
\boxed{Ta=\sqrt\pi\,G^*\sqrt{\frac{m}{2\Lambda}}.}      \tag{12}
\]

For generic nonzero `P`, (7) contains

\[
-\frac{\chi\gamma}{M}q^2(P\cdot e).                    \tag{13}
\]

The pure-quartic theorem is therefore restricted to `P=0` or the special
transverse sector `P dot e=0`.

## 3. Exact continuum gearbox

For a nontrivial rest orbit with turning amplitude `a>0`, (11) gives

\[
\dot q=\sqrt{\frac{2\Lambda}{m}}\sqrt{a^4-q^4}.        \tag{14}
\]

The two relevant beta integrals are

\[
\int_0^1\frac{dx}{\sqrt{1-x^4}}
=\frac14B\!\left(\frac14,\frac12\right)
=\frac{\sqrt\pi G^*}{4},                              \tag{15}
\]

\[
\int_0^1\frac{x^2\,dx}{\sqrt{1-x^4}}
=\frac14B\!\left(\frac34,\frac12\right)
=\frac{\sqrt\pi}{G^*}.                               \tag{16}
\]

At `P=0`, `dot C=-(chi gamma/M)q^2e`. Integration over a full clock cycle
therefore yields

\[
\boxed{
\Delta C_T
=-\frac{4\sqrt\pi\,\chi\gamma}{M G^*}
  a\sqrt{\frac{m}{2\Lambda}}\,e.}                     \tag{17}
\]

Dividing by (12) gives

\[
\boxed{
\frac{\overline{\dot C}\cdot e}{a^2}
=-\frac{4\chi\gamma}{M(G^*)^2}.}                     \tag{18}
\]

This is the exact gearbox:

- `G*` controls the quartic traversal time;
- `1/G*` controls oriented displacement per cycle; and
- `1/(G*)^2` controls mean speed per squared clock amplitude.

These are beta/gamma identities inside the imposed action. They are not
numerical near-misses, fitted predictions, a substrate derivation of `G*`, or
an integer-tick cadence.

## 4. Clockwise/counterclockwise information and reversal

The branch variable `chi` is time-odd orientation data. The branch-paired
map

\[
\Theta:(C,q,P,\pi,e,\chi)
\mapsto(C,q,-P,-\pi,e,-\chi)                            \tag{19}
\]

leaves (7) invariant. Reversing either `chi` or `e` reverses (17)--(18).
Holding `chi` fixed while reversing momenta is not a time-reversal symmetry
of one directed branch.

This identifies precisely what a symmetric square loses. The scalar `q^2`
retains intensity but not the sign of the polar axis or the
clockwise/counterclockwise sheet. The retained pair `(e,chi)` supplies that
missing orientation information in the reference model. It is not yet a
native BCC or substrate construction.

The complex structure `i^2=-1` can represent a quarter-turn orientation. It
does not determine the real coupling magnitude `|gamma|`, select `e`, or
create and maintain `chi`.

## 5. Exact discrete witness

The isolated implementation reuses the exact quartic discrete-gradient and
reciprocal-carry witness with coupling `Lambda`. Its common endpoint update
is

\[
C_1-C_0
=-\frac{h\chi\gamma}{2M}(q_1^2+q_0^2)e.              \tag{20}
\]

The locked certificate and compiled witness establish:

- exact rest-sector quartic energy conservation;
- exact mechanical impulse exchange (9);
- equal-and-opposite canonical channel impulses;
- reciprocal carry conditional on the supplied `p_*`;
- endpoint-exchange and signed-step reversal on the complete reference
  state;
- signed-cubic covariance conditional on transforming `e`;
- directed displacement for every nonzero registered step;
- branch-paired time reversal and failure of naive fixed-`chi` reversal;
- the moving-sector coefficient (13); and
- the exact continuum identities (12), (17), and (18).

The finite discrete map is not exact continuum flow. It neither reads `G*`
nor forces closure after an integer number of global ticks.

## 6. Epistemic accounting

Theorem-grade conditional on the imposed law (3):

- the inversion-equivariant even-polar no-go (1)--(2);
- the signed-cubic covariance (4);
- the Legendre transform, positivity, and exact rest quartic (6)--(11);
- mechanical impulse exchange and reciprocal-carry composition;
- branch-paired reversal and directed per-step displacement;
- the exact beta identities and inverse-`G*` gear ratios; and
- the moving-sector boundary (13).

Still open:

- native formation and maintenance of the local polar axis `e`;
- native formation, transport, and erasure of the time-odd chirality `chi`;
- derivation and normalization of `gamma`;
- physical identification of `C`, `q`, the channels, and the retained memory;
- `p_*`, total field-matter momentum, carry energy, and absolute mass;
- controller work and the thermodynamic cost of retaining/erasing direction;
- moving-clock closure beyond `P dot e=0`;
- finite-tick `G*` cadence and preferred-tick hiding;
- production coupling and stable constituent formation; and
- Born, Bell-laboratory, and operational Lorentz recovery.

No selected type, adoption currency, fitted value, target-coded probability,
or production integration is added.

## 7. Certificate provenance

The frozen protocol SHA-256 is
`A166A7EA4BBEAFD887DD66B4D4FF1D865D6EF0861688A58ECB1B91E885843C22`.
The frozen exact certificate SHA-256 is
`4627E99F50AA011B5C1FBF439681FB68B60CB341E4E87C9840DB3FB84D6ED0A3`.

The first immutable execution passed all `74/74` gates. No repair protocol
was required. The proof of record is
`scripts/proofs/proof_oriented_even_self_pair_rectifier_gstar_gear_ratio_boundary.py`.

## 8. Isolated reference implementation

The fail-closed reference analyzer is isolated under `ftd::eft`:

- `engine/include/ftd/eft/oriented_even_self_pair_rectifier.h`, SHA-256
  `E59D991BA248BABE3A408BB8D2E31947B48EFF233D0FDBE2F0CED8E958FDFCC8`;
- `engine/src/eft/oriented_even_self_pair_rectifier.cpp`, SHA-256
  `15D75D7672FCA23EC7AFB695576A451B4C00D1A8D1029A309FDA22D623DDEE11`;
- `engine/tests/test_oriented_even_self_pair_rectifier.cpp`, SHA-256
  `3BDEEABC58525178D58129A0F0B248B44A4AA0235859A70A046886DED392738C`.

The pinned MSVC 14.44 build succeeds. The focused Release CTest passes `1/1`
and the actualization/EFT chain passes `29/29`. No production `Voxel`,
renderer, boundary, default toggle, or tick phase was changed.

## 9. Next acceptance gate

Pre-register a native orientation-memory test. It must ask whether a local
substrate history can form and retain `(e,chi)` without importing a new
selected type. At minimum it must:

1. construct `e` from an inversion-odd native observable such as a local
   current, oriented edge history, or phase gradient;
2. construct `chi` from a time-odd ordered pair or discrete circulation, not
   from `q^2` or a symmetric square;
3. prove signed-cubic covariance and branch-paired time reversal;
4. book the energy, work, and information cost of retention and erasure;
5. survive zero-current, inversion-paired, time-reversed, and randomized
   controls; and
6. remain blind to `G*`, measurement context, outcome, and Born weights.

If no native observable retains both signs, the honest outcome is that this
reference gearbox requires additional orientation memory and is not a
substrate-derived clock mechanism.

```text
EVEN_POLAR_RECTIFIER_FROM_D_ALONE_WITH_INVERSION_EQUIVARIANCE=ZERO
ORIENTED_EVEN_CONNECTION=IMPOSED_REFERENCE_LAW
RETAINED_POLAR_AXIS_REQUIRED=TRUE_IN_REGISTERED_CLASS
TIME_ODD_CHIRALITY_REQUIRED_FOR_BRANCH_PAIRED_REVERSAL=TRUE
REST_SECTOR_CRITICAL_QUARTIC=EXACT
REST_SECTOR_CONTINUUM_GSTAR_PERIOD_FACTOR=EXACT
CONTINUUM_DISPLACEMENT_PER_CYCLE_PROPORTIONAL_TO_INVERSE_GSTAR=EXACT
CONTINUUM_MEAN_GEAR_RATIO_PROPORTIONAL_TO_INVERSE_GSTAR_SQUARED=EXACT
DISCRETE_DIRECTED_COMMON_DISPLACEMENT=EXACT_PER_STEP
MOVING_SECTOR_EXACT_QUARTIC=FALSE_GENERICALLY
POLAR_AXIS_SUBSTRATE_FORMATION=OPEN
CHI_SUBSTRATE_FORMATION=OPEN
GAMMA_MAGNITUDE_DERIVED_FROM_CHI_OR_I=FALSE
PHYSICAL_MOMENTUM_SCALE=OPEN
ABSOLUTE_MASS=NOT_DERIVED
INTEGER_TICK_GSTAR_CADENCE=OPEN
PRODUCTION_INTEGRATION=FORBIDDEN
NO_NEW_SELECTED_TYPE=TRUE
BORN_BELL_LORENTZ_COMPLETENESS=UNTOUCHED
```
