# Pre-registration — Production phase-connection representability classifier v1

**Identifier:** `FTD-0964`  
**Date locked:** 2026-08-11  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE FIRST EXECUTION]`  
**Expected classifier:** `Outcome B`

## 1. Question

Can the exact selected FTD-0962/0963 oriented phase connection be represented
and evolved by the current production `Voxel` state and unchanged production
tick, without adding a public degree of freedom or silently treating an
isolated `ftd::eft` witness as production physics?

This protocol separates three questions that may not be conflated:

1. **raw capacity:** are enough persistent complete canonical field pairs
   already stored;
2. **covariant charting:** can the clock and four exchange modes be selected
   locally without an imported frame; and
3. **dynamic realization:** does the current tick already integrate the
   complete-square connection Hamiltonian and book its energy and inverse?

Reference-state capacity is not production dynamics.

## 2. Frozen production corpus

| Source | Frozen SHA-256 |
|---|---|
| `engine/include/ftd/voxel.h` | `8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3` |
| `engine/src/render_bridge_phases/phase_read.cpp` | `D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8` |
| `engine/src/render_bridge_phases/phase_write.cpp` | `2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4` |
| `engine/src/transmutation_phases.cpp` | `4013A9B769199D54976347378FD03DFF6415B7F641F35D3FAE498125EB288043` |
| `engine/src/render_bridge.cpp` | `BFAD7886CB83A590F0AACA11C03CE25B1FF51D94B4C17B06F5D555E46C18D724` |
| `engine/src/diagnostics_compute.cpp` | `C3703292F8474EBC119F70024B0F3E4A23921C26EA58F8F6AB5E7581FB654AA6` |
| `engine/include/ftd/lagrangian.h` | `0225C75F34D1154CDF3783E73A86F051A3868E0E9087606E117411D75429350F` |
| `engine/tests/test_dual_substrate.cpp` | `B3DF5B36BD73D339E76609D9B5D1114398A61C804904C1B6FE2D1775071CF948` |
| `engine/tests/test_symplectic_wave.cpp` | `C8465563CADC245B3FB8AA19928E64D9D463BF293D29F1776F835633BA95EFF9` |

No production source, toggle, tick phase, energy audit, constant, selector,
Born law, or ontology type may change under this protocol.

## 3. Frozen target and definitions

The FTD-0962/0963 local target has five complete canonical pairs:

\[
 (\delta,\Pi),\quad B=(b_q,b_p),\quad D=(d_q,d_p),
 \quad C=(c_q,c_p),\quad R=(r_q,r_p).                 \tag{1}
\]

Its selected connection Hamiltonian is

\[
 H_{\rm conn}={\bigl[\Pi+{\cal A}(\delta)G\bigr]^2\over2M}
 +V(\delta)+\nu_TA_T+\nu_CA_C,                       \tag{2}
\]

where

\[
 G=(b_qd_p-d_qb_p)+(r_qc_p-c_qr_p).                  \tag{3}
\]

A production variable counts as a **complete canonical pair** only when both
coordinates persist and the production action/tick gives them conjugate
semantics. A diagnostic phase without a conjugate momentum does not count.

An isolated type or exact analyzer under `ftd::eft` does not count as a
production variable or production law.

## 4. Capacity witness frozen before execution

In dual-substrate mode, for a fixed selected orthonormal frame
`(e1,e2,e3)`, define six field pairs

\[
 z_{L,a}=(e_a\!\cdot J_L,e_a\!\cdot P_L),\qquad
 z_{R,a}=(e_a\!\cdot J_R,e_a\!\cdot P_R),\quad a=1,2,3, \tag{4}
\]

with `P_L=wave_vel_L` and `P_R=wave_vel_R`. Freeze the explicit local packing

\[
 (\delta,\Pi)=z_{L,1},\ B=z_{L,2},\ D=z_{L,3},
 \ C=z_{R,1},\ R=z_{R,2},                              \tag{5}
\]

leaving `z_(R,3)` unused. For a fixed frame this is a rank-ten symplectic
coordinate projection onto five of the six stored pairs. It establishes only
conditional local chart capacity.

The observable assignments `flux=flux_L+flux_R` and
`wave_vel=wave_vel_L+wave_vel_R` are derived writes; they do not identify or
remove the independent `L/R` registers.

## 5. Cubic-covariant linear-chart obstruction

Let `V` be the three-dimensional polar-vector representation of the full
signed-cubic group `O_h`. A site-local linear scalar extracted from the two
dual registers would be an invariant covector in `V+V`. The frozen test is

\[
 \operatorname{Hom}_{O_h}(V\oplus V,\mathbf 1)=0.     \tag{6}
\]

The certificate will enumerate all 48 signed permutation matrices and verify
that the group-average projector on `V+V` has rank zero. Therefore the packing
in (5) necessarily imports a frame. This does not exclude a regional or
nonlinear body-derived frame; it makes that derivation a separate open gate.

## 6. Dynamic realization gates

The unchanged production corpus must be audited for all of the following:

1. a persistent canonical clock pair rather than `phase` or `tau` alone;
2. the bilinear exchange generator (3);
3. the complete square in (2), including the positive `A(delta)^2 G^2` term;
4. continuous oriented `L/R`-mode exchange rather than a discrete whole-field
   swap;
5. a local phase-crossing profile whose integral supplies the quarter-turn;
6. reciprocal canonical/mechanical reaction;
7. connection energy in the production energy audit; and
8. an exact reverse tick or event map.

Absence of any item forbids an exact-production verdict. The weak
transmutation field swap has square `+I` and determinant `-1` per scalar
`L/R` sector; the oriented exchange quarter-turn has square `-I` and
determinant `+1`. They are not the same transformation.

## 7. Frozen checks

- **G1 — source lock:** all nine production hashes and required source markers.
- **G2 — canonical capacity:** six stored dual field pairs versus five target
  pairs.
- **G3 — explicit selected packing:** rank ten and fixed-frame symplectic form.
- **G4 — covariance boundary:** the `O_h` invariant-scalar projector on
  `V+V` has rank zero.
- **G5 — production pair semantics:** `wave_vel` is identified as flux-field
  Legendre momentum and the dual tick advances both pairs separately.
- **G6 — diagnostic-clock boundary:** `phase` is declared read-only and has no
  stored conjugate momentum or production consumer.
- **G7 — swap mismatch:** the weak `L/R` swap is algebraically distinct from
  the oriented quarter-turn.
- **G8 — connection-law audit:** no complete-square connection, exchange
  generator, holonomy profile, or reciprocal connection force occurs in the
  frozen production tick.
- **G9 — energy audit:** the production audit contains no connection energy or
  finite gearbox reserve/backpressure term.
- **G10 — scope firewall:** capacity may not be reported as native formation,
  production coupling, `G*` selection, Born/Bell recovery, hiding, or
  completeness.

No fitted tolerance, numerical search, near-miss scan, or target probability
is permitted.

## 8. Frozen classifier

- **Outcome A — exact production realization:** G1--G10 pass and the unchanged
  production tick already supplies the covariant chart, complete-square law,
  energy audit, and inverse.
- **Outcome B — conditional chart capacity / production law absent:** G1--G10
  pass, the fixed-frame packing exists, and at least one native covariance or
  dynamic realization gate is absent.
- **Outcome C — existing-type capacity insufficient:** the dual production
  registers cannot even supply five complete pairs under the fixed-frame
  witness.
- **Outcome D — invalid:** a frozen source changed, a required check is
  unevaluable, or the classifier is internally inconsistent.

The expected result is Outcome B. An expected result is not a licensed result
until the first immutable execution passes every registered check.

## 9. Promotion boundary

Outcome B licenses only this statement:

> No new public continuous storage type is forced by local scalar capacity,
> conditional on dual-substrate mode and a selected frame. The current
> production tick does not realize the FTD-0962/0963 connection, and no
> site-local cubic-covariant linear scalar chart exists on the raw dual vector
> registers.

It does not license a production implementation. A successor must separately
pre-register either a native regional frame/connection derivation or a priced
selected production action, then test nonlinear repeated-map stability,
positive phase-error export, energy/current closure, GPU parity, hiding, and
Born-target blindness.
