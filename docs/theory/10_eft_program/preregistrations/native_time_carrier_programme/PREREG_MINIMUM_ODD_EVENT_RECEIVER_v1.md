# PRE-REGISTRATION — Minimum odd event receiver v1

**Date locked:** 2026-08-10  
**Identifier:** `FTD-0851`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN; EXACT CERTIFICATE 30/30]`  
**Parents:** `FTD-0395`, `FTD-0449`, `FTD-0506`, `FTD-0570`, `FTD-0844`,
`FTD-0848`, `FTD-0850`

## 1. Question

For a local ternary event that maps two distinguishable signed preimages to
one actual record,

\[
  e(+1)=e(-1)=0,
\]

what is the minimum receiver that can (i) retain the erased sign distinction,
(ii) receive a declared nonnegative event-energy export exactly, and (iii)
remain blind to measurement context, selected outcome targets, Born weights,
`G*`, and cadence targets? Does the current production movement/event-journal
path already implement that receiver?

This is a type-and-transaction discriminator. It is not a Born campaign, a
clock-cadence test, a thermodynamic derivation, or authorization to change the
engine.

## 2. Frozen sources

| Source | SHA256 |
|---|---|
| `engine/src/render_bridge_phases/phase_movement.cpp` | `6149B37C5A28B8EE9B8544CAEC24006D0964D1C8F344CA63C68DC6536A47E8FB` |
| `engine/include/ftd/eft/history_event_journal.h` | `4A9AEDC650FE882C0CB6421901784095DA4EA079D3CCBC985DD412148583955A` |
| `engine/src/eft/history_event_journal.cpp` | `94EBB526F3F31CB53D8907109BA29BD207E3D8E3828DCCA6D2C2C7B31B620B91` |
| `engine/src/energy_ledger_compute.cpp` | `2E5138BA43F74624C47842E9C3B0372ADFA9288BFE175BFE75ED901F237DD61B` |
| `docs/theory/07_assessment/common_action_mechanics_reciprocity/AUDIT_PRODUCTION_SAME_SIGN_BOUNCE.md` | `090F139CBA8C930A9761A33EFBFB59BD2767F22E4DF50031120B70E18D42EA15` |
| `docs/theory/10_eft_program/derivations/common_action_mechanics_reciprocity/THEOREM_GENESIS_NATURAL_EXTENSION.md` | `2611A6DE2D2318DFC4EC97FDF148D91D952BE3775421BE4DDAC441EA2F534076` |
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_PRODUCTION_TERNARY_LATCH_BOUNDARY_v1.md` | `95F39274E361868E039368AB149A9196F2008D2BB58CD5F0DAD0CD8F7E92110B` |

Any mismatch invalidates the execution and books no theorem.

## 3. Frozen mathematical class

### 3.1 Erasure domain

Use exactly two distinguishable signed event preimages
`S={-1,+1}` and one reduced actual output `0`. No probability measure is
introduced.

### 3.2 Receiver requirements

A receiver map `R:S x R_{>=0} -> Y` is **sign-complete** when

\[
  R(+1,B)\ne R(-1,B)
\]

for every registered `B>=0`. It is **energy-complete** when a declared
receiver energy `H_R` obeys `H_R(R(s,B))=B`.

The general minimum reference receiver is

\[
  R_{\rm gen}(s,B)=(\chi,B)=(s,B),
  \qquad H_R(\chi,B)=B.                         \tag{1}
\]

The sign coordinate is logically required at `B=0`: an energy-only receiver
has the same value for both signs.

For the positive-export subdomain `B>0`, one signed real amplitude is enough:

\[
  a=s\sqrt{2B},\qquad H_a(a)=\frac{a^2}{2}=B.  \tag{2}
\]

The inverse is `s=sign(a)`, `B=a^2/2`.

### 3.3 Bilateral realization

The first selected two-channel representation of (2) is

\[
  L=s\sqrt{B},\qquad R=-s\sqrt{B}.              \tag{3}
\]

With

\[
 C=\frac{L+R}{\sqrt2},\qquad
 D=\frac{L-R}{\sqrt2},\qquad
 H_{LR}=\frac{L^2+R^2}{2},                     \tag{4}
\]

