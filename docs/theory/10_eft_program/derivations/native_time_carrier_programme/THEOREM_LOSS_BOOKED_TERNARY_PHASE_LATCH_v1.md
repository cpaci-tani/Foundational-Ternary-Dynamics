# FTD-0848 — Loss-booked ternary phase latch

**Status:** `[THEOREM — MINIMUM EVEN POLYNOMIAL THREE-WELL LATCH]` +
`[THEOREM — EXACT LOCAL DAMPED ENERGY/WORK CLOSURE]` +
`[THEOREM — EXPLICIT MANY-TO-ONE TERNARY RECORD QUOTIENT]` +
`[SELECTION/OPEN — LATCH TYPE, POTENTIAL, GATE SCHEDULE, BASIN CONVENTION, PRODUCTION REALIZATION, BORN/SELECTOR COUPLING, MICROSCOPIC BATH, THERMAL COST, AND G* CADENCE]`  
**Date:** 2026-08-10  
**Programme row:** `FTD-0848`  
**Invalid parent:** FTD-0847; all source hashes and C4--C24 passed, then
SymPy raised an undecidable-relational `TypeError` at C25; no theorem booked  
**Repair protocol:**
[`PREREG_LOSS_BOOKED_TERNARY_PHASE_LATCH_CERTIFICATE_REPAIR_v2.md`](../../preregistrations/native_time_carrier_programme/PREREG_LOSS_BOOKED_TERNARY_PHASE_LATCH_CERTIFICATE_REPAIR_v2.md),
pre-run SHA-256
`990507A74D5B57D2ECD97657719D5DB530936AC1697E5FE973876FBAC9A5F795`  
**Repaired certificate:**
[`proof_loss_booked_ternary_phase_latch_v2.py`](../../../../../scripts/proofs/proof_loss_booked_ternary_phase_latch_v2.py),
SHA-256
`53BD66C2E8674169790766E7CEC149739C324673B6F0609A5F984F4F3F60377F`,
`30/30 PASS`  
**Production impact:** none

## 0. Result

FTD-0846 supplies a selected continuous exchange-odd pointer but no law that
turns its history into an actual ternary record. FTD-0848 closes that missing
step at mathematical-reference scope.

For a continuous exchange-odd latch coordinate `x`, the lowest-degree even
polynomial capable of three isolated nondegenerate symmetric minima is degree
six. The selected minimum witness is

\[
 V_T(x)=\beta x^2(x^2-A^2)^2,
 \qquad A,\beta>0.                            \tag{1}
\]

Its stable wells are `-A,0,+A`, its barriers are `+/-A/sqrt(3)`, and the
barrier height is `4 beta A^6/27`. A signed pointer tilts the latch through
`-g r x`; the exact central-barrier removal threshold is derived below.

For every constant-coupling tick, an average-vector-field discrete-gradient
update with positive damping has one onsite next state under an explicit step
condition and satisfies

\[
 \Delta H=-\Delta B,
 \qquad \Delta B\ge0.                         \tag{2}
\]

Every coupling switch has a separate controller-work account. Thus the
recording mechanism neither destroys energy nor hides switching work.

The irreversible actual-record step is not finite-time damping. It is the
many-to-one basin quotient

\[
 \rho(x)\in\{-1,0,+1\}.                       \tag{3}
\]

This supplies an exact mathematical meaning for **unactualization**: detail
inside a basin is not retained in the actual record. A scalar bath-energy
number closes energy but does not retain the discarded microstate.

The latch coordinate, sextic potential, scales, damping, coupling schedule,
and threshold convention are selected types. Nothing here derives the latch
from primitive ternarity, implements it in production, derives a Born rule,
selects a quantum outcome, establishes a Landauer cost, or identifies `G*`
with a finite-tick cadence.

## 1. Degree-six is the symmetric polynomial minimum

An even polynomial with nondegenerate minima at `-A,0,+A` must have at least
one maximum between each adjacent pair of minima. Its derivative therefore
has at least five distinct real zeros. A polynomial of degree at most four
has a derivative of degree at most three, so it cannot meet this condition.
Hence the potential degree is at least six.

Equation (1) attains that floor. Its derivative is

