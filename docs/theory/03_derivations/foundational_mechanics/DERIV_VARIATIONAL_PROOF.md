# Variational scope: field-sector stationarity and production-rule replay

## Corrected status after FTD-0467 and FTD-0567

**Status:** `[PARTIAL THEOREM — FREE FIELD/J-SOURCE VARIATION] + [INTEGRATION REPLAY — SELECTED FORCE RULES] + [RETRACTED — ALL-UPDATE / STATE-TRANSITION GENERATOR]`
**Current verifier:** `engine/tests/test_action_stationarity.cpp`. The historical `test_variational_proof.cpp` and its “60 checks” tally are no longer present in the active test tree.
**Controlling results:** FTD-0467 proves the production matter-force branches are not the matter-side variations of the written state-flux interaction. FTD-0567 proves the written action cannot generate the production genesis magnitude threshold, drain, stochastic acceptance, or evaporation erasure.

**Depends on:**

- [SPEC_FTD_LAGRANGIAN.md](../../01_reference/SPEC_FTD_LAGRANGIAN.md) — selected field/kinematic action specification
- `engine/include/ftd/lagrangian.h` — C++ implementation of the six active diagnostic terms
- `engine/src/render_bridge.cpp` — Simulation update rules (phase_read, phase_write, phase_forces, etc.)

---

## Overview

The implemented diagnostic evaluates a discrete field/kinematic functional:

$$S[s, J] = \sum_t \sum_v \mathcal{L}(s, J, \partial J, \nabla J)$$

The active evidence has two distinct epistemic classes that must not be conflated:

1. **Variational:** the free-field stencil and stationary electric source match the written `J`-variation in their scoped sector.
2. **Integration replay:** selected production force formulas can be recomputed from the same snapshots. Replaying a coded formula is not proof that it is the variation of the written action.

The former Part 2 claim, `delta S/delta s` generates manifestation, evaporation, and transmutation, is retracted. Those update rules are threshold/stochastic transactions not present in the active action implementation.

### Test Architecture (v2)

The current test independently recomputes the field equation and selected force formulas. Its own header explicitly states that the selected force formulas are not all matter-side variations of the written interaction.

Two verification patterns are used:
- **Variational pattern:** Perturb the field variable by ±ε, recompute the action, compare -δS/δφ to the simulation's EOM output (Section A).
- **Integration pattern:** Run a simulation tick, compare the stored force diagnostics (`f_coulomb`, `f_strong`, `f_magnetic`, `f_gravity`) to analytical formulas (Sections B-H).

---

## Part 1: Field variation plus selected production-formula replay

**Scope guard:** Section A is the variational core. Sections B–J below preserve the historical replay record, but their agreement establishes implementation consistency of selected formulas, not derivation from one common action. FTD-0467 controls wherever the older prose says otherwise.

### Section A: Wave Equation (Laplacian) — Variational

**Lagrangian term:** $V_{\text{wave}} = \frac{c^2}{2} \sum_{\text{edges}} |J(v) - J(n)|^2$

**Euler-Lagrange equation:** $\ddot{J} = c^2 \nabla^2 J$

**Method:** Inject a particle, run 10 ticks to establish a flux field, then:
- Way 1: Compute the EOM acceleration from the simulation's discrete operators (Laplacian + coupling gradients)
- Way 2: Perturb J(probe) by ±ε = 10⁻⁶ per component, recompute V[J] over all edges, compute -ΔV/(2ε)

**Result:** All 3 components match to relative error ~10⁻⁹ (4 checks).

### Section B: Coulomb Force — Integration

**Lagrangian term:** $\mathcal{L}_{\text{coupling}} = -g_c \cdot s \cdot (\nabla \cdot J)$

**Force:** $F_{\text{Coulomb}} = -\alpha \cdot q_1 q_2 \cdot \hat{r} / r^3$

**Method:** Place opposite-sign particles at separations r = 3, 5, 8, 12. Run `phase_forces()`. Compare the simulation's `f_coulomb` diagnostic field to the analytical formula.

