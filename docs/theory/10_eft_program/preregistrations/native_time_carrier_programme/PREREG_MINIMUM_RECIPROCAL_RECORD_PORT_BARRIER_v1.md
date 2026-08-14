# PRE-REGISTRATION — Minimum reciprocal record-port barrier v1

**Date locked:** 2026-08-10  
**Identifier:** `FTD-0856`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN; EXACT CERTIFICATE 32/32]`  
**Parents:** `FTD-0506`, `FTD-0852`, `FTD-0855`

## 1. Question

What is the minimum deterministic local interface that both protects a ternary
actual record when closed and exchanges its signed energy reciprocally with a
causal field when open? Does production already contain the required gate,
incoming/outgoing distinction, and scattering transaction?

## 2. Frozen sources

| Source | SHA256 |
|---|---|
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_DIAGNOSTIC_EVENT_ENERGY_CUBIC_RAIL_GEARBOX_v1.md` | `4A498C6D7C7E65FA685D9F0879157D76713F310A6D025CCAA8756C3F1E0322E6` |
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_CAUSAL_ODD_PULSE_HISTORY_CARRIER_v1.md` | `7F393F78C2572ED9C61B20D897F3786BB366B305BA831DDB6CAD42344F4131E7` |
| `docs/theory/07_assessment/common_action_mechanics_reciprocity/AUDIT_PRODUCTION_SAME_SIGN_BOUNCE.md` | `090F139CBA8C930A9761A33EFBFB59BD2767F22E4DF50031120B70E18D42EA15` |
| `engine/include/ftd/voxel.h` | `8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3` |
| `engine/src/render_bridge_phases/phase_write.cpp` | `2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4` |
| `engine/src/render_bridge_phases/phase_movement.cpp` | `6149B37C5A28B8EE9B8544CAEC24006D0964D1C8F344CA63C68DC6536A47E8FB` |
| `engine/src/render_bridge_phases/phase_read.cpp` | `D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8` |

Any mismatch yields Outcome C and no theorem.

## 3. Frozen lower bound

Let `M_s` denote the same complete local input state of a protected occupied
record, with `s in {-1,+1}` and no incident event pulse. A deterministic
autonomous map `F` cannot satisfy both

\[
 F(M_s)=M_s                                                   \tag{1}
\]

for strict quiescent protection and

\[
 F(M_s)=P_s\ne M_s                                            \tag{2}
\]

for an event exchange on the identical complete input. Therefore the complete
input must contain at least two distinguishable eligibility values. This is a
cardinality lower bound on a gate/activation coordinate, not a derivation of
its physical dynamics.

Likewise, an emitted pulse and its time reverse have opposite causal
orientation. If an interface stores only an unlabeled scalar amplitude, the
incoming and outgoing states are identified. A reciprocal forward-time local
implementation must retain their distinction through an incoming/outgoing
port label, two counterpropagating characteristics, a conjugate field pair, or
equivalent directional state. The certificate may establish this scoped
orientation lower bound, not a universal state-dimension no-go.

## 4. Frozen attaining construction

For `B>0`, set `A=sqrt(2B)`. Represent a ternary record by the matter amplitude

\[
 m=A r,
 \qquad r\in\{-1,0,+1\},                                     \tag{3}
\]

and let `i,o in {-A,0,+A}` be the incoming and outgoing signed characteristic
amplitudes at the record boundary. Let the eligibility coordinate be
`g in {0,1}`. Define

\[
 \binom{m'}{o}=S_g\binom{m}{i},
 \qquad
 S_g=\begin{pmatrix}1-g&g\\g&1-g\end{pmatrix}.               \tag{4}
\]

Thus

\[
 S_0=I,
 \qquad S_1=\begin{pmatrix}0&1\\1&0\end{pmatrix}.            \tag{5}
\]

The selected boundary energy and signed content are

\[
 H=\frac12(m^2+i^2),
 \qquad \chi=m+i.                                             \tag{6}
\]

For `g=0`, the record is strictly held and an incident characteristic exits
without coupling. For `g=1`, matter and field characteristics swap:

\[
 (sA,0)\mapsto(0,sA) \quad\text{(emission)},                  \tag{7}
\]

\[
 (0,sA)\mapsto(sA,0) \quad\text{(absorption)}.                \tag{8}
\]

The outgoing amplitude in (7) is exactly the FTD-0855 rail amplitude
`D_0=s*sqrt(2B)`. The incoming time reverse requires an inward characteristic;
the outward history rail alone is not a complete reciprocal field.

## 5. Frozen claims

The certificate may prove only:

1. at least two eligibility states are necessary when the same reduced record
   must sometimes hold and sometimes exchange under deterministic dynamics;
2. reciprocal forward-time field exchange must retain causal orientation in
   the scoped first-order rail representation;
3. equations (4)--(8) attain both lower bounds with a symmetric orthogonal
   involution, exact energy/signed-content conservation, strict closed-gate
   persistence, and reciprocal emission/absorption;
4. the construction is local, deterministic, sign equivariant, and blind to
   measurement context, outcome target, Born weight, `G*`, and cadence; and
5. production's `locked` flag is a hold fragment, while its dual field type is
   a characteristic-capacity fragment, but no current phase implements the
   controlled record/relative-field scatterer.

It may not claim:

- a physical origin for `g` or free gate actuation;
- that `locked` is dynamically generated or reciprocal;
- that the production bidirectional wave stencil is already separated into a
  protected incoming/outgoing record channel;
- a full natural extension of all erased particle labels;
- Born, Bell, `G*`, thermodynamic, biological, or framework-completeness
  recovery.

A future physical gate may be a conjunction of target-blind local activation
and preregistered clock compliance. Clock phase alone must not force every
quiescent record to emit.

## 6. Gates

The exact source-and-algebra certificate has 32 gates:

1. seven source hashes;
2. production exposes a Boolean `locked` coordinate, skips locked records in
   movement and evaporation, and therefore contains a strict hold fragment;
3. production same-sign bounce is not reciprocal and erases subcell phase;
4. production dual fields provide relative position/velocity capacity but no
   event-controlled characteristic scatterer;
5. `S_0` is identity and `S_1` is exchange;
6. both matrices are symmetric, orthogonal, involutive, and energy preserving;
7. signed content and sign-reversal covariance hold;
8. closed-gate persistence, open-gate emission, and open-gate absorption hold;
9. the eligibility and causal-orientation lower bounds are attained;
10. the outgoing event matches the FTD-0855 normalized rail; and
11. all production and interpretation boundaries remain explicit.

## 7. Outcomes

- **Outcome A:** minimum reciprocal barrier proved and already realized by
  production dynamics.
- **Outcome B:** minimum reciprocal barrier proved as a selected exact
  reference interface; production contains only hold/type fragments.
- **Outcome C:** source mismatch or exact theorem failure.

Expected honest result: Outcome B. No production code may change in this run.

## 8. Recorded outcome

The first locked execution returned `32/32 PASS`, Outcome B. The scoped lower
bounds and attaining controlled scatterer are booked in
[`THEOREM_MINIMUM_RECIPROCAL_RECORD_PORT_BARRIER_v1.md`](../../derivations/native_time_carrier_programme/THEOREM_MINIMUM_RECIPROCAL_RECORD_PORT_BARRIER_v1.md).
