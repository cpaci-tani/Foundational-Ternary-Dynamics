# FTD-0757 — M3 fixed-chart parent qualification v1

**Status:** `[PRE-REGISTRATION — FROZEN BEFORE IMPLEMENTATION; NOT RUN]`  
**Date:** 2026-07-30  
**Parents:** FTD-0753 constructive causal-horizon replay; FTD-0755
infrastructure-unresolved validation; FTD-0756 parent-wrapper forensics  
**Scope:** observer-only qualification of the parent replay required before a
new held-out matter-family validation; no action, dynamics, predicate,
parameter, tolerance, scenario, or ontology change

## 1. Locked question

Does replacing the FTD-0755 validation wrapper's moving continuous regional
center by the same fixed integer measurement chart used in FTD-0753 reproduce
the established parent through tick 160 at both registered volumes, while
separately recording the continuous relational midpoint without feeding it
back into dynamics or readout?

FTD-0757 is a qualification of instrumentation only. It cannot establish M3
or consume a new hostile candidate.

## 2. Frozen distinction between object and chart

For the two constituents define the continuous relational midpoint

\[
m_t=\frac{x_+(t)+x_-(t)}{2}.
\]

Record `m_t` and

\[
\epsilon_t=\max_i |m_{t,i}-\operatorname{round}(m_{t,i})|.
\]

The regional observer center is instead the fixed integer preparation chart

\[
C_L=(\lfloor L/2\rfloor,\lfloor L/2\rfloor,\lfloor L/2\rfloor).
\]

Use `C_L` at every tick with the established selected radii

```text
{8,12,16,24,32,48}.
```

Do not round, snap, translate, constrain, or feed `m_t` into the state. Do not
reinterpret `C_L` as the object's material center. It is the fixed lattice
chart of the regional energy observer.

## 3. Frozen matrix and dynamics

Replay exactly

```text
L = {321,385}
ray = {face (0,0,1), edge (0,1,-1), body (1,1,1)}
ticks = 0,...,160
```

Use the unchanged FTD-0753/0756 finite-support preparation, radius-four
selected dressing, explicit-rounding ordered WSL2 CUDA library, common-action
options, implicit solve, sparse current, field update, cache, and interaction
normalization. Every accepted transaction must pass the existing validity,
common-action, energy, recoil, and causal-speed gates. A failed transaction is
not committed and the history stops.

At tick 160 require the support-independent core predicate to accept with

```text
graph margin  >= 1e-6
energy margin >= 1e-6.
```

These are the already-frozen FTD-0755 qualification margins, not new matter
thresholds.

## 4. Frozen comparisons

For `L=321`, compare every accepted transaction with the exact FTD-0753 CSV
strings for

```text
valid
common
separation
pair_energy
maximum common-action residual
energy residual
recoil diagnostic
causal speed excess
```

Report exact string equality and maximum numerical difference. No tolerance
may convert unequal strings to exact.

For each ray and volume, record the first transaction whose pre-transaction
midpoint would violate the current integer-center observer API
(`epsilon_t != 0` under its exact comparison). It must equal the corresponding
FTD-0756 stage-four abort tick:

```text
face 57
edge 30
body 122.
```

This comparison is confirmatory localization, not held-out evidence. If it
differs, the FTD-0756 source diagnosis is incomplete even if the fixed chart
replay succeeds.

## 5. Frozen outputs

Write one CSV/JSON pair per volume and ray under
`engine/results/ftd_0757/`. Each row records the transaction gates, scalar
parent observables, fixed chart, continuous midpoint, `epsilon_t`, API
admissibility, and state-only core margins. Each JSON records preparation,
row count, first fractional-midpoint tick, tick-160 status, and
`dynamics_changed = false`.

The independent certificate checks hashes, schemas, all six arms, exact
FTD-0753 replay at `L=321`, the frozen FTD-0756 tick map, and the first-failed
verdict map.

## 6. Frozen verdict map

Apply the first matching outcome:

1. any finite-support preparation fails:
   `M3_FIXED_CHART_PREPARATION_FAILURE`;
2. any fixed-chart regional observation fails:
   `M3_FIXED_CHART_OBSERVER_FAILURE`;
3. any physical transaction fails validity/common-action before tick 160:
   `M3_FIXED_CHART_PARENT_DYNAMICS_FAILURE`;
4. any accepted `L=321` scalar differs from FTD-0753:
   `M3_FIXED_CHART_PARENT_REPLAY_DIVERGENCE`;
5. any first fractional-midpoint tick differs from FTD-0756:
   `M3_MOVING_CENTER_DIAGNOSIS_INCOMPLETE`;
6. every `L=321` arm passes but an `L=385` arm fails:
   `M3_FIXED_CHART_LARGE_VOLUME_FAILURE`;
7. all six arms pass:
   `M3_FIXED_CHART_PARENT_QUALIFIED`.

The constructive verdict authorizes a separately frozen fresh held-out
validation. It does not reopen FTD-0755, establish a finite-time matter
family, promote a particle, or prove that the fixed chart is a unique physical
observer.

## 7. Execution firewall

Before registered execution, freeze the protocol, implementation, independent
certificate, WSL2 executable, output schema, and absence of
`engine/results/ftd_0757/`. Qualification is limited to one face transaction
at `L=321` and writes no registered artifact. Each of the six registered modes
may run exactly once. No failed arm may be rerun or tuned under FTD-0757.

Production defaults, established CUDA libraries, scenarios, ontology, and all
FTD-0753/0755/0756 artifacts remain unchanged.