**Result:** Exact match at all separations (relative error < 10⁻¹⁵). Like charges confirmed to repel (5 checks).

### Section C: Yukawa (Strong) Force — Integration

**Lagrangian term:** $\mathcal{L}_{\text{strong}} = -\alpha_s \cdot \rho \cdot \rho_{\text{screened}}$

**Force:** $F_r = -\alpha_s \cdot e^{-Mr}/r^2 \cdot (1 + Mr)$

**Method:** Same as B, but comparing `f_strong` to Yukawa formula at r = 2, 4, 6, 8.

**Result:** Exact match at all separations (5 checks).

### Section D: Lorentz (Magnetic) Force — Integration

**Lagrangian term:** $\mathcal{L}_{\text{velocity}} = -g_c \cdot s \cdot (\mathbf{v} \cdot \mathbf{J})$

**Force:** $\mathbf{F} = g_c \cdot q \cdot \mathbf{v} \times (\nabla \times \mathbf{J})$

**Method:** Create a uniform B-field by setting J_y = B₀·x (so curl(J)_z = B₀). Place a charged particle moving in +x with speed 0.5. Run `phase_forces()`. Compare `f_magnetic` to the analytical v×B cross product.

**Result:** Force perpendicular to both v and B, correct magnitude. curl(J)_z = B₀ confirmed (3 checks).

### Section E: Gravity — Integration

**Lagrangian term:** $\mathcal{L}_{\text{BI}} = -K_B \sqrt{1 - v^2 - L^2}$, where $L = \rho / (\rho + K_B)$

**Force:** $\mathbf{F}_{\text{grav}} = G_N \cdot \nabla \rho$

**Method:** Create a linear density ramp along x (flux magnitudes 3.0, 2.5, 2.0, 1.5, 1.0). Place a particle at center. Compute expected force from grad(ρ). Run `phase_forces()`.

**Result:** Exact match. Force points toward density peak (2 checks).

### Section F: Rayleigh Dissipation — Integration

**Dissipation function:** $\mathcal{R} = \frac{1}{2} \gamma |\dot{J}|^2$, with $\gamma = \alpha$

**Update rule:** $\dot{J} \to \dot{J} \cdot (1 - \gamma)$

**Method:** Set wave_vel = (0.5, -0.3, 0.1) and flux = (1.0, 0.5, -0.2). Run one tick with only damping enabled. Verify:
- wave_vel components match w₀ × (1 - DAMPING)
- flux components match (f₀ + w₀) × (1 - DAMPING) — the kinematic update flux += wave_vel occurs before damping

**Result:** Machine-precision match for all 4 checks (< 10⁻¹²).

### Section G: Born-Infeld Bandwidth Limit — Simulation + Analytical

**Lagrangian:** $\mathcal{L}_{\text{BI}} = -K_B \sqrt{1 - v^2 - L^2}$

**Method (simulation):** Place a particle with v = (0.95, 0.3, 0) and high density (5 K_B). Run one tick. Verify `enforce_bandwidth()` caps v² + L² < 1.

**Method (analytical):** Verify the Legendre transform H = p·v - L equals the analytical formula K_B(1 - L²)/√(1 - v² - L²), and that at L = 0 it reduces to relativistic energy K_B γ.

**Result:** Budget = 0.9999 after enforcement. Legendre transform exact (3 checks).

### Section H: Complete Force Consistency — Integration

**Test:** Multiple particle configurations at different separations and charge signs:
- r = 4, 8, 12 (opposite charges q = +1, -1)
- r = 6 (same charges q = +1, +1)

For each case, compare the simulation's `f_coulomb + f_strong` to the analytical Coulomb + Yukawa sum.

**Result:** Exact match (relative error < 10⁻¹⁵) across all configurations (4 checks).

### Section I: Gauss Constraint

**Lagrangian term:** $\mathcal{L}_{\text{Gauss}} = \frac{\lambda_G}{2} (\nabla \cdot J - \rho_q)^2$

**Method:** Inject a charged particle, run 20 ticks. Verify:
- div(J) is nonzero at the charge location (constraint is active)
- Integrated div(J) over the periodic lattice = 0 (divergence theorem)
- Gauss violation is finite (soft constraint)

