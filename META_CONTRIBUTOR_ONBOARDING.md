# FTD Contributor Onboarding

This guide is the public, contributor-first entry point for the FTD repository. It is meant to help a new engineer or researcher understand what the project is, which documents are authoritative, where the executable surfaces live, and where the repo currently shows drift between theory, implementation, and assessment.

For historical documentation cleanup provenance, see [docs/audits/AUDIT_DOCUMENT_CLEANUP_LEDGER.md](docs/audits/AUDIT_DOCUMENT_CLEANUP_LEDGER.md).

Use this document with three expectations in mind:

1. `docs/SPEC_FTD.md` is the authoritative framework spec.
2. `engine/SPEC_ENGINE.md` is the authoritative implementation-side spec.
3. Claim strength is not uniform across the repo; use the audit and evaluation documents to calibrate confidence.

---

## What This Repo Is

FTD is a mixed research-and-engineering repository with four public layers:

| Layer | Purpose | Start Here |
|-------|---------|------------|
| Theory and spec | Defines the model, ontology, derivations, and epistemic tags | [docs/SPEC_FTD.md](docs/SPEC_FTD.md), [docs/theory/META_INDEX.md](docs/theory/META_INDEX.md) |
| Executable engine | C++ simulation engine, CUDA path, WASM bindings, and browser dashboard | [engine/SPEC_ENGINE.md](engine/SPEC_ENGINE.md), [engine/CMakeLists.txt](engine/CMakeLists.txt) |
| Verification and proofs | Python constants, tests, verification runners, proof scripts | [scripts/constants.py](scripts/constants.py), [scripts/tests/README.md](scripts/tests/README.md) |
| Dissemination, critique, formalization | Manuscript/book/notebooks, evaluation reports, Lean formalization | [evaluation/AUDIT_WEAKNESSES_MASTER.md](evaluation/AUDIT_WEAKNESSES_MASTER.md), [lean/FTD.lean](lean/FTD.lean) |

The repo is not just a codebase. It is also a document library, a publication pipeline, a verification harness, and an assessment archive.

---

## Source Of Truth Policy

When documents disagree, use this order:

1. [docs/SPEC_FTD.md](docs/SPEC_FTD.md) for the framework definition and current epistemic discipline.
2. [engine/SPEC_ENGINE.md](engine/SPEC_ENGINE.md) for engine architecture, runtime model, and build surfaces.
3. Current code and filesystem reality for executable behavior:
   [engine/CMakeLists.txt](engine/CMakeLists.txt), [engine/src/](engine/src), [scripts/tests/](scripts/tests), [scripts/proofs/](scripts/proofs), [engine/config/](engine/config).
4. [docs/theory/07_assessment/AUDIT_EPISTEMIC_AUDIT.md](docs/theory/07_assessment/AUDIT_EPISTEMIC_AUDIT.md) and [evaluation/](evaluation) when you need to know how strong a claim really is.
5. Navigation docs such as [META_DOCUMENTATION_MAP.md](META_DOCUMENTATION_MAP.md) and [META_PROJECT_ATLAS.md](META_PROJECT_ATLAS.md) for discovery only, not as normative sources.
6. `archive/` and superseded theory documents as historical context only unless a newer document explicitly points back to them.

Two practical rules:

- Prefer live paths over stale counts.
- Prefer audits and evaluations over older “complete” or “derived” marketing shorthand when judging confidence.

---

## Epistemic Ground Rules

The project’s AI and documentation discipline is explicit and should be preserved:

- Do not run numerical near-miss searches and present them as evidence.
- Do not present substitution identities as discoveries.
- Do not label parametric insertions as derivations.
- Keep the epistemic tags in view:
  [AXIOM], [THEOREM], [SELECTION], [CONJECTURE], [IMPOSED], [EMERGENT], [OPEN].

Read these before making theory-facing edits:

- [docs/SPEC_FTD.md](docs/SPEC_FTD.md)
- [docs/theory/07_assessment/AUDIT_EPISTEMIC_AUDIT.md](docs/theory/07_assessment/AUDIT_EPISTEMIC_AUDIT.md)
- [docs/reference/REF_SCOPE_LIMITATIONS.md](docs/reference/REF_SCOPE_LIMITATIONS.md)
- [docs/reference/REF_EPISTEMIC_LABELS.md](docs/reference/REF_EPISTEMIC_LABELS.md)

---

## Subsystem Map

### 1. Theory And Documentation

**What it exists for**

- Defines the ontology, lattice postulates, derivation program, assessment layer, and theory catalog.

