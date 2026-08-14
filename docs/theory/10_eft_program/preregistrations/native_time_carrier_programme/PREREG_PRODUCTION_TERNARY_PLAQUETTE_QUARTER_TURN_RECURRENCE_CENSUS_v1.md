# FTD-0915 — Production ternary-plaquette quarter-turn recurrence census v1

**Identifier:** `FTD-0915`  
**Date locked:** 2026-08-11  
**Status:** `[PRE-REGISTRATION — LOCKED/PRE-RUN]`  
**Scope:** observation-only census of the exact FTD-0914 plaquette carrier
under the unchanged production `RenderBridge` tick

## 1. Question

Does the unchanged production dynamics form an identity-bearing neutral
ternary word on an elementary cardinal plaquette and then transport that same
positive/negative particle pair through four consecutive quarter-turns in one
direction, returning to its beginning word on the same support?

FTD-0914 proves that such a trajectory is the minimum exact spatial
realization of `J^2=-I`. It does not prove that production enters or preserves
that orbit. This campaign tests that missing premise without adding a
transport rule, plaquette energy, controller, compact variable, or selected
phase.

## 2. Frozen production sources

The following SHA-256 locks were taken before runner implementation and before
any campaign execution:

| Source | SHA-256 |
|---|---|
| `engine/include/ftd/voxel.h` | `8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3` |
| `engine/include/ftd/render_bridge.h` | `560CB59E2FCD6E174640CA6BF048FD16AEC36AD2B13EE97FA31E301CF373D91C` |
| `engine/src/render_bridge.cpp` | `BFAD7886CB83A590F0AACA11C03CE25B1FF51D94B4C17B06F5D555E46C18D724` |
| `engine/src/render_bridge_phases/phase_write.cpp` | `2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4` |
| `engine/include/ftd/constants.h` | `5C9E4EA46DE1D5E0BF4479AA9E115520E70B729E7E81335FCEF08CE99704BAB0` |
| `engine/include/ftd/eft/native_ternary_plaquette_quarter_turn.h` | `3A970B82EF0BDCCC457D5DDA049CAF971C2318429970E696E64DB84CEB7D1D09` |
| `engine/src/eft/native_ternary_plaquette_quarter_turn.cpp` | `E7891C5099D2DCA1F20DF72E6B37F29A60FE63A7A9E7E645D8AC6E2DF73E1F4C` |
| `THEOREM_NATIVE_TERNARY_PLAQUETTE_QUARTER_TURN_RECURSION_v1.md` | `DC98BB8E8A0CF39E832F7399631F831FF71D3216ED104B6C76384EEEF9100B26` |

Any drift before execution invalidates the campaign or requires a separately
locked source-drift repair. A repair may not change any physics definition,
arm, threshold, or outcome gate.

## 3. Frozen plaquette enumeration

For every periodic anchor `(x,y,z)`, enumerate exactly three elementary
plaquettes, with the displayed right-handed vertex order:

```text
xy (+z): (x+1,y,z), (x+1,y+1,z), (x,y+1,z), (x,y,z)
yz (+x): (x,y+1,z), (x,y+1,z+1), (x,y,z+1), (x,y,z)
zx (+y): (x,y,z+1), (x+1,y,z+1), (x+1,y,z), (x,y,z)
```

Each support is enumerated once. Periodic wrapping is performed only by the
canonical lattice indexer. A word is an **orbit exposure** exactly when its
four actual states are one of the cyclic shifts of

```text
(+1, 0, -1, 0).
```

It is an **identity-bearing exposure** only when the `+1` and `-1` endpoints
both have nonnegative production `particle_id` values. Its immutable key is

```text
(plane, anchor_x, anchor_y, anchor_z,
 positive_particle_id, negative_particle_id).
```

Raw state-only exposures are recorded separately and cannot satisfy a
recurrence gate.

## 4. Frozen transition and defect ledger

Compare consecutive complete ticks on the same fixed support and same
identity key. If the previous word is `w`, classify the next state as:

- `FORWARD` if it is the forward cyclic shift `S w`;
- `REVERSE` if it is `S^-1 w`;
- `STATIONARY` if it is `w`;
- `HALF_TURN` if it is `S^2 w`;
- `ADJACENT_DEFECT` if the same signed identities remain on the four sites
  but occupy one of the eight neutral non-orbit words; or
- `SUPPORT_LOSS` otherwise.

An **oriented run** is a maximal sequence of consecutive `FORWARD`
transitions or consecutive `REVERSE` transitions for one immutable key. A
**full recursive cycle** is an oriented run of at least four transitions. The
fourth transition must return the actual ternary word to its value at the
start of the run; this closure is checked directly, not inferred from the
label.

For every directed transition, use the doubled centered corner coordinates.
The observer must verify exactly:

\[
d_n\cdot d_{n+1}=0,
\qquad |d_n|^2=|d_{n+1}|^2=8,
\qquad L_n=d_n\times d_{n+1}=\pm8\hat n,
\]