equations (3)--(4) require `C=0`, `D=s sqrt(2B)`, and `H_LR=B`.
This is one continuous relative degree of freedom represented on two channels,
not two independent imported bits. It is `[SELECTED reference
representation]`, not a production derivation.

### 3.4 Repeated-event condition

Overwriting a finite receiver with a new event generally erases the old
preimage. Repeated exact reception therefore requires at least one of:

1. causal propagation of the old odd pulse into fresh environmental degrees
   of freedom before reuse;
2. an expanding retained history; or
3. an explicitly lossy external environment.

The immutable event journal is a diagnostic example of item 2, but it is not
a dynamical or energetic receiver because its source contract says it is an
observer, disabled by default, and writes no lattice, voxel, toggle, or
integrator state.

## 4. Frozen source classification

The certificate must establish directly from the frozen sources that:

- same-sign production collision flips only mover axes, resets mover
  remainder, leaves the target/field unchanged, and emits no journal event;
- production annihilation clears both signed states, velocities, remainders,
  identifiers, and internal labels, and redistributes pre-existing flux using
  no sign-bearing term;
- swapping `+/-` state labels while holding all continuous pre-event fields
  fixed therefore produces the same annihilation output;
- the history journal retains complete before/after voxel copies but is
  observation-only;
- the aggregate energy ledger reads voxel fields/velocities, not event
  histories, and has no event receiver coordinate.

The FTD-0506 measured result remains binding: the apparent same-sign hard wall
is not a reciprocal finite-range barrier because it erases remainder and lacks
target/field recoil and current. It may be called a **barrier fragment**, not
a closed latch mechanism.

## 5. Frozen certificate

The independent script must return exactly 30 checks:

1. seven source hashes;
2. exact source contracts for same-sign bounce, annihilation, journal
   neutrality, and aggregate-ledger scope;
3. noninjectivity of signed erasure;
4. impossibility of sign recovery from an energy-only receiver;
5. the two-output lower bound for all-`B` sign completeness;
6. exact sign and energy recovery by `(chi,B)`;
7. exact positive-export compression to one signed amplitude;
8. exact bilateral common/relative and energy identities;
9. repeated overwrite noninjectivity;
10. current-production non-equivalence and target-blindness controls.

No floating fit, parameter search, near-miss scan, stochastic campaign, or
formula substitution is allowed.

## 6. Frozen outcomes

- **Outcome A — production receiver already complete:** current movement plus
  lattice fields implement a sign-complete, event-energy-complete, repeatedly
  receivable transaction. This requires every production gate to pass.
- **Outcome B — minimum receiver derived, current production incomplete:**
  equations (1)--(4) pass, while production supplies only barrier/exhaust/
  observer fragments. Book the receiver as a selected reference type and keep
  physical realization open.
- **Outcome C — certificate invalid:** any source mismatch or exact check
  fails. Book no theorem.

The expected result is Outcome B. No engine change is authorized.

## 7. Scope ceiling

Success proves only a minimum information/energy receiver architecture. It
does not prove that production `flux_L/flux_R` carries equation (3), that the
dual fields have the declared energy, that a microscopic heat bath exists,
that the full map is reversible, or that Born frequencies, Bell correlations,
consciousness, biological hemispheres, `G*`, or finite-tick cadence follow.

## 8. Locked executable

- Script: `scripts/proofs/proof_minimum_odd_event_receiver.py`
- Script SHA256:
  `28030DDE523026CBF0587E82DDDE885C05D16D58D227D78E4923835A2662F805`
- Required command:
  `python scripts/proofs/proof_minimum_odd_event_receiver.py`
- Required denominator: exactly `30/30`.

## 9. Locked execution outcome

- Pre-run protocol SHA256:
  `374F571E155DEF0DE4A4CBF3A17C84E5D5EB60ED471308F3C02C5A1F8FBA8DDA`.
- First execution: `30/30 PASS`.
- Frozen outcome: **Outcome B — minimum receiver derived, current production
  incomplete**.
- Theorem of record:
  [`THEOREM_MINIMUM_ODD_EVENT_RECEIVER_v1.md`](../../derivations/native_time_carrier_programme/THEOREM_MINIMUM_ODD_EVENT_RECEIVER_v1.md).
