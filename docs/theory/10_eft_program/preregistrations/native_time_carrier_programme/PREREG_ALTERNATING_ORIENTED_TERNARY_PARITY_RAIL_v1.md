# FTD-0874 — Alternating oriented ternary parity rail v1

**Identifier:** `FTD-0874`  
**Status:** `[PRE-REGISTRATION — LOCKED/PRE-RUN]`  
**Date:** 2026-08-11  
**Parents:** `FTD-0852`, `FTD-0856`, `FTD-0862`, `FTD-0872`, `FTD-0873`  
**Production status:** unchanged; isolated exact reference scheduler only

## 1. Registered question

Can the exact oriented ternary quarter-turn

\[
 R(a,b)=(-b,a)
\]

be scheduled on nearest-neighbour bonds by the already-available global tick
parity and cubic-coordinate parity so that a prepared record propagates exactly
one cell per tick, occupied downstream data are exchanged rather than erased,
and the precise reversible one-shot boundary is stated without assuming an
infinite lattice or a production `Voxel` coupling?

This is a finite-horizon structural question. It does not derive a native
material carrier, a congestion-clearing rule, the Hamiltonian coupling between
different sites, a `G*` synchronization, or Born/Bell physics.

## 2. Frozen sources

| Source | SHA-256 |
|---|---|
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_CAUSAL_ODD_PULSE_HISTORY_CARRIER_v1.md` | `7F393F78C2572ED9C61B20D897F3786BB366B305BA831DDB6CAD42344F4131E7` |
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_MINIMUM_RECIPROCAL_RECORD_PORT_BARRIER_v1.md` | `5D13921555B2289ABC5425F4D2436545C4C3BF0638FC71C98A577E3325D215EA` |
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_PHASE_REFERENCED_ACTION_EXPORT_RAIL_v1.md` | `94A75E375B8CB918B04C6D5C8DF5021380E8DA74243490BF1DD954ECBA26E32A` |
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_ORIENTED_TERNARY_QUARTER_TURN_GEARBOX_v1.md` | `898A9130DFBAAE23B76D3FB5339851D026B50E5B7EFFB8B4B8DC66513F5A9317` |
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_HAMILTONIAN_TERNARY_QUARTER_TURN_ACTUATOR_v1.md` | `73214057949BC5BE115AF7E273DE2CECE1F87D63237E94ADADB83F64442C7B98` |
| `engine/include/ftd/eft/oriented_ternary_quarter_turn.h` | `46CD15943F5EB8EDBBCE4676CDE558A7C2B08556E1AC64E7C9720D30FFEB68E1` |
| `engine/include/ftd/eft/hamiltonian_ternary_quarter_turn_actuator.h` | `10BB9BFF5CC98E6CD72EC77F46E67766D458214E474296A7F3023AA27E2F8A94` |

Any mismatch invalidates the run. The certificate may read only these sources
and this protocol. Exact exhaustive enumeration of finite ternary rails is
permitted; numerical near-miss search, coefficient fitting, and target tuning
are forbidden.

## 3. Frozen rail class

For a finite rail

\[
 x=(x_0,\ldots,x_{L-1}),\qquad x_j\in\{-1,0,+1\},
\]

define the matching at global tick `n` by

\[
 \mathcal M_n=\{(j,j+1):0\leq j<L-1,\ j\equiv n\pmod 2\}.       \tag{1}
\]

All bonds in `M_n` are disjoint. The registered layer `U_n` applies

\[
 (x_j,x_{j+1})\mapsto(-x_{j+1},x_j)                           \tag{2}
\]

simultaneously on those bonds and leaves unmatched endpoints fixed. Its exact
inverse applies

\[
 (y_j,y_{j+1})\mapsto(y_{j+1},-y_j)                           \tag{3}
\]

on the same matching. No conditional empty-port branch is allowed.

For each finite horizon `T`, the propagation claim is made only on a retained
segment of length at least `T+1`, before the leading record reaches a boundary.
No thermodynamic or actually-infinite lattice limit is invoked.

## 4. Frozen claims

### 4.1 Exact local layer

Each `U_n` is a product of disjoint nine-state permutations. Therefore it is
bijective, nearest-neighbour local, sign-reversal equivariant, and preserves

\[
 Q(x)=\sum_j x_j^2.                                            \tag{4}
\]

It also preserves the number of nonzero labels. On an occupied bond, (2)
moves the upstream label forward and returns the previous downstream label
upstream with reversed orientation. This is reversible retention, not a proof
of congestion-free progress.

### 4.2 Prepared-pulse propagation

Let

\[
 x_0^{(0)}=s\in\{-1,+1\},\qquad x_j^{(0)}=0\quad(j>0).          \tag{5}
\]

Starting with the even matching at `n=0`, induction on (1)--(2) gives, for
every `0<=t<=T`,

\[
 x_j^{(t)}=s\,\delta_{jt}.                                    \tag{6}
\]

Thus the record retains its sign, clears every site behind it, and advances
exactly one adjacent cell per global tick. Reversing the layers in reverse
tick order recovers the complete history exactly.

One fixed disjoint matching cannot propagate any label beyond its initial
two-site block. Hence two alternating matchings are minimal within the
registered class of translation-periodic, disjoint, nearest-neighbour bond
layers for propagation over more than one edge.

### 4.3 Reversible one-shot boundary

For any injective time-homogeneous state map `F`,

\[
 F(x)=y\ne x\quad\Longrightarrow\quad F(y)\ne y,               \tag{7}
\]

because `F(y)=y=F(x)` would give two distinct preimages of `y`. A finite
bijective system is a disjoint union of cycles, so a distinct predecessor
cannot enter a literal fixed `done` state. A reversible one-shot record must
therefore keep moving, leave continuing controller/history state, or export
information to an environment.

The parity rail takes the first option: the local source clears, while the
global state continues to change as the record moves outward. If a finite
boundary value is discarded, exact reversibility is lost; retained endpoints,
a reciprocal tail port, or a larger finite-horizon segment are required.

### 4.4 Composition with the actuator

The FTD-0873 local actuator can prepare `(s,0)->(0,s)` at a source/port pair.
Identifying that output port with rail site zero and applying the scheduled
bond `(0,1)` yields `(s,0)->(0,s)` on the first spatial edge. This composition
does not move information more than one lattice edge in one global tick. It is
an isolated exact witness; no production tick phase is modified.

## 5. Registered certificate gates

The source-locked certificate must report exactly forty-eight checks.

### Provenance

- **C1--C7:** the seven source hashes match section 2.
- **C8:** this protocol hash matches the pre-run lock embedded in the frozen
  certificate before its first execution.

### Bond and layer algebra

- **C9:** (2) maps every ternary pair to a ternary pair.
- **C10:** (2) is a permutation of all nine ternary pairs.
- **C11:** (3) is its exact inverse.
- **C12:** `R^2=-I` on every ternary pair.
- **C13:** pair label norm is preserved.
- **C14:** pair nonzero-label count is preserved.
- **C15:** the pair map is sign-reversal equivariant.
- **C16:** its matrix determinant is positive one.
- **C17:** each registered parity matching is disjoint.
- **C18:** each tested finite layer is a permutation of its full ternary state
  space.
- **C19:** the registered inverse recovers every tested finite state.
- **C20:** one layer changes a site using only itself and at most one adjacent
  site.
- **C21:** no dependency crosses more than one rail edge per tick.
- **C22:** unmatched finite endpoints remain explicit state rather than being
  dropped.

### Propagation and backpressure

- **C23:** the first even layer moves a prepared pulse from site zero to one.
- **C24:** (6) holds for both signs through every registered finite horizon.
- **C25:** the propagated sign is unchanged.
- **C26:** displacement equals elapsed tick count.
- **C27:** the source site clears after the first layer.
- **C28:** cleared sites behind the pulse remain clear through the horizon.
- **C29:** inverse layers recover the prepared initial state.
- **C30:** a fixed even matching cannot transport the pulse beyond site one.
- **C31:** alternating matchings do transport it beyond site one.
- **C32:** two matchings are minimal in the declared disjoint-bond class.
- **C33:** every occupied downstream pair undergoes exact reciprocal exchange.
- **C34:** occupied exchange retains both labels up to registered orientation.
- **C35:** no ternary pair is erased or hidden by an empty-port condition.
- **C36:** a ready bond transfers exactly and clears its upstream site.
- **C37:** reversible retention does not imply an empty downstream port.
- **C38:** a fully occupied control proves that universal progress/readiness is
  not established.

### Boundary, composition, and scope

- **C39:** finite retained layers are bijective without boundary export.
- **C40:** discarding a post-layer endpoint is noninjective.
- **C41:** the injective fixed-state implication (7) holds.
- **C42:** finite bijections have no transient entry into a fixed point.
- **C43:** outward motion avoids a fixed-done collision while clearing the
  local source.
- **C44:** actuator preparation followed by the first rail bond is exact.
- **C45:** the composed spatial propagation bound remains one edge per tick.
- **C46:** all frozen scope markers below are present.
- **C47:** the protocol keeps finite-horizon and backpressure debts open.
- **C48:** the terminal verdict is emitted only if C1--C47 all pass.

## 6. Frozen interpretation

If all forty-eight gates pass, the permitted result is:

- **[THEOREM, CONDITIONAL]** alternating coordinate/tick parity schedules the
  exact FTD-0872 bond rotation into a causal finite-horizon record rail;
- **[THEOREM]** a prepared isolated ternary record advances exactly one cell
  per tick and is exactly recoverable;
- **[THEOREM]** occupied sites are exchanged without erasure, but readiness
  and universal progress do not follow;
- **[THEOREM]** a closed injective time-homogeneous map cannot perform a
  nontrivial transition into a literal fixed state;
- **[SELECTION]** the alternating bond schedule is a reference architecture
  using existing global tick and coordinate parity; it consumes the already
  booked `SEL-CA-PHASE-RAIL` type and adds no new selected type;
- **[OPEN]** native intersite Hamiltonian formation, collision/backpressure
  resolution, finite-boundary completion, multidimensional routing,
  production coupling, robustness, and separate `G*` synchronization.

## 7. Frozen outcome rule

- **Outcome A:** `48/48`; book the scoped theorem and an isolated `ftd::eft`
  witness.
- **Outcome B:** provenance passes but any mathematical gate fails; book the
  counterexample and no theorem.
- **Execution invalid:** any hash mismatch, exception, wrong check count, or
  scope-marker failure; preserve the run and preregister any repair.

No post-run rule change, horizon change, source substitution, or scope
promotion is permitted.

## 8. Scope markers

```text
PARITY_RAIL_STATUS=SELECTED_REFERENCE_EXISTING_TYPE
GLOBAL_TICK_ROLE=EXISTING_INTEGER_PARITY_SCHEDULER
FINITE_HORIZON_ONLY=TRUE
BACKPRESSURE_PROGRESS=OPEN
PRODUCTION_COUPLING=NONE
GSTAR_ROLE=SEPARATE_CALENDAR_NOT_RAIL
BORN_BELL_STATUS=UNTOUCHED
```

## 9. Pre-run lock

The exact SHA-256 of this byte-frozen protocol must be embedded in the
certificate and recorded in the preregistration manifest before first
execution. Later outcome prose must not alter this evidence hash.
