# Theorem — Catalytic phase-reference transducer (FTD-0863)

**Status:** `[THEOREM — AUTONOMOUS ACTION-PRESERVING PHASE REFERENCE]` +
`[THEOREM — RECIPROCAL PHASE-FRAME MATTER/SIGNAL EXCHANGE]` +
`[THEOREM — ZERO-BASELINE SIGNAL ENERGY TRANSFER]` +
`[THEOREM — PERIODIC WINDING COMMENSURABILITY]` +
`[SELECTED REFERENCE REALIZATION — CONSUMES SEL-CA-PHASE-RAIL]` +
`[REFERENCE IMPLEMENTATION — ISOLATED ftd::eft]` +
`[CLOSED NEGATIVE — G* FREQUENCY FORCED BY PHASE KINEMATICS]` +
`[OPEN — NATIVE FORMATION, ROBUST MAINTENANCE, BACKREACTION, CUBIC RAIL, CONTROLLER, AND PRODUCTION LEDGER]`  
**Date:** 2026-08-11  
**Protocol:**
[`PREREG_CATALYTIC_PHASE_REFERENCE_TRANSDUCER_v1.md`](../../preregistrations/native_time_carrier_programme/PREREG_CATALYTIC_PHASE_REFERENCE_TRANSDUCER_v1.md)  
**Pre-run protocol SHA256:**
`1515D4ED700B1AED7FDBC9E7EA3BA623EC0DEC979682844C28AC166E9B95EE96`  
**Certificate:**
`scripts/proofs/proof_catalytic_phase_reference_transducer.py`, SHA256
`CEA4C25D369732EA7F0CCF7675E11D20952760B0722ACCC5ACE8817B6427A105`,
`36/36 PASS`

## 1. Result

The phase reference and the event carrier need not be the same energetic
degree of freedom. A persistent nonzero canonical pair can supply a local
orientation frame while a **separate, initially zero** signal pair receives
the event energy through an exact reciprocal swap.

This is a stricter accounting architecture than loading the FTD-0862 baseline:

- reference action remains `I_*`;
- actual matter loses event energy `B`;
- the signal gains exactly `B`; and
- no energetic sign coordinate is added a second time.

The reference is catalytic only in this reduced exact sense. Its preparation,
frequency selection, protection, switching controller, and dynamical
backreaction remain physical debts.

## 2. Autonomous phase reference

Let

\[
 \beta=(q,p),\qquad I_*=\frac{q^2+p^2}{2}>0.       \tag{1}
\]

Select the exact unit-tick phase map

\[
 \beta^{n+1}=R(-\omega)\beta^n,
 \qquad
 R(-\omega)=
 \begin{pmatrix}\cos\omega&\sin\omega\\
 -\sin\omega&\cos\omega\end{pmatrix}.            \tag{2}
\]

Then

\[
 R^TR=1,\qquad \det R=1,
 \qquad R^T\Omega R=\Omega,
 \qquad I_*^{n+1}=I_*^n.                           \tag{3}
\]

The inverse is `R(+omega)`. For canonical time reversal
`K=diag(1,-1)`,

\[
 KR(-\omega)K=R(+\omega).                          \tag{4}
\]

Thus the isolated reference is reversible and needs no damping or energy
injection to persist in its ideal exact model. This closes **isolated
recurrence**, not formation or perturbation recovery.

For a spatial phase helix

\[
 \phi_j^n=\phi_0+\kappa j-\omega n,               \tag{5}
\]

coherence with the outward signal shift is again

\[
 \kappa-\omega\in2\pi\mathbb Z.                  \tag{6}
\]

On a periodic `N`-site ring, single-valued nonzero reference data additionally
require

\[
 N\kappa\in2\pi\mathbb Z.                         \tag{7}
\]

An open rail leaves `omega` free; a finite periodic rail makes it commensurate
with a selected winding. Neither case forces a particular frequency or the
quartic `G*` period.

## 3. Phase frame

The nonzero reference defines

\[
 e=\frac{\beta}{\sqrt{2I_*}},
 \qquad f=Je,
 \qquad
 J=\begin{pmatrix}0&-1\\1&0\end{pmatrix}.         \tag{8}
\]

The pair `(e,f)` is orthonormal. Every signal pair has the unique decomposition

\[
 D=af+be,
 \qquad a=f\cdot D,
 \qquad b=e\cdot D.                               \tag{9}
\]

`a` is the reference-orthogonal signed event channel. `b` is a parallel
spectator. The sign information lies in the relational antisymmetric area

\[
 \beta\wedge D=\sqrt{2I_*}\,a.                   \tag{10}
\]

This gives the substrate an exact clockwise/counterclockwise discriminator
without assigning orientation to scalar action itself.

## 4. Reciprocal matter/signal exchange

Let `m` be a signed matter amplitude and `g in {0,1}` a separately supplied
eligibility value. Apply the FTD-0856 gate in the phase frame:

\[
 \binom{m'}{a'}=
 S_g\binom ma,
 \qquad
 S_g=\begin{pmatrix}1-g&g\\g&1-g\end{pmatrix},
 \qquad b'=b.                                     \tag{11}
\]

