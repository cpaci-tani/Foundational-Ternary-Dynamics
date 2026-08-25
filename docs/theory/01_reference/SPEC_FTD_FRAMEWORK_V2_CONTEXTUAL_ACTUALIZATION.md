# SPEC — FTD Framework v2: Contextual Actualization

> **Branch status (FTD-1023, 2026-08-24):** retained as the ratified v2
> contextual reference branch. The active strict-discrete constitution is
> [`SPEC_FTD_FRAMEWORK_V3_STRICT_DISCRETE_COMMON_ACTION.md`](SPEC_FTD_FRAMEWORK_V3_STRICT_DISCRETE_COMMON_ACTION.md).
> V3 does not inherit the potentiality net as bedrock.

**Status:** `[RATIFIED SUCCESSOR CONSTITUTION — FORMAL REFERENCE GATES PASS]`  
**Programme row:** `FTD-0825`  
**Date:** 2026-08-09  
**Predecessor:** [`SPEC_FTD_FRAMEWORK_V1.md`](SPEC_FTD_FRAMEWORK_V1.md),
which remains the constitution of the v1 branch and is not rewritten by this
document.  
**Register:** [`contextual_actualization_register_v2.json`](contextual_actualization_register_v2.json)  
**Ratification record:** on 2026-08-09 the branch validator, discrete-local-net
proof, Bell/Born/clock reference proofs, focused C++ tests, and documentation
checks passed in one repository state. Ratification moves no v1 tag and does
not close any physical recovery debt.

---

## 0. Constitutional purpose

FTD v1 declines the noncommutative measurement map `M` through FC-1. This
successor branch takes the other branch of the independence theorem openly:
it **adopts** a noncommutative potentiality net and a contextual
actualization law while preserving the commutative substrate as the algebra
of actual beables. Adoption is not emergence and is not derivation.

The v2 objective is narrower than “derive quantum mechanics.” It is to set a
small, explicit type system in which:

1. actual substrate configurations remain classical and Moore-local;
2. potential states, effects, and instruments are meaningful;
3. exactly one context-dependent record is actualized deterministically;
4. operational probabilities are normalized and no-signalling;
5. the physical Born pushforward and hiding of the preferred tick remain
   named recovery debts rather than being assumed solved; and
6. `G*` is tested as a local critical-clock constant without being called
   global time.

No Standard Model, QFT, gravity, Born-frequency, or Lorentz-recovery claim is
promoted by this constitution.

## 1. Inherited kernel

### 1.1 P1–P5

V2 inherits P1–P5 from v1 unchanged. In particular, P2 supplies an ontically
real integer update order `n`, P4 bounds **substrate update dependencies** to
the previous Moore neighbourhood, and P5 makes the substrate update a
function.

### 1.2 The actual algebra

For a finite specified region `Lambda`, first let

```
Omega_Lambda^conf = (R^3_J x {-1,0,+1})^Lambda.
```

This is the actual **configuration** space. Because the inherited flux wave
equation is second order, a closed first-order tick requires the equivalent
phase-complete Markov space

```
Omega_Lambda^phase
  = ((R^3_J x R^3_P) x {-1,0,+1})^Lambda,
P_(n-1/2) = (J_n-J_(n-1))/h.
```

FTD-0876 proves that `(J_(n-1),J_n)` and `(J_n,P_(n-1/2))` are in exact
bijection for `h>0`; production stores `P` as `Voxel::wave_vel`. Thus `P` is
the retained local temporal difference needed to Markovize the trajectory,
not a noncommutative potentiality coordinate or an independently selected
oscillator type.

The actual-beable algebra `A_act(Lambda)` is the commutative algebra of bounded
complex-valued functions on `Omega_Lambda^phase`. The configuration algebra
on `Omega_Lambda^conf` is its position/state subalgebra. The state-only record
subalgebra remains

```
D_Lambda ~= C^(3^|Lambda|).
```

This is the v2 continuation of the A5 commutativity theorem. The three values
of a classical ternary variable do **not** by themselves make the actual
algebra `M_3(C)`.

FTD-0880 gives a derived coordinate refinement on the already selected
matched oriented-face probe. With incidence divergence `D` and `L=DD^T`,
`q=DJ` and `p=L^+DP` are a canonical charge/conjugate pair on the mean-zero
charge space, while the remainder is transverse. This does not add a beable
or change commutativity. The conjugate uses an inverse Laplacian and is not a
uniformly finite-range onsite coordinate; the local ternary record is the
divergence coordinate, not its global conjugate.

## 2. V2 framework commitments

### FC-CA1 — two-algebra commitment `[AXIOM-class adoption]`

For every finite specified region `Lambda`, v2 adopts a complex unital
potentiality algebra `A_pot(Lambda)` in addition to the commutative actual
algebra `A_act(Lambda)`. The assignment is an isotonic local net. Disjoint
local algebras commute in the selected reference representation.

The first witness representation is

```
A_pot^ref(Lambda) = tensor_(x in Lambda) M_3(C).
```

The qutrit representation is `[SELECTION]`. It is not derived from ternarity.
The norm-inductive quasi-local algebra is a UHF scaffold. No Type-III factor
claim follows without a separately declared state, GNS representation, and
factor-classification proof.

### FC-CA2 — preparation map `[AXIOM-class adoption + OPEN recovery debt]`

V2 adopts a compatible family of maps from substrate preparation classes to
positive normalized states `omega_Lambda` on the potentiality net. For
`Lambda subset Gamma`, restriction of `omega_Gamma` to `A_pot(Lambda)` must
equal `omega_Lambda`.

This naturality condition is part of the adopted interface. A substrate
derivation of the map is `OPEN-CA-PREP`; matching a desired quantum state is
not such a derivation.

### FC-CA3 — causal instruments `[AXIOM-class adoption]`

A measurement context is a finite family of localized effects and completely
positive instruments. Instruments localized in causally disjoint regions
must compose in either order and give identical operational probabilities.
This is an imported local-measurement type, not content forced by P1–P5.

### FC-CA4 — deterministic contextual actualization `[AXIOM-class adoption]`

For a complete context `C`, hidden state `lambda`, and global tick `n`, the
selector

```
Sigma(C, lambda, n) -> one joint outcome
```

is deterministic. It is intentionally context-complete and need not factor
into independent response functions at separated wings. This is the precise
ontic price paid to escape the FC-1/Fine joint-distribution ceiling.

The selector must satisfy the operational pushforward contract

```
(Sigma_C)_* mu_eq(o) = omega(E_o^C).
```

The deterministic quantile construction is only an `[IMPOSED reference
realization]`; because it consumes `omega(E)`, it is a compatibility witness,
not a Born derivation.

### FC-CA5 — measurement independence `[AXIOM-class declaration]`

For every admissible preparation ensemble and measurement context,

```
mu_eq(lambda | C) = mu_eq(lambda).
```

This is a statistical condition on declared ensembles, not a metaphysical
claim about free will. It supersedes the measurement-dependence posture of
FTD-0329 row 15 **on the v2 branch only**. The v1 record remains historical.

### FC-CA6 — split locality `[AXIOM-class declaration]`

P4 remains strict for substrate propagation. Potential states and the
actualization selector may be globally nonseparable. Operational parameter
independence is mandatory: no controllable remote setting may change a local
marginal. Thus v2 accepts ontic nonlocality/contextuality and rejects
operational signalling.

### FC-CA7 — three-level time contract `[SELECTION + OPEN recovery debts]`

V2 distinguishes:

1. the global ordinal tick `n` `[AXIOM via P2]`;
2. local operational duration `tau_x` reconstructed from a maintained clock
   phase `[CONSTRUCTION]`; and
3. a local actualization-gate count `k_x` `[CONSTRUCTION]`.

For the selected critical quartic clock,

```
T A = sqrt(pi) G* sqrt(m/(2 lambda))
```

at the critical surface and in the small-amplitude scope established by
FTD-0821/0824. `G*` is the exact period factor inside that selected clock
model. The claim that nature maintains such a clock is `[OPEN]`.

FTD-0827 adds a conditional mathematical closure without changing that price.
For the selected critical quartic Hamiltonian, the normalized energy shell
`y^2=1-x^4` maps exactly to the conductor-32 CM curve `v^2=u^3-u` by
`(u,v)=(x^-2,-yx^-3)`, with `du/(2v)=dx/y`. Thus the selected clock's period,
forward orientation, and Hecke/Frobenius calendar are one algebraic system.
Native localization/maintenance and any operational prime-indexed realization
remain `[OPEN]`; the selected-type count is unchanged.

