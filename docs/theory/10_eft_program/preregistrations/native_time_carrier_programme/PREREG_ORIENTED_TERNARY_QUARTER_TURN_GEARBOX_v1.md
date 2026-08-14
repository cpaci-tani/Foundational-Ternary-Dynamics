# FTD-0872 — Oriented ternary quarter-turn gearbox v1

**Identifier:** `FTD-0872`  
**Status:** `[PRE-REGISTRATION — LOCKED/PRE-RUN]`  
**Date:** 2026-08-11  
**Parents:** `FTD-0836`, `FTD-0856`, `FTD-0867`, `FTD-0871`  
**Production status:** unchanged; isolated exact actual-layer mathematics only

## 1. Registered question

FTD-0871 proves that a completed oriented signal can uncompute a matching
ternary latch, but leaves the physical controlled permutation open. Does the
actual ternary pair itself admit a minimum reversible, energy-preserving,
orientation-retaining transfer law that sends a ready source directly into an
empty output port?

The registered scope is deliberately narrower than a production mechanism.
It tests the exact finite-state gearbox and its backpressure boundary. It does
not test a continuous controller, a protected spatial rail, production
coupling, or a `G*` cadence.

## 2. Frozen sources

| Source | SHA-256 |
|---|---|
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/DERIV_BILATERAL_SELF_DUAL_QUARTIC_CLOCK_v1.md` | `779044879BB28CE0DB13BA8783EC7FF9AB5DFDFE10DF1C259D3D11998DEEDB9A` |
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_MINIMUM_RECIPROCAL_RECORD_PORT_BARRIER_v1.md` | `5D13921555B2289ABC5425F4D2436545C4C3BF0638FC71C98A577E3325D215EA` |
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_TERNARY_ELIGIBILITY_CLUTCH_AND_ONE_SHOT_HANDSHAKE_v1.md` | `6BD280A51DEF9A1B5E373D0084A9C19597772CD31D2B5D278B2323315AC2153D` |
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_REVERSIBLE_TERNARY_SIGNAL_UNCOMPUTATION_AND_RESET_BOUNDARY_v1.md` | `F52BE0CD97FAE06CF6A39C6E0784EC75746F7B8ABF9843C4EF78B37181C8D2CC` |
| `engine/include/ftd/eft/reciprocal_record_port.h` | `5973BF10BCE122304368E3BD191EA810D3DD6AB106B69B9D9022F662136D2B08` |
| `engine/include/ftd/eft/ternary_eligibility_clutch.h` | `C53ED1A7FCFF54E4236D2353CA319BCE61EC459C1A7A90F2069C01145256FE43` |
| `engine/include/ftd/eft/reversible_ternary_signal_uncomputation.h` | `2596A4873D957E43FFFA25DEDF984F7EF3D1146307DEF765164C72FCB22A65AD` |

Any mismatch invalidates the run. The proof may read these sources and this
protocol only. It may not modify a source or infer a production result from an
isolated reference calculation.

## 3. Frozen algebra

Use the FTD-0871 encoding of the ternary alphabet as the field

\[
 \mathbb T\cong\mathbb F_3,
 \qquad 0\mapsto0,\quad +1\mapsto1,\quad -1\mapsto2.
\]

Let the ordered actual-layer pair be

\[
 z=\binom{s}{o}\in\mathbb F_3^2,
\]

where `s` is the local latch and `o` is its oriented output port. Register the
sign-preserving transfer quarter-turn

\[
 R=\begin{pmatrix}0&-1\\1&0\end{pmatrix},
 \qquad R(s,o)=(-o,s).                         \tag{1}
\]

This is the inverse orientation of the matrix called `J` in FTD-0836. The
choice is not a second complex structure: it is the branch that respects the
registered output-sign convention

\[
 R(s,0)=(0,s).                                 \tag{2}
\]

For the binary eligibility value `a in {0,1}`, define

\[
 G_a=(1-a)I+aR.                                \tag{3}
\]

Use the exact ternary representative energy

\[
 Q(s,o)=s^2+o^2,                               \tag{4}
\]

and ordered one-step area

\[
 \chi_R(z)=\det[z,Rz].                         \tag{5}
\]

No numerical tolerance, fitted coefficient, or target value enters the
registered algebra.

## 4. Registered theorem gates

The source-locked certificate must report exactly forty checks.

### Provenance

- **C1--C7:** the seven source hashes equal the frozen values above.
- **C8:** this protocol hash equals the pre-run lock embedded in the frozen
  certificate before its first execution.

### Exact group and energy law

- **C9:** the displayed ternary encoding is a bijection.
- **C10:** equation (1) agrees with modular ternary evaluation on all nine
  ordered pairs.
