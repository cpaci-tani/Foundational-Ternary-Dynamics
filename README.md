# Foundational Ternary Dynamics

[![License: CC BY-NC-SA 4.0](https://img.shields.io/badge/License-CC%20BY--NC--SA%204.0-lightgrey.svg)](https://creativecommons.org/licenses/by-nc-sa/4.0/)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![Python 3.10+](https://img.shields.io/badge/python-3.10+-blue.svg)](https://www.python.org/downloads/)
[![Engine v2.18.0](https://img.shields.io/badge/engine-v2.18.0-orange.svg)](engine/SPEC_ENGINE.md)

A discrete computational framework on a 3D cubic lattice with ternary states {−1, 0, +1}. The project develops a small rigorous algebraic core, identifies the boundary of what discreteness alone can determine, and tracks every load-bearing claim under explicit epistemic tags.

**FTD version:** 5.40 · **Engine:** 2.18.0 · **Author:** William J. cpaci-tani III

> **Number-one goal:** *derive everything we can from a discrete ontology, and rigorously establish what we cannot.* The boundary is itself a deliverable.

---

## 1 · How to read this repository

This README is the landing page for a **skeptical physicist or mathematician** arriving cold. It states what is actually proven, what is conjectured, what has been ruled out, and where the open questions live. It does not summarize results to be stronger than they are.

The two canonical reference documents are:

- [`docs/theory/07_assessment/LEDGER.md`](docs/theory/07_assessment/LEDGER.md) — every load-bearing claim with its epistemic tag, dependency chain, and provenance. ~200 rows (FTD-0001 … FTD-0200+). **Single source of truth.**
- [`docs/theory/07_assessment/TRACKER_ONTIC_TRUTH.md`](docs/theory/07_assessment/TRACKER_ONTIC_TRUTH.md) — 5-tier truth bedrock distilled from the LEDGER. 21 core claims across T1 (rock-solid) through T5 (strongly motivated conjecture).

If this README and either of those documents disagree, the LEDGER and the tracker are correct. This README is never the place to strengthen a tag.

---

## 2 · Epistemic discipline

Every load-bearing claim in the corpus carries one of the following tags:

| Tag | Meaning |
|---|---|
| `[AXIOM]` | Structural postulate (not derivable) |
| `[THEOREM]` | Rigorously proven from axioms |
| `[DERIVED]` | Established by an explicit chain reproduced in the doc; weaker than `[THEOREM]` when the chain has named assumptions |
| `[SELECTION]` | Argued from consistency, not uniquely proven |
| `[STRONGLY MOTIVATED CONJECTURE]` | Conjecture with substantial structural and/or empirical evidence (e.g. uniqueness scans, multi-route convergence) but no derivation chain |
| `[CONJECTURE]` | Proposed interpretation requiring validation |
| `[NUMERICAL FACT]` | Verified by computation across a stated finite domain |
| `[PARAMETRIC]` | Standard physics formula filled with FTD constants — calibration input, not output |
| `[CLOSED NEGATIVE]` | Tested and falsified; preserved for provenance to prevent re-attempt |
| `[OPEN]` | Unresolved question |

Three project-level disciplines back the tags:

1. **No numerical near-miss searches.** Substitution identities (plugging FTD values into standard formulas and calling the result a discovery) are tagged `[PARAMETRIC]`, never `[DERIVED]`.
2. **Pre-registration with SHA256 + git tag before measurement.** Every locked scan/campaign has its protocol-script hash committed in a `preregister-*` git tag before the run; see [§7](#7--pre-registration-discipline).
3. **Closed-negative results are preserved, not deleted.** A falsified hypothesis stays in the LEDGER so the framework cannot zombie-re-emerge.

---

## 3 · What is proven: the algebraic spine

The canonical reference is [`SPEC_ALGEBRAIC_SPINE.md`](docs/theory/01_reference/SPEC_ALGEBRAIC_SPINE.md) — nine numbered results, **seven theorem-grade**, two honestly tiered below theorem grade.

### Seven theorem-grade results

| # | Name | Statement (informal) |
|---|---|---|
| **T1** | G\* algebraic identity | `G* := Γ(1/4)/Γ(3/4) = Γ(1/4)²/(π√2) = 2ϖ/√π ≈ 2.95867512…` via the Euler reflection identity. **Note: G\* ≠ ϖ** (the Bernoulli/Gauss lemniscate constant ≈ 2.62205755…); they are distinct numbers. |
| **T2** | Master quadratic + roots | The polynomial `P(x) = x² − 16G*²x + 16G*³` has roots `x_± = 8G*² ± 4G*·√(4G*² − G*)`. Pure algebra; no physics. |
| **T3** | CM-curve uniqueness (`h = 1`) | `K = Q(i)` is the unique imaginary quadratic field, among class-number-one IQ fields, satisfying the unit-group/discriminant order coincidence `|μ_K| = |disc(K)|`. |
| **T5** | Watson identity | `W₃ = G*² / (2π)`, where `W₃` is the BCC Watson integral. Conditional on Watson 1939 / Glasser-Zucker 1980. |
| **T6** | Phase G geometric Coulomb | The engine's gauss-projection step computes the lattice Poisson Green's function `G_L(r)` on the `L³` torus by construction; `α_r(r, L) := 2 r G_L(r)` is zero-free-parameter geometry. R² = 1.0000 at L = 384. |
| **T8** | (1 + i)-tower harmonic invariant | For the tower `M_k(x) = x² − 2^k G*^{k−2} x + 2^k G*^{k−1}` (`k ≥ 3`), `1/y_+ + 1/y_- = 1` with `y := x/G*`. Three-line Vieta. |
| **T9** | Field-theoretic Q(G\*) | `Q(G*)` is a maximal π-free subfield of `Q(π, Γ(1/4))`: `Q(G*) ∩ Q(π) = Q`. Conditional on Chudnovsky 1976 (algebraic independence of `{π, Γ(1/4)}` over `Q`). |

### Two results numbered but honestly tiered below theorem grade

- **T4 — Coefficient 16 = |Aut(E)|².** The prefactor `16` in the master quadratic equals `|Aut_{Q̄}(E: y² = x³ − x)|² = |Z[i]^×|² = 4² = 16`. **True at the value level.** The structural identification that the prefactor *must* equal `|Aut(E)|²` is conjectured (partial unification via [FTD-0122 / Paper B Thm 5.1](dissemination/papers/PAPER_B_BCC_COMPLEX_STRUCTURE.tex)), not proved.
- **T7 — Phase J partition-function ultralocality.** `[THEOREM at L = 2]` only (Nyquist-mode degeneracy origin); general L is numerically disconfirmed at L ≥ 4 (see `proof_phase_j_general_L.py`).

The 21-entry `[OT-N.M]` bedrock list with full T1-T5 tier assignments lives in [`TRACKER_ONTIC_TRUTH.md`](docs/theory/07_assessment/TRACKER_ONTIC_TRUTH.md).

---

## 4 · The central physics conjecture

The numerical root `x_+` of the master quadratic equals `137.036…`, matching CODATA 2022 `α⁻¹ = 137.035999177(21)` to **1.26 ppm**. This is the load-bearing physics claim of the framework. Its honest status:

> **`x_+ = 1/α`** — `[STRONGLY MOTIVATED CONJECTURE]` ([OT-5.1](docs/theory/07_assessment/TRACKER_ONTIC_TRUTH.md), [LEDGER FTD-0013](docs/theory/07_assessment/LEDGER.md)). **Not `[DERIVED]`.** No derivation chain from FTD axioms to α exists.

The structural-uniqueness evidence (which is what motivates the tag above `[CONJECTURE]`):

- **FTD-0189 adversarial look-elsewhere scan**: zero non-G\* dual-matchers across 2.65 M degree-2 polynomials over an 18-constant basket the framework did not design; the master quadratic is rank 1 by ~130×. Pre-reg tag [`preregister-adversarial-look-elsewhere-v1`](docs/theory/10_eft_program/REF_PREREGISTER_MANIFEST.md), commit `9e5ad8f`.
- **OT-1.9 + OT-3.3**: `Q(i)` is the unique class-number-one IQ field meeting the unit/discriminant coincidence; the polynomial-shape scan over `M_{n,p,m,q}(x) = x² − n G*^p x + m G*^q` (2,871,576 candidates, plus rational/cubic/Eisenstein extensions) returns 0 dual-matchers in the Eisenstein-integer family.
- **OT-1.5**: the BCC complex-structure theorem unifies `|Aut(E)|` count with tower level `k = 4` through `Z[BCC] ⊗ Q ⊇ Z[i]²`.

What is missing — **MC-T4.3**, the observable-selection readout principle — is the single load-bearing gap for upgrading FTD-0013 to `[DERIVED]`. The first locked attack on it is [FTD-0198 ARC-B1](docs/theory/10_eft_program/REF_PREREGISTER_MANIFEST.md) (pre-reg tag `preregister-alpha-readout-observable-selection-v1`, commit `0e79820`, SHA256 `e273ca85…`); the prior-favoured outcome is `[CLOSED NEGATIVE]`, and the value of the pre-reg is in making whichever verdict lands rigorous.

> **Retired claim.** The companion identification `x_- ↔ N_c` was removed 2026-05-22 per [FTD/FQCR Cleanup Taxonomy v1.4 §5](docs/theory/01_reference/SPEC_DOCTRINE_LEDGER.md); LEDGER row FTD-0014 deleted in commit `ca7eb61`. `N_c = 3` stands on independent structural grounds — see [`DERIV_NC_FROM_TOPOLOGY.md`](docs/theory/03_derivations/DERIV_NC_FROM_TOPOLOGY.md) (four routes) and the Moore Layer Theorem.

---

## 5 · What is closed-negative (the boundary of discreteness)

Per the project's number-one goal, closed-negative results are themselves deliverables. They map how far the discrete ontology reaches. A representative list:

**Eleven closed-negative routes to α derivation** (preserved in [OT-5.1](docs/theory/07_assessment/TRACKER_ONTIC_TRUTH.md); do not re-attempt):

- R1 transverse stiffness, R2 source-current normalization, R3 two-sector response eigenvalue, R4 projected Dirac matter
- Z-factor reading ([FTD-0116](docs/theory/07_assessment/LEDGER.md))
- RG-running, algebraic combinations of (G\*, π, 1/√d), Langevin-equipartition, monomial-fit scans, and others

**Other notable closures:**

- [FTD-0050](docs/theory/07_assessment/LEDGER.md) — master quadratic as RG-step characteristic polynomial (engine stencil is (SC + FCC)/2, BCC-orthogonal; master quadratic lives on BCC Watson integral). Closed without demoting FTD-0001/0013.
- [FTD-0093](docs/theory/07_assessment/LEDGER.md) — Mechanism C (`g_c` as bridge-operator eigenvalue on σ_BCC). Combined with prior closures of Mechanisms A + B, all three first-principles routes for `g_c` are now closed; `g_c` remains `[PARAMETRIC]`.
- [FTD-0094](docs/theory/07_assessment/LEDGER.md) — the L2 identity `2·m_e/α = 16 G*²` was demoted to `[PARAMETRIC]` per the pre-registered criterion.
- [FTD-0184](docs/theory/07_assessment/LEDGER.md) — the parallel exponential-metric gravity ontology is the Yilmaz metric (`[CLOSED NEGATIVE]`).
- [FTD-0200](docs/theory/07_assessment/LEDGER.md) — the "threshold-crossing produces the Born rule" assertion is `[CLOSED NEGATIVE]` in the 6-neighbour substrate (corpus assertions retagged); canonical 26-neighbour engine status `[OPEN]`.
- [FTD-0131](docs/theory/07_assessment/LEDGER.md) — the "G_N = 1/100 from `1/(b₃ + N_c)²`" identification was falsified; the substrate-derived gravitational fine-structure ratio for one electron is instead `α_G(e, e) = (m_e/m_P)² ≈ 1.745 × 10⁻⁴⁵`, matching the measured value to 0.38%.

Full closure provenance: [`LEDGER.md`](docs/theory/07_assessment/LEDGER.md). Sector-organized open queue: [`SPEC_OPEN_MATH_BY_SECTOR.md`](docs/theory/01_reference/SPEC_OPEN_MATH_BY_SECTOR.md).

---

## 6 · What landed recently (live state)

The session-resume document — what's actually happening as of this commit — is [`docs/WHERE_WE_LEFT_OFF.md`](docs/WHERE_WE_LEFT_OFF.md). Two recent closures of note (2026-05-25):

- **[FTD-0209 spin-2 boundary theorem](docs/theory/10_eft_program/FOUND_SPIN2_BOUNDARY_THEOREM.md)** — `[THEOREM]` at the free-theory + Gauss-only + canonical-toggle scope. Establishes that the FTD discrete substrate is fundamentally a scalar-vector gravity theory and does not admit emergent massless spin-2 gravitons at the stated scope. Pre-reg tag `preregister-spin2-boundary-theorem-v1`, commit `d8e016b`.
- **[FTD-0208 clock hypothesis](docs/theory/07_assessment/LEDGER.md)** — v2 closure attempt **INVALIDATED** by post-hoc audit on two independent axes (process: pre-reg + result authored same minute, never hash-locked; substance: imported a quadratic L²-norm budget primitive that is not substrate-derived). Status: `[UNDERDETERMINED]`. **v3 pre-reg queued.** Honest status documented in [`AUDIT_CLOCK_HYPOTHESIS_v2_UNDERDETERMINED.md`](docs/theory/03_derivations/AUDIT_CLOCK_HYPOTHESIS_v2_UNDERDETERMINED.md). The companion claim FTD-0131 retains `[DERIVED]` with one flagged interpretive step (clock hypothesis).

This pattern — one positive closure plus one honest audit-invalidation of a closure attempt — is the framework's normal state. The LEDGER tracks both.

---

## 7 · Pre-registration discipline

Every measurement scan or numerical campaign has its protocol script's SHA256 committed in a `preregister-*` git tag **before** the run. This is the project's structural anti-overclaim mechanism. The canonical manifest is [`REF_PREREGISTER_MANIFEST.md`](docs/theory/10_eft_program/REF_PREREGISTER_MANIFEST.md); a representative subset:

| Tag | FTD ID | Status |
|---|---|---|
| `preregister-look-elsewhere-scan-v1` | FTD-0097 | Closed (monomial-level rejection) |
| `preregister-emergent-spectrum-g1` / `-g2` | FTD-0107 | Closed (L = 64, L = 128) |
| `preregister-polynomial-scan-extended-v1` | FTD-0121 | Closed (2.87 M polynomials, 0 dual-matchers) |
| `preregister-fqcr-quotient-uniqueness-v1` | FTD-0143 | Queued |
| `preregister-alpha-arithmetic-generativity-v1` | FTD-0185 | Locked; desk-audit gate |
| `preregister-structural-dynamical-discriminator-v2` | FTD-0186 | Stage 1 closed positive per v2 |
| `preregister-adversarial-look-elsewhere-v1` | FTD-0189 | Closed (2.65 M-polynomial basket, 0 non-G\* dual-matchers) |
| `preregister-alpha-readout-observable-selection-v1` | FTD-0198 | Locked; closure attempt is a downstream arc |
| `preregister-threshold-crossing-born-v1` | FTD-0200 | Closed negative in 6-neighbour substrate |
| `preregister-spin2-boundary-theorem-v1` | FTD-0209 | Closed (FOUND, `[THEOREM]`) |
| `preregister-clock-hypothesis-derivation-v3` | FTD-0208 | Queued (v1 + v2 attempts UNDERDETERMINED) |

Tag/SHA verification: `git rev-list -n1 <tag-name>` and `git tag -l <tag-name>`.

---

## 8 · Open research queue

Top items from [`TRACKER_OPEN_ITEMS.md`](docs/theory/07_assessment/TRACKER_OPEN_ITEMS.md) and `WHERE_WE_LEFT_OFF.md`:

1. **MC-T4.3 alpha readout principle** — the single load-bearing gap for promoting FTD-0013 from `[STRONGLY MOTIVATED CONJECTURE]`. FTD-0198 ARC-B1 is the first locked attack.
2. **FTD-0186 Structural Decoupling Theorem Stage 2** — Stage 1 closed positive per v2; Stage 2 must produce a provable proposition with stated axioms.
3. **FTD-0208 clock-hypothesis v3** — substrate-derive the budget-conservation primitive itself, not import it.
4. **FTD-0110 nonlinear bridge** — linear-level `[DERIVED]` from `O_h` representation theory; nonlinear regime remains `[STRONGLY MOTIVATED CONJECTURE]`.
5. **Confinement substrate-derivation** — FTD-0025 still `[SELECTION]`.
6. **Engine callstack audit** — top finding F2 (four toggles silently no-op on CPU). See [`AUDIT_ENGINE_CALLSTACK.md`](docs/theory/07_assessment/AUDIT_ENGINE_CALLSTACK.md).

---

## 9 · Engine

A C++17 discrete-lattice simulator with CUDA GPU backend, WASM bindings, and a Three.js web dashboard. Engine version **2.18.0**. Bit-exact CPU ↔ CUDA parity is verified by a golden-tick regression gate (hash `0xcd957b601d47868a` at L = 16).

**Scale.** 257 C++ test source files (211 active CMake targets), 454 Python scripts, 257 JS modules, ~2,500 tracked files total.

### Build

```bash
# Math (Python)
pip install numpy scipy sympy mpmath pytest
python -m pytest scripts/tests/

# Engine (C++, native CPU)
cmake -S engine -B engine/build -DCMAKE_BUILD_TYPE=Release
cmake --build engine/build --config Release
cd engine/build && ctest --output-on-failure -C Release

# Engine (CUDA via WSL2 — required for GPU campaigns; RTX 5090 ~30× speedup)
wsl.exe -d Ubuntu-22.04 -- bash -c "cd /mnt/c/Users/cpaci/Desktop/ftd && \
    cmake --build engine/build_wsl --target test_render_bridge_golden -j 8 && \
    engine/build_wsl/test_render_bridge_golden"

# WASM (Emscripten)
emcmake cmake -S engine -B engine/build_wasm -DCMAKE_BUILD_TYPE=Release
emmake cmake --build engine/build_wasm --target ftd_wasm

# Web dashboard (no-cache dev server)
python engine/web/serve.py 8080   # http://localhost:8080
```

GPU campaigns **must** go through WSL2; Windows-native CUDA is for compile-time / single-tick correctness checks only.

### Tests

| Suite | Run with |
|---|---|
| C++ ctest (engine) | `cd engine/build && ctest --output-on-failure -C Release` |
| Python pytest | `python -m pytest scripts/tests/` |
| Playwright (web) | `cd engine/web/tests && npx playwright test` |
| Golden-tick regression | `cd engine/build && ctest -L golden -C Release` |

Engine reference: [`engine/SPEC_ENGINE.md`](engine/SPEC_ENGINE.md).

---

## 10 · Reproducibility

**Verifying a single LEDGER claim.** Most theorem-grade and derived claims have a script under [`scripts/proofs/`](scripts/proofs/) (`proof_master_verification.py`, `proof_field_theoretic_qgstar.py`, `proof_harmonic_invariant_tower.py`, `proof_bcc_complex_structure.py`, `proof_complete_sm.py`, etc.). Each script prints PASS/FAIL per assertion; the master verification runs 54/54 checks.

**Reproducing a pre-registered campaign.** Check out the locked tag, run the launcher in [`engine/tools/`](engine/tools/), compare hashes against the analysis document. Recipes in [`REF_PREREGISTER_MANIFEST.md`](docs/theory/10_eft_program/REF_PREREGISTER_MANIFEST.md). Engine campaign outputs are local-only by default — they regenerate from `engine/tests/campaign_*.cpp` plus the corresponding launcher.

**Compiled paper PDFs are no longer tracked** (see commit history for the cleanup). Rebuild with `latexmk -pdf` inside [`docs/papers/`](docs/papers/).

**Hardware actually used.** RTX 5090 via WSL2 Ubuntu-22.04, CUDA 13.0, for all GPU campaigns. Native Windows builds are correctness-only.

---

## 11 · Reading paths by audience

| You are a … | Start with |
|---|---|
| **Mathematician** | [`SPEC_ALGEBRAIC_SPINE.md`](docs/theory/01_reference/SPEC_ALGEBRAIC_SPINE.md) + [`docs/papers/PAPER_GSTAR_INTRODUCTION.tex`](docs/papers/PAPER_GSTAR_INTRODUCTION.tex) |
| **Physicist** | [`SPEC_DOCTRINE_LEDGER.md`](docs/theory/01_reference/SPEC_DOCTRINE_LEDGER.md) + [`LEDGER.md`](docs/theory/07_assessment/LEDGER.md) |
| **Skeptic / reviewer** | [`AUDIT_EPISTEMIC_AUDIT.md`](docs/theory/07_assessment/AUDIT_EPISTEMIC_AUDIT.md) + [`AUDIT_HIDDEN_SELECTIONS.md`](docs/theory/07_assessment/AUDIT_HIDDEN_SELECTIONS.md) + [§5 closed-negative results above](#5--what-is-closed-negative-the-boundary-of-discreteness) + [`CATALOG_PARAMETRIC_INSERTIONS.md`](docs/theory/07_assessment/CATALOG_PARAMETRIC_INSERTIONS.md) |
| **Programmer** | [`engine/SPEC_ENGINE.md`](engine/SPEC_ENGINE.md) + [`META_PROJECT_ATLAS.md`](META_PROJECT_ATLAS.md) + [`CONTRACTS.md`](CONTRACTS.md) |
| **New contributor** | [`META_CONTRIBUTOR_ONBOARDING.md`](META_CONTRIBUTOR_ONBOARDING.md) + [`CONTRIBUTING.md`](CONTRIBUTING.md) |

For the full audience-segmented map: [`META_DOCUMENTATION_MAP.md`](META_DOCUMENTATION_MAP.md).

---

## 12 · Publications

Per the [FTD-0171 paper split](docs/theory/07_assessment/LEDGER.md), the two G\* papers are now separate:

- [`docs/papers/PAPER_GSTAR_INTRODUCTION.tex`](docs/papers/PAPER_GSTAR_INTRODUCTION.tex) — pure number theory (Kronecker χ\_{−4} as joint source of the lemniscatic identity algebra). 30 pages, MSC 11G15 primary. Final grade A after 4-round red-team audit (FTD-0173 / FTD-0174). Target: Crelle / Compositio / Math. Ann. / JLMS; submission-ready pending arXiv form-fill.
- [`docs/papers/PAPER_GSTAR_FTD_BRIDGE.tex`](docs/papers/PAPER_GSTAR_FTD_BRIDGE.tex) — physics-facing bridge synthesis (the master quadratic `P_{G*}` as math/physics bridge through χ\_{−4} and the ternary substrate). 5 pages. Target: Foundations of Physics.

Earlier publication drafts and historical papers live under [`docs/papers/`](docs/papers/) and [`dissemination/papers/`](dissemination/papers/).

---

## 13 · Project layout

```
docs/             # Theory (~420 markdown docs, 10 categories), audits, papers, ADRs
engine/           # C++17 simulation engine + CUDA + WASM + Three.js dashboard
scripts/          # Python: constants, proofs (~57), verification, tests, experiments
resources/        # Pure-data artifacts (constants.json, etc.)
dissemination/    # Manuscripts, whitepaper, notebooks, interactive HTML
META_*.md         # Root-level orientation: atlas, documentation map, onboarding
CONTRACTS.md      # Cross-module contract specifications
MAINTAINABILITY.md  # Field manual: hazards, recipes, tech-debt ledger
```

---

## 14 · How to cite + License

```bibtex
@misc{cpaci-tani2026ftd,
  author = {William J. cpaci-tani III},
  title  = {Foundational Ternary Dynamics},
  year   = {2026},
  note   = {Version 5.40, engine 2.18.0},
  url    = {https://github.com/<repo>}
}
```

For the algebraic results specifically, cite the G\* paper(s) under [§12](#12--publications) rather than this repository.

**License.** [CC BY-NC-SA 4.0](https://creativecommons.org/licenses/by-nc-sa/4.0/) — share and adapt with attribution, non-commercial use only. See [LICENSE](LICENSE).

---

## 15 · For AI collaborators

[`CLAUDE.md`](CLAUDE.md) and [`AGENTS.md`](AGENTS.md) carry the agent instructions, including the epistemic-discipline rules (no numerical near-miss searches, no substitution identities, no `[PARAMETRIC]`-labelled-as-`[DERIVED]`). [`META_PROJECT_ATLAS.md`](META_PROJECT_ATLAS.md) is the task → file navigation atlas. [`docs/WHERE_WE_LEFT_OFF.md`](docs/WHERE_WE_LEFT_OFF.md) is the session-resume document — the correct entry point for any LLM continuing work. AI co-authorship is **not** credited in commits on this project; see CLAUDE.md "Commit Policy".
