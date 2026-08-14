# FTD-0954 — Phase-locked canonical charge transfer and global-phase boundary v1

**Date:** 2026-08-11  
**Status:** `[THEOREM — POLAR BODY PHASE/CHARGE CHART]` +
`[THEOREM — LOCAL PHASE-REACTING CANONICAL CHARGE/ACTION SHEAR]` +
`[THEOREM — EXACT PHASE-LOCKED ENDPOINT ENERGY/CHARGE/INVERSE CLOSURE]` +
`[THEOREM — FINITE POSITIVE RESERVE AND FAIL-CLOSED BACKPRESSURE]` +
`[CLOSED NEGATIVE — GLOBAL PERIODIC PHASE-INDEPENDENT SEPARABLE SHEAR]` +
`[CONDITIONAL REFERENCE — POSITIVE COMPLETE-MODE EXCHANGE]` +
`[OPEN — GLOBAL AUTONOMOUS PHYSICAL HAMILTONIAN, NATIVE RESERVOIR, SYNCHRONIZATION, 3D RECYCLING, STABILITY, PRODUCTION]`  
**Verdict:** `OUTCOME_B_PHASE_LOCKED_CANONICAL_RESERVOIR_GLOBAL_NATIVE_FORMATION_OPEN`

## 1. Result

The phase-blind reservoir obstruction of FTD-0952/0953 has an exact minimum
local repair. The recursive body's axial angle and charge must be retained
together with the reservoir's phase and action. Their relative phase supplies
the reciprocal momentum response that a scalar action debit omitted.

On one declared co-rotating phase section, a local Hamiltonian shear moves the
exact charge required by the target-blind nonlinear Routh update into a
positive reservoir. Composed with the already certified positive Routh port,
the resulting discrete gate is:

- symplectic on the full local phase space;
- exactly axial-charge conserving;
- exactly endpoint physical-energy conserving;
- exactly reversible;
- local and target-blind in the FTD-0953 sense; and
- finite-capacity with explicit fail-closed backpressure.

This is a stroboscopic section theorem, not a native autonomous formation
law. The transfer generator uses a local lift of the phase circle. A globally
periodic separable generator cannot produce the same nonzero debit
independently of phase. A global positive complete-mode exchange exists, but
it forms a prescribed nonlinear target only when that target mode is already
prepared in the reservoir.

## 2. The body already contains the required phase/charge pair

At one active transverse site write

\[
 q=\rho e_r(\theta),
 \qquad
 p=p_\rho e_r(\theta)+{L\over\rho}e_\theta(\theta),
 \qquad \rho>0.                                           \tag{1}
\]

Direct differentiation gives

\[
 \boxed{p\cdot dq=p_\rho\,d\rho+L\,d\theta},
 \qquad
 \boxed{q\wedge p=L}.                                    \tag{2}
\]

Thus the physical transverse field has the canonical chart

\[
 \Omega_B=d\rho\wedge dp_\rho+d\theta\wedge dL.          \tag{3}
\]

The axial charge is the momentum conjugate to the body phase. It is not a
scalar label appended after the fact. On the registered circular branch,

\[
 p_\rho=0,
 \qquad
 L=\sigma\omega\rho^2,
 \qquad \sigma\in\{-1,+1\},\quad\omega>0,                \tag{4}
\]

with `rho=A_0 phi`. Equation (4) is the local version of the FTD-0948/0949
axial charge on the selected rotating branch.

## 3. Exact target-blind charge increment

During one eight-color layer, inactive neighbours are held fixed. Let
`rho_*` be the unique local minimizer already computed by the FTD-0953
target-blind coordinate solve. Define

\[
 \boxed{D=\sigma\omega(\rho_*^2-\rho^2).}                 \tag{5}
\]

No completed tailed profile is read. `D` depends only on the current active
amplitude, current inactive neighbours, core marker, and selected nonlinear
action parameters.

On the circular ready section, `D` is exactly the axial-charge change needed
to carry equation (4) from `rho` to `rho_*`:

\[
 L+D=\sigma\omega\rho_*^2.                               \tag{6}
\]

## 4. The phase-reacting canonical shear

Give the site one complete reservoir pair `(vartheta,I)` with

\[
 \Omega_R=d\vartheta\wedge dI,
 \qquad
 E_R=\omega I,
 \qquad
 Q_R=\sigma I,
 \qquad I>0.                                             \tag{7}
\]

On a local lift of the two phase circles, introduce the unit transfer pulse

