# Open Problem: Native Action or Measure for the FTD EFT

**Status:** [PARTIAL] bridge gate 2 from `SPEC_FTD_EFT_BRIDGE_CONTRACT.md`; linear source/flux generator derived; microscopic constrained-history measure selected in `DERIV_FTD_NATIVE_COMPLETE_HISTORY_ACTION.md`; explicit nonlinear blocked effective action and a non-tautological unified generative action remain open
**Purpose:** Define the source-coupled generator required before FTD-native source/flux dynamics can be called a Wilsonian EFT.

> **Unification correction (2026-08-23):** The exact kernel action
> \(S_H=-\log K\) is a complete formal encoding of an already specified tick.
> It does not derive manifestation, matter, clocks, electromagnetic response,
> gravity, and contextual measurement from one smaller transaction law.  It
> closes the history-measure choice for the EFT bridge but does **not** close
> the stronger ontological unification problem.
>
> The constitutional reason is audited explicitly in
> [`AUDIT_POSTULATE_ACTION_SUFFICIENCY_AND_OCCAM_BOUNDARY_v1.md`](../../07_assessment/framework_postulates_constitution/AUDIT_POSTULATE_ACTION_SUFFICIENCY_AND_OCCAM_BOUNDARY_v1.md):
> P1--P5, reversibility, and invariant counting each leave multiple exact
> dynamics. The next closure must supply one actual rule, not another
> adjective shared by many rules.

---

## Problem statement

The native bridge currently has:

```text
rho = s
J = J_L[rho] + J_T
C_L^FTD = 1
K_T^FTD = 1
Z_j^FTD = 1
g_sJ^FTD = 1
```

in bare engine units. This is a fixed response dictionary, but it is not yet a
full EFT. A Wilsonian EFT must specify what generates observables:

```text
correlators
response functions
operator mixing
blocking flow
renormalized couplings
```

Therefore the next bridge gate is:

```text
construct a native source-coupled action, transfer matrix, or deterministic
history measure whose observables reproduce the measured source/flux response.
```

No physical alpha value, Standard Model mass, or CODATA input is allowed in this
construction.

The native microscopic object is selected as a
constrained source-coupled history measure:

```text
S_H = -log mu_0(q_0) - sum_t log K_u(q_{t+1}, l_t | q_t)
```

with Gauss, continuity, locality, ledger, deterministic tick, Langevin, and
discrete channel terms carried by the transfer kernel. This resolves the
"action vs transfer matrix vs deterministic history measure" choice at the
microscopic level. The open problem is then narrower:

```text
derive/measure the blocked nonlinear effective action S_eff after B_b,
including the full operator mixing matrix.
```

For the stronger unified-action objective, an additional problem remains:

```text
find one phase-complete local transaction whose distinct moments/solutions are
manifestation, stable matter, internal clocks, electromagnetic response,
universal transport geometry, and contextual actualization.
```

Repackaging independently selected production phases inside `K_u` does not
meet this strengthened gate.

---

## Why the existing static action is not enough

`DERIV_PARTITION_FUNCTION_L2.md` showed that the current static analytical
action, after imposing:

```text
div J = s
```

collapses to an ultralocal state cost:

```text
S_E[J_min, s] = (c^2/2 + g_c) sum_x s_x^2.
```

That action counts manifested sites but does not distinguish charge separation.
It therefore does not generate the Coulomb-like Green response used by the
native response tuple.

The engine has Coulomb-like response through:

```text
dual-cell Gauss / Poisson response
field-energy diagnostics
emergent force extraction
```

The bridge must now decide which of these belongs inside the generator of the
EFT, and how.

---

## Candidate generators

### Option A: constrained flux-energy action

Define the native static generator by:

```text
S_native[J, rho] =
    (K_L/2) sum_x |J_L(x)|^2
  + (K_T/2) sum_x |grad J_T(x)|^2
  + constraint[div J_L - rho]
```

or, after integrating out constrained longitudinal flux:

```text
S_eff[rho, J_T] =
    (C_L/2) sum_{k != 0} |rho(k)|^2 / sigma_18(k)
  + (K_T/2) sum_k sigma_18(k) |J_T(k)|^2.
```

Advantages:

```text
matches the native response tuple directly
produces Coulomb-like source response
keeps QED alpha out of the definition
```

Cost:

```text
this is a new native generator, not the old static action
must be justified from engine energy ledger or dual-cell flux ontology
```

Status: [SELECTION] candidate.

### Option B: real-time transfer map

Treat the deterministic tick update as the fundamental object:

```text
U_tick: (s_t, J_t) -> (s_{t+1}, J_{t+1})
```

and define observables through long-time histories:

```text
<O> = history average over initialized ensembles and fixed toggles.
```

Advantages:

```text
closest to the engine
handles reaction-transport dynamics naturally
does not invent an Euclidean action
```

Cost:

```text
Wilsonian blocking of deterministic histories must be defined
stationary ensemble and source insertions remain open
reflection positivity / unitarity analog is not automatic
```

Status: [OPEN] candidate.

### Option C: Euclidean history measure

Define a path weight over histories:

```text
Z[eta, A_ext] =
  sum_{s(t)} int DJ(t)
    exp(-S_history[s,J] + eta rho + A_ext . j_T).
```

The action would include:

```text
Gauss constraint
flux kinetic/gradient terms
transport current cost
reaction source cost
native source/flux response
```

Advantages:

```text
closest to standard EFT and RG machinery
supports source insertions and loop expansion
```

Cost:

```text
largest new theoretical commitment
must avoid retrofitting coefficients to external targets
```

Status: [OPEN] candidate.

### Option D: constrained source-coupled history measure

Define the exact microscopic generator by the engine history kernel:

```text
Z_u[sources] =
  sum_H exp(-S_H[H; u] + source insertions)
```

where:

```text
S_H = -log mu_0(q_0) - sum_t log K_u(q_{t+1}, l_t | q_t).
```

The kernel is a hard constraint for deterministic phases and a noise/log
likelihood for Langevin or stochastic reaction channels. It reduces to the G18
linear generator in the no-reaction, low-amplitude sector and defines the
blocked Wilsonian action by:

```text
exp(-S_eff[H']) = sum_{H: B_b H = H'} exp(-S_H[H]).
```

Advantages:

```text
closest to the engine
keeps ledgers inside the generator
does not invent continuum coefficients before blocking data exists
contains the linear generator as a tangent sector
```

Cost:

```text
not yet a smooth continuum Lagrangian
requires channel probability catalogue
requires measured nonlinear operator mixing matrix
```

Status: [SELECTION] microscopic native action; nonlinear `S_eff` [OPEN].

---

## Required source terms

A valid generator must support external probes:

```text
eta(x,t) rho(x,t)           static source response
h_i(x,t) J_T,i(x,t)         transverse flux response
a_i(x,t) j_T,i(x,t)         current/radiation coupling
lambda_R(x,t) S_reaction    reaction-source ledger
```

These probes define the native correlators:

```text
<rho rho>       -> C_L^FTD
<J_T J_T>       -> K_T^FTD and dispersion
<j_T j_T>       -> Z_j^FTD
<j_T J_T>       -> g_sJ^FTD after projection
<S_reaction O>  -> reaction-sector corrections
```

No external QED observable appears in these definitions.

---

## Unified carrier gate (2026-08-23)

The exact
[`Moore-bond capacity census`](../derivations/gravity_cosmology/FOUND_MOORE_BOND_CAPACITY_TYPE_CENSUS_v1.md)
identifies a precise candidate and a precise type price.

For one inversion-even scalar capacity on each of the 13 antipodal Moore-bond
pairs,

```text
even bond module = 3 A1g + 2 Eg + 2 T2g
symmetric second moment = A1g + Eg + T2g
second-moment rank = 6
```

An oriented phase-complete bond state can therefore have both:

```text
odd first moment   -> vector/current response
even second moment -> trace plus shear transport capacity
phase winding      -> recurrence/history interference
```

This is only kinematic capacity.  At an isotropic vacuum, no differentiable
O_h-equivariant map from the existing scalar/vector variables has a linear
shear output.  Occupancy provides a static membrane; a J-derived tensor is
composite and remains under the spin-2 no-pole boundary.  The program must
therefore choose honestly between:

