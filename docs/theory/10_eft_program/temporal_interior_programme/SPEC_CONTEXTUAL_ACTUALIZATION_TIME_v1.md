# SPEC — Contextual Actualization and Local Time v1

**Status:** `[IMPLEMENTED REFERENCE SPEC — PHYSICAL RECOVERY OPEN]`  
**Branch:** FTD v2 contextual actualization  
**Programme row:** `FTD-0825`  
**Parents:** the v2 successor constitution and the Temporal-Interior v2
charter.  
**Engine scope:** isolated `ftd::eft`; no production `Voxel`, `RenderBridge`,
toggle, or tick-phase consumer.

## 1. Public reference interfaces

| Type | Contract | Epistemic status |
|---|---|---|
| `Region` | finite set of cubic-lattice sites; containment and disjointness | `[DEFINITION]` |
| `PotentialityNet` | actual/potential finite-algebra descriptors and isotony/locality checks | `[SELECTED reference representation]` |
| `PotentialState` | normalized context outcome weights | `[IMPOSED reference representation]` |
| `PreparationMap` | positive weights to normalized reference state | `[AXIOM-class interface; recovery OPEN]` |
| `LocalInstrument` | localized outcome labels | `[IMPORTED measurement type]` |
| `MeasurementContext` | complete joint context and its state–effect weights | `[DEFINITION inside adopted type]` |
| `SelectorState` | deterministic `u in [0,1)` with doubling-map evolution | `[IMPOSED reference realization]` |
| `ActualizationBatch` | one context plus local clock-compliance records | `[DEFINITION]` |
| `ActualizationEvent` | one joint outcome at one global tick | `[CONSTRUCTION]` |
| `CriticalClockState` | phase, accumulated phase parameter, gate count, amplitude, detuning | `[SELECTED interface witness]` |
| `FeedbackAudit` | mixed-coordinate correction magnitudes | `[DIMENSIONLESS REFERENCE DIAGNOSTIC]` |
| `NativePairEnergyState` | retained canonical `(q,p)` lift for the isolated FTD-0840 mechanics | `[SELECTED reference state; not production-native coupling]` |
| `NativePairEnergyStep` | fail-closed implicit solve, pair coordinates, energy/equation residuals, and swept-area orientation | `[CONSTRUCTION — exact discrete-gradient reference]` |
| `TernaryPhaseLatchState` | continuous latch phase-space state plus its retained `-1,0,+1` basin record | `[SELECTED reference state; current production map NOT EQUIVALENT under FTD-0850]` |
| `LossLedger` | exported scalar bath energy and signed controller-switch work | `[CONSTRUCTION — energy closure only; not a microscopic bath state]` |
| `EventReceiverState` | erased odd label `chi` plus nonnegative event-energy account `B` | `[THEOREM — minimum all-energy sign-complete receiver type; physical realization OPEN]` |
| `OddEventPulse` | positive-export signed amplitude, optionally represented by balanced L/R channels | `[CONDITIONAL CONSTRUCTION — one relative degree of freedom; not production-native]` |
| `CausalHistoryRail` | ordered odd amplitudes advanced one causal cell per tick | `[SELECTED exact reference dynamics; production L/R difference is only a candidate channel]` |
| `CubicOddEventDeposit` | six-face opposite L/R impulse plus exact ready-port predicate | `[SELECTED exact reference transaction; production implementation OPEN]` |
| `DiagnosticEventEnergy` | `B=gamma E_REST` removed from the adopted matter diagnostic by manifested-to-void assignment | `[THEOREM conditional on IMPOSED matter-energy role]` |
| `CubicHistoryGearbox` | `D=Q/sqrt(12)` plus the one-cell outward write/shift | `[THEOREM on selected reserved radial receiver; production implementation OPEN]` |
| `ReciprocalRecordPort` | controlled matter/incoming/outgoing identity-or-swap boundary | `[THEOREM — minimum exact reference barrier] + [REFERENCE IMPLEMENTATION — isolated ftd::eft] + [OPEN — physical eligibility]` |
| `NativeEventAcceptance` | source-level genesis/evaporation predicates with four ordered event cases | `[ENGINE FACT conditional on selected hazards/fixed inputs] + [REFERENCE IMPLEMENTATION — isolated ftd::eft]` |
| `RelativeCharacteristicPort` | `i=(p+g)/sqrt2`, `o=(p-g)/sqrt2` energy/current chart | `[THEOREM — exact local chart] + [REFERENCE IMPLEMENTATION — isolated ftd::eft] + [OPEN — protected propagation]` |
| `CatalyticPhaseReference` | conserved nonzero reference frame plus reciprocal matter/zero-signal exchange | `[THEOREM — exact selected reference realization] + [REFERENCE IMPLEMENTATION — isolated ftd::eft] + [OPEN — physical formation/backreaction/production coupling]` |
| `ClockGatedHamiltonianExchange` | full-mode autonomous harmonic hold/swap with transient reference reserve | `[THEOREM — exact imposed reference law] + [REFERENCE IMPLEMENTATION — isolated ftd::eft] + [CLOSED NEGATIVE — universal uncompensated quartic load-blind controller in minimal class]` |
| `TernaryEligibilityClutch` | ternary-square hold/exchange command plus one-shot signal-retained reset request | `[THEOREM — exact reduced selected-reference handshake] + [REFERENCE IMPLEMENTATION — isolated ftd::eft] + [OPEN — autonomous acknowledgement/reset/bath/synchronization/production]` |
| `SignalAcknowledgedTwoStrokeReset` | local signal-completion acknowledgement plus selected nonsmooth finite-time latch reset and empty-port handoff | `[THEOREM — acknowledgement/smooth reset boundary] + [THEOREM, CONDITIONAL — selected cusp reset/ledger] + [REFERENCE IMPLEMENTATION — isolated ftd::eft] + [OPEN — microscopic bath/robustness/production/G* synchronization]` |
| `ReversibleTernarySignalUncomputation` | signal-controlled `Z_3` subtraction that reversibly resets the matching actual ternary latch | `[THEOREM — exact actual-layer reset/minimum oriented record] + [REFERENCE IMPLEMENTATION — isolated ftd::eft] + [CLOSED NEGATIVE — extra logical reset state/bath necessity] + [OPEN — physical controller/transport]` |
| `CollectiveReactionTripletInput/Result` | exact Helmert collective sector, impulse sum, selected-dispersion minimum, inertial curvature, and static-binding mismatch | `[THEOREM — exact reduction inside selected constituent phase space] + [CONDITIONAL THEOREM — composite dispersion/inertia] + [REFERENCE IMPLEMENTATION — isolated ftd::eft] + [OPEN — constituent formation/absolute mass/total Noether momentum]` |
| `BlochQuasimomentumLiftInput/Result` | exact principal wrap, reciprocal-lattice carry, optional winding lift, imposed scale witness, and finite-range sawtooth control | `[THEOREM — exact torus quasimomentum addition] + [THEOREM — global section/finite-range obstruction] + [REFERENCE IMPLEMENTATION — isolated ftd::eft] + [OPEN — winding dynamics/physical momentum scale/total matter-field charge]` |
| `ReciprocalCarryInput/Result` | supplied opposite pair increment, exact principal carries, integer reservoir update, lifted conservation/reversal, imposed physical scale, and band-energy boundary | `[THEOREM — exact conditional reciprocal-carry update] + [THEOREM — unique reservoir increment] + [REFERENCE IMPLEMENTATION — isolated ftd::eft] + [OPEN — impulse origin/reservoir energy/substrate ownership/physical scale]` |
| `QuarticRelativeCarryInput/Result` | selected relative-quartic endpoint, internally generated equal/opposite channel impulse, exact reciprocal chart/carry composition, signed-step reversal, and continuum period-amplitude product | `[THEOREM — exact conditional quartic-relative impulse/energy/carry gearbox] + [REFERENCE IMPLEMENTATION — isolated ftd::eft] + [OPEN — common coupling/channel identification/p_*/carry energy/integer-tick G* cadence]` |

