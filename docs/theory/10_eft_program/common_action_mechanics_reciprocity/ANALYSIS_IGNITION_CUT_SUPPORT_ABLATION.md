# Analysis — Ignition-Cut Support Ablation

**FTD ID:** FTD-0587  
**Status:** `[DERIVED — INTERVENTION DECOMPOSITION]` +
`[MEASURED — 144 LOCKED CONTINUATIONS]` +
`[MIXED/UNRESOLVED — PROJECTOR/INHERITED-FIELD INTERACTION]` +
`[CLOSED NEGATIVE — ISOLATED CAUSAL SUPPORT]`  
**Date:** 2026-07-26  
**Verdict:** `MIXED_OR_UNRESOLVED`

## 1. Mechanism split

FTD-0474 prepared a manifested support with a one-time local flux injection,
then evolved it with wave propagation, genesis/evaporation, and the selected
Gauss projector. Native state--flux coupling was off in that arm. Before the
first post-cut reaction, a continuation may be separated interventionally as

\[
 z_n=T^{n-n_c}z_{n_c}
     +\sum_{m=n_c}^{n-1}T^{n-1-m}K s_m
     +\mathcal P_G[s_m].
\]

The terms are inherited field evolution, the causal coded source, and the
selected Gauss correction. This is not a superposition theorem after genesis
or evaporation, because those events change `s` nonlinearly. It is a causal
ablation: keep or clear `z_cut`, then enable `K`, `P_G`, or neither.

## 2. Locked execution

The campaign exactly replayed all 24 FTD-0474 reaction-dispersal cells through
tick 150: `L={24,32}`, amplitudes `{12,20,40} K_GENESIS`, and four fixed seeds.
Each prefix was replayed for six continuations. All six selected-state and RNG
hashes agreed bit-for-bit before intervention.

| continuation | field at cut | causal source | Gauss | stable runs | passing cells |
|---|---|---:|---:|---:|---:|
| intact reservoir | retained | off | off | 0/24 | 0/6 |
| intact causal | retained | on | off | 0/24 | 0/6 |
| intact projected | retained | off | on | 20/24 | 5/6 |
| cleared control | zero | off | off | 0/24 | 0/6 |
| cleared causal | zero | on | off | 0/24 | 0/6 |
| cleared projected | zero, then one Gauss solve | off | on | 18/24 | 4/6 |

The stability and cell predicates are exactly the FTD-0474 predicates. No
amplitude, cut, tolerance, or post-run cell rule was changed.

## 3. What the split establishes

The retained field is not sufficient. It gives 12 new genesis and 929
evaporation events but zero stable runs. Adding native state-gradient coupling
does not repair it: the intact causal arm gives seven genesis, 899 evaporation,
and zero stable runs.

The cleared control and cleared causal arms both fully evaporate all 1,496 cut
sites and finish with zero occupancy in all 24 runs. The causal source does
regenerate a field history—its mean final quadratic amplitude norm is
`0.45635515435907353`—but it does not regenerate manifested support.

Repeated Gauss projection is the only registered branch associated with
stable support. Starting from a cleared field, it retains 18/24 stable runs
and four passing cells. The two failures are the high-amplitude cells:

| cell | intact projected | cleared projected |
|---|---:|---:|
| `L=24,A=40` | 0/4 | 0/4 |
| `L=32,A=40` | 4/4 | 2/4 |

All lower-amplitude cells pass 4/4 in both projected arms. Thus the projector
is the dominant support mechanism, while the inherited reservoir changes the
high-amplitude `L=32` outcome enough for the full five-cell qualification.
Neither component alone passes the complete registered grid. The locked
verdict is therefore an interaction, `MIXED_OR_UNRESOLVED`, rather than
`GAUSS_CONSTRAINT_SUFFICIENT`.

The reported quantity

\[
 Q=\frac12\sum_x\left(|J_x|^2+|W_x|^2\right)
\]

is only the engine's quadratic field-amplitude norm. It is not the exact
modified wave Hamiltonian and is not used as a conservation claim.

## 4. Correction to the front interpretation

The qualified intact-projector tail records **zero genesis events** after the
cut and 204 evaporation events. Its manifested support does not reproduce or
advance. The open/dispersal FTD-0474 object is therefore more accurately a

> Gauss-stabilized, externally prepared, evaporative manifestation remnant.

Calling that tail a self-sustaining reaction front was too strong. The
periodic and thermal comparisons remain evidence that periodic recirculation
and the registered bath are unnecessary **conditional on** the inherited
field plus repeated Gauss projection. They do not establish native causal
autocatalysis.

## 5. Boundary

FTD-0587 does not close every larger causal cluster. FTD-0586 still leaves an
independently derived `N>=4` geometry open. It does close the externally
ignited FTD-0474 dispersal tail as evidence for autonomous native matter:

- no causal arm is stable;
- no arm moves matter;
- no qualified dispersal tail creates a new site;
- the selected projector is not derived from the five postulates or from the
  face-flux common action;
- the inherited injection reservoir remains dynamically relevant at the
  highest qualified cell.

No production toggle, scenario, particle, membrane, pole, infrared, or
Lorentz claim is licensed.

## 6. Verification

- preregistration SHA-256:
  `C2417CD829E665C6A4936D37DFA7C83F790925E5395FA387C34C03F27C857B2B`;
- native CTest: PASS, 144 continuations / 43,200 registered ticks;
- independent verifier: 39/39 PASS;
- run record: `engine/results/ftd_0587/windows_msvc_cpu.{csv,json}`;
- production/default/toggle/scenario changes: none.