- **C11:** `R^2=-I` over `F_3`.
- **C12:** `R^4=I` over `F_3`.
- **C13:** `det(R)=+1` over `F_3`.
- **C14:** `R^T R=I` over `F_3`.
- **C15:** `Q(Rz)=Q(z)` for all nine ternary pairs using canonical
  representatives.
- **C16:** `G_0=I`.
- **C17:** `G_1=R`.
- **C18:** the registered inverse of `G_0` is identity.
- **C19:** the registered inverse of `G_1` is `R^-1=-R`.
- **C20:** `G_0` permutes all nine states.
- **C21:** `G_1` permutes all nine states.

### Event transfer, orientation, and minimum

- **C22:** `R(s,0)=(0,s)` for all three latch values.
- **C23:** `R^-1(0,s)=(s,0)` for all three latch values.
- **C24:** the no-event state `(0,0)` is fixed.
- **C25:** sign reversal commutes with both `R` and `R^-1`.
- **C26:** for every nonzero `z`, `chi_R(z)=Q(z)>0` as an ordinary integer.
- **C27:** for every nonzero `z`, `chi_{R^-1}(z)=-Q(z)<0`.
- **C28:** `Sym^2(R)=Sym^2(-R)`, so the ordered lift retains a sign that its
  symmetric square loses.
- **C29:** exhaustive enumeration of all `3^4` two-by-two matrices over
  `F_3` finds exactly one matrix satisfying `M e1=e2`, `det(M)=1`, and
  `M^T M=I`; it is `R`.
- **C30:** the unsigned swap also sends `e1` to `e2` but has determinant
  `-1`, proving why orientation preservation is a real discriminator.

### Backpressure and scope boundary

- **C31:** the rule “apply `R` only when `o=0`, otherwise identity” maps
  `(1,0)` and `(0,1)` to the same output.
- **C32:** that conditional ready-port rule is therefore noninjective.
- **C33:** the all-domain controlled maps `G_0` and `G_1` remain bijections;
  readiness is a registered operating subspace, not a hidden fail-closed
  mutation of `R`.
- **C34:** the joint gate has nine inputs and nine outputs and performs no
  logical erasure.
- **C35:** on the ready-event subspace, the endpoint value of `Q` is unchanged.
- **C36:** the protocol contains `CONTROLLER_WORK_STATUS=OPEN`.
- **C37:** the protocol contains `PRODUCTION_COUPLING=NONE`.
- **C38:** the protocol contains `GSTAR_ROLE=NOT_DERIVED`.
- **C39:** the protocol contains `BORN_BELL_STATUS=UNTOUCHED`.
- **C40:** the terminal verdict is emitted only when C1--C39 all pass.

## 5. Frozen interpretation

If all forty gates pass, the permitted result is:

- **[THEOREM]** `R` is the unique orientation-preserving ternary isometry
  carrying a ready latch into its output port with sign retained.
- **[THEOREM]** the actual-layer event transfer is an order-four reversible
  quarter-turn with exact endpoint energy closure.
- **[THEOREM]** a naive conditional “empty-port or identity” wrapper is not
  reversible; a physical implementation must schedule readiness or retain a
  reciprocal/reflected output.
- **[SYNTHESIS]** `R^2=-I` identifies the already-adopted two-register complex
  structure with the actual-layer transfer operation. It does not derive
  complex potentiality, Hilbert space, or the Born rule.
- **[OPEN]** continuous actuation, controller work, protected cubic transport,
  native source/reference formation, production coupling, robustness, and
  `G*` synchronization.

The result may be composed with the already-selected FTD-0852/0855 rail and
FTD-0867 eligibility interfaces, but their physical realization is not
promoted by this certificate.

## 6. Frozen outcome rule

- **Outcome A:** `40/40`; book the exact scoped theorem and an isolated
  `ftd::eft` witness.
- **Outcome B:** all provenance checks pass but any mathematical gate fails;
  book the counterexample and no theorem.
- **Execution invalid:** a source/hash mismatch, exception, wrong check count,
  or protocol-marker failure; preserve the run and preregister any repair.

No post-run tolerance change, source substitution, or reinterpretation is
permitted.

## 7. Scope markers

```text
CONTROLLER_WORK_STATUS=OPEN
PRODUCTION_COUPLING=NONE
GSTAR_ROLE=NOT_DERIVED
BORN_BELL_STATUS=UNTOUCHED
```

## 8. Pre-run lock

The exact SHA-256 of this protocol must be embedded in the certificate and
recorded in the manifest before first execution. Later outcome prose does not
alter the evidence hash used by that frozen run.