\[
 V_T'(x)=2\beta x(x^2-A^2)(3x^2-A^2).         \tag{4}
\]

The second derivative is positive at `0,+/-A` and negative at
`+/-A/sqrt(3)`. The two barrier values coincide:

\[
 E_b=V_T(A/\sqrt3)=\frac{4\beta A^6}{27}.    \tag{5}
\]

This is a minimum theorem only within the registered class: even polynomial,
three isolated nondegenerate symmetric wells. Non-polynomial, piecewise,
driven, higher-dimensional, and already-discrete latches are outside it.

## 2. Exact acquisition threshold

Let the FTD-0846 pointer value `r` tilt the latch with `f=g r`. On the
positive central interval, a stationary obstruction satisfies

\[
 V_T'(x)=f.
\]

The maximum slope occurs at

\[
 z_* = \frac{6-\sqrt{21}}{15},
 \qquad x_*=A\sqrt{z_*},                      \tag{6}
\]

because the stationary equation for `z=x^2/A^2` is

\[
 15z^2-12z+1=0.                               \tag{7}
\]

Only the smaller root lies in `(0,1/3)`, and the second derivative of the
slope is negative there. Therefore

\[
 F_c=V_T'(x_*)
 =\frac85\beta A^5\sqrt{z_*}(1-2z_*).        \tag{8}
\]

For `f>F_c`, the positive central-side stationary barrier is absent. The
negative result follows by parity. A pulse must still be maintained until
`x` crosses the basin boundary; its duration is a selected control, not an
output of (8).

## 3. Closed pointer--latch and switch-work ledger

After decoupling the clock from the FTD-0846 pointer transaction, select

\[
 U_g(r,x)=\frac{\alpha}{4}r^4+V_T(x)-g r x,
 \qquad \alpha>0.                             \tag{9}
\]

The positive quartic/sextic leading forms make (9) coercive for bounded `g`.
It is exchange invariant under `(r,x)->(-r,-x)`.

At fixed `(r,x)`, switching `g_0` to `g_1` changes system energy by

\[
 \Delta_gU=-(g_1-g_0)rx.                     \tag{10}
\]

Book the controller work as

\[
 \Delta W_{\rm ctrl}=(g_1-g_0)rx.            \tag{11}
\]

Equations (10)--(11) sum exactly to zero. The sign is not fixed: the
controller may supply or recover work.

## 4. Exact local damped transaction

Put

\[
 Q=(r,x),\quad P=(\pi,p_x),\quad
 M=\operatorname{diag}(M_r,M_x),\quad
 \Gamma=\operatorname{diag}(\gamma_r,\gamma_x),                \tag{12}
\]

with positive masses and damping. For one constant-`g` tick define the
average-vector-field discrete gradient

\[
 \overline\nabla U_g(Q_0,Q_1)
 =\int_0^1\nabla U_g((1-\xi)Q_0+\xi Q_1)\,d\xi.                 \tag{13}
\]

Because (9) is polynomial, (13) is exact and obeys the discrete chain rule

\[
 \overline\nabla U_g\cdot(Q_1-Q_0)=U_g(Q_1)-U_g(Q_0).          \tag{14}
\]

Use

\[
 \frac{Q_1-Q_0}{h}=M^{-1}\frac{P_1+P_0}{2},                  \tag{15}
\]

\[
 \frac{P_1-P_0}{h}
 =-\overline\nabla U_g
  -\Gamma M^{-1}\frac{P_1+P_0}{2}.                           \tag{16}
\]

For `H_g=P^TM^{-1}P/2+U_g(Q)`, dot (16) with `Q_1-Q_0` and use
(14)--(15):

\[
 \Delta H_g
 =-\frac1h(Q_1-Q_0)^T\Gamma(Q_1-Q_0).                        \tag{17}
\]

Define

\[
 \Delta B=\frac1h(Q_1-Q_0)^T\Gamma(Q_1-Q_0)\ge0.             \tag{18}
\]

Then `Delta(H_g+B)=0` exactly. Together with (10)--(11), this closes
every piecewise-constant acquisition schedule.

## 5. One onsite next state

The latch curvature has the exact sum-of-squares floor

