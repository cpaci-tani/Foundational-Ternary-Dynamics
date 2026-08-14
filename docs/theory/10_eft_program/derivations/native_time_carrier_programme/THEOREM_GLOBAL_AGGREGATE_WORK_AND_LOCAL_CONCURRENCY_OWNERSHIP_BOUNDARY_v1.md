# Theorem — Global aggregate work and local concurrency ownership boundary v1

**Identifier:** `FTD-0985`  
**Date:** 2026-08-12  
**Status:** `[THEOREM — ONE-PAIR AGGREGATE SYMPLECTIC SUFFICIENCY]` +
`[THEOREM — SAME-TICK GLOBAL-RESERVE MOORE-LOCALITY OBSTRUCTION]` +
`[THEOREM — SHARED-PHASE/MANY-ACTION PRESYMPLECTIC BOUNDARY]` +
`[THEOREM — MINIMUM LOCAL PHASE-COMPLETE WORK OWNERSHIP]` +
`[BOUNDARY — CENTRAL ADMISSION DELAY / DOUBLE SPEND]` +
`[CONDITIONAL CAPACITY — UNUSED SIXTH PRODUCTION PAIR]` +
`[OPEN — NATIVE LOCAL PORT FORMATION / SYNCHRONIZATION / REPLENISHMENT]`

## Result

One global clock/action pair is mathematically sufficient to book the sum of
arbitrarily many clock-dependent symplectic events. It is not a same-tick
Moore-local physical energy bus for spatially separated events.

The substrate-local law is instead:

\[
 \boxed{
 z_a'=R_a(\theta_a)z_a,\qquad
 \theta_a'=\theta_a,\qquad
 I_a'=I_a+w_a,
 \quad
 w_a={1\over2}z_a^TB_a(\theta_a)z_a .}
                                                               \tag{1}
\]

Here

\[
 B_a(\theta_a)=R_a(\theta_a)^T\Omega_a
                 \partial_{\theta_a}R_a(\theta_a)              \tag{2}
\]

is symmetric. At the registered FTD-0982 seam,

\[
 w_a=H_a(z_a)-H_a(z_a'),\qquad
 H_a(z_a')+I_a'=H_a(z_a)+I_a.                                  \tag{3}
\]

For pairwise causally disjoint same-tick work events, each work-owning cone
must contain one complete local pair `(theta_a,I_a)`, or an equivalent local
canonical work-port field. The global time order `n` may synchronize
eligibility. It does not thereby transport work or supply reserve.

This theorem changes no production engine state and does not derive the
native identity of the local work pair.

## Certificate of record

- Parent protocol:
  [`PREREG_GLOBAL_WORK_PAIR_VERSUS_LOCAL_BATCH_CONCURRENCY_v1.md`](../../preregistrations/native_time_carrier_programme/PREREG_GLOBAL_WORK_PAIR_VERSUS_LOCAL_BATCH_CONCURRENCY_v1.md),
  SHA-256 `4D47C48793A591A54168B4A24EFFBB537EA8F11F6F226C0B52049A3E7CBD8C6C`.
- Immutable parent proof:
  [`proof_global_work_pair_local_batch_concurrency.py`](../../../../../scripts/proofs/proof_global_work_pair_local_batch_concurrency.py),
  SHA-256 `E985B8EE6952AC494963F0B7DD1A4BD81FEBBD8D72881BCDA8E4C6D8DC733F0D`.
  First execution stopped after all reached checks passed because two local
  energy predicates called nonexistent `sympy.simpl`.
- v2 repair protocol:
  [`PREREG_GLOBAL_WORK_PAIR_LOCAL_BATCH_CONCURRENCY_CERTIFICATE_REPAIR_v2.md`](../../preregistrations/native_time_carrier_programme/PREREG_GLOBAL_WORK_PAIR_LOCAL_BATCH_CONCURRENCY_CERTIFICATE_REPAIR_v2.md),
  SHA-256 `4557C4DDAF9D9A987F84C3779A5659AEAE83A3960DC3119C2A46572FB116FD18`.
- Immutable v2 wrapper:
  [`proof_global_work_pair_local_batch_concurrency_v2.py`](../../../../../scripts/proofs/proof_global_work_pair_local_batch_concurrency_v2.py),
  SHA-256 `16E950F3F92C588864E41981F3FA29B19D98FB220BD499394F476DC1344120B0`.
  Its inherited proof passed `62/62`, Outcome B; its own marker expected
  `59/59`, so the wrapper correctly failed closed at `13/14`.
- v3 marker-only repair protocol:
  [`PREREG_GLOBAL_WORK_PAIR_LOCAL_BATCH_CONCURRENCY_CERTIFICATE_REPAIR_v3.md`](../../preregistrations/native_time_carrier_programme/PREREG_GLOBAL_WORK_PAIR_LOCAL_BATCH_CONCURRENCY_CERTIFICATE_REPAIR_v3.md),
  SHA-256 `216FDA8F40D22FAE27D9B8388B00F9ECE7500125B47DAB7A5CCB3EE2F2460DD8`.
- Final wrapper:
  [`proof_global_work_pair_local_batch_concurrency_v3.py`](../../../../../scripts/proofs/proof_global_work_pair_local_batch_concurrency_v3.py),
  SHA-256 `86A32EEDBFDC0CB113ECA1DB9E6E1882B88214D6F5873C8250514F43D2E19738`.
- Final execution: inherited physical certificate `62/62`, repaired v2
  integrity `14/14`, final integrity `18/18`, **Outcome B**.
- Production mutation: none.

## 1. What one global pair can do

For disjoint phase spaces `(Z_a,Omega_a)`, adjoin one pair `(theta,I_G)` and
define

\[
 z_a'=R_a(\theta)z_a,\qquad
 I_G'=I_G+\sum_a {1\over2}z_a^TB_a(\theta)z_a.          \tag{4}
\]

The Jacobian of (4) preserves

\[
 \bigoplus_a\Omega_a+d\theta\wedge dI_G.               \tag{5}
\]

At the seam, equation (4) becomes

\[
 I_G'=I_G+\sum_a[H_a(z_a)-H_a(z_a')],                  \tag{6}
\]

