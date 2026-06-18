# Foundational Ternary Dynamics

[![License: CC BY-NC-SA 4.0](https://img.shields.io/badge/License-CC%20BY--NC--SA%204.0-lightgrey.svg)](https://creativecommons.org/licenses/by-nc-sa/4.0/)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![Python 3.10+](https://img.shields.io/badge/python-3.10+-blue.svg)](https://www.python.org/downloads/)
[![Engine v2.18.0](https://img.shields.io/badge/engine-v2.18.0-orange.svg)](engine/SPEC_ENGINE.md)

Foundational Ternary Dynamics (FTD) is a research repository for asking one question as rigorously as possible:

> What can be derived from a finite, local, deterministic, ternary substrate, and where does that derivation fail?

The project combines a theory corpus, verification scripts, a C++/CUDA simulation engine, and a browser dashboard. It does not claim to replace the Standard Model or general relativity. Its discipline is to separate theorem-grade results, derived consequences, imposed choices, parametric insertions, open problems, and closed-negative routes.

For the latest live state, start with [`docs/WHERE_WE_LEFT_OFF.md`](docs/WHERE_WE_LEFT_OFF.md).

## Scientific Posture

FTD starts from five substrate commitments:

| Commitment | Meaning |
|---|---|
| Discrete space | A cubic lattice with undefined boundary, not a completed-infinity object. |
| Discrete time | Evolution occurs in ticks. |
| Ternary state | Each voxel resolves to `s in {-1, 0, +1}`. |
| Local causality | Updates are local to the 26-neighbor Moore neighborhood. |
| Determinism | The next state is fixed by the current state and rules. |

The model uses two coupled fields:

| Layer | Role |
|---|---|
| Flux field `J in R^3` | Dispositional content and wave-sector carrier. |
| State field `s in {-1, 0, +1}` | Manifested ternary state. |

The strongest current layer is mathematical: the lemniscatic constant `G* = Gamma(1/4) / Gamma(3/4)`, the master quadratic, BCC/Moore geometry, and related finite-invariant structure. The physical identification `x_+ = 1/alpha` remains `[STRONGLY MOTIVATED CONJECTURE]`, not a derivation. The current alpha-readout boundary is documented in [`AUDIT_ALPHA_OPERATOR_FORCING_ROUTE_INVARIANCE.md`](docs/theory/07_assessment/audits/AUDIT_ALPHA_OPERATOR_FORCING_ROUTE_INVARIANCE.md) and [`SPEC_ALPHA_READOUT_CONTRACT.md`](docs/theory/01_reference/SPEC_ALPHA_READOUT_CONTRACT.md).

## Start Here

| Reader | First stop |
|---|---|
| New reviewer | [`docs/WHERE_WE_LEFT_OFF.md`](docs/WHERE_WE_LEFT_OFF.md), then [`MONOGRAPH_FTD_CONSTRUCTION.md`](docs/theory/01_reference/MONOGRAPH_FTD_CONSTRUCTION.md) |
| Skeptical mathematician | [`TRACKER_ONTIC_TRUTH.md`](docs/theory/07_assessment/core_ledgers/TRACKER_ONTIC_TRUTH.md), then [`SPEC_ALGEBRAIC_SPINE.md`](docs/theory/01_reference/SPEC_ALGEBRAIC_SPINE.md) |
| Physicist | [`SPEC_DOCTRINE_LEDGER.md`](docs/theory/01_reference/SPEC_DOCTRINE_LEDGER.md), then [`SPEC_PHYSICS_BRIDGE.md`](docs/theory/01_reference/SPEC_PHYSICS_BRIDGE.md) |
| Engine contributor | [`engine/SPEC_ENGINE.md`](engine/SPEC_ENGINE.md), then [`META_PROJECT_ATLAS.md`](META_PROJECT_ATLAS.md) |
| Open-problem hunter | [`SPEC_OPEN_MATH_BY_SECTOR.md`](docs/theory/01_reference/SPEC_OPEN_MATH_BY_SECTOR.md), then [`TRACKER_OPEN_ITEMS.md`](docs/theory/07_assessment/core_ledgers/TRACKER_OPEN_ITEMS.md) |

Status authority is intentionally centralized. If a downstream document disagrees with the canonical ledgers, the ledgers win:

- Claim status: [`LEDGER.md`](docs/theory/07_assessment/core_ledgers/LEDGER.md)
- Bedrock truth tiers: [`TRACKER_ONTIC_TRUTH.md`](docs/theory/07_assessment/core_ledgers/TRACKER_ONTIC_TRUTH.md)
- Theorem statements: [`SPEC_ALGEBRAIC_SPINE.md`](docs/theory/01_reference/SPEC_ALGEBRAIC_SPINE.md)
- Parametric insertions: [`CATALOG_PARAMETRIC_INSERTIONS.md`](docs/theory/07_assessment/CATALOG_PARAMETRIC_INSERTIONS.md)

## Repository Map

```text
docs/
  SPEC_FTD.md                 readable framework spec; claim status defers to ledgers
  WHERE_WE_LEFT_OFF.md        latest session and scientific state
  theory/                     active theory corpus and provenance archives
engine/                       C++17 simulation engine, CUDA path, WASM bridge, web dashboard
scripts/                      proof scripts, verification, tests, experiments, visualization
evaluation/                   project health, weaknesses, and assessment material
dissemination/                papers, whitepaper, manuscript/book assets, notebooks, demos
```

Use [`docs/theory/META_INDEX.md`](docs/theory/META_INDEX.md) for the curated theory catalog and [`META_DOCUMENTATION_MAP.md`](META_DOCUMENTATION_MAP.md) for broader navigation.

Archives are provenance, not trash. Closed-negative routes, retractions, superseded scaffolding, and completed campaigns are preserved so failed derivation paths do not quietly reappear.

## Build And Verify

### Python

```bash
python -m pytest scripts/tests/
python scripts/proofs/proof_master_verification.py
```

### C++ Engine

```bash
cmake -S engine -B engine/build -DCMAKE_BUILD_TYPE=Release
cmake --build engine/build --config Release --parallel 24
ctest --test-dir engine/build -j 24 --output-on-failure -C Release
```

### Web Dashboard

```bash
python engine/web/serve.py 8080
```

Then open `http://localhost:8080`, or visit the published dashboard at [williamsteinmetz.github.io/Foundational-Ternary-Dynamics](https://williamsteinmetz.github.io/Foundational-Ternary-Dynamics/).

GPU measurement campaigns should run through WSL2 Ubuntu-22.04. The exact local convention is recorded in [`AGENTS.md`](AGENTS.md) and [`CLAUDE.md`](CLAUDE.md).

## Contribution Rules

FTD uses explicit epistemic tags as a safety system:

| Tag | Meaning |
|---|---|
| `[AXIOM]` | Structural postulate or modeling choice. |
| `[THEOREM]` | Rigorously proven from axioms or named mathematical results. |
| `[DERIVED]` | Explicit derivation chain exists, possibly with stated assumptions. |
| `[NUMERICAL FACT]` | Verified over a stated finite domain. |
| `[SELECTION]` | Chosen or argued by consistency, not uniquely forced. |
| `[STRONGLY MOTIVATED CONJECTURE]` | Substantial evidence, but no derivation chain. |
| `[PARAMETRIC]` | Standard physics formula filled with FTD constants. |
| `[IMPOSED]` | Calibration or model input. |
| `[CLOSED NEGATIVE]` | Tested route failed and is preserved for provenance. |
| `[OPEN]` | Unresolved research obligation. |
| `[SYNTHESIS]` | Consolidates existing claims without promoting them. |

Three rules govern all scientific work:

1. Do not run numerical searches for near-misses or coincidences.
2. Do not call substitutions into standard formulas derivations.
3. Do not promote a claim unless the canonical proof or readout chain actually exists.

See [`CONTRIBUTING.md`](CONTRIBUTING.md) and [`docs/audits/PLAN_PROJECT_CLEANUP_2026-06-17.md`](docs/audits/PLAN_PROJECT_CLEANUP_2026-06-17.md) for the current cleanup and contribution plan.

## Data And Reproducibility

The repository tracks source, theory, proof scripts, and verification code. Regenerable campaign outputs are usually local-only; pre-registered measurements record the git tag, runner, and analysis script hashes needed to reproduce them. See [`REF_PREREGISTER_MANIFEST.md`](docs/theory/10_eft_program/REF_PREREGISTER_MANIFEST.md).

## License And Citation

This repository is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License (**CC BY-NC-SA 4.0**).

```bibtex
@misc{steinmetz2026ftd,
  author = {William J. Steinmetz III},
  title  = {Foundational Ternary Dynamics},
  year   = {2026},
  note   = {FTD research corpus and simulation engine},
  url    = {https://github.com/williamsteinmetz/Foundational-Ternary-Dynamics}
}
```