FTD-0840 supplies the smallest exact recursive reference realization without
changing that price. Retaining the source-free canonical lift `(q,p)`, then
adopting the signed self-pair `u=q|q|` and positive pair energy `lambda u^2`,
gives `H=lambda(u^2+y^2)` in normalized pair coordinates. The registered
symmetric discrete-gradient update is globally unique, reversible, exactly
energy-conserving, strictly oriented off the origin, and bounded on every
positive-energy shell. Its continuum generator obeys the displayed `G*`
period law without reading `G*`. The pair coupling is absent from production,
the modal lift is not yet local hardware, and the finite-step map differs
from exact quartic flow; no exact global-tick gate cadence is thereby derived.
A bath is unnecessary for isolated conservative recurrence but remains
required for damping, recovery, or maintenance of a selected shell.
The post-certificate implementation lives only in
`engine/include/ftd/eft/native_pair_energy_recursion.h`; its focused CTest
passes, and it has no production tick consumer.

FTD-0841 closes one narrower type debt without changing the constitutional
price. Production voxels already carry the local canonical pair `(J,W)`, and
the rank-one self-pair tensor `U=J otimes J` obeys
`||U||_F^2=|J|^4`. Conditional on selecting the local radial coupling
`lambda||U||_F^2`, the vector discrete-gradient recursion is globally unique,
reversible, exactly energy- and angular-momentum-conserving, strictly
oriented, and bounded. Every fixed linearly polarized sector reduces exactly
to FTD-0840 and therefore has the continuum `G*` period factor. Cubic symmetry
alone does not force this radial quartic, generic angular-momentum sectors are
not pure `G*` clocks, and the production spatial field energy is not yet
closed together with the selected onsite term. Thus local state type is
available; coupling, body support, polarization, spatial energy closure,
maintenance, and integer-tick gating remain `[OPEN]`.

FTD-0842 proves that simply adding the selected onsite radial quartic to the
positive production spatial energy does not retire those debts. A symmetric
simultaneous discrete gradient is unique, reversible, and exactly conserves
the combined energy, but its exact inverse is dense on a connected quotient
and therefore is not a one-Moore-shell ontic update. Moreover, the positive
edge energy has only the spatially constant zero mode; every nonzero bounded
profile carries quadratic stiffness and is not an exact critical-quartic
clock. A local accounting architecture and a bounded zero/soft relative mode
are additional recovery types. They remain independent of the contextual
selector and Born pushforward.

FTD-0844 supplies a selected reference witness for both missing types. The
rank-one common/relative channel metric makes only `C=(L+R)/sqrt(2)` propagate
and leaves `D=(L-R)/sqrt(2)` as an onsite quartic soft mode. The common
production tick invariant plus relative quartic energies close exactly, all
dependencies remain P4-local, and one relative site stays compact. Production
does not select the required `b=a` cross-gradient, and exact decoupling makes
the clock unreadable. This construction remains outside the selector: it
contains no context, outcome, instrument, or state-effect weight. Formation,
readout/backreaction, common-action provenance, and finite-tick gating remain
`[OPEN]`.

FTD-0846 sharpens the readout type. A common/exchange-even position pointer
can retain only the symmetric-square phase, identifying opposite
half-cycles. Positivity plus exact zero clock Hessian excludes a nonzero
bilinear faithful pointer. Conditional on adding a selected exchange-odd
pointer, the degree-four interaction `kappa(r-q)^4/4` is the minimum in the
registered nonnegative polynomial position class and admits a unique,
reversible, onsite, exactly energy-closed discrete transaction. Its local
history retains signed phase, but its torque and quartic loading perturb the
clock. Production/common coupling and orientation/rate compliance remain
`[OPEN]`. The pointer mechanism is still independent of measurement context,
outcome, selector state, and Born weight.

FTD-0848 supplies a selected persistent ternary record law at reference
scope. Three nondegenerate symmetric polynomial wells require degree at least
six, and `beta*x^2*(x^2-A^2)^2` attains the floor. Its exact AVF transaction
books damping export and coupling-switch work; sub-barrier basins persist,
and their quotient to `{-1,0,+1}` is explicitly many-to-one. Thus the lossy
step is the record quotient, not damping by itself. The latch type, potential,
schedule, and basin convention remain selected. FTD-0850 then proves that the
current production map is not equivalent: genesis supplies a deterministic,
context-blind signed acquisition fragment and evaporation supplies genuine
many-to-one loss, but unlocked finite-energy records have no strict invariant
basin and the event has no retained bath/controller receiver. Native
movement-enabled barrier/reservoir recovery or an explicit open-system
adoption, microscopic bath information/thermal cost, selector/Born coupling,
and finite-tick `G*` cadence remain `[OPEN]`.

FTD-0851 fixes the minimum receiver currency for that open line. A
nonnegative energy account alone is even under sign reversal and cannot retain
which signed record was erased. The minimum receiver valid at zero export is
therefore an odd label plus energy account `(chi,B)`. For `B>0`, these compress
to one signed amplitude `a=s*sqrt(2B)`. The selected balanced realization
`(L,R)=(s*sqrt(B),-s*sqrt(B))` leaves the common mode zero, places orientation
in the relative mode, and carries energy `B`. This books a receiver type, not
its physical realization: production movement/exhaust/journaling does not
implement the transaction, and causal pulse propagation plus a reciprocal
barrier remain `[OPEN]`.

FTD-0852 closes causal propagation at selected reference scope. The update
`D_0'=s*sqrt(2B), D_{j+1}'=D_j` transports every prior event one local cell
outward, is injective on a half-line, and closes an exact energy-current
continuity equation. A truncated shift rail must export the signed tail
amplitude; exporting its squared energy alone loses orientation. This does not
exclude fixed-dimensional exact-real natural extensions. Production has a
homogeneous relative candidate because L/R receive identical local operators
and equal sources, but its events supply no odd deposit, its aggregate ledger
does not count pure relative energy, and its bidirectional stencil gives no
exact port-clearing/injectivity result. A reciprocal barrier and production
transaction remain `[OPEN]`.

FTD-0853 closes that transaction at selected ready-port reference scope. On
the six face neighbours, `W'_L,nu=W_L,nu+s*sqrt(B/6)*nu` and
`W'_R,nu=W_R,nu-s*sqrt(B/6)*nu` preserve the common field armwise. With the
selected dual kinetic energy their increment is `s*sqrt(B/6)*Q0+B`, so the
preregistered local gate `Q0=0` transfers exactly `B`. The post-event radial
relative coordinate is `Q1=s*sqrt(24B)`, which recovers the erased sign and
energy and makes the map injective on the declared reduced background domain.
Six faces are minimal only among equal-orbit one-tick deposits on the first
Moore shell with full cubic covariance. Production event-energy provenance,
ready-port formation, relative-field energy accounting, propagation
compliance, reciprocal barrier, and full-state natural extension remain
`[OPEN]`.

FTD-0855 identifies the event-energy and port-formation gearbox conditionally.
Within the imposed production diagnostic matter-energy role,
`B_diag=gamma*E_REST>0` is exactly removed when a manifested record becomes
void. On the selected six-ray radial receiver, `D=Q/sqrt(12)` and
`D^2/2=Q^2/24`; hence the cubic deposit coordinate is the causal history-rail
coordinate. The single local rule `D_0'=s*sqrt(2B_diag), D_{j+1}'=D_j` shifts
the old port value outward, writes the new event, and closes receiver energy.
This makes readiness recursive and target blind at reference scope. It neither
derives `E_REST` nor shows that production's shared bidirectional dual field,
aggregate drift ledger, record barrier, or complete erased state implements
the lift. Those remain `[OPEN]`.

FTD-0856 gives the minimum reciprocal boundary. A deterministic record cannot
both remain fixed and exchange on the identical complete input, so eligibility
must have at least hold/exchange values. An emitted history pulse and its time
reverse must also retain outgoing/incoming orientation in the selected
first-order rail class. For `A=sqrt(2B)`, the controlled boundary
`S_g=[[1-g,g],[g,1-g]]` is identity for a closed gate and swaps matter with the
incident characteristic for an open gate. It is a symmetric orthogonal
involution and therefore realizes energy-exact persistence, emission, and
absorption. This is a selected interface, not a derivation of physical
activation. Production lock/dual-field types are partial hardware; protected
characteristics, controller work, event coupling, and full-state lift remain
`[OPEN]`. The contract has an isolated `ftd::eft` C++ witness whose focused
Release CTest passes `1/1`; that witness supplies no physical eligibility law
and changes no production tick phase.