and

\[
d_{n+1}=\frac{L_n\times d_n}{|d_n|^2}.
\]

The sign is positive for `FORWARD`, negative for `REVERSE`, relative to the
frozen positive plane normal. Exchanging the temporal order must negate
`L_n`. These are reconstruction controls, not fitted tolerances.

At both ends of every transition record the local four-site native field
energy

\[
E_P=\frac12\sum_{j\in P}(|J_j|^2+|W_j|^2).
\]

Energy contrasts are descriptive. This campaign cannot infer a barrier from
an absent transition or from an average energy difference.

## 5. Frozen production arms

All arms force the CPU backend and strict toggle validation. They execute
`128` complete ticks on held-out volumes `L in {21,27}` and held-out seeds

```text
0x09150001, 0x09150002, 0x09150003, 0x09150004,
0x09150005, 0x09150006, 0x09150007, 0x09150008.
```

The inherited, untuned production families are:

| Family | Toggles and initial data |
|---|---|
| `axial_live` | wave propagation, Gauss projection, genesis, coupling, and Langevin on; `T=0.005`, `gamma=0.02`; center injection `(10 K_GENESIS,0,0)` |
| `diagonal_live` | same, with center injection `10 K_GENESIS (1,1,1)/sqrt(3)` |
| `axial_no_bath` | axial injection and coupling, with Langevin off |
| `empty_control` | live toggles, no injected field |

No production constant or default is overridden. The complete matrix is
`2 volumes x 8 seeds x 4 families = 64` arms.

## 6. Validity gates

The only verdict is `PROTOCOL_INVALID_NO_RECURRENCE_VERDICT` unless all of
the following hold:

1. every frozen source hash matches;
2. a separately locked runner/preflight record exists before execution;
3. all 64 arms execute all 128 ticks with finite telemetry;
4. pre/post hashes prove that every observation leaves the voxel array and
   RNG state unchanged;
5. every elementary plaquette is enumerated exactly once per tick;
6. every stored word, identity, relation, dipole, bivector, and energy is
   independently reconstructible from the corpus; and
7. every exact algebraic transition control passes.

## 7. Frozen outcomes

After validity:

- **Outcome A — cross-volume production recurrence:** in every injected
  family/volume cell, at least six of eight seeds contain an identity-bearing
  full recursive cycle.
- **Outcome B — isolated exact production recurrence:** at least one injected
  arm contains an identity-bearing full recursive cycle, but Outcome A fails.
- **Outcome C — directed formation without full recurrence:** no injected arm
  contains a full cycle, but at least one contains an identity-bearing
  `FORWARD` or `REVERSE` transition.
- **Outcome D — exposure without directed transport:** at least one injected
  arm contains an identity-bearing orbit exposure, but none contains a
  directed transition.
- **Outcome E — no identity-bearing production exposure:** no injected arm
  contains an identity-bearing orbit exposure.

Empty controls, raw state-only exposures, defect classes, direction balance,
local energy, and all per-cell counts are reported but do not alter A--E.

## 8. Promotion and stop boundaries

Outcome A licenses only
`[MEASURED — REPLICATED PRODUCTION PLAQUETTE RECURRENCE CANDIDATE]`.
It does not establish an invariant basin, energetic/topological protection,
long-horizon stability, a common action, or coupling to `G*`.

Outcome B licenses an isolated candidate and replication debt. Outcome C
licenses a one-step gearbox candidate but closes four-step production
recurrence negative in this matrix. Outcomes D/E close the current production
formation route negative in scope. No outcome licenses a new production term.

The runner and adjudicator may not read `G*`, `gamma`, a target period, a Born
weight, Bell setting, selector state, measurement context, or desired outcome.
No parameter search, near-miss search, post-data threshold change, or formula
substitution is permitted.

```text
PRODUCTION_TICK_MODIFIED=FALSE
OBSERVATION_ONLY=TRUE
SUPPORT=ALL_ELEMENTARY_CARDINAL_PLAQUETTES
IDENTITY_KEY=FIXED_SUPPORT_PLUS_POSITIVE_AND_NEGATIVE_PARTICLE_IDS
FULL_CYCLE=FOUR_CONSECUTIVE_SAME_DIRECTION_QUARTER_TURNS
VOLUMES=21,27
TICKS_PER_ARM=128
SEED_COUNT=8
ARM_COUNT=64
CELL_GATE=6_OF_8
GSTAR_READ=FALSE
GAMMA_DERIVED=FALSE
BORN_BELL_TARGET_READ=FALSE
PRODUCTION_INTEGRATION_ADDED=FALSE
NO_NEW_SELECTED_TYPE=TRUE
STATUS=LOCKED_PRE_RUN
```

**LOCKED CONTENT ENDS HERE.** Any substantive change requires `v2`.