FTD-0846 additionally defines an `OddPhasePointerState=(r,pi)` at
mathematical-reference scope. FTD-0848 adds the two reference interfaces above
and closes one selected continuous-to-ternary persistence law. FTD-0850 proves
that current production genesis/evaporation is not equivalent to that law:
production supplies the ternary codomain, nonzero odd sign, and a many-to-one
loss fragment, but no strict unlocked basin or event-level receiver ledger.
The contextual-actualization core, `ReciprocalRecordPort`, FTD-0858 event/
characteristic interfaces, FTD-0860 `RelativeActionPump`, FTD-0862
`PhaseReferencedActionRail`, FTD-0863 `CatalyticPhaseReference`, and FTD-0865
`ClockGatedHamiltonianExchange`, FTD-0867 `TernaryEligibilityClutch`, FTD-0869
`SignalAcknowledgedTwoStrokeReset`, FTD-0871
`ReversibleTernarySignalUncomputation`, FTD-0872
`OrientedTernaryQuarterTurn`, FTD-0873 `HamiltonianTernaryActuator`, FTD-0874
`AlternatingTernaryParityRail`, FTD-0875 `LocalCanonicalHamiltonianParityRail`,
FTD-0876 `FluxWaveVelocityMarkovCarrier`, FTD-0880
`GaussRecordCanonicalReduction`, FTD-0882
`ReversibleCheckerboardGaussPreparation`, FTD-0884
`FinitePortGaussBattery`, FTD-0886 `CanonicalSourceCenteredGaussGate`, and
FTD-0888 `AutonomousPhaseParitySourceReaction`, FTD-0890
`CubicReactionVectorSourceTransport`, and FTD-0892
`CollectiveReactionTripletInput/Result`, FTD-0893
`DressedBoostMomentumMap`, FTD-0896 `BlochQuasimomentumLift`, and FTD-0897
`ReciprocalCarryInput/Result`, plus FTD-0898
`QuarticRelativeCarryInput/Result` and FTD-0901
`CommonRelativeConnectionState/Parameters/Result`, and FTD-0903
`SelfPairConnectionState/Parameters/Result` now have isolated public
`ftd::eft` C++ witnesses.
The remaining mathematical interfaces are not thereby promoted to production
types. These witnesses test declared contracts; they do not settle native
recovery versus explicit open-system adoption.

## 2. Data flow

```
global tick n
  -> substrate preparation history
  -> PreparationMap -> PotentialState omega
  -> localized instruments and complete context C
  -> context-blind critical-clock eligibility
  -> deterministic contextual selector Sigma_C
  -> one ActualizationEvent
  -> commutative local records
```

The clock decides **when** a batch is eligible. The selector decides **which**
joint outcome is written. Neither mechanism is allowed to read the other's
control variables.

## 3. Selector contract

For normalized weights `w_o = omega(E_o^C)`, the reference selector chooses
the first outcome whose cumulative interval contains `u`. With Lebesgue
equilibrium measure, interval length proves

```
Prob(Sigma_C = o) = w_o.
```

The update `u -> 2u mod 1` preserves Lebesgue measure. These are exact facts
about the imposed reference realization. They do not show that an FTD
preparation supplies Lebesgue equilibrium or the weights `w_o`.

Measurement independence is tested by using the same equilibrium rotor
measure for every context. Bell factorization is intentionally not imposed:
the selector consumes the complete joint context.

## 4. Bell reference context

The selected reference uses the standard singlet joint weights

```
P(a,b | x,y) = (1 - a b cos(x-y))/4,  a,b in {-1,+1}.
```

Each local marginal is `1/2`, independent of the remote setting. The optimal
CHSH angles give `|S| = 2 sqrt(2)`. A PR-box table has `|S| = 4` and is outside
the adopted norm-bounded C*-observable model.

This section verifies the adopted architecture. The singlet, the angle map,
and the operator calculus remain selected/imported.

## 5. Critical clock and feedback

The selected small-amplitude period is

```
T(A,m,lambda) = sqrt(pi) G* sqrt(m/(2 lambda)) / A.
```

The lightweight interface witness tracks:

- global tick `n`;
- unwrapped phase and a duplicate accumulated phase parameter (the legacy
  `operational_duration` / `local_duration` field is not yet physical time);
- compliant gate count;
- amplitude and target amplitude;
- a dimensionless detuning coordinate;
- quantities retained under the legacy names `feedback_work` /
  `controller_work` and `dissipated_energy`, which are dimensionless
  correction diagnostics rather than joules or Hamiltonian energy.

The reference controller uses separate proportional corrections for detuning
and amplitude. Its API accepts no context, setting, weight, outcome, or
instrument. A gate opens only on a registered phase-section crossing while
both amplitude and detuning are within tolerance.

This controller is `[IMPOSED]`. It tests interface coherence and
context-blind gate logic; it does not establish a physical maintenance cost
or derive an actuator from P1–P5.

The physically dimensional maintained reference is implemented separately in
`scripts/experiments/temporal_interior/maintained_gstar_clock_v1.py`. It uses

```
H = p^2/(2m) + mu q^2/2 + lambda q^4,
delta = mu/(2 lambda A^2),
```

and performs a termwise mechanical-energy balance. Even there, recorded
oscillator work is only a lower bound on complete actuator cost because
sensing, controller thermodynamics, and inefficiency remain outside the model.