**Result:** All 3 checks pass (total div = 0 to < 10⁻¹⁰).

### Section J: Completeness

**Constant relationships verified:**
- ALPHA = 1/X_PLUS (from master quadratic)
- G_C = √ALPHA (state-flux coupling)
- G_N = 1/(B₃ + N_C)² (gravitational coupling)
- K_GENESIS = 3 K_B (genesis threshold)
- BINDING_ENERGY = K_B φ (golden ratio binding)
- DAMPING = ALPHA (dissipation rate)

**Force completeness test:** Run a 2-particle system. Verify that the vector sum of all diagnosed forces (`f_coulomb + f_strong + f_magnetic + f_gravity`) equals the particle's velocity change. This is a real completeness check — no hardcoded passes.

**Result:** Sum(forces) = velocity change (relative error < 10⁻⁶). All force channels verified as active or correctly zero (10 checks).

| Implemented object | Licensed relation | Scope |
|--------------------|-------------------|-------|
| field kinetic + gradient terms | free `J`-variation matches the production stencil | variational, scoped |
| state-flux coupling | written `J`-side source matches the stationary source term | variational, scoped |
| selected Coulomb/gravity/Lorentz formulas | diagnostic replay matches coded formulas | integration replay, not common-action proof |
| Gauss penalty/projector | selected `div J=rho` constraint realization | does not prove full-event conservation or U(1) |
| Rayleigh term | selected non-conservative damping description | not conservative action |
| genesis/evaporation | absent from the written action | common-action claim closed negative by FTD-0567 |

---

## Part 2: State-transition claim — retracted

The active action code contains no `L_HIGGS`, `K_GENESIS`, exponential manifestation probability, kinetic drain, or evaporation rule. Its Born–Infeld term is evaluated without a factor of `s`, so it cancels in a candidate-state comparison.

### Section K: Genesis (0 -> +/-1) — historical integration test, not a variation

The formerly cited `L_HIGGS = K_B rho(1-s^2)` is not present in `lagrangian.h` or `lagrangian.cpp`. Production eligibility depends on `|J|>K_GENESIS`, while the written candidate-state terms depend on `div J` and `s`.

FTD-0567 supplies an exact counterexample: two uniform fields with amplitudes `K_GENESIS/2` and `2K_GENESIS` have identical candidate-state action values because both have `div J=0` and `grad J=0`, yet only the second is production-eligible. No minimization or maximization of the implemented values reproduces that threshold.

### Section L: Claimed self-field restoration — retracted as an action result

The active genesis code drains superthreshold flux; it does not contain a rule that scales a manifested site's flux up to `K_B`. An annihilation or persistence integration test does not derive either update from the written action.

### Section M: Weak transmutation — integration replay only

**Lagrangian term:** $\mathcal{L}_{\text{weak}} = -\alpha_W |s| \cdot \sigma(k(\text{stress} - \theta))$

where σ is a sigmoid function and stress = |div J| + |curl J| + |∇ρ|.

**Method:**
- Test 1: Verify sigmoid properties of L_weak at different stress levels (below, at, and above threshold).
- Test 2: Create extreme flux gradients around a particle (stress = 15 >> threshold 1.533). Run `phase_weak`. Verify polarity flips (s → -s).
- Test 2b: Place a particle with small uniform flux (stress ≈ 0). Run `phase_weak`. Verify NO flip occurs.
- Test 3: Verify analytically that coupling energy determines preferred polarity direction.

**Note:** `phase_weak` implements transmutation as `s → -s` when stress exceeds threshold. The L_weak term determines WHEN transmutation activates (via the sigmoid), not which direction the flip goes. The direction is determined by the coupling energy term L_coupling = -g_c s div(J).

**Correct status:** high/low-stress behavior may be replayed as an implemented threshold rule. This document does not establish it as a discrete action variation.

---

## Summary of Results

