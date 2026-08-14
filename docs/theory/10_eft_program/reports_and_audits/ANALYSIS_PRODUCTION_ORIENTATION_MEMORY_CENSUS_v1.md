# FTD-0910 — Production orientation-memory census result v1

**Identifier:** `FTD-0910`  
**Date:** 2026-08-11  
**Status:** `[MEASURED — FTD-0908 OUTCOME A: PERSISTENT SIGN-INTERVAL CANDIDATES]`
`+ [BOUNDARY — PAIR-SPECIFIC/PROTECTED MEMORY NOT ESTABLISHED]`
`+ [POST-HOC DIAGNOSTIC — ROTATED NULL MATCHES ARM PASS PATTERN]`  
**Governing protocol:** FTD-0908, SHA-256
`53348A90021C609E3EBA5DC7D565F6EA78832498C206D0D4B3F1964CCC7C4993`  
**Instrument lock:** FTD-0909, SHA-256
`41034D7E2CE032D053BB98E223973864D53EB24995B7FD7B764E7EF5A9F0A355`

## 1. Verdict

The frozen FTD-0908 campaign is protocol-valid and reaches its preregistered
**Outcome A**:

```text
PRODUCTION_FORMATION_VERDICT=CROSS_VOLUME_PERSISTENT_ORIENTATION_MEMORY_CANDIDATES
```

All six live family-by-volume cells meet the `>=3/4` seed gate. The empty
controls form no valid pair. This establishes that unchanged production
genesis/coupling dynamics can form durable neutral Moore-pair carriers with a
native dipole axis and nonzero time-odd phase-wedge sign for preregistered
eight-tick intervals.

It does **not** establish a protected or pair-specific memory bit. Every one
of the 78 actual production-ID pairs changes chirality during its observed
life, and a post-hoc persistence reconstruction of the frozen rotated-
negative null reaches the same seed-pass pattern in every family-by-volume
cell. The correct physical reading is therefore:

> Production supplies durable carrier hardware and persistent local
> clockwise/counterclockwise **intervals**. It has not supplied the restoring
> gearbox that binds one chirality to the carrier as protected memory.

## 2. Locked execution and corpus

The source/protocol preflight passed `30/30` immediately before execution.
The runner/adjudicator preflight passed `28/28`. Focused CTest
`production_orientation_memory_census` passed `1/1` in `44.46 s`; no other
CTest target was executed. The independent frozen adjudicator passed `19/19`
and reconstructed the same Outcome A.

| Corpus artifact | Bytes | SHA-256 |
|---|---:|---|
| `engine/results/ftd_0908/ftd_0908_pair_observations_v1.csv` | 2,780,021 | `9AC0FFED497615362D26C4D2BE1E295CEE7BEDFB445AB92B5F53B5F9225FCB07` |
| `engine/results/ftd_0908/ftd_0908_tick_census_v1.csv` | 644,842 | `16AF93BF2AE469F219BEEDBF6FBF001D27FD097C33C9EC91995A4FFF1CB0A3B7` |
| `engine/results/ftd_0908/ftd_0908_summary_v1.json` | 17,645 | `F6BDEB2C033C0351B97E1ECDA56EB998D8563649E4C0DD00A770A52F9676F775` |

The frozen runner source is `4FBA0AF9...B9F34`; the independent adjudicator
is `26FD25DA...A6A`. The post-data diagnostic audit is
`scripts/verification/audit_production_orientation_memory_census_null.py`,
SHA-256 `3C40C958F3DC7774F276ECF58513EB018873F17D692F5E304D6E2BFAC455158A`.
That audit is explicitly post-hoc and cannot redefine FTD-0908.

## 3. Frozen arm result

| L | Family | Seeds with run `>=8` | Seed maxima | Persistent actual IDs | Valid observations |
|---:|---|---:|---|---:|---:|
| 17 | `axial_live` | 4/4 | 21, 17, 25, 23 | 11 | 973 |
| 17 | `diagonal_live` | 4/4 | 12, 13, 27, 32 | 13 | 1,224 |
| 17 | `axial_no_bath` | 4/4 | 22, 21, 26, 56 | 16 | 1,449 |
| 17 | `empty_control` | 0/4 | 0, 0, 0, 0 | 0 | 0 |
| 25 | `axial_live` | 4/4 | 26, 19, 17, 26 | 9 | 769 |
| 25 | `diagonal_live` | 3/4 | 29, 17, 0, 36 | 12 | 979 |
| 25 | `axial_no_bath` | 4/4 | 23, 26, 22, 36 | 14 | 1,332 |
| 25 | `empty_control` | 0/4 | 0, 0, 0, 0 | 0 | 0 |

Aggregate frozen observations:

- `6,726` valid pair-tick observations;
- `78` distinct production-ID pairs;
- `75/78` pairs contain at least one sign-stable run of eight ticks;
- persistent-run minimum/median/maximum `8 / 21 / 56` ticks;
- `3,318` positive- and `3,408` negative-chirality observations;
- first pair appearance range `0--17`, median tick `3`;
- no empty-control genesis, valid pair, or wedge observation; and
- all pre/post voxel and RNG hashes match at every observation.

The no-bath arms pass at both volumes, so the Langevin bath is not necessary
for carrier formation or the frozen interval criterion. The injected field
and live genesis/coupling stack are necessary within this matrix because all
eight empty-control arms remain empty.

