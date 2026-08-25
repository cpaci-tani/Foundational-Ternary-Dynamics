# V3 charged-frame unique one-defect decoder v1

**Date:** 2026-08-24  
**Status:** **[THEOREM — EXACT MIXED-CARRIER CODE AND UNIQUE ONE-DEFECT DECODER]** +
**[SELECTION — NONINJECTIVE REPAIR PROJECTION]** +
**[CONDITIONAL — ONE-TICK KINEMATIC PERTURBATION BASIN]** +
**[OPEN — PHYSICAL ACTION/ENERGY, REFLECTION CLOSURE, FORMATION,
ARBITRATION, PHI INTEGRATION, SCATTERING, AND MASS]**  
**Scope:** the 24 exact circulation-frame presentations of the charged
Phi-v3 candidate at one fixed anchor  
**Production status:** unchanged  
**Ledger status:** no row minted  
**Parent boundary:**
[`THEOREM_V3_CHARGED_FRAME_MATTER_PERTURBATION_BOUNDARY_v1.md`](THEOREM_V3_CHARGED_FRAME_MATTER_PERTURBATION_BOUNDARY_v1.md)  
**Reversible successor:**
[`THEOREM_V3_REVERSIBLE_SYNDROME_CONVEYOR_AND_BORN_MEASURE_BOUNDARY_v1.md`](THEOREM_V3_REVERSIBLE_SYNDROME_CONVEYOR_AND_BORN_MEASURE_BOUNDARY_v1.md)  
**Atomic successor:**
[`THEOREM_V3_CHARGED_FRAME_ATOMIC_SYNDROME_REPAIR_TRANSACTION_v1.md`](THEOREM_V3_CHARGED_FRAME_ATOMIC_SYNDROME_REPAIR_TRANSACTION_v1.md)  
**Work-port successor:**
[`THEOREM_V3_CHARGED_FRAME_PAYLOAD_COMPLETE_A2_WORK_PORT_v1.md`](THEOREM_V3_CHARGED_FRAME_PAYLOAD_COMPLETE_A2_WORK_PORT_v1.md)  
**Exact certificate:**
[`proof_v3_charged_frame_unique_one_defect_decoder.py`](../../../../../scripts/proofs/proof_v3_charged_frame_unique_one_defect_decoder.py)

---

## 1. Presentation metric

Write a charged-frame presentation as

\[
 X=(R_X,F_X,L_X),                                      \tag{1}
\]

where `R` is the keyed A9 relation map, `F` is the set of occupied field
slots, and `L` is the keyed C3-layer map. Define the mixed-coordinate Hamming
metric

\[
 d(X,Y)=
 \#\{k:R_X(k)\ne R_Y(k)\}
 +|F_X\triangle F_Y|
 +\#\{x:L_X(x)\ne L_Y(x)\}.                           \tag{2}
\]

Missing keyed records count as unequal values. The 24 exact period-four
presentations form a finite code `C` in this metric. Complete pairwise
enumeration gives

\[
 \min_{X\ne Y\in C}d(X,Y)=36.                         \tag{3}
\]

This large separation is a property of the registered finite presentation;
it is not yet a binding energy or a dynamical barrier.

---

## 2. Unique one-defect theorem

The parent certificate exhausts the full registered one-coordinate shell:

| defect class | exact rows |
|---|---:|
| delete one relation | 96 |
| flip one relation owner | 96 |
| change one relation phase | 288 |
| flip one relation polarity | 96 |
| delete one required field record | 384 |
| add one unowned local field record | 36,480 |
| change one vertex layer | 192 |
| **total** | **37,632** |

Every row has distance one from exactly one member of `C`. Therefore the
state-only decoder

\[
 D(Y)=X \quad\Longleftrightarrow\quad
 X\in C,\ d(X,Y)\le1,\text{ and }X\text{ is unique}    \tag{4}
\]

is defined on the entire registered one-defect shell and returns its unique
parent. Blank and an explicit distance-two deletion control fail closed.

The earlier zero-radius result remains true: the *original charged macro*
rejects all these defects. Equation (4) is a newly selected decoder, not a
reinterpretation of that macro.

---

## 3. Candidate repair tick

A minimal state-level extension can act as follows:

- exact codewords execute the existing charged period-four tick;
- uniquely decoded one-defect states project to their parent codeword; and
- all other states fail closed to the existing fallback.

Under this selected projection every registered one-defect presentation
enters the exact recurrent family after one repair tick. This is an exact
one-step **kinematic basin**. It is stronger than recurrence of one prepared
orbit, but it is not yet a physical stability theorem.

---

## 4. Exact resource price

The projection changes finite owned records. Across the complete shell its
relation/field count deltas are

| repair delta `(relations, fields)` | rows | required interpretation |
|---|---:|---|
| `(1,0)` | 96 | supply one missing A9 relation |
| `(0,1)` | 384 | supply one missing field record |
| `(0,-1)` | 36,480 | expire one extra field record |
| `(0,0)` | 672 | erase and replace a payload or layer value |

The map is many-to-one: 37,632 defect states project onto 24 parents. A
reversible common action therefore cannot implement it as bare erasure. The
successor theorems construct the exact finite physical syndrome conveyor, an
atomic repair transaction, and a two-slot existing-A2 work port that retains
the defect identity for any registered survival horizon. In the reversible
branch the remaining physical realization must provide at least:

1. common-action derivation of the selected equal-occupancy energy and its
   absolute multiplier;
2. reciprocal work assigned to the bundle clock debit;
3. native formation and multi-event arbitration; and
4. homogeneous Phi integration. Retained inverse information is now closed
   for the registered shell.

Those are construction requirements for the reversible branch. The v3
constitution also permits genuine noninjective expiry; that alternative may
discard the syndrome, but it must be labeled nonreversible and must still
price record supply/removal and work.

---

## 5. Matter verdict

The charged frame now has an exact state-only predicate, exact recurrence,
and an exact unique decoder on its entire registered one-coordinate shell.
This licenses **conditional one-defect kinematic robustness**.

It does not license stable matter. A finite reversible lift, physical neutral
syndrome carrier, atomic repair/syndrome emission transaction, payload-complete
A2 work port, selected conserved occupancy energy, and arbitrary registered
kinematic survival horizon now exist. Common-action derivation/normalization,
clock-debit work, full reflection closure, formation, arbitration,
homogeneous Phi integration, and multi-object survival with mass/dispersion
remain open. The alternative noninjective-expiry branch is likewise
unintegrated. Stable matter remains open at that sharper implementation gate.

---

## 6. Reproduction

From the repository root:

```bash
python scripts/proofs/proof_v3_charged_frame_unique_one_defect_decoder.py
```

Expected result: `10/10` checks pass, minimum codeword distance `36`, and
`37,632` unique decoded one-defect states.
