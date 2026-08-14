# PRE-REGISTRATION — Phase-referenced action export rail v1

**Date locked:** 2026-08-11  
**Identifier:** `FTD-0861`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN; EXECUTION INVALID 35/36 — NO THEOREM]`  
**Parents:** `FTD-0852`, `FTD-0855`, `FTD-0856`, `FTD-0858`, `FTD-0860`

## 1. Question

Can the exact nonzero-carrier pump of FTD-0860 become a faithful, recursively
reusable event transducer when its input is restricted to a maintained
phase-referenced baseline and its output is transported by the selected
one-cell causal rail of FTD-0852?

The certificate must distinguish three levels:

1. an exact theorem on a prepared nonzero baseline;
2. a selected directed rail and open-environment completion; and
3. the still-open physical origin, maintenance, and production realization of
   that baseline and rail.

The result may remove FTD-0860's arbitrary-background collision only on the
registered prepared subspace. It may not erase that collision on a general
unlabelled canonical-pair background.

## 2. Frozen sources

| Source | SHA256 |
|---|---|
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_CAUSAL_ODD_PULSE_HISTORY_CARRIER_v1.md` | `7F393F78C2572ED9C61B20D897F3786BB366B305BA831DDB6CAD42344F4131E7` |
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_DIAGNOSTIC_EVENT_ENERGY_CUBIC_RAIL_GEARBOX_v1.md` | `4A498C6D7C7E65FA685D9F0879157D76713F310A6D025CCAA8756C3F1E0322E6` |
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_MINIMUM_RECIPROCAL_RECORD_PORT_BARRIER_v1.md` | `5D13921555B2289ABC5425F4D2436545C4C3BF0638FC71C98A577E3325D215EA` |
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_NATIVE_EVENT_ACTIVATION_CHARACTERISTIC_BOUNDARY_v1.md` | `06ED4EFEF16CF815A44E26F04213FC67F5388E917E9ED9D7B41F9FD8BA736B53` |
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_RELATIVE_ACTION_ORIENTATION_TRANSDUCER_BOUNDARY_v1.md` | `8269A241928681A6126B4D1F189FDEC3C5869916AF90E8825216844048D5A4C8` |
| `docs/theory/10_eft_program/native_time_carrier_programme/SPEC_CARRIER_CONSTRAINTS_v1.md` | `6E14439EF155FF3590910DAEFDAACB2A348942112664712E681D3F84C11EB23C` |
| `engine/include/ftd/eft/relative_action_transducer.h` | `E4E7C237D7AF7BB3B3000CFAC4D63C0E8126801422EF43809754BAF086400D42` |

Any mismatch gives Outcome C and books no theorem.

## 3. Frozen reference construction

### 3.1 Prepared phase calendar

Let every rail site carry a real canonical pair

\[
 Z_j^n=(Q_j^n,P_j^n),\qquad I(Z)=\frac{Q^2+P^2}{2}.
\]

Freeze a nonzero baseline action `I_*>0` and phase calendar

\[
 \beta_j^n=\sqrt{2I_*}
 \begin{pmatrix}\cos\phi_j^n\\ \sin\phi_j^n\end{pmatrix},
 \qquad
 \phi_j^n=\phi_0+\kappa j-\omega n.              \tag{1}
\]

The selected one-cell outward rail obeys

\[
 Z_{j+1}^{n+1}=Z_j^n.                             \tag{2}
\]

The baseline is coherent under (2) exactly when

\[
 \kappa-\omega\in2\pi\mathbb Z,                 \tag{3}
\]

because `phi_{j+1}^{n+1}-phi_j^n=kappa-omega`. On the principal
representative the certificate may set `kappa=omega`.

The incoming boundary pair at depth `-1` and time `n` is then precisely the
next output-time baseline at depth zero:

\[
 \beta_{-1}^{n}=\beta_0^{n+1}.                   \tag{4}
\]

Equations (1), (3), and (4) are a `[SELECTION]` of prepared clock/rail
hardware. Their existence and maintenance are not derived from P1--P5.

### 3.2 Phase-referenced event load

For an accepted event `(s_n,B_n)` with `s_n in {-1,+1}` and `B_n>0`, apply
FTD-0860's selected pump to the prepared incoming baseline:

\[
 Y_n=\sqrt{\frac{I_*+B_n}{I_*}}\,s_nJ\beta_{-1}^{n},
 \qquad
 J=\begin{pmatrix}0&-1\\1&0\end{pmatrix}.        \tag{5}
\]

The open finite rail of length `N` updates by

\[
 Z_0^{n+1}=Y_n,
 \qquad
 Z_{j+1}^{n+1}=Z_j^n\quad(0\le j<N-1),           \tag{6}
\]

and exports the complete old tail pair `Z_{N-1}^n`. A no-event boundary input
is the unpumped next baseline. The first certificate tests the positive-event
branch; it does not define an event hazard.

### 3.3 Frozen readout

At every later point on the loaded characteristic, compare the pair with its
local baseline. The exact readout is

