# FTD-0904 — oriented even-self-pair rectifier and G* gear-ratio boundary v1

**Identifier:** `FTD-0904`  
**Status:** `[PRE-REGISTRATION — LOCKED/PRE-RUN]`  
**Date:** 2026-08-11  
**Production status:** unchanged

## 1. Question

Can the FTD-0903 rest-sector critical gearbox produce nonzero cycle-averaged
common transport without an externally timed clutch, and what local
orientation data are mathematically necessary?

For a linearly polarized relative clock, write

\[
D=q e,
\qquad |e|=1,                                            \tag{1}
\]

where `e` is a retained polar axis. Let `chi in {-1,+1}` be the retained
clockwise/counterclockwise branch. The registered candidate is the even
connection

\[
A(q,e,\chi)=\chi\gamma q^2e.                            \tag{2}
\]

Away from `q=0`, (2) factorizes as

\[
A=\chi\gamma\,\operatorname{sgn}(q)U(D),
\qquad U(D)=|D|D.                                       \tag{3}
\]

Equation (2), not the discontinuous factorization (3), is the adopted
**[IMPOSED reference coupling law]**. The effective law is smooth in `q`.
It does not derive `e`, `chi`, `gamma`, or a production controller.

## 2. Frozen sources

| source | SHA256 |
|---|---|
| `THEOREM_I_GAMMA_QUARTIC_SQUARE_SPLIT_v1.md` | `07BDB4CA22A655C378BCC4BA4B6A69830686200A4B4F59B19136363F5F4F6496` |
| `THEOREM_LOCAL_FLUX_SELF_PAIR_TENSOR_RECURSION_v1.md` | `62A95FF322C99773D03002444376B9244A93CC19D01CF4400230277288CADAEB` |
| `THEOREM_CUBIC_REACTION_VECTOR_AND_RELATIVISTIC_SOURCE_TRANSPORT_BOUNDARY_v1.md` | `56F3DF2B830A5C52320757DAF368EAA72F3E4A4B1DA388090A2E1EB7F30C2D27` |
| `THEOREM_POSITIVE_CONNECTION_ORDER_AND_SELF_PAIR_CRITICAL_GEARBOX_BOUNDARY_v1.md` | `C6504B179463E2AA93F3B93F29FD672BC96771AF2BB9184A0FB1E1214F98F21D` |
| `self_pair_connection_critical_gearbox.h` | `038F48F4E99D3CD55CAE25CF09170670733057FF1A43279839D3C78B0DC74447` |

Any source-hash mismatch invalidates the certificate.

## 3. Inversion-equivariant rectification boundary

Let `F:R^3->R^3` be a polar-vector rectifier depending only on one polar
vector `D`. If it is even,

\[
F(-D)=F(D),                                              \tag{4}
\]

but equivariant under spatial inversion `Q=-I`, then

\[
F(-D)=QF(D)=-F(D).                                      \tag{5}
\]

Equations (4)--(5) force `F=0`. Therefore a nonzero even polar rectifier
cannot be both inversion-equivariant and a function of `D` alone. It must
either:

1. retain auxiliary inversion-odd polar data such as `e`;
2. break inversion symmetry explicitly; or
3. produce a non-polar output.

The candidate (2) uses option 1. Under every signed cubic transform `Q`,

\[
(q,e)\mapsto(q,Qe),
\qquad A(q,Qe,\chi)=QA(q,e,\chi).                       \tag{6}
\]

Thus no global preferred direction is introduced by the law, although each
prepared branch carries a local oriented axis whose formation and retention
remain open.

## 4. Positive Hamiltonian and clock

Adopt

\[
L=\frac M2|\dot C|^2+\frac m2\dot q^2
  +\chi\gamma q^2 e\cdot\dot C-\lambda q^4,
\qquad M,m,\lambda>0.                                   \tag{7}
\]

Then

\[
P=M\dot C+\chi\gamma q^2e,
\qquad \pi=m\dot q,                                    \tag{8}
\]

