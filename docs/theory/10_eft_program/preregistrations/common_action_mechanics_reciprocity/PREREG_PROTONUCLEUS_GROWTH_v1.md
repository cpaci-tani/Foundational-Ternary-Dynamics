# PREREG — Protonucleus Continued-Growth Experiment v1

**Status:** `[PREREGISTRATION — LOCKED BEFORE EXECUTION]`
**Question:** Does a manifested body above the critical radius grow without
bound, saturate at a preferred size, or collapse?
**Parents:** `FTD-0567` (genesis drain), `FTD-0586`–`FTD-0597` (sub-critical
no-go), `FTD-0781` (drain map), the quasi-static critical-radius computation
(`R_c = 12.63`, `|J| = 0.121199 R - 0.014143`)
**Production impact:** none — standalone simulation, no engine change

## 1. Why

The quasi-static calculation shows a uniform manifested ball radiates from its
**surface** with `|J| ~ 0.1212 R`, crossing `K_GENESIS = 1.5163860592` at
`R_c = 12.63` (`N_c ~ 8,400` sites). Above `R_c` genesis can fire at the
surface, adding a layer, which raises `R`, which raises `|J|` — a
self-reinforcing accretion instability. But **genesis is a dissipative drain**
(FTD-0567), so each layer costs. Whether growth pays for itself is undecided
and decides whether FTD has a **protonucleus with a derived size** or a
runaway.

## 2. Model — taken verbatim from engine source, no substitutions

Field (from the retired DagEngine `recursive_read`/`recursive_write` source,
recoverable from Git history through baseline `21566b63`; movement and curl off
in this profile):

```text
delta_j    = C_WAVE^2 * lap18(J) - G_C * grad(s)
wave_vel  += delta_j
J         += wave_vel
```

`lap18` is the 18-point SC+FCC Moore stencil (6 faces at 1/3, 12 edges at 1/6,
centre -4), matching the active production operator in
`engine/src/render_bridge_phases/phase_read.cpp`.
`C_WAVE^2 = 1/3`, `G_C = 0.0854245431028543695`.

Genesis (from `transmutation_phases.cpp` `pair_production_cpu`):

1. candidate sites have `s == 0` and `|J| > K_GENESIS`;
2. fire with `p = 1 - exp(-(|J| - K_GENESIS)/K_MANIFEST)`,
   `K_MANIFEST = 0.5054620197173260`;
3. partner = neighbour along the **major axis of the flux vector**, in the
   sign direction of that component; partner must have `s == 0`;
4. drain: `wave_vel[site] *= 0.5`, `wave_vel[partner] *= 0.5`,
   `J[site] *= max(0, 1 - K_GENESIS/|J|)`;
5. states: `s[site] = -1` (upstream), `s[partner] = +1` (downstream).

**Declared deviation from engine semantics:** the engine processes sites
sequentially, so partner collisions resolve first-come-first-served. This
implementation vectorises and resolves collisions by keeping the **first
candidate in index order** per partner site, discarding the rest that tick.
Declared in advance; a control arm re-runs with a shuffled resolution order.

## 3. Arms

| arm | seed radius | lattice `L` | note |
|---|---|---|---|
| A | 8 | 129 | sub-critical control — must NOT grow |
| B | 12 | 129 | just below `R_c` |
| C | 13 | 129 | just above `R_c` |
| D | 16 | 161 | clearly super-critical |
| E | 20 | 201 | 8.1M sites, deep super-critical |

Seed is a uniform ball of `s = +1`. RNG seed `20260803`; held-out control seed
`7` on arm C. Ticks: 600, or until `N` exceeds 40% of lattice volume
(boundary-contamination stop).

## 4. Observables (declared before execution)

`N(t)` manifested count · `R_eff(t) = (3N/4pi)^{1/3}` · `max|J|(t)` ·
genesis events per tick · net charge `sum(s)` (must remain 0 — pairs) ·
total field energy proxy `sum(|J|^2)`.

## 5. Preregistered outcomes

- **`SUBCRITICAL_ARREST`** — arms A/B show zero net growth over 600 ticks.
- **`RUNAWAY`** — `N` grows monotonically until the boundary stop.
- **`SATURATION`** — `N` converges to `N*` with `|dN/dt| < 0.1%/tick` sustained
  over 100 ticks. **This is the interesting outcome: a derived body size.**
- **`COLLAPSE`** — `N` falls below the seed value.
- **`OSCILLATION`** — `N` cycles with amplitude > 5% of mean.

## 6. Kill conditions

- If arm C (just super-critical) does **not** grow, the quasi-static
  critical-radius picture is **falsified** and `R_c` is not a nucleation
  threshold.
- If arm A (sub-critical) **does** grow, the model implementation disagrees
  with FTD-0586's registered no-go and the run is **invalid**, not evidence.
- Net charge departing from 0 invalidates the run (pair bookkeeping error).

## 7. What this cannot show

Movement, curl coupling, evaporation, and Gauss projection are all **off**.
This is the frozen coupled wave + genesis profile only. A saturation result
would be a property of *this* profile, not of the full engine. Evaporation in
particular is the most likely omitted stabiliser and is deferred to v2.
