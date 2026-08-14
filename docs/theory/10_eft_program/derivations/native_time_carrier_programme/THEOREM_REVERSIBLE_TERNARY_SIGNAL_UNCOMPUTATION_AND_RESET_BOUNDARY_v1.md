# THEOREM — Reversible ternary signal uncomputation and reset boundary v1

**Identifier:** `FTD-0871`  
**Status:** `[THEOREM — EXACT REVERSIBLE ACTUAL-LAYER RESET]` +
`[THEOREM — MINIMUM RETAINED-ORIENTATION REQUIREMENT]` +
`[CLOSED NEGATIVE — LOGICAL NECESSITY OF AN EXTRA ACK BIT/RESET TRIT/BATH]` +
`[OPEN — CONTINUOUS REALIZATION, CONTROLLER WORK, OUTPUT TRANSPORT, PRODUCTION]`  
**Certificate:** repaired FTD-0871 `40/40`; FTD-0870 remains
execution-invalid at `39/40`  
**Date:** 2026-08-11

## 1. Result

The completed outgoing signal of FTD-0869 does more than acknowledge the
event. Because it retains the same oriented ternary value as the latch, it can
serve as reversible workspace that **uncomputes** the actual latch in one
discrete update. No additional acknowledgement bit, reset-history trit, or
logical bath is required.

This closes reset only for the actual ternary label. It does not supply a
continuous trajectory for FTD-0848's selected coordinate `x`, physical gate
work, protected output transport, or a production rule.

## 2. Ternary group law

Encode

\[
 \mathbb T=\{-1,0,+1\}
\]

as the additive group `Z_3` by

\[
 \zeta(0)=0,\qquad \zeta(+1)=1,\qquad \zeta(-1)=2. \tag{1}
\]

Define

\[
 s\oplus u=\zeta^{-1}(\zeta(s)+\zeta(u)\bmod3),
\]

\[
 s\ominus u=\zeta^{-1}(\zeta(s)-\zeta(u)\bmod3). \tag{2}
\]

Let `E` be the completed local signal and

\[
 u=d(E)\in\mathbb T                                  \tag{3}
\]

its oriented sign decoder. With the signal-completion acknowledgement
`a in {0,1}`, define

\[
 U_a(s,E)=\left(s\ominus a\,d(E),E\right).           \tag{4}
\]

For `a=0`, equation (4) is identity. On the registered event subspace,
`a=1` and `d(E)=s`, hence

\[
 \boxed{U_1(s,E_s)=(0,E_s).}                         \tag{5}
\]

Both event signs and the no-event value reset exactly.

## 3. Reversibility and minimum information

The inverse is

\[
 U_a^{-1}(s',E)=\left(s'\oplus a\,d(E),E\right).     \tag{6}
\]

Direct substitution gives

\[
 U_a^{-1}U_a=U_aU_a^{-1}=\operatorname{id}.          \tag{7}
\]

Therefore equation (5) is not erasure on the joint latch/signal state. It is
a reversible controlled subtraction. Under simultaneous sign reversal,

\[
 U_a(-s,-E)=-U_a(s,E),                               \tag{8}
\]

where the minus on the right acts on both ternary components.

The retained orientation is necessary. The bare map

\[
 -1,0,+1\longmapsto0
\]

is three-to-one. Any retained record that makes it invertible on all three
inputs needs at least three distinguishable labels. Scalar energy is
insufficient because `(-1)^2=(+1)^2`. The oriented signal decoder already has
exactly the required ternary range, so adding another reset-history coordinate
would duplicate the information at this logical interface.

This is the discrete analogue of ancilla uncomputation: once the answer has
been copied into a retained output, the correlated working register can be
returned to ready by an invertible controlled operation.

## 4. Why this does not contradict the smooth-reset theorem

FTD-0869 proves that a locally Lipschitz autonomous ODE cannot take a nonzero
continuous coordinate to an exact equilibrium in finite time. Equation (4) is
not such an ODE. It is a finite-state discrete update on the actual layer.

For the selected continuous reference potential