1. a nonlinear collective pole inside the existing L/R canonical fields; or
2. an explicitly adopted phase-complete dynamical link type. The unadopted
   minimal candidate is stated in
   [`SCOPE_PHASE_COMPLETE_BOND_TRANSACTION_ACTION_v1.md`](SCOPE_PHASE_COMPLETE_BOND_TRANSACTION_ACTION_v1.md).

No third branch is licensed by merely defining a tensor readout.

The unified carrier passes only if the **same** capacity enters both the body
clock and the wave principal symbol.  A latency well read by matter but not by
wave propagation reproduces the FTD-1020 class-0 lensing null.

---

## Acceptance criteria

A proposed native generator passes this gate only if it provides:

1. A declared configuration space:

```text
s histories, J histories, or both
boundary conditions
zero-mode convention
enabled toggle class
```

2. A declared weight or evolution rule:

```text
action, transfer matrix, or deterministic ensemble
```

3. Source insertion rules:

```text
functional derivatives that define C_L, K_T, Z_j, g_sJ
```

4. Agreement with the fixed native response tuple in the bare linear limit:

```text
C_L^FTD = 1
K_T^FTD = 1
Z_j^FTD = 1
g_sJ^FTD = 1
c_FTD = 1/sqrt(3)
```

5. A blocking-compatible form:

```text
B_b S_native -> S_native' with transformed couplings
```

6. An error and scheme ledger:

```text
finite L
zero mode
operator choice
blocking choice
reaction toggles
CPU/GPU parity if engine-measured
```

For the strengthened unified-action gate, add:

7. Autonomous reciprocal manifestation with an explicit local reserve source
   and inverse transaction.
8. A finite-energy stable localized recurrence whose internal phase is its
   physical clock; no imposed independent clock frequency.
9. One sourced transport capacity retaining the conditional common-admission
   identity $a_0=a_t$, with independently generated $a_m$ and spatial/Hodge
   $a_s$ responses and nonzero blind
   $\mathscr D=\mathscr S=(a_0+a_s)/a_m$ agreement between deflection and
   Shapiro delay.
10. Either exactly two positive-energy gapless transverse shear modes, or a
    separately stated equivalent geometric structure that passes the same
    radiation and polarization tests.
11. A target-blind deterministic basin pushforward equal to the squared
    coherent history weight across multiple physical contexts; inserting the
    target Born weights in a selector fails.
12. A blind long-distance native measurement of
    \(g_{\rm eff}^2/(4\pi\hbar_{\rm eff}c_{\rm eff})\) before comparison with
    the fine-structure root.

---

## Current best routes

For the narrower EFT bridge, the least speculative route remains:

```text
Option A for the linear source/flux sector
Option B for reaction-transport extensions
Option C deferred until a history measure is required for loops
```

This gives a staged bridge:

```text
1. constrained flux-energy generator for native linear response
2. deterministic history ledger for nonlinear/reaction updates
3. optional Euclidean history measure for loop/RG calculations
```

The first milestone is now:

```text
DERIV_FTD_NATIVE_RESPONSE_AND_BLOCKING.md
```

with no external constants and no QED matching.

It closes the linear constrained-flux sector:

```text
Gamma_lin[rho, J_T, Pi_T] =
    1/2 rho sigma_18^-1 rho
  + 1/2 (Pi_T^2 + c_FTD^2 sigma_18 J_T^2)
```

and leaves the nonlinear state-history measure open.

For the stronger ontological unification problem, the next milestone is not
another blocked-response fit.  It is a registered derivation-or-type-price
test:

```text
Can a Moore-local, cubic-covariant, target-blind nonlinear functional of the
existing L/R canonical pairs and ternary occupancy develop a separable
bond-capacity shear mode while preserving reciprocal energy and manifestation
work?
```

A negative result prices a new dynamical link type.  A positive result is the
first ontology-preserving carrier for the one-action program.

The phase-complete bond scope now supplies the explicit type-priced branch and
its action skeleton. It does not supersede the discriminator: the finite-state
transaction and capacity-wave gates must pass before any adoption or engine
implementation.

The exact
[C4 paired-history theorem](../derivations/quantum_foundations/THEOREM_C4_PAIRED_HISTORY_BORN_COUNT_AND_PHYSICAL_BOUNDARY_v1.md)
now supplies a non-tautological combinatorial square: after opposite-phase
cancellation, the compatible ordered-pair count is exactly \(|Z_o|^2\). This
narrows acceptance criterion 11 but does not pass it. The native action must
still generate the cancellation, pair basin, and target-blind equilibrium
pushforward; an externally defined pair set or uniform interval selector
fails the gate.

The
[coprime-ring pushforward](../derivations/quantum_foundations/THEOREM_C4_COPRIME_RING_BORN_PUSHFORWARD_v1.md)
passes the next finite sub-gate: consecutive cyclic memory lengths make a
single deterministic orbit through every ordered record pair, so
same-port/same-rail events have exact normalized \(|Z_o|^2\) frequencies. This
is no longer a probability-table selector. Criterion 11 remains open because
the common action has not generated the residual records, reversible
cancellation, shared-bank/address-head preparation, detector work/reset, native
general-amplitude bank formation, or multipartite no-signalling.

The
[reversible cancellation/click circuit](../derivations/quantum_foundations/THEOREM_REVERSIBLE_C4_CANCELLATION_AND_CLICK_CIRCUIT_v1.md)
passes the finite inverse and record-retention sub-gates: opposite phases move
to complete dark records, and a nondestructive C4-covariant comparator
preserves the self-address terms. Criterion 11 remains open at the stronger
physical boundary: the common action must generate the router, detector
reserve, manifestation work, amplification/reset, native general-amplitude
bank formation, and multipartite causal composition.

The exact
[controlled-actualization/work boundary](../derivations/quantum_foundations/THEOREM_C4_CONTROLLED_ACTUALIZATION_AND_CONTINUOUS_WORK_BOUNDARY_v1.md)
adds a prior gate for the one-action program. The C4-controlled
reserve-to-link transaction is an exact charge-neutral, payload-preserving,
one-token-energy involution. But no finite reserve alphabet can compensate a
generic continuous switching work. Therefore a discrete-first native action
must derive the real \(q_\pm,p_\pm,J\) action by blocking finite transaction
records; the current real-field action is a hybrid effective skeleton, not the
microscopic closure.

The exact
[C18 uniform-token bare blocking theorem](../derivations/gravity_cosmology/THEOREM_C18_UNIFORM_TOKEN_BARE_BLOCKING_v1.md)
passes the first finite-to-real rung without target constants. Reversible
one-hop streaming of uniform C4-plus-blank directed records yields an exact
common/relative covariance chart, isotropic relative-vector Hessian, and
positive rank-six capacity trace/shear Hessian. It remains a bare counting
measure: interactions, work, stable matter, a tensor pole, lensing, and native
coupling are not derived, and the bare shear cost retains cubic anisotropy.

The
[C18 single-record collision no-go](../derivations/gravity_cosmology/THEOREM_C18_EQUIVARIANT_SINGLE_RECORD_COLLISION_NO_GO_v1.md)
then removes the smallest putative interaction. The exact cubic centralizer
contains only identity and antipodal reversal on each direction shell, which
become ballistic rays or two-tick bounces after streaming. Any native
interaction must therefore be conditional on a joint multi-record state or a
dynamical local controller; fixed one-token routing cannot close this item.

The
[two-record phase-complete scattering construction](../derivations/gravity_cosmology/THEOREM_C18_TWO_RECORD_PHASE_COMPLETE_SCATTERING_AND_AXIAL_ROUTING_BOUNDARY_v1.md)
is the first explicit survivor of that boundary. Its selected local
involution scatters exact FCC doubletons while preserving momentum and C4
payload. A unique cubic-covariant route exists in twelve grazing sector types;
six axial types route only equal phases unless a native handed/controller
record is supplied. This advances the item from bare streaming to a concrete
interacting reference permutation, but does not close the common-action,
blocked-kernel, matter, gravity, Born, or coupling requirements.

