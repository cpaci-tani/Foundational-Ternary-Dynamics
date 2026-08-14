# FTD-0911 — Held-out pair-specific phase-wedge and centrality census v1

**Identifier:** `FTD-0911`  
**Date locked:** 2026-08-11  
**Status:** `[PRE-REGISTRATION — LOCKED/PRE-RUN]`  
**Parent result:** FTD-0910

## 1. Question and disclosed motivation

FTD-0908 validly reached its frozen Outcome A, but the FTD-0910 post-hoc
diagnosis found that all actual pair identities eventually flip chirality and
that the one-place rotated-negative null has the same arm-level pass pattern.
That observation motivates this protocol and is fully disclosed; none of the
FTD-0908 seeds or volumes is reused.

This held-out campaign asks two separate questions:

1. **Pair specificity:** on exactly matched endpoint histories and tick
   support, does the actual positive/negative production pairing retain its
   wedge sign more often than every fixed cyclic derangement?
2. **Centrality:** do the measured projected coordinates obey the parameter-
   free midpoint identities required by a discrete central phase-space map,
   and therefore conserve the phase wedge at production precision?

No fitted Hamiltonian, persistence threshold revision, parameter sweep,
near-miss search, or formula substitution is allowed.

## 2. Frozen production/reference sources

| Source | SHA-256 |
|---|---|
| `engine/include/ftd/voxel.h` | `8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3` |
| `engine/include/ftd/render_bridge.h` | `560CB59E2FCD6E174640CA6BF048FD16AEC36AD2B13EE97FA31E301CF373D91C` |
| `engine/src/render_bridge.cpp` | `BFAD7886CB83A590F0AACA11C03CE25B1FF51D94B4C17B06F5D555E46C18D724` |
| `engine/src/render_bridge_phases/phase_write.cpp` | `2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4` |
| `engine/include/ftd/constants.h` | `5C9E4EA46DE1D5E0BF4479AA9E115520E70B729E7E81335FCEF08CE99704BAB0` |
| `engine/include/ftd/eft/native_ternary_dipole_phase_wedge_memory.h` | `BADAE9D26E5FED6FCD4317A7534648256AFF051E2CAADB7E6BEEA00603AEDF46` |
| `engine/src/eft/native_ternary_dipole_phase_wedge_memory.cpp` | `AA021926D1DE32AE9D04FB72682379DBB7F6CD3A1BB150AADBA6A957DFBF20B5` |

Any drift requires a versioned source-drift protocol before execution. The
production tick itself remains unchanged.

## 3. Frozen held-out matrix