Both branches are orthogonal involutions. Hence

\[
 \frac12(m'^2+|D'|^2)=\frac12(m^2+|D|^2),
 \qquad m'+a'=m+a.                                \tag{12}
\]

The reference pair is unchanged, so the full declared energy

\[
 H=I_*+\frac12(m^2+|D|^2)                         \tag{13}
\]

is exact.

For an emitted event

\[
 m=s\sqrt{2B},\qquad D=0,qquad g=1,              \tag{14}
\]

equation (11) gives

\[
 m'=0,
 \qquad
 D'=s\sqrt{\frac{B}{I_*}}J\beta.                 \tag{15}
\]

Therefore

\[
 \frac{|D'|^2}{2}=B,
 \qquad
 \operatorname{sign}(\beta\wedge D')=s.         \tag{16}
\]

The same open gate applied to `(m=0,D=D')` returns
`m=s sqrt(2B)` and clears the signal. Emission and absorption are the same
involution, not separate fitted laws.

## 5. What improved over FTD-0862

FTD-0862 used a loaded carrier of action `I_*+B` and read event energy as
excess above the baseline. FTD-0863 factors that construction into two lanes:

\[
 \text{reference lane: }I_*,
 \qquad
 \text{signal lane: }0\longleftrightarrow B.       \tag{17}
\]

The new factorization has three advantages.

1. The event signal may start at zero because the **joint** state still has the
   nonzero reference required by the FTD-0860 equivariance obstruction.
2. The reference does not accumulate event energy and needs no tail export.
3. The signal energy is exactly the event energy, so there is no baseline
   subtraction in its transport ledger.

This is not a new selected-type charge. It is a more explicit realization of
the already-booked `SEL-CA-PHASE-RAIL`: the retained phase reference and
directed signal transport are precisely the two resources that selection
prices.

## 6. Remaining physical boundary

The exact reference model does not establish production hardware.

1. `omega`, `I_*`, and the spatial twist remain selected initial/constitutive
   data.
2. Production has no protected reference lane or exact outward signal rail;
   C18 remains dispersive.
3. Production common-field event acceptance does not determine the relative
   on-shell exchange input.
4. The eligibility controller and its switching work remain open.
5. A genuine coupling must include reference backreaction, phase/angular
   bookkeeping, and perturbation recovery rather than treating `beta` as an
   immutable external parameter.
6. Cubic realization must distribute reference and signal modes without
   multiplying `B` across face arms.

Most importantly, harmonic phase kinematics does not select the quartic clock.
`G*` could enter only through a separately derived nonlinear reference
Hamiltonian or clock-to-phase calibration. FTD-0863 closes the **interface**,
not that gearbox.

## 7. Isolated implementation

The reference API is
[`catalytic_phase_reference.h`](../../../../../engine/include/ftd/eft/catalytic_phase_reference.h),
SHA256
`25C094B166DE32894A2FB4F0B0BCEE7A68AB279AB8C7D3BA48D4CAEE2BD4B9AB`.
Its implementation and focused test are:

- [`catalytic_phase_reference.cpp`](../../../../../engine/src/eft/catalytic_phase_reference.cpp),
  SHA256
  `77DC75A5175820EADEEC747D706C4A0B8DBA73F0F52C6420C49D2EC1F966350D`;
- [`test_catalytic_phase_reference.cpp`](../../../../../engine/tests/test_catalytic_phase_reference.cpp),
  SHA256
  `8F31273E9EEDB52768AF8FD286EF4B055158D9AFCBB6189C826B664A5281A2C9`.

The API fails closed on an empty/nonfinite reference, nonfinite signal or
matter amplitude, invalid tolerance, and invalid eligibility. The focused
Release CTest passes `1/1` and reports:

```text
FTD-0863 catalytic phase-reference EFT: PASS
scope=SEPARATE_CONSERVED_REFERENCE_PLUS_ZERO_BASELINE_SIGNAL
matter_signal_exchange=RECIPROCAL_ENERGY_EXACT
reference_action=UNCHANGED
pilot_frequency_gstar_gearbox=OPEN
production_integration=NONE
```

## 8. Certificate record

```text
FTD-0863 catalytic phase-reference transducer: 36/36 PASS
AUTONOMOUS_PHASE_REFERENCE_ROTATION_IS_REVERSIBLE_AND_ACTION_PRESERVING
REFERENCE_ORIENTS_AN_EXACT_RECIPROCAL_MATTER_SIGNAL_SWAP
ZERO_BASELINE_SIGNAL_CARRIES_EVENT_ENERGY_WITHOUT_SPENDING_PILOT_ACTION
FINITE_PERIODIC_PILOT_REQUIRES_COMMENSURATE_SPATIAL_TEMPORAL_WINDING
PILOT_FREQUENCY_GSTAR_GEARBOX_AND_PRODUCTION_REALIZATION_REMAIN_OPEN
VERDICT=OUTCOME_B_EXACT_CATALYTIC_REFERENCE_PRODUCTION_UNREALIZED
```

No `G*`, Born, Bell, Hilbert-recovery, biological, de Broglie-guidance,
thermodynamic, CM/substrate, operational-Lorentz, production, or completeness
claim is made.