Its first blocked diagnostic is now exact. The
[two-record linearized-kernel theorem](../derivations/gravity_cosmology/THEOREM_C18_TWO_RECORD_LINEARIZED_KERNEL_AND_TENSOR_BOUNDARY_v1.md)
gives the complete FCC product-reference spectrum. Only four phase counts and
three phase-blind momentum components are collision-protected. All nontrivial
C4 phase-vector currents have correction eigenvalue $-8$; the $E_g$ and
$T_{2g}$ capacity shears relax with unequal rates, and 24 normalized SC
tangent modes remain spectators. Thus this collision is a valid scattering
primitive but is closed negative by itself as the missing electromagnetic or
tensor/gravity action.

The scoped vector obstruction now has a constructive repair. The
[FCC Gaussian-current collision theorem](../derivations/charge_gauss_native_em/THEOREM_C18_FCC_GAUSSIAN_CURRENT_COLLISION_AND_MAXWELL_MODE_PRICE_v1.md)
defines one target-free order-four local permutation on equal-phase FCC pairs.
It protects exactly record number and one complex vector current
$\mathcal C=\sum i^p d$, rather than four phase species or phase-blind
momentum. Its exact product-reference kernel has nullity seven and strictly
damps every other zero-wavevector mode. The actualization vertex injects
exactly one ninth of a normalized FCC current quantum. This passes the
zero-mode representation/source-alignment sub-gate, but not the finite-$k$
Maxwell gate: native longitudinal constraints, a propagating pole, static
Green kernel, conservative source work, SC exchange, and operational coupling
are still absent. No C4-weighted tensor moment is protected.

The registered streaming completion then closes this particular vector route
negative. The
[Gaussian-current Bloch boundary](../derivations/charge_gauss_native_em/THEOREM_C18_FCC_GAUSSIAN_CURRENT_BLOCH_DIFFUSION_BOUNDARY_v1.md)
proves $LK_aR=0$ on all three axes. The protected transverse branches start at
$1+\beta k^2$, with negative real and nonzero chiral imaginary parts, rather
than $1\pm i c|k|$. A common C4 phase advance commutes with the operator and
does not restore a cone. The exact current/source alignment remains useful,
but the next action must add an oriented bond--plaquette Hodge/curl exchange
or a true cotangent/Jordan structure.

The first repair target is now exact. The
[oriented bond--plaquette Hodge theorem](../derivations/charge_gauss_native_em/THEOREM_ORIENTED_BOND_PLAQUETTE_HODGE_MAXWELL_TARGET_AND_FINITE_LIFT_BOUNDARY_v1.md)
proves that one orientation bit is both necessary for the axial sign and
sufficient to define a cubic-covariant face carrier. Its centered-incidence
edge--face generator has characteristic polynomial
$\lambda^2(\lambda^2+|q|^2)^2$, preserves both divergence constraints, and
has the required two linear-cone polarization pairs. This passes a kinematic
target gate only. A payload-complete finite local lift, work, source
continuity, and absence of extra modes remain open.

The transport half of that lift is now exact. The
[shared-edge Hodge flag theorem](../derivations/charge_gauss_native_em/THEOREM_SHARED_EDGE_HODGE_FLAG_BCC_PROPAGATION_AND_MAXWELL_REDUCTION_BOUNDARY_v1.md)
constructs a 48-state finite permutation whose three orthogonal SC steps give
one BCC displacement at speed $1/\sqrt3$. Its Laurent Bloch operator has
nonzero first-order group velocity on eight BCC rays. The remaining debt is
now sharply dynamical: sixteen ballistic flag cycles must be mixed by a
reversible collision into two transverse Hodge modes, with Gauss continuity,
capacity/work, and source ownership intact.

The cotangent successor now closes the finite vacuum-vector gate. The
[global-C3 cotangent collision](../derivations/charge_gauss_native_em/THEOREM_GLOBAL_C3_COTANGENT_LAYER_COLLISION_AND_VACUUM_MAXWELL_PASS_v1.md)
uses three clock-indexed conjugate collisions with exact layer covariance. Its
three-tick first-order generator contains two transverse electric--magnetic
pairs at speed $1/6$. The same scalar block does not admit a local constant
charged Gauss graph, so this is a vacuum-Maxwell pass rather than charged
electromagnetism.

The
[cotangent stabilizer packet](../derivations/charge_gauss_native_em/THEOREM_COTANGENT_STABILIZER_PACKET_REVERSIBLE_GAUSS_SOURCE_v1.md)
then supplies one canonical electric edge quantum as an eight-record $D_4$
orbit. Its reserve/active transfer is reversible and obeys exact Gauss
incidence with the manifested endpoint pair. The source and transverse carrier
are now canonically aligned, but charged relaxation, active/reserve work, and a
static Coulomb coat remain open.

The corresponding
[joint cotangent EM/tensor collision](../derivations/gravity_cosmology/THEOREM_COTANGENT_EM_TENSOR_EQUIVARIANT_COLLISION_AND_SPIN2_BOUNDARY_v1.md)
closes the current two-record tensor completion negative. Its equivariant rank
ceiling forces an extra phase-blind $E_g$ shear pair, all C4 tensor modes have
zero first-order envelope velocity, and the surviving modes generically split
the Maxwell cone. Higher-occupancy, staggered, or collective capacity transport
is required for gravity.

The
[cotangent STF parity-price and spin-2 curl target](../derivations/gravity_cosmology/THEOREM_COTANGENT_STF_PARITY_PRICE_AND_SPIN2_CURL_TARGET_v1.md)
now explains that zero rather than merely observing it. Both old tensor
quadratures are inversion even, so inversion equivariance kills every
linear-in-$k$ even-to-even tensor operator. The existing handed flag already
spans the missing odd pseudotensor $h\,\operatorname{STF}(D)$. The unique
isotropic even/odd symmetric curl preserves the TT projector, has exactly two
helicity-two modes, and conditionally shares the Maxwell speed $1/6$ when the
same cotangent incidence ledger is reused. A parity-preserving on-site C4
implementation has exact rank-twenty phase/parity price; a smaller physical
implementation must declare a genuine primal/dual stagger. This is the new
finite-lift target, not yet a propagating native gravity sector.

The
[even-STF second-order action theorem](../derivations/gravity_cosmology/THEOREM_EVEN_STF_SECOND_ORDER_ACTION_SPIN2_ESCAPE_AND_CONSTRAINT_PRICE_v1.md)
adds an alternate finite-action target. A nearest-neighbor STF potential with
local divergence constraints leaves two configurations and has an exact
symplectic, positive, full-band-stable speed-\(1/6\) transfer. It escapes the
first-derivative parity no-go through a Jordan/gauge-degenerate uniform mode,
so it can use the existing rank-ten even tensor potential/momentum type rather
than an explicit rank-twenty parity carrier. Its constraints remain selected,
not native. Because the local manifestation STF source has determinant
\(2/27\) and is never TT, a scalar/vector constraint sector is compulsory for
local sourcing and is simultaneously the missing static/lensing sector.

The
[common-capacity lensing/Shapiro discriminator](../derivations/gravity_cosmology/THEOREM_COMMON_CAPACITY_LENSING_SHAPIRO_RESPONSE_DISCRIMINATOR_v1.md)
also replaces the qualitative lensing demand by one blind ratio. Let
$(a_m,a_t,a_0,a_s)$ be the slow-body, material-clock, wave-temporal, and
wave-spatial responses to one weak static capacity depth. For a point-depth
exterior,

\[
 \mathscr D=-{b\theta\over2\mu_m}
 =\mathscr S={a_0+a_s\over a_m}.
\]

The source normalization cancels, while $a_t/a_m$ remains independent. Thus
FTD-1019 clock/fall coherence and FTD-1020 class 0 are mutually consistent:
the current wave coefficients are zero. Shared capacity alone does not fix
them. A finite capacity-weighted Maxwell principal operator must derive
$a_0/a_m$ and $a_s/a_m$ before any lensing fixture is authorized.