FTD-0858 identifies what production eligibility does and does not supply. The
fixed-input genesis/evaporation predicates are deterministic, Moore-local, and
target blind, but depend on common fields only. The common projection has an
arbitrary relative kernel, so those predicates cannot determine the incoming
relative amplitude, its event energy, or port readiness. A separate exact
characteristic chart preserves incoming/outgoing energy and current, while the
frozen C18 dispersion with `c^2=1/3` is not the selected one-cell rail. Hence a
clock-compliance bit cannot complete actualization by multiplication alone.
FTD-0860 closes the abstract nonzero-carrier transducer law but separates its
semantics. The selected map `z'=sqrt((I+B)/I)sJz` is target blind, symplectic,
and gives `I'=I+B` on `I>0`; it is a deliberately lossy mixer because opposite
sign/background phases collide and output action does not separate `I` from
`B`. Zero has no positive rotation-equivariant phase. Faithful history remains
the distinct signed rail `a=s sqrt(2B)`; assigning both deposits quadratic
energy would double count `B`. V2 must therefore declare which event labels
are intentionally lossy or adopt the reserved rail, then supply a local phase
anchor, relative-energy/loss/controller ledgers, and bounded clearing/export.
The isolated witnesses pass their focused CTests; production is unchanged.

FTD-0862 supplies the exact selected reference for the faithful prepared-rail
branch and books it as a **separate selected type**. Let
`phi_j^n=phi_0+kappa j-omega n` and `I_*>0`. The baseline phase is preserved by
the one-cell outward shift exactly when `kappa-omega in 2pi Z`. Loading that
baseline with the FTD-0860 quarter-turn makes the downstream readout
`B=I-I_*`, `s=sign(beta wedge Z)` exact. A finite rail satisfies
`Delta H_ex=B-E_tail` and retains at most `N B_max`; the full tail pair, not
scalar energy alone, preserves orientation. Including the baseline input and
tail environment makes the fixed-control shift symplectic/injective. This
closes abstract faithful readout only on a prepared subspace. The origin and
maintenance of the nonzero phase calendar, protected directed propagation,
cubic embedding, production relative/tail ledger, and controller cost remain
`OPEN-CA-TRANSDUCER`; frozen C18 is not promoted.

FTD-0863 gives a cleaner realization of that same selected type and **does not
add selection currency**. A separate nonzero pair `beta` carries the conserved
reference action `I_*`, defining `e=beta/sqrt(2I_*)` and `f=Je`; a signal pair
decomposes uniquely as `D=af+be`. The FTD-0856 controlled identity/swap acts on
`(m,a)` only. It is reciprocal and preserves
`I_*+(m^2+|D|^2)/2`. From `m=s sqrt(2B), D=0`, the open gate yields
`D'=s sqrt(B/I_*)J beta`, hence `|D'|^2/2=B` and
`sign(beta wedge D')=s`; applying the same gate absorbs the signal. The
reference supplies orientation but does not receive or duplicate the event
energy. Autonomous harmonic rotation preserves `I_*`; finite periodic
coherence adds `N kappa in 2pi Z` to the travelling condition, but neither
condition selects `omega` or derives a `G*` gearbox. Native reference
formation, perturbation recovery/backreaction, protected cubic transport,
production event coupling and accounting, and controller work remain
`OPEN-CA-TRANSDUCER`.

FTD-0865 supplies the first autonomous Hamiltonian lift of that transaction.
The scalar FTD-0856 swap is anti-symplectic on one pair, so the lift retains
full matter and signal modes. In common/relative coordinates it imposes
`H=omega I+nu(I_c+I_r)+epsilon chi(1-cos theta)I_r`. At
`nu=omega`, `chi=omega/2`, a complete harmonic reference cycle gives exact
identity for frozen `epsilon=0` and exact full-mode swap for `epsilon=1`.
Backreaction is explicit: `I_min=I_0-I_r`, `I(T)=I_0`; emission from an empty
signal has `I_r=B/2` and therefore requires `I_0>B/2`. This is an imposed law
on the existing phase-rail types, not a sixth selected type.

The same minimal law does not turn the quartic `G*` clock into a universal
load-blind controller. For a nonlinear clock `K(I)`, the pulse-area derivative
is `chi^2 integral g^2 K''/(K')^3 dtheta`; it is strictly positive for the
quartic action law `K proportional I^(4/3)`. Consequently a separate
isochronous phase reference, an additional compensating reservoir/controller,
or a declared fixed-load sector is required. Frozen `epsilon` is not dynamic
eligibility. Those alternatives and production realization remain
`OPEN-CA-TRANSDUCER`.

FTD-0867 supplies dynamic eligibility at the reduced selected-reference level
without adding a sixth selected type. For the existing persistent ternary latch
`s in {-1,0,+1}`, the unique even quadratic hold/exchange command is
`epsilon=s^2`. A switch to the hold branch at a zero of `1-cos(theta)` has zero
clutch-interaction work. On the registered signed preparation, the exact active
cycle exports the complete event mode into a signal satisfying
`B=|D'|^2/2` and `s=sign(beta wedge D')`; that declared record survives a local
latch reset request. The active map is involutive, so an unreset second cycle
undoes the export. Native latch formation, autonomous acknowledgement/reset,
microscopic bath and reset-work closure, clock synchronization, and production
coupling remain `OPEN-CA-TRANSDUCER`. No Born weight, measurement setting, or
`G*` cadence enters the clutch.

FTD-0869 closes the reduced reference acknowledgement/reset loop without a
new selected type. The completed local signal is itself the sign-even,
target-blind acknowledgement: at the midpoint it tests empty matter and
nonzero signal, not a target energy, Born weight, setting, remote context, or
`G*`. The imposed half-cycle waveform compresses the swap and strengthens the
transient reserve to `I_0>B`. A theorem-grade uniqueness argument excludes
exact finite-time reset by a locally Lipschitz autonomous attraction. The
selected cusp inclusion `gamma xdot in -kappa partial|x|` instead reaches zero
in `gamma A/kappa`; controller loss and scalar-bath gain both equal `kappa A`.
An initially empty output-port handoff makes the local reference recursively
ready while preserving `(s,B)`. This is a mathematical controller witness, not
a microscopic bath or protected transport mechanism. Native formation,
thermal/microscopic reset, perturbative robustness, quartic compensation,
cubic production coupling, native `G*` synchronization, and operational hiding
remain `OPEN-CA-TRANSDUCER`.

FTD-0871 then removes the logical need for FTD-0869's dissipative reset branch
at the actual ternary layer. Identify `{-1,0,+1}` with `Z_3` and let `d(E)` be
the oriented value decoded from the completed signal. The controlled map
`U_a(s,E)=(s minus a d(E),E)` has inverse
`U_a^-1(s',E)=(s' plus a d(E),E)`. On the registered completed-event state,
`a=1,d(E)=s`, so the latch becomes zero while the signal is unchanged. Bare
reset is noninjective and energy alone loses sign, but the existing signal
already carries the minimum three-valued retained record. No new
acknowledgement bit, reset-history trit, logical bath, or selected type is
required. The smooth-reset theorem remains binding only if the selected
continuous `x` realization is retained; the cusp law is one optional branch.
Physical controlled-permutation work, native formation, robustness, protected
cubic clearing/export, production coupling, quartic compensation, native `G*`
synchronization, and operational hiding remain `OPEN-CA-TRANSDUCER` at the
FTD-0871 boundary.

FTD-0872 closes the remaining **logical** controlled-permutation form. For the
ordered actual latch/output pair over `F_3`, the unique sign-preserving
orientation-preserving isometry is `R(s,o)=(-o,s)`. It obeys `R^2=-I`, maps a
ready event `(s,0)` to `(0,s)`, and uses `R^-1` for absorption. The ordered
area distinguishes the two directions although `Sym^2(R)=Sym^2(-R)`. An
empty-port/otherwise-identity wrapper is noninjective, so readiness is a
physical scheduling/reciprocal-output requirement rather than a hidden branch
of the gate. No new selected type is booked. Physical amplitude/action
scaling, actuation and work, protected transport, production coupling,
robustness, and `G*` cadence remain `OPEN-CA-TRANSDUCER` at the FTD-0872
boundary.