\[
 \boxed{H_{\rm tr}=(\sigma\vartheta-\theta)D.}            \tag{8}
\]

Because `D` is a configuration function and the configurations are fixed
during this shear, its time-one flow is exact:

\[
 \begin{aligned}
 \rho'&=\rho,\\
 p_\rho'&=p_\rho+(\theta-\sigma\vartheta)\partial_\rho D,\\
 \theta'&=\theta,\\
 L'&=L+D,\\
 \vartheta'&=\vartheta,\\
 I'&=I-\sigma D.
 \end{aligned}                                           \tag{9}
\]

The exact Jacobian satisfies

\[
 J^{\mathsf T}\Omega J=\Omega,
 \qquad \det J=1,                                       \tag{10}
\]

on the full six-dimensional chart, not merely on the operating section. The
map also preserves

\[
 \boxed{L+\sigma I}.                                     \tag{11}
\]

Equation (9) displays the missing dynamics. A phase-blind debit retained only
`I'=I-sigma D` and produced the FTD-0953 two-form defect. The canonical map
also gives the radial conjugate the response

\[
 \Delta p_\rho=(\theta-\sigma\vartheta)\partial_\rho D.  \tag{12}
\]

It is this term—not an additional verbal interpretation—that restores the
symplectic form.

## 5. Co-rotation is the zero-recoil gate

The selected operating section is

\[
 \boxed{\theta=\sigma\vartheta}                          \tag{13}
\]

inside one declared phase chart. On (13), equation (12) vanishes while the
charge/action transfer remains exact. A phase error `delta` instead produces

\[
 \Delta p_\rho=\delta\,\partial_\rho D.                  \tag{14}
\]

Thus phase mismatch is not silently ignored: it backreacts on the radial
motion. Whether that feedback restores or destabilizes phase lock is not
proved here and remains a preregistered stability problem.

The two phases are the minimum operational content of the proposed
left/right or self/other recursion: one is the body's actual orientation,
the other is the environment's counter-account. No biological hemisphere or
consciousness identification follows.

## 6. Exact physical endpoint ledger

Apply the charge shear immediately before the FTD-0953 positive nonlinear
Routh-port quarter-turn. On the circular, phase-locked, zero-conjugate ready
section, the field and port obey

\[
 \Delta H_{\rm rot}+\Delta E_{\rm port}
 =\sigma\omega D.                                        \tag{15}
\]

The reservoir obeys

\[
 \Delta E_R=\omega\Delta I=-\sigma\omega D.             \tag{16}
\]

Therefore

\[
 \boxed{
 \Delta(H_{\rm rot}+E_{\rm port}+E_R)=0,}                \tag{17}
\]

while equation (11) gives exact total axial-charge conservation. The inverse
first reverses the Routh port, recovering the old local data, and then runs
the shear backward. Every amplitude, conjugate, port output, body charge,
reservoir action, and phase is recovered.

Equation (17) is an exact finite-gate endpoint invariant on the declared
section. It does not assert that (8) plus the nonlinear body law is one
globally autonomous positive continuum Hamiltonian at intermediate times.

## 7. Locality, capacity, and backpressure

One reservoir pair is assigned to each simultaneously active site. Same-
color C18 sites have disjoint active canonical coordinates, so their transfer
Hamiltonians Poisson commute with inactive neighbours held fixed.

For `sigma D>0`, reservoir positivity requires

\[
 I>\sigma D.                                             \tag{18}
\]

The sign-independent rule `I>|D|` is sufficient. A finite grounded region
and finite color depth therefore need finite total action and finitely many
complete pairs.

If (18) fails, the gate rejects the transfer. Clipping, saturating, resetting,
or erasing the action would destroy the inverse and is forbidden. This is the
first explicit physical backpressure rule for the nonlinear charge account.
It does not solve indefinite replenishment or port return.

## 8. Why the local shear is not global on one phase circle

Let

\[
 \delta=\theta-\sigma\vartheta.                           \tag{19}
\]

A globally single-valued separable pulse

\[
 H_{\rm per}=g(\delta)D                                  \tag{20}
\]

that produces the same nonzero charge shear at every phase would require
`g'(delta)` to be one fixed nonzero constant. But periodicity gives

\[
 \int_0^{2\pi}g'(\delta)d\delta=0.                       \tag{21}
\]

Hence:

\[
 \boxed{
 \text{no nonzero phase-independent separable shear is generated by one
 globally periodic }g.}                                  \tag{22}
\]