The
[self-dual trace-capacity action boundary](../derivations/gravity_cosmology/THEOREM_SELF_DUAL_TRACE_CAPACITY_STATIC_POLE_AND_EQUAL_RESPONSE_LENSING_BOUNDARY_v1.md)
now supplies a complete conditional witness. The trace of the same
actualization moment sources a selected primal/dual quadratic action with
massless common pole \(1/(\kappa\Lambda)\) and exact solution \(U_t=U_s\).
Selected normalized matter, clock, Maxwell-time, and Hodge-space readouts then
give \((a_m,a_t,a_0,a_s)=(1,1,1,1)\) and
\(\mathscr D=\mathscr S=2\). The discriminator is passed by the reference
action, but native lensing is not: the equal coupling/readouts, finite
capacity transaction, vector constraints, and inhomogeneous wave operator
remain unproved.

The exact
[common-admission clock/Maxwell theorem](../derivations/common_action_mechanics_reciprocity/THEOREM_COMMON_ADMISSION_CLOCK_MAXWELL_AND_SPATIAL_LENSING_PRICE_v1.md)
now closes the temporal part conditionally. For every retained binary
permission history, a material clock and a complete cotangent Maxwell advance
gated by the same history evolve by the same admitted count. The admission
fraction $\nu$ therefore scales the material-clock rate and the Maxwell cone
to $\nu/6$. If $\nu(U)=1-a_tU+O(U^2)$, this forces $a_0=a_t$. Together with
clock/fall coherence $a_t=a_m$, admission alone reaches the class-1/1911
response $\mathscr D=\mathscr S=1$, not class 2. The remaining second half is
a distinct spatial/Hodge transaction generating $a_s=a_m$. The current
production stencil remains class 0 because it does not yet share this gate.

The successor
[primal/dual permission theorem](../derivations/common_action_mechanics_reciprocity/THEOREM_PRIMAL_DUAL_PERMISSION_IDEMPOTENCE_AND_LENSING_FACTOR_PRICE_v1.md)
proves the minimum binary type price. Reusing the temporal bit cannot produce
the spatial coefficient because $g^2=g$. Two separately retained permissions
give clock count $N_t$ and first-order wave count $N_{11}$; a reversible
deterministic product orbit realizes
$N_{11}/N=(N_t/N)(N_s/N)$ exactly. Equal primal/dual marginals would therefore
give $c_{\rm ray}=\nu^2/6$ and class 2 under $a_t=a_m$. The native action has
not yet generated those permissions, their equality, their physical Hodge
ownership, or an inhomogeneous lensing operator.

The exact
[A9/cotangent no-spare-scalar theorem](../derivations/common_action_mechanics_reciprocity/THEOREM_A9_COTANGENT_NO_SPARE_SCALAR_PERMISSION_AND_DUAL_COPY_PRICE_v1.md)
then rules out obtaining the pair for free. On the physical A9 clock orbit,
link and reserve capacities are complementary and no nonconstant partial
self-gate is reversible. The cotangent $O_h\times C_4$ carrier is transitive,
so handedness or phase cannot be repurposed as an invariant scalar capacity.
A second independently owned A9 copy on the dual complex is the minimum
existing-alphabet type repair: its complete product census has marginals
$1/2,1/2$ and joint $1/4$. Native dual placement, generator selection, and
sourced marginals remain open at that type-audit stage.

The
[dual-A9 skew generator](../derivations/common_action_mechanics_reciprocity/THEOREM_DUAL_A9_SKEW_CAPACITY_CLOCK_GENERATOR_AND_HOMOGENEOUS_FACTOR_PASS_v1.md)
now closes the local-generator part at one homogeneous point. The dual record
advances every global tick and its retained blank capacity admits or stalls
the primal A9 clock; the primal capacity is the separate spatial permission.
The triangular map has an exact inverse and all 32 deterministic orbits obey
$(N,N_t,N_s,N_{11})=(16,8,8,4)$. Thus no external permission word is needed
for the fixed half-admission fixture. Variational selection, a source-induced
change of those residence counts, the finite cotangent/TT gate, and lensing
remain open.

The exact
[dual-capacity cyclic-mixing theorem](../derivations/common_action_mechanics_reciprocity/THEOREM_DUAL_CAPACITY_CORRELATION_OBSTRUCTION_AND_CYCLIC_MIXING_RESPONSE_v1.md)
then tests that source-induced extension. Collocating occupied skew cells with
open vacuum cells leaves covariance $\rho(1-\rho)/4$ and produces coefficient
$3/2$, not 2. A reversible one-hop shift of the dual layer repairs this
exactly: over one cycle every primal slot meets every dual slot once, so
$\bar j=\nu_t\nu_s$ for arbitrary finite patterns and equal counts give
$(1-M/L)^2$. The same theorem proves that one actualization token cannot
simultaneously occupy both independent A9 layers. A two-token or explicit
time-sharing source ledger, $M(U)$, and the actual field lift remain open.

The exact
[3D $O_h$ Moore-local mixing theorem](../derivations/common_action_mechanics_reciprocity/THEOREM_OH_MOORE_LOCAL_DUAL_CAPACITY_MIXING_AND_ISOTROPIC_FACTOR_PASS_v1.md)
then closes the axial-reference defect. A mixed-radix walk visits every
relative translation of $\mathbb Z_L^3$ once; its signed-cubic forward/reverse
completion has zero drift and isotropic second moment, and arbitrary 3D
binary patterns factorize exactly. This is a global-clock reference schedule,
not a local variational selection. Native frame scheduling, a C18-only
alternative if required, the source ledger, cotangent/C3 composition, and the
TT lift remain open.

The exact
[single-record STF streaming-lift obstruction](../derivations/gravity_cosmology/THEOREM_COTANGENT_SINGLE_RECORD_STF_STREAMING_LIFT_OBSTRUCTION_v1.md)
then closes the most economical tensor implementation negative. The regular
48-flag orbit leaves only eighteen cubic-equivariant C18 route seeds per C4
phase. Every co-layer first moment is zero. Both adjacent-layer staggers
together span only a rank-three cubic operator family, while the symmetric
tensor curl lies outside it; the natural witness preserves TT only on cubic
axes. A genuine multi-record parity collision, larger carrier, or longer
finite construction is now mandatory.

The complete
[right-regular collision census](../derivations/gravity_cosmology/THEOREM_COTANGENT_RIGHT_REGULAR_COLLISION_SPIN2_SLOW_CLOSURE_OBSTRUCTION_v1.md)
then tests whether a nontrivial one-record local collision repairs the
failure. Without a closure gate its projected derivative span does contain
the curl. But only sixteen of 48 collisions preserve the selected ten tensor
variables at zero momentum on each layer, and their entire derivative span is
zero. The apparent curl is therefore fast-mode leakage. The spin-2 branch now
requires a declared multi-record invariant slow carrier, not another
one-record permutation.

The
[rank-twenty collision-closure theorem](../derivations/gravity_cosmology/THEOREM_COTANGENT_RANK20_COLLISION_CLOSURE_AND_TT_LEAKAGE_v1.md)
then takes the alternative of retaining the fast copies. The complete closure
census is $16\times10$, $16\times20$, and $16\times26$ on every layer, so the
minimum target-producing closure has rank twenty. A selected five-hop C18
witness is finite and energy-self-adjoint in the induced metric, but its TT
Krylov dimension is 8 on axis, 16 on a body diagonal, and 18 generically.
Thus the next action must derive a complete constraint/gauge reduction inside
the larger invariant carrier; it may not insert a TT projector.

Alternatively, the second-order even-STF branch may replace this
first-derivative carrier. It pays a Jordan zero mode and local constraint
multipliers instead of retaining the rank-twenty collision closure. It too may
not insert a TT projector: the common action must generate the divergence
constraints and the longitudinal static response.

The exact
[rank-twenty constraint-count theorem](../derivations/gravity_cosmology/THEOREM_COTANGENT_RANK20_CONSTRAINT_COUNT_OBSTRUCTION_v1.md)
corrects the earlier four-constraint shorthand. All 98 primitive nonzero
wavevectors have rank sixteen and four conserved zero-mode rows. Even if all
four were first-class, $20-2(4)=12$, not the required helicity-two phase-space
dimension four. The actual price is $2F+S=16$: eight first-class constraints,
or four first-class plus eight second-class reductions, or an equivalent
prior elimination of the eight collision-copy dimensions.

