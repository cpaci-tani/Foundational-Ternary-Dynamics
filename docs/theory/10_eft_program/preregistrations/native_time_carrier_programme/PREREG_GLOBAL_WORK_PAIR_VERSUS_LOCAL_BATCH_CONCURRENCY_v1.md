# Pre-registration — Global work pair versus local batch concurrency v1

**Identifier:** `FTD-0983`  
**Date locked:** 2026-08-12  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE FIRST EXECUTION]`  
**Expected classifier:** `Outcome B`

## 1. Question

FTD-0982 proves that one complete phase/action pair is the minimum exact
canonical work port for one independently gated batch. Can the single
physical clock pair from the FTD-0977 reference law also serve arbitrarily
many pairwise spacelike local events in one tick, or would that make it a
nonlocal energy bus or double-spend the reserve?

The gate must distinguish:

1. **aggregate symplectic sufficiency:** whether one global pair can book the
   sum of many event works as pure mathematics;
2. **substrate locality:** whether a work update may depend in one tick on
   state outside the output carrier's Moore causal neighbourhood;
3. **phase completeness:** whether several locally owned actions can share
   one phase without a degenerate two-form; and
4. **concurrent reserve safety:** whether independently admitted local events
   can spend the same global reserve without an atomic nonlocal gate.

No production change or global-selector energy role is adopted by this test.

## 2. Frozen sources

| Source | Frozen SHA-256 |
|---|---|
| `THEOREM_LOCAL_CANONICAL_WORK_PORT_AND_C18_FACTOR_EVENT_BOUNDARY_v1.md` | `3BF425E7F826844BDD1F87ACA3B57EE9A26704996CC8A6F7781C683477D3B994` |
| `THEOREM_ONE_CLOCK_C4_COTANGENT_LIFT_AND_CONNECTION_UNDERDETERMINATION_v1.md` | `9D80C133F5D99D0F789C320DC7C2C2A9E41C4DBB56FAECD39054B7BF0DB69E7F` |
| `THEOREM_PRODUCTION_PHASE_CONNECTION_REPRESENTABILITY_AND_CUBIC_CHART_BOUNDARY_v1.md` | `FF80023FA73326B439405C8A07F08A72A5EBD8CC845AC145224B5BE4D647F07C` |
| `THEOREM_PRODUCTION_CLOCK_INDEXED_C4_TWIST_CENSUS_v1.md` | `3873CEE3BD61C894A99857C0527FBC1082F244CE7E7890FEB3E2F01C6D64E58F` |

FTD-0982 supplies the one-batch lift. FTD-0977 supplies the conditional
single-clock cotangent law. FTD-0965 supplies the six-pair capacity and cubic
scalar-chart obstruction. FTD-0978 supplies the production facts that phase
is diagnostic, CUDA has no phase buffer, and the ledger has no work reserve.

## 3. Aggregate one-pair lift

Let disjoint batch phase spaces be `(Z_a,Omega_a)`, with clock-dependent
symplectic maps `R_a(theta)` and

\[
 B_a(\theta)=R_a(\theta)^T\Omega_a\partial_\theta R_a(\theta).
                                                               \tag{1}
\]

Each `B_a` is symmetric. On the direct sum plus one global pair
`(theta,I_G)`, register

\[
 z_a'=R_a(\theta)z_a,
\qquad \theta'=\theta,
\qquad
 I_G'=I_G+\sum_a{1\over2}z_a^TB_a(\theta)z_a.           \tag{2}
\]

The certificate must prove that (2) preserves

\[
 \bigoplus_a\Omega_a+d\theta\wedge dI_G.                \tag{3}
\]

At simultaneous seam crossings, if each summand is the FTD-0982 event,

\[
 I_G'=I_G+\sum_a[H_a(z_a)-H_a(z_a')],                   \tag{4}
\]

so aggregate field-plus-port energy is exact. This establishes mathematical
sufficiency only.

## 4. Moore-locality discriminator

Use the project locality contract: a radius-`r` one-tick output at site `x`
may depend only on input in `N_r(x)`. If the single action `I_G` is stored at
site `o`, while a nonconstant remote batch work `w_a(z_a)` is supported in
`Lambda_a` with `dist(o,Lambda_a)>r`, equation (2) gives

\[
 {\partial I_G'\over\partial z_a}=\nabla w_a\ne0.        \tag{5}
\]

Equation (5) violates the one-tick Jacobian support condition. Calling
`I_G` a global variable accessible everywhere avoids a storage location only
by adopting an explicitly nonlocal primitive. A contextual selector may
choose a joint outcome globally, but it is not thereby a physical substrate
energy reservoir.

A centralized reserve can influence a batch at graph distance `d` only after
at least

\[
 \left\lceil d/r\right\rceil                              \tag{6}
\]

ticks. A same-tick local event therefore requires reserve already
prepositioned inside its causal neighbourhood.

## 5. One phase with many actions is presymplectic

Trying to keep local reserves `I_1,...,I_N` while sharing only one phase gives

\[
 \omega_{\rm shared}=d\theta\wedge d\!\left(\sum_aI_a\right).
                                                               \tag{7}
\]

For `N>1`, every relative-action vector

\[
 \partial_{I_a}-\partial_{I_b}                            \tag{8}
\]

lies in the kernel of (7). Thus the local reserve differences are not
phase-complete canonical degrees of freedom.

Starting with local pairs and imposing `theta_a=theta` has the same pullback
form (7). To retain independently physical `I_a`, the system must retain
their conjugate relative phases, or else reduce the relative actions as
gauge/constraints and keep only the aggregate action.

## 6. Minimum local concurrent lift

For pairwise causally separated ready batches, give each local work owner a
complete pair `(theta_a,I_a)` and apply

\[
 z_a'=R_a(\theta_a)z_a,
\quad \theta_a'=\theta_a,
\quad I_a'=I_a+{1\over2}z_a^TB_a(\theta_a)z_a.          \tag{9}
\]

The product map must preserve

\[
 \bigoplus_a(\Omega_a+d\theta_a\wedge dI_a),            \tag{10}
\]

be order independent for disjoint supports, preserve each local
`H_a+I_a`, and preserve the total sum. This is equivalent to a local
canonical work-port field; it need not be interpreted as permanently one
new species per event.

For `N` pairwise `2r`-separated same-tick batches with independent nonzero
work, their future cones are disjoint. Local reversibility and local energy
ownership require at least one phase-complete work carrier in each cone.
This is the registered minimum statement. Overlapping events must instead be
compiled as one joint batch or scheduled in non-overlapping sublayers; they
may not independently debit the same field energy.

## 7. Double-spend gate

Let one global reserve have `I_G=1` and two separated events each require
`3/4`. Each event passes an independent test against `I_G`, but their joint
demand is `3/2>1`. Preventing this requires an atomic aggregate admission
decision that reads every demand, which is nonlocal at one tick, or separate
prepositioned local reserves.

The certificate must distinguish this admission problem from the additive
symplectic identity (2): algebraic addition is well defined even when the
positive reserve domain is violated.

## 8. Production-capacity audit

The frozen sources must retain all of the following:

- production has six scalar field pairs per site only after choosing a frame;
- the selected five-pair connection chart leaves one complete pair unused;
- no site-local linear cubic-covariant scalar chart exists on the two raw
  vectors;
- host `phase` and `tau` are accumulators without conjugate action;
- CUDA has no phase buffer; and
- the production ledger has no work reserve, connection reaction, or
  backpressure transaction.

Therefore current storage gives a **conditional capacity witness** for one
local port after a selected frame, not a native globally shared or locally
formed work field.

## 9. Frozen checks

- **G1:** protocol/source hashes and scope markers;
- **G2:** exact two-batch aggregate symplectic lift and energy sum;
- **G3:** shared-phase/many-action kernel and synchronized-pair pullback;
- **G4:** exact product local lift, local energy, inverse, and order
  independence;
- **G5:** radius-local Jacobian obstruction and causal-delay bound;
- **G6:** exact positive-reserve double-spend witness;
- **G7:** production capacity/chart/diagnostic/ledger source audit;
- **G8:** minimum one local pair per disjoint same-tick work cone, or an
  equivalent canonical work field;
- **G9:** no selector-energy, `G*`, Born/Bell, Hilbert, mass, or completeness
  promotion and no engine mutation.

No numerical search, fit, near-miss comparison, or production mutation is
permitted.

## 10. Frozen classifier

- **Outcome A — one global pair is substrate-local:** one existing clock pair
  services arbitrarily separated simultaneous events with exact positivity,
  phase completeness, Moore locality, and no atomic nonlocal admission.
- **Outcome B — aggregate pair / local ownership boundary:** one global pair
  closes aggregate symplectic bookkeeping but is nonlocal as a same-tick
  physical energy bus; independently owned local reserves require complete
  local pairs or an equivalent canonical work field, and present production
  supplies only conditional fixed-frame capacity.
- **Outcome C — no coherent concurrency law:** neither the aggregate nor the
  local product lift is canonical and energy consistent.
- **Outcome D — invalid:** a hash, identity, locality, reserve, source, or
  scope gate fails.

The expected result is Outcome B. It does not itself adopt the local work
field or prove its formation, synchronization, replenishment, stability, or
operational hiding.