FTD-0873 closes the **reference Hamiltonian trajectory and work-ledger**
sub-debt. With `(p,q)=a(s,o)` and one independent clock pair, the imposed
harmonic law gives exact hold, `R`, and `R^-1` after one cycle, transiently
exchanges at most `Omega A/2`, and returns the clock action and total energy at
the endpoint. The scale `Omega a^2/2` is imposed, not derived. Repeating the
active cycle gives `R^2=-I`, so dynamic one-shot scheduling is not supplied.
No new selected type is booked. Native scale/formation, gate-zero control,
backpressure-safe handoff, protected transport, production coupling,
robustness, synchronization to the separate quartic `G*` calendar, and
operational hiding remain `OPEN-CA-TRANSDUCER`.

FTD-0874 closes the **selected finite-horizon reference scheduling** sub-debt.
At tick `n`, existing integer tick parity and cubic rail-coordinate parity
select disjoint nearest-neighbour bonds, each applying `R(a,b)=(-b,a)`. A
prepared isolated record then advances exactly one cell per tick with its sign
unchanged and is exactly recovered by inverse layers. Occupied bonds exchange
both labels without erasure, but a fully occupied control proves that
readiness and universal progress are not supplied. The injective fixed-state
argument also excludes a distinct reversible predecessor entering a literal
fixed `done` state; local one-shot clearing is instead realized by continuing
outward record motion. No new selected type is booked. Native intersite
Hamiltonian formation, multidimensional routing, congestion/backpressure
resolution, finite-boundary completion, production coupling, robustness,
synchronization to the separate quartic `G*` calendar, and operational hiding
remain `OPEN-CA-TRANSDUCER`.

FTD-0875 closes the **local canonical Hamiltonian reference-lift** sub-debt.
For an even finite scalar rail, a common symplectic form exists but pairs each
site with its boundary mirror, so it is nonlocal and length-dependent. In the
registered onsite-direct-sum local class, one canonical pair `(q_j,p_j)` per
site is minimum and sufficient. With
`N=sum_j(q_j^2+p_j^2)/2`, the matching generator `L_n`, and common clock
`(theta,I)`, the imposed law
`H=Omega I+Omega N+sigma Omega(1-cos theta)L_n/4` gives the exact forward or
inverse FTD-0874 layer after one complete cycle. It also gives a local
antisymmetric bond current and an exact clock/action ledger. The actual
section `p=0` is a special orbit with zero generator value and clock
backreaction but nonzero Hamiltonian transport; this is not a no-cost hardware
claim. No new selected type is booked. Native canonical-doublet formation and
scale, routing, finite boundaries, congestion, production coupling,
robustness, quartic-`G*` synchronization, and operational hiding remain
`OPEN-CA-TRANSDUCER`.

FTD-0876 closes the **native carrier-coordinate availability** sub-debt. The
exact history chart identifies `Voxel::flux` and staggered `Voxel::wave_vel`
as three onsite canonical pairs per voxel. For symmetric stiffness, the free
kick/drift preserves the canonical form exactly, has determinant one, and is
invertible. This supplies the FTD-0875 carrier *type* without adding a selected
oscillator or invoking Hilbert space. It does not supply the prepared record
section or the intersite bond actuator. Uniform damping is conformally
symplectic, nonidentity Gauss projection is not invertible on the unconstrained
phase space, Langevin consumes bath randomness, and genesis/loss/boundary maps
lie outside the free-wave theorem. Therefore the complete production tick is
not promoted to a symplectic map. At the FTD-0876 boundary, native record preparation/persistence,
amplitude and scale recovery, production insertion of the FTD-0875 generator,
routing, constrained Gauss dynamics, environment-complete loss, robustness,
quartic-`G*` synchronization, and operational hiding remain
`OPEN-CA-TRANSDUCER`.

FTD-0880 closes the **matched constrained-coordinate and static-record
representation** sub-debt without adding selection currency. On the selected
face-incidence complex,
`J=J_T+D^TL^+q`, `P=P_T+D^Tp`, and
`Omega=Omega_T+dq wedge dp`; a neutral ternary configuration has the exact
minimum-energy static section `(J,P)=(D^TL^+g s,0)`. Matched curl recursion
preserves the fixed charge because `DC=0`.

The same theorem sharpens three boundaries. First, no uniformly finite-range
translation-invariant right inverse of `D` exists across arbitrarily large
periodic probes, so the charge conjugate is relational rather than onsite.
Second, affine preparation erases a longitudinal discrepancy unless that
quantity is exported; retaining it makes the map exactly reversible but does
not supply an environment dynamics. Third, production central divergence/
gradient and the 18-point SOR operator have different Fourier symbols, so the
live finite-iteration, default-source-skipping pass is approximate constraint
relaxation, not the exact matched projector. At the FTD-0880 boundary, dynamic native formation and
persistence, a local reversible environment, production matched-complex/
bond-actuator coupling, scale, routing, robustness, quartic-`G*`
synchronization, and operational hiding remain `OPEN-CA-TRANSDUCER`.

FTD-0882 closes the **conditional dynamic reference-preparation** sub-debt.
On an even periodic matched-face probe, the local gate
`(r_x/sqrt(6),e_x/sqrt(6))->(e_x/sqrt(6),-r_x/sqrt(6))` rotates the six-face
Gauss residual into a signed environment port. With fresh zero ports, each
checkerboard half-layer is an affine orthogonal projection; alternating the
two colors from empty flux converges to `D^TL^+q` without evaluating `L^+` in
any local gate. Keeping the ordered outgoing ports makes every finite history
exactly reversible. At the empty-field/fresh-port boundary, the exact local
work ledger gives `E_field=E_hist=||J_s||^2/2` and
`W_source=||J_s||^2`. This is a reference field/history energy equality, not
a universal matter or cosmological self-duality claim.

No probe-independent finite number of local layers can complete every size;
otherwise it would realize the finite-range right inverse excluded by
FTD-0880. Autonomous port freshness/recycling, a positive source-reservoir
microdynamics, local stopping, moving-source continuity, nonperiodic and
uncontained boundaries, finite-capacity backpressure, production migration,
physical scale, separate quartic-`G*` synchronization, and operational hiding
remain `OPEN-CA-TRANSDUCER`. The gate reads neither Born weights nor outcomes,
and FTD-0882 adds no selected type.

FTD-0884 closes two additional **finite-horizon reference** sub-debts. First,
an explicit cyclic bank of `C` initially zero signed-port vectors supplies
exactly the first `C` fresh checkerboard layers. Retaining each complete signed
output makes the field/bank/cursor state exactly reversible. A generic nonzero
port returns on layer `C+1`, so indefinite freshness closes negative only in
this registered explicit-bank class. No universal finite-dimensional memory
no-go is claimed; growing/open history and exact-real compression remain
distinct possibilities.

Second, once positive quadratic battery energy `E_b=b_x^2/2`, a nonzero sign
branch, and strict reserve are imposed, exact local work conservation uniquely
gives

`b'_x=sgn(b_x)sqrt(b_x^2-2w_x)`, with
`w_x=q_x(e_x-r_x)/6`.

The inverse restores the signed amplitude, battery loss equals source work,
and field+bank+battery energy is exact. The quadratic battery law, reserve
scale, and cursor bank are imposed reference structure on existing carrier
types; no canonical Hamiltonian or natural scale is derived. A canonical
reservoir and native formation/recharge, unbounded/open or justified compressed
signed history, 3D routing/backpressure, moving-source continuity, production
migration, physical scale, separate quartic-`G*` synchronization, and
operational hiding remain `OPEN-CA-TRANSDUCER`. FTD-0884 reads no Born target
and adds no selected type.

FTD-0886 refines that result at full canonical phase-space resolution. For
one active fixed-source cell, set

\[
y=\frac{d_xJ}{\sqrt6},\qquad s=\frac{q_x}{\sqrt6},\qquad
u=y-s,\qquad a=\frac{e_x}{\sqrt6},
\]

restore conjugates `{u,pi_u}=1` and `{a,pi_a}=1`, and define

\[
N=\frac12(u^2+a^2+\pi_u^2+\pi_a^2),\qquad
L=a\pi_u-u\pi_a.
\]