The exact
[common Maxwell/tensor closure theorem](../derivations/gravity_cosmology/THEOREM_COTANGENT_COMMON_MAXWELL_TENSOR_COLLISION_CLOSURE_PRICE_v1.md)
then applies the one-action requirement. Every collision preserving the
registered seven Maxwell/Gauss rows has zero tensor-curl span. The symmetric
curl first appears in common invariant closures of dimension thirty, where
the tensor carrier is twenty-dimensional and the Maxwell carrier has enlarged
from seven to ten. Consequently the action must recover Maxwell/Gauss and the
tensor constraint algebra together; a gravity-only collision cannot be
appended to the existing Maxwell sector.

The exact
[rank-twenty chiral-commutant theorem](../derivations/gravity_cosmology/THEOREM_COTANGENT_RANK20_CHIRAL_COMMUTANT_AND_PARITY_PAIR_PRICE_v1.md)
then exposes the internal structure of the sixteen-dimensional reduction.
The three spatial generators have a four-dimensional commutant containing an
energy-orthogonal involution $Q$. Its two rank-ten eigensectors are invariant
and have identical spectra, while inversion anticommutes with $Q$ and
exchanges them. The four-dimensional TT seed splits $2+2$, but each chiral
seed still expands to 4, 8, or 9 dimensions. A parity-complete physical kernel
therefore requires an eight-dimensional reduction in each partner sector; no
single-sector projection is authorized.

The exact
[rank-thirty common irreducibility theorem](../derivations/gravity_cosmology/THEOREM_COTANGENT_RANK30_COMMON_IRREDUCIBILITY_AND_PARITY_INDEX_OBSTRUCTION_v1.md)
then tests whether that tensor-only split survives the one-action requirement.
It does not: after retaining the independent Maxwell-10 closure, the three
common first moments have scalar-only commutant, so there is no nontrivial
constant Maxwell/tensor projector. Inversion instead grades the common
carrier as $17+13$. Eighty-six registered wavevectors have the forced generic
four-dimensional even kernel, while twelve FCC face diagonals acquire a
$5+1$ even/odd kernel and rank only 24. A generic transverse-Maxwell plus TT
seed generates all thirty dimensions. The common physical reduction price is
$2F+S=22$; even four optimistic first-class constraints would still require
fourteen second-class reductions. A momentum-dependent local constraint/gauge
complex, not a sector projection, is required. More strongly, the normalized
nonzero axis and body-diagonal characteristic polynomials are coprime. The
selected one-layer carrier has no exact isotropic linear cone at all, so its
generator or layer composition must be repaired before constraints can
isolate Maxwell and spin-2 poles.

The exact
[phase-complete common-closure theorem](../derivations/gravity_cosmology/THEOREM_COTANGENT_PHASE_COMPLETE_COMMON_CLOSURE_AND_C4_SELECTION_v1.md)
then restores the C4 type omitted by that diagnostic. Rank thirty is a
fixed-quadrature slice, not the complete native carrier. With all four phases
retained, the closure census is \(27,33,50,71\), and the minimum
target-containing class is tensor-40 plus Maxwell-10. C4 acts nontrivially on
the tensor quadratures and trivially on Maxwell, forcing all vacuum linear
cross blocks to vanish. This is compatible with one action but localizes
physical coupling to a phase-neutral nonlinear or matter-mediated vertex.
The conditional full-carrier reduction price is \(2F+S=42\). The earlier
\(2F+S=22\) price applies only after a twenty-dimensional phase-reality or
synchronization quotient, which the action has not generated.

Finally, the
[native-alpha action-scale obstruction](../derivations/charge_gauss_native_em/THEOREM_COTANGENT_NATIVE_ALPHA_ACTION_SCALE_OBSTRUCTION_v1.md)
proves that speed $1/6$, unit packet norm, exact Gauss incidence, token energy
$8\to8$, and tangent weight $2^{-191}$ still do not determine a coupling.
Multiplying the quadratic field action by $\Gamma>0$ leaves all those
kinematics fixed while scaling the static source energy. The remaining blind
normalization is the blocked dimensionless curvature
$\chi_{\rm EM}=\Gamma/I_*$, conditional on first deriving a charged massless
pole. Only then could $\alpha_{\rm native}=3\chi_{\rm EM}/(2\pi)$ be measured.

The later C4-trivial field-handoff selection relates, but does not determine,
this normalization. If the same (I_*,\Gamma) govern emission and the static
residue, the clocked-worldline successor gives
\(I_*=\Gamma/2+\mu/(2L)\),
\(\chi_{\rm work}=\Gamma L/(\Gamma L+\mu)\), and
\(\chi_{\rm EM}=\Gamma/I_*=2\Gamma L/(\Gamma L+\mu)\), where
\(\mu=m/L\) is the material impulse. Thus the remaining blind data are
\(L,m\), the field Noether charge, and proof that the emission and static
coefficients are one action parameter. No master-root comparison is licensed.

The
[charged-pole reciprocal-alpha protocol](../derivations/charge_gauss_native_em/THEOREM_COTANGENT_CHARGED_POLE_RECIPROCAL_ALPHA_MEASUREMENT_PROTOCOL_v1.md)
now supplies the missing pole and blind estimator for one selected canonical
Maxwell--Gauss reference action. Its cubic kernel is $1/\Lambda(k)$, while the
static estimator $2(H_k/I_*)\Lambda(k)/|\rho_k|^2$ and the unit-packet
free-field Hessian both return exactly $\chi_{\rm EM}$. The open gate is no
longer what to measure; it is deriving the selected action, its local
reversible constraint realization, and the value of that curvature from the
same microscopic transaction.

The
[common-phase tensor-doublet theorem](../derivations/gravity_cosmology/THEOREM_C18_COMMON_PHASE_TENSOR_DOUBLET_AND_CONSTRAINT_PRICE_v1.md)
then shows that the same finite alphabet nevertheless contains the required
kinematic type: two independent rank-six common-phase tensors with a native
C4 quarter-turn $(Q,P)\mapsto(-P,Q)$. The gravity debt is now localized to the
native bracket, four constraints or equivalent, derivative pole, universal
sourcing, and lensing rather than missing tensor phase-space capacity.
That four-constraint statement applies to the bare rank-twelve $(Q,P)$ type.
The collision-closed fixed-quadrature successor carrier is rank twenty and
obeys the stronger $2F+S=16$ tensor-only price above. Retaining Maxwell on
that slice raises the common carrier to rank thirty and the slice quotient
price to $2F+S=22$. Restoring the native C4 quadrature pair raises the honest
phase-complete carrier to rank fifty and its conditional quotient price to
$2F+S=42$ unless a native phase-reality quotient is first derived.

The
[actualization shared-moment source theorem](../derivations/common_action_mechanics_reciprocity/THEOREM_C18_ACTUALIZATION_SHARED_MOMENT_SOURCE_VERTEX_v1.md)
provides the first exact common vertex. One reversible token transfer produces
the ternary endpoint pair and simultaneous relative-vector, tensor-doublet,
and capacity increments. This removes the previous freedom to source those
candidate sectors independently. The compatibility trigger, stable body,
material clock, propagating kernels, physical Born preparation, and operational
coupling readout remain open.

The exact
[phase-neutral shared charge/stress theorem](../derivations/common_action_mechanics_reciprocity/THEOREM_C18_PHASE_NEUTRAL_SHARED_CHARGE_STRESS_VERTEX_v1.md)
then supplies the nonlinear C4 bridge required by the phase-complete selection
rule. Contracting both source doublets with the manifested token phase gives
\(j_{\rm evt}=\epsilon d/9\) and
\(t_{\rm evt}=dd^{\mathsf T}/18=-\Delta K\). Thus the same event has a
phase-neutral charge-odd directed current and a charge-even tensor/capacity
source, with exact ledger
\(j_{\rm evt}j_{\rm evt}^{\mathsf T}=4t_{\rm evt}^2\). This closes common
source type, not by itself reciprocal work, constrained poles, lensing, Born
preparation, or a coupling measurement.