FTD-0840 adds an exact conservative reference below the maintained-controller
layer. Retain `(q,p)`, derive `u=q|q|`, and adopt

```
H = p^2/(2m) + lambda q^4 = lambda(u^2 + y^2),
y = p/sqrt(2m lambda).
```

Its preregistered symmetric discrete-gradient update has a globally unique
next state, conserves `H` exactly, is reversible, retains one orientation on
every nonzero step, and stays on a compact energy shell. This shows that a
bath is not logically required for isolated clock recurrence. The maintained
controller still requires a work/bath audit because it damps, restores, and
targets a shell. The FTD-0840 pair coupling is not present in production, and
its finite-step map is not exact quartic Hamiltonian flow, so neither a native
local clock nor an exact integer gate cadence follows.

The companion implementation is
`engine/include/ftd/eft/native_pair_energy_recursion.h`, with focused CTest
`native_pair_energy_recursion`. It is deliberately outside the contextual
selector namespace and has no outcome, setting, weight, or instrument input.

FTD-0841 identifies the corresponding production-local state type without
coupling it to the selector. A voxel already carries `(J,W)`, and the
self-pair tensor `U=J otimes J` has `||U||_F^2=|J|^4`. The exact vector
discrete-gradient construction is conservative, reversible, oriented, and
angular-momentum-preserving conditional on the selected radial onsite energy.
Only invariant linearly polarized sectors reduce to the scalar `G*` clock;
generic vector motion does not. No outcome, context, state-effect weight, or
instrument appears in this clock mechanics. The radial coupling, local-body
support, spatial energy closure, and finite-tick gate remain physical recovery
debts.

FTD-0842 then couples the selected onsite energy to the positive production
spatial operator at reference level. Exact combined energy closure is
possible, but the simultaneous solve is globally dependent and the only
zero-stiffness spatial mode is box-wide constant. Thus it supplies neither a
P4-local one-tick gate nor a bounded exact critical-quartic clock. This
obstruction is clock-side only: the construction still has no outcome,
context, weight, or instrument input, and it changes no selector claim.

FTD-0844 gives a P4-local selected carrier by separating the common
propagating mode from a compact relative quartic mode. Its sector energies
close exactly and the relative phase remains context blind. The same exact
decoupling prevents operational readout, so this is not yet an actualization
gate. Any future common--relative readout must accept no context/outcome/weight
input and must book its energy current and phase backreaction.

FTD-0846 supplies the first such local reference transaction. Common/even
position readout sees only the symmetric-square phase; a positive bilinear
faithful readout would add quadratic clock stiffness. The selected
exchange-odd pointer with `kappa(r-q)^4/4` has one reversible onsite next
state and an exact three-account energy ledger. Its force/history retains the
signed phase without reading a context, outcome, weight, `G*`, or target
period.

FTD-0848 closes the next reference step with the degree-minimum symmetric
sextic latch `beta*x^2*(x^2-A^2)^2`. It derives the barriers and acquisition
threshold, gives one compliant onsite damped AVF endpoint, and closes exported
bath energy plus coupling-switch work exactly. A sub-barrier basin persists
after decoupling, and the basin quotient supplies the ternary record. The
many-to-one quotient—not finite-time damping—is the explicit loss step. It is
still not an actualization gate: no production substrate transition realizes
the selected latch, the scalar bath account is not a microscopic environment,
and selector/Born coupling plus orientation/cadence tolerances remain open.

FTD-0850 closes the first production-equivalence discriminator. With
seed/site/tick retained, genesis is deterministic and context blind; its
nonzero divergence sign is odd, while evaporation is a genuine many-to-one
`+/-1 -> 0` loss map. But every finite-energy unlocked record has positive
evaporation hazard, so the current map has no strict invariant record basin.
The production energy audit also has no event-level receiver for the acquired
or erased field energy and metadata. Therefore present genesis/evaporation is
a partial ternary open-system witness, not the FTD-0848 latch. The next
physical target is either a native movement-enabled matter-field barrier plus
event reservoir, or an explicit adoption of the selected latch/reservoir
interface. Either route must remain blind to context, outcome, Born weight,
`G*`, and cadence targets.

FTD-0851 derives the minimum receiver behind that open line. An energy-only
account cannot distinguish erased `+1` from `-1`; a receiver valid also at
zero export minimally has `(chi,B)=(s,B)`. For positive export, one signed
amplitude `a=s*sqrt(2B)` stores both sign and energy. Its selected bilateral
form `L=s*sqrt(B), R=-s*sqrt(B)` has zero common mode, signed relative mode,
and energy `B`. Current collision, annihilation exhaust, and history-journal
paths do not implement it. Repeated reuse additionally requires the old odd
pulse to propagate away, retained history to grow, or the environment to be
declared open.

FTD-0852 supplies the exact reference propagation law. At each tick the new
amplitude `s*sqrt(2B)` enters rail depth zero and every older amplitude moves
one cell outward. This is local, causal, injective, recursively ready, and
obeys a sitewise energy-current equation. A finite rail remains complete only
if its signed tail—not merely tail energy—continues into another receiver.
Production provides a genuine candidate: identical L/R update operators and
equal sources make their difference a homogeneous local channel. But current
events do not deposit the pulse, the aggregate ledger ignores pure relative
energy, and the bidirectional stencil is not the exact one-way clearing law.

FTD-0853 supplies the missing reference write. For the six face neighbours,
opposite L/R impulses of magnitude `sqrt(B/6)` add no common wave velocity.
Their dual kinetic increment is `s*sqrt(B/6)*Q0+B`; hence the local, context-
blind gate `Q0=0` gives exact energy transfer, and the post-event coordinate
`Q1=s*sqrt(24B)` recovers sign and energy. This map is injective only on the
declared reduced domain. It does not encode the complete production state.
Event-energy provenance, dynamical ready-port formation, dual production
accounting, reciprocal record protection, and propagation compliance remain
`[OPEN]`.

FTD-0855 closes the two named reference debts behind that gate. The adopted
matter diagnostic loses `B=gamma E_REST>0` when evaporation changes a
manifested record to void. On the selected radial six-ray receiver,
`D=Q/sqrt(12)` and `D^2/2=Q^2/24`, so the cubic shell is exactly the causal
history rail. The synchronous write/shift `D_0'=s*sqrt(2B), D_{j+1}'=D_j`
moves the old port content outward while writing the new event and increases
receiver energy by exactly `B`. This does not derive `E_REST` or implement the
production transaction. A reserved directed relative channel, corresponding
production ledger term, reciprocal barrier, and complete erased-state lift
remain `[OPEN]`.

