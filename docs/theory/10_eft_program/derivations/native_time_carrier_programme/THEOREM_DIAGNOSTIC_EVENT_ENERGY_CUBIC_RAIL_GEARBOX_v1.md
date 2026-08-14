# Theorem — Diagnostic event energy and cubic rail gearbox (FTD-0855)

**Status:** `[THEOREM — CONDITIONAL DIAGNOSTIC EVENT-ENERGY IDENTITY]` +
`[THEOREM — CUBIC/RADIAL-RAIL ISOMETRY]` +
`[THEOREM — CAUSAL RECURSIVE READY-PORT FORMATION]` +
`[IMPOSED — E_REST MATTER-ENERGY ROLE]` +
`[SELECTION — RESERVED SIX-RAY RELATIVE RECEIVER]` +
`[CLOSED NEGATIVE — CURRENT PRODUCTION REALIZATION]` +
`[OPEN — DUAL LEDGER, RESERVED CHANNEL, BARRIER, AND FULL-STATE LIFT]`  
**Date:** 2026-08-10  
**Protocol:**
[`PREREG_DIAGNOSTIC_EVENT_ENERGY_CUBIC_RAIL_GEARBOX_CERTIFICATE_REPAIR_v2.md`](../../preregistrations/native_time_carrier_programme/PREREG_DIAGNOSTIC_EVENT_ENERGY_CUBIC_RAIL_GEARBOX_CERTIFICATE_REPAIR_v2.md)  
**Parent invalid protocol:**
[`PREREG_DIAGNOSTIC_EVENT_ENERGY_CUBIC_RAIL_GEARBOX_v1.md`](../../preregistrations/native_time_carrier_programme/PREREG_DIAGNOSTIC_EVENT_ENERGY_CUBIC_RAIL_GEARBOX_v1.md)  
**Repair pre-run SHA256:**
`DE703D4C1702C8B8E34F741CBA22790A64F19981938908DFBA3B7B0AD3DC0829`  
**Certificate:**
`scripts/proofs/proof_diagnostic_event_energy_cubic_rail_gearbox_v2.py`,
SHA256 `8953357829B0814BE60D6855FF2DE9A167256E757C2431C2BA997FCB9E26C647`,
`32/32 PASS`

## 1. The event-energy account already exists conditionally

Production diagnostics assign a manifested record with valid raw speed `v` the
matter energy

\[
 B_{\rm diag}(v)=E_{\rm rest}+K(v)
 =\gamma(v)E_{\rm rest},
 \qquad
 \gamma(v)=\frac{1}{\sqrt{1-v^2/C_{\rm speed}^2}}.             \tag{1}
\]

Because `E_REST>0` in the adopted engine role map and `gamma>=1`, equation (1)
is strictly positive, including at rest. It is even under record-sign reversal
and reads no measurement context, target outcome, Born weight, `G*`, or cadence.

The frozen evaporation assignment changes a nonzero record to zero and clears
particle ID, spin, and color without changing the site's continuous field or
velocity coordinates. Production diagnostics count rest and kinetic energy
only while the record is nonzero. Therefore their exact matter-energy decrement
for this assignment is

\[
 \Delta E_{\rm matter}=B_{\rm diag}.                            \tag{2}
\]

Equation (2) derives `B` inside the already adopted diagnostic contract. It
does not derive `E_REST` from P1--P5. `E_REST=M_INERTIAL*C_SPEED^2` and the
electron-primary inertial scale remain imposed role/calibration choices.

## 2. The cubic shell is the causal rail in another coordinate

At depth `j` along the six axial rays, use the pure radial relative mode

\[
 W_{L,j,\nu}=a_j\nu,
 \qquad W_{R,j,\nu}=-a_j\nu,
 \qquad \nu\in\{\pm e_x,\pm e_y,\pm e_z\}.                    \tag{3}
\]

Its FTD-0853 radial coordinate and normalized rail amplitude are

\[
 Q_j=\sum_\nu\nu\cdot(W_{L,j,\nu}-W_{R,j,\nu})=12a_j,
 \qquad D_j=\frac{Q_j}{\sqrt{12}}=\sqrt{12}\,a_j.             \tag{4}
\]