The exact
[C4 stress-capacity reciprocal-feedback theorem](../derivations/common_action_mechanics_reciprocity/THEOREM_C4_STRESS_CAPACITY_RECIPROCAL_FEEDBACK_AND_MAXWELL_PARITY_PRICE_v1.md)
now closes the smallest autonomous even-source loop. Response capacity admits
the material A9 clock, the post-drift persistent stress source toggles response
ownership, and the response phase advances on the global tick. The full
256-state map is reversible without an event log or external permission word;
its sourced cycles contain eight admitted material ticks and eight stress
kicks per twelve global ticks. This is finite backpressure, not a variationally
selected action or physical energy-work law. The same exact C4 involution
census excludes reusing that clock orbit for charge-odd Maxwell response, so
the distinct signed/cotangent Maxwell carrier remains a required part of the
one-action lift.

The subsequent
[common material/stress/Gauss theorem](../derivations/common_action_mechanics_reciprocity/THEOREM_C4_COMMON_MATERIAL_STRESS_GAUSS_TRANSACTION_AND_FIELD_BOUNDARY_v1.md)
places that distinct carrier in the same finite local map. A material A9
token, stress-response A9 token, and eight-record cotangent packet obey one
explicit inverse. On every active SC source state,
\(E/8=9j=\epsilon d\), \(B=0\), \(\partial E=\rho\), and the even response
has the same one-token tensor norm as \(t\). Charge conjugation reverses the
current/electric packet and preserves tensor/stress. The packet is still
bound source dressing, however; release, layer-covariant propagation,
field-to-matter momentum transfer, and physical work remain open.

The subsequent
[cotangent framed-plaquette release theorem](../derivations/charge_gauss_native_em/THEOREM_COTANGENT_FRAMED_PLAQUETTE_NUMBER_NEUTRAL_RADIATION_RELEASE_v1.md)
corrects the naive release target. An eight-record Gauss packet cannot become
free radiation because it changes carrier number and has nonzero boundary.
Two phase-distinct packets give one fixed-number ternary edge, and four edges
on an ordered perpendicular SC plane give an exact 64-record transverse circulation:
\(\Delta N=0\), \(\partial\Delta E=0\), charge-odd response, and first-order
membership in the certified vacuum-Maxwell sector. This is a reversible local
release vertex, not yet a global radiative action. The plane is the four-way
quotient $v=hn$ and is supplied equivariantly by an ordered right-angle
material turn; the current one-bond matter clock does not yet retain that
history. Finite-amplitude collision scheduling, positive field energy, recoil,
and Lorentz force remain acceptance debts.

The
[C4 square-material turn theorem](../derivations/common_action_mechanics_reciprocity/THEOREM_C4_SQUARE_MATERIAL_TURN_CLOCK_AND_ENDOGENOUS_RADIATION_FRAME_v1.md)
closes that frame route on a prepared spatial recurrence. A neutral ternary
dipole advances around one ordered SC square as a period-four material clock;
its two endpoint currents obey exact continuity, its cycle current cancels,
and its positive mean plane stress survives. Every corner carries the ordered
turn required by the transverse release seed. The remaining debt is no longer
an external plane selector but formation/binding of the loop and a
nondegenerate field/matter energy-momentum exchange.

The exact
[C4 square-matter/stress/radiation work theorem](../derivations/common_action_mechanics_reciprocity/THEOREM_C4_SQUARE_MATTER_STRESS_RADIATION_RECIPROCAL_WORK_EXCHANGE_v1.md)
now closes the local **energy** half of that exchange. One matched permutation
combines the material clock, stress-capacity owner, and transverse seed; its
36,864 states form 1,536 period-24 orbits and obey
\(\Delta h_F=-\Delta h_C\), \(h_F+h_C=1\). The active field norm is exactly
16. Identifying that complete seed with one capacity unit fixes
\(\chi_{\rm EM}=1/16\) only on a declared conditional section, so it is not a
native alpha result. The seed remains a local emission/reabsorption oscillator
with \(B=0\) and zero initial Poynting momentum. Finite collision/streaming,
directional field momentum, reciprocal material recoil, and Lorentz force are
therefore the next electromagnetic action gate.

The
[handed directional-port theorem](../derivations/charge_gauss_native_em/THEOREM_COTANGENT_HANDED_DIRECTIONAL_RADIATION_PORT_AND_MOMENTUM_BOUNDARY_v1.md)
then resolves the field-momentum half and exposes its exact type price. The
reflection fixing an ordered polar material plane reverses every polar normal,
so that plane alone cannot choose an outgoing direction. One spatial
pseudoscalar gives \(r=\chi(d\times v)\). With it, two phase-distinct
eight-record ray banks form fixed-number standing/outgoing ports with
canonical norms \(1,2\), Poynting momenta \(0,r\), and an exact microscopic
three-tick centroid displacement \(r\). Complementary capacity gives
\(g+h_F=2\) without the earlier internal \(1/16\) divisor, but dimensional
sector matching remains selected. The planar material loop does not yet own
\(\chi\) or a translational recoil coordinate, and the coarse Maxwell norm is
not preserved after the collisionless packet spreads. Those are now the
precise force/propagation debts.

The exact
[C6 cubic-Petrie material theorem](../derivations/common_action_mechanics_reciprocity/THEOREM_C6_CUBIC_PETRIE_MATERIAL_CLOCK_AND_ENDOGENOUS_DIRECTIONAL_PORT_v1.md)
retires the independent-pseudoscalar ownership debt on a prepared nonplanar
recurrence. The closed route \(d,v,w,-d,-v,-w\) carries a neutral period-six
material clock with exact continuity and isotropic mean stress \(I_3/3\).
At every corner its third retained direction obeys
\(e_{q+2}=\chi_q(e_q\times e_{q+1})\), where the route determinant
\(\chi_q\) has exactly the required pseudoscalar transformation law. Thus the
same prepared matter history supplies the outgoing port direction without a
free chirality label. The remaining reciprocal debt is now a derived
center-of-mass momentum coordinate and the write \(\Delta p_M=-e_{q+2}\),
plus a collision that preserves coarse Maxwell energy after emission.

The
[Petrie reciprocal-recoil theorem](../derivations/common_action_mechanics_reciprocity/THEOREM_C6_PETRIE_DIRECTIONAL_PORT_RECIPROCAL_RECOIL_CURRENT_v1.md)
now closes that write at the level the ternary ontology presently earns. The
local involution
\((x,{\cal S},1)\leftrightarrow(x-r,{\cal O},0)\) holds the
sixteen-record field anchor fixed, moves both manifested endpoints one SC hop,
preserves exact charge continuity and \(g+h_F=2\), and satisfies
\(\Delta x_M+\Delta p_F=0\). The material vector is a displacement current,
not yet physical momentum: deriving a translational Legendre/dispersion map
and kinetic energy is now the precise matter-side debt. The field-side debt is
ownership handoff into a coarse-energy-preserving Maxwell collision/streaming
map.

The exact
[recoil energy-partition theorem](../derivations/charge_gauss_native_em/THEOREM_DIRECTIONAL_PORT_RECOIL_ENERGY_PARTITION_AND_COUPLING_MEASURE_BOUNDARY_v1.md)
shows that those two debts cannot be separated. Full cubic invariance forces
the quadratic translational form to an isotropic scalar coefficient. Its
\(\mu/2\) cost applies to the provisional immediate unit-step model. The
C4-trivial handoff successor fixes emitted field work to \(\Gamma/2\), and
the clocked-remainder successor below changes stable material motion to speed
\(1/L\), with impulse \(\mu=m/L\) and recoil cost \(\mu/(2L)\). The current
conditional partition is
\(I_*=\Gamma/2+\mu/(2L)\) and
\(\chi_{\rm work}=\Gamma L/(\Gamma L+\mu)\). This is an operational
definition, not alpha. Deriving \(L,m\), and the field Noether charge from one
formed transaction is now the coupled acceptance gate.