- CPU backend and strict validation;
- odd volumes `L in {19,23}` (held out from FTD-0908's `{17,25}`);
- `128` complete ticks;
- seeds `0x09110001` through `0x09110008` inclusive;
- the exact four FTD-0908 families and parameters:
  `axial_live`, `diagonal_live`, `axial_no_bath`, `empty_control`;
- wave propagation, Gauss projection, genesis, and coupling on;
- Langevin `T=0.005`, `gamma=0.02` except in `axial_no_bath`;
- axial or diagonal center injection of magnitude `10 K_GENESIS`, or no
  injection for `empty_control`; and
- no override of any threshold, drain, evaporation, time step, SOR setting,
  manifestation scale, or engine constant.

The matrix contains `2 x 8 x 4 = 64` arms. The production pairs and raw
endpoint telemetry are defined exactly as in FTD-0908 at tolerance `1e-11`.

## 4. Common-support pair-specific discriminator

For one arm, retain every production-ID pair having at least eight
consecutive valid observations. Sort the retained identities by
`(positive_id,negative_id)` and call their count `m`. Let `I` be the longest
consecutive tick interval common to all `m` histories.

The arm is **pair-discriminator-qualified** only if

```text
m >= 2
length(I) >= 32 ticks.
```

On `I`, compute the actual wedge signs `chi_i(t)`. The actual lag-one
same-sign count is the exact integer

\[
S_0=\sum_{i=0}^{m-1}\sum_{t,t+1\in I}
\mathbf 1[\chi_i(t+1)=\chi_i(t)].
\]

For every nonzero fixed cyclic shift `r=1,...,m-1`, keep positive endpoint
history `i`, replace its negative endpoint fields by the simultaneous fields
of history `(i+r) mod m`, and project them on the unchanged actual axis of
pair `i`. The pseudo-pair ID is
`(positive_id_i,negative_id_(i+r))`. Recompute the wedge at `1e-11`; use the
same common support `I`. Its exact same-sign count is `S_r`.

The arm passes pair specificity iff every pseudo-wedge is finite/nonzero on
`I` and

\[
S_0>\max_{1\le r<m}S_r.
\]

This is an integer ordering on identical sample counts—there is no fitted
margin, p-value, or tolerance. A tie fails. A nonzero pseudo-wedge failure
makes the arm unqualified, not a pass.

A family-by-volume cell passes pair specificity iff at least six of its eight
seeds are qualified and pass. Global `P+` requires all six live cells to pass.
Otherwise the result is `P-`; any cell with fewer than six qualified seeds is
`P_UNQUALIFIED` and makes the full campaign unqualified rather than negative.
Empty controls are diagnostics only.

## 5. Parameter-free discrete centrality ledger

For each actual production-ID history and every consecutive valid tick pair,
write

\[
q=(q_+,q_-),\qquad p=(p_+,p_-),\qquad
\ell=q\wedge p,
\]

\[
\bar q={q_{n+1}+q_n\over2},\quad
\bar p={p_{n+1}+p_n\over2},\quad
\Delta q=q_{n+1}-q_n,\quad
\Delta p=p_{n+1}-p_n.
\]

The exact bilinear identity is

\[
\Delta\ell
=\ell_{n+1}-\ell_n
=\bar q\wedge\Delta p+\Delta q\wedge\bar p.
\]

The runner must verify that identity at `256 x 10^-11` relative tolerance.
Define the two parameter-free central-map residuals

\[
T_p=\bar q\wedge\Delta p,\qquad
T_q=\Delta q\wedge\bar p.
\]

For a radial central update, `Delta p` is parallel to `qbar`; for a scalar
kinetic update, `Delta q` is parallel to `pbar`. Thus both `T_p` and `T_q`
vanish and `ell` is conserved without choosing `mu`, `kappa`, or any force
profile. A transition passes exact centrality iff

```text
abs(T_p), abs(T_q), abs(Delta ell)
  <= 256e-11 * max(1, all contributing product magnitudes).
```

The exact-production-central-law gate `C+` is evaluated only in the no-bath
family, because an imposed Langevin bath is explicitly non-Hamiltonian. It
requires at least 12 of the 16 no-bath seeds to contain a valid history and
every valid no-bath transition in every qualified seed to pass. Any
well-formed residual failure gives `C-`. Fewer than 12 qualified seeds gives
`C_UNQUALIFIED`. Bath-family residuals are reported descriptively.

This tests the exact FTD-0907 central-law form class, not approximate
mean-reversion or another effective stochastic law.

## 6. Secondary chronology controls

On the same common support, recompute each actual wedge after shifting only
its negative endpoint history by `+1`, `+2`, `+4`, and `+8` ticks wherever
overlap remains. Report the exact same-sign counts and flip counts. These
time-shift controls are descriptive; they cannot rescue or overturn `P` or
`C`.

Retain the FTD-0908 signed-cubic, inversion, time-reversal, symmetric-square,
Gram, endpoint-reconstruction, state/RNG nonmutation, finite-telemetry, and
source-hash controls.

## 7. Frozen outcomes

Protocol validity requires all source hashes, 64 arms x 128 ticks, exact raw
reconstruction, algebraic controls, matched sample counts, nonmutation, and
finite telemetry. Failure gives only
`PROTOCOL_INVALID_NO_PAIR_OR_CENTRALITY_VERDICT`.

If valid and qualified:

- **Outcome A (`P+C+`)** — actual pairing uniquely dominates every fixed
  cyclic derangement in all six cells, and the exact no-bath central map
  passes.
- **Outcome B (`P+C-`)** — pair-specific phase persistence exists but the
  exact central FTD-0907 law is not the production law.
- **Outcome C (`P-C+`)** — the exact central law is present but it does not
  bind persistence specifically to actual endpoint pairing.
- **Outcome D (`P-C-`)** — neither pair-specific persistence nor the exact
  central law is observed.
- **Outcome U** — either `P_UNQUALIFIED` or `C_UNQUALIFIED`; no substantive
  pair/centrality verdict.

No result derives protected recursive memory by itself. Outcome A is only
permission to preregister perturbation recovery and work/erasure tests.
Outcome B permits searching for a different natural binding law but forbids
calling the imposed central law emergent. Outcome C or D blocks perturbation
promotion of this wedge pairing until a different pair-specific observable is
pre-registered.

## 8. Firewalls

The runner and adjudicator may not read FTD-0908 outputs, the desired result,
G*, a target period, context, selector state, outcomes, or Born weights.
FTD-0908 may be used only as disclosed design motivation. No training/fitting
step is permitted. No post-data change to identities, support, derangements,
qualification, central residuals, six-of-eight rule, or outcome logic is
allowed.

```text
HELD_OUT_FROM_FTD_0908=TRUE
VOLUMES=19,23
TICKS_PER_ARM=128
SEEDS=0X09110001..0X09110008
FAMILIES=AXIAL_LIVE,DIAGONAL_LIVE,AXIAL_NO_BATH,EMPTY_CONTROL
ARM_COUNT=64
PAIR_MINIMUM_RUN=8
COMMON_SUPPORT_MINIMUM=32
PAIR_CELL_GATE=6/8
CENTRAL_NO_BATH_QUALIFICATION=12/16
PAIR_DISCRIMINATOR=EXACT_SAME_SIGN_COUNT_ORDER
DERANGEMENTS=ALL_NONZERO_FIXED_CYCLIC_SHIFTS
CENTRALITY=PARAMETER_FREE_MIDPOINT_WEDGE_LEDGER
PRODUCTION_TICK_MODIFIED=FALSE
GSTAR_READ=FALSE
CONTEXT_OUTCOME_BORN_READ=FALSE
PERTURBATION_APPLIED=FALSE
MAINTENANCE_ERASURE_WORK_CLOSED=FALSE
NO_NEW_SELECTED_TYPE=TRUE
STATUS=LOCKED_PRE_RUN
```
