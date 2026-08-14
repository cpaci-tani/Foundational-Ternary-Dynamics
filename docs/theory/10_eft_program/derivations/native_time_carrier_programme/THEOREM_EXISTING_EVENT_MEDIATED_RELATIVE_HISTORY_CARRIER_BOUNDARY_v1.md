# Existing event-mediated relative-history carrier boundary v1

**Identifiers:** `FTD-0944/0945`  
**Date:** 2026-08-11  
**Status:** `[THEOREM — ORDINARY PRODUCTION PRESERVES THE RELATIVE-ZERO SUBMANIFOLD]` + `[THEOREM — WEAK L/R EXCHANGE IS A LOCAL SYMPLECTIC INVOLUTION]` + `[CLOSED NEGATIVE — CURRENT EVENT STACK DOES NOT REALIZE A REVERSIBLE RELATIVE-HISTORY CARRIER]` + `[DESIGN BOUNDARY — NEW ACTION IN EXISTING FIELDS OR SELECTED PORT TYPE]`  
**Parent protocol:** [`PREREG_EXISTING_EVENT_MEDIATED_RELATIVE_HISTORY_CARRIER_AUDIT_v1.md`](../../preregistrations/native_time_carrier_programme/PREREG_EXISTING_EVENT_MEDIATED_RELATIVE_HISTORY_CARRIER_AUDIT_v1.md), pre-run SHA-256 `9E2EF3C707A798AD73F7DF1280273F2924B9C7D3B337393000C6175E55811B1D`  
**Parent certificate:** `scripts/proofs/proof_existing_event_mediated_relative_history_carrier_audit.py`, SHA-256 `2B7E9AE5427B5EAA680E50433AB343EE4D3315C28081C7832364597FDFAA34B7`, first immutable execution `136/137`, **Outcome D** from one prose-marker mismatch after all source and mathematical gates passed  
**Repair protocol:** [`PREREG_EXISTING_EVENT_MEDIATED_RELATIVE_HISTORY_CARRIER_CERTIFICATE_REPAIR_v2.md`](../../preregistrations/native_time_carrier_programme/PREREG_EXISTING_EVENT_MEDIATED_RELATIVE_HISTORY_CARRIER_CERTIFICATE_REPAIR_v2.md), SHA-256 `2E170186B8D5CBCE61005CCDF9A90715A31A894D576A5FFBCA3F91744B3F612D`  
**Repair certificate:** `scripts/proofs/proof_existing_event_mediated_relative_history_carrier_audit_v2.py`, SHA-256 `CA939C837FDF2700AF5AF7515D6B892FB13CF19EB1BFCC04E1B9A236849C01FE`, inherited `137/137` plus repair integrity `7/7`, **Outcome B**

## 1. Result

The unchanged production event stack does not supply the missing gearbox
inside the existing L/R fields:

> **[THEOREM — RELATIVE-ZERO INVARIANT]** Under the frozen ordinary dual-field
> wave, coupling, injection, Gauss, damping, genesis, pair-production, weak,
> movement, annihilation, and boundary actions, the submanifold
> `D=P_D=0` at every site is invariant. No composition of these actions can
> create a relative record from the ordinary symmetric relative vacuum.

Two useful but insufficient mechanisms survive:

> **[THEOREM — WEAK SURVIVOR]** A fired weak event acts as the exact local
> symplectic involution `(s,D,P_D)->(-s,-D,-P_D)`.

> **[CLASSIFICATION — MOVEMENT SURVIVOR]** Void movement can advect a scalar
> fraction of an already-prepared `D`, while carrying the manifested body's
> mechanical velocity and occupancy. It does not transport `P_D`, does not
> write its displacement into `D`, and is singular on the common production
> branch where the full capped self-field is transferred.

No existing event action simultaneously provides autonomous relative deposit,
a phase-complete inverse, collision-separated history/backpressure, and a
source-energy transaction. The observation journal can reconstruct events
externally but is not ontic production state.

## 2. Exact relative-zero theorem

Define the relative canonical coordinates

\[
D=J_L-J_R,\qquad P_D=P_L-P_R.                                     \tag{1}
\]

Every frozen ordinary dual-field mutation belongs to one of four algebraic
classes:

1. **Equal additive action.** Wave coupling, ordinary injection, and Gauss
   correction add the same value to L and R, so the addition cancels from
   equation (1).
2. **Equal homogeneous action.** Damping, the absorbing sponge, movement
   fractions, and annihilation shell distribution apply the same scalar
   linear rule to L and R, so they act homogeneously on `D`.