The
[directional-port rigid-propagation boundary](../derivations/charge_gauss_native_em/THEOREM_DIRECTIONAL_PORT_LOCAL_COLLISION_RIGID_PROPAGATION_BOUNDARY_v1.md)
then rules out the simplest handoff. Exhausting every stage-one port target
and translation shows that a local record-number/\(E/B\)-preserving collision
can only refocus the sixteen-record port through in-plane re-anchoring;
every survivor has \(a\cdot r=0\). No one-tick rigid outward port translation
exists in this class. The vacuum lift must therefore be a multi-tick
dispersive/coherent packet, a larger collective carrier, or a different
energy-preserving amplitude/cochain realization.

The exact
[post-separation multi-ray theorem](../derivations/charge_gauss_native_em/THEOREM_DIRECTIONAL_PORT_POSTSEPARATION_MULTIRAY_ENERGY_MOMENTUM_CARRIER_v1.md)
passes the multi-tick branch. Solving every affine ray-intersection equation
shows that after tick two the sixteen records form eight nonintersecting
phase-distinct, readout-identical BCC rays forever, with constant coarse norm \(1\), Poynting
\(r/2\), and centroid advance \(r\) per three ticks. Thus finite directional
propagation and stable post-separation field momentum now exist. The remaining
raw coarse-moment defect is localized to the handoff:
\((2,r)\to(1,r/2)\). The eight ballistic rays have not been reduced to the two
Maxwell hydrodynamic modes.

The
[coherence-metric handoff theorem](../derivations/charge_gauss_native_em/THEOREM_DIRECTIONAL_PORT_COHERENCE_METRIC_HANDOFF_AND_PHASE_COMPATIBILITY_BOUNDARY_v1.md)
shows that this cross-term debt cannot be settled by symmetry alone. The four
handedness/phase channels admit an invariant Gram \(G(a,b,c)\). Handoff
conservation fixes only \(c=-a\); positivity leaves a continuum and emission
work \((b-a)/2\). Fully resolved and phase-coherent flag-resolved metrics both
conserve the handoff while assigning different work. Therefore the common
action requires the actual C4 field-type datum in addition to symmetry.

The exact
[C4 Born/radiation kernel-separation theorem](../derivations/common_action_mechanics_reciprocity/THEOREM_C4_BORN_RADIATION_KERNEL_SEPARATION_AND_CONTEXTUAL_MIXER_BOUNDARY_v1.md)
proves that this field Gram is **not** the raw Born kernel. The phase-blind
field readout occupies the trivial C4 sector, while the Born norm occupies the
orthogonal quadrature sector; using the Born opposite-phase weight in the
handoff family forbids positive emission. Coupling and contextual measurement
remain one shared **action** gate, but the action must derive two sector
projectors and a physical linear or nonlinear conversion vertex rather than
reuse one compatibility coefficient.

The exact
[C4-trivial field-sector handoff theorem](../derivations/charge_gauss_native_em/THEOREM_C4_TRIVIAL_FIELD_SECTOR_UNIQUE_DIRECTIONAL_PORT_HANDOFF_METRIC_v1.md)
supplies that datum. Phase blindness fixes (b=1,c=a); handoff conservation
fixes (c=-a); hence ((a,b,c)=(0,1,0)) uniquely. The selected physical
candidate ledger is ((H,P):(1,r/2)\to(1,r/2)), with emitted field work
(1/2). The handoff metric ambiguity is closed conditionally, leaving action
realization, field translational Noether momentum, cadence, and inertia.

The exact
[C4 phase-parity half-admitted carrier](../derivations/charge_gauss_native_em/THEOREM_C4_PHASE_PARITY_HALF_ADMITTED_TWO_POLARIZATION_MAXWELL_CARRIER_v1.md)
closes the kinematic mode-count/speed target. The eight-ray readout has rank
two with (B=r\times E); alternate C4 phase parity admits movement on half the
ticks, preserves (H=1,P=r/2), and yields two degenerate outgoing transverse
modes at speed (1/6). The remaining field debt is action selection and
nonlinear protection of this cadence, plus canonical translation momentum,
not an eight-versus-two internal polarization mismatch.

The preregistered
[field-packet reserve-current theorem](../derivations/common_action_mechanics_reciprocity/THEOREM_C4_FIELD_PACKET_RESERVE_DENSITY_CURRENT_AND_ATOMIC_CLOCK_DEBIT_BOUNDARY_v1.md)
then reuses the same phase-complete carrier as physical reserve hardware. The
eight positive energy groups obey an exact pointwise discrete continuity law,
give a signed Moore-local boundary current, retain their inverse, and support
atomic whole-packet debit/refill without double spending. Packet ownership
realizes the FTD-0999 balance exactly. This closes the scalar-reserve-to-carrier
gap only conditionally: the field metric, parity schedule, clock absorption,
and scale compliance remain selected. If $d$ packets maintain one clock
quantum, the common action must force rather than assume
$\chi_{\rm EM}=\omega_0/d$.

The subsequent selected
[reciprocal packet/clock/recoil generator](../derivations/common_action_mechanics_reciprocity/THEOREM_RECIPROCAL_PACKET_CLOCK_RECOIL_ABSORPTION_GENERATOR_AND_GRAVITY_SOURCE_BOUNDARY_v1.md)
closes one exact canonical absorption map: packet energy becomes clock action
plus recoil, declared translation charge moves to matter, the complete map is
symplectic and invertible, and scalar-$T_{00}$ ownership is continuous. This
does not make the map substrate-native. Its trigger, packet momentum, inertia,
clock rate, action scale, finite ternary realization, tensor-stress handoff,
and nonlinear completion remain unforced.

The subsequent locked
[symmetric-stress discriminator](../derivations/common_action_mechanics_reciprocity/THEOREM_C4_SYMMETRIC_STRESS_PACKET_MOMENTUM_AND_SOURCE_HANDOFF_BOUNDARY_v1.md)
proves that the finite packet alone does not select real momentum. If the
common action derives the one Lorentz-compatible relation $J_E=c^2p_F$, then
$p_F=6Er$ and $\Sigma_F=Err^{\mathsf T}=18E t_{\rm evt}$ follow uniquely,
making recoil and scalar/STF gravity sourcing projections of one stress. That
condition and its tensor transfer remain selected/open; no native lensing or
coupling value follows.

The subsequent
[C18 existing-type constraint seam](../derivations/common_action_mechanics_reciprocity/THEOREM_C18_EXISTING_TYPE_SCALAR_STF_VECTOR_CONSTRAINT_ABSORPTION_SEAM_AND_EQUAL_COUPLING_BOUNDARY_v1.md)
shows that no new spatial irrep is needed for the scalar, STF, EM-vector, and
separate longitudinal-vector owners. Constraint preservation uniquely ties
the vector load to the STF load, and one generator preserves energy and the
inverse. However, the scalar and tensor coefficients remain independently
rescalable; the finite action must derive their common normalization before
the reference lensing tuple can become physical.

The subsequent
[finite transverse constraint-bundle theorem](../derivations/common_action_mechanics_reciprocity/THEOREM_C18_TRANSVERSE_CHARGE_EVEN_CONSTRAINT_BUNDLE_AND_AXIAL_TWO_OWNER_BOUNDARY_v1.md)
now closes the predecessor's finite charge-even ownership debt on transverse
nearest-neighbor charts. One retained SC/FCC plane bundle has
\(\Delta J_{\rm EM}=0\), \(\Delta J_{\rm C}=4r\), exact inverse, signed-cubic
covariance, and common-C4 compatibility, and it realizes \(216T_rq\) when
\(q\perp r\). Axial incidence is not closed: its \(D_4\) stabilizer exchanges
the two transverse planes, the required \(8r\) load costs both bundles, and
one local SC owner cannot pay both. The action must supply a second owner or
distributed retained-history repair before the static/tensor/lensing branch
can become finite.

The subsequent
[Hodge-framed all-axis signed-event theorem](../derivations/common_action_mechanics_reciprocity/THEOREM_HODGE_FRAMED_ALL_AXIS_CONSTRAINT_LIFT_AND_ONE_SIGNED_EVENT_GENERATOR_BOUNDARY_v1.md)
closes that spatial-context repair at blocked prepared-reference level. The
existing electromagnetic Hodge flag provides two covariant transverse axes,
so one/two finite owner bundles realize every \(216T_rq\) chart while leaving
the electromagnetic bundle current zero. One signed type-2 generator then
joins the prepared bright-pair ownership swap to manifestation, charge
current, trace/STF/constraint source, recoil, clock action, event energy, and
the required port-conjugate reaction. All 748,824 exact checks pass. Native
flag/owner formation, autonomous history preparation, field poles, stable
matter, coupling normalization, static response, and lensing remain open.

