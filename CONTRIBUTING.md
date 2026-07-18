# Contributing to Foundational Ternary Dynamics

Thanks for helping improve FTD. This repository mixes theory documents, verification scripts, a C++/CUDA engine, and web dashboard code, so contributions need to be explicit about both engineering impact and epistemic status.

## Good Contribution Types

- Bug reports with OS, Python/CMake/compiler versions, commands run, and full error output.
- Focused engine, web, or verification fixes with tests.
- Documentation corrections that align downstream prose to the canonical ledgers.
- Theory or derivation edits that preserve provenance and avoid claim promotion unless the proof chain is actually present.
- Cleanup PRs that archive or relabel superseded material without deleting scientific history.

## Development Setup

```bash
git clone https://github.com/YOUR_USERNAME/Foundational-Ternary-Dynamics.git
cd Foundational-Ternary-Dynamics

python -m venv venv
source venv/bin/activate  # Windows: venv\Scripts\activate

pip install -r requirements.txt
pip install -r requirements-dev.txt
```

## Common Verification

Run the narrowest relevant checks first, then broaden when touching shared code.

```bash
python -m pytest scripts/tests/
python scripts/proofs/proof_master_verification.py
```

For C++ engine work:

```bash
# Windows native (pins MSVC 14.44 -- VS 18's default toolset crashes CUDA 13's cudafe++)
engine\build_native.bat
ctest --test-dir engine/build -j 24 --output-on-failure -C Release
```

For the web dashboard:

```bash
python engine/web/serve.py 8080
```

## Epistemic Standards

Claim status is not decided by local prose. Use the canonical sources:

- `docs/theory/07_assessment/core_ledgers/LEDGER.md` for claim tags.
- `docs/theory/07_assessment/core_ledgers/TRACKER_ONTIC_TRUTH.md` for bedrock truth tiers.
- `docs/theory/01_reference/SPEC_ALGEBRAIC_SPINE.md` for theorem statements.
- `docs/theory/07_assessment/CATALOG_PARAMETRIC_INSERTIONS.md` for imported standard-physics formulas.

Use current tags such as `[AXIOM]`, `[THEOREM]`, `[DERIVED]`, `[NUMERICAL FACT]`, `[SELECTION]`, `[STRONGLY MOTIVATED CONJECTURE]`, `[PARAMETRIC]`, `[IMPOSED]`, `[CLOSED NEGATIVE]`, `[OPEN]`, and `[SYNTHESIS]`. Do not introduce obsolete labels for new work.

Mandatory rules:

1. Do not run numerical searches for near-misses or coincidences.
2. Do not call standard-physics substitutions derivations.
3. Do not promote a claim during cleanup.
4. Preserve closed-negative and retracted routes as provenance.
5. Update navigation layers when moving or status-changing documents.

## Documentation Cleanup

Cleanup work should follow `docs/theory/META_STRUCTURE.md`, `docs/audits/AUDIT_DOCUMENT_CLEANUP_LEDGER.md`, and `docs/audits/PLAN_PROJECT_CLEANUP_2026-06-17.md`.

Prefer these moves:

- Fix stale links and stale metadata in place.
- Move superseded theory docs to the appropriate archive using `git mv`.
- Update `docs/theory/META_INDEX.md` and local `INDEX_*.md` files with any moved document.
- Keep generated outputs and regenerable campaign data out of git unless a specific campaign is intentionally force-added.

## Pull Request Checklist

Before opening a PR:

1. Explain whether the change is code, documentation, theory, cleanup, or verification.
2. Cite the canonical source for any changed claim status.
3. List tests or documentation checks run.
4. Note any intentionally deferred checks.
5. Keep commits small enough to audit.

## Code of Conduct

Be precise, respectful, and falsification-minded. Strong claims need strong evidence, and negative results are part of the project record.
