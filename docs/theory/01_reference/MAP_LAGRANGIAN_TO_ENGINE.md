# MAP · Lagrangian Term ↔ Engine Implementation

**Tag:** [REFERENCE]
**Date:** 2026-05-05
**Status:** [REFERENCE] — explicit cross-reference table from `SPEC_FTD_LAGRANGIAN.md` §3 to `engine/src/lagrangian.cpp` and the per-phase implementation files.
**Purpose:** Phase R1 deliverable of the FTD-EFT roadmap. Closes the long-standing "force-dictionary unification" gap (Critical Gap 4 in the 2026-05-05 Phase-1 audit): every Lagrangian term in the canonical spec is mapped to its line-anchored engine site and the diagnostic that verifies the correspondence is identified.

The intent is twofold:

1. **For readers of `SPEC_FTD_LAGRANGIAN.md`**: where in the engine does each term live, and what test checks that the engine actually implements the analytical action?
2. **For readers of `engine/src/`**: where in the canonical spec does each line of physics-bearing code come from?

Cross-reference is line-anchored at HEAD `8b1a750` (post the 2026-05-04/05 Tier-A + Tier-B engine cleanup). If line numbers drift, this doc carries no force; refresh against the live source.

---

## §1 — Three-term action (analytical) ↔ Six-term engine decomposition (computational)

`SPEC_FTD_LAGRANGIAN.md` §3.3 writes the **analytical** action as three terms in $\mathcal{L}_\text{matter}$ plus a gravitational sector. The engine tracks **six** independent per-site terms because the field-sector kinetic + gradient (Terms 5, 6) are the weak-field expansion of the Born-Infeld core (Term 1), tracked separately for diagnostic purposes — not double-counted (see SPEC §3.6 footnote and `lagrangian.h:75-76`).

