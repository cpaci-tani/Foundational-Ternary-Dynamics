# SPEC — Minimum Requirements for a Substrate-Native Clock v1

**Status:** `[DEFINITION + SYNTHESIS]` + `[THEOREM — ORIENTED DISCRETE
PHASE TEST]` + `[CORRECTION — HARMONIC DOES NOT MEAN CLOCKLESS]` +
`[OPEN — LOCAL AUTONOMOUS CARRIER AND NATIVE CRITICAL QUARTICITY]`  
**Date:** 2026-08-10  
**Purpose:** separate the minimum logic of a clock from the stronger
requirements of a local physical clock, a native `G*` clock, and a completed
measurement campaign  
**Parents:** FTD-0659, FTD-0663, FTD-0676, FTD-0772, FTD-0780, FTD-0804/0805,
FTD-0826, FTD-0827, `SPEC_CARRIER_CONSTRAINTS_v1.md`  
**Certificate:**
[`proof_substrate_native_clock_minimum.py`](../../../../scripts/proofs/proof_substrate_native_clock_minimum.py)
— `13/13` exact symbolic checks  
**Production impact:** none

## 0. Result

The framework already contains **clock algebra** but has not yet exhibited a
qualified **local clock body**.

- The source-free production `(J,W)` map has an exact target-blind phase,
  positive action, signed orientation, and radians-per-primitive-tick rate
  for every nonzero elliptic C18 mode (FTD-0826). It is therefore a
  substrate-native **modal clock**.
- A Fourier mode is not a bounded local object. The first localized matter
  doublet carries a clean phase, but its bare action drains into the coupled
  field and its rate lies inside the radiation band (FTD-0659/0663/0676). It
  is a finite-lived resonance, not an autonomous local clock.
- Harmonicity excludes that doublet only as a **critical-quartic `G*`
  clock**. It does not logically exclude it from being a clock. A harmonic
  oscillator has a perfectly good phase even though its rate does not encode
  its action.
- The selected critical quartic Hamiltonian is an ideal `G*` clock and its
  exact CM gearbox is closed by FTD-0827. What remains open is to make that
  critical quartic dynamics native, local, persistent, and energetically
  closed.

This gives a strict ladder:

```text
oriented recurrent phase
        ↓
substrate-native modal clock                 [established]
        ↓ + bounded local carrier/closure
substrate-native local physical clock        [open]
        ↓ + native critical quartic normal form
substrate-native G* clock                    [open]
        ↓ + maintained gate/operational hiding
actualization-clock role                     [open]
```

The twelve constraints in `SPEC_CARRIER_CONSTRAINTS_v1.md` remain the full
`G*` carrier acceptance programme. They are not all logically necessary to
call a dynamical system a clock.

## 1. Minimum mathematical clock

Let `F:X→X` be one deterministic substrate update and let `Gamma` be a
forward-invariant set. An **ideal discrete clock** on `Gamma` is a pair of
real state observables `(Q,P)`, or equivalently `Z=Q+iP`, for which the
following hold.

### M1. Nontrivial phase

There is a state-functional phase

\[
 \phi(z)=\arg Z(z)\in S^1
\]

and at least two operationally distinguishable phase values. A global tick
label attached after the update is not a clock state.

### M2. Oriented progress

On the clock sector,

\[
 Z(Fz)=e^{-i\theta(z)}Z(z),
 \qquad 0<\theta_-\le\theta(z)\le\theta_+<\pi .
 \tag{1}
\]

The ordered phase current

\[
 \chi_F(z):=\operatorname{Im}\!\left(\overline{Z(z)}Z(Fz)\right)
 \tag{2}
\]

then has a fixed nonzero sign. This is the minimum datum that distinguishes
the two orientations. A two-state flip with `theta=pi` does not carry this
clockwise/counterclockwise information.

For a rigid rotation, (2) is

\[
 \chi_F=-|Z|^2\sin\theta<0.
 \tag{3}
\]

The reverse rotation has the opposite sign. Thus the substrate can retain
orientation whenever the state exposes an ordered canonical pair or an
equivalent antisymmetric two-form. State-only energies, covariance matrices,
Green functions, and symmetric-square observables need not retain it.

### M3. Repeated gates

For a fixed preregistered phase `phi_g` and readout resolution `epsilon_g`,
let `G_epsilon` be the oriented gate section

\[
 G_\epsilon=\{z:d_{S^1}(\phi(z),\phi_g)<\epsilon_g,
 \ \chi_F(z)<0\}.
\]