Then `{N,L}=0` and `|L|<=N`. The imposed positive clocked Hamiltonian

\[
H=\omega I+\omega N+\sigma\frac{\omega}{4}
  (1-\cos\theta)L
\]

generates in one clock cycle the exact canonical quarter-turn
`(u,a,pi_u,pi_a)->(a,-u,pi_a,-pi_u)` for `sigma=+1`; the reverse orientation
uses `sigma=-1`. The carrier contribution is at least `omega N/2`, clock
action returns after the cycle, and the zero-conjugate section reproduces the
FTD-0882 gate exactly.

The work ledger closes without an independent battery at this fixed-source
local scope. With

\[
E_{\rm raw}=\frac12(y^2+a^2),\qquad
U_{\rm int}=-sy+\frac12s^2,
\]

one has `E_raw+U_int=(u^2+a^2)/2`, `Delta E_raw=w_x`, and
`Delta U_int=-w_x`. Thus the previously identified source work is exactly an
interaction-energy exchange. This does not derive the formation, motion, or
recoil of the source represented by the fixed offset `s`.

The FTD-0884 square-root law remains an exact one-amplitude ledger and is
retained as an imposed Lagrangian-section reference. It is not a phase-complete
canonical reservoir: its zero-section-preserving cotangent lift changes
`(b^2+p_b^2)/2` by `-w(1+p_b^2/b^2)`, and a phase-blind state-dependent action
drain adds a nonzero `-dw wedge dphi` term. Even a constant action translation
is locally symplectic but not globally Hamiltonian on a periodic phase
cylinder. A complete-pair open or bilateral history shift is symplectic and
reversible, but its physical native realization and finite-boundary policy
remain open.

At the FTD-0886 boundary, the result closes only a positive source-centered
**local clocked layer**; autonomous parity, source reaction, and the other
physical debts remained open. FTD-0888 closes the first two only at reference
level.

Let `(Q,P)` be the full source-centered matched-face canonical pair, give each
cell a history pair `(a_x,pi_{a,x})` and reaction pair `(r_x,pi_{r,x})`, and let
`N` be their common positive quadratic norm. For checkerboard color `m`, define
the residual/history, history/reaction, and reaction-phase generators
`L_ua^(m)`, `L_ar^(m)`, and `N_r^(m)`. They obey

\[
|L_{ua}^{(m)}|\le N,\qquad |L_{ar}^{(m)}|\le N,\qquad
0\le N_r^{(m)}\le N.
\]

Partition one phase circle into six disjoint `C^1` windows `rho_j`, ordered as

\[
(L_{ua}^{(0)},L_{ar}^{(0)},N_r^{(0)},
 L_{ua}^{(1)},L_{ar}^{(1)},N_r^{(1)}),
\]

with target angles `(pi/2,eta,pi/2,pi/2,eta,pi/2)`. The imposed autonomous
Hamiltonian

\[
H=\Omega I+6\Omega N
 +\Omega\sum_{j=0}^5\kappa_j\rho_j(\theta)G_j,
\qquad \kappa_j=6\alpha_j/\pi,
\]

contains no external tick argument. Each base-norm flow makes one identity
winding, each active pulse reaches its exact target angle, the two colors are
composed in the declared order without assuming they commute, and

\[
H-\Omega I\ge3\Omega N\ge0.
\]

Clock action returns at all six boundaries. Thus an external integer-parity
switch is not mathematically required at reference level. The phase origin,
window law, frequency, scale, and generator order remain imposed rather than
substrate-derived.

For one active ready cell, the exact reaction endpoint is

\[
u'=0,\qquad a'=-\cos\eta\,u,\qquad r'=0,
\qquad \pi_r'=\sin\eta\,u,
\]

so

\[
E_{\rm hist}'=\cos^2\eta\,E_{\rm res},\qquad
E_{\rm react}'=\sin^2\eta\,E_{\rm res}.
\]

The history-only FTD-0886 endpoint is positive-energy saturated and cannot
also give a zero-initialized positive source mode nonzero recoil. One
additional canonical pair is minimum and sufficient in the registered local
symplectic class. Equal splitting uniquely gives `eta=pi/4` only after
imposing output-channel exchange symmetry; that is a selection within the
existing `SEL-CA-PHASE-RAIL` type, not a new type or an ontological theorem.

With fixed equilibrium source offset `s_0`, the energy ledger becomes

\[
\Delta E_{\rm raw}=w-E_{\rm react},\qquad
\Delta U_{\rm int}=-w,qquad
\Delta(E_{\rm raw}+U_{\rm int}+E_{\rm react})=0.
\]

The reaction is therefore paid by reduced history energy. Physical
identification of the reaction pair with spatial ternary-source displacement
and momentum, native mass/inertia and intercell motion, physical open history,
3D routing, production migration, separate quartic-`G*` synchronization, and
operational hiding remain `OPEN-CA-TRANSDUCER`. FTD-0888 adds no selected type
and reads no Born target.

FTD-0890 resolves the next representation question. The scalar reaction pair
above is minimum for a scalar internal reaction, but it cannot select a
spatial recoil direction: the common fixed subspace of the cubic vector
representation `T1u` is zero. One vector copy is odd-dimensional and admits no
nondegenerate alternating form. Therefore the minimum orientation-free
registered spatial carrier is

\[
(R,\Pi)\in T_{1u}\oplus T_{1u},\qquad
\omega=\sum_i dR_i\wedge d\Pi_i,
\]

namely three instances of the existing canonical-pair type. One pair remains
sufficient conditionally when an independent local field/current vector fixes
a one-dimensional direction during the gate.

Given the already selected source dispersion

\[
K(p)=\sqrt{E_0^2+c^2|p|^2}-E_0,
\]

the map

\[
p=\frac{\sqrt{E_0+|\Pi|^2/4}}{c}\Pi,\qquad
x=Dg(\Pi)^{-T}R
\]

is an exact symplectic cotangent chart and satisfies
`K(p)=|Pi|^2/2`. Its free Hamiltonian drift is reversible, energy preserving,
and subluminal, while the inherited face-current segment supplies exact
continuity. A matched local field transaction supplies the required impulse
`Delta p_matter=-Delta P_field`, and conservation then fixes

\[
\sin^2\eta=K(\Delta p_{\rm matter})/E_{\rm res}.
\]

Equal splitting is therefore special, not universal. These are conditional
reference theorems: the dispersion, `E0`, `c`, vector-reaction role, and source
initial data remain selected/imposed. Native vector common-action formation,
field-to-triplet coupling, stable source formation, and derivation of the
inertial scale `m=E0/c^2` remain `OPEN-CA-TRANSDUCER`. FTD-0890 adds no
selected type and reads no Born target.

FTD-0892 closes the phrase “native vector common-action formation” only at
the selected constituent-phase-space level. If canonical constituent pairs
`(x_a,p_a)` are already admitted, their exact Helmert reduction gives

\[
\sum_a p_a\cdot dx_a
=P\cdot dX+\sum_{\mu>0}\pi_\mu\cdot dq_\mu,
\qquad X=N^{-1}\sum_a x_a,\quad P=\sum_a p_a.
\]

Thus `(X,P)` is exactly the three-pair cubic reaction carrier required by
FTD-0890, with no additional selected type. Internal pair impulses cancel in
`P`, while external constituent impulses sum exactly. This does not derive
the constituent phase space, its relational graph, or the field impulse from
the substrate.

Conditional on selected constituent energies
`sqrt(epsilon_a^2+c^2|p_a|^2)`, strict convexity gives the unique
minimum-energy allocation `p_a=(epsilon_a/sum epsilon)P` and

\[
E_{\rm coll}(P)=\sqrt{(\sum_a\epsilon_a)^2+c^2|P|^2},
\qquad M_{\rm coll}=\frac{\sum_a\epsilon_a}{c^2}.
\]

This is exact conditional additivity, not an absolute mass derivation. Static
stability and its Hessian do not identify the kinetic metric, and a static
binding offset that does not participate in the boosted family changes rest
energy without changing this curvature. Exact lattice translations are
`Z^3`, so `P` is an exact collective matter coordinate but not yet an exact
additive field-plus-matter continuous Noether charge. `OPEN-CA-TRANSDUCER` is
therefore narrowed to substrate formation of the selected constituent phase
space, dynamically dressed boost/mass closure, exact total physical momentum,
stable pole formation, production coupling, and hiding. FTD-0892 reads no
Born target and changes no branch currency.