## 4. What Outcome A does and does not mean

The preregistered observable is

\[
 d=x_+-x_-,\qquad
 \ell=q_+p_- - q_-p_+,\qquad
 \chi=\operatorname{sgn}\ell .
\]

The campaign closes three limited existence questions:

1. production can form a neutral adjacent actual-state carrier with stable
   production identity;
2. existing endpoint flux and wave-velocity data distinguish the two
   orientation branches at each valid tick; and
3. one branch can persist longer than the frozen transient threshold.

It does not show that `ell` is conserved. Across the 78 gap-free pair
histories there are `1,779` consecutive-tick sign changes; every identity
changes sign at least once. The median identity has `25` such changes. Thus
the actual matter carrier is durable while its projected phase-wedge branch
is dynamically labile.

This distinction is the missing dynamics in concrete form. A discriminator
answers “clockwise or counterclockwise now.” A memory must additionally
explain why that branch persists, how a perturbation is corrected, where the
maintenance work comes from, and how erasure exports information/energy.
FTD-0908 tests only the discriminator plus finite intervals.

## 5. Post-hoc null diagnosis

FTD-0908 froze a rotated-negative null as descriptive only. The runner stored
its per-tick nonzero count and maximum magnitude, not a null persistence
verdict. Those nonzero counts equal the actual counts on every positive-pair
tick, showing that nonzero wedge alone is generic under the observed fields.

After the corpus was frozen, the stored endpoint fields permitted a fully
reproducible diagnostic: retain the runner's one-place rotated negative
endpoint, assign the pseudo-pair its positive and rotated-negative IDs, and
compute its maximum sign-stable run. This gives:

| L | Family | Actual seed passes | Null seed passes | Actual persistent IDs | Null persistent pseudo-IDs |
|---:|---|---:|---:|---:|---:|
| 17 | `axial_live` | 4 | 4 | 11 | 11 |
| 17 | `diagonal_live` | 4 | 4 | 13 | 11 |
| 17 | `axial_no_bath` | 4 | 4 | 16 | 15 |
| 25 | `axial_live` | 4 | 4 | 9 | 9 |
| 25 | `diagonal_live` | 3 | 3 | 12 | 12 |
| 25 | `axial_no_bath` | 4 | 4 | 14 | 11 |

The totals are `75` actual persistent IDs versus `69` null pseudo-IDs. No
inference is licensed from that numerical difference: a null-persistence
decision rule was not preregistered. What is licensed is the boundary that
the actual arm-level success pattern is not pair-specific under the one null
available in this corpus. The null diagnosis cannot retroactively erase the
valid Outcome A, but it blocks promotion from “candidate interval” to
“protected pair memory.”

## 6. Physical interpretation

The simplest reading is a two-part substrate:

- the manifested `+/-` pair is the durable **matter-like skeleton**; and
- the endpoint flux/wave phase is a continuously evolving **orientation
  signal** on that skeleton.

This is more than matter alone because the actual-state pair does not contain
the chirality; the phase wedge does. But it is less than a self-dual recursive
system because production does not yet bind the signal to one branch. The
two sides exist and alternate; a natural coupling that turns alternating
phase into retained state, with a work and reversal ledger, is still missing.

No G* clock claim follows. The census reads no G*, period, context, outcome,
or Born weight. It shows local clock/memory *hardware capacity*, not the
gearbox identifying that hardware with the global CM/G* calendar.

## 7. Next locked gate

The next campaign must be held out and must make pair specificity load-
bearing before any perturbation or energy claim:

1. freeze actual-versus-rotated persistence statistics and exact decision
   rules on new seeds/volumes;
2. record the discrete `Delta ell` ledger and test central-force/torque
   residuals rather than assuming the FTD-0907 central Hamiltonian;
3. distinguish ordinary oscillatory sign autocorrelation from carrier-bound
   chirality using time-shifted, endpoint-rotated, and sign-reversed controls;
4. only if pair specificity survives, apply preregistered phase and amplitude
   perturbations to twins and measure recovery, controller work, dissipation,
   and erasure; and
5. couple a surviving protected mode to the separate FTD-0904 rectifier and
   G* clock only in a later protocol.

```text
FROZEN_OUTCOME=A
PROTOCOL_VALID=TRUE
INDEPENDENT_ADJUDICATION=19/19
PRODUCTION_CARRIER_FORMATION_MEASURED=TRUE
PERSISTENT_SIGN_INTERVALS_MEASURED=TRUE
EMPTY_CONTROL_FORMATION=FALSE
ALL_ACTUAL_IDENTITIES_EVENTUALLY_FLIP_CHIRALITY=TRUE
POST_HOC_NULL_ARM_PATTERN_MATCH=TRUE
PAIR_SPECIFIC_MEMORY_ESTABLISHED=FALSE
PROTECTED_RECURSIVE_MEMORY_ESTABLISHED=FALSE
CENTRAL_MEMORY_LAW_TESTED=FALSE
PERTURBATION_RECOVERY_TESTED=FALSE
MAINTENANCE_ERASURE_WORK_CLOSED=FALSE
GSTAR_GEARBOX_IDENTIFIED=FALSE
BORN_OR_CONTEXT_READ=FALSE
NO_NEW_SELECTED_TYPE=TRUE
```
