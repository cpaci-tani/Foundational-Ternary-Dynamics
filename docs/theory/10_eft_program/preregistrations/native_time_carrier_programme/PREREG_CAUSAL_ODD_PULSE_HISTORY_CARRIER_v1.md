# PRE-REGISTRATION — Causal odd-pulse history carrier v1

**Date locked:** 2026-08-10  
**Identifier:** `FTD-0852`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN; EXACT CERTIFICATE 32/32]`  
**Parents:** `FTD-0781`, `FTD-0844`, `FTD-0850`, `FTD-0851`

## 1. Question

Can the positive-export odd receiver of FTD-0851 be advanced into a stable
recursive history carrier whose update is local, causal, injective, and exactly
energy/current closed? Which parts of that carrier already exist in the
production dual-substrate update, and which transactions remain absent?

The target is not indefinite erasure in the registered finite rail. The
certificate must state that rail's tail-export boundary exactly. No universal
finite-dimensional receiver no-go is licensed: fixed-dimensional exact-real
natural extensions can encode arbitrarily long branch histories, as FTD-0570
already demonstrates.

## 2. Frozen sources

| Source | SHA256 |
|---|---|
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_MINIMUM_ODD_EVENT_RECEIVER_v1.md` | `ED76BCD3266A472A96601BD673E85FF43B60CD0B2C5AF09E27CD08DA0ED700CF` |
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_COMMON_RELATIVE_LOCAL_QUARTIC_CLOCK_v1.md` | `64241D7AB18AD2079ECADF9EA25448F53F42696AB3FF439637970D4284497FD0` |
| `engine/src/render_bridge_phases/phase_read.cpp` | `D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8` |
| `engine/src/render_bridge_phases/phase_write.cpp` | `2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4` |
| `engine/src/energy_ledger_compute.cpp` | `2E5138BA43F74624C47842E9C3B0372ADFA9288BFE175BFE75ED901F237DD61B` |
| `engine/include/ftd/voxel.h` | `8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3` |

Any mismatch makes the execution Outcome C and books no theorem.

## 3. Frozen reference carrier

### 3.1 One causal rail

For one local event per tick, let `s_n in {-1,+1}`, `B_n>0`, and

\[
 a_n=s_n\sqrt{2B_n}.                              \tag{1}
\]

Let `D_j^n` be the odd receiver amplitude at causal depth `j>=0`. Define

\[
 D_0^{n+1}=a_n,\qquad D_{j+1}^{n+1}=D_j^n.        \tag{2}
\]

The receiver energy and outward face current are

\[
 e_j^n=\frac{(D_j^n)^2}{2},\qquad
 F_{j+1/2}^n=e_j^n,qquad F_{-1/2}^n=0.           \tag{3}
\]

Equation (2) is one-neighbour local and propagates exactly one cell per global
tick. The local continuity equation to certify is

\[
 e_j^{n+1}-e_j^n+F_{j+1/2}^n-F_{j-1/2}^n
 =B_n\,\delta_{j0}.                              \tag{4}
\]

On the half-line, the update is injective. Its inverse reads `a_n=D_0^{n+1}`,
then `s_n=sign(a_n)`, `B_n=a_n^2/2`, and
`D_j^n=D_{j+1}^{n+1}`.

### 3.2 Finite capacity

On a rail of length `N`, define the outgoing tail energy

\[
 E_{\rm out}^n=\frac{(D_{N-1}^n)^2}{2}.          \tag{5}
\]

The internal energy obeys

\[
 H^{n+1}-H^n=B_n-E_{\rm out}^n.                 \tag{6}
\]

A scalar accumulation of `E_out` closes energy but not sign history. Exact
injectivity after boundary crossing requires the signed tail amplitude itself
to enter another receiver/environment. This is the finite-information-capacity
boundary; no thermodynamic or Landauer claim is allowed.

### 3.3 Bilateral and cubic representations

At every rail site use

\[
 L_j=\frac{D_j}{\sqrt2},\qquad
 R_j=-\frac{D_j}{\sqrt2}.                        \tag{7}
\]

Then `C_j=(L_j+R_j)/sqrt(2)=0`, the relative coordinate is `D_j`, and

\[
 \frac{L_j^2+R_j^2}{2}=\frac{D_j^2}{2}.          \tag{8}
\]

The first cubically symmetric equal-arm representation uses the six face
directions. Deposit

\[
 D_{\nu,0}=s\sqrt{B/3},\qquad
 \nu\in\{+x,-x,+y,-y,+z,-z\}.                   \tag{9}
\]

The six arm energies sum to `B`; each arm uses (2). This six-arm choice is a
`[SELECTED reference representation]`, not a unique derivation from the
production vector field.

## 4. Frozen production classification

The certificate must establish from source that:

1. `Voxel` already stores `flux_L`, `flux_R`, `wave_vel_L`, and `wave_vel_R`;
2. production defines the observable common fields as the L/R sums;
3. `phase_read` applies the same Laplacian, clock coefficient, and equal
   coupling source to L and R, so subtraction cancels the equal source and
   leaves a homogeneous relative equation;
4. `phase_write` integrates and damps L/R separately before rebuilding their
   sums;
5. the frozen stencil reads both positive and negative neighbours, so it is
   not equation (2)'s one-way shift;
6. the aggregate energy ledger uses only the common `flux` and `wave_vel`, not
   separate L/R squares. A pure relative state is therefore invisible to that
   ledger;
7. FTD-0851 remains binding: no production event deposits equation (1), (7),
   or (9) into the relative channel.

The permitted positive result is consequently narrow: production contains a
local homogeneous **candidate carrier channel**, not a complete event receiver
or an accounted history transport.

## 5. Frozen certificate

The independent executable must return exactly `32/32` checks covering:

- six source hashes;
- production dual type, common reconstruction, identical L/R propagation,
  equal-source cancellation, separate integration, bidirectional stencil, and
  common-only energy-ledger facts;
- equation (1) sign/energy recovery;
- one-cell locality and explicit depth/time solution of (2);
- energy recurrence and sitewise continuity (4);
- half-line injectivity and inverse;
- receiver reuse without overwriting prior amplitudes;
- finite-rail equation (6), loss of sign in a scalar tail ledger, and recovery
  when signed tail amplitude is retained;
- bilateral identities (7)--(8);
- six-arm energy, inversion, and cubic balance;
- current-production non-equivalence and target-blindness controls.

No fit, stochastic experiment, numerical near-miss search, or formula
substitution is permitted.

## 6. Frozen outcomes

- **Outcome A — production complete:** the current event, dual propagation,
  and ledger together implement the exact injective odd history carrier.
- **Outcome B — reference carrier exact, production partial:** equations
  (1)--(9) pass; production has a homogeneous relative channel but lacks event
  deposit, relative-energy accounting, exact clearing/injectivity, or the
  reciprocal barrier.
- **Outcome C — invalid:** any source or exact gate fails; no theorem.

The expected result is Outcome B. No engine change is authorized.

## 7. Scope ceiling

Success does not derive the six-arm representation, production relative-field
energy, receiver coupling, a reciprocal record barrier, a microscopic bath,
thermal entropy, Born frequencies, Bell correlations, biological hemispheres,
`G*`, or finite-tick cadence. It provides the first exact recursive
unactualization carrier and a precise production interface debt.

## 8. Locked executable

- Script: `scripts/proofs/proof_causal_odd_pulse_history_carrier.py`
- Script SHA256:
  `9E1238C161851798442D75607A81E80346FFD6CBD16F9F13194FDC311FD9920D`
- Required command:
  `python scripts/proofs/proof_causal_odd_pulse_history_carrier.py`
- Required denominator: exactly `32/32`.

## 9. Locked execution outcome

- Pre-run protocol SHA256:
  `881AB0032444085885B65141091DC54FA6F493024FA94EB47D34C463F4CE6C39`.
- First execution: `32/32 PASS`.
- Frozen outcome: **Outcome B — reference carrier exact, production partial**.
- Theorem of record:
  [`THEOREM_CAUSAL_ODD_PULSE_HISTORY_CARRIER_v1.md`](../../derivations/native_time_carrier_programme/THEOREM_CAUSAL_ODD_PULSE_HISTORY_CARRIER_v1.md).

### Scope correction booked with the theorem

The pre-run text (hash above) used “finite-capacity boundary” once in language
broad enough to suggest a universal finite-receiver no-go. The 32 checks prove
only equations (5)--(6) for the registered finite shift rail: discarding its
tail sign is noninjective. They do not exclude fixed-dimensional exact-real
natural extensions. The theorem and canonical summaries use the narrower
`[FINITE RAIL TAIL-EXPORT BOUNDARY]` status. The locked hash preserves the
original wording as provenance.
