# PRE-REGISTRATION — Catalytic phase-reference transducer v1

**Date locked:** 2026-08-11  
**Identifier:** `FTD-0863`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN; 36/36 PASS — OUTCOME B]`  
**Parents:** `FTD-0856`, `FTD-0858`, `FTD-0860`, `FTD-0862`

## 1. Question

Can the phase standard required by FTD-0862 be separated from the energetic
event carrier, so that a locally maintained canonical reference supplies only
orientation while an initially zero signal pair receives the event's own
energy through an exact reciprocal transaction?

The construction must answer four issues without conflation:

1. whether a local canonical phase reference can evolve autonomously with an
   exact energy and inverse;
2. whether that reference can orient a reciprocal matter/signal exchange;
3. whether the event can enter a zero-baseline signal without consuming or
   double counting the reference action; and
4. what periodic/cubic/production and `G*` debts remain.

“Catalytic” means only that the ideal reduced transaction leaves the declared
reference pair and its quadratic action unchanged. It is not a claim that
physical phase control, preparation, switching, or backreaction is free.

## 2. Frozen sources

| Source | SHA256 |
|---|---|
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_MINIMUM_RECIPROCAL_RECORD_PORT_BARRIER_v1.md` | `5D13921555B2289ABC5425F4D2436545C4C3BF0638FC71C98A577E3325D215EA` |
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_NATIVE_EVENT_ACTIVATION_CHARACTERISTIC_BOUNDARY_v1.md` | `06ED4EFEF16CF815A44E26F04213FC67F5388E917E9ED9D7B41F9FD8BA736B53` |
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_RELATIVE_ACTION_ORIENTATION_TRANSDUCER_BOUNDARY_v1.md` | `8269A241928681A6126B4D1F189FDEC3C5869916AF90E8825216844048D5A4C8` |
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_PHASE_REFERENCED_ACTION_EXPORT_RAIL_v1.md` | `94A75E375B8CB918B04C6D5C8DF5021380E8DA74243490BF1DD954ECBA26E32A` |
| `engine/include/ftd/eft/reciprocal_record_port.h` | `5973BF10BCE122304368E3BD191EA810D3DD6AB106B69B9D9022F662136D2B08` |
| `engine/include/ftd/eft/phase_referenced_action_rail.h` | `19EA541D11547460CC3AA3D041E8854E5A0277B6FDF58097B087E6D2139DF5DB` |

Any mismatch gives Outcome C and books no theorem.

## 3. Frozen phase reference

Let a nonzero real canonical pair

\[
 \beta=(q,p),\qquad I_*=\frac{q^2+p^2}{2}>0                 \tag{1}
\]

evolve by the selected exact rotation

\[
 \beta^{n+1}=R(-\omega)\beta^n,
 \qquad
 R(-\omega)=
 \begin{pmatrix}\cos\omega&\sin\omega\\
 -\sin\omega&\cos\omega\end{pmatrix}.            \tag{2}
\]

Equation (2) is the unit-tick flow of the selected harmonic action Hamiltonian
`H_ref=omega I_*`. It must preserve `I_*`, the symplectic form, and orientation;
its inverse is `R(+omega)`, and canonical time reversal exchanges the two
rotation senses.

For a spatial family initialized with twist `kappa`,

\[
 \phi_j^n=\phi_0+\kappa j-\omega n,               \tag{3}
\]

the reference is coherent with the outward signal shift iff

\[
 \kappa-\omega\in2\pi\mathbb Z.                  \tag{4}
\]

On a periodic ring of `N` sites, nonzero single-valued reference data also
require

\[
 N\kappa\in2\pi\mathbb Z.                         \tag{5}
\]

Equations (2)--(5) do not select `omega`. On an open rail it is a free selected
parameter; on a finite periodic ring it is commensurate with the ring winding.
No `G*` value or quartic period is permitted in the certificate.

## 4. Frozen reference-oriented reciprocal transaction

Normalize the reference frame as

\[
 e=\frac{\beta}{\sqrt{2I_*}},
 \qquad f=Je,
 \qquad J=\begin{pmatrix}0&-1\\1&0\end{pmatrix}.  \tag{6}
\]

Then `(e,f)` is orthonormal. Decompose an arbitrary signal pair `D` into

\[
 a=f\cdot D,
 \qquad b=e\cdot D,
 \qquad D=af+be.                                  \tag{7}
\]

Let `m` be the signed matter amplitude and `g in {0,1}` the separately supplied
hold/exchange eligibility. Freeze

\[
 \binom{m'}{a'}=
 \begin{pmatrix}1-g&g\\g&1-g\end{pmatrix}
 \binom m a,
 \qquad b'=b,
 \qquad \beta'=\beta.                            \tag{8}
\]

The transformation must be an orthogonal involution for both gate values. It
must conserve

\[
 H=\frac12(m^2+|D|^2)+I_*                        \tag{9}
\]

and the signed interface content `m+a`, while leaving the parallel spectator
component `b` and reference action fixed.

For emission from matter record `s` with energy `B>0`, use

\[
 m=s\sqrt{2B},\qquad D=0,qquad g=1.              \tag{10}
\]

The required output is

\[
 m'=0,
 \qquad
 D'=s\sqrt{\frac{B}{I_*}}J\beta,                 \tag{11}
\]

so that

\[
 \frac{|D'|^2}{2}=B,
 \qquad
 \operatorname{sign}(\beta\wedge D')=s.         \tag{12}
\]

Applying the same open gate to `(m=0,D=D')` must absorb the signal back into
the signed matter amplitude. This is the FTD-0856 reciprocal swap expressed in
the local phase frame.

## 5. Frozen interpretation and boundaries

The reference and signal are separate lanes:

- the reference lane carries persistent phase action `I_*` and is not loaded
  with event energy;
- the signal lane may start exactly at zero and receives exactly `B`;
- the actual record loses the same `B` under the adopted event-energy account;
  and
- the complete signal pair can then enter the FTD-0862 outward rail.

This construction avoids FTD-0860's empty-carrier obstruction because the
joint input is not rotationally structureless: `beta` supplies a nonzero
reference even though `D=0`. Joint rotation of `(beta,D)` must leave the law
covariant. The construction does not contradict the arbitrary-background
one-pair collision because it uses a **separate retained reference pair**.

The physical boundary remains:

1. `omega`, `I_*`, and the spatial twist are selected preparation data;
2. production has no protected phase-reference lane or exact signal rail;
3. the eligibility/controller input and its work are not derived;
4. a dynamical coupling must audit backreaction and angular/phase exchange;
5. periodic and cubic embeddings must avoid duplicate event-energy deposits;
   and
6. no clock-to-`G*` gearbox follows from harmonic phase kinematics.

## 6. Frozen certificate

The executable must return exactly `36/36`, covering:

- six source hashes;
- complex-structure, rotation-energy, orthogonality, symplecticity, inverse,
  and time-reversal identities;
- spatial/temporal coherence, periodic-ring winding, and the unselected
  frequency boundary;
- the orthonormal phase frame and exact signal decomposition;
- hold/swap matrices, orthogonality, involution, energy, signed-content, and
  spectator identities;
- exact emission, energy, sign, reciprocal absorption, unchanged reference,
  and non-double-counting accounts;
- zero-signal admissibility, joint rotational/sign covariance, and downstream
  phase transport; and
- production and scope firewalls.

No numerical search, fit, stochastic run, target probability, measurement
context, desired period, `G*` value, or formula substitution may enter.

## 7. Frozen outcomes

- **Outcome A — production mechanism:** production already supplies the
  autonomous protected phase reference, reciprocal phase-frame exchange,
  signal rail, controller/backreaction ledger, and cubic embedding.
- **Outcome B — exact catalytic reference transaction:** equations (1)--(12)
  pass, while the phase frequency/preparation, controller, backreaction,
  protected propagation, cubic embedding, and production realization remain
  selected/open.
- **Outcome C — invalid:** any source hash or exact identity fails; no theorem
  is booked.

Expected outcome: B.

## 8. Scope ceiling

Success does not derive the pilot frequency, does not derive G* cadence, does
not derive Born frequencies, does not establish cost-free control, does not
alter production C18, and does not establish a vacuum ground state, quartic
clock, thermodynamic arrow, Hilbert recovery, Bell mechanism, biological
identification, CM/substrate gearbox, Lorentz hiding, or completeness.

## 9. Locked executable

- Script: `scripts/proofs/proof_catalytic_phase_reference_transducer.py`
- Script SHA256:
  `CEA4C25D369732EA7F0CCF7675E11D20952760B0722ACCC5ACE8817B6427A105`
- Required command:
  `python scripts/proofs/proof_catalytic_phase_reference_transducer.py`
- Required denominator: exactly `36/36`.

## 10. Execution record

- Pre-run protocol SHA256:
  `1515D4ED700B1AED7FDBC9E7EA3BA623EC0DEC979682844C28AC166E9B95EE96`.
- First execution: `36/36 PASS`.
- Frozen outcome: **Outcome B — exact catalytic reference transaction**.
- Result:
  [`THEOREM_CATALYTIC_PHASE_REFERENCE_TRANSDUCER_v1.md`](../../derivations/native_time_carrier_programme/THEOREM_CATALYTIC_PHASE_REFERENCE_TRANSDUCER_v1.md).
