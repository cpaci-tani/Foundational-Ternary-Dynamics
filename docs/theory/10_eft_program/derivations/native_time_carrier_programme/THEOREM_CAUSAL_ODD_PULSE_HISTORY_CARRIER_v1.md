# Theorem — Causal odd-pulse history carrier (FTD-0852)

**Status:** `[THEOREM — LOCAL CAUSAL INJECTIVE ODD-PULSE SHIFT]` +
`[THEOREM — EXACT ENERGY-CURRENT CONTINUITY]` +
`[THEOREM — FINITE RAIL TAIL-EXPORT BOUNDARY]` +
`[ENGINE FACT — HOMOGENEOUS PRODUCTION RELATIVE CHANNEL]` +
`[CLOSED NEGATIVE — CURRENT EVENTS/LEDGER AS COMPLETE CARRIER]` +
`[SELECTION/OPEN — SHIFT LAW, CUBIC EMBEDDING, EVENT DEPOSIT, BARRIER]`  
**Date:** 2026-08-10  
**Protocol:**
[`PREREG_CAUSAL_ODD_PULSE_HISTORY_CARRIER_v1.md`](../../preregistrations/native_time_carrier_programme/PREREG_CAUSAL_ODD_PULSE_HISTORY_CARRIER_v1.md)  
**Pre-run protocol SHA256:**
`881AB0032444085885B65141091DC54FA6F493024FA94EB47D34C463F4CE6C39`  
**Certificate:** `scripts/proofs/proof_causal_odd_pulse_history_carrier.py`,
SHA256 `9E1238C161851798442D75607A81E80346FFD6CBD16F9F13194FDC311FD9920D`,
`32/32 PASS`

## 1. Exact recursive carrier

For a positive-export signed event `(s_n,B_n)`, define

\[
 a_n=s_n\sqrt{2B_n}.                              \tag{1}
\]

Let `D_j^n` denote the odd receiver amplitude at causal depth `j`. The selected
reference update is

\[
 D_0^{n+1}=a_n,\qquad D_{j+1}^{n+1}=D_j^n.        \tag{2}
\]

Equation (2) has five exact properties.

1. **Locality:** each output reads only the same event port or one adjacent
   predecessor.
2. **Causality:** information advances exactly one cell per global tick.
3. **Injectivity on the half-line:** the newest event is recovered from
   `D_0^{n+1}` and every previous amplitude from `D_{j+1}^{n+1}`.
4. **Recursive readiness:** writing a new event at depth zero moves the old
   pulse to depth one instead of overwriting it.
5. **Orientation retention:** `sign(D_j)` retains the erased signed branch;
   `D_j^2/2` retains its exported energy.

After `T` events from the zero state,

\[
 (D_0^T,D_1^T,\ldots,D_{T-1}^T)
 =(a_{T-1},a_{T-2},\ldots,a_0).                  \tag{3}
\]

Thus the substrate clock index becomes a history depth: older locally
unactualized details occupy more distant causal cells.

## 2. Exact energy and current

Set

\[
 e_j^n=\frac{(D_j^n)^2}{2},\qquad
 F_{j+1/2}^n=e_j^n,qquad F_{-1/2}^n=0.           \tag{4}
\]

At the event port,

\[
 e_0^{n+1}-e_0^n+F_{1/2}^n=B_n.                 \tag{5}
\]

At every positive depth,

\[
 e_j^{n+1}-e_j^n+F_{j+1/2}^n-F_{j-1/2}^n=0.     \tag{6}
\]

Summing equations (5)--(6) on the half-line gives

\[
 H^{n+1}-H^n=B_n.                               \tag{7}
\]

If the record/latch loses energy `B_n` in the same event transaction, the
combined record-plus-carrier energy is exactly conserved. No damping,
statistical averaging, or target probability enters this identity.

## 3. Finite-rail tail-export boundary

For a rail of length `N`, the old tail leaves the retained domain. With

\[
 E_{\rm out}^n=\frac{(D_{N-1}^n)^2}{2},
\]

the exact internal ledger is

\[
 H^{n+1}-H^n=B_n-E_{\rm out}^n.                 \tag{8}
\]

Adding `E_out` to a scalar external-energy account closes energy but loses
the sign because `D` and `-D` have the same square. A complete deterministic
extension must export the **signed tail amplitude**, not energy alone.

Therefore the registered finite shift rail cannot retain histories after its
tail is discarded without one of:

- further causal storage;
- recurrence/reuse that eventually identifies histories; or
- an explicitly open environment receiving the signed tail.