FTD-0856 closes the minimum reciprocal barrier interface. Deterministic strict
hold and exchange require a distinguishable eligibility state, while reciprocal
first-order rail dynamics must retain incoming/outgoing orientation. With
`A=sqrt(2B)`, the selected matrix `S_g=[[1-g,g],[g,1-g]]` is identity when
closed and swaps matter with the field when open. It is symmetric, orthogonal,
involutive, and conserves energy and signed content. Production's `locked` flag
and relative field/velocity type are only fragments. Physical activation,
protected characteristic separation, controller work, and full-state lift
remain `[OPEN]`. Clock compliance can be one factor in eligibility, but cannot
by itself make every stable record emit. The isolated
`ftd::eft::scatter_reciprocal_record_port` implementation passes its focused
Release CTest `1/1`; it has no `Voxel`, toggle, event hook, or tick-phase
consumer.

FTD-0858 separates acceptance from actuation. With fixed seed, tick, and local
inputs, production genesis/evaporation predicates are deterministic,
Moore-local, and target blind, but they read the common fields
`C=J_L+J_R`, `V=W_L+W_R`. Arbitrary antisymmetric changes leave those inputs
fixed while changing the relative receiver `(D,P)`, so acceptance cannot
determine incoming on-shell amplitude or ready-port vacancy. The exact local
chart `i=(p+g)/sqrt2`, `o=(p-g)/sqrt2` closes energy/current and retains
orientation, but production `c^2=1/3` has trace defect
`(8/3)sin^2(k/2)` from a one-cell rail. Clock compliance may schedule an event;
it cannot repair this kernel. The isolated implementation passes `1/1` without
a common-to-relative transducer, protected rail, controller ledger, `Voxel`, or
production consumer.

FTD-0860 supplies the first exact abstract actuation law on a pre-existing
nonzero receiver. With `I=(q^2+p^2)/2`, it selects
`z'=sqrt((I+B)/I)sJz`, so `I'=I+B` and the signed quarter-turns exchange under
time reversal. This law is symplectic on `I>0` but deliberately lossy: zero has
no positive rotation-equivariant phase, `F_+(z)=F_-(-z)`, and output action is
only the sum `I+B`. It is therefore not the faithful signed history rail and
must not be combined energetically with `a=s sqrt(2B)`. The remaining contract
is a declared loss ledger or reserved signed rail, a local phase anchor,
relative-energy/controller accounting, and bounded export/clearing. The
isolated CTest passes `1/1`; no production consumer exists.

FTD-0862 closes the faithful prepared-reference branch at selected scope. For
`phi_j^n=phi_0+kappa j-omega n`, the phase standard follows the outward
one-cell characteristic iff `kappa-omega in 2pi Z`. The FTD-0860 load then has
the exact downstream readout `B=I-I_*`, `s=sign(beta wedge Z)`. Its finite
rail obeys `Delta H_ex=B-E_tail`, is bounded by `N B_max`, and becomes
symplectic/injective when its incoming baseline and outgoing complete tail
environment are retained. The nonzero baseline and directed protected rail
are separate `[SELECTION]` currency. Their oscillator source, maintenance
work, cubic/protected propagation, native event hook, relative/tail ledger,
controller, and any `G*` cadence identification remain `[OPEN]`; production
C18 remains excluded by FTD-0858.

FTD-0863 refines that same selected phase-rail resource into two lanes. The
nonzero reference `beta` defines `e=beta/sqrt(2I_*)`, `f=Je` and retains action
`I_*`; a separate signal `D=af+be` may start at zero. The FTD-0856 controlled
gate swaps matter amplitude `m` with `a` while preserving `b` and `beta`, so
emission and absorption are one exact involution. The signal receives exactly
`B`, `sign(beta wedge D')` retains the event sign, and no second energetic sign
coordinate is counted. This consumes `SEL-CA-PHASE-RAIL` and adds no selected
type. Harmonic phase evolution and ring winding do not select `omega`; native
formation, robust maintenance/backreaction, protected cubic propagation,
eligibility/controller work, production coupling/accounting, and the `G*`
gearbox remain `[OPEN]`.

FTD-0865 supplies the autonomous Hamiltonian timing law for complete cycles.
The scalar gate cannot be Hamiltonian on one pair, so full matter and signal
modes are retained. The imposed harmonic law
`H=omega I+nu A+epsilon chi(1-cos theta)I_r` gives exact hold/swap for
`nu=omega`, `chi=omega/2`, with the visible reserve
`I_min=I_0-I_r`, `I(T)=I_0`. It consumes existing reference/rail types but adds
an imposed functional law, not a substrate derivation. A nonlinear convex
clock has `dXi/dI_r>0`; hence the quartic clock cannot be a universal fixed
load-blind swap controller in this minimal uncompensated class. The frozen
`epsilon` remains an input, not dynamic eligibility. Separation of the
isochronous orientation reference from the quartic calendar, or a compensating
controller/reservoir, remains `[OPEN]` with production realization.

FTD-0867 replaces the frozen abstract `epsilon` only at reduced reference
scope. The existing persistent ternary latch gives the unique even quadratic
command `epsilon=s^2`. Releasing the clutch at a zero of `1-cos(theta)` changes
the interaction energy by zero, while the outgoing signal retains
`B=|D'|^2/2` and `s=sign(beta wedge D')` across the requested local reset. The
active map is involutive, so a second cycle without acknowledgement/reset
undoes the export. Physical latch acquisition, autonomous acknowledgement and
reset, microscopic bath/reset-work closure, clock synchronization, native
mode preparation, and cubic production remain `[OPEN]`. No `G*` cadence or
Born/outcome target enters this clutch.

FTD-0869 removes the extra-acknowledgement ambiguity at the same reduced
reference scope. Midpoint completion—empty matter and nonzero local signal—is
the acknowledgement token and does not read a target magnitude. The compressed
exchange requires `I_0>B`. Locally Lipschitz autonomous flow cannot reach an
exact stable reset in finite time, so the registered closure explicitly
selects `gamma xdot in -kappa partial|x|`, with
`T_R=gamma A/kappa` and controller-to-scalar-bath transfer `kappa A`. An empty
output-port swap returns the latch and local modes ready while the output keeps
`(s,B)`. The waveform, cusp law, controller reservoir, and scalar bath are
imposed/selected reference content—not emergent substrate dynamics. Native
formation, microscopic thermal reset, robustness, quartic compensation,
protected cubic production, `G*` synchronization, and operational hiding stay
`[OPEN]`.

