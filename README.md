# Foundational Ternary Dynamics

[![License: CC BY-NC-SA 4.0](https://img.shields.io/badge/License-CC%20BY--NC--SA%204.0-lightgrey.svg)](https://creativecommons.org/licenses/by-nc-sa/4.0/)
[![Python 3.10+](https://img.shields.io/badge/python-3.10+-blue.svg)](https://www.python.org/downloads/)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![Engine: v2.17.0](https://img.shields.io/badge/engine-v2.17.0-orange.svg)](engine/SPEC_ENGINE.md)
[![Tests: 290+](https://img.shields.io/badge/tests-290%2B-brightgreen.svg)](#tests-and-verification)

A discrete framework built on the CM elliptic curve E_i: y² = x³ − x, its automorphism group {1, i, −1, −i}, and the closed-form transcendental constant G\* = Γ(¼)² / (2·√(2π)·Γ(½)) ≈ 2.958675. The framework's **algebraic spine** is eight theorems about these objects. The framework's **physics interpretations** sit downstream and carry honest epistemic tags.

**Author:** William J Steinmetz III · **FTD version:** 5.34 · **Engine version:** 2.17.0 · **Date:** 2026-04-29

**Navigation:**
- [META_CONTRIBUTOR_ONBOARDING.md](META_CONTRIBUTOR_ONBOARDING.md) — start-here for new contributors
- [META_PROJECT_ATLAS.md](META_PROJECT_ATLAS.md) — task → file table, directory tree, subsystem dependency graph
- [docs/SPEC_FTD.md](docs/SPEC_FTD.md) — authoritative theoretical specification
- [docs/theory/01_reference/SPEC_ALGEBRAIC_SPINE.md](docs/theory/01_reference/SPEC_ALGEBRAIC_SPINE.md) — eight canonical theorems
- [docs/theory/01_reference/SPEC_DIMENSIONAL_MAP.md](docs/theory/01_reference/SPEC_DIMENSIONAL_MAP.md) — dimensionless ↔ dimensional bridge (15-entry reference)
- [docs/theory/07_assessment/LEDGER.md](docs/theory/07_assessment/LEDGER.md) — master claim ledger (FTD-0001..FTD-0111+)
- [docs/theory/07_assessment/CATALOG_PARAMETRIC_INSERTIONS.md](docs/theory/07_assessment/CATALOG_PARAMETRIC_INSERTIONS.md) — honest accounting of ~162 SM quantities

---

## The Algebraic Spine — Eight Theorems

The load-bearing rigorous core of FTD is a list of mathematical theorems with no physics interpretation. They stand on number-theoretic and lattice-Green's-function grounds, independent of any later physical identification. Stating them cleanly — without rhetorical lift from physics — is the project's primary claim.

| # | Theorem | Statement |
|---|---|---|
| 1 | **G\* algebraic identity** | G\* = Γ(¼)² / (2·√(2π)·Γ(½)). Closed-form transcendental real number. |
| 2 | **Master quadratic** | x² − 16·G\*²·x + 16·G\*³ = 0 has roots x_± = 8·G\*² ± √(16·G\*⁴ − 4·G\*³). |
| 3 | **CM curve uniqueness** | Among the nine class-number-1 imaginary-quadratic discriminants {−3, −4, −7, …, −163}, only d = −4 (the lemniscatic curve) yields master-quadratic roots that simultaneously match dimensionless physical constants (1/α, N_c) to permille precision. |
| 4 | **Coefficient 16 = \|Aut(E)\|²** | For E: y² = x³ − x, \|Aut(E)\| = 4 over ℚ̄, so the coefficient 16 in the master quadratic equals \|Aut(E)\|² (three independent routes converge). |
| 5 | **Watson identity** | The body-centered-cubic Watson integral W₃ = G\*²/(2π) (proven via theta-function identities). |
| 6 | **Phase G geometric Coulomb** | The engine's emergent Coulomb plateau α_r(r, L) = 2·r·G_L(r) is identically the periodic lattice Poisson Green's function at every finite L. R² = 1.0000 at L = 384. Zero free parameters, zero fine-structure content. |
| 7 | **Phase J ultralocality** | At L = 2 the partition function factorizes site-locally: Z = ∏_v Z_v(s_v, J_v). |
| 8 | **Harmonic invariant tower** | For the (1+i)-tower of master quadratics, 1/y₊ + 1/y₋ = 1 where y_± := x_±/G\* (LEDGER FTD-0111, added 2026-04-29). |

See [SPEC_ALGEBRAIC_SPINE.md](docs/theory/01_reference/SPEC_ALGEBRAIC_SPINE.md) for full proofs and source citations.

**These are theorems, not physics claims.** Each is a verifiable mathematical identity. The numerical coincidence that the larger root x₊ ≈ 137.036 ≈ 1/α and the smaller root x₋ ≈ 3.024 ≈ N_c is recorded as a separate empirical observation in the next section.

---

## Physics Identifications — Conjectures, Not Theorems

Where the algebraic spine meets nature:

| Quantity | FTD value | CODATA / PDG | Tag | LEDGER |
|---|---|---|---|---|
| 1/α (fine-structure) | x₊ tree-level: 137.0362 (1.26 ppm) | 137.035999177 | **[STRONGLY MOTIVATED CONJECTURE]** | FTD-0013 |
| N_c (color number) | x₋ tree-level: 3.024 (0.80% of 3) | 3 (integer) | **[STRONGLY MOTIVATED CONJECTURE]** | FTD-0014 |
| m_μ/m_e | 3·b₃·(b₃ + N_c) − N_c = 207 | 206.7682830 (6) | **[DERIVED from framework integers]** | FTD-0021 |
| m_τ/m_e | (N_eff + N_base)·207 − 2·N_c·b₃ = 3477 | 3477.23 (23) | **[DERIVED from framework integers]** | FTD-0021 |
| m_p/m_e | N_eff/α + N_base·N_eff + N_c = 1836.47 | 1836.15267343 | **[DERIVED]** (174 ppm) | FTD-0017 |

**Why "STRONGLY MOTIVATED CONJECTURE" rather than "THEOREM" for α and N_c.** Per the April 19, 2026 audit ([AUDIT_MASTER_QUADRATIC.md](docs/theory/07_assessment/AUDIT_MASTER_QUADRATIC.md)), the polynomial x² − 16·G\*²·x + 16·G\*³ is a [THEOREM], but the **physical identification** of x₊ with 1/α and x₋ with N_c is structurally rigid (the 60 000-polynomial scan in [AUDIT_LOOK_ELSEWHERE_RESULTS.md](docs/theory/07_assessment/AUDIT_LOOK_ELSEWHERE_RESULTS.md) confirms the dual-prediction property is non-coincidental) but not a theorem. Reviewers and paper drafts must cite both layers separately.

**The 7-term lattice correction series** (`x₊ → 137.0359991770000…`) historically described in this README at "24-digit precision" is honest only at the polynomial level: CODATA 2022 itself measures α⁻¹ to 11 significant figures (137.035999177(21)). Digits beyond the 12th are post-hoc fit, not prediction. See [LEDGER.md FTD-0013 tag history](docs/theory/07_assessment/LEDGER.md) for the downgrade record.

---

## The Calibration Bridge — Dimensionless ↔ Dimensional

FTD's predictions sit in three theorem-enforced layers:

1. **Dimensionless layer** — pure-number theorems (the 8 spine theorems + dimensionless ratios like m_μ/m_e). Calibration-independent. Falsifiable on their own algebraic content.
2. **Calibration layer** — exactly two SI-dimensional anchors are theorem-enforced as the irreducible minimum:
   - `a_phys ≡ ℓ_P` (one voxel ≡ Planck length) — [CALIBRATION], no-go theorem [FTD-0059](docs/theory/02_foundations/THEOREM_A_PHYS_NO_GO.md) closes all four derivation candidates
   - `K_B = m_e ≈ 0.511 MeV/c²` (mass anchor) — [IMPOSED], no-go theorem [FTD-0096](docs/theory/07_assessment/LEDGER.md) closes the mass-from-axioms route
   - `t_phys = √3·ℓ_P/c ≈ 9.34×10⁻⁴⁴ s` is [DERIVED] from `a_phys` + cubic-lattice CFL constraint c_lat = 1/√3
3. **Dimensional layer** — physical-unit predictions (m_e in MeV, lifetimes in seconds, lengths in metres). Every dimensional FTD value is a dimensionless ratio multiplied by one of the two calibration anchors. The bridge mechanism IS this arithmetic.

The full 15-entry reference (7 spine theorems + 4 dimensionless predictions + 3 calibration declarations + 1 worked dimensional application) is canonical at [SPEC_DIMENSIONAL_MAP.md](docs/theory/01_reference/SPEC_DIMENSIONAL_MAP.md), backed by [dimensional_map.json](docs/theory/01_reference/dimensional_map.json) (single source of truth, 12 pytest assertions, idempotent renderer). Cite map entry ids when drafting papers or replying to reviewers about whether a claim is dimensionless or calibration-conditional.

---

## EFT Recovery Program (April 2026)

A pre-registered measurement campaign asking whether the engine qualifies as a Wilsonian effective field theory. Five pillars — Ward identities, Lorentz covariance, RG flow, operator expansion, continuum matching — were measured on the lattice against expectations committed to the repository **before any code ran**. Seven-phase campaign (Phase 0 pre-registration → Phase F 4-point continuum extrapolation) is complete.

**Headline:** the engine's emergent Coulomb-tail coupling plateaus at ~3.6× α_ref at L = 384, not at 1/137. **Phase G reframe** (Theorem 6 above) showed this plateau is identically the periodic lattice Poisson Green's function — zero free parameters, zero fine-structure content. The plateau is therefore lattice geometry, not a QED deviation. The Day-2 interim "1.23× α_ref" claim from under-equilibrated CPU data is **retracted**.

**Spec:** [SPEC_EFT_RECOVERY_PROGRAM.md](docs/theory/10_eft_program/SPEC_EFT_RECOVERY_PROGRAM.md) · **Day-2 campaign:** [DERIV_DAY2_CAMPAIGN.md](docs/theory/10_eft_program/DERIV_DAY2_CAMPAIGN.md) · **Phase G derivation:** [DERIV_EMERGENT_COULOMB_GEOMETRIC.md](docs/theory/10_eft_program/DERIV_EMERGENT_COULOMB_GEOMETRIC.md) · **Audit:** [AUDIT_ALPHA_EXTRACTION.md](docs/theory/10_eft_program/AUDIT_ALPHA_EXTRACTION.md)

**Honest bookkeeping** ([CATALOG_PARAMETRIC_INSERTIONS.md](docs/theory/07_assessment/CATALOG_PARAMETRIC_INSERTIONS.md)): of the ~162 SM quantities FTD reports, **~23 are genuine derivations**, **~129 are parametric insertions** (standard-physics formulas with FTD-supplied inputs), and **~10 are imposed / selected**. The EFT program's contribution is an auditable measurement stack plus the Phase G theorem reframe — not a completed Wilsonian EFT.

---

## Recent Audit + Refactor Cycles (April 2026)

Four substantive maintenance cycles landed this month:

**1. Engine modular refactor sweep** (April 27, 17 commits across 8 phases). The five hottest files were decomposed; physics is bit-identical to pre-refactor, verified by a frozen byte-hash regression gate (`0xcd957b601d47868a`). `viewport.js` 3953→1256 LOC; `wasm-bridge-dag.js` 2395→42 LOC; `render_bridge.cpp` 1231→545 LOC; `kernels_stencil.cu` 1530→deleted-and-split-into-3-TUs. WSL2 GPU parity verified at L=16 and L=32 (`gpu_parity_complete` 70/0 across 20 physics domains). See [META_PROJECT_ATLAS.md §10](META_PROJECT_ATLAS.md) and [docs/audits/AUDIT_2026-04_refactor-sweep.md](docs/audits/AUDIT_2026-04_refactor-sweep.md).

**2. Vacuum-particle scenario suite** (April 28). 15 new `s0-vacuum-*` scenarios — one per canonical elementary or near-elementary particle (3 charged leptons, 3 neutrino flavors, photon, W±/Z⁰/Higgs, proton, neutron, π±, π⁰, K±) — with uniform initialization (lattice-centre injection, zero background field, applyVacuumEnvironment hook) and live telemetry. Spec: [engine/web/docs/SPEC_VACUUM_PARTICLE_SCENARIOS.md](engine/web/docs/SPEC_VACUUM_PARTICLE_SCENARIOS.md). JS↔C++ parity guard.

**3. Four scenario-audit cycles** (April 28). Caught and fixed:
- 11 scenarios that triggered spurious mass genesis (photon, gluon, light-dipole, light-two-slit, quantum-tunnel, quantum-aharonov-bohm, flux-soliton, s0-seed-moore-cell, s0-field-uniform-e, etc.) — the wave amplitude crossed K_GENESIS during evolution and the lattice silently filled with manifested particles. Fix: `toggles.genesis = false` in each scenario body, JS + C++ mirror.
- Helium nucleus topology corrected from a single +1 particle to the proper 4-nucleon (2p + 2n) tetrahedral structure with 1s² electron shell.
- 6 redundant/test-only scenarios removed (`s0-seed-{electron-l3, neutrino, quark, antiquark, proton-candidate, symmetry-regression}`); 12 vacuum-mirror seed scenarios consolidated; `s0-seed-h2-molecule` renamed to `s0-seed-2-hydrogen-atoms` to reflect actual topology (no shared bond).
- 13 legacy `ftd_wasm.cpp` scenarios converted to clean alias map routing to modern equivalents.

**4. Volumetric-alignment audit** (April 28–29). Found and fixed:
- A systemic 0.5-voxel offset between particles (rendered at `(k+0.5, k+0.5, k+0.5)`) and field overlays (rendered at raw `(k, k, k)`) — every E-field arrow, B-field line, Poynting vector, and force volume across the dashboard.
- An X⇄Z layout swap in the WASM `get_flux_volume()` binding — every photon / gluon / wavepacket scenario was rendering with X and Z spatially swapped on the WasmBridge path. Fix: transpose at the binding boundary; single function, ~10 LOC.

---

## The Blind Derivation: From *i* to α

No physics is invoked until the final comparison. Two selection principles remain (steps 9, 12); everything else is forced.

| Step | Result | Status | Method |
|---|---|---|---|
| 1 | i exists | [AXIOM] | x² + 1 = 0 has a solution |
| 2 | ℤ[i] = square lattice | [THEOREM] | Unique ring of integers in ℚ(i) |
| 3 | E_i: y² = x³ − x | [THEOREM] | Unique CM curve, j = 1728 |
| 4 | \|Aut(E_i)\| = 4 | [THEOREM] | The group {1, i, −1, −i} |
| 5 | Γ(¼), Γ(¾) | [THEOREM] | Periods of E_i |
| 6 | G\* = Γ(¼)² / (2·√(2π)·Γ(½)) | [THEOREM] | Spine Theorem 1 |
| 7 | \|Aut\|² = 16 | [THEOREM] | 4² = 16 (Spine Theorem 4) |
| 8 | D = 3 | [THEOREM] | Unique solution of 16 = 2^D · (D−1)! |
| 9 | x² − 16·G\*²·x + 16·G\*³ = 0 | [SELECTION] | Vieta exponents (2, 3) from D |
| 10 | x₊ ≈ 137.036, x₋ ≈ 3.024 | [THEOREM] | Quadratic formula (Spine Theorem 2) |
| 11 | x₊ = 1/α, x₋ = N_c | **[STRONGLY MOTIVATED CONJECTURE]** | Empirical match (downgraded 2026-04-19) |
| 12 | One-loop tadpole correction | [SELECTION] | a = 2/D lattice spacing |
| 13 | 137.036000 (1.26 ppm tree, 9.6 ppb post-correction) | **[CONJECTURE conditional on Step 11]** | Computational check |

See [FOUND_BLIND_DERIVATION_CHAIN.md](docs/theory/02_foundations/FOUND_BLIND_DERIVATION_CHAIN.md) for the full chain with proofs.

---

## Moore Neighborhood Gauge Structure

The 26-neighbor Moore neighborhood decomposes as 6 SC + 12 FCC + 8 BCC. Each sublattice excites a different number of J-components orthogonally:

| Sublattice | Neighbors | Distance | J-components | Gauge group | Tag |
|---|---|---|---|---|---|
| SC (face) | 6 | 1 | 1 | U(1) — electromagnetism | [SELECTION] |
| FCC (edge) | 12 | √2 | 2 | SU(2) — weak force | [SELECTION] |
| BCC (corner) | 8 | √3 | 3 | SU(3) — strong force | [SELECTION] |

The Higgs quartic λ = 3/23 follows from the ternary decomposition {−1, 0, +1} = 2(active) + 1(void), giving λ = sin²θ_W / (2 − sin²θ_W). Tag: [PARAMETRIC] — the formula is electroweak; FTD supplies the inputs.

See [DERIV_MOORE_GAUGE_STRUCTURE.md](docs/theory/03_derivations/DERIV_MOORE_GAUGE_STRUCTURE.md).

---

## Epistemic Honesty

FTD maintains transparent accounting. Every claim carries a tag ([SPEC_EPISTEMIC_LABELS.md](docs/reference/SPEC_EPISTEMIC_LABELS.md)):

| Tag | Meaning | Live count |
|---|---|---|
| [AXIOM] | Lattice postulate (state + position) | 1 |
| [THEOREM] | Proven from axioms | ~35 (algebraic spine: 8 canonical) |
| [SELECTION] | Argued from consistency, not uniquely forced | ~5 |
| [STRONGLY MOTIVATED CONJECTURE] | Empirically rigid identification, not proven (e.g. x₊ ↔ 1/α) | ~4 |
| [DERIVED] | Closed-form lattice / framework derivation | ~23 |
| [PARAMETRIC] | Standard physics formula with FTD-supplied inputs | ~129 |
| [IMPOSED] | External input or anchor (m_e, Higgs VEV, m_Z) | ~10 |
| [OPEN] | Unresolved | ~8 |

Full enumeration: [CATALOG_PARAMETRIC_INSERTIONS.md](docs/theory/07_assessment/CATALOG_PARAMETRIC_INSERTIONS.md). Master claim ledger with reframe history: [LEDGER.md](docs/theory/07_assessment/LEDGER.md).

---

## Selected Predictions — Honest Precision

Reported with current epistemic tags and at honest precision (CODATA 2022 cap ≈ 11 figures).

### Coupling constants

| Parameter | FTD value | CODATA / PDG | Tag |
|---|---|---|---|
| 1/α (master quadratic root, tree) | 137.0362 (1.26 ppm) | 137.035999177(21) | x₊ identification: [STRONGLY MOTIVATED CONJECTURE] |
| 1/α (one-loop lattice correction) | 137.036000 (9.6 ppb) | 137.035999177(21) | conditional on Step 11; [CONJECTURE] |
| sin²θ_W (Weinberg angle) | 3/13 = 0.2308 | 0.2312 (0.19%) | [DERIVED ratio] |
| α_s (strong coupling) | 7/59 = 0.1186 | 0.1179 (0.6%) | [DERIVED ratio] |

### Mass ratios (calibration-independent)

| Parameter | FTD value | PDG | Tag |
|---|---|---|---|
| m_μ / m_e | 207 | 206.7682830 (0.11%) | [DERIVED from framework integers] |
| m_τ / m_e | 3477 | 3477.23 (0.01%) | [DERIVED from framework integers] |
| m_p / m_e | 1836.47 | 1836.15267 (174 ppm) | [DERIVED from framework integers] |

### Dimensional masses (calibration-conditional, anchor K_B = m_e)

| Parameter | FTD value | PDG | Tag |
|---|---|---|---|
| m_H (Higgs boson) | 125.69 GeV (0.47%) | 125.25 GeV | [PARAMETRIC] (SM formula + FTD inputs) |
| v (Higgs VEV) | 246.08 GeV (0.06%) | 246.22 GeV | [PARAMETRIC] |
| m_e | 0.511 MeV/c² (anchor) | 0.51099895 MeV/c² | [IMPOSED] (calibration) |

The dimensional rows are **conditional on the K_B = m_e anchor**, which is declared, not derived. The dimensionless rows are calibration-free.

---

## Project Structure

```
ftd/
  docs/
    SPEC_FTD.md                  # Authoritative specification
    theory/
      01_reference/              # 8 algebraic-spine theorems, dimensional map, master quadratic
      02_foundations/            # Lattice postulates, dimensional selection, no-go theorems
      03_derivations/            # Core physics derivations
      04_coupling/               # Coupling constants and precision
      05_particles/              # Particle physics
      06_consciousness/          # Consciousness and measurement
      07_assessment/             # Epistemic audits, LEDGER, CATALOG
      08_structural/             # Geometry and information theory
      09_mathematical/           # Number theory connections
      10_eft_program/            # EFT recovery program
    audits/                      # Sweep ledgers (refactor, scenario-audit cycles)
    adr/                         # 13 architectural decision records
  engine/                        # C++17 simulation engine, v2.17.0 post-refactor
    include/ftd/                 # 30+ headers (ontic.h umbrella, render_bridge_phases/*)
    src/                         # Phase-decomposed sources, scenarios/{flux,light,quantum,s0_seed,s0_field,vacuum}.cpp
    tests/                       # 250+ C++ tests (ctest LABELS: unit/physics/golden/slow/gpu)
    cuda/                        # CUDA backend (kernels_stencil_{single,dual,aux}.cu post-split)
    wasm/                        # Emscripten bindings (ftd_wasm.cpp + bindings_{render_bridge,particle,atom}.cpp)
    web/                         # Three.js dashboard, 17 Playwright specs
    build_wsl/                   # WSL2 GPU build (RTX 5090, 30× speedup)
  scripts/
    constants.py                 # Canonical Python constants (single source of truth)
    proofs/                      # Mathematical proofs + computational checks
    tests/                       # 23 pytest files + 7-tier comprehensive framework
    verification/                # Formal derivation verification
    experiments/                 # Bell tests, CERN analysis, physics simulations
    visualization/               # Publication figure generation
    benchmarks/                  # Engine-vs-theory benchmark harness
  resources/data/                # Pure-data artifacts (constants.json, etc.)
  dissemination/                 # Manuscript, whitepaper, notebooks, interactive HTML
```

---

## Tests and Verification

| Suite | Count | Run with |
|---|---|---|
| C++ CTest (engine) | 250+ tests, 5 LABELS (unit / physics / golden / slow / gpu) | `cd engine/build && ctest --output-on-failure` |
| Python pytest (math) | 23 test files including `test_dimensional_map.py` (12 assertions) | `python -m pytest scripts/tests/` |
| Python verification | 40+ formal derivation checks | `python scripts/verification/<name>.py` |
| Python proofs | 57+ mathematical proofs with explicit error bounds | `python -m scripts.proofs.<name>` |
| Playwright (web) | 17 specs (scenario-parity, vacuum-seed, audit-regression, etc.) | `cd engine/web/tests && npx playwright test` |
| Master verification | 54-check single-script comprehensive pass | `python scripts/proofs/proof_master_verification.py` |
| Golden-tick gate | bit-exact regression hash `0xcd957b601d47868a` (CPU + WSL2 CUDA) | `ctest -L golden -C Release` |

Bit-exact CPU↔CUDA parity is verified at L = 16 and L = 32 across all 20 physics domains.

---

## Quick Start

### Verify the math

```bash
git clone https://github.com/williamsteinmetz/Foundational-Ternary-Dynamics.git
cd Foundational-Ternary-Dynamics
pip install numpy scipy sympy mpmath pytest
python scripts/proofs/proof_master_verification.py    # 54/54 checks
python scripts/proofs/build_dimensional_map.py        # build the dimensional map (idempotent)
python -m pytest scripts/tests/test_dimensional_map.py -v   # 12/12 PASS
```

### Build the engine

```bash
cmake -S engine -B engine/build -DCMAKE_BUILD_TYPE=Release
cmake --build engine/build --config Release
cd engine/build && ctest --output-on-failure -C Release
```

### GPU (WSL2 — required for CUDA campaigns)

```bash
wsl.exe -d Ubuntu-22.04 -- bash -c "cd /mnt/c/Users/cpaci/Desktop/ftd && \
    cmake --build engine/build_wsl --target test_render_bridge_golden -j 8 && \
    engine/build_wsl/test_render_bridge_golden"
# Expected: hash 0xcd957b601d47868a bit-exact match
```

### Launch the web dashboard

```bash
python engine/web/serve.py 8080
# Open http://localhost:8080
```

---

## The Simulation Engine

A C++17 lattice simulation implementing the FTD Lagrangian:

```
ℒ = ½|∂_t J|² − ½ c² Σ w |∇J|² − K_B √(1−v²) − g_c s ∇·J − g_c s (v·J) − λ_G (∇·J − ρ)²
```

Six terms, each derived from the action principle. Variational proof confirms δS = 0 reproduces all 10 update rules (60 checks, 0 failures).

### Tick cycle

```
phase_read → phase_write → gauss_project → phase_forces → phase_movement → tick++
```

Each phase is now a separate translation unit under `engine/src/render_bridge_phases/` (post-refactor v2.17.0). The golden-tick regression gate ([ADR-0012](docs/adr/0012-golden-tick-regression-gate.md)) ensures bit-exact byte-hash preservation across any future physics-touching extraction.

### Three physics scales

| Scale | Engine | Description |
|---|---|---|
| 0 | RenderBridge | Discrete lattice dynamics, flux propagation, Gauss constraint, Langevin thermostat |
| 1 | ParticleEngine | Continuous positions, Velocity Verlet, analytical EM + gravity |
| 2 | AtomEngine | Composite atoms, ionic / covalent / Van der Waals bonding |

### Scenarios

Post-audit catalog: ~95 Scale-0 scenarios across 6 prefix groups (`flux-` 20, `light-` 4, `quantum-` 8, `s0-seed-` 30+, `s0-field-` 8, `s0-vacuum-` 15) plus Scale-1/2/3 catalogs. Every scenario passes the JS↔C++ parity guard. See [engine/web/docs/SPEC_VACUUM_PARTICLE_SCENARIOS.md](engine/web/docs/SPEC_VACUUM_PARTICLE_SCENARIOS.md) for the canonical particle suite.

### Dashboard surfaces

- **Verify** — static evidence scoreboard with three epistemic tiers (hard / parametric / unpredicted). Spec: [SPEC_VERIFICATION_LAB.md](engine/web/docs/SPEC_VERIFICATION_LAB.md).
- **FAQ** — 16-entry framing of canonical hard problems; every FTD-side bullet carries an inline epistemic tag.
- **Scene** — render-controls panel (camera / lighting / post / environment, `localStorage`-backed).
- **KB** — searchable concept / notation / UI library.
- **Diagnostics / Charts / Lagrangian** — telemetry panels. Catalog: [TELEMETRY_CATALOG_SCALE0.md](engine/web/docs/TELEMETRY_CATALOG_SCALE0.md).

Math renders via KaTeX. Tests assert no raw `\(...\)` delimiters leak and no PASS / FAIL verdict badges appear on Verify rows.

---

## Interactive Engine Gallery

Launch with `python engine/web/serve.py 8080` and open [localhost:8080](http://localhost:8080).

### Scale 0 — Substrate Lattice

![Scale 0: Flux Dipole](engine/web/screenshots/scale0_dipole.png)
![Scale 0: Pair Production](engine/web/screenshots/scale0_pair_production.png)
![Scale 0: Lagrangian Inspector](engine/web/screenshots/scale0_lagrangian.png)

### Scale 1 — Particles

![Scale 1: Hydrogen Atom](engine/web/screenshots/scale1_hydrogen.png)

### Scale 2 — Atoms

![Scale 2: Water Pentamer](engine/web/screenshots/scale2_water_pentamer.png)
![Scale 2: NaCl Crystal](engine/web/screenshots/scale2_nacl_crystal.png)

### Scale 3 — Molecules

![Scale 3: Benzene](engine/web/screenshots/scale3_benzene.png)
![Scale 3: Caffeine](engine/web/screenshots/scale3_caffeine.png)

---

## Key Theory Documents

| Document | Content |
|---|---|
| [SPEC_ALGEBRAIC_SPINE](docs/theory/01_reference/SPEC_ALGEBRAIC_SPINE.md) | Eight canonical theorems, no physics interpretation |
| [SPEC_DIMENSIONAL_MAP](docs/theory/01_reference/SPEC_DIMENSIONAL_MAP.md) | 15-entry dimensionless ↔ dimensional bridge |
| [SPEC_FTD](docs/SPEC_FTD.md) | Full theoretical specification |
| [LEDGER](docs/theory/07_assessment/LEDGER.md) | Master claim ledger (FTD-0001..FTD-0111+) with reframe history |
| [CATALOG_PARAMETRIC_INSERTIONS](docs/theory/07_assessment/CATALOG_PARAMETRIC_INSERTIONS.md) | ~162 SM quantities tagged honestly |
| [Lattice Postulate](docs/theory/02_foundations/FOUND_AXIOM_ZERO.md) | State + position: the two lattice properties |
| [Master Quadratic](docs/theory/01_reference/MATH_MASTER_QUADRATIC.md) | Complete algebraic structure |
| [Gap Equation from ℤ](docs/theory/03_derivations/DERIV_MASTER_QUADRATIC_GAP_EQUATION.md) | Master quadratic from partition function |
| [Watson–G\* Identity](docs/theory/04_coupling/DERIV_WATSON_GSTAR_IDENTITY.md) | I₁ = G\*²/(2π) and lattice symmetry |
| [Higgs from State Transition](docs/theory/03_derivations/DERIV_HIGGS_FROM_MANIFESTATION.md) | λ = 3/23, m_H = 125.69 GeV [PARAMETRIC] |
| [Confinement](docs/theory/03_derivations/DERIV_CONFINEMENT_FROM_GAP_EQUATION.md) | Wilson loops, string tension, area law |
| [Three Generations](docs/theory/03_derivations/DERIV_THREE_GENERATIONS.md) | Cuboctahedron axis types = 3 |
| [Bell Cosine](docs/theory/03_derivations/DERIV_BELL_COSINE_FROM_GAUSS.md) | E(θ) = −cos(θ) from Gauss constraint |
| [Nonlinear Einstein](docs/theory/03_derivations/DERIV_EINSTEIN_NONLINEAR_FROM_LATTICE.md) | Full EFE via Deser bootstrap |
| [Moore Gauge Structure](docs/theory/03_derivations/DERIV_MOORE_GAUGE_STRUCTURE.md) | SC→U(1), FCC→SU(2), BCC→SU(3) |
| [Blind Derivation](docs/theory/02_foundations/FOUND_BLIND_DERIVATION_CHAIN.md) | 13 steps from "i exists" to α |
| [k from O_h irrep multiplicity](docs/theory/03_derivations/DERIV_K_FROM_OH_A1G_MULTIPLICITY.md) | k = 1/N_base = ¼ [DERIVED at linear level] |
| [Hidden Selections Audit](docs/theory/07_assessment/AUDIT_HIDDEN_SELECTIONS.md) | Honest accounting of all assumptions |
| [Master Quadratic Audit](docs/theory/07_assessment/AUDIT_MASTER_QUADRATIC.md) | April 19 downgrade record |
| [Look-Elsewhere Scan](docs/theory/07_assessment/AUDIT_LOOK_ELSEWHERE_RESULTS.md) | 60 000-polynomial rigidity check |

---

## What FTD Claims and What It Does Not

**Claims (current epistemic state, post April-2026 audits):**

The eight algebraic-spine theorems (G\* identity, master quadratic, CM uniqueness, |Aut(E)|² = 16, Watson identity, Phase G geometric Coulomb, Phase J ultralocality, harmonic invariant tower) are rigorous mathematical content with proof references. They stand independent of any physics interpretation.

The empirical match between the master quadratic's two roots and the dimensionless physical constants (1/α to 1.26 ppm, N_c to 0.80% — *the dual prediction*) is structurally rigid (60 000-polynomial scan confirms non-coincidental dual matching) but not a theorem. Tagged [STRONGLY MOTIVATED CONJECTURE]. The same pattern of mass ratios from framework integers (m_μ/m_e = 207, m_τ/m_e = 3477, m_p/m_e = 1836.47) is [DERIVED] at percent-to-permille precision, calibration-free.

The dimensional bridge is theorem-enforced as the irreducible minimum: two declared anchors (`a_phys ≡ ℓ_P`, `K_B = m_e`) are required; no-go theorems FTD-0059 and FTD-0096 prove neither is derivable from Axiom Zero. Once accepted, every dimensional FTD value is a dimensionless ratio multiplied by an anchor. [SPEC_DIMENSIONAL_MAP.md](docs/theory/01_reference/SPEC_DIMENSIONAL_MAP.md) is the canonical citation target.

The framework derives the gauge group U(1) × SU(2) × SU(3) from the orthogonal decomposition of J² on the Moore neighborhood (tag [SELECTION]); the lattice produces Coulomb 1/r² behavior (now [THEOREM 6]: identically the periodic lattice Poisson Green's function, R² = 1.0000 at L = 384, zero free parameters, zero fine-structure content); the Bell correlation S = 2√2 emerges from continuous-vector projections (tag [SELECTION]); the Einstein equations are recovered via Deser bootstrap from BI core + flux gradients.

**FTD-0107 emergent cluster** (April 2026): point injection of A = 10·K_GENESIS at the lattice center produces an L-invariant 25-voxel cluster at L ∈ {32, 64} with 5/5 seeds; tagged [PARTIAL]. **FTD-0110**: cluster size scales as N(A) ≈ ¼·(A/K_GENESIS)² with k = 1/N_base = ¼; coefficient ¼ is [DERIVED at linear level] from O_h representation theory (mult(A_{1g}) = 4 in the 27-block by character-table formula [THEOREM]); the cluster ↔ SM-mass identification at ~5% across 5 particles is [STRONGLY MOTIVATED CONJECTURE] for the full nonlinear regime.

**Does not claim:**

- α to 24-digit precision. CODATA 2022 measures α⁻¹ to 11 figures; digits beyond the 12th are post-hoc fit, not prediction. The "9.6 ppb residual" applies only to the [CONJECTURE]-tagged identification of x₊ with 1/α and the [SELECTION]-tagged tadpole correction.
- Derivation of the absolute electron mass. K_B = m_e is the [IMPOSED] mass anchor; m_e in MeV is bookkeeping, not derivation.
- Replacement of QED. The Phase G theorem reframed the engine's emergent Coulomb plateau as zero-fine-structure-content lattice geometry.
- Full Wilsonian EFT recovery. The pre-registered "1% match to continuum QED" target was not met; the result is reported without retrofit.
- A complete Standard Model derivation. ~129 of ~162 reported SM quantities are [PARAMETRIC] (SM formulas with FTD-supplied inputs). The catalog enumerates honestly.

**Open [OPEN]:**

- Rigorous proof of the linear→nonlinear bridge for FTD-0110 (whether engine genesis + Langevin + projection preserves linear-mode equipartition).
- Hadronic mass spectrum (scheme-dependent; confinement is derived but hadron masses are not).
- L = 128 follow-up to FTD-0107 (locks L-invariance further).
- Magic numbers from lattice geometry.
- Three derivation routes for the gauge coupling g_c are all [CLOSED-NEGATIVE]; g_c remains [PARAMETRIC].

**Status:** A rigorous algebraic core (eight theorems) plus a calibration-conditional bridge to physics. Every claim carries an honest epistemic tag traced to the ledger. The framework is **not** "99% complete" (a phrasing the previous README used and which has been retracted) — it is precisely as complete as the algebraic spine is rigorous and as honest as the catalog of parametric insertions makes the gap visible.

---

## License

This work is licensed under [CC BY-NC-SA 4.0](https://creativecommons.org/licenses/by-nc-sa/4.0/) — share and adapt with attribution, non-commercial use only. See [LICENSE](LICENSE).