The entry times

\[
 n_0<n_1<n_2<\cdots,
 \qquad F^{n_k}z\in G_\epsilon
 \tag{4}
\]

exist over the declared operating horizon and have positive bounded gaps.
An exactly periodic clock may take `epsilon_g=0` with an equality section.
Exact periodicity is sufficient but not necessary: an irrational rigid
rotation is still a clock because its phase progresses and returns to every
nonempty phase window. A single transient return is not a clock.

### M4. A declared rate and error

The clock reports phase per ontic update,

\[
 \Omega_n=\phi(F^{n+1}z)-\phi(F^nz)\pmod{2\pi}.
\]

An ideal clock has constant `Omega`; a physical clock must declare a bound on
accumulated phase error over a finite horizon. This fixes radians per
primitive tick. It does not fix seconds per tick.

M1--M4 are the logical minimum. Conservation, nonlinearity, amplitude
dependence, localization, and `G*` are not part of the definition of a clock.

## 2. Exact correction: a harmonic oscillator is a clock

For

\[
 H=\frac{p^2}{2m}+\frac{kq^2}{2},
 \qquad m,k>0,
\]

the canonical phase advances at `Omega=sqrt(k/m)`, and the action
`I=H/Omega` is conserved. It obeys M1--M4. Yet

\[
 \frac{d\Omega}{dI}=0.
\]

The last equality says that the **rate cannot measure action**. It does not
say that the phase cannot measure elapsed time. Therefore FTD-0780's sentence
that a harmonic mode “cannot serve as a clock that distinguishes states” is
too strong. The supported conclusion is narrower:

> The FTD-0659 doublet is not a quartic/action-sensitive `G*` clock in the
> measured regime. Its status as a finite-lived ordinary phase clock is
> limited instead by action leakage, band embedding, and lack of autonomous
> closure.

This correction changes no measured value and promotes no native carrier.

## 3. Minimum substrate-native local physical clock

An object satisfying M1--M4 becomes a **substrate-native local physical
clock** only if it also satisfies N1--N5.

### N1. Native dynamical licensing

Its state, preparation, evolution, and readout use already-declared substrate
variables and the frozen production or explicitly named selected common
action. No desired phase, target period, `G*`, external oscillator, or
outcome-dependent controller may be read to generate the motion.

This clause must report two separate statuses:

- `production-native`: fixed by P1--P5 plus the frozen production map;
- `selection-scoped native`: native only after a declared selected common
  action or interaction type is supplied.

The statuses must not be conflated.

### N2. Bounded body-relative support and local readout

There is a finite or uniformly bounded body-following region `Lambda_n` from
which `(Q,P)` can be reconstructed using data in a declared causal
neighbourhood. A box-wide Fourier projection is a valid mathematical mode but
not a local clock body.

### N3. Autonomous recurrence and persistence

After preparation, the clock supplies at least `K_min` registered gates
without an externally prescribed phase drive. The campaign must state
`K_min`, the amplitude/action tolerance, and the allowed support drift. A
decaying resonance may be reported as a metastable clock over its measured
horizon, but not as an autonomous maintained clock.

### N4. Energy and work closure

The carrier's energy/action ledger must include matter, field dressing,
radiation, controller work, dissipation, and any genesis/evaporation drain.
If maintenance is required, the controller may stabilize the clock only from
context-blind local clock variables, and its work is part of the clock.

### N5. Reproducible rate and orientation

The rate and the sign of (2) must survive the registered changes of amplitude,
orientation, translation, volume, and held-out perturbation within declared
tolerances. The local readout must retain an antisymmetric phase datum. A
symmetric-square statistic alone cannot qualify.

These five clauses are minimal physical requirements. Spectrum avoidance,
long campaign telemetry, look-elsewhere controls, and calibrated synthetic
instruments remain necessary for strong evidence, but belong to the
acceptance protocol rather than the logical definition.

## 4. Why BCC loses a datum the substrate retains

Let `R_+` and `R_-` be the normalized inert-prime quarter turns,

\[
 R_+=\begin{pmatrix}0&-1\\1&0\end{pmatrix},
 \qquad R_-=-R_+.
\]

They have opposite oriented rank-two lifts. But

\[
 \operatorname{Sym}^2(R_-)
 =\operatorname{Sym}^2(-R_+)
 =\operatorname{Sym}^2(R_+).
 \tag{5}
\]