FTD-0871 separates that continuous realization from the actual ternary reset.
Writing the ternary alphabet as `Z_3`, the completed signal supplies
`u=d(E)=s`; controlled subtraction `s -> s-u` returns the latch to zero while
leaving `E` intact, and controlled addition is the exact inverse. Thus an extra
acknowledgement bit, reset-history trit, and logical bath are unnecessary. The
FTD-0869 cusp branch remains optional only if the selected continuous `x`
coordinate is retained physically. Endpoint-degenerate minima do not make the
controller free. The live requirements are native controlled-permutation work,
formation and robustness, empty-port clearing, protected cubic signed export,
production coupling, compensation, `G*` synchronization, and operational
hiding. A finite `N`-trit rail also cannot retain every longer ternary history
without signed tail export or other state.

FTD-0872 gives the minimum all-domain discrete transfer map explicitly. For
the ordered latch/output pair over `F_3`, `R(s,o)=(-o,s)` is the unique
isometry satisfying sign-preserving ready transfer and `det R=1`. It obeys
`R^2=-I`, sends `(s,0)` to `(0,s)`, and uses `R^-1` for absorption. Ordered
area retains the forward/reverse sign that `Sym^2` loses. A wrapper that
applies `R` only for an empty port and otherwise holds is noninjective, so
readiness must be guaranteed by the schedule or an occupied value must leave
through a reciprocal/reflected output. This closes the logical controlled
permutation, not its physical amplitude/action scale, actuation/work,
protected transport, native formation, production coupling, robustness, or
`G*` synchronization at the FTD-0872 boundary.

FTD-0873 supplies the exact continuous reference lift. Embed
`(p,q)=a(s,o)` and add the independent clock pair `(theta,I)`. The imposed
harmonic Hamiltonian with base winding `nu=Omega` and gate coupling
`kappa=Omega/4` yields identity, `R`, or `R^-1` after one cycle. The declared
record-energy scale is `Omega a^2/2`; maximum transient clock/interaction
exchange is `Omega A/2`; endpoint residual is zero; gate-zero switching costs
zero in the interaction account while off-phase switching generally does not.
Repeating the active branch gives `R^2=-I`, so a dynamic one-shot schedule is
still required. Native scale/formation, gate acquisition/release,
backpressure-safe handoff, protected transport, production, robustness, and
synchronization to the separate quartic `G*` calendar remain open.

FTD-0874 supplies the exact selected finite-horizon scheduler and rail. At
global tick `n`, the disjoint nearest-neighbour bonds whose left coordinate
has parity `n mod 2` apply `R(a,b)=(-b,a)`. For a prepared isolated record,

\[
 x_j^{(0)}=s\delta_{j0}
 \quad\Longrightarrow\quad
 x_j^{(t)}=s\delta_{jt}.
\]

The sign is unchanged, the speed is one cell per global tick, the trail
clears, and inverse layers recover the full state. A fixed matching cannot
propagate beyond one two-site block. Occupied bonds retain both labels through
reciprocal exchange, but a fully occupied rail remains fully occupied, so
backpressure retention is not a proof of progress. An injective
time-homogeneous map also cannot take a distinct predecessor into a literal
fixed `done` state; the reference one-shot mechanism is continuing outward
record motion. This refines the existing `SEL-CA-PHASE-RAIL` without adding a
type. Native intersite Hamiltonian formation, multidimensional routing,
finite-boundary completion, congestion resolution, production coupling,
robustness, and synchronization to the separate quartic `G*` calendar remain
open.

FTD-0875 supplies the exact local canonical Hamiltonian reference lift. The
undoubled scalar rail does possess a common symplectic form for every even
finite open length, but that form pairs each site with its boundary mirror and
therefore is not a length-independent local substrate structure. In the
registered onsite-direct-sum local class, one scalar per site cannot carry a
nondegenerate skew block; one canonical pair `(q_j,p_j)` per site is minimum
and sufficient.

For the active matching `M_n`, define

\[
 N=\frac12\sum_j(q_j^2+p_j^2),\qquad
 L_n=\sum_{(j,k)\in M_n}(q_jp_k-q_kp_j),
\]

and impose

\[
 H_{n,\sigma}=\Omega I+\Omega N+
 \sigma\frac{\Omega}{4}(1-\cos\theta)L_n.
\]

Freezing the matching during one complete harmonic cycle gives spatial angle
`sigma pi/2`, hence the exact FTD-0874 forward or inverse parity layer on both
canonical components. The inequality `|L_n|<=N` makes the carrier-plus-
interaction energy nonnegative. The prepared record carries
`epsilon_rec=Omega a^2/2` and transfers it through the local antisymmetric
current

\[
 J_{j\to k}=\Omega c(t)(q_jq_k+p_jp_k),\qquad
 c(t)=\sigma\frac{\Omega}{4}(1-\cos\theta).
\]

The endpoint residual is zero and a sufficient clock reserve is
`I_0>|L_n|/2`. On the actual section `p_j=0`, `L_n` vanishes and remains zero,
so clock backreaction vanishes on that special orbit even though the
Hamiltonian gradient transports the record. This does not prove free physical
hardware. It refines the existing `SEL-CA-PHASE-RAIL` without adding a type.
Native formation of the canonical doublet and its scale, multidimensional
routing, reciprocal finite boundaries, congestion resolution, production
coupling, robustness, synchronization to the separate quartic `G*` calendar,
and operational hiding remain open.

FTD-0876 identifies the native carrier coordinates. For `h>0`,

\[
 (J_{n-1},J_n)\longleftrightarrow
 (J_n,P_{n-1/2}),\qquad
 P_{n-1/2}=\frac{J_n-J_{n-1}}{h}
\]

is an exact bijection. Production already stores `(J,P)` as
`Voxel::flux`/`Voxel::wave_vel`, providing three onsite canonical pairs per
voxel. The symmetric-stiffness free kick/drift

\[
 P^+=P-hKJ,\qquad J^+=J+hP^+
\]

preserves the canonical form exactly. This closes coordinate availability,
not record preparation or the FTD-0875 bond interaction. The complete
production tick is not promoted: damping is conformally symplectic,
nonidentity Gauss projection is noninvertible on the unconstrained phase
space, Langevin consumes bath randomness, and genesis/loss/boundary maps are
outside the free-wave theorem. The actual algebra is correspondingly split
into the commutative configuration algebra `(J,s)` and the commutative phase-
complete Markov algebra `(J,P,s)`. Preparation/persistence of the ternary
record section, scale recovery, production bond actuation, constrained Gauss
dynamics, environment-complete loss, routing, robustness, quartic-`G*`
synchronization, and hiding remain open.

FTD-0880 reduces the selected matched Gauss sector canonically. Let `D` be
the oriented-face divergence, `L=DD^T`, and `Q=1^perp`. With