FTD-0893 makes the dressed-boost mass target exact. Near a stable rest state,
write the complete time-odd matter--field tangent energy and independently
defined physical total momentum as

\[
H=E_0+\tfrac12 y^{\mathsf T}Ay+O(|y|^3),\qquad
\mathcal P=By+O(|y|^2).
\]

For positive `A` and rank-three `B`, fixed-`mathcal P` minimization gives

\[
y_*=A^{-1}B^{\mathsf T}(BA^{-1}B^{\mathsf T})^{-1}\mathcal P,
\qquad M=BA^{-1}B^{\mathsf T}.
\]

This is a conditional dressed-inertia theorem, not an absolute mass
derivation. Field dressing contributes only through its time-odd sector and
its participation in `A` or `B`; a static offset does not. The same `A` under
`B -> sB` gives `M -> s^2M`, so exact energy and a co-moving dressed picture do
not identify inertia without a physical momentum normalization. The selected
common action does not yet supply that exact total field--matter map: the
natural spline-Poynting candidate fails its coupled recoil ledger. The live
gate is an independently derived local stress/momentum state or additive
operational quasimomentum ledger for which constrained energy curvature,
impulse/center velocity, and matter--field momentum partition agree. FTD-0893
adds no selected type and changes no Born, Bell, `G*`, Lorentz, production, or
branch-currency status.

FTD-0894--0896 price the translation-spectral route to that missing `B`.
For the local uncontained translation algebra modeled conditionally by `Z^3`,
the unitary characters are labelled by

\[
\mathbb T^3=\mathbb R^3/(2\pi\mathbb Z^3),
\qquad \chi_k(n)=e^{i k\cdot n},
\]

and character multiplication adds quasimomentum exactly modulo reciprocal
lattice vectors. No continuous group-homomorphic section
`T^3 -> R^3` exists: its image would be a compact additive subgroup of
`R^3`, hence zero. Likewise any strictly finite-range translation-invariant
spectral charge has a periodic trigonometric-polynomial weight and cannot
equal the globally unwrapped coordinate. On `-pi<k<pi`, the exact unwrapped
coordinate is the infinite-range sawtooth series

\[
k=2\sum_{r\ge1}\frac{(-1)^{r+1}}{r}\sin(rk),
\]

with a branch discontinuity at the zone edge.

Thus an exact globally real spectral lift must be nonlocal or retain an
integer winding/history triplet `w`, with `k_tilde=k+2 pi w`. The winding
update and conversion `P=p_* k_tilde` remain open and are not a new v2
selection. This theorem does not rule out a new local stress/bond-impulse
state with its own exact exchange law. `OPEN-CA-TRANSDUCER` is narrowed to
deriving one of those dynamics plus its physical unit, then recovering the
same FTD-0893 tensor from energy curvature, impulse/velocity, and complete
matter--field partition. FTD-0896 changes no Born, Bell, `G*`, Lorentz,
production, or branch-currency status.

FTD-0897 closes the minimum conditional winding update. For principal pair
labels receiving a supplied opposite increment `+q,-q`, let `c_1,c_2` be the
two reciprocal-zone carries and update an integer triplet componentwise by

\[
W'=W+c_1+c_2.
\]

Then `k_1'+k_2'+2 pi W'=k_1+k_2+2 pi W` exactly. Given the selected branch
and conservation, this reservoir increment is unique; applying the inverse
increment returns the complete state, and event sequences telescope. This is
an exact recursive bookkeeping theorem, not a native interaction law. It does
not derive the increment, assign the carry to particle/bond/substrate/stress,
or fix `p_*`.

The energy boundary is equally important. A periodic band is blind to `W`,
the same carry algebra permits inequivalent reservoir-energy laws, and a
supplied opposite quasimomentum increment need not conserve band energy. The
next `OPEN-CA-TRANSDUCER` gate is one local matter--field action that derives
the increment, energy/work and backreaction, physical carry ownership or
transport, and the impulse unit. A conjugate phase/action realization is a
reference candidate only. FTD-0897 adds no selected type and changes no Born,
Bell, `G*`, Lorentz, production, or branch-currency status.

FTD-0898 closes the increment-origin debt **inside the already selected
relative-quartic reference recursion**. In the orthogonal common/relative
chart, its exact discrete-gradient step preserves

\[
H_D=\frac{\Pi^2}{2m}+\lambda D^4
\]

and generates the two channel impulses

\[
\Delta P_L=+\frac{\Delta\Pi}{\sqrt2},\qquad
\Delta P_R=-\frac{\Delta\Pi}{\sqrt2}.
\]

The induced dimensionless increment composes exactly with FTD-0897 across
zero, one, or multiple reciprocal zones and reverses under the signed inverse
step. The associated continuum quartic obeys

\[
T A=\sqrt\pi\,G^*\sqrt{\frac{m}{2\lambda}}.
\]

This is a reference gearbox theorem, not yet a physical common action. The
common momentum `P_L+P_R=sqrt(2)P_C` is exactly invariant, so the model cannot
transfer its relative clock/reaction impulse into the common matter/field
sector. The conversion `p_*`, carry energy/ownership, channel identification,
and integer-tick `G*` phase cadence remain open. The next
`OPEN-CA-TRANSDUCER` gate is a substrate-derived local coupling that breaks
this decoupling while conserving full energy and total momentum and retaining
the history required for reversal. FTD-0898 adds no selected type or currency
and changes no Born, Bell, Lorentz, or production status.

FTD-0899--0901 close that **reference common-action existence** question at a
strictly conditional level. FTD-0899 and its first FTD-0900 repair are
preserved execution-invalid; the source-marker-only FTD-0901 repair passes the
inherited exact certificate `87/87`. For the imposed velocity-linear law

\[
L=\frac M2|\dot C|^2+\frac m2|\dot D|^2
  +\gamma D\cdot\dot C-\lambda|D|^4,
\]

the canonical momentum `P=M Cdot+gamma D` is conserved while the mechanical
common momentum `K=P-gamma D` obeys `Delta K=-gamma Delta D`. The registered
discrete-gradient map exactly preserves the positive full Hamiltonian,
canonical total momentum, total canonical angular momentum, signed reversal,
and FTD-0897 carry composition conditional on supplied `p_*`. This is a
coherent imposed reference gearbox, not a substrate derivation or a physical
matter--field identification.

The same result creates a sharper clock boundary. The complex structure `i`
supplies orientation but does not determine the real magnitude `gamma`. At
rest the continuous connection adds clock Hessian `gamma^2/M`; exact critical
quarticity is recovered only at `gamma=0`, which also turns off the gearbox.
The next `OPEN-CA-TRANSDUCER` gate is therefore a context-blind phase clutch or
positive compensated action that preserves the critical clock while booking
all switching/controller work. It may not read `G*`, a context, an outcome, or
a Born weight. FTD-0901 adds no selected type or currency and changes no Born,
Bell, Lorentz, production, absolute-mass, or finite-tick-cadence status.

FTD-0902--0903 sharpen that clock boundary by connection order. The first
FTD-0902 run is preserved invalid at `80/81`; its one permitted sequential-
substitution repair passes inherited `81/81` as FTD-0903. For a general
positive cyclic connection with `B=DA(0)`, the rest-sector clock Hessian is

\[
B^T M^{-1}B,
\]

so every nonzero linearized connection detunes the critical clock. The
existing signed self-pair supplies a reference escape:

\[
A(D)=\gamma|D|D,
\qquad
H_{P=0}=\frac{|\Pi|^2}{2m}
 +\left(\lambda+\frac{\gamma^2}{2M}\right)|D|^4.
\]

This preserves the exact rest-sector critical quartic and continuum `G*`
factor while giving `Delta K=-gamma Delta(|D|D)`. The result is conditional
and imposed: `|D|D` is `C1` but not `C2` at the origin, generic moving sectors
contain a quadratic ray term, and a symmetric polarized rest cycle has zero
net common drift. The successor therefore books an oscillatory reference
gearbox, not net transport, mass, or production. A context-blind rectifier
with an enlarged energy/reversal ledger or a separate-clock architecture is
the next `OPEN-CA-TRANSDUCER` gate. FTD-0903 adds no selected type or currency
and changes no Born, Bell, Lorentz, production, absolute-mass, or finite-tick-
cadence status.