**Read these first**

1. [docs/SPEC_FTD.md](docs/SPEC_FTD.md)
2. [docs/theory/META_INDEX.md](docs/theory/META_INDEX.md)
3. [docs/theory/07_assessment/AUDIT_EPISTEMIC_AUDIT.md](docs/theory/07_assessment/AUDIT_EPISTEMIC_AUDIT.md)
4. [docs/reference/REF_SCOPE_LIMITATIONS.md](docs/reference/REF_SCOPE_LIMITATIONS.md)
5. [docs/reference/REF_SYMBOL_GLOSSARY.md](docs/reference/REF_SYMBOL_GLOSSARY.md)

**Practical entry points**

- Use [docs/theory/META_INDEX.md](docs/theory/META_INDEX.md) to pick the relevant theory lane.
- Use [docs/theory/07_assessment/core_ledgers/TRACKER_ONTIC_TRUTH.md](docs/theory/07_assessment/core_ledgers/TRACKER_ONTIC_TRUTH.md) for the 5-tier bedrock-truth ranking; read this FIRST before defending any FTD math claim.
- Use [docs/theory/07_assessment/core_ledgers/LEDGER.md](docs/theory/07_assessment/core_ledgers/LEDGER.md) for full per-claim provenance and tag history.
- Use [docs/theory/07_assessment/REF_CLAIMS_MATRIX.md](docs/theory/07_assessment/REF_CLAIMS_MATRIX.md) when you need dependencies or falsification framing.

**What feels stable**

- The existence of a single flagship spec.
- The category structure in `docs/theory/`.
- The explicit epistemic discipline and audit posture.

**Where drift is visible**

- Documentation counts and version labels have needed active cleanup across the public navigation layer; use [docs/audits/AUDIT_DOCUMENT_CLEANUP_LEDGER.md](docs/audits/AUDIT_DOCUMENT_CLEANUP_LEDGER.md) for the 2026-04 cleanup provenance.
- Claim strength differs across layers. [docs/SPEC_FTD.md](docs/SPEC_FTD.md) uses strong “derived/complete” language, while [docs/theory/07_assessment/AUDIT_EPISTEMIC_AUDIT.md](docs/theory/07_assessment/AUDIT_EPISTEMIC_AUDIT.md) explicitly downgrades many results to selection, insertion, or external physics adoption.

### 2. Engine And Runtime

**What it exists for**

- Implements the lattice engine, scale engines, optional CUDA backend, WASM bindings, and browser dashboard.

**Read these first**

1. [engine/SPEC_ENGINE.md](engine/SPEC_ENGINE.md)
2. [engine/include/ftd/scale_engine.h](engine/include/ftd/scale_engine.h)
3. [engine/include/ftd/render_bridge.h](engine/include/ftd/render_bridge.h)
4. [engine/src/render_bridge.cpp](engine/src/render_bridge.cpp)
5. [engine/CMakeLists.txt](engine/CMakeLists.txt)

**Practical entry points**

- Native build and test live in [engine/CMakeLists.txt](engine/CMakeLists.txt).
- The lattice runtime centers on [engine/src/render_bridge.cpp](engine/src/render_bridge.cpp).
- Cross-scale mechanics live in [engine/src/scale_bridge.cpp](engine/src/scale_bridge.cpp).
- Browser integration starts with [engine/web/js/app.js](engine/web/js/app.js) and [engine/web/js/bridge/bridge-factory.js](engine/web/js/bridge/bridge-factory.js).
- Runtime toggles and scenarios live in [engine/config/toggles.json](engine/config/toggles.json) and [engine/config/scenarios/](engine/config/scenarios).

**What feels stable**

- Clear separation between core C++, web modules, config JSON, and optional CUDA.
- A strong test culture with many engine tests registered in CMake.

**Where drift is visible**

- Large orchestration hotspots concentrate complexity:
  [engine/src/render_bridge.cpp](engine/src/render_bridge.cpp),
  [engine/src/main.cpp](engine/src/main.cpp),
  [engine/web/js/app.js](engine/web/js/app.js),
  [engine/CMakeLists.txt](engine/CMakeLists.txt).
- CPU and GPU paths can drift because the CUDA implementation mirrors core behavior in [engine/cuda/](engine/cuda).

### 3. Python Verification, Tests, And Proofs

**What it exists for**

- Houses the constants core, formal verification scripts, proof scripts, test runners, and exploratory checks.

**Read these first**