\[
 q=DJ,\qquad p=L^+DP,
\]

the exact decomposition is

\[
 J=J_T+D^TL^+q,\qquad P=P_T+D^Tp,
 \qquad \Omega=\Omega_T+dq^T\wedge dp,
\]

and `{q,p}=Pi_Q`. A neutral ternary configuration therefore has the unique
minimum-energy static record section

\[
 (J,P)=(D^TL^+gs,0).
\]

Matched curl recursion preserves its charge. This closes a representation
problem, not dynamic preparation: a fixed-range translation-invariant right
inverse of `D` is impossible across arbitrarily large probes, so the canonical
conjugate is relational. Affine preparation is noninjective unless the
discarded longitudinal discrepancy is retained; with that discrepancy the
input is reconstructed exactly, but at the FTD-0880 boundary a physical environment carrier and its
dynamics remain open. The live cell-centred Gauss path is not promoted: its
central divergence/gradient symbol differs exactly from the 18-point SOR
symbol, before finite iteration and manifested-site skipping. Production
actuation, amplitude/scale recovery, routing and finite boundaries,
finite-support uncontained completion, robustness, quartic-`G*`
synchronization, and hiding remain open. No selected type is added.

FTD-0882 supplies a conditional local preparation dynamics for that record.
For a cell residual `r_x=d_xJ-q_x` and signed environment amplitude `e_x`,

\[
 J'=J+\frac{d_x^T}{6}(e_x-r_x),\qquad e'_x=-r_x.
\]

Hence the normalized residual/port pair undergoes the oriented quarter-turn
`(r_x/sqrt(6),e_x/sqrt(6))->(e_x/sqrt(6),-r_x/sqrt(6))`. Same-color cells do
not share faces, so fresh-zero ports make each checkerboard layer an affine
orthogonal projection. Alternating even and odd layers from empty flux
converges to `D^TL^+q`; the inverse Laplacian labels the limit but is not read
by a local gate. Keeping every outgoing port and reversing the layers recovers
the complete finite input history exactly.

The local source work is `w_x=q_x(e_x-r_x)/6`. At the empty-field/fresh-port
limit,

\[
 E_{\rm field}=E_{\rm hist}=\frac12\lVert J_s\rVert^2,
 \qquad W_{\rm source}=\lVert J_s\rVert^2.
\]

This closes a reference field/history self-dual energy split, not autonomous
hardware. Fresh signed ports, their reversible recycling, a positive source
reservoir, local stopping, moving sources, boundaries, finite capacity,
production migration, physical scale, and synchronization to the separate
quartic-`G*` calendar remain open. A fixed size-independent finite layer count
is impossible by the FTD-0880 finite-range right-inverse theorem. No Born
target or measurement context is read, and no selected type is added.

FTD-0884 makes finite readiness and source work explicit. A cyclic bank of
`C` initially zero signed environment vectors supplies the first `C` fresh
layers. Each outgoing vector replaces its consumed bank coordinate and the
cursor advances modulo `C`. Decrementing the cursor and applying the FTD-0882
inverse restores the complete finite state. On a generic nonzero history, the
returning coordinate is occupied on layer `C+1`; a finite cyclic explicit bank
is therefore not an indefinite fresh environment. This is not a universal
finite-dimensional memory no-go: exact-real compression and growing/open
signed history remain separate branches.

For local work `w_x=q_x(e_x-r_x)/6`, impose a nonzero battery amplitude with
energy `b_x^2/2`. Strict reserve and sign preservation uniquely give

\[
 b'_x=\operatorname{sgn}(b_x)\sqrt{b_x^2-2w_x},
 \qquad b_x^2-2w_x>0.
\]

The inverse restores `b_x`; battery loss equals source work and the complete
field+bank+battery energy is constant. The law and reserve scale remain
imposed. A canonical Hamiltonian reservoir, native formation/recharge,
unbounded/open or justified compressed history, 3D routing/backpressure,
moving-source continuity, production migration, physical scale, and separate
quartic-`G*` synchronization remain open. No Born target is read and no
selected type is added.

FTD-0886 restores the missing canonical phase coordinates and refines that
battery interpretation. For one active cell, let

\[
y=\frac{d_xJ}{\sqrt6},\quad s=\frac{q_x}{\sqrt6},\quad
u=y-s,\quad a=\frac{e_x}{\sqrt6},
\]

with canonical pairs `(u,pi_u)` and `(a,pi_a)`. Define

\[
N=\frac12(u^2+a^2+\pi_u^2+\pi_a^2),\qquad
L=a\pi_u-u\pi_a.
\]

The exact identities `{N,L}=0` and `|L|<=N` make

\[
H=\omega I+\omega N
 +\sigma\frac{\omega}{4}(1-\cos\theta)L
\]

a positive imposed clocked Hamiltonian. One clock cycle produces the canonical
forward or reverse quarter-turn; for `sigma=+1`,
`(u,a,pi_u,pi_a)->(a,-u,pi_a,-pi_u)`. Clock action returns after the cycle,
and on `pi_u=pi_a=0` the gate is exactly the FTD-0882 residual/port update.
This is a positive source-centered local clocked layer, not yet an autonomous
Hamiltonian for the alternating parity schedule.

The source-work ledger now closes internally. With
`E_raw=(y^2+a^2)/2` and `U_int=-s*y+s^2/2`,

\[
E_{\rm raw}+U_{\rm int}=\frac12(u^2+a^2),\qquad
\Delta E_{\rm raw}=w_x,\qquad
\Delta U_{\rm int}=-w_x.
\]

Thus no separate post-hoc battery is required at the fixed-source local scope.
The FTD-0884 square-root amplitude remains an exact imposed ledger on the
zero-conjugate Lagrangian section, but it is not a phase-complete canonical
reservoir: its cotangent lift changes oscillator energy by
`-w(1+p_b^2/b^2)`, a state-dependent phase-blind action drain is not
symplectic, and a constant action translation is not globally Hamiltonian on
a periodic phase cylinder. A complete-pair open/bilateral history shift is
canonical and reversible as a kinematic reference, while its physical native
realization and any finite-boundary completion remain open.

Autonomous parity control, dynamical source formation/motion/recoil, a physical
open complete-pair history or justified compression, 3D routing, production
migration, physical scale, and synchronization to the separate quartic-`G*`
calendar remain open. No Born target is read and no selected type is added.

FTD-0888 removes the external integer-parity switch at reference level. One
phase circle is divided into six nonoverlapping `C^1` windows. Their generator
order is residual/history, history/reaction, and reaction phase for color 0,
then the same three pulses for color 1. With common positive norm `N`, target
angles `alpha_j`, and `kappa_j=6 alpha_j/pi`, the imposed Hamiltonian is