3. **Exchange.** Weak transmutation swaps L and R, hence reflects `(D,P_D)`.
4. **No dual-field write.** Dual genesis and the current pair-production
   implementation alter actual states/metadata or observable fields but do
   not write `flux_L/R` or `wave_vel_L/R`.

Each class maps zero relative data to zero. An arbitrary composition of
zero-preserving maps is zero preserving. Therefore

\[
\boxed{D_x=P_{D,x}=0\ \forall x
\quad\Longrightarrow\quad
D_x'=P_{D,x}'=0\ \forall x}                                      \tag{2}
\]

for the registered ordinary event stack.

Equation (2) is a production-source theorem, not a claim that the storage is
unwritable. Selected particle, wavepacket, neutrino, and molecule preparation
paths can seed L/R asymmetry. Those are external initial-condition
constructions. The present event stack does not turn a natural occupancy hop
or genesis event into that seed autonomously.

## 3. Weak exchange is exact but local

When its stress threshold and keyed draw fire, weak transmutation performs

\[
W:(s,J_L,J_R,P_L,P_R)
\mapsto(-s,J_R,J_L,P_R,P_L).                                      \tag{3}
\]

Thus

\[
W:(s,D,P_D)\mapsto(-s,-D,-P_D),
\qquad W^2=I.                                                      \tag{4}
\]

On one relative canonical pair, `W=-I_2`, so

\[
W^T\Omega W=\Omega,qquad \det W=1.                               \tag{5}
\]

This is a legitimate reversible chirality exchange. It remains onsite,
leaves the support fixed, supplies no spatial direction label, and maps zero
relative data to zero. The **event map** is involutive; the full production
trigger schedule is not an inverse algorithm, because firing depends on the
current stress, global tick, site, seed, and keyed random branch.

## 4. Movement carries prepared D but tears the canonical pair

For movement into a void target, production uses the capped self-field
fraction

\[
f={\min(\rho,K_B)\over\rho},\qquad 0<f\le1.                        \tag{6}
\]

Both halves receive the same transfer. For source `i` and target `t`, one
scalar relative component obeys

\[
\begin{pmatrix}D_i'\\D_t'\end{pmatrix}
=A_f\begin{pmatrix}D_i\\D_t\end{pmatrix},
\qquad
A_f=\begin{pmatrix}1-f&0\\f&1\end{pmatrix}.                      \tag{7}
\]

But `phase_movement.cpp` never transfers `wave_vel_L/R`, so

\[
\begin{pmatrix}P_{D,i}'\\P_{D,t}'\end{pmatrix}
=\begin{pmatrix}P_{D,i}\\P_{D,t}\end{pmatrix}.                   \tag{8}
\]

The full scalar phase-space map is `S_f=diag(A_f,I_2)`. It has

\[
\det S_f=1-f.                                                      \tag{9}
\]

Consequently:

- for `f=1`, which occurs whenever `0<rho<=K_B`, the map has rank three and
  is noninjective;
- for every `f!=0`, `S_f^T Omega S_f != Omega`, because the conjugate
  momenta are not transformed by `A_f^{-T}`; and
- `D=0` remains `D'=0`, so the event does not write its displacement into the
  relative field.

An exact `f=1` collision witness is

\[
(D_i,D_t)=(a,b),\qquad
(\widetilde D_i,\widetilde D_t)=(0,a+b),                           \tag{10}
\]

which are distinct for `a!=0` but both map to `(0,a+b)`. Any later
deterministic production map preserves this identification.

The manifested record does carry `velocity`, `remainder`, IDs, spin, color,
and one occupancy crossing to the target. That can describe the present body
motion. It is not an exported cumulative history, and it does not repair the
relative canonical transaction in equations (7)--(8).

## 5. Genesis and pair production do not write the relative sector

Dual genesis reads chirality density, a threshold, and a keyed random draw,
then manifests ternary state and metadata. Its frozen branch contains no
assignment to `flux_L`, `flux_R`, `wave_vel_L`, or `wave_vel_R`. It therefore
can **read** pre-existing relative chirality into an actual sign without
creating the relative datum it reads.

The current pair-production branch chooses the dominant observable-flux axis,
creates an adjacent `-1/+1` pair, and writes observable `flux` and
`wave_vel`. It likewise contains no dual-field write. The six axial direction
choice is a geometric actual-record construction, not an L/R history deposit.
The production source itself correctly states that its selected drain implies
no common-action energy identity.

Evaporation clears state, particle ID, spin, and color while retaining field
data. Distinct manifested label states can therefore share one post-state. It
is an explicit lossy actualization/unactualization action, not a reversible
relative carrier.

## 6. Collision and boundary classification

The remaining branches reinforce rather than evade the boundary:

- **Same-sign bounce** flips selected mechanical velocity components but
  resets the remainder. Distinct pre-remainders can therefore reach the same
  post-state. No L/R datum is written.
- **Annihilation** clears both manifested records and their L/R source flux,
  then distributes L and R over the corresponding six-neighbor shells by the
  same rule. The relative flux distribution is homogeneous and conserves its
  aggregate in the audited linear shell model, but the conjugate relative
  momenta are not distributed and cleared particle labels are unrecoverable.
- **Absorbing crossing** clears manifested state and L/R flux.
- **Reflective crossing** flips mechanical velocity and resets the remainder.
- **Sponge/damping** scales L and R equally and has no direction-specific
  record or carrier-side bath transaction.

These mechanisms may be useful phenomenological responses. None is the
reversible collision-separated source/stream/backpressure transaction of
FTD-0941.

## 7. Why composition and the journal cannot repair it

There are two independent composition no-gos.

First, equation (2) is closed under composition. A chain of operations that
all map relative zero to relative zero cannot create a nonzero relative
record.

Second, if an action `A` is noninjective, so `A(x)=A(y)` for distinct complete
states, then every later deterministic action `B` satisfies

\[
B(A(x))=B(A(y)).                                                    \tag{11}
\]

Later dynamics cannot reconstruct data already identified by a movement,
evaporation, bounce, annihilation, or absorbing branch.

The history journal avoids that conclusion only by retaining before/after
copies outside the production state. Its frozen contract is explicit: it is
an observer, disabled by default, consumes no random numbers, and never writes
lattice, voxel, toggle, or integrator state. It is excellent evidence and not
an ontic repair.

## 8. Parent failure and exact repair

The first immutable FTD-0944 execution reported `136/137`. Every source,
transition, invariant, matrix, witness, outcome, and other scope gate passed.
The only failure was literal:

- parent certificate expected `No new primitive storage type`;
- locked protocol said `not force a new primitive storage type`.

FTD-0945 preserves both parent files and replaces exactly that one string in
memory. The wrapper verifies the parent protocol hash, parent certificate
hash, repair protocol hash, unique old/new occurrence counts, one
substitution, and zero inherited exit. The repaired execution reports
inherited `137/137` and repair integrity `7/7`. No physics or outcome gate was
changed.

## 9. Interpretation

The L/R pair is real substrate hardware, but current production treats its
relative sector as a **prepared orientation variable**, not a self-writing
history variable:

- weak action can reverse it;
- free waves can propagate it globally;
- movement can partially advect its position coordinate; but
- ordinary events cannot create it from symmetric vacuum, and movement does
  not carry its conjugate momentum canonically.

In the user's bilateral analogy, the left/right halves exist and can exchange
roles, but the corpus has not yet supplied the commissural learning rule that
writes a reversible event history across them. This is a mathematical
hardware/software distinction, not evidence that biological hemispheres are
fundamental physics.

## 10. Verification record

The repaired immutable run reported:

```text
FTD-0944 exact certificate: 137/137 checks passed
OUTCOME B — the registered ordinary event stack preserves the
relative-zero submanifold. Weak exchange is an exact local involution
and movement can advect preloaded D, but no existing action supplies
autonomous relative deposit plus phase-complete inverse, collision/
backpressure, and a source-energy transaction.
A new action in existing fields or a separately selected port type is
required; the observation journal cannot repair production dynamics.

FTD-0945 repair integrity: 7/7 checks passed
PARENT_PROTOCOL_AND_CERTIFICATE=PRESERVED
REPAIR_COUNT=EXACTLY_ONE
REPAIR_SCOPE=ONE_PROTOCOL_PROSE_MARKER
MATHEMATICS_SOURCES_OUTCOMES=UNCHANGED
```

No engine or CMake source changed and no numerical search was performed.

## 11. Design consequence and next gate

FTD-0943 and FTD-0944/0945 together exhaust the unchanged isolated-linear and
existing event-mediated routes for an exact reversible relative-history
carrier. The next step must add a declared action or a declared type.

The lower ontic price is to use the existing canonical pair first:

> **[NEXT — PREREGISTER]** Construct the minimum local nonlinear relative-field
> action with a degenerate vacuum, finite-energy protected orientation pulse,
> canonical movement of both `D` and `P_D`, an odd occupancy-event source,
> exact reverse/erasure, collision/backpressure, and a local energy current.

The control must be the existing linear `C18` action, which FTD-0943 proves
cannot pass. If no such minimal same-field action survives, adopt the
FTD-0941 oriented channel/port carrier explicitly. Neither route may be called
derived before its source and action are specified.
