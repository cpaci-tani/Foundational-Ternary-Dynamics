# Engine Tick Cycle

One paragraph per phase. Full spec: `engine/SPEC_ENGINE.md` §Tick Cycle.

## The ten phases

```
phase_read → phase_write → pair_production → gauss_project → latency_solve →
phase_forces → phase_movement → boundary → weak/triad → proper_time → tick++
```

Each phase runs over the whole lattice before the next phase starts. No phase can "peek ahead" at later phases in the same tick. This is the local-causality axiom made operational. Every phase is individually toggle-gated (`engine/src/render_bridge.cpp::tick()`, "Rule 1" through "Rule 8"); phases 3, 5, 8, and 9 (`pair_production`, `latency_solve`, `boundary`, `weak`/`triad`) and phase 10 (`proper_time`) are the ones added after this doc's original "six phases" framing and default OFF like the rest of the phenomenological extensions.

## 1. `phase_read`

Reads the current flux field `J(x)` and state field `s(x)` into per-voxel read-only caches. Builds the Moore neighborhood (26 cells + self) indices for every voxel. No writes. This is the only phase that's trivially parallel over the whole lattice without worrying about write hazards.

**What it establishes:** a frozen snapshot of "now" that every subsequent write phase can compare against.

## 2. `phase_write`

Applies the wave equation, genesis threshold, and any active toggles that modify `J` or `s`. Writes go to **next-tick buffers**, not the current buffers — so `phase_write` always reads from phase 1's caches and produces independent outputs. This is where particles are born (`|J| ≥ K_GENESIS`) and where dissipation subtracts `α |J|` per tick.

**Integrator note (verified 2026-04-17):** the advance pair `wave_vel += Δ; flux += wave_vel` is Störmer–Verlet leapfrog under the stagger interpretation (`wave_vel = v(t+h/2)`, `flux = J(t)`). Symplectic — cumulative energy balance to 0.1 % over 5000 ticks with damping off. `C_SPEED = 1/√3` is the correct leapfrog CFL limit. See `tests/test_leapfrog_integrator_audit.cpp`.

**Key toggles:** `wave_equation`, `manifestation`, `damping`, `dual_substrate`, `chirality`, `confinement`.

## 3. `pair_production` (toggle `pair_production`, default OFF)

Correlated ±1 pairs manifesting from high-flux void: `state==0` sites with `|J| > K_GENESIS` become a linked ±1 pair via `pair_production_cpu()`. This is a code path independent of `phase_write`'s ordinary genesis threshold — the two do not share logic, they just both watch `|J|` against a genesis-scale bound.

## 4. `gauss_project`

Enforces the Gauss constraint `∇·J = ρ` via a successive-over-relaxation (SOR) solver. Without this step, numerical drift in the wave equation would accumulate ∇·J errors and break Coulomb's law. The solver iterates until the constraint residual is below tolerance, then updates `J` in place.

**Why separate phase:** the constraint coupling is non-local, so SOR must run over the whole lattice. Attempting to enforce it per-voxel would fight the wave equation's explicit update.

## 5. `latency_solve` (toggle `latency_field`, default OFF)

Gravitational-potential Poisson solve: `∇²φ_L = 4πG·ρ_mass`, then `L = √(clamp(φ_L, 0, 0.998))` via `solve_latency_poisson()`. Must run after `gauss_project` (which modifies flux) and before `phase_forces` (which consumes `L`) — the ordering is load-bearing, not incidental.

## 6. `phase_forces`

Computes force fields per voxel for every active interaction: electromagnetic (Coulomb + Lorentz), gravity (from flux gradient), strong (color-coupled, SU(3) confinement), weak (chirality-mediated). Each force is stored in its own field so overlays can display them separately. Forces do **not** move particles yet — they just populate the force tables.

**Why separate from movement:** the strong and weak forces require color/spin state from `phase_write`, but movement needs the combined net force from all active toggles. Splitting lets each force contribute independently.

## 7. `phase_movement`

Particles integrate their positions using **remainder-accumulation integer jumps**: `remainder += v · dt`, and whenever any axis crosses ±1 the particle moves one lattice cell in that direction and the remainder decrements by 1. Collisions are resolved synchronously via a `moved_` flag: void target → move in; same-sign target → bounce; opposite-sign target → annihilate (cancel to void, flux burst).

**Force integration (`phase_forces`, as of 2026-04-17):** the force step that feeds this phase uses **γ_FTD momentum integration**, not a velocity clamp:

```
γ_in  = 1 / √(1 − |v|²/C² − L²)
p     = γ_in · v           (reconstruct momentum)
p    += F · dt             (Newton on p)
|v|²  = C²(1 − L²)·|p|² / (C² + |p|²)
v     = p · C · √((1−L²)/(C² + |p|²))
```

Properties:
- Newtonian limit (|v| ≪ C, L = 0): `v_new ≈ v + F·dt`.
- Ultra-relativistic (huge F): `|v| → C·√(1−L²)` asymptotically — **no clamp, no energy discard, Lorentz-invariant by construction**.
- Horizon (L → 1): `|v| → 0`.

Verified in `engine/tests/test_gamma_ftd_momentum.cpp`.

**Boundary conditions (particle-level):** configurable per scenario (cube / sphere / torus / reflective). Particles that leave the boundary either wrap, bounce, or vanish depending on setup. This is distinct from the flux-field boundary phase below — one governs particles leaving the lattice, the other governs the flux field's edge behavior.

## 8. `boundary` — flux-field boundary law (toggles `absorbing_boundary`, `flux_boundary`, default: periodic / no-op)

Runs after `gauss_project`/`phase_forces`/`phase_movement` (the last flux writers) so the boundary treatment isn't refilled by a later phase. Two independent gates:
- **Absorbing sponge** (`apply_absorbing_boundary`, toggle `absorbing_boundary`): disperses outgoing waves into the void at the lattice faces.
- **Flux boundary mode** (`flux_boundary`): `Periodic` (default) is handled by the lattice's neighbor tables directly — no pass runs, golden-tick hash unaffected. `Reflective`/`Dispersal` re-impose their boundary condition on the shell via a dedicated pass.

## 9. `weak` / `triad` (toggles `weak_transmutation`, `triad_binding`, default OFF)

**Weak transmutation** (`weak_transmutation_cpu()`): polarity flip under field stress. **Triad binding** (`triad_binding_cpu()`): detects 3 same-sign particles forming a locked configuration. Two independent, sequential checks over the lattice.

## 10. `proper_time` (toggles `latency_field` or `de_broglie_clock`, default OFF)

Accumulates each particle's proper time via `accumulate_proper_time()`. The rate is `dτ/dt = √max(1 − v²/C_SPEED² − L², 0)` (`engine/include/ftd/causal_kinematics.h::proper_time_rate()` — **not** the `√(f²−v²)/√f`, `f=1−L²` form this cheatsheet previously stated, which does not match the shipped formula). At `L = 0` this reduces to the special-relativistic rate `√(1−v²)`; with `latency_field` also on, `L` (the gravitational-analogue potential from phase 5) contributes gravitational time dilation.

## `tick++` + energy ledger

Commits the next-tick buffers to the current buffers, increments `_tick`, and calls `update_energy_ledger()` to snapshot the total scalar energy for this tick and compute the drift:

```
drift_frac    = (E_curr − E_prev) / max(|E_prev|, ε)
expected_rate = −DAMPING  if damping ON,  else 0
residual      = drift_frac − expected_rate      ← CI assertion target
```

Tests that run with damping off can assert `|residual| < tol` and refuse regressions that introduce unaccounted energy drift. Accessible via `bridge.energy_ledger()` — auto-populated on **both CPU and GPU paths** (the GPU `tick()` calls `gpu_sync_to_host()` + `update_energy_ledger()` before returning).

Then the render path runs: `syncRenderableData` reads the new state, the memory recorder captures a timeline snapshot if we've crossed a sample cadence, and the viewport repaints.

## Three-level pause

Because the engine separates physics from visualization from presentation:

| State | Physics (phases 1–10) | Viz recompute (overlays) | Render (canvas) |
|---|---|---|---|
| Global play | ✓ | ✓ | ✓ |
| Global pause | ✗ | ✗ | ✓ (last frame) |
| Scenario locally paused (global play) | ✗ | ✓ | ✓ |
| Scrubbing (`state.scrubbing = true`) | ✗ | ✓ (triggered by hydrate) | ✓ |
| Rendering (`state.rendering = true`) | ✗ (owned by render controller) | ✓ (after each slice) | ✓ |

## Cross-references

- `engine/SPEC_ENGINE.md` §Tick Cycle — full details
- `engine/src/render_bridge.cpp::tick()` — the actual phase call sequence and toggle gates
- `engine/include/ftd/render_bridge_phases.h` — decomposed phase free-function declarations
- `engine/web/js/scales/scale0/runtime/tick.js` — browser-side equivalent
- `resources/cheatsheets/MOORE_NEIGHBORHOOD.md` — the 26-cell structure every phase reads
