# Physics Status — FTD Multi-Scale Engine

> **Canonical engine reference:** [`SPEC_ENGINE.md`](SPEC_ENGINE.md) — version-aligned with code. For force tables, toggle catalogues, and test counts, prefer SPEC_ENGINE.md when this document and SPEC_ENGINE.md disagree.

**Canonical unresolved-work ledger:** [`docs/theory/07_assessment/core_ledgers/TRACKER_OPEN_ITEMS.md`](../docs/theory/07_assessment/core_ledgers/TRACKER_OPEN_ITEMS.md).

---

## Overview

| Scale | Engine | Forces Implemented | Toggles | Force Diag | Tests |
|-------|--------|--------------------|---------|------------|-------|
| 0 | RenderBridge | 6 | 43 (TermToggles) | 5 components | ~100 |
| 1 | ParticleEngine | 9 (+damping) | 11 (ParticleToggles) | 9 components | ~23 |
| 2 | AtomEngine | 8 (+damping, auto-bond) | 12 (AtomToggles) | 8 components | ~12 |
| 3 | MoleculeEngine | (Natively handled via AtomEngine) | — | — | — |

**Total CTests registered: 610** (CPU + GPU-conditional; verified 2026-08-18 by counting `ftd_add_test(...)` registrations in `engine/CMakeLists.txt` — 444 calls minus 12 `BUILD_ONLY` calls that don't register a CTest — plus 178 direct `add_test(...)` calls in the same file; `engine/cuda/CMakeLists.txt` builds only the `ftd_cuda` library and registers no tests itself, and no other `engine/**/CMakeLists.txt` registers tests)

**DagEngine** (sparse-voxel-DAG prototype): `phase_read` + `phase_write` implemented against `SparseVoxelDAG`; `gauss_project`, `phase_forces`, `phase_movement` are `[OPEN]` stubs. **Experimental, not production.** See `include/ftd/dag_engine.h` banner.

---

## Scale 0 — Lattice Field Theory (RenderBridge)

Discrete 3D cubic lattice. Voxel states {-1, 0, +1}. Flux field J in R^3.
**Störmer–Verlet leapfrog wave propagation under the stagger interpretation** (TRACKER_OPEN_ITEMS §1.4 adjudication; verified by `tests/test_leapfrog_integrator_audit.cpp` — the earlier "forward-Euler-like" description was incorrect) + Gauss projection (SOR) + Poisson Coulomb solver + per-tick `EnergyLedger` for conservation drift.

### Forces

| Force | Formula | Toggle | Status | Test Coverage |
|-------|---------|--------|--------|---------------|
| Coulomb (Poisson) | nabla^2 phi = -s; F = -alpha * s * nabla(phi) | `poisson_coulomb` | Implemented | maxwell, em_energy_conservation, coulomb_1d |
| Coulomb (legacy) | F = -alpha * s * nabla(nabla . J) | `forces` (when poisson off) | Implemented | coulomb_isotropy |
| Gravity | F = G_N * nabla(rho_smoothed) | `gravity` | Implemented | gravity_attraction |
| Lorentz | F = alpha * s * (v x B), B = nabla x J | `lorentz_force` | Implemented | campaign_lorentz_measure, larmor |
| Color (Yukawa + linear) | Three-regime: Coulomb → flux tube → linear | `color_forces` | **[PHENOMENOLOGICAL FIT]** — labelling is emergent, force law is imposed. See TRACKER §1.3. | GP-COLOR |
| Exchange (Pauli) | Repulsive exponential, same-spin | `exchange_force` | Implemented (toggle-gated) | GP-EXCHANGE |
| Weak | Chirality-based polarity flip at stress threshold | `weak_transmutation` | Implemented (toggle-gated) | GP-WEAK |
| Strong (Yukawa) | Nuclear Yukawa potential | `strong_force` | Implemented (toggle-gated) | GP-STRONG |

### Toggles (TermToggles — 43 booleans)

**Core (10):** wave_propagation, coupling, damping, genesis, gauss_projection, forces, gravity, poisson_coulomb, movement, lorentz_force

**Extension (33):** evaporation, selective_damping, larmor_radiation, dual_substrate, color_forces, strong_stress_energy, weak_transmutation, strong_force, triad_binding, pair_production, exchange_force, latency_field, exact_dual_gauss, matched_gauss_dynamics, emergent_forces, langevin, symplectic_leapfrog, verlet_wave_integrator, lorentz_period2_floquet, lorentz_bcc_time_floquet, su2_gauge, su3_gauge, symmetric_movement_order, absorbing_boundary, reflective_boundary, field_energy_gravity, cluster_inertia, de_broglie_clock, db_clock_coulomb, confinement, knot_tracking, strict_validation, ew_background_sweep (source: `include/ftd/term_toggles.h` `TOGGLE_SPECS[]`, verified 2026-08-18)

### Force Diagnostics (ForceDiag — per voxel)

| Component | Populated By |
|-----------|-------------|
| f_coulomb | Poisson or legacy Coulomb |
| f_strong | Color/Yukawa force |
| f_magnetic | Lorentz force |
| f_gravity | Density gradient |
| f_exchange | Pauli repulsion |

### Energy Audit

34 fields (`EnergyAudit` struct, `include/ftd/render_bridge_diagnostics.h`, verified 2026-08-18): field_energy, wave_energy, particle_ke, total_energy, gauss_violation, max_gauss_error, self_field_injection, coulomb_pe, E_field_energy, B_field_energy, charge_total, manifested_count, total_poynting, E_L_total, E_R_total, wv_L_total, wv_R_total, chirality_total, strong_energy, weak_energy, particle_rest_energy, particle_energy, particle_momentum, dynamic_energy, cell_volume, field_energy_density_sum, wave_energy_density_sum, strong_potential_energy, strong_gravitational_mass, strong_projection_residual, strong_projection_lambda, strong_projection_events, strong_projection_failures, strong_topology_failures

---

## Scale 1 — Particle Mechanics (ParticleEngine)

Continuous positions + analytical pairwise forces. Velocity Verlet integration.
Softening = 1.0. Speed limit at C_SPEED = 1/sqrt(3). Annihilation at contact.

### Forces

| Force | Formula | Toggle | Status | Phase | Test Coverage |
|-------|---------|--------|--------|-------|---------------|
| Coulomb | F = -alpha * q_i * q_j * r_hat / (4*pi*r^2) | `coulomb` | **Implemented** | 1 | particle_toggles, pe_coulomb, hydrogen |
| Gravity | F = +G_N * m_i * m_j * r_hat / r^2 | `gravity` | **Implemented** | 1 | particle_toggles, gravity_pe |
| Damping | v *= (1 - DAMPING * dt) | `damping` | **Implemented** | 1 | particle_toggles |
| Exchange (Pauli) | F = ALPHA^2 * exp(-r^2/9) / r^2, same-spin same-charge | `exchange` | **Implemented** | 2 | pe_exchange (6 checks) |
| Strong (Yukawa) | Yukawa + linear confinement, color-dependent | `strong` | **Implemented** | 2 | pe_strong (6 checks) |
| Lorentz | F = alpha * q * (v x B_dipole) | `lorentz` | **Implemented** | 2 | pe_lorentz (6 checks) |
| Magnetic dipole | Standard dipole-dipole: 5(mi.r)(mj.r)/r^2 - mi.mj - ... | `magnetic_dipole` | **Implemented** | 2 | pe_magnetic_dipole (6 checks) |
| Spin-orbit | F = alpha/(2m^2 c^2 r^3) * (L . S) * r_hat | `spin_orbit` | **Implemented** | 2 | pe_spin_orbit (6 checks) |
| Radiation reaction | F = -(2/3) * alpha * q^2/(mc^3) * |a_prev|^2 * v_hat | `radiation` | **Implemented** | 2 | pe_radiation (5 checks) |
| Relativistic | F_total *= 1/gamma (last force applied) | `relativistic` | **Implemented** | 2 | pe_relativistic (5 checks) |

### Force Computation Order

Pairwise j-loop: Coulomb → Gravity → Exchange → Strong → Magnetic Dipole → Spin-Orbit.
Then: Lorentz (separate B-field accumulation) → Radiation Reaction → Relativistic (MUST be last).

### Toggles (ParticleToggles — 11 booleans)

- **Active (10):** coulomb (on), gravity (on), damping (on), exchange (off), strong (off), lorentz (off), magnetic_dipole (off), spin_orbit (off), radiation (off), relativistic (off)
- **11th toggle:** relativistic_verlet (off by default; turned on by `minimal()`)
- **Helpers:** `enable_all()`, `minimal()`
- **Backward compat:** `set_damping_enabled()`, `set_gravity_enabled()` delegate to toggles

### Force Diagnostics (ParticleForceDiag — per particle)

| Component | Status |
|-----------|--------|
| f_coulomb | **Populated** |
| f_gravity | **Populated** |
| f_lorentz | **Populated** (Phase 2) |
| f_exchange | **Populated** (Phase 2) |
| f_strong | **Populated** (Phase 2) |
| f_radiation | **Populated** (Phase 2) |
| f_spin_orbit | **Populated** (Phase 2) |
| f_relativistic | **Populated** (Phase 2) |
| f_magnetic_dipole | **Populated** (Phase 2) |

### Phase 2 Campaign: Fine Structure

`campaign_pe_fine_structure` (8 checks): spin-orbit + relativistic energy correction, spin-up/down splitting, small relativistic correction, all forces stable orbit, combined diagnostics, radiation energy loss.

### Diagnostics (ParticleDiagnostics)

Fields: tick, particle_count, total_ke, total_pe, total_energy, total_momentum (Vec3), total_angular_momentum (Vec3)

### Particle Struct Extensions (Phase 2)

- `prev_acceleration` (Vec3): stores previous tick's acceleration for radiation reaction
- `add_particle()` now accepts `spin` and `color` parameters (backward-compatible defaults)

---

## Scale 2 — Atomic / Chemical Physics (AtomEngine)

Composite atoms with Z, N, charge, mass, radius, bonds. All atomic properties derived from ontic chain.
Velocity Verlet integration. dt = 0.01, softening = 0.5.

### Forces

| Force | Formula | Toggle | Status | Phase | Test Coverage |
|-------|---------|--------|--------|-------|---------------|
| Ionic (Coulomb) | F = -ALPHA * Q_i * Q_j * r_hat / (4*pi*r^2) | `ionic` | Implemented | 1 | atom_toggles, atom_engine |
| Van der Waals (LJ 12-6) | F = 24*eps * [2*(sig/r)^12 - (sig/r)^6] / r | `van_der_waals` | Implemented | 1 | atom_toggles, atom_engine |
| Covalent bonds (harmonic) | F = -k * (r - r_eq) * r_hat | `covalent_bonds` | Implemented | 1 | atom_toggles, campaign_h2_molecule |
| Auto-bonding | Formation at r < 1.2*sigma; breaking at r > 2*r_eq | `auto_bonding` | Implemented | 1 | atom_engine |
| Damping | v *= (1 - DAMPING * dt) | `damping` | Implemented | 1 | atom_toggles |
| H-bonds | LJ 10-12 + cos²(θ_DHA) angular | `h_bonds` | Implemented | 3 | ae-water-dimer scenario |
| Dipole-dipole | 1/r⁵ from bond dipoles × Pauling chi | `dipole_dipole` | Implemented | 3 | — |
| Angle strain (VSEPR) | V = k_theta * (theta - theta_eq)^2 / 2 | `angle_strain` | Implemented | 3 | ae-vsepr-* scenarios |
| Torsional (dihedral) | V = V_n/2 * [1 + cos(n*phi - gamma)] | `torsional` | Implemented | 4 | ae-organic-chem |
| Improper Torsions | V = K_improper * Vol^2 / 2 (Planarity) | `improper_torsional`| Implemented | 4 | ae-sp2-planarity |
| Thermostat (Berendsen) | λ = √(1 + dt/τ·(T_target/T - 1)) | `thermostat` | Implemented | 3 | ae-thermal-gas scenario |
| Electronegativity | Pauling chi-driven bond formation threshold | `electronegativity` | Implemented | 3 | — |

### Toggles (AtomToggles — 12 booleans)

- **Active (5):** ionic (on), van_der_waals (on), covalent_bonds (on), auto_bonding (on), damping (off)
- **Phase 3/4 Native (7):** h_bonds, dipole_dipole, angle_strain, torsional, improper_torsional, thermostat, electronegativity (off by default)
- **Helpers:** `enable_all()`, `minimal()`
- **Backward compat:** `set_damping_enabled()`, `set_bonding_enabled()` delegate to toggles

### Force Diagnostics (AtomForceDiag — per atom)

| Component | Status |
|-----------|--------|
| f_ionic | Populated |
| f_vdw | Populated |
| f_bond | Populated |
| f_hbond | Active (JS MockBridge) |
| f_dipole | Active (JS MockBridge) |
| f_angle | Active (JS MockBridge) |
| f_torsion | Zero (Phase 4) |

### Diagnostics (AtomDiagnostics)

Fields: tick, atom_count, bond_count, total_ke, total_pe_ionic, total_pe_vdw, total_pe_bond, total_energy, temperature, total_momentum (Vec3)

---

## Scale 3 — Molecular Physics

Currently reuses AtomEngine with no additional forces. 25 molecular presets in web UI (`particle-catalog.js`).

Future Phase 4 additions: torsional (dihedral) angles, improper torsions (planarity), reaction dynamics.

---

## Web UI Toggle Panels

| Scale | Toggle Panel | Status |
|-------|-------------|--------|
| 0 | 13 active toggles + field viz | Complete |
| 1 | Coulomb, Gravity, Damping + 7 Phase 2 forces | Complete (Phase 2) |
| 2 | Ionic, vdW, Bonds, Auto-bond, Damping + 5 Phase 3 toggles (H-bonds, Angle, Dipole, Thermostat, Electronegativity) + 1 grayed-out (Torsional) | Complete (Phase 3) |

---

## WASM Bindings

| Binding | Function | Status |
|---------|----------|--------|
| PE toggle set/get | `peSetToggle(name, bool)` / `peGetToggle(name)` | Implemented |
| AE toggle set/get | `aeSetToggle(name, bool)` / `aeGetToggle(name)` | Implemented |
| PE force diag | `get_pe_force_diag(idx, component)` | Implemented |
| AE force diag | `get_ae_force_diag(idx, component)` | Implemented |

---

## Implementation Roadmap

| Phase | Description | Status | Dependencies |
|-------|-------------|--------|-------------|
| 1 | Toggle & Diagnostics Infrastructure | **Complete** | — |
| 2 | Scale 1 Force Expansion (7 new forces) | **Complete** | Phase 1 |
| 3 | Scale 2 Force Expansion (5 new forces) | **Complete (JS)** | Phase 1 |
| 4 | Scale 3 Force Expansion (molecular) | Planned | Phases 2+3 |
| 5 | Per-force visualization (color-coded arrows) | Planned | Phase 1 |
| 6 | Performance (neighbor lists, SIMD) | Planned | Phases 2+3 |
| 7 | This living document | **Active** | Continuous |

---

## Runtime Constants (from ontic.h)

Engine constants mix framework integers, structural matches, simulation
parameters, and calibrated/imported reference values. Claim status is
defined by the project ledgers, not by this runtime table.

| Constant | Symbol | Value | Origin |
|----------|--------|-------|--------|
| Fine structure | ALPHA | 0.00729735... | Engine input using the `x_+` master-quadratic structural match; physical identification remains `[STRONGLY MOTIVATED CONJECTURE]` |
| Boltzmann/mass | K_B | 0.511 | Electron-mass anchor (Boltzmann/kinetics scale split to K_MANIFEST := W_SC = 0.505462, FTD-0388) |
| Gravity | G_N | 0.01 | Lattice-scaled simulation gravity parameter |
| Speed of light | C_SPEED | 1/sqrt(3) | **[SELECTION]** (FTD-0407) — not CFL-forced; the production 18-point stencil permits c <= sqrt(3)/2 ~ 0.866 |
| Bohr radius | R_BOHR | 1/(K_B * ALPHA) | Standard definition |
| Damping | DAMPING | ALPHA | Dissipation = fine structure constant |
| Exchange coupling | ALPHA_EXCHANGE | ALPHA^2 | Pauli repulsion strength |
| Exchange range | EXCHANGE_RANGE_SQ | 9.0 | Characteristic exchange length^2 |
| Strong coupling | ALPHA_S | 1.0 | Strong force at lattice scale |
| String tension | SIGMA_STRING | (from ontic.h) | Linear confinement |
| Confinement radius | R_CONFINEMENT | 1.0 | Yukawa→linear transition |