so total field-plus-action energy is exact. Disjoint work additions commute.
This proves a global **accounting identity**. It does not establish a local
storage or transport mechanism for `I_G`.

## 2. Why that pair is not a local energy bus

Suppose `I_G` is stored at site `o`, while a nonconstant work term `w_a` is
supported in a remote region `Lambda_a` outside the one-tick radius `r`.
Then

\[
 {\partial I_G'\over\partial z_a}=\nabla w_a\ne0,
 \qquad \operatorname{dist}(o,\Lambda_a)>r.             \tag{7}
\]

Equation (7) violates the Moore-local one-tick Jacobian support condition.
If `I_G` is instead declared accessible at every site, it is an explicitly
nonlocal primitive, not derived substrate hardware.

A central reserve at graph distance `d` cannot authorize or replenish a
local port earlier than

\[
 N_{\min}=\left\lceil {d\over r}\right\rceil            \tag{8}
\]

ticks. A same-tick event therefore requires its usable reserve to be already
inside the event's causal cone.

## 3. Why one phase cannot canonically own many local reserves

Retaining local actions `I_1,...,I_N` while sharing a single phase gives

\[
 \omega_{\rm shared}=d\theta\wedge
 d\!\left(\sum_a I_a\right).                            \tag{9}
\]

For `N>1`, equation (9) has `N-1` exact kernel directions,

\[
 \partial_{I_a}-\partial_{I_b}.                         \tag{10}
\]

It is presymplectic. Imposing `theta_a=theta` on a product of local pairs has
the same degenerate pullback. Therefore independently physical local reserve
differences require their conjugate relative phases. Otherwise those
differences must be removed as gauge/constraints, leaving only the aggregate
action and losing independent local ownership.

This does not prohibit synchronization. The global tick or a propagated
calendar signal may align the **eligibility conditions** of complete local
clocks while their canonical phase spaces remain distinct.

## 4. Minimum concurrent local law

For pairwise causally disjoint ready batches, the direct product of equation
(1) preserves the nondegenerate form

\[
 \bigoplus_a\left(\Omega_a+
 d\theta_a\wedge dI_a\right).                           \tag{11}
\]

The maps commute for disjoint supports, preserve each local `H_a+I_a`, and
therefore preserve the total sum. Each inverse is local when the FTD-0982
event inverse and retained orientation record are local.

Under the registered assumptions of exact local reversibility, positive
energy ownership, and independent same-tick work, one phase-complete carrier
per pairwise disjoint future cone is minimum. This may be a local canonical
field distributed over the substrate; it need not be a permanent new
particle species for every event.

Overlapping events are not independent transactions. They must be compiled
as one joint batch or scheduled in nonoverlapping sublayers so that the same
field energy is not debited twice.

## 5. Reserve admission is separate from symplectic addition

A single reserve `I_G=1` and two separated demands of `3/4` provide an exact
counterexample to independent admission: each local check passes, while the
joint demand is `3/2>1`. Preventing this double spend requires either:

1. an atomic aggregate gate that reads all demands, which is not same-tick
   Moore-local for separated events; or
2. prepositioned local reserves with locally checkable budgets.

The additive map (4) remains algebraically well defined outside the positive
reserve domain. Canonical consistency and admissible positive capacity are
different gates.

## 6. Production capacity and the next discriminator

The unchanged production fields have six scalar canonical pairs per site
only after choosing a frame. The selected five-pair connection chart of
FTD-0965 leaves one complete pair unused. That is a conditional storage
capacity witness for one local work port.

It is not yet a native port because:

- the raw dual vectors have no site-local linear cubic-invariant scalar
  chart;
- host `phase` and `tau` have no conjugate action;
- CUDA has no phase buffer; and
- the production energy ledger contains no switching reserve, reciprocal
  reaction, admission backpressure, or replenishment transaction.

The next discriminator is therefore exact and finite: determine whether a
locally formed regional frame can turn the unused sixth pair into equation
(1) with covariant formation, synchronization, reserve charging, recycling,
and a local inverse. If not, the canonical work-port field must be booked as
an adopted physical type with its ontic and energetic cost exposed.

## 7. Epistemic disposition

Established:

- **[THEOREM]** one global pair is sufficient for aggregate symplectic work
  bookkeeping;
- **[THEOREM]** a spatially stored global pair cannot read remote nonconstant
  work in one Moore-local tick;
- **[THEOREM]** one phase with independently retained local actions is
  presymplectic;
- **[THEOREM]** complete local work pairs give an exact, order-independent,
  locally energy-preserving product law; and
- **[BOUNDARY]** centralized reserve admission is delayed by the causal cone
  and otherwise permits double spending.

Conditional only:

- production has selected-frame capacity for one unused scalar pair per site.

Still open:

- form a native regional scalar chart and identify the unused pair;
- synchronize complete local clocks without collapsing relative phase modes;
- charge, route, replenish, recycle, and fail-close local reserves;
- combine the local work field with the retained orientation/history latch;
- prove repeated finite-tick stability, CPU/CUDA parity, and operational
  hiding; and
- establish any physical `G*`, Born, Bell, mass, or selector-energy role.

No whole-framework completeness claim follows.
