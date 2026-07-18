# Foundational Ternary Dynamics

[![License: CC BY-NC-SA 4.0](https://img.shields.io/badge/License-CC%20BY--NC--SA%204.0-lightgrey.svg)](https://creativecommons.org/licenses/by-nc-sa/4.0/)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![Python 3.10+](https://img.shields.io/badge/python-3.10+-blue.svg)](https://www.python.org/downloads/)
[![Engine v2.18.0](https://img.shields.io/badge/engine-v2.18.0-orange.svg)](engine/SPEC_ENGINE.md)

**Foundational Ternary Dynamics (FTD)** is a philosophy-of-mathematics project with a rigorous algebraic core and deliberately-bounded physics connections. It asks one question, as honestly as it can be asked:

> Starting from a finite, local, deterministic, ternary substrate, what can be *built* — and exactly where, and why, does the building stop?

FTD is **not** an attempt to replace the Standard Model or general relativity. It is ordered **Ontology > Logic > Math > Physics**: the substrate fixes a small set of commitments, mathematics is built forward from them, and physics enters as a *constraint*, not the sole arbiter. The discipline that makes the project worth reading is that it labels every claim by exactly how far it has actually been carried — theorem, derivation, selection, conjecture, parametric insertion, imposed input, open problem, or closed-negative route — and refuses to let rhetoric promote one into another.

The repository combines a theory corpus, verification and proof scripts, a C++/CUDA simulation engine, and a browser dashboard. For the latest working state, start with [`docs/WHERE_WE_LEFT_OFF.md`](docs/WHERE_WE_LEFT_OFF.md).

## The organizing principle: context before content

One principle runs through the whole framework ([`FOUND_TYPE_PRIORITY_PRINCIPLE.md`](docs/theory/02_foundations/FOUND_TYPE_PRIORITY_PRINCIPLE.md)): a *context* — a **type** — is prior to, and the precondition for, the value of any *content* — a **token**. You cannot derive a context from its content. The elementary picture is `z = |z|·e^{iθ}`: the magnitude `|z|` (content) does not locate the number until the orientation `θ` (context) is supplied.

So FTD proceeds in one order, and the goal is stated in that order:

> **Set the smallest honest set of types from which a discrete ontology can speak; build the mathematics and physics forward, sector by sector, until every physical structure is either forced content or a rigorously marked and priced import; drive every priced line to retirement, to a theorem-grade no-go, or to a sharper falsifier — never leaving a line merely booked; and where a line provably resists retirement, search deliberately for the next honest type whose declared adoption converts it into content at a minimal, falsifiable price.**

Marking the boundary — which types are *not* native — is as much a deliverable as any derivation, and every priced line is a standing work item, not a resting state (amendment of record 2026-07-12). (Type-priority is an adopted organizing commitment, not a theorem; it is offered for outside critique, not asserted as proven.)

## What is load-bearing, and what is not

**The algebraic core is the load-bearing part, and it stands as mathematics independent of any physics reading.** The lemniscatic constant `G* = Γ(1/4)/Γ(3/4) ≈ 2.95868`, the master quadratic `x² − 16G*²x + 16G*³`, the Watson identity via Chowla–Selberg, the CM-curve uniqueness scan, and `D = 3` from `|Aut(E)|² = 2^D(D−1)!` are theorem-grade results (conditional, where noted, on Chudnovsky 1976's algebraic independence of `π` and `Γ(1/4)`). Status authority for these lives in [`SPEC_ALGEBRAIC_SPINE.md`](docs/theory/01_reference/SPEC_ALGEBRAIC_SPINE.md) and [`TRACKER_ONTIC_TRUTH.md`](docs/theory/07_assessment/core_ledgers/TRACKER_ONTIC_TRUTH.md).

**The physics connections are suggestive, not derived, and are labeled as such.** The master quadratic's larger root matches `1/α` to 1.26 ppm, but the identification `x₊ = 1/α` is a `[STRONGLY MOTIVATED CONJECTURE]`, not a derivation — and the framework is explicit about why. The value of `α` is an *imported* type: a chosen orientation a deterministic substrate cannot fix for itself. That is the same status physics itself gives `α` — no theory derives it; it is measured and supplied. Mass formulas, gauge ratios, and similar results are `[PARAMETRIC]` or `[STRONGLY MOTIVATED CONJECTURE]` — standard physics filled with framework numbers, flagged plainly rather than dressed as derivations.

**The boundary is stated canonically** in [`FOUND_MODULUS_ARGUMENT_FRONTIER.md`](docs/theory/02_foundations/FOUND_MODULUS_ARGUMENT_FRONTIER.md): a finite, discrete, deterministic substrate natively sets the *forced / modulus* structure and must import the *chosen / argument* structure. `α` is the worked example of that boundary, not the project's goal.

`G* / ℚ(G*)` functions as a **lever, not a discovery** — an acknowledged but underexploited mathematical structure (the lemniscatic CM point, Watson's body-centred-cubic integral, Gauss's AGM) that an ontology-first construction forces into a central role.

## The substrate

FTD begins from five commitments — the types the dynamics presuppose:

| Commitment | Meaning |
|---|---|
| Discrete space | A cubic lattice with undefined boundary, not a completed-infinity object. |
| Discrete time | Evolution proceeds in ticks. |
| Ternary state | Each voxel resolves to `s ∈ {−1, 0, +1}`. |
| Local causality | Updates are local to the 26-neighbour Moore neighbourhood. |
| Determinism | The next state is fixed by the current state and the rules. |

On that lattice, two coupled fields:

| Layer | Role |
|---|---|
| Flux field `J ∈ ℝ³` | Dispositional content; the wave-sector carrier. |
| State field `s ∈ {−1, 0, +1}` | The manifested ternary state. |

Above the five postulates sit the **Framework Commitments** (FC-0…FC-W) — the imported types the substrate cannot set for itself (the `ℤ[i]` reading, the declined quantum measurement-map, the native arrow, scale-ratio covariance, and the chosen orientation behind `α`). Each is a declared `[AXIOM]`-class commitment with a stated falsification criterion. The canonical framework statement is [`SPEC_FTD_FRAMEWORK_V1.md`](docs/theory/01_reference/SPEC_FTD_FRAMEWORK_V1.md).

## Start here

| Reader | First stop |
|---|---|
| New reviewer | [`WHERE_WE_LEFT_OFF.md`](docs/WHERE_WE_LEFT_OFF.md), then [`MONOGRAPH_FTD_CONSTRUCTION.md`](docs/theory/01_reference/MONOGRAPH_FTD_CONSTRUCTION.md) |
| Skeptical mathematician | [`TRACKER_ONTIC_TRUTH.md`](docs/theory/07_assessment/core_ledgers/TRACKER_ONTIC_TRUTH.md), then [`SPEC_ALGEBRAIC_SPINE.md`](docs/theory/01_reference/SPEC_ALGEBRAIC_SPINE.md) |
| Foundations / philosophy of math | [`FOUND_TYPE_PRIORITY_PRINCIPLE.md`](docs/theory/02_foundations/FOUND_TYPE_PRIORITY_PRINCIPLE.md), then [`FOUND_MODULUS_ARGUMENT_FRONTIER.md`](docs/theory/02_foundations/FOUND_MODULUS_ARGUMENT_FRONTIER.md) |
| Physicist | [`SPEC_DOCTRINE_LEDGER.md`](docs/theory/01_reference/SPEC_DOCTRINE_LEDGER.md), then [`SPEC_PHYSICS_BRIDGE.md`](docs/theory/01_reference/SPEC_PHYSICS_BRIDGE.md) |
| Engine contributor | [`engine/SPEC_ENGINE.md`](engine/SPEC_ENGINE.md), then [`META_PROJECT_ATLAS.md`](META_PROJECT_ATLAS.md) |
| Open-problem hunter | [`SPEC_OPEN_MATH_BY_SECTOR.md`](docs/theory/01_reference/SPEC_OPEN_MATH_BY_SECTOR.md), then [`AUDIT_BOUNDARY_MAP.md`](docs/theory/07_assessment/AUDIT_BOUNDARY_MAP.md) |

Status authority is intentionally centralized. If a downstream document disagrees with the canonical ledgers, the ledgers win:

- Claim status: [`LEDGER.md`](docs/theory/07_assessment/core_ledgers/LEDGER.md)
- Bedrock truth tiers: [`TRACKER_ONTIC_TRUTH.md`](docs/theory/07_assessment/core_ledgers/TRACKER_ONTIC_TRUTH.md)
- Theorem statements: [`SPEC_ALGEBRAIC_SPINE.md`](docs/theory/01_reference/SPEC_ALGEBRAIC_SPINE.md)
- The boundary (what is not derivable): [`AUDIT_BOUNDARY_MAP.md`](docs/theory/07_assessment/AUDIT_BOUNDARY_MAP.md)
- Parametric insertions: [`CATALOG_PARAMETRIC_INSERTIONS.md`](docs/theory/07_assessment/CATALOG_PARAMETRIC_INSERTIONS.md)

## Repository map

```text
docs/
  SPEC_FTD.md                 readable framework spec; claim status defers to the ledgers
  WHERE_WE_LEFT_OFF.md        latest session and scientific state
  theory/                     active theory corpus and provenance archives
engine/                       C++17 simulation engine, CUDA path, WASM bridge, web dashboard
scripts/                      proof scripts, verification, tests, experiments, visualization
evaluation/                   project-health, weaknesses, and assessment material
dissemination/                papers, whitepaper, manuscript/book assets, notebooks, demos
```

Use [`docs/theory/META_INDEX.md`](docs/theory/META_INDEX.md) for the curated theory catalog and [`META_DOCUMENTATION_MAP.md`](META_DOCUMENTATION_MAP.md) for broader navigation. Archives are provenance, not trash: closed-negative routes, retractions, and superseded scaffolding are preserved so failed paths do not quietly reappear.

## Build and verify

### Python

```bash
python -m pytest scripts/tests/
python scripts/proofs/proof_master_verification.py
```

### C++ engine

```bash
# Windows native (pins MSVC 14.44 -- VS 18's default toolset crashes CUDA 13's cudafe++)
engine\build_native.bat
ctest --test-dir engine/build -j 24 --output-on-failure -C Release
```

### Web dashboard

```bash
python engine/web/serve.py 8080
```

Then open `http://localhost:8080`, or visit the published dashboard at [williamsteinmetz.github.io/Foundational-Ternary-Dynamics](https://williamsteinmetz.github.io/Foundational-Ternary-Dynamics/). GPU measurement campaigns run through WSL2 Ubuntu-22.04; the exact local convention is recorded in [`AGENTS.md`](AGENTS.md) and [`CLAUDE.md`](CLAUDE.md).

## Epistemic discipline

FTD uses explicit tags as a safety system. The tag *records* how far a claim has been carried; it does not, by itself, resolve the underlying question.

| Tag | Meaning |
|---|---|
| `[AXIOM]` | A set type — a structural postulate or modeling commitment. |
| `[THEOREM]` | Rigorously proven from axioms or named mathematical results. |
| `[DERIVED]` | An explicit derivation chain exists, possibly with stated assumptions. |
| `[NUMERICAL FACT]` | Verified over a stated finite domain. |
| `[SELECTION]` | Chosen or argued by consistency, not uniquely forced. |
| `[STRONGLY MOTIVATED CONJECTURE]` | Substantial evidence, but no derivation chain. |
| `[PARAMETRIC]` | A standard physics formula filled with FTD constants. |
| `[IMPOSED]` | A calibration or model input. |
| `[CLOSED NEGATIVE]` | A tested route that failed, preserved for provenance. |
| `[OPEN]` | An unresolved research obligation. |
| `[SYNTHESIS]` | Consolidates existing claims without promoting them. |

Three rules govern all scientific work:

1. Do not run numerical searches for near-misses or coincidences.
2. Do not call substitutions into standard formulas derivations.
3. Do not promote a claim unless the canonical proof or readout chain actually exists.

A worked instance of the discipline: pre-registered measurements hash-lock their instrument (a SHA256 and a git tag) *before* the run, so a frozen gate decides the verdict — no clean story can be cherry-picked from a compromised run. See [`CONTRIBUTING.md`](CONTRIBUTING.md).

## Data and reproducibility

The repository tracks source, theory, proof scripts, and verification code. Regenerable campaign outputs are usually local-only; pre-registered measurements record the git tag, runner, and analysis-script hashes needed to reproduce them. See [`REF_PREREGISTER_MANIFEST.md`](docs/theory/10_eft_program/REF_PREREGISTER_MANIFEST.md).

## License and citation

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
