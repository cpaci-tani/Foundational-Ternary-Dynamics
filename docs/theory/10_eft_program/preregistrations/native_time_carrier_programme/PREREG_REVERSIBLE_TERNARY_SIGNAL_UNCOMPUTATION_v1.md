# PRE-REGISTRATION — Reversible ternary signal uncomputation v1

**Date locked:** 2026-08-11  
**Identifier:** `FTD-0870`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN; EXECUTION INVALID 39/40]`  
**Parents:** `FTD-0848`, `FTD-0852`, `FTD-0856`, `FTD-0869`

## 1. Question

Does the actual ternary latch require the selected nonsmooth dissipative reset
of FTD-0869, or can the already-completed outgoing signal reversibly uncompute
the latch in one discrete update while retaining the signed event record?

The discriminator must keep three levels separate:

1. exact reset of the actual ternary label;
2. continuous reset of FTD-0848's selected coordinate realization; and
3. physical transport of the retained signal away from the local port.

## 2. Frozen sources

| Source | SHA256 |
|---|---|
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_LOSS_BOOKED_TERNARY_PHASE_LATCH_v1.md` | `1C1BE138260B4CD3B639F7B6E1DB9E78886B2CCC9E6C0388CFC83E0D0FE073CA` |
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_CAUSAL_ODD_PULSE_HISTORY_CARRIER_v1.md` | `7F393F78C2572ED9C61B20D897F3786BB366B305BA831DDB6CAD42344F4131E7` |
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_MINIMUM_RECIPROCAL_RECORD_PORT_BARRIER_v1.md` | `5D13921555B2289ABC5425F4D2436545C4C3BF0638FC71C98A577E3325D215EA` |
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_SIGNAL_ACKNOWLEDGED_TWO_STROKE_RESET_AND_SMOOTH_BOUNDARY_v1.md` | `7E6B7CD0488EFE7A5A5108CEC251AD0972276F015C8F4B3F1C8F3FCEE3308B9E` |
| `engine/include/ftd/eft/signal_acknowledged_two_stroke_reset.h` | `7C2308CA97DD1ED17FF1E38FB56FAE6FC56AD7D46B4B8A13E5CB083AD42F9C7D` |

Any mismatch gives Outcome C and books no theorem.

## 3. Registered ternary uncomputation

Encode the actual ternary alphabet

\[
 \mathbb T=\{-1,0,+1\}
\]

as `Z_3` by

\[
 \zeta(0)=0,\qquad \zeta(+1)=1,\qquad \zeta(-1)=2.
\]

Let `u=d(E) in T` be the sign decoded from the completed local signal `E`.
Define ternary addition/subtraction by pullback of `Z_3`, and for the
signal-completion acknowledgement `a in {0,1}` define

\[
 U_a(s,E)=\bigl(s\ominus a\,d(E),E\bigr).       \tag{1}
\]

For `a=0`, equation (1) is identity. On the registered completed-event domain
`a=1`, `d(E)=s`, so

\[
 U_1(s,E_s)=(0,E_s).                            \tag{2}
\]

The proposed inverse is

\[
 U_a^{-1}(s',E)=\bigl(s'\oplus a\,d(E),E\bigr).\tag{3}
\]

Equations (1)--(3) are a reversible controlled subtraction, not a many-to-one
erasure. The retained signal is the workspace that makes reset injective.

## 4. Minimum-information discriminator

The certificate must distinguish:

- the bare map `s -> 0`, which is noninjective on three latch values;
- a sign-even energy-only record, which identifies `+1` and `-1` and cannot
  invert the reset; and
- the oriented signal record, whose decoder is injective on `T` and therefore
  already supplies the minimum three-valued retained information.

No new acknowledgement bit, reset-history trit, or scalar bath coordinate may
be counted if equation (3) passes.

## 5. Energy and continuous-coordinate boundary

For the FTD-0848 selected ternary potential

\[
 V_T(x)=\beta x^2(x^2-A^2)^2,
\]

the registered minima `x=sA`, `s in T`, are endpoint-degenerate:

\[
 V_T(-A)=V_T(0)=V_T(+A)=0.                      \tag{4}
\]

Equation (4) permits zero endpoint storage-energy difference, but it does not
derive a zero-work physical trajectory. The locally Lipschitz autonomous
finite-time obstruction of FTD-0869 remains binding for the continuous
coordinate `x`. Outcome A may demote the cusp bath from a logical necessity to
one optional continuous realization only; it may not erase controller/work,
native-formation, or robustness debt.

The signal energy `B=|E|^2/2` is unchanged by (1). After uncomputation, a
reciprocal swap with an empty output port may hand off `E`, leaving local
`(s,E)=(0,0)`. Protected propagation of that output remains the FTD-0852/0869
open production boundary.

## 6. Finite retained-history boundary

If output signals are represented only by a finite length-`N` ternary rail,
the rail has `3^N` configurations. A length-`T` ternary event/no-event history
has `3^T` possibilities. For `T>N`, no injection from all such histories into
the rail exists without signed tail export, recurrence/identification, or some
other retained state. Exporting scalar energy alone loses sign.

This scoped counting statement does not exclude exact-real encodings or other
alphabets. It applies only to the registered finite ternary history rail.

## 7. Frozen certificate gates

The certificate must execute exactly 40 checks:

1. all five frozen source hashes;
2. ternary encoding is bijective;
3. pulled-back addition is closed, associative, and has identity/inverses;
4. the controlled reset is total on its declared domain;
5. `a=0` is identity;
6. `a=1,d(E)=s` resets all three latch values, including both active signs;
7. the no-event zero state remains zero;
8. the signal/decoder is unchanged;
9. equation (3) is an exact two-sided inverse and the controlled map is
   bijective;
10. simultaneous sign reversal is equivariant;
11. joint latch/signal evolution is not erasure;
12. bare reset is noninjective;
13. energy-only retention cannot distinguish the two signs;
14. the oriented signal decoder is injective;
15. all-three-state reversible reset needs at least three retained labels and
    the existing signal supplies them;
16. no persistent acknowledgement bit or separate reset-history trit is
    required by the logical map;
17. one discrete update reaches exact zero without contradicting the smooth
    ODE no-go;
18. equation (4) is exact;
19. signal energy is unchanged and no logical scalar-bath term appears;
20. endpoint degeneracy is not promoted to zero physical controller work;
21. empty-port handoff returns local readiness and preserves `(s,B)`;
22. the finite ternary rail has `3^N` capacity, fails full-history injectivity
    for `T>N`, and scalar tail energy loses sign; and
23. the scope firewall retains continuous-latch dynamics, native formation,
    robust controller work, protected cubic transport, production coupling,
    `G*`, Born/Bell, Lorentz, biological, and completeness debts.

The printed check labels, not this grouped prose count, are authoritative for
the exact total of 40.

## 8. Locked implementation and outcomes

The unrun certificate is

```text
scripts/proofs/proof_reversible_ternary_signal_uncomputation.py
```

- **Outcome A — reversible actual-layer uncomputation:** all 40 gates pass.
  The signal reversibly resets the actual ternary latch with no extra reset
  state; the cusp remains only a selected continuous-coordinate realization.
- **Outcome B — extra reset state required:** the signal record is insufficient
  for an injective reset, while a separately retained reset/history state
  closes the registered map.
- **Outcome C — invalid:** any source or exact gate fails. Book no theorem; any
  repair requires a fresh lock.

Expected result: Outcome A. This expectation is frozen before first execution.

No numerical search, tolerance fit, target weight, Born/outcome probability,
remote setting, production mutation, or whole-framework completeness claim is
permitted.

## 9. Recorded outcome

The first locked execution recorded `39/40`. All five source hashes, every
ternary-group/reset/inverse/minimum-information identity, endpoint-energy and
output-handoff check, finite-rail boundary, and scope firewall passed.

C35 searched for the exact contiguous prose marker
`does not derive a zero-work physical trajectory`, while this frozen protocol
places `does not` and `derive` on adjacent Markdown lines. The registered
meaning is identical; the failure is whitespace-sensitive source-prose
verification. FTD-0870 remains execution-invalid and books no theorem. Any
repair must be separately locked and may normalize only C35's protocol
whitespace before applying the same marker.