1. [scripts/constants.py](scripts/constants.py)
2. [scripts/tests/README.md](scripts/tests/README.md)
3. [scripts/tests/test_framework_integers.py](scripts/tests/test_framework_integers.py)
4. [scripts/tests/comprehensive/ftd_test_utils.py](scripts/tests/comprehensive/ftd_test_utils.py)
5. [scripts/proofs/common.py](scripts/proofs/common.py)

**Practical entry points**

- Small regression tests: [scripts/tests/run_all_tests.py](scripts/tests/run_all_tests.py)
- Tiered comprehensive suite: [scripts/tests/comprehensive/run_ultimate_test.py](scripts/tests/comprehensive/run_ultimate_test.py)
- Proof aggregation: [scripts/proofs/proof_master_verification.py](scripts/proofs/proof_master_verification.py)
- Decision/stress runners: [scripts/runners/run_decisive_tests.py](scripts/runners/run_decisive_tests.py), [scripts/runners/run_stress_tests.py](scripts/runners/run_stress_tests.py)

**What feels stable**

- `scripts/constants.py` is the shared constants hub.
- There is real shared infrastructure for proof results and tier verdicts.

**Where drift is visible**

- Naming drift in the classic test runner:
  [scripts/tests/run_all_tests.py](scripts/tests/run_all_tests.py) still imports `test_particle_masses` and `test_mixing_matrices`, while the live test files are [scripts/tests/test_mass_derivations_rigorous.py](scripts/tests/test_mass_derivations_rigorous.py) and [scripts/tests/test_mixing_matrices_rigorous.py](scripts/tests/test_mixing_matrices_rigorous.py).
- Some logic is duplicated across small tests, comprehensive tiers, and runner scripts.
- `scripts/constants.py` is useful but monolithic and carries visible repetition.

### 4. Dissemination, Evaluation, And Lean

**What it exists for**

- Produces outward-facing publications and pedagogy artifacts, records critique, and formalizes part of the math/program in Lean.

**Read these first**

1. [evaluation/AUDIT_WEAKNESSES_MASTER.md](evaluation/AUDIT_WEAKNESSES_MASTER.md)
2. [evaluation/AUDIT_UNRESOLVED_ISSUES.md](evaluation/AUDIT_UNRESOLVED_ISSUES.md)
3. [docs/theory/07_assessment/core_ledgers/TRACKER_ONTIC_TRUTH.md](docs/theory/07_assessment/core_ledgers/TRACKER_ONTIC_TRUTH.md)
4. [lean/FTD.lean](lean/FTD.lean)
5. [dissemination/manuscript/src/index.qmd](dissemination/manuscript/src/index.qmd)

**Practical entry points**

- Publication tree: [dissemination/manuscript/](dissemination/manuscript), [dissemination/book/](dissemination/book), [dissemination/whitepaper/](dissemination/whitepaper)
- Interactive and notebook outputs: [dissemination/interactive/](dissemination/interactive), [dissemination/notebooks/](dissemination/notebooks)
- Critique baseline: [evaluation/expert_reviews/PHYSICIST_FINAL_REPORT.md](evaluation/expert_reviews/PHYSICIST_FINAL_REPORT.md)

**What feels stable**

- The repo preserves critique instead of hiding it.
- The dissemination surface is broad and intentionally separate from the engine/runtime.

**Where drift is visible**

- Publication structure is not obvious from root alone; for example, the main manuscript config lives under `dissemination/manuscript/src/`.
- The Lean tree is substantial but hybrid: some bridges remain axiomatized rather than fully proven.

---

## Canonical Commands

Use the existing public commands from the repo docs unless you have a reason to verify or replace them.

```bash
# Engine build
cmake -S engine -B engine/build -DCMAKE_BUILD_TYPE=Release
cmake --build engine/build --config Release

# Engine tests
cd engine/build && ctest --output-on-failure -C Release

# WASM build
emcmake cmake -S engine -B engine/build_wasm -DCMAKE_BUILD_TYPE=Release
emmake cmake --build engine/build_wasm --target ftd_wasm

# Web dashboard
python -m http.server 8080 -d engine/web

# Python tests
python scripts/tests/run_all_tests.py
python scripts/tests/comprehensive/run_ultimate_test.py

# Proof / verification entry points
python scripts/proofs/proof_master_verification.py
python scripts/runners/run_decisive_tests.py

# Publication builds
cd dissemination/manuscript && quarto render
cd dissemination/whitepaper && pdflatex FTD_Whitepaper.tex
```

---

## Start Here By Contributor Type

### Theory / Documentation Contributor