| Section | What is proved | Test pattern | Error | Status |
|---------|---------------|-------------|-------|--------|
| A | Wave EOM: δS/δJ = c²∇²J | Variational (ε perturbation) | ~10⁻⁹ | PASS |
| B | Coulomb force = -α q₁q₂ r̂/r³ | Integration (sim vs formula) | < 10⁻¹⁵ | PASS |
| C | Yukawa force = -αₛ e⁻ᴹʳ(1+Mr)/r² | Integration (sim vs formula) | 0 | PASS |
| D | Lorentz force = gₒ q v×B | Integration (sim vs formula) | 0 | PASS |
| E | Gravity = G_N ∇ρ | Integration (sim vs formula) | 0 | PASS |
| F | Dissipation: ẇ → ẇ(1-γ) | Integration (sim output) | < 10⁻¹² | PASS |
| G | BI bandwidth caps at v²+L² < 1 | Sim enforcement + analytical | exact | PASS |
| H | Full force: F_sim = F_analytical | Integration (4 configs) | 0 | PASS |
| I | Gauss: ∫div(J) = 0 on periodic lattice | Simulation | < 10⁻¹⁰ | PASS |
| J | Sum(forces) = Δv (force completeness) | Integration (real check) | < 10⁻⁶ | PASS |
| K | Genesis from candidate-state action | Exact uniform-field counterexample | exact | **RETRACTED / FAIL** |
| L | Self-field restoration from action | active source audit | absent | **RETRACTED** |
| M | Weak threshold behavior | Integration replay | implementation match | **NOT VARIATIONAL** |

The historical “60 checks, 0 failures” tally is provenance for a removed test and cannot support the active all-update claim. The current `test_action_stationarity.cpp` verifies seven scoped field/replay categories and explicitly excludes the common matter-side-action interpretation.

---

## Significance

The surviving theorem is narrower: the free field and stationary source possess a checked discrete `J`-variation in their declared sector. Several selected force formulas have consistent diagnostic replays. The production engine as a whole is **not** currently derived from one action.

Genesis/evaporation supplies the clearest counterexample: its magnitude threshold, exponential random acceptance, branch-dependent drains, and label-erasing evaporation have no corresponding terms in the written action. FTD-0567 closes the current frozen-variable conservative common-action reading.

### What this does NOT prove

- That the Lagrangian is unique (other actions could produce the same dynamics)
- That the Lagrangian describes physical reality (that requires experimental tests)
- That the written action generates genesis, evaporation, annihilation, weak transmutation, or every selected force branch
- That fine-spacing convergence recovers known physics (that is a separate claim, addressed in DERIV_QFT_GRT_BRIDGE.md)
- That the constants are correctly derived or physically identified. The current canonical status is split across `SPEC_ALGEBRAIC_SPINE.md`, `SPEC_FQCR.md`, and `TRACKER_ONTIC_TRUTH.md`: G* and the master quadratic are theorem-level algebra; `x_+ = 1/α` remains [STRONGLY MOTIVATED CONJECTURE].

### Proof levels remaining

| Level | Statement | Status |
|-------|-----------|--------|
| 1 | δS = 0 → scoped free-field/stationary-source rules | **PARTIAL THEOREM** (this document) |
| 1b | one conservative action → all production updates | **CLOSED NEGATIVE for the current frozen genesis/evaporation map** (FTD-0567) |
| 2 | Long-wavelength behavior → Maxwell + Schrodinger (error O(a^p) at fine spacing) | [THEOREM] (see DERIV_QFT_GRT_BRIDGE.md) |
| 3 | Algebraic constants from G*; physical identifications with α, masses, mixing | Algebraic spine [THEOREM]; physical identifications [STRONGLY MOTIVATED CONJECTURE] / [SELECTION] / [PARAMETRIC] per LEDGER |
| 4 | Substrate → aggregate QM statistics | [OPEN] |
| 5 | Novel experimental prediction | [OPEN] |

---

## Test Location

`engine/tests/test_action_stationarity.cpp`

Build and run:
```bash
cmake --build build --config Release --target test_action_stationarity
./build/Release/test_action_stationarity.exe
```