FTD-0904 closes the registered rest-sector rectification question
conditionally. For a polarized clock `D=q e`, retain a local unit polar axis
`e` and time-odd branch `chi`, and impose

\[
A(q,e,\chi)=\chi\gamma q^2e.
\]

This even connection preserves the same exact rest quartic but no longer
cancels over a full cycle. Exact beta identities give

\[
\Delta C_T=-\frac{4\sqrt\pi\,\chi\gamma}{M G^*}
 a\sqrt{\frac{m}{2\Lambda}}e,
\qquad
\frac{\overline{\dot C}\cdot e}{a^2}
=-\frac{4\chi\gamma}{M(G^*)^2}.
\]

Thus the reference action has an exact `G*`, inverse-`G*`, and inverse-
`G*`-squared gearbox. The accompanying no-go is equally important: a nonzero
polar function of `D` alone cannot be both even and inversion-equivariant.
The retained `(e,chi)` supplies the polar and clockwise/counterclockwise sign
information that `q^2` and a symmetric square lose. FTD-0904 does not derive
that memory from the substrate, fix `gamma`, close a generic moving clock, or
establish production/Born/cadence physics. The next `OPEN-CA-TRANSDUCER` gate
is native formation, retention, and erasure of `(e,chi)` with its full
energy/information ledger. No selected type or currency is added.

FTD-0905--0907 refine that boundary without adding a type. For every finite
neutral ternary region,

\[
d_\Lambda=\sum_{x\in\Lambda}s_x(x-r)
\]

is origin independent and transforms as a polar vector. A minimum distinct
`+/-` pair therefore supplies `e=d_Lambda/|d_Lambda|`. Projecting the existing
endpoint flux and wave-velocity fields gives `q_+`, `q_-`, `p_+`, `p_-`; the
antisymmetric wedge

\[
\ell=q_+p_- - q_-p_+,
\qquad \chi=\operatorname{sgn}(\ell)
\]

is a spatial scalar and time odd. The symmetric dipole square and bilateral
Gram matrix retain only the squared magnitudes and lose the two signs. Thus
existing native field types can represent the orientation memory required by
FTD-0904. This does not prove that production dynamics forms or retains it.

Under the imposed central reference law
`H=(p_+^2+p_-^2)/(2mu)+kappa(q_+^2+q_-^2)^2`, `ell` is conserved and every
nonzero wedge has a strict positive radial minimum. But the same nonzero
wedge adds a centrifugal inverse-square term. It therefore cannot remain the
pure radial quartic G* clock. The registered minimum is now a separate
critical clock mode plus a bilateral chirality-memory mode. Production
formation, maintenance, erasure, coupling, work, cadence, Born, and hiding
remain open.

FTD-0981--0990 now closes a separate portion of `OPEN-CA-TRANSDUCER`: the
local work-carrier, ownership, and static body-clock architecture at reference
scope. The existing dual flux/wave-velocity state contains the required
canonical pair. Exact C18 incidence channels give local work current and a
positive regional boundary clutch. The physical action of a nonzero regional
mode is frequency-normalized,

\[
H_u=\omega I_u,
\qquad I_u'=I_u+\frac{H-H'}{\omega}.                     \tag{CA-31}
\]

The actual ternary state then removes the contemplated static bond-memory
price. With

\[
m_x=s_x^2,
\qquad g_{xy}=1-(m_x-m_y)^2,
\qquad K_m=B^T\operatorname{diag}(g_b)B,                 \tag{CA-32}
\]

`K_m` is a positive Moore-local membrane cutting exactly the matter--void
bonds. Conditional on assigning that membrane to the common dual sector while
retaining full C18 propagation on the relative sector, L/R exchange symmetry
uniquely gives

\[
K_{LR}={1\over2}
\begin{pmatrix}K_m+K&K_m-K\\K_m-K&K_m+K\end{pmatrix}.    \tag{CA-33}
\]

The common sector is the locally owned recursive clock/work channel; the
relative sector remains the environmental interaction channel. The same
occupancy predicate supports the imposed matter-site Klein--Gordon term. On a
connected body its uniform common mode is uniquely lowest and satisfies
`H_u=omega_0 I_u` when `omega_0>0`.

Equations (CA-31)--(CA-33) are theorem-grade consequences of the registered
reference requirements, but (CA-33) is not in unchanged production and
`omega_0` remains imposed. The occupancy normal is time-even and does not
replace the retained time-odd C4 history. Active-aperture control, a physical
formation actuator and history-complete inverse, body/membrane motion, target-blind mode
preparation, collision/backpressure, complete-tick closure, robustness,
CPU/CUDA parity, `G*` cadence, and operational hiding remain
`OPEN-CA-TRANSDUCER`. No Born weight, context, setting, or outcome may enter
the coupling or preparation law.

FTD-0991/0992 closes the next conditional reference layer. For fixed field
coordinates, an occupancy update has the exact work

\[
W_{m\to m'}={1\over2}\sum_b(g_b'-g_b)a_bd_b^2.          \tag{CA-34}
\]

If a set `S` of occupancy bits is flipped simultaneously, only its cut-set
changes:

\[
W_S={1\over2}\sum_{b\in\partial S}(1-2g_b)a_bd_b^2.    \tag{CA-35}
\]

Consequently a cluster formed in uniform void releases exactly its common
boundary strain, while one-site growth costs `E_join-E_cut`. An already
prepared positive action of frequency `Omega` books this work by

\[
I'=I-{W\over\Omega},\qquad H'+\Omega I'=H+\Omega I.     \tag{CA-36}
\]

The minimum registered fail-closed active aperture uses the retained two-slot
orientation transfer and

\[
\gamma_b=g_b+(1-g_b)r_b^2,
\qquad(\sigma,0)\longleftrightarrow(0,\sigma).          \tag{CA-37}
\]

Equations (CA-34)--(CA-37) are conditional on (CA-33). They do not derive the
occupancy-controlled coupling, select the flip set or charge sign, prepare a
phase from zero action, or implement production genesis. The actuator,
positive reserve, target-blind phase-bearing input, and reciprocal moving
membrane remain `OPEN-CA-TRANSDUCER`. No Born target may enter their admission
or preparation rules.

FTD-0993/0994 corrects one part of that boundary. The zero-action origin is
singular only in action--angle coordinates. For local net work `U(x,Q)>0` and
retained time-odd `sigma`, the Cartesian generator

\[
\mathcal S_\sigma(x,Q)
=\sigma\int_0^Q\sqrt{2U(x,\xi)}\,d\xi                  \tag{CA-38}
\]

defines the exact symplectic momentum shear

\[
\pi'=\pi+\partial_x\mathcal S_\sigma,
\qquad P'=P+\partial_Q\mathcal S_\sigma.               \tag{CA-39}
\]

On `Q=P=0`, this gives

\[
P'=\sigma\sqrt{2U},\qquad
I'={U\over\Omega},\qquad
\theta'=-\sigma{\pi\over2}.                            \tag{CA-40}
\]

The opposite retained sign generates the exact inverse. Equations
(CA-38)--(CA-40) read no target phase, Born weight, context, setting, outcome,
or `G*` value.

They are local seed laws, not instantaneous body-wide preparation. The exact
uniform projector of an extended body is dense; a bounded seed requires at
least graph-radius time to reach every site. Direct copying into a blank
canonical pair is not symplectic. A new site joins an `N`-site uniform mode
without a relative mismatch only when

\[
q={Q_N\over\sqrt N},\qquad p={P_N\over\sqrt N},         \tag{CA-41}
\]

so it must arrive phase matched with the corresponding energy share. The
remaining `OPEN-CA-TRANSDUCER` debt was a nearest-neighbor conservative
growth/locking law deriving physical `U`, mismatch current, backreaction,
finite-speed propagation, and inverse.

FTD-0995/0996 closes that law on an exact compliance surface. At an occupied
donor's kinetic crossing, let `U_y=-W_y>0` be the released work of forming an
adjacent blank receiver and retain `sigma=sgn(p_x)`. In a kinetic chart with
mass `m`, the Cartesian seed is

\[
q_y'=0,\qquad p_y'=\sigma\sqrt{2mU_y}.                 \tag{CA-42}
\]

Therefore the receiver inherits the donor's exact state iff

\[
\boxed{C_{xy}=2mU_y-p_x^2=0.}                         \tag{CA-43}
\]