\[
H=\Omega I+6\Omega N
 +\Omega\sum_{j=0}^5\kappa_j\rho_j(\theta)G_j.
\]

Every window carries one `2*pi` base winding and exactly its declared pulse
angle. The different colors need not commute because the windows are ordered
and disjoint. The carrier satisfies `H-Omega I>=3 Omega N`, and clock action
returns at every window boundary. This is autonomous in extended phase space:
there is no external `n mod 2` input. Its phase origin, window profiles,
frequency, scale, and generator order remain imposed reference structure.

The same cycle supplies the minimum positive reaction refinement. For a ready
residual `u` and split angle `eta`, the local endpoint is

\[
u'=0,\qquad a'=-\cos\eta\,u,\qquad
r'=0,\qquad \pi_r'=\sin\eta\,u,
\]

with exact energy split

\[
E_{\rm hist}'=\cos^2\eta\,E_{\rm res},\qquad
E_{\rm react}'=\sin^2\eta\,E_{\rm res}.
\]

The FTD-0886 history-only endpoint already consumes the complete positive
residual energy, so it cannot also generate positive zero-initialized recoil.
One additional canonical pair is minimum in the registered local symplectic
class. Equal history/reaction energy uniquely selects `eta=pi/4` only after
imposing channel-exchange symmetry; this refines the existing phase-rail type
and adds no selected type.

The completed local ledger is

\[
\Delta(E_{\rm raw}+U_{\rm int}+E_{\rm react})=0.
\]

The reaction impulse is therefore paid by reduced outgoing-history energy.
Native formation of the phase controller, physical identification of the
reaction pair as spatial ternary-source momentum, source mass/inertia and
intercell motion, physical open history, routing, production migration, and
synchronization to the separate quartic-`G*` calendar remain open. No Born
target is read.

FTD-0890 distinguishes the scalar reaction clockwork from spatial source
transport. Cubic symmetry forbids a nonzero equivariant map from the scalar
reaction magnitude to a spatial vector. In the registered orientation-free
onsite class, one `T1u` vector is not symplectic and the minimum carrier is
three canonical pairs `(R,Pi) in T1u+T1u`. A fixed one-pair slice is sufficient
only when a separate local field/current vector supplies the direction.

Conditional on the selected relativistic source dispersion, the exact
cotangent chart is

\[
p=\frac{\sqrt{E_0+|\Pi|^2/4}}{c}\Pi,\qquad
x=Dg(\Pi)^{-T}R,
\]

with `K(p)=|Pi|^2/2` and `x dot dp=R dot dPi`. The corresponding free drift is
Hamiltonian, exactly reversible, energy preserving, and has speed below `c`;
the existing face-current segment closes endpoint continuity. When the matched
field ledger supplies `Delta p_matter=-Delta P_field`, local conservation fixes

\[
\sin^2\eta=K(\Delta p_{\rm matter})/E_{\rm res}.
\]

Thus equal splitting occurs only when the requested kinetic energy is half the
available residual energy. This physical context is local energy/momentum
data, not a measurement setting. `E0`, `c`, the dispersion, and the vector
reaction role remain selected/imposed; native vector common action, stable
source formation, inertial-scale derivation, production, and quartic-`G*`
synchronization remain open. No Born target is read and no selected type is
added.

FTD-0892 identifies the exact location of the spatial triplet once the
selected constituent common-action phase space exists. An orthogonal Helmert
reduction separates

\[
X=N^{-1}\sum_a x_a,\qquad P=\sum_a p_a
\]

from internal modes while preserving the full canonical one-form. The
collective `(X,P)` sector is therefore precisely three canonical pairs and
adds no clock or reaction type. Internal binding impulses cancel from `P`, and
the summed external constituent impulse is the exact collective kick.

For the selected relativistic constituent dispersions, the fixed-`P`
minimum has common velocity and exact energy

\[
E_{\rm coll}=\sqrt{(\sum_a\epsilon_a)^2+c^2|P|^2},
\qquad M_{\rm coll}=\sum_a\epsilon_a/c^2.
\]

The statement is conditional on `epsilon_a` and `c`. Static clock/source
stability and a positive static Hessian cannot set the kinetic curvature; a
binding or field offset counts toward inertial mass only if that dressing
participates in the moving family. Moreover, exact `Z^3` translation and
positive Peierls curvature do not supply a continuous total field-matter
Noether momentum. The clock still controls eligibility, while the selected
collective triplet can carry reaction momentum; their separation is retained.
Constituent formation, dressed-boost mass closure, exact total momentum,
stable source dynamics, production, and quartic-`G*` synchronization remain
open. No Born target is read.

FTD-0893 specifies what “dressed-boost mass closure” must mean. If `A` is the
positive energy Hessian on the complete time-odd matter--field tangent state
and `B` is an independently defined additive physical total-momentum map, the
fixed-momentum minimum has inertial tensor

```text
M = B A^-1 B^T.
```

A field coat may therefore contribute to inertia through its moving odd
sector, but a static energy offset cannot. The same `A` under `B -> sB` gives
`M -> s^2M`; clock cadence, energy bookkeeping, and co-motion do not set the
momentum normalization. The clock continues to decide event eligibility only.
It neither supplies `B` nor selects the reaction outcome. Exact total
field--matter momentum, agreement of energy-curvature/impulse/partition mass
routes, absolute mass, production, and quartic-`G*` synchronization remain
open. No Born target is read and no selected type is added.

FTD-0894--0896 then separate the cyclic translation label from a globally
real momentum ledger. Integer translations have characters
`chi_k(n)=exp(i k dot n)` labelled by `T^3=R^3/(2 pi Z^3)`; their labels add
exactly modulo reciprocal vectors. There is no continuous homomorphic section
`T^3 -> R^3`, and no finite-range periodic spectral weight equals the global
unwrapped coordinate. A real spectral lift therefore needs a nonlocal branch
or retained integer winding `w`, with `k_tilde=k+2 pi w`.

This is structurally analogous to the clock programme's separation of phase
from counted cycles: a principal phase/quasimomentum does not retain completed
turns. The analogy is not an identification. FTD-0896 derives neither a native
winding update nor the physical conversion `P=p_* k_tilde`, and it does not
connect that winding to the quartic `G*` calendar. A new local stress route
also remains open. Exact total field--matter momentum, absolute mass,
production, and quartic-`G*` synchronization remain open; no Born target is
read and no selected type is added.

FTD-0897 supplies the minimum exact update missing from that analogy. For a
supplied opposite increment, the two principal-zone carries uniquely update
an integer triplet so the lifted pair total is conserved. The update is fully
reversible when the inverse increment is regenerated and telescopes over
multiple events. It still does not identify a physical clock cycle with a
reciprocal carry.