The subsequent
[uniform-counting joint source-metric theorem](../derivations/common_action_mechanics_reciprocity/THEOREM_C18_UNIFORM_COUNTING_JOINT_SOURCE_METRIC_AND_BORN_MEASURE_SEAM_v1.md)
replaces the raw source-norm comparison by the inverse covariance of the same
finite five-state measure. The resulting rank-24 current/tensor/capacity
Mahalanobis cost is invariant when both coordinates and covariance are changed,
and the prepared Born map is the pushforward of the corresponding uniform
ordered-pair count. This supplies one bare action/measure normalization
candidate, not a physical coupling: the interacting dynamical Hessian has not
been shown to equal the Fisher Hessian, and the distinct SC/FCC shear costs
leave the native spin-2, lensing, and autonomous-preparation gates open.

The exact
[half-admitted energy-current theorem](../derivations/common_action_mechanics_reciprocity/THEOREM_C4_HALF_ADMITTED_ENERGY_CURRENT_AND_MECHANICAL_MOMENTUM_BOUNDARY_v1.md)
separates three previously conflated quantities. The conserved selected energy
has centroid current \(J_E=r/6\); the local \(E\times B\) readout is
\(r/2=3J_E\); canonical translation momentum is still absent. A one-tick
material hop is \(6c_{\rm eff}\) if interpreted as coarse velocity, so the
recoil identity cannot itself be a stable matter worldline.

The exact
[clocked-remainder recoil theorem](../derivations/common_action_mechanics_reciprocity/THEOREM_CLOCKED_REMAINDER_RECOIL_AND_DISCRETE_TRANSLATION_CHARGE_BOUNDARY_v1.md)
now closes the material-side reference gate. Its finite lift
\(Y=Lx+a\) obeys \(Y'=Y+d\); one SC impulse produces one visible hop per
\(L\) ticks and stable speed \(1/L\). The standing/outgoing vertex writes
\(d:0\leftrightarrow-r\) without immediate displacement. A conditional
quadratic action yields \(p_M=(m/L)d\), \(K_M=m/(2L^2)\), and a canonical
exchange coefficient \(\kappa=m/L\). What remains is to derive \(L,m\), the
field translation charge, and the complete energy-preserving interaction
from the common action.

The exact
[paired-history phase-neutral actualization vertex](../derivations/common_action_mechanics_reciprocity/THEOREM_C4_PAIRED_HISTORY_PHASE_NEUTRAL_ACTUALIZATION_SOURCE_VERTEX_v1.md)
now supplies the nonlinear sector bridge on the prepared domain. The unique
normalized symmetric C4 quadrature contraction gives (+1,0,-1) for bright,
cross-rail, and dark pairs; reversible dark cancellation leaves exactly
(|Z_o|^2) positive contractions. A positive pair controls the manifested
token whose same ownership transfer produces the phase-neutral charge-odd
current and charge-even stress/capacity source. What remains external is the
autonomous generation and contextual routing of the history ensemble, plus
the reciprocal field-work response.

The earlier
[physical C4 Born actualization tape](../derivations/quantum_foundations/THEOREM_C4_PHYSICAL_BORN_ACTUALIZATION_TAPE_v1.md)
closed the detector manifestation step on the prepared finite equal-weight
domain. One separate detector token per coprime pointer state manifests at the
physical outcome route exactly for a bright pair, giving physical tape counts
$M_o=|Z_o|^2$ while preserving self-address terms, signal records, dark
records, token payload, and a global inverse.

The subsequent
[Gaussian-integer general-amplitude physical limit](../derivations/quantum_foundations/THEOREM_C4_GAUSSIAN_INTEGER_GENERAL_AMPLITUDE_PHYSICAL_LIMIT_v1.md)
closes the prepared amplitude-representation debt. Every finite normalized
complex response has a sequence of finite C4 banks whose manifested cotangent
Gauss-event frequencies converge with an explicit total-variation bound and
finite $O(mN^2)$ address-time price. This does not allow an amplitude compiler
in the action.

The
[autonomous reversible Born renewal detector](../derivations/quantum_foundations/THEOREM_C4_AUTONOMOUS_REVERSIBLE_BORN_RENEWAL_DETECTOR_v1.md)
now removes the tape and closes prepared steady-stream exclusivity/reset. One
balanced-ternary detector stage reuses one detector token plus one eight-record
cotangent source packet. Its total permutation quarantines misprepared dark
states, emits one exclusive Gauss event at every bright pair, and returns the
same resource to ready ownership before advancing. Criterion 11 remains open
in general because the action has not generated the residual bank and rings,
associated every externally heralded source emission with one renewal event,
controlled incomplete windows, formed a macroscopic record, or proved
multipartite no-signalling.

The
[heralded fixed-window Born Poincare pushforward](../derivations/quantum_foundations/THEOREM_C4_HERALDED_FIXED_WINDOW_BORN_POINCARE_PUSHFORWARD_v1.md)
now associates each isolated selected source herald with exactly one physical
Gauss event and closes incomplete-window/entry-phase robustness for a prepared
bank. Its reversible bright-section successor gives exact $M_o=|Z_o|^2$
counts on every complete $B$-trial cycle and total-variation discrepancy less
than $B/N$ on an arbitrary $N$-trial window; padding gives one common
$T$-tick completion time. Criterion 11 remains open in general because the
action has not formed the bank/rings, herald latch, counter, amplification,
overlapping-traffic composition, or multipartite no-signalling.

The
[recurrent C4 actualization material clock](../derivations/common_action_mechanics_reciprocity/THEOREM_C4_RECURRENT_ACTUALIZATION_MATERIAL_CLOCK_AND_LOCAL_RATE_BOUNDARY_v1.md)
adds the first localized recurrence. A persistent bright pair drives the same
token through an exact four-admitted-tick cycle with zero net phase-vector and
phase-tensor moments, nonzero mean capacity deficit, and neutral recurrent
ternary activity. Arbitrary capacity permission words exactly separate global
ticks from local admitted ticks. This is a prepared proto-body/clock;
formation, stability, mass, motion, and physical time dilation remain open.

The subsequent
[ternary-square phase/polarity carrier](../derivations/common_action_mechanics_reciprocity/THEOREM_TERNARY_SQUARE_PHASE_POLARITY_CARRIER_AND_AUTONOMOUS_CROSSING_CLOCK_v1.md)
repairs the remaining state-type mismatch. Blank plus four phases and two
manifestation polarities requires exactly nine states, so the full product of
the two already-proposed ternary slots carries the complete token without an
extra orientation register. Charge-conjugation equivariance proves that the
polarity cannot be generated from a fully symmetric blank; it must be retained
incoming history. A selected phase-crossing permutation then gives an exact
controller-free period-eight recurrence with the same mean capacity deficit
as the prepared clock. Reserve formation/routing, non-tautological action
selection, stable binding, blocked kernels, and physical observables remain
open.

The
[C18 tensor-doublet TT reduction](../derivations/gravity_cosmology/THEOREM_C18_TENSOR_DOUBLET_TT_REDUCTION_AND_DYNAMICAL_BOUNDARY_v1.md)
also removes the kinematic ambiguity around spin-2. Four explicit
constraint/gauge pairs reduce the native rank-twelve $(Q,P)$ carrier to two
tensor polarizations and their partners, with a TT projector commuting with
the native C4 complex structure. The parity/curl successor then fixes the
only isotropic first-derivative TT target and the missing odd tensor type. The
action-derived constraint algebra, finite staggered lift, static sector,
universal composite source, and lensing remain open.

---

## Non-goals

This gate does not try to:

```text
derive physical QED alpha
derive electron charge
derive Dirac matter
match Standard Model parameters
```

Those belong to later matching branches. This gate only decides what native
FTD object generates native EFT observables.

The strengthened unification gate does eventually require a native coupling
measurement, but it still forbids using the physical value of alpha to choose
the microscopic action or its observable.