The BCC symmetric square therefore cannot choose the lift. This is not a
general no-go for the substrate. The production phase/action pair of FTD-0826
has

\[
 Z_{n+1}=e^{-i\theta_{18}(k)}Z_n,
 \qquad
 \chi_n=-|Z_n|^2\sin\theta_{18}(k)<0,
 \]

so the ordered `(J,W)` state retains the missing sign. The loss occurs when
one projects to the BCC symmetric-square period object, not in the primitive
dynamics.

## 5. Additional minimum for the critical-quartic `G*` branch

Within the declared one-natural-coordinate Hamiltonian programme, a native
local clock becomes a **critical-quartic `G*` clock** only if G1--G5 also
hold.

### G1. Natural coordinate and positive mass

On a state-independent support there is a closing pair `(Q,P)` with

\[
 H_{\rm eff}=\frac{P^2}{2m}+V(Q),\qquad m>0,
\]

up to a declared error over the operating window. An activity-selected
coordinate or nonclosing projection does not qualify.

### G2. Native criticality

Reflection symmetry and the small-amplitude normal form must give

\[
 V(Q)=\lambda Q^4+O(Q^6),
 \qquad V''(0)=0,quad \lambda>0.
 \tag{6}
\]

The zero quadratic coefficient must be forced by the native structure or
maintained by a context-blind feedback law whose work and detuning are
recorded. Manually setting `mu=0` is an imposed reference clock.

### G3. Quartic purity over a declared window

Equation (6) yields the `G*` law only asymptotically. An exact finite-amplitude
identity requires `V(Q)=lambda Q^4` over the full orbit. Otherwise the
campaign must bound the `O(Q^6)` period correction and show convergence as
`A→0` without reading the target.

For the pure quartic orbit,

\[
 T A=\sqrt\pi\,G^*\sqrt{\frac{m}{2\lambda}},
 \qquad
 \widehat G:=\frac{TA}{\sqrt\pi}\sqrt{\frac{2\lambda}{m}}=G^*.
 \tag{7}
\]

### G4. Oriented CM gearbox

The energy shell and its time differential must carry the orientation, not
only an even period statistic. FTD-0827 supplies this condition once G1--G3
hold:

\[
 y^2=1-x^4
 \longrightarrow
 v^2=u^3-u,qquad
 (u,v)=(x^{-2},-yx^{-3}),qquad
 \frac{du}{2v}=\frac{dx}{y}.
\]

No additional prime-by-prime tick rule follows. Primes are arithmetic places,
not successive clock readings.

### G5. Gate separation

`G*` may determine the phase cadence at which an event is eligible. It may
not determine the event outcome. Clock maintenance, gate eligibility, and
the contextual selector remain three separately audited mechanisms.

## 6. Candidate scorecard