\[
H=\frac{|P-\chi\gamma q^2e|^2}{2M}
 +\frac{\pi^2}{2m}+\lambda q^4.                        \tag{9}
\]

The mechanical common momentum is

\[
K=P-\chi\gamma q^2e,
\qquad \Delta K=-\chi\gamma\Delta(q^2)e.              \tag{10}
\]

At `P=0`, define

\[
\Lambda=\lambda+\frac{\gamma^2}{2M}>0.                \tag{11}
\]

The rest Hamiltonian is exactly

\[
H_{P=0}=\frac{\pi^2}{2m}+\Lambda q^4.                  \tag{12}
\]

Hence the origin clock Hessian is zero and the period-amplitude product is

\[
T a=\sqrt\pi\,G^*\sqrt{\frac{m}{2\Lambda}}.            \tag{13}
\]

At nonzero `P`, (9) contains

\[
-\frac{\chi\gamma}{M}q^2(P\cdot e),                   \tag{14}
\]

so the exact quartic remains rest-sector or the special transverse sector
`P dot e=0`; it is not generic moving-clock closure.

## 5. Exact continuum rectification

Let the nontrivial rest orbit have turning amplitude `a>0`. From (12),

\[
\dot q=\sqrt{\frac{2\Lambda}{m}}\sqrt{a^4-q^4}.        \tag{15}
\]

The certificate must prove

\[
\int_0^1\frac{dx}{\sqrt{1-x^4}}
=\frac14B\!\left(\frac14,\frac12\right)
=\frac{\sqrt\pi G^*}{4},                               \tag{16}
\]

\[
\int_0^1\frac{x^2\,dx}{\sqrt{1-x^4}}
=\frac14B\!\left(\frac34,\frac12\right)
=\frac{\sqrt\pi}{G^*}.                                \tag{17}
\]

At `P=0`, `dot C=-(chi gamma/M)q^2e`. Therefore one full
clock cycle gives

\[
\boxed{
\Delta C_T
=-\frac{4\sqrt\pi\,\chi\gamma}{M G^*}
  a\sqrt{\frac{m}{2\Lambda}}\,e.}                     \tag{18}
\]

Dividing (18) by (13) gives the exact mean gear ratio

\[
\boxed{
\frac{\overline{\dot C}\cdot e}{a^2}
=-\frac{4\chi\gamma}{M(G^*)^2}.}                      \tag{19}
\]

Equations (13), (18), and (19) are exact beta/gamma consequences of the
imposed reference action. They are not numerical matches, substrate
predictions, or integer-tick cadence results.

## 6. Orientation and reversal

The retained `chi` is time-odd orientation data. The branch-paired map

\[
\Theta:(C,q,P,\pi,e,\chi)
\mapsto(C,q,-P,-\pi,e,-\chi)                            \tag{20}
\]

leaves (9) invariant. Reversing `chi` or `e` reverses (18)--(19). A fixed
`chi` branch is not by itself invariant under naive momentum reversal.

This separates the two roles:

```text
E_SUPPLIES_POLAR_AXIS=IMPOSED_REFERENCE_DATA
CHI_SUPPLIES_CLOCKWISE_COUNTERCLOCKWISE_ORIENTATION=TRUE
GAMMA_MAGNITUDE_DERIVED_FROM_CHI_OR_I=FALSE
```

The certificate must not represent a BCC symmetric square, a scalar clock, or
`q^2` alone as retaining the sign of `e` or `chi`.

## 7. Registered discrete rest-sector witness

Reuse the exact quartic discrete-gradient recursion with coupling `Lambda`.
For one signed nonzero step, add

\[
C_1-C_0
=-\frac{h\chi\gamma}{2M}(q_1^2+q_0^2)e.              \tag{21}
\]

The certificate must prove:

- exact quartic energy conservation through (12);
- exact mechanical endpoint identity (10);
- exact equal-and-opposite channel impulse and reciprocal carry conditional
  on supplied `p_*`;
