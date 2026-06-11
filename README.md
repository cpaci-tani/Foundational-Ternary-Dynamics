# Foundational Ternary Dynamics (FTD)

[![License: CC BY-NC-SA 4.0](https://img.shields.io/badge/License-CC%20BY--NC--SA%204.0-lightgrey.svg)](https://creativecommons.org/licenses/by-nc-sa/4.0/)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![Python 3.10+](https://img.shields.io/badge/python-3.10+-blue.svg)](https://www.python.org/downloads/)
[![Engine v2.18.0](https://img.shields.io/badge/engine-v2.18.0-orange.svg)](engine/SPEC_ENGINE.md)

Foundational Ternary Dynamics is a discrete-first research framework for asking a precise question:

> What does physics look like if discreteness is more fundamental than a continuous substrate, and continuity is the large-scale language that emerges from local finite dynamics?

FTD is not an argument that continuous science has failed. Standard continuum physics remains the most successful operational description we have. This project asks what can be rebuilt when the primitive ontology is instead a finite, local, deterministic substrate, and where that rebuilding honestly stops.

The current public posture is deliberately narrow:

- FTD has a rigorous algebraic core: a finite-invariant construction around `Z[i]`, `G* = Gamma(1/4)/Gamma(3/4)`, the master quadratic, BCC/Moore geometry, and several verified no-go results.
- The famous match `x_+ ~= 137.036` with `1/alpha` is real and structurally interesting, but it is still a `[STRONGLY MOTIVATED CONJECTURE]`, not a derivation of the electromagnetic coupling.
- The newest alpha-readout audits classify alpha as **dynamical, not structural**: the discrete ontology forces useful ingredients, but not the operator assembly that would make the match a physical coupling without an additional binding law.
- Negative results are part of the science here. Closed routes are preserved because mapping what discreteness cannot force is as important as deriving what it can.

For the live state, start with [Where We Left Off](docs/WHERE_WE_LEFT_OFF.md), then the [Construction Monograph](docs/theory/01_reference/MONOGRAPH_FTD_CONSTRUCTION.md), [Doctrine Ledger](docs/theory/01_reference/SPEC_DOCTRINE_LEDGER.md), [Ontic Truth Tracker](docs/theory/07_assessment/core_ledgers/TRACKER_ONTIC_TRUTH.md), and [Master Claim Ledger](docs/theory/07_assessment/core_ledgers/LEDGER.md).

---

## The Model

FTD starts from five substrate postulates:

| Postulate | Meaning |
|---|---|
| Discrete space | A three-dimensional cubic lattice with undefined boundary, not a completed-infinity object. |
| Discrete time | Evolution occurs in ticks, not in primitive continuous time. |
| Ternary states | Each voxel resolves to `s in {-1, 0, +1}`. |
| Local causality | Updates are local to the 26-neighbor Moore neighborhood. |
| Determinism | The next state is fixed by the current state and rules. |

The model has a two-layer ontology:

| Layer | Role |
|---|---|
| Flux field `J in R^3` | Dispositional content: what the site is poised to do. |
| State field `s in {-1,0,+1}` | Manifestation: what the substrate has resolved into. |

The important philosophical move is not "continuous math is wrong." It is that continuous descriptions enter as effective readouts, coarse-grained fields, finite-block approximations, and long-wavelength limits. A number, field, coupling, or particle is not physics-facing until an operational readout says what measurement accesses it.

Canonical statement: [SPEC_MATH_FIRST_ONTOLOGY.md](docs/theory/01_reference/SPEC_MATH_FIRST_ONTOLOGY.md).

---

## Current Scientific Picture

### 1. What is solid

The strongest layer is mathematical and finite-invariant. The core references are [SPEC_ALGEBRAIC_SPINE.md](docs/theory/01_reference/SPEC_ALGEBRAIC_SPINE.md) and [TRACKER_ONTIC_TRUTH.md](docs/theory/07_assessment/core_ledgers/TRACKER_ONTIC_TRUTH.md).

Examples of bedrock results include:

- the bridge constant `G* = Gamma(1/4)/Gamma(3/4)` and its equivalent constructions;
- the finite-`N` attractor `G*_N -> G*`, giving a finitary route compatible with undefined-boundary ontology;
- the master quadratic `x^2 - 16G*^2 x + 16G*^3 = 0`;
- the harmonic invariant tower;
- the BCC complex-structure theorem and related `Z[i]` no-go;
- the Watson identity linking the BCC lattice Green function to `G*`;
- the Moore-neighborhood decomposition that supplies much of the framework's structural vocabulary.

These claims are tied to proof documents and verification scripts. When a README sentence conflicts with the tracker or ledger, the tracker and ledger win.

### 2. What is suggestive

The larger root of the master quadratic lands near the inverse fine-structure constant:

```text
x_+ = 137.036171458...
alpha^-1(CODATA 2022) = 137.035999177...
```

That agreement is the reason the project exists. It is also exactly where the project's discipline matters most. The identification `x_+ = 1/alpha` remains `[STRONGLY MOTIVATED CONJECTURE]` because the required electromagnetic readout is not derived from the substrate.

The current alpha boundary is recorded in:

- [AUDIT_ALPHA_OPERATOR_FORCING_ROUTE_INVARIANCE.md](docs/theory/07_assessment/audits/AUDIT_ALPHA_OPERATOR_FORCING_ROUTE_INVARIANCE.md)
- [AUDIT_RSI_LEG3_CONDITIONAL_THEOREM.md](docs/theory/07_assessment/audits/AUDIT_RSI_LEG3_CONDITIONAL_THEOREM.md)
- [SPEC_ALPHA_READOUT_CONTRACT.md](docs/theory/01_reference/SPEC_ALPHA_READOUT_CONTRACT.md)

Short version: FTD can force the menu of ingredients; it has not forced the dish. A future closure would need either a new substrate-native binding law `W` or a successful engine-native readout that does not import the target value.

### 3. What is emergent

Continuity, relativity-style geometry, field equations, quantum statistics, EFT flow, and particle language are treated as emergent or readout-level structures. The corpus explores these through:

- lattice Green functions and continuum approximations;
- effective field theory and blocking maps;
- Moore-neighborhood geometry;
- frame-relative measurement and projection layers;
- engine campaigns that test whether proposed readouts survive finite computation.

This is the scientific stance of the repository: continuous physics is not discarded; it is treated as the scale-appropriate description that a discrete substrate must recover, approximate, or fail to recover under explicit tests.

---

## Epistemic Tags

FTD uses explicit claim tags. These are not decoration; they are the safety system.

| Tag | Meaning |
|---|---|
| `[AXIOM]` | Structural postulate or modeling choice. |
| `[THEOREM]` | Rigorously proven from axioms or named classical results. |
| `[DERIVED]` | Explicit chain exists, but may carry nontrivial assumptions. |
| `[NUMERICAL FACT]` | Verified over a stated finite domain. |
| `[SELECTION]` | Argued by consistency or naturalness, not uniquely forced. |
| `[STRONGLY MOTIVATED CONJECTURE]` | Strong evidence, but no derivation chain. |
| `[PARAMETRIC]` | Standard physics formula filled with FTD numbers. |
| `[IMPOSED]` | Calibration or input choice. |
| `[CLOSED NEGATIVE]` | Tested route failed; preserved for provenance. |
| `[OPEN]` | Unresolved research obligation. |
| `[SYNTHESIS]` | Consolidates existing claims without promoting them. |

Three rules govern all scientific work in this repo:

1. Do not run numerical searches for near-misses or coincidences.
2. Do not call substitutions into standard formulas "derivations."
3. Do not promote a claim unless the proof or readout chain actually exists.

See [AUDIT_EPISTEMIC_AUDIT.md](docs/theory/07_assessment/AUDIT_EPISTEMIC_AUDIT.md) and [CATALOG_PARAMETRIC_INSERTIONS.md](docs/theory/07_assessment/CATALOG_PARAMETRIC_INSERTIONS.md).

---

## Theory Corpus Map

The theory directory is the scientific corpus of the project: **586 documents** (11.8 MB) across 10 categories plus **118 archived** documents preserving provenance. Use [META_INDEX.md](docs/theory/META_INDEX.md) as the catalog.

| Directory | Role |
|---|---|
| [01_reference](docs/theory/01_reference/INDEX_01_REFERENCE.md) | Canonical references, status maps, algebraic spine, math-first ontology, readout contracts. |
| [02_foundations](docs/theory/02_foundations/INDEX_02_FOUNDATIONS.md) | Discrete ontology, `i`, ternary states, dimensional emergence, structural/dynamical boundary. |
| [03_derivations](docs/theory/03_derivations/INDEX_03_DERIVATIONS.md) | Working physics derivations: EM, QFT, gravity, QM statistics, Standard Model sectors, cluster mass bridge. |
| [04_coupling](docs/theory/04_coupling/INDEX_04_COUPLING.md) | Couplings and precision claims: alpha, lattice one-loop schemes, QCD scale, cosmological constant. |
| [05_particles](docs/theory/05_particles/INDEX_05_PARTICLES.md) | Particle-spectrum applications, masses, color binding, material emergence, SM-observable rollups. |
| [06_reference_frames_and_measurement](docs/theory/06_reference_frames_and_measurement/INDEX_06_CONSCIOUSNESS.md) | Measurement, frame-relative projection, von Neumann chain, Wigner's friend, collapse mechanisms. |
| [07_assessment](docs/theory/07_assessment/INDEX_07_ASSESSMENT.md) | Ledgers, audits, red teams, look-elsewhere controls, parametric-insertion catalog. |
| [08_structural](docs/theory/08_structural/INDEX_08_STRUCTURAL.md) | Moore-neighborhood geometry, cuboctahedral integers, BCC structure, coefficient 16, structural audits. |
| [09_mathematical](docs/theory/09_mathematical/INDEX_09_MATHEMATICAL.md) | Number theory, CM curves, `G*`, L-values, Clifford/bivector algebra, FQCR observer tests. |
| [10_eft_program](docs/theory/10_eft_program/INDEX_FTD_NATIVE_EFT.md) | Native EFT recovery program, pre-registrations, blocking maps, open readout work, archived failed routes. |

The archive directories are not trash. They are scientific provenance: retractions, closed-negative routes, completed campaigns, and superseded scaffolding.

---

## Suggested Reading Paths

| Reader | Start here |
|---|---|
| First-time reviewer | [MONOGRAPH_FTD_CONSTRUCTION.md](docs/theory/01_reference/MONOGRAPH_FTD_CONSTRUCTION.md) |
| Skeptical mathematician | [TRACKER_ONTIC_TRUTH.md](docs/theory/07_assessment/core_ledgers/TRACKER_ONTIC_TRUTH.md), then [SPEC_ALGEBRAIC_SPINE.md](docs/theory/01_reference/SPEC_ALGEBRAIC_SPINE.md) |
| Physicist | [SPEC_DOCTRINE_LEDGER.md](docs/theory/01_reference/SPEC_DOCTRINE_LEDGER.md), then [SPEC_PHYSICS_BRIDGE.md](docs/theory/01_reference/SPEC_PHYSICS_BRIDGE.md) |
| Alpha-readout audit | [SPEC_ALPHA_READOUT_CONTRACT.md](docs/theory/01_reference/SPEC_ALPHA_READOUT_CONTRACT.md), then the FTD-0242/0243 audits |
| Engine implementer | [engine/SPEC_ENGINE.md](engine/SPEC_ENGINE.md), [MAP_ENGINE_ARCHITECTURE.md](docs/theory/01_reference/MAP_ENGINE_ARCHITECTURE.md), [MAP_LAGRANGIAN_TO_ENGINE.md](docs/theory/01_reference/MAP_LAGRANGIAN_TO_ENGINE.md) |
| Open-problem hunter | [SPEC_OPEN_MATH_BY_SECTOR.md](docs/theory/01_reference/SPEC_OPEN_MATH_BY_SECTOR.md), [TRACKER_OPEN_ITEMS.md](docs/theory/07_assessment/core_ledgers/TRACKER_OPEN_ITEMS.md) |

---

## Repository Layout

```text
docs/
  SPEC_FTD.md                 readable framework spec; tag status defers to ledgers
  WHERE_WE_LEFT_OFF.md        latest live state
  theory/                     main scientific corpus
engine/                       C++17 simulation engine, CUDA path, web dashboard
scripts/                      proof scripts, verification, tests, visualization
evaluation/                   assessment and certification material
dissemination/                papers, whitepaper, book/manuscript, notebooks
```

Important note: [docs/SPEC_FTD.md](docs/SPEC_FTD.md) is a readable framework specification, but its body contains historical tag drift. For claim status, use [LEDGER.md](docs/theory/07_assessment/core_ledgers/LEDGER.md) and [TRACKER_ONTIC_TRUTH.md](docs/theory/07_assessment/core_ledgers/TRACKER_ONTIC_TRUTH.md).

---

## Complete Corpus At A Glance

| Component | Count | Detail |
|-----------|-------|--------|
| **Theory documents** | 586 files (11.8 MB) | 10 categories + archives |
| **Archived provenance** | 118 documents | Retractions, closed-negatives, campaign-completes |
| **Ledger entries** | 254 unique IDs | FTD-0001 through FTD-0267, never-delete policy |
| **C++ engine** | 59 headers, 22 sources | v2.18.0, 27K LOC, 10-phase tick cycle |
| **Engine tests** | 298 source files | 211 active CTest targets, all passing |
| **Python scripts** | 520 files | 139K LOC across 9 subdirectories |
| **Python tests** | 255 passing | pytest suite, 4 skipped |
| **Interactive demos** | 31 HTML files | Standalone browser simulations |
| **LaTeX papers** | 70 .tex files | Across docs/papers/ and dissemination/papers/ |
| **Web dashboard** | 868 JS files | Three.js + WASM, 5 CSS themes, 180K LOC |
| **Total commits** | 1,119 | Changelog: 3,574 lines |
| **Total LOC** | ~346K | C++ 27K + Web 180K + Python 139K |

### Verification Status (June 10, 2026)

| Suite | Result |
|-------|--------|
| Master verification (54 checks) | ✅ All pass |
| Physics battery (50 tests, 3 tiers) | ✅ All pass |
| Complete SM computation (46 observables) | ✅ All computed |
| Python pytest (255 tests) | ✅ All pass |
| C++ CTest (211 targets) | ✅ All pass |

---

## Build And Verify

### C++ Engine

```bash
cmake -S engine -B engine/build -DCMAKE_BUILD_TYPE=Release
cmake --build engine/build --config Release --parallel 24

cd engine/build
ctest -j 24 --output-on-failure -C Release
```

### Python Proofs And Tests

```bash
python -m pytest scripts/tests/
python scripts/proofs/proof_master_verification.py
```

### Web Dashboard

```bash
python -m http.server 8080 -d engine/web
```

Then open `http://localhost:8080`.

Or just go to: https://williamsteinmetz.github.io/Foundational-Ternary-Dynamics/

### GPU Campaigns

CUDA campaigns are expected to run through WSL2 Ubuntu-22.04, not Windows-native CUDA. The current hardware profile and command conventions are recorded in [AGENTS.md](AGENTS.md) and [CLAUDE.md](CLAUDE.md).

---

## What This Project Is Not

FTD is not a completed replacement for QED, the Standard Model, or general relativity.

It does not claim that the fine-structure constant is derived from the five substrate postulates.

It does not treat numerical agreement as proof.

It does not treat failed derivation routes as embarrassing debris. They are part of the map.

The least-wrong self-assessment from the corpus is this: FTD is a discrete-first philosophy-of-mathematics and computational-physics program with a rigorous algebraic core, suggestive physics bridges, a serious simulation engine, and unusually explicit boundaries around what has not been derived.

---

## License And Citation

This repository is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License (**CC BY-NC-SA 4.0**).

```bibtex
@misc{steinmetz2026ftd,
  author = {William J. Steinmetz III},
  title  = {Foundational Ternary Dynamics},
  year   = {2026},
  note   = {Version 5.47 corpus checkpoint, engine 2.18.0},
  url    = {https://github.com/williamsteinmetz/Foundational-Ternary-Dynamics}
}
```