Under the selected dual kinetic energy,

\[
 K_j=\frac12\sum_\nu(|W_{L,j,\nu}|^2+|W_{R,j,\nu}|^2)
 =6a_j^2=\frac{D_j^2}{2}=\frac{Q_j^2}{24}.                    \tag{5}
\]

Thus the FTD-0853 shell and FTD-0852 rail are exactly isometric on the declared
reserved radial subspace. This is the gearbox that was previously missing: it
identifies the local cubic hardware coordinate with the recursive history
coordinate without a numerical fit.

## 3. The shift forms the next ready port

For an erasure `(s,B_diag)`, the synchronous update is

\[
 a_0'=s\sqrt{B_{\rm diag}/6},
 \qquad a_{j+1}'=a_j,                                         \tag{6}
\]

or equivalently

\[
 D_0'=s\sqrt{2B_{\rm diag}},
 \qquad D_{j+1}'=D_j.                                        \tag{7}
\]

Every prior port value moves one cell outward before it can be overwritten,
while the new event occupies depth zero. In an equivalent split-step reading,
the outward permutation first clears depth zero and the FTD-0853 deposit then
writes onto the exact ready surface. The synchronous rule is still Moore-local:
depth `j+1` reads its inward nearest neighbour, and depth zero reads the
adjacent event site.

The total receiver energy satisfies

\[
 \frac12\sum_{j\ge0}(D_j')^2
 =B_{\rm diag}+\frac12\sum_{j\ge0}D_j^2.                      \tag{8}
\]

The half-line inverse reads the newest `(s,B_diag)` from `D_0'` and recovers
every prior amplitude from `D_{j+1}'`. The next update again moves the current
port occupation outward. Ready-port formation is therefore a property of the
recursive dynamics, not a context-sensitive eligibility test.

## 4. What is closed

At selected reference scope, the complete event-to-history chain is now

\[
 \text{manifested diagnostic energy}
 \to (s,B_{\rm diag})
 \to \text{six-face relative deposit}
 \to \text{one-cell outward history shift}
 \to \text{fresh input port}.                                \tag{9}
\]

The transaction is deterministic, cubically balanced, common-field neutral,
causal, recursively reusable, signed-history preserving, and exactly closed in
the selected energy. It explains how an actual record may become void while a
declared part of its former information and energy continues as potential
field history.

## 5. What remains open

Production does not implement equation (9):

1. its aggregate drift ledger excludes rest energy and separate L/R quadratic
   energy even though diagnostics report them;
2. evaporation does not route the diagnostic decrement into a relative pulse;
3. the existing dual wave field is shared with ambient propagation rather than
   partitioned into a protected receiver mode;
4. the bidirectional wave stencil is not the directed shift (6);
5. no reciprocal barrier prevents the actual record from being erased or
   overwritten without the declared transaction; and
6. particle ID, spin, color, remainder, latency, and other erased distinctions
   are not encoded by `(s,B_diag)`.

Accordingly this result does not establish production energy conservation,
full-state unitarity, a microscopic bath, thermodynamic cost, Born frequencies,
Bell correlations, `G*` cadence, biology, or completeness.

No production code changed.

## 6. Certificate record

The FTD-0854 parent run returned `29/32` because of three verifier-domain/slice
defects. FTD-0855 changed only those checks in memory and returned:

```text
FTD-0854 diagnostic event energy and cubic rail gearbox: 32/32 PASS
DIAGNOSTIC_MATTER_DECREMENT_SUPPLIES_POSITIVE_CONTEXT_BLIND_EVENT_B
CUBIC_RADIAL_COORDINATE_IS_EXACTLY_THE_NORMALIZED_ODD_HISTORY_RAIL
OUTWARD_SHIFT_FORMS_THE_NEXT_READY_PORT_AND_CLOSES_RECEIVER_ENERGY
PRODUCTION_DUAL_LEDGER_RESERVED_RAIL_BARRIER_AND_FULL_STATE_LIFT_REMAIN_OPEN
VERDICT=OUTCOME_B_EXACT_REFERENCE_GEARBOX_PRODUCTION_INCOMPLETE
```