\[
 V_T(x)=\beta x^2(x^2-A^2)^2,                       \tag{9}
\]

the registered endpoints are degenerate:

\[
 V_T(-A)=V_T(0)=V_T(+A)=0.                          \tag{10}
\]

Thus actual-layer uncomputation has zero endpoint storage-energy difference.
Equation (10) does **not** prove zero controller work or provide a continuous
path. There are now two honest realization branches:

1. take the ternary actual state as fundamental and implement a local
   controlled permutation directly; or
2. retain the selected continuous coordinate and supply a nonsmooth, switched,
   or larger reversible canonical realization.

FTD-0869's cusp law belongs to branch 2. It is no longer a logical necessity
for branch 1.

## 5. Recursive local sequence

The minimum reference cycle is

\[
 \text{exchange}\to
 \text{completed signal }E_s\to
 U_1\to
 \text{empty-port handoff}.                        \tag{11}
\]

After the exchange, `(s,E)=(s,E_s)`. Equation (5) gives `(0,E_s)`. The
reciprocal output swap gives

\[
 (0,E_s;O=0)\longmapsto(0,0;O=E_s).                \tag{12}
\]

The local latch and signal port are ready. The output retains

\[
 B=|O|^2/2,
 \qquad s=\operatorname{sign}(\beta\wedge O).       \tag{13}
\]

Acknowledgement is combinational: it is computed from local signal completion
and is not retained as a new state after (12).

## 6. Finite history capacity

Transport, not reset, is now the live recursion boundary. A finite length-`N`
ternary rail has `3^N` configurations. There are `3^T` ternary event/no-event
histories of length `T`. For `T>N`,

\[
 3^T>3^N,                                           \tag{14}
\]

so no injection of every registered history into that finite rail exists
without signed tail export, recurrence/identification, or another retained
state. Exporting only scalar tail energy loses the two orientations.

Equation (14) is scoped to a finite ternary rail. It does not exclude exact-real
natural extensions or other alphabets.

## 7. Isolated implementation

The reference contract is implemented without production mutation at:

- `engine/include/ftd/eft/reversible_ternary_signal_uncomputation.h`;
- `engine/src/eft/reversible_ternary_signal_uncomputation.cpp`; and
- `engine/tests/test_reversible_ternary_signal_uncomputation.cpp`.

The focused Release CTest passes `1/1`. The API recomputes acknowledgement from
the matching local signal, performs exact ternary subtraction/inversion, hands
the signal to an empty output port, and fails closed on mismatch or
backpressure. Its scope flags explicitly deny continuous reset, controller
work, protected transport, production coupling, and native `G*`
synchronization.

## 8. Epistemic accounting

- **[THEOREM]** Equations (1)--(8): exact reversible ternary uncomputation and
  minimum retained-orientation requirement.
- **[THEOREM]** Equations (11)--(14): conditional local readiness and finite
  ternary-rail capacity boundary using the prior selected output interface.
- **[CLOSED NEGATIVE]** An extra acknowledgement bit, reset-history trit, or
  logical scalar bath is necessary at the registered actual-layer interface.
- **[SELECTED/OPEN]** The physical controlled-permutation implementation and
  output rail remain selected reference structures until derived in production.
- **[OPEN]** Continuous-latch dynamics if `x` is retained physically.
- **[OPEN]** Controller switching/work, detuning/noise/overlap/backpressure
  robustness, and native latch/reference/mode formation.
- **[OPEN]** Protected cubic output transport, finite-boundary signed export,
  and production event/energy-current coupling.
- **[OPEN]** Native `G*` synchronization, Born/Bell recovery, operational
  Lorentz hiding, and framework completeness.

## 9. Conclusion

At the actual ternary layer, unactualization need not mean deletion or
dissipation. It can mean reversible removal of a redundant local record after
the same information has moved into a retained signal. Local actuality becomes
ready; global information remains in the potential/output channel. The hard
dynamics have moved to where they belong: implementing the controlled
permutation physically and transporting the signal away without backpressure.
