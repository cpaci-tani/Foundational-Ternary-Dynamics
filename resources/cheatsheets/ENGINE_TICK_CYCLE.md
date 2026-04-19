# Engine Tick Cycle

One paragraph per phase. Full spec: `engine/SPEC_ENGINE.md` §Tick Cycle.

## The six phases

```
phase_read  →  phase_write  →  gauss_project  →  phase_forces  →  phase_movement  →  tick++
```

Each phase runs over the whole lattice before the next phase starts. No phase can "peek ahead" at later phases in the same tick. This is the local-causality axiom made operational.

## 1. `phase_read`

Reads the current flux field `J(x)` and state field `s(x)` into per-voxel read-only caches. Builds the Moore neighborhood (26 cells + self) indices for every voxel. No writes. This is the only phase that's trivially parallel over the whole lattice without worrying about write hazards.

**What it establishes:** a frozen snapshot of "now" that every subsequent write phase can compare against.

## 2. `phase_write`

Applies the wave equation, genesis threshold, and any active toggles that modify `J` or `s`. Writes go to **next-tick buffers**, not the current buffers — so `phase_write` always reads from phase 1's caches and produces independent outputs. This is where particles are born (`|J| ≥ K_GENESIS`) and where dissipation subtracts `α |J|` per tick.

**Integrator note (verified 2026-04-17):** the advance pair `wave_vel += Δ; flux += wave_vel` is Störmer–Verlet leapfrog under the stagger interpretation (`wave_vel = v(t+h/2)`, `flux = J(t)`). Symplectic — cumulative energy balance to 0.1 % over 5000 ticks with damping off. `C_SPEED = 1/√3` is the correct leapfrog CFL limit. See `tests/test_leapfrog_integrator_audit.cpp`.

**Key toggles:** `wave_equation`, `manifestation`, `damping`, `dual_substrate`, `chirality`, `confinement`.

## 3. `gauss_project`

Enforces the Gauss constraint `∇·J = ρ` via a successive-over-relaxation (SOR) solver. Without this step, numerical drift in the wave equation would accumulate ∇·J errors and break Coulomb's law. The solver iterates until the constraint residual is below tolerance, then updates `J` in place.

**Why separate phase:** the constraint coupling is non-local, so SOR must run over the whole lattice. Attempting to enforce it per-voxel would fight the wave equation's explicit update.

## 4. `phase_forces`

Computes force fields per voxel for every active interaction: electromagnetic (Coulomb + Lorentz), gravity (from flux gradient), strong (color-coupled, SU(3) confinement), weak (chirality-mediated). Each force is stored in its own field so overlays can display them separately. Forces do **not** move particles yet — they just populate the force tables.

**Why separate from movement:** the strong and weak forces require color/spin state from `phase_write`, but movement needs the combined net force from all active toggles. Splitting lets each force contribute independently.

## 5. `phase_movement`

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

**Latency `L`** (gravitational analogue) is tracked per particle and used for proper-time accumulation `dτ/dt = √(f² − v²)/√f` with `f = 1 − L²` — but **only when the `latency_field` toggle is on**. Without it, no gravity-induced time dilation.

**Boundary conditions:** configurable per scenario (cube / sphere / torus / reflective). Particles that leave the boundary either wrap, bounce, or vanish depending on setup.

## 6. `tick++` + energy ledger

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

| State | Physics (phase 1–5) | Viz recompute (overlays) | Render (canvas) |
|---|---|---|---|
| Global play | ✓ | ✓ | ✓ |
| Global pause | ✗ | ✗ | ✓ (last frame) |
| Scenario locally paused (global play) | ✗ | ✓ | ✓ |
| Scrubbing (`state.scrubbing = true`) | ✗ | ✓ (triggered by hydrate) | ✓ |
| Rendering (`state.rendering = true`) | ✗ (owned by render controller) | ✓ (after each slice) | ✓ |

## Cross-references

- `engine/SPEC_ENGINE.md` §Tick Cycle — full details
- `engine/include/ftd/phases.h` — phase entry points
- `engine/web/js/scales/scale0/runtime/tick.js` — browser-side equivalent
- `resources/cheatsheets/MOORE_NEIGHBORHOOD.md` — the 26-cell structure every phase reads