The integer triplet is history/topological memory, not self-dual energy.
Periodic band energy is blind to it, and its conservation law does not choose
an energy function. A physical recursive gearbox therefore still needs one
local matter--field action that produces the increment and an exact
work/backreaction update, assigns or transports the carry, and fixes `p_*`.
An added conjugate phase/action cell is a reference candidate only; no quartic
`G*` synchronization, production type, or Born target is introduced.

FTD-0898 then composes that carry with the selected relative-quartic
discrete-gradient recursion. The recursion itself generates
`Delta P_L=-Delta P_R=Delta Pi/sqrt(2)`, exactly conserves
`H_D=Pi^2/(2m)+lambda D^4`, reconstructs each endpoint chart and winding, and
reverses under the signed step. The associated continuum quartic obeys
`T A=sqrt(pi) G* sqrt(m/(2 lambda))`, so internal impulse, positive energy,
reciprocal history, and the lemniscatic traversal factor coexist coherently in
one reference model.

The composition remains a boundary rather than a native clock. Its common
momentum is exactly invariant, so no impulse reaches an actual common/matter
mode. `p_*` is imposed, the carry has no derived energy, the channels have no
physical identification, and `G*` is absent from the finite discrete map. A
finite-step phase crossing has therefore not been synchronized to global tick
`n`. One common local action must break the decoupling while preserving full
energy, total momentum, locality, orientation, and reversible carry before
this reference gearbox can enter production.

FTD-0899--0901 supply the minimum imposed reference action that breaks that
mechanical decoupling:

\[
L=\frac M2|\dot C|^2+\frac m2|\dot D|^2
  +\gamma D\cdot\dot C-\lambda|D|^4.
\]

Its conserved canonical momentum `P=M Cdot+gamma D` and mechanical momentum
`K=P-gamma D` obey `Delta K=-gamma Delta D`. The registered exact
discrete-gradient endpoint conserves the positive full Hamiltonian, canonical
total momentum, and canonical total angular momentum; generates the
equal/opposite channel increment; composes with reciprocal carry conditional
on `p_*`; and reverses under the signed step. FTD-0899 and FTD-0900 are
preserved execution-invalid; the exact source-marker-only FTD-0901 repair
passes inherited `87/87`.

This closes an abstract gearbox, not the native clock. The complex structure
supplies orientation but does not derive `gamma`. A continuously active
nonzero connection gives the rest-sector clock Hessian `gamma^2/M`, so the
exact critical quartic survives only at `gamma=0`, which also switches off the
mechanical impulse. The live clock/gearbox interface is therefore a
context-blind local clutch or positive compensation law with all
switching/controller work recorded. It may not read `G*`, measurement
context, outcome, or Born weight; `gamma`, `p_*`, physical identification,
mass, production, and integer-tick cadence remain open.

FTD-0902--0903 prove that the detuning is controlled by connection order.
For a general positive cyclic connection, `B=DA(0)` contributes the rest
clock Hessian `B^T M^-1 B`, which vanishes exactly when `B=0`. The imposed
signed self-pair connection

\[
A(D)=\gamma|D|D
\]

has zero derivative at the origin and, at `P=0`, gives

\[
H=\frac{|\Pi|^2}{2m}
 +\left(\lambda+\frac{\gamma^2}{2M}\right)|D|^4.
\]

Thus the rest-sector critical quartic and continuum `G*` period factor survive
exactly while mechanical common impulse obeys
`Delta K=-gamma Delta(|D|D)`. This is not a complete clock/transport closure.
The self-pair is `C1` but not `C2` at the origin, a moving `P` generates a
quadratic ray term, and a symmetric polarized rest cycle has zero net common
drift. The next clock/gearbox interface is therefore either a context-blind
rectifier with controller state and switching work included in the reversible
ledger or a separate critical clock coupled to a distinct gearbox. The first
FTD-0902 execution is preserved invalid at `80/81`; the exactly one-
substitution FTD-0903 repair passes inherited `81/81`, focused CTest `1/1`,
and the isolated actualization/EFT chain `28/28`.

FTD-0904 then supplies the first exact imposed rest-sector rectifier without
an externally timed clutch. For `D=q e`, retain a local polar axis `e` and a
time-odd clockwise/counterclockwise branch `chi`, and use
`A=chi gamma q^2 e`. The rest clock remains exactly quartic, while the cycle
displacement is proportional to `1/G*` and mean speed per squared amplitude
is proportional to `1/(G*)^2`. The locked exact certificate passes `74/74`;
focused CTest `1/1` and the actualization/EFT chain `29/29` pass.

This is not yet clock hardware closure. A nonzero even polar function of the
clock vector alone cannot be inversion-equivariant, so the reference must
retain the polar and temporal signs `(e,chi)` that a symmetric square loses.
Their native formation, maintenance, and erasure, including work and
information cost, are now the live clock/gearbox gate. The analyzer remains
context-, outcome-, Born-weight-, and target-period-blind.

FTD-0905--0907 now establish the minimum native-type representation. A
neutral ternary dipole `d=sum s_x(x-r)` is origin independent and supplies a
polar axis `e=d/|d|`. At its `+/-` endpoints, the projected native
flux/wave-velocity modes define
`ell=q_+p_- - q_-p_+`; `chi=sign(ell)` is spatially scalar and time odd.
The symmetric dipole square and bilateral Gram data erase their respective
signs. The first FTD-0905 execution and FTD-0906 repair remain invalid at
`74/75`; the exactly one-marker FTD-0907 repair passes inherited `75/75`.
Focused CTest `1/1` and the actualization/EFT chain `30/30` pass.

This closes representability only. Under the imposed central quartic memory
law the wedge is conserved and bounded, but every nonzero wedge introduces a
centrifugal inverse-square term. The same mode therefore cannot be the pure
radial critical G* clock. The minimum registered clock architecture is now a
separate critical radial clock plus a bilateral chirality-memory mode.
Production formation, persistence, maintenance/erasure work, coupling, and
finite-tick synchronization remain open and target blind.

## 6. Acceptance and non-claims

The implemented reference must pass normalization, exact quantile partition,
Bell no-signalling, Tsirelson/PR controls, context dependence, clock-period,
feedback-convergence, context-blindness, and reference-accounting tests. The
separate maintained-clock implementation must pass the dimensional
mechanical-energy audit.

Passing those tests does **not** establish substrate Born frequencies,
laboratory Bell recovery, relativistic covariance, a Type-III local algebra,
or a native `G*` clock. Those remain explicitly registered open debts.
