# FTD-0908 — Production neutral-dipole/phase-wedge formation census v1

**Identifier:** `FTD-0908`  
**Date locked:** 2026-08-11  
**Status:** `[PRE-REGISTRATION — LOCKED/PRE-RUN]`  
**Scope:** observation-first production census of the FTD-0907 native
orientation-memory observables; no production-tick modification

## 1. Question

Does the unchanged production `RenderBridge` tick form local, neutral,
opposite-state pairs whose native ternary dipole and bilateral projected
flux/wave-velocity wedge remain nonzero with one chirality sign for more than
a transient?

This campaign tests formation and persistence only. It cannot by itself
establish the imposed FTD-0907 central memory Hamiltonian, exact conservation
of the wedge, maintenance/erasure work, coupling to the FTD-0904 rectifier,
or synchronization to the separate G* clock.

## 2. Frozen production sources

The following SHA-256 locks are taken before runner implementation and before
any campaign execution:

| Source | SHA-256 |
|---|---|
| `engine/include/ftd/voxel.h` | `8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3` |
| `engine/include/ftd/render_bridge.h` | `560CB59E2FCD6E174640CA6BF048FD16AEC36AD2B13EE97FA31E301CF373D91C` |
| `engine/src/render_bridge.cpp` | `BFAD7886CB83A590F0AACA11C03CE25B1FF51D94B4C17B06F5D555E46C18D724` |
| `engine/src/render_bridge_phases/phase_write.cpp` | `2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4` |
| `engine/include/ftd/constants.h` | `5C9E4EA46DE1D5E0BF4479AA9E115520E70B729E7E81335FCEF08CE99704BAB0` |
| `engine/include/ftd/eft/native_ternary_dipole_phase_wedge_memory.h` | `BADAE9D26E5FED6FCD4317A7534648256AFF051E2CAADB7E6BEEA00603AEDF46` |
| `engine/src/eft/native_ternary_dipole_phase_wedge_memory.cpp` | `AA021926D1DE32AE9D04FB72682379DBB7F6CD3A1BB150AADBA6A957DFBF20B5` |

Any production-source drift before execution invalidates this protocol or
requires an explicitly versioned source-drift repair. No physics threshold
may change inside such a repair.

## 3. Frozen observer

At the end of every complete production tick, enumerate every unordered pair
of currently manifested sites satisfying all of:

1. one endpoint has state `+1` and the other `-1`;
2. their periodic minimum-image Chebyshev separation is exactly one, so the
   pair occupies one Moore-neighbour edge;
3. both endpoints have nonnegative production `particle_id` values; and
4. the FTD-0907 analyzer returns `Valid` at relative endpoint positions with
   its default tolerance `1e-11`.

The positive endpoint fixes the orientation. For each valid pair record

\[
d=x_+-x_-,\qquad e=d/|d|,
\]

\[
q_\pm=e\cdot J_\pm,
\quad p_\pm=e\cdot W_\pm,
\quad \ell=q_+p_- - q_-p_+,
\quad \chi=\operatorname{sgn}(\ell).
\]

The tracking key is the ordered production-ID pair
`(particle_id_plus,particle_id_minus)`, never a fitted spatial track. A
**sign-stable run** is a maximal sequence of consecutive complete ticks on
which the same key remains a valid Moore pair and retains the same nonzero
`chi`. The preregistered persistence threshold is eight consecutive ticks.
This is a candidate-memory threshold, not a theorem of stability.

The observer also records per tick: total `+/-` counts, genesis and
evaporation events, valid-pair count, both chirality counts, maximum and RMS
`|ell|`, total native wave energy
`sum_x (|J_x|^2+|W_x|^2)/2`, and the longest current sign-stable run.

The observer is read only. It may not write a `Voxel`, toggle, RNG state,
clock, history journal, controller, or outcome record.

## 4. Frozen arms

All arms force the CPU backend, strict toggle validation, `L in {17,25}`,
`96` complete ticks, and seeds

```text
0x09080001, 0x09080002, 0x09080003, 0x09080004.
```

The production families are inherited from the canonical FTD-0267 genesis
trajectory stack rather than tuned to the FTD-0907 observable:

| Family | Toggles and initial data |
|---|---|
| `axial_live` | wave propagation, Gauss projection, genesis, coupling, and Langevin on; `T=0.005`, `gamma=0.02`; center injection `(10 K_GENESIS,0,0)` |
| `diagonal_live` | same, with center injection `10 K_GENESIS (1,1,1)/sqrt(3)` |
| `axial_no_bath` | same axial injection and coupling, but Langevin off |
| `empty_control` | same live toggles but no injected field |

No genesis threshold, manifestation scale, drain fraction, evaporation rate,
time step, SOR tolerance, or engine constant is overridden. The first
campaign corpus therefore contains `2 volumes x 4 seeds x 4 families = 32`
arms.

## 5. Frozen controls

For every valid observed pair, the runner must execute observer-only controls
without advancing or mutating production state:

- a fixed signed-cubic transform of positions, `J`, and `W` must transform
  `e` covariantly and preserve `ell`;
- inversion of positions, `J`, and `W` must reverse `e` and preserve `ell`;
- canonical time reversal `W -> -W` must reverse `ell` and `chi`;
- the symmetric dipole square must be identical after `d -> -d`;
- the bilateral Gram determinant must equal `ell^2`; and
- the complete canonical reversal of the FTD-0840 one-step swept-area probe
  must leave that swept area invariant.

A deterministic randomized null is also frozen. At each tick, sort all valid
negative endpoints by particle ID, rotate their `(J,W)` data by one place,
and recompute wedges against the unchanged positive endpoints. This null is
descriptive only; it cannot set a threshold or redefine a successful pair.

## 6. Adjudication

Protocol validity requires:

- every frozen source hash matches;
- all 32 arms execute all 96 ticks with finite telemetry;
- the observer never mutates the production state (pre/post state hashes at
  each observation are equal);
- all algebraic controls pass at `256 x 10^-11` relative tolerance; and
- each record is independently reconstructible from the stored endpoint
  fields and IDs.

If validity fails, the only verdict is
`PROTOCOL_INVALID_NO_FORMATION_VERDICT`.

Otherwise:

- **Outcome A — cross-volume persistent candidates:** in each live injected
  family (`axial_live`, `diagonal_live`, `axial_no_bath`) and each volume, at
  least three of four seeds contain at least one sign-stable run of eight or
  more ticks.
- **Outcome B — formation without cross-volume persistence:** at least one
  live injected arm contains a valid nonzero-wedge pair, but Outcome A fails.
- **Outcome C — no observed local formation:** no live injected arm ever
  contains a valid nonzero-wedge Moore pair.

The empty control is reported but does not change A/B/C. Any manifested state
in that no-injection family is a production/RNG diagnostic, not positive
memory evidence.

## 7. Promotion firewall

Even Outcome A licenses only
`[MEASURED — PRODUCTION PERSISTENT ORIENTATION-MEMORY CANDIDATES]`.
It does not license `[EMERGENT — STABLE RECURSIVE MEMORY]` because the central
law, attraction basin, held-out perturbation recovery, and work/erasure
ledger are not tested here. A separate preregistered successor must test
those properties only after this observation-first census.

The runner and adjudicator may not read `G*`, target periods, measurement
context, selector state, outcomes, Born weights, or the desired A/B/C label.
No numerical near-miss search, parameter sweep, post-data threshold change,
or formula substitution is permitted.

```text
PRODUCTION_TICK_MODIFIED=FALSE
OBSERVATION_ONLY=TRUE
PAIR_SUPPORT=MOORE_NEIGHBOUR_PLUS_MINUS
PAIR_TRACKING=PRODUCTION_PARTICLE_IDS
PHASE_WEDGE_TOLERANCE=1E-11
PERSISTENCE_THRESHOLD_TICKS=8
VOLUMES=17,25
TICKS_PER_ARM=96
SEEDS=0X09080001,0X09080002,0X09080003,0X09080004
ARM_COUNT=32
GSTAR_READ=FALSE
CONTEXT_OUTCOME_BORN_READ=FALSE
CENTRAL_MEMORY_LAW_TESTED=FALSE
MAINTENANCE_ERASURE_WORK_CLOSED=FALSE
PRODUCTION_INTEGRATION_ADDED=FALSE
NO_NEW_SELECTED_TYPE=TRUE
STATUS=LOCKED_PRE_RUN
```
