# Foundational Ternary Dynamics

[![License: CC BY-NC-SA 4.0](https://img.shields.io/badge/License-CC%20BY--NC--SA%204.0-lightgrey.svg)](https://creativecommons.org/licenses/by-nc-sa/4.0/)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![Python 3.10+](https://img.shields.io/badge/python-3.10+-blue.svg)](https://www.python.org/downloads/)
[![Engine v2.17.0](https://img.shields.io/badge/engine-v2.17.0-orange.svg)](engine/SPEC_ENGINE.md)

A discrete computational framework on a 3D cubic lattice with ternary states {−1, 0, +1}.

**FTD version:** 5.34 · **Engine:** 2.17.0 · **Author:** William J Steinmetz III

---

## Documentation

| Document | Purpose |
|---|---|
| [docs/SPEC_FTD.md](docs/SPEC_FTD.md) | Authoritative theoretical specification |
| [docs/theory/01_reference/SPEC_ALGEBRAIC_SPINE.md](docs/theory/01_reference/SPEC_ALGEBRAIC_SPINE.md) | Eight canonical theorems (the rigorous core) |
| [docs/theory/01_reference/SPEC_DIMENSIONAL_MAP.md](docs/theory/01_reference/SPEC_DIMENSIONAL_MAP.md) | Dimensionless ↔ dimensional bridge (15 entries) |
| [docs/theory/07_assessment/LEDGER.md](docs/theory/07_assessment/LEDGER.md) | Master claim ledger with epistemic tags |
| [docs/theory/07_assessment/CATALOG_PARAMETRIC_INSERTIONS.md](docs/theory/07_assessment/CATALOG_PARAMETRIC_INSERTIONS.md) | ~162 SM quantities, honestly classified |
| [META_PROJECT_ATLAS.md](META_PROJECT_ATLAS.md) | Task → file table, directory tree, dependency graph |
| [META_CONTRIBUTOR_ONBOARDING.md](META_CONTRIBUTOR_ONBOARDING.md) | New-contributor entry point |
| [engine/SPEC_ENGINE.md](engine/SPEC_ENGINE.md) | Simulation engine reference |

---

## Build

### Math (Python)

```bash
pip install numpy scipy sympy mpmath pytest
python -m pytest scripts/tests/
```

### Engine (C++)

```bash
cmake -S engine -B engine/build -DCMAKE_BUILD_TYPE=Release
cmake --build engine/build --config Release
cd engine/build && ctest --output-on-failure -C Release
```

### Engine (CUDA, via WSL2)

```bash
wsl.exe -d Ubuntu-22.04 -- bash -c "cd /mnt/c/Users/cpaci/Desktop/ftd && \
    cmake --build engine/build_wsl --target test_render_bridge_golden -j 8 && \
    engine/build_wsl/test_render_bridge_golden"
```

### Web dashboard

```bash
python engine/web/serve.py 8080
# http://localhost:8080
```

---

## Project layout

```
docs/         # Theory, specifications, audits, ADRs
engine/       # C++17 simulation engine + WASM bindings + Three.js dashboard
scripts/      # Python: constants, proofs, verification, tests, experiments
resources/    # Pure-data artifacts (constants.json, etc.)
dissemination/  # Manuscripts, whitepaper, notebooks, interactive HTML
```

---

## Tests

| Suite | Run with |
|---|---|
| C++ ctest (engine) | `cd engine/build && ctest --output-on-failure -C Release` |
| Python pytest | `python -m pytest scripts/tests/` |
| Playwright (web) | `cd engine/web/tests && npx playwright test` |
| Golden-tick regression | `cd engine/build && ctest -L golden -C Release` |

Bit-exact CPU↔CUDA parity is verified by the golden-tick gate (hash `0xcd957b601d47868a`).

---

## License

[CC BY-NC-SA 4.0](https://creativecommons.org/licenses/by-nc-sa/4.0/) — share and adapt with attribution, non-commercial use only. See [LICENSE](LICENSE).