On this surface the membrane loses `U_y`, the receiver gains `U_y`, and the
opposite retained shear plus reverse occupancy flip is the same-crossing
inverse. The occupancy Laplacian annihilates the enlarged uniform state.
Moore-independent frontier events have additive work and exact coherence
remains within the local causal cone.

For the selected critical quartic clock, identical Cartesian state also
means identical amplitude, orientation, CM normalization, and period

\[
TA=\sqrt\pi G^*\sqrt{m\over2\lambda}.                  \tag{CA-44}
\]

No `G*` value or target phase enters (CA-42)--(CA-43). If
`r=U_y/E_x != 1`, then instead

\[
{T_y\over T_x}=r^{-1/4}.                               \tag{CA-45}
\]

This is exact conditional growth, not autonomous attraction. The remaining
`OPEN-CA-TRANSDUCER` debt is to derive why the common/relative membrane
Hamiltonian forces `C_xy=0`, or supply a positive local mismatch port,
backreaction, tolerance/robustness, controller scheduling, and production
inverse. Critical quarticity, its scales, and finite-tick `G*` hardware remain
selected/open. Production implements none of equations (CA-42)--(CA-45).

FTD-0997 supplies the missing phase-complete machine without adding another
continuous pair type. Let `C` be the donor common clock, `R` its local
relative port, and `Y` a blank common receiver. On the prepared crossing
subspace `C=R=z`, `Y=0`, perform a complete-pair swap and refill the emptied
relative pair from formation work:

\[
(C,R,Y)=(z,z,0)\longmapsto(z,z_U,z),
\qquad z_U=(0,\sigma\sqrt{2mU}).                       \tag{CA-46}
\]

The swap is symplectic, orthogonal, and involutive; the refill is (CA-42).
If the source loses `U`, then

\[
\Delta H_R=U-e,
\qquad
\Delta H_{\rm source+clock+port+receiver}=0.           \tag{CA-47}
\]

Inverse refill followed by inverse swap recovers the registered input for
every positive `U`. The port is recursively catalytic exactly when

\[
R'=R\quad\Longleftrightarrow\quad U=e
\quad\Longleftrightarrow\quad 2mU-p_C^2=0.             \tag{CA-48}
\]

This is constrained, source-paid copying; off compliance the relative port
retains the mismatch.

The unchanged common/relative Hamiltonian does not force the preparation or
compliance. It is block diagonal, and fixed-coordinate formation work has no
crossing-momentum dependence:

\[
F(q,p_C,m)=2mU(q,m)-p_C^2,
\qquad \partial_{p_C}F=-2p_C\ne0.                      \tag{CA-49}
\]

Thus compliance is a regular codimension-one admission surface. At a
quiescent matching seam with zero affected strain and no onsite load,

\[
W_y=U=0.                                               \tag{CA-50}
\]

The static membrane can support one swap from a preloaded port but cannot
refill it recursively. `OPEN-CA-TRANSDUCER` now means physical power and
control: prepare/protect/own the relative port, derive stored strain, latent
energy, relative/environmental inflow, or a positive local reserve, and close
capacity, backpressure, replenishment, robustness, scheduling, and production
inverse. Equations (CA-46)--(CA-50) add no v1 currency and are absent from
production.

FTD-0998/0999 closes the cumulative resource law without declaring a native
reservoir. For an accepted receiver batch `F_n`, define

\[
D_n=\sum_{y\in F_n}e_{y,n},\qquad e_{y,n}>0,             \tag{CA-51}
\]

and let `B_n>=0` be usable reserve already inside the batch's causal ownership
domain, `Phi_n` signed boundary inflow, and `U_n=U(F_n)` the exact joint local
formation release. Because every compliant catalyst is restored, its net
energy contribution is zero. Closed-completion conservation uniquely gives

\[
\boxed{B_{n+1}=B_n+\Phi_n+U_n-D_n}.                     \tag{CA-52}
\]

The whole shared-reserve batch must pass the atomic positive-domain gate

\[
\boxed{B_n+\Phi_n+U_n\ge D_n}.                          \tag{CA-53}
\]

before any receiver, source, reserve, port, or history state changes. For
overlapping supports, `U(F_n)` is one joint before-minus-after source energy.
Only a Moore-independent frontier has

\[
\boxed{U(F_n)=\sum_{y\in F_n}U_{y,n}.}                  \tag{CA-54}
\]

Summation telescopes to

\[
\boxed{
\sum_{n<T}D_n=B_0-B_T+\sum_{n<T}(\Phi_n+U_n).}          \tag{CA-55}
\]

For identical per-site clock energy `e`, this becomes

\[
\boxed{
eN_{\rm add}(T)=B_0-B_T+\sum_{n<T}(\Phi_n+U_n).}        \tag{CA-56}
\]

At the quiescent/no-inflow seam,

\[
\boxed{N_{\rm add}(T)\le\lfloor B_0/e\rfloor}.         \tag{CA-57}
\]

If long-time growth and causal-supply rates exist, the necessary power law is

\[
\boxed{\bar P\ge e v_g}.                                \tag{CA-58}
\]

Remote supply at graph distance `d` and update radius `r` is delayed by
`ceil(d/r)` ticks. Complete signed transaction history gives the exact reverse
reserve law

\[
\boxed{B_n=B_{n+1}-\Phi_n-U_n+D_n}.                     \tag{CA-59}
\]

Equations (CA-51)--(CA-59) are necessary accounting, positivity, locality,
and inverse conditions. They do not turn a scalar `B_n` into a canonical
work pair, derive the per-site energy, or generate inflow. `OPEN-CA-TRANSDUCER`
therefore sharpens to a native nonnegative reserve density and signed
Moore-local current with phase-complete ownership, charging, routing, atomic
debit, replenishment, backpressure, refill coupling, reverse transport, and
production realization. `G*` remains cadence only. No v1 currency or
production type is added.

## 3. Bell and Born at the correct strength

### 3.1 Bell

Bell/Fine is not suspended by discreteness. Under FC-CA5, a locally
factorized family of four setting-independent response variables would admit
a joint distribution and obey `|S| <= 2`. V2 instead makes `Sigma_C`
context-dependent and globally nonfactorizable.

For norm-bounded dichotomic observables in commuting wing algebras, the
selected C*-algebraic model proves the conditional operator bound
`|S| <= 2 sqrt(2)`. A selected singlet reference state saturates it. This is a
theorem **inside the adopted potentiality type**, not a substrate derivation
of laboratory Bell correlations.

### 3.2 Born

Once a positive normalized state and an effect are part of the adopted type,
`p(E)=omega(E)` is the state–effect pairing. Additivity on effects gives the
Born form conditionally on the standard effect-space assumptions. That result
does not determine why repeated substrate histories sample those weights.

`OPEN-CA-BORN` is closed only by a non-target-coded, preregistered physical
pushforward from substrate preparations. The free positive-frequency flux
density `|phi_+|^2` is the first candidate; interacting and genesis transfer
remain separate gates.

## 4. Falsifiers and stop conditions

The successor branch is rejected or demoted if any of the following holds:

- the preparation family cannot satisfy restriction consistency;
- separated instruments fail order independence;
- the selector enables operational signalling;
- a claimed Born recovery reads target probabilities or fitted quantum
  weights;
- the preferred tick produces an observable foliation effect outside the
  preregistered recovery envelope;
- `G*` contributes only a removable rescaling or the maintained critical
  clock requires outcome/context-dependent control; or
- a Type-III claim is inferred from finite spectral density or random-matrix
  spacing instead of a valid representation-level classification.

## 5. Ratification protocol

The v2 charter is owner-ratified, but this constitution becomes the ratified
successor only when all of the following pass in one repository state:

1. `proof_contextual_actualization_register_v2.py`;
2. `proof_von_neumann_type.py` in its corrected actual/potential scope;
3. `proof_contextual_actualization_v2.py`;
4. the focused C++ contextual-actualization and maintained-clock tests; and
5. documentation link/index and stale-path checks.

Passing this gate establishes formal consistency of the selected reference
architecture. It does not close `OPEN-CA-PREP`, `OPEN-CA-BORN`,
`OPEN-CA-LORENTZ`, or `OPEN-CA-GSTAR`.

**Gate result (2026-08-09): PASS.** The successor constitution is ratified at
reference-model scope only. Production integration remains forbidden until
the preregistered physical Born and operational-hiding gates pass.
