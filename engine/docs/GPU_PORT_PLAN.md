# GPU Port Plan: AtomEngine + ParticleEngine

**Status**: Wave 5.3 WIP (2026-04-14). Wave 5.1 (GPU latency Poisson) and
Wave 5.2 (auto-push voxels mutations) are shipped. This doc plans the
bigger port of the two CPU-only engine classes to GPU.

**Design intent**: All tests GPU primary, CPU backup (TEST_AUDIT §10).
`RenderBridge` already runs on GPU via `gpu::GpuEngine`. The two classes
below are the last remaining CPU-only engines.

---

## 1. Why this is a substantial task

`AtomEngine` (982 LOC CPU) and `ParticleEngine` (similar) are not simple
kernels — they implement full molecular dynamics with:

| Force / phase | Complexity | GPU difficulty |
|---|---|---|
| Pair forces (ionic, vdW, H-bond) | O(N²) or O(N log N) via Barnes-Hut | **Easy** — direct O(N²) kernel for small N, Barnes-Hut for large |
| Covalent bonds (harmonic stretch) | O(bonds) per atom | **Easy** — per-atom kernel over ai.bonds[] |
| Angle strain (3-body VSEPR) | O(bond_pairs) per atom | **Medium** — requires atom→bond indirection; Wave 4a.1's center-atom reaction force translates directly |
| Dipole-dipole (N² with exclusion) | O(N²) with bonded-pair skip | **Medium** — the Wave 4a.1 implementation is already GPU-friendly |
| Torsional (4-body chains) | O(chains) per bond | **Hard** — 4-body topology walks are awkward in SIMT |
| Improper torsional (center + 3) | O(N) per sp2 center | **Medium** |
| Thermostat (Berendsen) | Global reduction + per-atom rescale | **Medium** — needs parallel reduction (cub or warp-shuffle) |
| Barnes-Hut octree | O(N log N) tree build + force walk | **Hard** — dynamic tree construction on GPU is non-trivial |
| Bond dynamics (create/break) | O(N²) with topology updates | **Hard** — variable-sized bond lists per atom |
| CPU<>GPU sync (upload/download) | O(N) per tick | **Easy** but expensive for small N |

**Total estimate**: gpu::AtomEngine full port is ~800-1000 LOC CUDA
(~3-5 kernels + host wrapper + buffer mgmt + integration). gpu::ParticleEngine
is similar scope but different force set (spin/color/exchange).

---

## 2. Phased rollout

### Phase 1 — Minimal viable gpu::AtomEngine (Wave 5.3)

**Goal**: A working gpu::AtomEngine that handles the two most common
use cases — pair forces + Velocity Verlet integration — on GPU.

**Scope**:
- `gpu::AtomBuffers` — SoA device arrays for Atom fields
- `atom_pair_forces_kernel` — O(N²) kernel for ionic + vdW
- `half_kick_kernel`, `drift_kernel`, `speed_limit_kernel`, `damping_kernel`
- `gpu::AtomEngine` class with upload/download + tick()
- Host shim: `AtomEngine::use_gpu(bool)` toggle; when enabled, calls
  internal gpu_ backend for pair forces, falls back to CPU for
  bond/angle/dipole/thermostat/torsional
- Parity test: small test (5 atoms) that compares CPU vs GPU forces
  element-by-element and asserts agreement to 1e-12

**Out of scope for Phase 1**:
- H-bond (needs bond partner lookup — Phase 2)
- Angle strain, dipole-dipole, torsional (Phase 2)
- Thermostat reduction (Phase 2)
- Barnes-Hut on GPU (Phase 3 — large N only)
- Bond creation/breaking (Phase 3)

**Files created**:
- `engine/include/ftd/gpu_atom_engine.h` (wrapper class)
- `engine/cuda/atom_engine_gpu.cu` (kernels + launchers)
- `engine/tests/test_gpu_atom_parity.cpp` (parity test)

### Phase 2 — Multi-body forces on GPU

**Added kernels**:
- `atom_bond_force_kernel` — per-atom loop over ai.bonds[]
- `atom_angle_strain_kernel` — per-atom loop over bond pairs, Wave 4a.1
  center-atom reaction force included
