# V3 charged-frame payload-complete A2 work port v1

**Date:** 2026-08-24  
**Status:** **[THEOREM, CONDITIONAL ON SELECTED REPAIR — EXACT EXISTING-A2
WORK-PORT BIJECTION]** +
**[THEOREM — COMPLETE A9 PHASE/POLARITY RETENTION AND SOURCE INVISIBILITY]** +
**[THEOREM, CONDITIONAL ON SELECTED EQUAL-OCCUPANCY METRIC — EXACT FINITE
ENERGY INVARIANT]** +
**[SELECTION — TWO A2 SLOTS AND CROSS-CARRIER UNIT-ENERGY IDENTIFICATION]** +
**[REFLECTION CLOSURE — CONDITIONALLY CLOSED BY ORIENTED-HEADER SUCCESSOR]** +
**[OPEN — COMMON-ACTION DERIVATION, ABSOLUTE MULTIPLIER, CLOCK-DEBIT WORK,
ORIENTED-CHART FORMATION/ADMISSION, ARBITRATION, PHI, AND SURVIVAL]**  
**Scope:** the complete 37,632-row registered atomic repair section  
**Production status:** unchanged  
**Ledger status:** no row minted  
**Atomic parent:**
[`THEOREM_V3_CHARGED_FRAME_ATOMIC_SYNDROME_REPAIR_TRANSACTION_v1.md`](THEOREM_V3_CHARGED_FRAME_ATOMIC_SYNDROME_REPAIR_TRANSACTION_v1.md)  
**Normalization successor:**
[`THEOREM_V3_COMMON_OCCUPANCY_INVARIANT_AND_RELATIVE_NORMALIZATION_BOUNDARY_v1.md`](../common_action_mechanics_reciprocity/THEOREM_V3_COMMON_OCCUPANCY_INVARIANT_AND_RELATIVE_NORMALIZATION_BOUNDARY_v1.md)  
**Reflection successor:**
[`THEOREM_V3_ORIENTED_REPAIR_CHART_FULL_OH_COVARIANCE_AND_PRICE_v1.md`](THEOREM_V3_ORIENTED_REPAIR_CHART_FULL_OH_COVARIANCE_AND_PRICE_v1.md)  
**Exact certificate:**
[`proof_v3_charged_frame_payload_complete_a2_work_port.py`](../../../../../scripts/proofs/proof_v3_charged_frame_payload_complete_a2_work_port.py)

---

## 1. Question closed

The atomic parent tracked work only by an integer `w in {0,1,2}`. The selected
v3 carrier already contains four A9 slots on every plaquette A2 cell, where

\[
 \mathcal A_9=\{\varnothing\}\cup
 \bigl(\mathbb Z_4\times\{\!-1,+1\}\bigr),
 \qquad |\mathcal A_9|=9.                              \tag{1}
\]

Select two of those existing slots as the repair work port:

\[
 W=\mathcal A_9^2,
 \qquad |W|=81.                                        \tag{2}
\]

No scalar counter or new primitive type is introduced. Both occupied states
retain complete C4 phase and polarity.

The ready port is

\[
 W_0=((k_x,\epsilon_x),\varnothing),                    \tag{3}
\]

where the payload is read from the charged parent frame. Native formation of
equation (3) remains open.

---

## 2. Complete finite work-port map

Let `Delta=N_obj(x)-N_obj(y)` be the atomic parent's object-record difference.
The work-port output is selected as

\[
 W'(y)=
 \begin{cases}
  (\varnothing,\varnothing),&\Delta=+1,\\
  ((k_x,\epsilon_x),\varnothing),&\Delta=0,\\
  ((k_x,\epsilon_x),(k_f,\epsilon_f)),&\Delta=-1,
 \end{cases}                                           \tag{4}
\]

where `(k_f,epsilon_f)` is the complete phase/polarity payload of the extra
field record in the last branch. The syndrome retains the field slot's tangent,
normal, hand, position, and all other defect coordinates. Thus no missing
coordinate is inferred from an anonymous energy count.

Composing equation (4) with the atomic repair gives

\[
 (y,p_{\rm in},S_0,W_0)
 \longleftrightarrow
 (\Phi x,p_{\rm out},S_{d_x(y)},W'(y)).                \tag{5}
\]

The certificate proves that the 37,632 inputs and 37,632 outputs are disjoint
sections and that equation (5) is a bijection between them. Declaring the
reverse arrow on the output section gives an exact finite involution. The
syndrome plus exact next frame reconstructs the complete input, including any
coordinate not present in the final work payload.

The exact occupancy census is:

| object-record difference `Delta` | rows | occupied A9 work slots after repair |
|---:|---:|---:|
| `+1` | 480 | 0 |
| `0` | 672 | 1 |
| `-1` | 36,480 | 2 |

All eight nonblank A9 payloads occur. The work port is an A2 factor, not an SC
source relation or a site field slot, so changing its occupancy leaves the
registered charged Gauss presentation unchanged.

---

## 3. Selected occupancy-energy invariant

Assign one energy unit to every occupied object relation, object field slot,
syndrome field slot, and A9 work slot:

\[
 H_{\rm occ}
 =N_{\rm relation}+N_{\rm field}+N_{\rm syndrome}
  +N_{\rm work}.                                       \tag{6}
\]

Every syndrome has eighteen occupied records. Equation (4) then gives, on
every registered transaction,

\[
 \boxed{H_{\rm occ}^{\rm before}=H_{\rm occ}^{\rm after}.} \tag{7}
\]

Because equation (5) is bijective, equation (7) is an exact invariant of the
selected finite repair permutation. It upgrades the parent's integer ledger
to an actual finite-carrier occupancy invariant.

Equation (7) is not yet a native energy derivation. The normalization
successor proves that the selected absorption and complete repair transactions
force equal relative cost for SC A9 occupancy, site-field occupancy,
neutral-syndrome occupancy, and the A2 work port within the homogeneous
additive phase-blind ansatz. A second selected FCC/A2 incidence bridge extends
that conditional equality to FCC A1 occupancy. Neither theorem makes the
invariant a variational generator, fixes the positive multiplier `Gamma`, or
prices the eighteen-record clock stall. Therefore neither a physical mass nor
the electromagnetic coupling follows from equation (7).

---

## 4. Exact result versus remaining action debt

The following are now exact for the registered shell:

1. the work count is represented by finite existing A9 states;
2. every occupied work token retains phase and polarity;
3. the complete repair is a finite bijection with an explicit inverse;
4. A2 work occupancy is source-invisible to the charged SC Gauss record; and
5. one selected positive occupancy metric is exactly conserved.

The remaining stable-matter debts are:

1. derive or falsify the conditionally forced equal-occupancy metric from the
   same common action that governs the field and charged frame;
2. fix its absolute multiplier and its relation to packet/clock energy;
3. price reciprocal work for the eighteen-record syndrome clock debit;
4. integrate the successor's conditionally closed improper-reflection chart
   and close overlapping-event arbitration;
5. form the ready A2 port, syndrome bundle, and route natively; and
6. integrate the transaction into homogeneous `Phi` and prove repeated
   survival, scattering, mass, and dispersion.

Accordingly this is a **payload-complete finite repair/action seam**, not a
stable particle theorem.

---

## 5. Reproduction

```bash
python scripts/proofs/proof_v3_charged_frame_payload_complete_a2_work_port.py
```

Expected result: `11/11` exact checks pass, with 37,632 bijection rows and A2
occupancy census `{(+1,0):480, (0,1):672, (-1,2):36480}`.