| candidate | M1--M4 | N1 native | N2 local body | N3/N4 autonomous closure | G1--G5 | present verdict |
|---|---:|---:|---:|---:|---:|---|
| production C18 eigenmode `(J,W)` | pass exact | production-native conditional on selected `C_WAVE` | fail: Fourier support | pass as an ideal global mode | fail: harmonic/no CM identification | native modal clock, not local hardware |
| FTD-0659 matter doublet | phase pass over measured horizon | selection-scoped | pass | fail: band-embedded action leakage | fail: harmonic | metastable local phase resonance |
| FTD-0774 complete tangent + FTD-0829--0834 ladder | unadjudicated | selected connected branch | candidate bounded body | open | not tested | FTD-0832 repairs the zero-component chart norm and completes Krylov, but independent replay is `94/95` at an ill-conditioned sign-angle metric cross-check; no physics verdict |
| selected pure quartic Hamiltonian | pass exact | fail | model-local | pass only in isolated reference model | pass conditional, FTD-0827 | ideal `G*` clock, not native |
| FTD-0836 bilateral signed-energy representation | pass exact conditional | fail: coordinate theorem only | none | radial repair imposed; bath ledger algebraic | pass conditional, `17/17` | exact self-dual energy-circle and `G*` traversal interpretation; not hardware |
| FTD-0838 production-core obstruction/minimum repair | pass only after adopting `+/-J` | exact negative: current L/R block diagonal | none | exact negative for damping; conditional radial/bath repair | gearbox still open, `22/22` | current core lacks quarter-turn, smooth quartic restorer, and positive shell; requires oriented pair + pair closure + bath + cadence |
| FTD-0839 `i`/Gamma/quartic-square split | pass for selected `J`; `J^2` alone does not choose direction | no production realization tested | none | square gives quartic energy kinematically; no bath | conditional chiral determinant only, `24/24` | `G*` needs twist + polarization + origin + scale + order + multiplicity; square erases orientation; retain `(U,chi)` or unsquared lift |
| FTD-0840 signed pair-energy recursion | pass exact: strict orientation on every nonzero discrete step | modal `(q,p)` is source-native; pair coupling is absent | fail: modal/Fourier support only | pass exact for isolated conservative recursion; no bath needed | continuum law exact; finite-tick cadence open, `24/24` + CTest `1/1` | smallest conditional recursion is unique, reversible, energy-closed, and bounded; isolated header implemented; localization, maintenance, native `lambda u^2`, and global-tick gate remain open |
| FTD-0841 local flux self-pair tensor | pass exact: vector swept-area orientation | local `(J,W)` type is production-native; radial quartic coupling is selected/absent | fail: one voxel is a local degree of freedom, not a bounded clock body | pass exact for isolated onsite recursion; coupled spatial energy closure open | exact only on invariant linearly polarized sectors; generic `L!=0` is not pure quartic, `26/26` | local canonical type closed; cubic symmetry does not force radial quarticity; production coupling, support, polarization, maintenance, and cadence open |
| FTD-0842 coupled edge/onsite closure | global internal orientation conserved; local clock orientation not closed | selected map, not production kick--drift; exact solve globally dependent | fail exactly: positive edge kernel is spatially constant | exact global energy closure, not P4-local one-tick accounting | bounded profiles have quadratic stiffness; no exact critical `G*`, `26/26` | simplest positive single-field architecture obstructed; needs local transactions/multi-tick solve plus bounded zero/soft relative mode |
| FTD-0844 common/relative carrier (FTD-0843 invalid parent) | pass on polarized relative site | selected rank-one `b=a`; production is `b=0` | pass exact: one relative site | pass as sum of common tick and relative quartic ledgers; no common action yet | continuum exact; finite-tick cadence open, repaired `28/28` | first positive P4-local compact selected carrier; formation, readout/backreaction, production coupling, and cadence open |
| two-scale four-chain MVC | pass conditional | fail by purchased range-3 species | bounded | spectrum/drain/persistence open | quartic pass conditional | cheapest explicit nonnative carrier |
| maintained quartic reference clock | pass | fail: imposed controller/model | bounded model | work audit partly implemented | conditional | reference/control system only |
| native unit-strut tensegrity | open | candidate native class | candidate bounded | open | open | narrowest live `G*` hardware target |

## 7. Exact native quartic realization criterion for the live class

For a zero-tension central-force framework, let `R` be the rigidity matrix,
`q` a nontrivial first-order flex, and `kappa(q)` its second-order bond
extension. After minimizing the second-order correction `w`, the quartic
coefficient is

\[
 \lambda(q)=\frac12\min_w
 \left\|K^{1/2}\bigl(\kappa(q)+Rw\bigr)\right\|^2.
 \tag{8}
\]

The live carrier passes the quartic gate exactly iff

\[
 Rq=0,qquad
 \lambda(q)>0
 \quad\hbox{for every non-rigid }q\in\ker R.
 \tag{9}
\]

Equivalently, the weighted projection of `kappa(q)` into `coker(R)` is
nonzero on the entire nontrivial flex sphere. No self-stress gives
`lambda=0`; actual pre-tension restores a quadratic term. Because long
compressed chains can buckle and escape (9), the narrowest remaining native
class is the one already isolated by FTD-0804/0805:

> a finite mixed unit-strut tensegrity with single-bond compression struts,
> straight integer-span tension chains, all polarity/support/capacity margins
> satisfied, and a blocking form positive definite on the full flex space.

This is a search specification, not an existence claim. The next campaign
must be preregistered and use exact, symbolic, or interval certificates over a
declared realization class. Another numerical near-miss scan cannot close it.

## 8. Immediate execution order