- `atom_dipole_dipole_kernel` — the Wave 4a.1 code verbatim, with
  bonded-pair exclusion check
- `compute_dipole_moments_kernel` — per-atom loop over bonds, reads
  electronegativity
- `apply_thermostat_kernel` + reduction for T_current

**Output**: gpu::AtomEngine covers everything except torsional + bond
dynamics + Barnes-Hut.

### Phase 3 — Full parity + large-N scaling

**Added**:
- `atom_torsional_kernel` (4-body chains)
- `atom_improper_torsional_kernel`
- Barnes-Hut octree build on GPU (or use NVIDIA's HIP-Barnes-Hut / Thrust)
- Bond dynamics kernels (create/break)

**Output**: Full AtomEngine parity on GPU for 1-10k atom simulations.

---

## 3. gpu::ParticleEngine

**Scope similar to AtomEngine but different forces**:
- Pair forces: coulomb, gravity, lorentz, magnetic_dipole
- Per-particle: radiation, spin_orbit, relativistic
- Exchange force (same-spin same-charge repulsion)
- Strong force (color triplet binding)

**Reuses the same phased approach** (Phase 1: pair forces + integration,
Phase 2: multi-body/exchange/strong, Phase 3: Barnes-Hut).

**Key difference**: ParticleEngine has a Particle struct with ~20 fields
(slightly more than Atom). SoA layout needs:
- position (double × 3)
- velocity (double × 3)
- acceleration (double × 3)
- prev_acceleration (double × 3)
- mass (double)
- r_eff (double)
- charge (int8)
- spin (int8)
- color (int8)
- locked (bool)
- pair_id (int32)
- spin_axis (double × 3)
- id (int32)

Upload/download adds ~100 bytes per particle per tick. For 1000 particles,
that's 100 KB/tick — negligible vs any real simulation work.

---

## 4. Integration strategy

### Option A: New gpu::AtomEngine class (pure)

```cpp
namespace ftd::gpu {
class AtomEngine {
    AtomBuffers bufs_;
    void tick();
    void upload_atoms(const std::vector<ftd::Atom>& host);
    void download_atoms(std::vector<ftd::Atom>& out);
    // ...
};
}
```

Users explicitly choose between `ftd::AtomEngine` (CPU) and
`ftd::gpu::AtomEngine` (GPU) at construction time. Matches the
`gpu::GpuEngine` pattern.

**Pro**: Clean separation. No internal use_gpu_ flag on CPU class.
**Con**: Tests need to switch engine type.

### Option B: Internal use_gpu_ flag on AtomEngine

```cpp
class AtomEngine {
    void set_use_gpu(bool b);
    void tick() {
#ifdef FTD_ENABLE_CUDA
        if (use_gpu_) return gpu_tick();
#endif
        cpu_tick();
    }
private:
    std::unique_ptr<gpu::AtomGpuState> gpu_;
    bool use_gpu_ = false;
};
```

Matches `RenderBridge` pattern — transparent to callers.

**Pro**: Zero-change for existing tests. `engine_select.h`-style wrappers
can default to GPU when available.
**Con**: Mixes CPU + GPU code in one class.

**Recommendation**: Option B. Keep API stable, mirror RenderBridge
semantics (including auto-push-to-device on host mutations à la Wave 5.2).

---

## 5. Phase 1 implementation checklist

```
□ 1. Create gpu::AtomBuffers struct + allocate/free
□ 2. Add d_pos_x, d_pos_y, d_pos_z, d_vel_*, d_mass, d_charge,
     d_radius, d_vdw_eps, d_vdw_sig, d_force_x, d_force_y, d_force_z
□ 3. upload_atoms / download_atoms helpers (AoS Atom → SoA device)
□ 4. atom_pair_forces_kernel (ionic + vdW, O(N²))
□ 5. half_kick, drift, enforce_speed_limit, apply_damping kernels
□ 6. launch_compute_pair_forces / launch_integration_step launchers
□ 7. gpu::AtomEngine wrapper class with tick()
□ 8. Integrate via AtomEngine::use_gpu_ flag
□ 9. Unit test: 5-atom configuration, compare CPU vs GPU forces
□ 10. Regression test: run test_atom_engine_forces on GPU path
□ 11. Commit as "gpu: Wave 5.3 Phase 1 — gpu::AtomEngine pair forces"
```

Estimated: 300-400 LOC CUDA + 150 LOC host wrapper. ~2-3 hours of
careful work + debugging.

---

## 6. Known risks

| Risk | Mitigation |
|---|---|
| CPU/GPU numerical drift | Parity tests with strict 1e-12 tolerance on pair forces, 1e-10 on integration |
| Host↔device sync overhead dominates for small N | Document it; explicit `AtomEngine::use_gpu(false)` to disable for tiny simulations |
| Barnes-Hut on GPU is hard | Phase 1 uses O(N²) — Barnes-Hut is Phase 3 |
| H-bond needs bond table walks | Phase 2 — upload bond table with atoms each tick |
| Dipole-dipole needs dipole_moment recomputation | Phase 2 — add compute_dipole_moments_kernel |
| Thermostat needs reduction | Phase 2 — use cub::DeviceReduce::Sum or warp reductions |
| Angle strain requires atom→bond indirection | Phase 2 — upload bonds as (atom_i, partner_id, r_eq, k_bond) tuples |

---

## 7. What ships in this commit (Wave 5.3 scaffolding)

This doc + placeholder files for the two new CUDA engines. The actual
kernel implementations will land in follow-up commits as Phase 1 / 2 / 3
material. Until then, `AtomEngine` and `ParticleEngine` stay CPU-only —
tests using them continue to run on CPU. No regression.

**Next commit** (Wave 5.3 Phase 1): minimal gpu::AtomEngine with pair
forces only. Validated by a new `test_gpu_atom_parity.cpp`.

---

## 8. Wave 5.3 Phase 1 shipped (51a625b)

Landed as commit `gpu: Wave 5.3 Phase 1 — gpu::AtomEngine pair-force
backend`. Coulomb + vdW Lennard-Jones handled on the device. Bonds,
angle strain, dipole-dipole, thermostat, h-bonds still CPU.

Parity evidence (from test_atom_engine_forces cpu_gpu_parity section):
- Ionic: max abs err 5.9e-23, rel err 1.6e-16 (double-precision noise)
- Ionic+vdW: max abs err 1.55e-10 (CPU total 9.4e-9)

## 9. Wave 5.4 — gpu::ParticleEngine Phase 1 plan

Parallel port for ParticleEngine. Phase 1 covers Coulomb + Gravity
(the two toggles that ship ON by default in `minimal()`). Any test
with `toggles.{strong, exchange, radiation, spin_orbit, relativistic,
lorentz, magnetic_dipole}` on stays on the CPU path for now.

**Files**:
- `engine/include/ftd/gpu_particle_engine.h` (ParticleBuffers, ParticleEngineGpu)
- `engine/cuda/particle_engine_gpu.cu` (pair-force kernel + host wrapper)
- `engine/cuda/CMakeLists.txt` (add source)
- `engine/include/ftd/particle_engine.h` (set_use_gpu, opaque pimpl)
- `engine/src/particle_engine.cpp` (fast path in compute_all_forces)
- `engine/tests/test_pe_forces.cpp` (new cpu_gpu_parity section)

**Kernel scope** (`particle_pair_forces_kernel`):
```
for each i:
    for j ≠ i:
        r = pj.pos - pi.pos
        r² = r·r + soft²
        r̂ = r/|r|

        if (coulomb):
            F += -ALPHA_EFT q_i q_j / (4π r²) r̂
        if (gravity):
            F += +G_N m_i m_j / r² r̂

    force_x[i] = F_x; force_y[i] = F_y; force_z[i] = F_z
    f_coulomb[i] = ...;  f_gravity[i] = ...
```

**Phase 2** (deferred): strong, exchange, lorentz, magnetic_dipole,
spin_orbit, radiation, relativistic. The non-pairwise radiation +
relativistic corrections keep running CPU after the pair kernel
returns (matches compute_all_forces order of operations).