This is a topology/scheduling boundary, not a no-go for all phase-reacting
dynamics. Phase windows, multiple strokes, phase-dependent exchange, and
complete-mode transfer remain admissible.

## 9. Positive global complete-mode control

For two complete action modes `B=(x_b,y_b)` and `R=(x_R,y_R)`, define

\[
 N={1\over2}(|B|^2+|R|^2),
 \qquad
 G=x_b y_R-x_R y_b.                                      \tag{23}
\]

The identities

\[
 N\pm G={1\over2}\left[(x_b\pm y_R)^2+(x_R\mp y_b)^2\right]
 \ge0                                                     \tag{24}
\]

give `|G|<=N`. Thus

\[
 H_{\rm ex}=4\kappa N+\kappa G
 \ge3\kappa N>0                                         \tag{25}
\]

away from zero. At `T=pi/(2kappa)`, its base mode makes one complete winding
and its species mode makes a quarter-turn:

\[
 \boxed{(B,R)\mapsto(-R,B).}                             \tag{26}
\]

The map is orthogonal, symplectic, determinant one, fourth order, and
conserves total action. It is a global periodic-phase compatibility witness.

It does not solve target-blind nonlinear formation. With a blank reservoir,
equation (26) empties the body. To place a context-dependent target `B_*` in
the body, the incoming reservoir must already contain `-B_*`. The target has
been prepared elsewhere rather than generated by the exchange.

## 10. Ontology and representation

Existing FTD common/relative canonical variables can represent two complete
modes. This means FTD-0954 does not force a new primitive type merely to write
equations (8) or (25).

It does not prove that the production common field is the reservoir. The
following remain selected or open:

- the reservoir identity, normalization, orientation, phase origin, and
  initial action;
- native formation, replenishment, and erasure/reversal protocol;
- local phase synchronization and recovery;
- finite three-dimensional routing, congestion, return, and recycling;
- autonomous color scheduling and stopping;
- stability, mobility, collision/backpressure beyond the capacity gate,
  mass, scale, and production normalization;
- the ordered two-frame/pseudoscalar source and `gamma`;
- quartic-`G*` synchronization, Born/Bell recovery, Lorentz hiding, and
  completeness; and
- every production-engine integration.

## 11. Certificate

The frozen protocol SHA-256 is
`E734CB02FFC6980844488E7AD2C4BEAF09422DFB92BC115D396ED925927FD6A7`.
The first immutable certificate SHA-256 is
`C08625778490F7559311CB8A24A6E04BA150D8009302DC6BD5F8405507FE0257`.
Its first execution passes `83/83`, Outcome B, without repair.

No numerical search, parameter scan, floating tolerance, empirical
substitution, engine source, CMake file, `Voxel` type, production field,
constant, toggle, or default tick phase changed.

## 12. Next gate

The next admissible test is no longer “add phase reaction.” That debt is
closed on the local co-rotating section. The next test must globalize and
stabilize it without preloading the nonlinear target:

1. compile a globally periodic multi-stroke phase-window Hamiltonian;
2. book clock work and require the controller action to return;
3. prove exact endpoint energy, charge, inverse, and fail-closed reserve;
4. perturb the relative phase and determine whether equation (14) restores
   or destroys lock;
5. retain every outgoing port and controller distinction; and
6. forbid final-profile, measurement-context, outcome, probability, Born,
   Bell-setting, or `G*` reads.

```text
POLAR_BODY_PHASE_CHARGE_CHART=EXACT
LOCAL_PHASE_REACTING_CHARGE_SHEAR=EXACT
PHASE_LOCKED_RADIAL_REACTION=ZERO
OFF_LOCK_RECIPROCAL_RADIAL_KICK=EXACT
TOTAL_AXIAL_CHARGE=CONSERVED
ENDPOINT_PHYSICAL_ENERGY_ON_READY_SECTION=CONSERVED
FINITE_POSITIVE_RESERVE=SUFFICIENT
INSUFFICIENT_RESERVE=FAIL_CLOSED
GLOBAL_PERIODIC_PHASE_INDEPENDENT_SEPARABLE_SHEAR=IMPOSSIBLE
POSITIVE_COMPLETE_MODE_EXCHANGE=EXACT_PREPARED_CONTROL
GLOBAL_AUTONOMOUS_NATIVE_RESERVOIR=OPEN
GSTAR_BORN_BELL_LORENTZ_COMPLETENESS=UNTOUCHED
PRODUCTION_INTEGRATION=FORBIDDEN
```