- exact endpoint-exchange/`h -> -h` reversal of (21) together with the child
  quartic map;
- signed-cubic covariance conditional on transforming `e`;
- monotone per-step directed displacement for fixed nonzero `chi gamma h`;
- the inversion-equivariant even-vector no-go (4)--(5);
- the continuum identities (13), (18), and (19); and
- all axis, chirality, scale, mass, controller, production, Born, and cadence
  firewalls.

The finite discrete map is not exact continuum flow and need not close after
an integer number of global ticks. It does not read `G*`.

## 8. Outcome map

- **Outcome A:** the symmetry price, positive rest Hamiltonian, exact
  rectification formulas, discrete energy/impulse/carry/reversal, and scope
  gates all close. Book (2) as an imposed oriented reference gearbox and keep
  axis/chirality formation, gamma, scale, mass, production, and integer-tick
  cadence open.
- **Outcome B:** a frozen source already derives the retained polar axis,
  clockwise/counterclockwise branch, or normalization from substrate dynamics.
  Identify that source explicitly before any promotion.
- **Outcome C:** any symmetry, positivity, clock, transport, reversal, beta/
  gamma, or scope gate fails. Book no theorem.
- **Execution invalid:** any frozen hash or terminal firewall fails.

## 9. Post-certificate implementation

Only after a passing locked certificate, add an isolated `ftd::eft`
linearly-polarized analyzer. It must compose the existing effective-quartic
recursion and carry witness and report axis normalization, chirality,
connection endpoints, common displacement, mechanical impulse, rest energy,
reversal, moving-sector coefficient, continuum period/displacement/mean-speed
ratios, and all open-debt flags. It must fail closed on nonfinite input,
nonpositive masses/coupling/tolerance or scale, zero chirality, nonunit axis,
zero step, child failure, overflow, endpoint mismatch, or inverse failure.

## 10. Scope firewall

```text
EVEN_POLAR_RECTIFIER_FROM_D_ALONE_WITH_INVERSION_EQUIVARIANCE=ZERO
RETAINED_POLAR_AXIS_REQUIRED=TRUE_IN_REGISTERED_CLASS
CLOCKWISE_COUNTERCLOCKWISE_BRANCH_REQUIRED_FOR_TIME_REVERSAL=TRUE
ORIENTED_EVEN_CONNECTION=IMPOSED_REFERENCE_LAW
REST_SECTOR_CRITICAL_QUARTIC=EXACT
REST_SECTOR_CONTINUUM_GSTAR_PERIOD_FACTOR=EXACT
CONTINUUM_DISPLACEMENT_PER_CYCLE_PROPORTIONAL_TO_INVERSE_GSTAR=EXACT
CONTINUUM_MEAN_GEAR_RATIO_PROPORTIONAL_TO_INVERSE_GSTAR_SQUARED=EXACT
DISCRETE_DIRECTED_COMMON_DISPLACEMENT=EXACT_PER_STEP
MECHANICAL_COMMON_IMPULSE=EXACTLY_EXCHANGED_WITH_Q_SQUARED
MOVING_SECTOR_EXACT_QUARTIC=FALSE_GENERICALLY
GAMMA_MAGNITUDE_DERIVED_FROM_CHI_OR_I=FALSE
POLAR_AXIS_SUBSTRATE_FORMATION=OPEN
CHI_SUBSTRATE_FORMATION=OPEN
PHYSICAL_MOMENTUM_SCALE=OPEN
ABSOLUTE_MASS=NOT_DERIVED
INTEGER_TICK_GSTAR_CADENCE=OPEN
PRODUCTION_INTEGRATION=FORBIDDEN
NO_NEW_SELECTED_TYPE=TRUE
BORN_BELL_LORENTZ_COMPLETENESS=UNTOUCHED
```

The exact SHA256 of this protocol and its certificate must be entered in the
preregistration manifest before first execution.