\[
 B=I(Z)-I_*,
 \qquad
 s=\operatorname{sign}[\beta\wedge Z],
 \qquad
 \beta\wedge Z=\beta_Q Z_P-\beta_P Z_Q.          \tag{7}
\]

Equation (7) works because the loaded pair remains at relative phase
`s*pi/2` and action `I_*+B`. It is not a universal readout on arbitrary
backgrounds. The collision `F_{+,B}(Z)=F_{-,B}(-Z)` from FTD-0860 must remain
explicitly true outside the prepared-baseline restriction.

## 4. Frozen action and reversibility claims

Define excess action at each retained site by

\[
 E_j^n=I(Z_j^n)-I_*.
\]

For `H_ex^n=sum_{j=0}^{N-1}E_j^n`, equations (5)--(6) must give

\[
 H_{\rm ex}^{n+1}-H_{\rm ex}^{n}
 =B_n-E_{N-1}^n.                                  \tag{8}
\]

If `0<=B_n<=B_max` and the rail is initialized on its baseline, the retained
load must obey

\[
 0\le H_{\rm ex}^n\le N B_{\max}.                \tag{9}
\]

Exporting only the scalar tail excess loses orientation. The complete tail
pair together with its phase-calendar reference retains both `B` and `s`.
That pair is the environment output and must not be counted as a second
energetic sign rail.

For fixed event controls, the pump is symplectic on `I>0`. Completing the
finite open rail with one incoming environment pair and one outgoing tail pair
makes the shift a permutation of canonical pairs composed with that pump. The
extended map is injective/symplectic and has a known-event inverse. The
retained finite rail alone is open and is not reversible without the incoming
pair, outgoing tail, and event control.

## 5. Frozen production classification

Success licenses only an isolated selected reference mechanism.

- The rail chooses an outward direction and protected channel.
- A physical cubic embedding would require a separately registered collection
  of face rails and an exact non-double-counting energy split.
- Frozen production C18 is dispersive and fails the primitive one-cell shift
  identity by FTD-0858.
- Production event acceptance reads the common-field quotient and does not
  actuate the relative pair.
- Production supplies no maintained `I_*`, phase calendar, baseline source,
  signed-tail environment, or controller-work account.

No production code change is authorized.

## 6. Frozen certificate

The executable must return exactly `36/36`, covering:

- seven source hashes;
- the real quarter-turn identities;
- the phase-calendar mismatch, coherence, characteristic, and boundary-input
  identities;
- exact baseline action, pumped action, orthogonality, oriented area, and
  recovery of `(s,B)`;
- disjoint sign branches on one prepared baseline while preserving the exact
  arbitrary-background collision;
- one-cell locality, characteristic transport, multi-event ordering, and
  recursive port reuse;
- the finite excess-action/tail ledger, scalar-sign loss, complete-pair tail
  recovery, and retained-load bound;
- symplectic canonical-pair permutation, fixed-branch pump area, and extended
  injectivity; and
- production non-equivalence and the scope firewall.

No numerical search, fitting, stochastic sample, target probability,
measurement context, desired period, `G*` value, or formula substitution may
enter the certificate.

## 7. Frozen outcomes

- **Outcome A — production mechanism:** production already supplies the
  coherent nonzero baseline, directed protected rail, event load, relative
  ledger, tail environment, and controller account.
- **Outcome B — exact selected reference rail:** equations (1)--(9) pass, but
  the prepared phase calendar, directed rail, maintenance, and production
  realization remain selected/open.
- **Outcome C — invalid:** a source hash or exact identity fails; no theorem is
  booked.

Expected outcome: B.

## 8. Scope ceiling

Success does not derive a persistent vacuum carrier, does not derive the phase
calendar, does not derive G* cadence, does not derive Born frequencies, does
not alter production C18, and does not establish a Hilbert-space recovery,
Bell mechanism, thermodynamic arrow, biological hemisphere identification,
CM/substrate gearbox, Lorentz hiding, or completeness.

## 9. Locked executable

- Script: `scripts/proofs/proof_phase_referenced_action_export_rail.py`
- Script SHA256:
  `098FA1885B72D60DD0B8DAE547CEAD73B96A8977D92EB11DD896EC4311840F09`
- Required command:
  `python scripts/proofs/proof_phase_referenced_action_export_rail.py`
- Required denominator: exactly `36/36`.

## 10. Execution record

- Pre-run protocol SHA256:
  `D5CEFB6550DD7EED1DE5C5001E970EFB1F2D6EE25F8F5C1644E0CA4A5532CE80`.
- First execution: `35/36 FAIL`. All seven source hashes and all exact
  mathematical checks passed. C35 used the absent prose marker `does not
  actuate the relative pair`; the frozen source instead states the stronger
  exact boundary `its common-field trigger cannot determine the relative
  port`.
- Frozen outcome: **execution invalid; no theorem**.
- Verifier-only successor:
  [`PREREG_PHASE_REFERENCED_ACTION_EXPORT_RAIL_CERTIFICATE_REPAIR_v2.md`](PREREG_PHASE_REFERENCED_ACTION_EXPORT_RAIL_CERTIFICATE_REPAIR_v2.md).