1. Read [docs/SPEC_FTD.md](docs/SPEC_FTD.md).
2. Read [docs/theory/07_assessment/AUDIT_EPISTEMIC_AUDIT.md](docs/theory/07_assessment/AUDIT_EPISTEMIC_AUDIT.md) and [docs/reference/REF_SCOPE_LIMITATIONS.md](docs/reference/REF_SCOPE_LIMITATIONS.md).
3. Use [docs/theory/META_INDEX.md](docs/theory/META_INDEX.md) to choose a category.
4. Before editing, check [docs/theory/07_assessment/core_ledgers/TRACKER_ONTIC_TRUTH.md](docs/theory/07_assessment/core_ledgers/TRACKER_ONTIC_TRUTH.md) and [docs/theory/07_assessment/core_ledgers/LEDGER.md](docs/theory/07_assessment/core_ledgers/LEDGER.md) to confirm current tag status.
5. Treat `archive/` and superseded docs as historical unless a current document sends you there.

### Engine / Runtime Contributor

1. Read [engine/SPEC_ENGINE.md](engine/SPEC_ENGINE.md).
2. Build the engine and run CTest.
3. Read [engine/include/ftd/render_bridge.h](engine/include/ftd/render_bridge.h) and [engine/src/render_bridge.cpp](engine/src/render_bridge.cpp).
4. Trace one browser path through [engine/web/js/app.js](engine/web/js/app.js) and [engine/web/js/bridge/bridge-factory.js](engine/web/js/bridge/bridge-factory.js).
5. If a change touches parity-sensitive behavior, inspect both CPU and CUDA surfaces.

### Verification / Proof Contributor

1. Read [scripts/constants.py](scripts/constants.py).
2. Run [scripts/tests/run_all_tests.py](scripts/tests/run_all_tests.py) and [scripts/tests/comprehensive/run_ultimate_test.py](scripts/tests/comprehensive/run_ultimate_test.py).
3. Read [scripts/proofs/common.py](scripts/proofs/common.py) and [scripts/proofs/proof_master_verification.py](scripts/proofs/proof_master_verification.py).
4. Compare runner imports against the live `scripts/tests/` tree before assuming names in README files are current.
5. Use evaluation docs when deciding whether a script is demonstrating a theorem, a selection argument, or a parametric insertion.

---

## Cross-Cutting Risk Register

- **Version skew in navigation docs.**
  The root guides lag the flagship specs: [META_PROJECT_ATLAS.md](META_PROJECT_ATLAS.md) and [META_DOCUMENTATION_MAP.md](META_DOCUMENTATION_MAP.md) still report older framework/engine versions than [docs/SPEC_FTD.md](docs/SPEC_FTD.md) and [engine/SPEC_ENGINE.md](engine/SPEC_ENGINE.md).
- **Claim-strength tension across repo layers.**
  [README.md](README.md) and [docs/SPEC_FTD.md](docs/SPEC_FTD.md) present strong result language; [docs/theory/07_assessment/AUDIT_EPISTEMIC_AUDIT.md](docs/theory/07_assessment/AUDIT_EPISTEMIC_AUDIT.md) and [evaluation/AUDIT_UNRESOLVED_ISSUES.md](evaluation/AUDIT_UNRESOLVED_ISSUES.md) provide the necessary caveats.
- **Engine orchestration hotspots.**
  Changes that touch [engine/src/render_bridge.cpp](engine/src/render_bridge.cpp), [engine/src/main.cpp](engine/src/main.cpp), [engine/web/js/app.js](engine/web/js/app.js), or [engine/CMakeLists.txt](engine/CMakeLists.txt) tend to have wide blast radius.
- **Python naming drift and duplication.**
  The classic test runner and README lag current filenames in `scripts/tests/`, and some verification logic is duplicated across multiple layers.
- **Public vs. internal docs.**
  `docs/internal/` contains useful material but is gitignored and should not be treated as the public onboarding path.

---

## Actionable First-Day Checklist

1. Read this guide, then read [docs/SPEC_FTD.md](docs/SPEC_FTD.md).
2. Pick one lane: theory/docs, engine/runtime, or verification/proofs.
3. Read the lane-specific “Read these first” files above.
4. Run one canonical command from that lane.
5. Cross-check any strong claim against the audit or evaluation layer before repeating it in docs or code comments.
6. When you find a discrepancy, prefer live code and current authoritative specs over stale summaries.

If you only remember one rule, remember this: treat FTD as both a research program and a software project, and do not assume every polished statement in one layer has the same epistemic status in the others.
