# PRE-REGISTRATION — Cubic odd event deposit v1

**Date locked:** 2026-08-10  
**Identifier:** `FTD-0853`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN; EXACT CERTIFICATE 32/32]`  
**Parents:** `FTD-0395`, `FTD-0506`, `FTD-0851`, `FTD-0852`

## 1. Question

Can a signed positive-energy record erasure deposit the FTD-0851 odd pulse
directly into the existing dual-field type by one local, cubically symmetric,
energy-closed transaction? What exact ready-port condition is required, and
what does the construction still fail to recover from production?

## 2. Frozen sources

| Source | SHA256 |
|---|---|
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_CAUSAL_ODD_PULSE_HISTORY_CARRIER_v1.md` | `7F393F78C2572ED9C61B20D897F3786BB366B305BA831DDB6CAD42344F4131E7` |
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_MINIMUM_ODD_EVENT_RECEIVER_v1.md` | `ED76BCD3266A472A96601BD673E85FF43B60CD0B2C5AF09E27CD08DA0ED700CF` |
| `docs/theory/02_foundations/ANALYSIS_FULL_STATE_IRREVERSIBILITY_v1.md` | `50CB845B2CB3874028A9C49C36141EB061785E6160F7880C361A21526C3461C0` |
| `engine/src/render_bridge_phases/phase_write.cpp` | `2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4` |
| `engine/src/energy_ledger_compute.cpp` | `2E5138BA43F74624C47842E9C3B0372ADFA9288BFE175BFE75ED901F237DD61B` |
| `engine/include/ftd/voxel.h` | `8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3` |
| `engine/include/ftd/lattice.h` | `C4FCF605FEAC11BB60EC77584F2E9D6BD33A1ADC576BE9EBFBED0E8478B2B831` |

Any mismatch yields Outcome C and no theorem.

## 3. Frozen deposit law

Let an event at lattice site `x` erase sign `s in {-1,+1}` and release energy
`B>0`. Let the six face directions be

\[
 \mathcal F=\{+e_x,-e_x,+e_y,-e_y,+e_z,-e_z\}.  \tag{1}
\]

At the neighbouring site `x+nu`, denote the pre-event dual wave velocities by
`W_{L,nu}` and `W_{R,nu}`. Define their signed radial receiver coordinate

\[
 Q_0=\sum_{\nu\in\mathcal F}
       \nu\cdot(W_{L,\nu}-W_{R,\nu}).            \tag{2}
\]

The exact ready-port gate is

\[
 Q_0=0.                                          \tag{3}
\]

It does not require every background component to vanish; it requires zero
net radial relative occupation at the event port.

Set

\[
 p=\sqrt{B/6},                                   \tag{4}
\]

and update only the dual wave velocities:

\[
 W'_{L,\nu}=W_{L,\nu}+s p\nu,
 \qquad
 W'_{R,\nu}=W_{R,\nu}-s p\nu.                   \tag{5}
\]

The actual record changes `s -> 0`, and its declared event-energy account
changes by `-B`. All other reduced variables are unchanged.

## 4. Frozen exact claims

### 4.1 Common/relative and energy closure

Equation (5) adds zero common wave velocity at every arm. With the selected
dual kinetic energy

\[
 K_{LR}=\frac12\sum_{\nu\in\mathcal F}
  (|W_{L,\nu}|^2+|W_{R,\nu}|^2),                \tag{6}
\]

the exact increment is

\[
 \Delta K_{LR}=s p Q_0+6p^2.                    \tag{7}
\]

Under (3), `Delta K_LR=B`; record plus receiver energy closes exactly.

### 4.2 Signed recovery and reduced injectivity

The post-event radial coordinate is

\[
 Q_1=Q_0+12sp=s\sqrt{24B}.                      \tag{8}
\]

Hence

\[
 s=\operatorname{sign}(Q_1),\qquad
 B=Q_1^2/24.                                     \tag{9}

\]

After recovering `(s,B)`, subtracting equation (5)'s pulse recovers every
pre-event shell velocity. Thus the transaction is injective on the declared
reduced ready-port domain.

This is not full-engine injectivity. Particle identifiers, spin, color,
remainders, and other metadata erased by production are not encoded by (5).
FTD-0395 remains binding.

### 4.3 Cubic symmetry and support minimum

The six impulses have zero vector sum separately in L and R. The radial scalar
in (2) and energy in (6) are invariant under signed coordinate permutations.
Conjugation `(s,L,R)->(-s,R,L)` leaves the transaction covariant.

Within the 26 nonzero first-Moore-shell directions, the signed-permutation
orbits have sizes `6` (face), `12` (edge), and `8` (corner). Therefore the
six-face orbit is the minimum nonzero directed orbit that preserves the full
cubic signed-permutation symmetry. This minimum is scoped to equal-orbit
first-shell deposits; it is not a universal minimum over arbitrary onsite
scalar or multi-tick representations.

## 5. Frozen production classification

The certificate must confirm that production already has the required L/R
wave-velocity type and six-face neighbour interface, but does not implement
equation (5). Its aggregate ledger squares only the common L/R sum, so the
deposit's zero-common pulse would currently register zero receiver energy.

The construction is consequently `[SELECTED REFERENCE TRANSACTION]`. Promotion
requires a separate production implementation/campaign with:

- an event-energy definition `B` derived from the exact local pre/post state;
- the ready-port gate (3) or a reversible background-aware replacement;
- a separate dual/common-relative energy and face-current ledger;
- propagation/port-clearing compliance;
- a reciprocal protected-record barrier; and
- explicit treatment of every erased full-state label.

## 6. Frozen certificate

The independent script must return exactly `32/32` checks covering:

- seven source hashes;
- the production dual type, six-face shell, common reconstruction, and
  common-only ledger boundary;
- exact enumeration of the 48 signed permutation matrices and the `6/12/8`
  Moore-direction orbits;
- face-vector norms and zero sum;
- armwise common cancellation and relative orientation;
- equations (7)--(9);
- reduced inverse recovery on `Q_0=0`;
- explicit failure of naive exact energy off the ready-port condition;
- P4 locality, cubic covariance, and L/R conjugation covariance;
- production non-equivalence, full-state ceiling, and target-blindness.

No fitting, stochastic run, parameter search, near-miss scan, or formula
substitution is permitted.

## 7. Frozen outcomes

- **Outcome A — production transaction complete:** current event code and
  ledger implement the exact deposit and full receiver requirements.
- **Outcome B — exact selected deposit, production incomplete:** equations
  (1)--(9) pass, while production lacks the deposit/ledger/barrier/full-state
  closure.
- **Outcome C — invalid:** a source hash or exact gate fails.

Expected outcome: B. No production change is authorized.

## 8. Scope ceiling

Success does not derive `B`, the ready-port dynamics, production dual energy,
the six-arm selection, a full natural extension, a microscopic bath, thermal
entropy, Born frequencies, Bell correlations, consciousness, biological
hemispheres, `G*`, or finite-tick cadence.

## 9. Locked executable

- Script: `scripts/proofs/proof_cubic_odd_event_deposit.py`
- Script SHA256:
  `902815340FC6B830D41036337B18DE3D6556BBE98215E5F2859D8E21254BA5AD`
- Required command: `python scripts/proofs/proof_cubic_odd_event_deposit.py`
- Required denominator: exactly `32/32`.

## 10. Locked execution outcome

- Pre-run protocol SHA256:
  `F89BAB6F49566CC2EC38CCBA6F7EDFB5B0E8319A4ED3AEB89949D9F8B26B2AF3`.
- First execution: `32/32 PASS`.
- Frozen outcome: **Outcome B — exact selected deposit, production
  incomplete**.
- Theorem of record:
  [`THEOREM_CUBIC_ODD_EVENT_DEPOSIT_v1.md`](../../derivations/native_time_carrier_programme/THEOREM_CUBIC_ODD_EVENT_DEPOSIT_v1.md).