This is a theorem about the declared rail representation, not a universal
finite-dimensional memory no-go. In particular, it does not exclude
fixed-dimensional exact-real natural extensions that encode branch history in
progressively finer coordinates (FTD-0570 supplies such a counterclass to the
broader wording). No Landauer or entropy-cost result is derived.

## 4. Bilateral realization

Represent every rail coordinate as

\[
 L_j=\frac{D_j}{\sqrt2},\qquad
 R_j=-\frac{D_j}{\sqrt2}.                        \tag{9}
\]

Then

\[
 C_j=\frac{L_j+R_j}{\sqrt2}=0,qquad
 \frac{L_j-R_j}{\sqrt2}=D_j,qquad
 \frac{L_j^2+R_j^2}{2}=\frac{D_j^2}{2}.          \tag{10}
\]

The actual/common channel can remain unchanged while the potential/relative
channel transports the erased orientation and energy. This is the precise
mathematical content of the earlier bilateral intuition; no biological
left/right identification is made.

The selected cubic embedding launches six equal face-directed rails,

\[
 D_{\nu,0}=s\sqrt{B/3},\qquad
 \nu\in\{\pm x,\pm y,\pm z\}.                   \tag{11}
\]

Their vector directions balance to zero and their six quadratic energies sum
to `B`. Equation (11) is a reference representation, not a unique result of
the production lattice dynamics.

## 5. Production contains a candidate relative channel

Production `Voxel` already stores `flux_L`, `flux_R`, `wave_vel_L`, and
`wave_vel_R`. In the dual path:

- `phase_read` applies the same local Laplacian coefficient to L and R;
- matter coupling is added equally to both;
- imposed clock terms, when enabled, use the same coefficient on both;
- `phase_write` integrates and damps the two channels separately before
  rebuilding `flux=flux_L+flux_R` and
  `wave_vel=wave_vel_L+wave_vel_R`.

Subtracting the two acceleration equations cancels every equal matter source.
The relative field therefore obeys a homogeneous local equation. This is a
real production-native **candidate carrier channel**, stronger than a merely
invented reservoir type.

The positive conclusion stops there.

1. No production event deposits equation (1), (9), or (11) into the relative
   channel.
2. The frozen wave stencil reads both positive and negative neighbours; it is
   not equation (2)'s one-way injective shift and supplies no exact receiver-
   clearing theorem.
3. The aggregate energy ledger squares only the reconstructed common `flux`
   and `wave_vel`. A pure relative state with `R=-L` is therefore invisible to
   the current ledger even though its component amplitudes are nonzero.
4. The same-sign movement wall remains nonreciprocal and phase-erasing under
   FTD-0506/0851.

Current production is therefore partial, not equivalent to the exact history
carrier.

## 6. Physical interpretation

The complete reference cycle is now

\[
 \text{signed acquisition}\to\text{protected record}\to
 \text{signed erasure}\to\text{odd pulse}\to
 \text{causal history depth}\to\text{fresh event port}.      \tag{12}
\]

Equation (12) supplies a deterministic mechanism for local lossiness without
global deletion: actuality keeps only the coarse ternary record, while the
potential layer carries discarded branch information into its causal past
cone. Local observers need not retain or reconstruct that history.

This is compatible with the user's interpretation of unactualization, but it
does not by itself produce a probability law. Born frequencies would require
a separate theorem connecting substrate preparation measures and basin/event
volumes; no such weights occur here.

## 7. Remaining dynamics

The next production gate is sharply defined:

1. add or derive an event deposit into the existing relative channel using
   only local pre-event state;
2. define and close the separate L/R or common/relative energy and face-current
   ledger;
3. show that an outgoing production packet vacates the event port within a
   declared compliance window without sign loss or controllable signalling;
4. replace the remainder-reset wall with a reciprocal protected-record
   barrier; and
5. verify held-out multi-event histories and finite-boundary signed export.

No production code changed. The shift law, six-arm embedding, receiver energy,
event coupling, barrier, thermal reading, Born rule, biology, `G*` cadence, and
operational Lorentz hiding remain selected or open.

## 8. Certificate record

```text
FTD-0852 causal odd-pulse history carrier: 32/32 PASS
HALF_LINE_ODD_PULSE_SHIFT_IS_LOCAL_CAUSAL_INJECTIVE_AND_ENERGY_CLOSED
FINITE_RECEIVER_CAPACITY_REQUIRES_SIGNED_TAIL_EXPORT
PRODUCTION_DUAL_DIFFERENCE_IS_A_HOMOGENEOUS_CANDIDATE_CHANNEL
PRODUCTION_LEDGER_AND_EVENTS_DO_NOT_COMPLETE_THE_HISTORY_CARRIER
VERDICT=OUTCOME_B_EXACT_REFERENCE_CARRIER_PRODUCTION_PARTIAL
```
