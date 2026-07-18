# Copilot Instructions

## Repository Shape

This is the Foundational Ternary Dynamics (FTD) research repository. It contains:

- `docs/` - framework specs, theory corpus, audit ledgers, reference material.
- `engine/` - C++17 simulation engine, CUDA backend, WASM bindings, browser dashboard.
- `scripts/` - Python proofs, verification tools, tests, experiments, and visualization scripts.
- `evaluation/` - project health and weakness audits.
- `dissemination/` - papers, whitepaper, manuscripts, notebooks, and interactive demos.

The first navigation files are `README.md`, `META_PROJECT_ATLAS.md`, `META_DOCUMENTATION_MAP.md`, and `docs/WHERE_WE_LEFT_OFF.md`.

## Scientific Ground Rules

FTD uses canonical epistemic ledgers. Do not infer claim status from local prose when a canonical source exists.

- Claim tags: `docs/theory/07_assessment/core_ledgers/LEDGER.md`
- Truth tiers: `docs/theory/07_assessment/core_ledgers/TRACKER_ONTIC_TRUTH.md`
- Theorem statements: `docs/theory/01_reference/SPEC_ALGEBRAIC_SPINE.md`
- Parametric insertions: `docs/theory/07_assessment/CATALOG_PARAMETRIC_INSERTIONS.md`

Mandatory discipline:

- Do not run numerical near-miss or coincidence searches.
- Do not call standard-physics substitutions derivations.
- Do not promote epistemic tags during cleanup.
- Preserve closed-negative and retracted routes as provenance.
- If a README or downstream doc conflicts with the ledgers, the ledgers win.

## Where To Edit

- Engine physics: `engine/src/render_bridge.cpp`, `engine/include/ftd/`, `engine/cuda/`, and registered tests in `engine/tests/`.
- WASM bindings: `engine/wasm/`.
- Web dashboard: `engine/web/js/`, with Scale 0 runtime in `engine/web/js/scales/scale0/`.
- Physics constants: keep `scripts/constants.py`, `engine/include/ftd/ontic.h`, and `engine/web/js/constants.js` synchronized.
- Theory docs: use the cluster indexes under `docs/theory/*/INDEX_*.md` plus `docs/theory/META_INDEX.md`.
- Cleanup provenance: `docs/audits/AUDIT_DOCUMENT_CLEANUP_LEDGER.md` and `docs/audits/PLAN_PROJECT_CLEANUP_2026-06-17.md`.

## Common Commands

Python:

```bash
python -m pytest scripts/tests/
python scripts/proofs/proof_master_verification.py
```

C++ engine:

```bash
# Windows native builds MUST use the MSVC 14.44 toolset (VS 18's default
# 14.51+ crashes CUDA 13.0's cudafe++). The wrapper enters
# vcvarsall x64 -vcvars_ver=14.44 and drives engine/CMakePresets.json:
engine\build_native.bat
ctest --test-dir engine/build -j 24 --output-on-failure -C Release
```

Web dashboard:

```bash
python engine/web/serve.py 8080
```

GPU measurement campaigns should run through WSL2 Ubuntu-22.04, not Windows-native CUDA.
