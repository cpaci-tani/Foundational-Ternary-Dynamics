# scripts — Python tooling

**Purpose.** All Python scripts for the FTD project: canonical constants,
formal verification, mathematical proofs, experiments, exploration,
visualization, test runners.

## Public API

The single most-imported file is [`constants.py`](constants.py) — the
canonical Python source for all physics constants. Other scripts and
notebooks import from here:

```python
from scripts.constants import G_STAR, ALPHA, K_B, N_C, N_EFF, ...
```

For runnable scripts, use the `python -m scripts.<subdir>.<name>` form
(some scripts use relative imports).

## Internal structure

| Subdir | Role | Run pattern |
|---|---|---|
| `verification/` | Formal derivation verification (40+ scripts) | `python scripts/verification/verify_<name>.py` |
| `proofs/` | Mathematical proofs with error bounds (57+ scripts) | `python -m scripts.proofs.proof_<name>` (some use relative imports) |
| `experiments/` | Bell tests, CERN analysis, photon simulations (17 scripts) | `python scripts/experiments/<name>.py` |
| `exploration/` | Research investigations (25+ scripts; includes the 50-test physics battery `test_all_physics.py`) | `python scripts/exploration/<name>.py` |
| `tests/` | pytest suites (11 scripts) | `pytest scripts/tests/` |
| `tests/comprehensive/` | 7-tier verification framework | `python scripts/tests/comprehensive/run_ultimate_test.py` |
| `visualization/` | Publication figure generation (11 scripts) | `python scripts/visualization/<name>.py` |
| `runners/` | Test protocol orchestrators (2 scripts) | `python scripts/runners/<name>.py` |
| `benchmarks/` | Engine-vs-theory benchmark harness | `python scripts/benchmarks/benchmark_engine_vs_theory.py` |

Plus: `tools/` (under `engine/tools/` — engine-side tooling, not Python core).

## Dependencies

- **Imports from**: stdlib + `numpy`, `scipy`, `mpmath`, `sympy`, `matplotlib`
- **Imported by**: notebooks in `dissemination/notebooks/`, build scripts, manual runs
- **Path convention**: scripts are run from the repo root or via `python -m`

## Constants chain

`scripts/constants.py` is the **canonical Python source**. The C++
`engine/include/ftd/ontic.h` derives the same values; `engine/web/js/constants.js`
mirrors them; `resources/data/constants.json` is a tracked JSON snapshot
for non-Python consumers. All four MUST agree to ≥10 digits.

See [CONTRACTS.md §7](../CONTRACTS.md#7--constants-chain-contract) for the
chain invariants.

## Run-all helpers

```bash
# Full Python test pass
python scripts/tests/run_all_tests.py

# Master verification (54 checks)
python scripts/proofs/proof_master_verification.py

# 50-test physics battery
python scripts/exploration/test_all_physics.py
```

## How to extend

- **Add a verification script** → drop into `verification/` with a
  `verify_<name>.py` filename; should be runnable standalone with a
  pass/fail exit code.
- **Add a formal proof** → drop into `proofs/`; if you need shared helpers,
  use `proofs/common.py`. Run as `python -m scripts.proofs.<name>`.
- **Add a pytest test** → drop into `tests/`; standard `test_*.py`
  conventions; runs via `pytest scripts/tests/`.
- **Add a new physics constant** → edit `constants.py` first; mirror to
  C++ ontic chain, JS constants, and `resources/data/constants.json`;
  cite the layer/source in a comment.

## Invariants

- `constants.py` is the canonical source for Python; values agree with
  C++ `ontic.h` to ≥10 digits
- Verification scripts produce non-zero exit code on failure
- pytest suites are deterministic (no time-of-day dependencies)
- Proof scripts cite LEDGER rows in their docstrings

## Related docs

- [CONTRACTS.md §7](../CONTRACTS.md) (constants chain contract)
- [META_PROJECT_ATLAS.md](../META_PROJECT_ATLAS.md) §1 (run-all commands)
- C++ mirror: `engine/include/ftd/ontic.h`
- JS mirror: `engine/web/js/constants.js`
- JSON snapshot: `resources/data/constants.json`