1. **Ordinary native local clock:** FTD-0829--0834 now replace the incomplete
   FTD-0774 corpus with a complete chart/Krylov corpus but no certified physics
   verdict. FTD-0832 fixes the singular denominator and reaches one eligible,
   zero qualified cluster; the independent replay fails one principal-angle
   metric-row equality at `94/95`. Do not fit that tolerance after seeing the
   result. Either preregister a stable coincident-subspace certificate without
   using the observed discrepancy, or construct an independent bounded native
   carrier. Only a constructive certified result may proceed to volume
   localization, nonlinear continuation, recurrence, and phase-current
   orientation. See
   [`ANALYSIS_L17_COMPLETE_TANGENT_CERTIFICATE_LADDER_v1.md`](../derivations/native_time_carrier_programme/ANALYSIS_L17_COMPLETE_TANGENT_CERTIFICATE_LADDER_v1.md).
2. **Native `G*` clock:** preregister the exact unit-strut-tensegrity decision
   (9), including all flex directions and buckling escapes. Do not require
   `G*` during the search; first find native critical quarticity.

   > **Status note (2026-08-13, handoff, not shelved): step 2 was never run.**
   > The programme proceeded directly to step 3 below without gating on it,
   > and produced the FTD-0899–0999 chain instead — none of which attacks
   > this question. Full handoff (what's been tried, what's genuinely open,
   > exact next steps): `temporal_interior_programme/INDEX.md` §6, "Native
   > C3" entry.
3. **Gearbox:** only after step 2 passes, invoke FTD-0836, FTD-0838, FTD-0839,
   FTD-0840, FTD-0841, FTD-0842, FTD-0844, and
   FTD-0827. FTD-0836 rewrites the quartic shell as an oriented self-dual
   energy circle and derives `G*` as its nonuniform traversal weight. FTD-0838
   proves that the present production L/R core does not supply the required
   quarter-turn, pair closure, positive bath, or cadence map, and gives the
   degree-minimum conditional radial repair. FTD-0839 proves that the unsquared
   oriented lift must be retained alongside the squared quartic carrier: the
   full-line determinant ratio is `1`, and both squared quarter sectors become
   one half-twist. It also isolates the additional domain debts—chiral
   polarization, origin, spectral unit, operator order, and multiplicity.
   FTD-0840 retains the unsquared lift, adopts `u=q|q|` and `lambda u^2`, and
   closes a globally unique, reversible, exactly conservative, strictly
   oriented recursion without a bath. This removes the bath from the minimum
   isolated-recursion type list, but not from dissipative maintenance. Its
   finite-step series differs from exact quartic flow, so it does not close
   the global-tick cadence map.
   FTD-0841 lifts that recursion to the production-local canonical type:
   `U=J otimes J` gives `||U||_F^2=|J|^4`, and the vector discrete gradient
   preserves energy, angular momentum, reversal, and orientation exactly.
   This identifies the smallest onsite interaction, but it also proves that
   only the invariant linearly polarized sectors inherit the scalar `G*`
   period. A native mechanism must still select or stabilize such a sector
   and close the onsite energy together with spatial propagation.
   FTD-0842 performs that combined closure exactly and proves why it is not
   yet ontic hardware: the simultaneous solve is globally dependent, and
   positive edge energy makes every bounded profile quadratically stiff. The
   next candidate must therefore introduce P4-local accounting and a bounded
   zero/soft relative mode without reading the `G*` target.
   FTD-0844 constructs exactly that selected witness: common propagation plus
   an onsite relative quartic makes the sector ledger local and closes compact
   support. It does not derive the rank-one cross-gradient, formation, or
   readout. The next gate is energy-closed common--relative phase readout.
   FTD-0827 supplies the conductor-32 CM map. Promotion requires a native rule
   that realizes all missing interfaces without reading the quartic/`G*` target; only then
   do the conditional coordinate and CM theorems become properties of native
   clock hardware rather than selected identifications.
4. **Maintenance and measurement:** book controller work, dissipation,
   detuning, phase error, and context blindness before using a gate in the
   Born/actualization programme.

## 9. Falsifiers

- No local clock: every bounded native phase carrier disperses or loses phase
  before the declared minimum gate count, with no energy-closed maintenance
  law.
- No native quartic clock in the registered central-force class: an exact or
  interval-complete class decision proves (9) fails for every admissible
  realization.
- No `G*` promotion: the measured normalized period (7) requires target-coded
  parameters or fails held-out amplitudes after higher-order corrections are
  bounded.
- No actualization role: the clock or its controller depends on measurement
  settings/outcomes, or the preferred gate permits controllable signalling.

The present status is: **native oriented modal algebra and an exact
conditional conservative pair recursion established; local autonomous clock
hardware open; native pair coupling and finite-tick `G*` hardware open.**