| # | Spec name (§3.5/§3.6) | Expression | `lagrangian.h` function | Per-tick caller in engine | Notes |
|---|---|---|---|---|---|
| 1 | Born-Infeld core (particle sector) | $-K_B\sqrt{(f^2-v^2)/f}$ | `born_infeld_term()` at `lagrangian.h:29-31` (delegates to `Voxel::born_infeld_core()`) | accumulated by `compute_lagrangian_diagnostics()` at `lagrangian.cpp:27`; **integrated** by the γ_FTD momentum step at `phase_forces.cpp:225-253` (CPU) and `kernels_forces.cu:342-378` (GPU, BH-F2 commit `2504c9b`) | Encodes rest mass + speed limit. EOM gives γ_FTD-respecting velocity update with bandwidth $v < f$. |
| 2 | State-flux coupling (electric) | $-g_c\,s\,(\nabla_L\cdot\mathbf{J})$ | `coupling_term()` at `lagrangian.h:36-38` | $\delta L/\delta J$ source term lands in `phase_read.cpp` (state-flux coupling); $\delta L/\delta s$ lands in `phase_forces.cpp:100-105` as $F_\text{EM} = -\alpha s\,\nabla\phi_C$ (Poisson) or $-\alpha s\,\nabla(\nabla\cdot\mathbf{J})$ (legacy gradient). GPU mirror at `kernels_forces.cu:235-244`. | Coupling constant $g_c = \sqrt{\alpha}$ (constants.h:127). The two EM regimes are different blockings of this same term — see `[OPEN, R2]` DERIV_EM_REGIMES_UNIFIED. |
| 3 | Velocity coupling (magnetic) | $-g_c\,s\,(\mathbf{v}\cdot\mathbf{J})$ | `velocity_coupling_term()` at `lagrangian.h:43-48` | EOM gives Lorentz force $F_\text{Lorentz} = \alpha\,s\,(\mathbf{v}\times\nabla\times\mathbf{J})$ at `phase_forces.cpp:131-135` (CPU) and `kernels_forces.cu:266-286` (GPU). | Vanishes for $\mathbf{v}=0$. After CS-F8 cleanup (`56985a4`), all uses standardised on bare `ALPHA` (numerically equal to $G_C^2$ to 1e-8 by `constants.h:127` static_assert). |
| 4 | Gauss constraint | $-\lambda_G(\nabla_L\cdot\mathbf{J} - \rho)^2$ | `gauss_term()` at `lagrangian.h:57-60`; finite $\lambda_G = 100.0$ at `lagrangian.h:55` | constraint enforced each tick by SOR Poisson on CPU at `engine/src/poisson_solvers.cpp:21-84` and cuFFT on GPU at `engine/cuda/kernels_poisson.cu` (CALLSTACK F6 RESOLVED — both solvers documented in `engine/SPEC_ENGINE.md:937-946`). Per-site violation summed in `compute_lagrangian_diagnostics()` at `lagrangian.cpp:30`. | Spec says $\lambda_G \to \infty$ exactly; engine uses $\lambda_G = 100$ as a finite penalty calibrated such that residual $\nabla\cdot\mathbf{J} - \rho < 10^{-8}$ at end of each tick under cuFFT, $\sim 10^{-4}$ under SOR. |
| 5 | Field kinetic energy | $\tfrac{1}{2}\lvert\Delta_t\mathbf{J}\rvert^2$ | `field_kinetic_term()` at `lagrangian.h:77-79` | $\Delta_t\mathbf{J}$ stored as `Voxel::wave_vel`, leapfrog-updated in `phase_write.cpp` (CPU) and `kernels_stencil_single.cu` (GPU). | NOT double-counted with Term 1 — `wave_vel` is the field oscillation velocity, distinct from the manifested-particle velocity that drives Born-Infeld. See header note `lagrangian.h:75-76`. |
| 6 | Field gradient energy (18-pt) | $-\tfrac{1}{2}c^2\sum_\mu w_\mu\lvert\Delta_\mu\mathbf{J}\rvert^2$ with $w_\text{face}=1/3$, $w_\text{edge}=1/6$ | `field_gradient_term()` at `lagrangian.h:86-100` | variational $\delta/\delta J$ produces the 18-point Laplacian stencil used by `phase_read.cpp` (CPU isotropic 18-pt) and `kernels_stencil_*.cu` (GPU). | Stencil weights $\{1/3, 1/6\}$ are currently empirical at the spec level; the variational derivation is queued as `DERIV_18PT_LAPLACIAN_VARIATIONAL.md` in R2. |
| — | Rayleigh dissipation $R$ | $(\alpha/2)\,\lvert\mathbf{v}_\text{wave}\rvert^2$ | `rayleigh_dissipation()` at `lagrangian.h:64-66` | applied as `flux *= (1 - DAMPING)^dt` near manifested particles in `phase_write.cpp` (selective_damping mode). Larmor-modulated when `larmor_radiation = true`. | Not part of the action $S$; enters EOM via $\frac{d}{dt}\frac{\partial L}{\partial \dot{q}} - \frac{\partial L}{\partial q} = -\frac{\partial R}{\partial \dot{q}}$. Currently [IMPOSED] per `ontic.h:771` ASSUMP.6; either-derive-or-justify queued as `DERIV_DAMPING_RAYLEIGH.md` in R2. |

**Gravitational sector** ($\mathcal{L}_\text{grav} = -\frac{1}{8\pi G}\,\lvert\nabla_L\mathcal{L}\rvert^2$ in spec §3.3) is implemented as the latency-field Poisson solve: `RenderBridge::solve_latency_poisson()` at `render_bridge.cpp:280` (CPU) and `gpu_solve_latency_poisson()` at `gpu_engine.h:93` (GPU). The tier-2 density gradient at `phase_forces.cpp:115-125` (CPU) / `kernels_forces.cu:245-263` (GPU) is the per-particle force $F_\text{grav} = G_N\nabla\rho$ that emerges from this sector's EOM (see SPEC §4.2 derivation of $\nabla_L^2\mathcal{L} = 4\pi G\rho_\text{mass}$).

---

## §2 — Verification: the EL-residual diagnostic

The engine's correspondence to the analytical action is not a claim — it's a measurement. Three diagnostic functions, defined in `lagrangian.cpp`, independently recompute the Euler-Lagrange residuals:

| Diagnostic | Function | Where | What it verifies |
|---|---|---|---|
| Field-EOM residual | `compute_el_residual()` | `lagrangian.cpp:69-95`, decl. `lagrangian.h:202` | After `phase_read()`, `delta_j_[i]` should equal $c^2\nabla^2\mathbf{J} + g_c\nabla s + g_c\nabla\times(s\,\mathbf{v})$. Returns `{rms, max_abs}`; expected RMS $\sim 10^{-15}$. |
| Particle-EOM residual | `compute_particle_el_residual()` | `lagrangian.cpp:97+`, decl. `lagrangian.h:220` | After `tick()`, `force_diag_[i]` for each manifested voxel should equal independently-recomputed $F_\text{EM} + F_\text{grav} + F_\text{Lorentz}$ from the Lagrangian's $\delta L/\delta x$. Returns `{rms, max_abs, particle_count}`. |
| Lagrangian-density audit | `compute_lagrangian_diagnostics()` | `lagrangian.cpp:7-65`, decl. `lagrangian.h:198` | Sums every per-site term independently for cross-checking. Populates `LagrangianDiag` (`lagrangian.h:156-187`) with field-kinetic, field-gradient, Born-Infeld, coupling, velocity-coupling, gauss, dissipation totals + the discrete action $S = \sum_v L(v)$ + Gauss-constraint violation maxima. |

These run in tests `engine/tests/test_lagrangian_diagnostics.cpp` and similar; the field-EOM residual passes at machine epsilon at every standard L. **This is the strongest possible operational evidence that the engine's tick cycle implements the analytical action faithfully** — anything other than this reading would surface as a non-machine-epsilon residual.

---

## §3 — Calibration-gauge interface (where dimensional content enters)

`SPEC_FTD_LAGRANGIAN.md` §1 declares the calibration $a \equiv \ell_P$ as a **gauge** (FTD-0137), not an axiom. The engine respects this through the constants chain:

| Symbol | Engine declaration | Origin |
|---|---|---|
| $K_B$ (manifestation threshold) | `constants.h` (via `using ontic::*`); ontic layer in `ontic/particle_masses.h` | $M_P\sqrt{2\pi}\,(16/3)\,\alpha^{11}$ — anchored to Planck mass under $a \equiv \ell_P$ |
| $g_c$ (state-flux coupling) | `using ontic::G_C` at `constants.h:103` | $\sqrt{\alpha}$ — see `ontic/gauge_couplings.h` |
| $\alpha$ (fine structure) | `constants.h:127` `inline constexpr double ALPHA_EFT = G_C * G_C` | $1/x_+$ from master quadratic — see `ontic/master_quadratic.h` |
| $G_N$ (Newton's constant) | per `constants.h` | derived from $K_B$ + $\alpha_G$ — see `DERIV_NEWTON_FROM_SUBSTRATE.md` |
| $\lambda_G$ (Gauss penalty) | `lagrangian.h:55` `LAMBDA_G = 100.0` | finite numeric calibration of the constraint strength; spec target $\lambda_G \to \infty$ |
| $C$ (lattice speed) | `constants.h` `using ontic::C_SPEED` | $1/\sqrt{3}$ from CFL stability on cubic lattice |
| DAMPING ($\alpha$) | `constants.h` | `[IMPOSED]` per `ontic.h:771` ASSUMP.6 (queued for R2 closure) |

Dimensional translation enters only through the gauge declaration in `SPEC_FTD_LAGRANGIAN.md` §1 + the $K_B$ chain. Dimensionless predictions ($\alpha$, mass ratios, mixing angles) are calibration-invariant.

---

## §4 — Per-phase tick site index

For readers tracing the execution path, here's where each Lagrangian term enters per tick:

```
RenderBridge::tick()                                            [render_bridge.cpp:344]
│
├─ toggles.validate()                                           [:345-373, F3 audit RESOLVED]
│
├─ [GPU fork]  backend_->tick(); accumulate_proper_time(); update_energy_ledger(); return;  [:378-384]
│
├─ phase_read()                                                 [render_bridge_phases/phase_read.cpp]
│   │
│   ├─ Term 5+6 (field sector) implicit: 18-pt Laplacian + leapfrog source
│   │    via δL/δJ = c²∇²J + g_c∇s + g_c∇×(s·v)
│   │
│   └─ EL residual independently audited by compute_el_residual()  [lagrangian.cpp:69-95]
│
├─ phase_write()                                                [phase_write.cpp]
│   │
│   ├─ Term 5 commits: flux += wave_vel; wave_vel += delta_j (leapfrog)
│   ├─ Rayleigh R: flux *= (1 - DAMPING)^dt (near particles)
│   ├─ Genesis manifestation: |J| > K_GENESIS && s == 0 → s = sign(...)  (Term 1 turns ON)
│   └─ Evaporation: gated on (do_genesis || do_evaporation)               [BH-F6 + 255c1dd]
│
├─ gauss_project()                                              [render_bridge.cpp:411-412]
│   │
│   └─ Term 4 enforced by SOR (CPU) or cuFFT (GPU) Poisson on -(∇·J - ρ)²
│      Verified: max |∇·J - ρ| < 10⁻⁸ (cuFFT) / 10⁻⁴ (SOR)
│
├─ phase_forces()                                               [phase_forces.cpp]
│   │
│   ├─ Term 2 → δL/δs = -α·s·∇φ_C   (Poisson Coulomb mode)              [:100-105]
│   ├─ Term 3 → δL/δs = α·s·(v × curl J)  (Lorentz)                     [:131-135]
│   ├─ Gravitational sector → F_grav = G_N · ∇ρ_t2                       [:115-125]
│   ├─ accel_mag = |F_em + F_grav + F_lorentz|  (BH-F3 unified)          [:201-202, 10f00f9]
│   └─ Term 1 EOM: γ_FTD momentum integration                            [:204-253]
│
├─ phase_movement()                                             [render_bridge.cpp]
│
└─ accumulate_proper_time() + weak_transmutation_cpu()         [transmutation_phases.cpp:15-55]
```

Every line that reads or writes physics-bearing voxel data has a Lagrangian term backing it; conversely, every term in `SPEC_FTD_LAGRANGIAN.md` §3.5 + §3.6 has at least one engine call site in this tree.

---

## §5 — What this map deliberately does NOT cover

To keep the doc focused:

- **Toggle-gated extensions** (color force, weak transmutation, pair production, triad binding): these are not in the canonical 3-term action; they're documented separately in `engine/include/ftd/term_toggles.h` and are conditional physics layered on top. The MAP covers only the always-on Lagrangian.
- **Toggle interactions**: when multiple toggles are active simultaneously, behavior may not be the linear sum of single-toggle effects — non-linearity is documented in the engine's status doc (CLAUDE.md mentions the 2026-05-04 finding).
- **Inter-scale matching** (Scale 0→1 cluster emergence, Scale 1→2 atom emergence, etc.): handled in R5 of the EFT roadmap; this MAP is Scale-0-only.
- **Calibration sensitivity**: how dimensional predictions shift under different gauge choices; tracked in `FOUND_LATTICE_SPACING_GAUGE_FREEDOM.md`.
- **Stochastic kernels** (Langevin, Boltzmann evaporation): governed by the Rayleigh dissipation row and the BH-F5/F8/F9 RNG portability decision pending in `DESIGN_RNG_PORTABILITY.md`.

---

## §6 — Refresh policy

This MAP carries no force when line numbers drift. After any commit that:

- Restructures `lagrangian.h` or `lagrangian.cpp`,
- Renames or moves files in `engine/src/render_bridge_phases/` or `engine/cuda/`,
- Adds or removes terms in `SPEC_FTD_LAGRANGIAN.md` §3,

run a verification pass: grep for the function names and verify each line-anchored citation. The Phase-1 audit pattern (`grep -nE 'function_name'`) suffices.

A formal refresh is queued as part of the R6 manuscript phase, where this doc graduates to a paper-grade appendix.