\[
 V_T''(x)+\frac{14}{5}\beta A^4
 =30\beta(x^2-2A^2/5)^2\ge0.                                  \tag{19}
\]

For `|g|<=g_max`, the Hessian of (9) therefore satisfies

\[
 \nabla^2U_g\succeq-LI,
 \qquad L=g_{\max}+\frac{14}{5}\beta A^4.                    \tag{20}
\]

Indeed, after shifting by `L I`, its quadratic form is a sum of the
nonnegative radial and latch-square terms, `14 beta A^4 u^2/5`, and the
two-by-two block `g_max(u^2+v^2)-2g u v`, whose determinant is
`g_max^2-g^2>=0`.

After eliminating momenta from (15)--(16), the symmetric endpoint Jacobian
is bounded below by

\[
 \delta I,
 \qquad
 \delta=\frac{2M_{\min}}h+\gamma_{\min}-\frac{hL}{2}.          \tag{21}
\]

The factor `1/2` is the integral of the endpoint weight `xi` in (13). If
`delta>0`, the residual is continuous, strongly monotone, and coercive on
the onsite endpoint space, so it has exactly one zero. No neighbour,
measurement context, outcome, probability, `G*`, or target period enters.

## 6. Persistence, loss, and unactualization

After setting `g=0`, define

\[
 \rho(x)=
 \begin{cases}
 -1,&x<-A/\sqrt3,\\
 0,&|x|\le A/\sqrt3,\\
 +1,&x>A/\sqrt3.
 \end{cases}                                                     \tag{22}
\]

The equality convention is deterministic. Equation (22) is odd and has
exactly the production state's ternary codomain.

If the latch begins strictly within one basin with mechanical energy below
`E_b` after decoupling, (17) makes that energy non-increasing. The trajectory
cannot reach either barrier, so the record persists while the inequality
holds.

The continuous damped vector field has phase-volume divergence

\[
 -\frac{\gamma_r}{M_r}-\frac{\gamma_x}{M_x}<0.                 \tag{23}
\]

This contraction is not itself a finite-time erasure proof. The explicit
many-to-one operation is (22). For example,
`x=+A/(2sqrt(3))` and `x=-A/(2sqrt(3))` have equal latch energy and both map
to `s=0`. The actual record does not retain which within-basin microstate was
present.

The bath ledger `B` retains only exported energy. It is not a microscopic
bath state and does not invert (22). A Landauer bound would require additional
thermal-bath, temperature, and protocol types; none is inferred here.

## 7. Certificate record

FTD-0847 passed through C24 and then aborted because SymPy would not order two
expressions carrying the same positive symbolic factor `A`. FTD-0848 changed
only the comparison implementation by dividing through the positive threshold
before exact sign queries. It returned:

```text
FTD-0847 loss-booked ternary phase latch: 30/30 PASS
SEXTIC_IS_THE_MINIMUM_EVEN_POLYNOMIAL_THREE_WELL_LATCH
DAMPED_AVF_TICK_PLUS_BATH_AND_SWITCH_WORK_CLOSES_EXACTLY
TERNARY_BASIN_QUOTIENT_IS_THE_EXPLICIT_MANY_TO_ONE_RECORD_STEP
PRODUCTION_REALIZATION_BORN_SELECTOR_AND_THERMAL_COST_REMAIN_OPEN
FTD-0848 CERTIFICATE_REPAIR_ONLY_C25_C28_NORMALIZED_EXACT_ORDERING
```

## 8. What this changes

The continuous-to-ternary reference problem is no longer undefined. There is
a smallest symmetric polynomial memory architecture, a deterministic local
acquisition threshold, an exact discrete damping/work ledger, and a precise
location where retained information becomes lossy.

What remains is physical provenance. The production `Voxel::state` supplies
only the codomain. The engine does not contain `x`, (1), the coupling `g`, the
controller schedule, the AVF transaction, or the basin map (22). The next
falsifiable gate is to test whether existing native genesis/evaporation and
flux/state transitions realize an equivalent three-basin latch and exported-
energy ledger without reading an outcome, Born weight, measurement context,
`G*`, or target cadence. Only after that recovery may the record be coupled
to the actualization selector.
