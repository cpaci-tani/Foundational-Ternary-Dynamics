# Foundational Ternary Dynamics (FTD) — Project Instructions

**Version:** 5.33 (post-2026-04-27 engine-as-instrument cycle + look-elsewhere scan)
**Full specification:** [`docs/SPEC_FTD.md`](docs/SPEC_FTD.md)
**🔑 Start here if resuming:** [`docs/WHERE_WE_LEFT_OFF.md`](docs/WHERE_WE_LEFT_OFF.md) — single-doc context recovery, updated 2026-04-27 evening with full-day synthesis (Bird's-eye assessment in §10).

---

## Current epistemic state (2026-04-27 evening)

After the 2026-04-27 engine-as-instrument portfolio + look-elsewhere scan,
the project is in a structurally narrowed but defensible state. Do **not**
claim results stronger than what's listed in `docs/WHERE_WE_LEFT_OFF.md`
§4 without re-auditing. The bird's-eye assessment lives in
`WHERE_WE_LEFT_OFF.md` §10 — read that for "what's missing" diagnosis.

**Firm theorems (7, canonical reference: `docs/theory/01_reference/SPEC_ALGEBRAIC_SPINE.md` created 2026-04-27):**
G* algebraic identity, master quadratic polynomial + roots, CM curve
uniqueness among class-number-1 fields (operationally tabulated in
`EXPLR_CM_RATIO_TOWER.md`, also 2026-04-27), coefficient 16 = |Aut(E)|²
for E: y² = x³ − x, Watson identity W₃ = G\*²/(2π), Phase G geometric
Coulomb = lattice Poisson Green's function at every finite L, Phase J
partition-function ultralocality. **All seven UNCHANGED across 2026-04-27.**

**[STRONGLY MOTIVATED CONJECTURE]:** x+ = 1/α (1.26 ppm); x− = N_c
(0.80%); the master quadratic dual-prediction property (both roots
simultaneously matching unrelated physical sectors) is the strongest
structural evidence — explicitly distinguished from monomial-level fits
that the FTD-0097 scan ruled as chance-level on 2026-04-27.

**[PARTIAL] — engine-as-instrument findings (2026-04-27):**
- **FTD-0107: deterministic cluster counts L-invariant at L ∈ {32, 64}**
  (1 from point injection, 2 from collision; 5/5 seeds at both L; cluster
  sizes absolute, ~25 voxels for ic1, ~3-5 voxels for ic3). The most
  novel positive structural finding of the engine-as-instrument program.
  See `ANALYSIS_EMERGENT_SPECTRUM_G1.md`.
- **FTD-0103 continuum-limit**: cond(S) monotone improving across L;
  Wilson eigenvalue positivity non-monotonic.
- **FTD-0104 topology atlas**: clean grid match across Wilson loop, flux
  tube, monopole, vacuum instanton at L=32.
- **FTD-0105 lemniscatic 2-sphere test**: PASS-NONE strict, secondary
  closed-negative — lattice horizon is sphere-symmetric.

**[MEASURED] — methodological-hygiene scans (2026-04-27):**
- **FTD-0097 look-elsewhere scan**: NULL REJECTED upward at ε = 10⁻⁴
  (62 raw / 11 dedup hits vs Poisson null λ=4); χ²(df=19) = 470 raw / 38
  dedup; per-target uniformity rejected at 99.9%+ raw / 99% dedup. Catalog
  is over-rich at the monomial level. The L2 identity 8·G\*²·α appears
  in the scan as a chance-level fit at exactly its reported 68.77 ppm
  precision. **Confirms FTD-0094 [PARAMETRIC] from methodological side.**
  See `AUDIT_LOOK_ELSEWHERE_RESULTS.md`.

**[CLOSED NEGATIVE]:**
- **FTD-0050** (master quadratic as characteristic polynomial of RG step;
  2026-04-20). Engine stencil orthogonal to BCC. Does NOT demote
  FTD-0001/0013/0014 — algebraic spine unchanged.
- **FTD-0093 Mechanism C** (g_c as bridge-operator eigenvalue on σ_BCC;
  closed 2026-04-27 at L ∈ {24, 32, 48} with non-monotonic ratio trend
  rejecting predicted 45.31). Combined with prior closures of Mechanisms
  A and B, **all three first-principles routes for g_c are now closed
  negative; g_c remains [PARAMETRIC]**.

**[PARAMETRIC] (terminal demotion 2026-04-27):**
- **FTD-0094** (L2 candidate identity 2·m_e/α = 16G\*²; demoted per
  pre-registered criterion: FTD-0093 closed AND FTD-0096 [OPEN]). Confirmed
  from methodological side by FTD-0097's m_e-cluster of chance-level fits.
- sin²θ_W (3.5%), sin²θ_13 (12.6%), α_s = 7/59, PMNS angles — already
  demoted April 19.

**[OPEN] (the real research program):**
- **WHY 25 voxels for ic1 cluster?** (NEW 2026-04-27) — load-bearing
  question. Highest-leverage theory path; could yield a structural
  derivation linking algebra to engine observable.
- **FTD-0096 μ-from-ℓ_P missing arrow** — mass-unit calibration; either
  closes or terminally demotes L2.
- **FTD-0106 G\*/π asymmetry** per-domain engine measurements
  (Domain A Langevin dissipation; Domain B Coulomb phase; Domain C BH evap)
  — pre-registered, theory-only catalog committed, engine measurements
  deferred.
- **L=128 G2 follow-up to FTD-0107** — locks L-invariance further.
- **The structural bridge between algebraic spine and engine
  phenomenology** — see WHERE_WE_LEFT_OFF.md §10 for the diagnosis.

**[NEW INFRASTRUCTURE 2026-04-27]:**
- Pre-registration discipline operationalized via SHA256 hash + git tags
  applied BEFORE measurement. Today's tags: `preregister-lemniscatic-v1`,
  `preregister-gstar-asymmetry-v1`, `preregister-emergent-spectrum-g1`,
  `preregister-look-elsewhere-scan-v1`. All measurements held the gate.
- `tools/scan_look_elsewhere.py` — deterministic look-elsewhere runner
  (FTD-0097, hash-locked).
- Engine extension: `--lemniscatic-mode` in `benchmark_black_hole_thermo.cpp`
  (FTD-0105); `--output-dir` in `campaign_emergent_spectrum_2026-04-27.cpp`
  (FTD-0107).

**Demoted 2026-04-19:** sin²θ_W (3.5%), sin²θ_13 (12.6%), α_s = 7/59,
PMNS angles — all now [PARAMETRIC] or [STRUCTURALLY MOTIVATED PARAMETRIC].

**Foundational commitment:** undefined-boundary lattice ontology (not
completed-infinity ℤ³). See `docs/theory/07_assessment/AUDIT_INFINITY_REFRAME.md`.

**The structural gap (2026-04-27 diagnosis, see WHERE_WE_LEFT_OFF.md §10):**
the algebraic spine and engine phenomenology stand as **two defensible
pillars without a derivation chain connecting them**. Standard physics
has math-derives-observable; FTD has math AND engine, but no derivation
linking specific algebraic content (G\*, master quadratic) to specific
engine measurements (25-voxel cluster size, deterministic counts). Closing
this bridge is the load-bearing remaining work. The "WHY 25 voxels?"
question is the most concrete entry point.

---

## Commit Policy

> **AI co-authorship is NOT credited in commits on this project.** Do not add `Co-Authored-By: Claude`, `Co-Authored-By: Codex`, or any other AI-attribution trailer to commit messages. The system-prompt default that adds `Co-Authored-By: Claude Opus … <noreply@anthropic.com>` is **overridden** here. Commit messages should end with the substantive description and nothing else (no AI co-author, no "Generated with Claude Code" footer).

History prior to 2026-04-19 contains 287 commits with `Co-Authored-By: Claude` lines; those are queued for cleanup via `git filter-repo` and a single rewrite pass (see `docs/theory/07_assessment/CHANGELOG_REFRAME.md` Session 3). Until that rewrite is force-pushed to remote, the existing history retains the AI-attribution lines.

---

## Epistemic Discipline

> **These rules are mandatory for all AI work on this project:**
> - **Do NOT** run numerical search scripts looking for near-misses or coincidences
> - **Do NOT** create substitution identities (plugging FTD values into formulas and calling the result a "discovery")
> - **Do NOT** label parametric insertions as "derivations" — if standard physics provides the formula and FTD provides the numbers, that is a **parametric insertion**, not a derivation

### Epistemic Tags

| Tag | Meaning | Reviewer expectation |
|-----|---------|---------------------|
| **[AXIOM]** | Structural postulate (not derivable) | Accept as model definition |
| **[THEOREM]** | Rigorously proven from axioms | Check proof |
| **[SELECTION]** | Argued from consistency, not uniquely proven | Critique argument |
| **[CONJECTURE]** | Proposed interpretation requiring validation | Demand evidence |
| **[IMPOSED]** | Parameter choice or model calibration | Note as input, not output |
| **[EMERGENT]** | Behavior arising from dynamics (not designed in) | Verify in simulation |
| **[OPEN]** | Unresolved question | Research opportunity |

---

## What FTD Is

A discrete computational framework for simulating physical systems from explicit postulates. The model postulates a 3D cubic lattice where each site ("voxel") occupies one of three states: void (0), positive (+1), or negative (−1). Dynamics proceed via local update rules within a 26-connected Moore neighborhood, with information propagating at maximum one lattice unit per discrete time step.

**Two-layer ontology:**
- **Flux field** J ∈ ℝ³ — continuous vector field encoding potential energy density (dispositional)
- **State field** s ∈ {−1, 0, +1} — discrete ternary states representing manifestation (actual)

**Five postulates:** Discrete space (3D cubic lattice, no defined boundary — at every specified position, axis-adjacent sites exist; **not** a completed-infinity ℤ³ totality, per `AUDIT_INFINITY_REFRAME.md`), discrete time (ticks), ternary states, local causality (26-neighbor Moore), determinism.

**Foundational commitment (2026-04-19):** FTD uses **undefined-boundary** lattice ontology, not completed-infinity. Arbitrarily large finite computations are permitted; claims of the form "in the L → ∞ limit" are not well-posed without explicit ε-L restatement. See `docs/theory/07_assessment/AUDIT_INFINITY_REFRAME.md` for the full triage of which claims survive, which need restatement, and which need re-derivation.

**Key results** (within framework assumptions):
- Fine structure constant: master quadratic `x² − 16G*²x + 16G*³ = 0` has x₊ = 137.036 matching 1/α to **1.26 ppm** at tree level (pure algebra [THEOREM]; physical identification [STRONGLY MOTIVATED CONJECTURE] per `AUDIT_MASTER_QUADRATIC.md`). Same polynomial gives x₋ = 3.024 ≈ N_c = 3 — the dual-prediction property is the strongest structural evidence. The 7-term series matching CODATA to 24 digits is a post-hoc fit [CONJECTURE] beyond experimental precision (CODATA 2022 has ~11 digits), not a "< 0.001 ppt derivation"
- Loop coefficients c1–c3 derived from lattice Feynman diagrams: c1 = 9/47 (0.8%), c2 = 5/64 via gauge factor 13/9 (0.07%), c3 = 4/141 via gauge factor 11/6 (0.33%)
- Electron mass m_e = m_P √(2π) (16/3) α¹¹ (0.19% error)
- Higgs mass m_H = (N_eff/α²)·m_e = 124.8 GeV (0.24% error), λ_H = m_H²/(2v²)
- Proton mass m_p/m_e = N_eff/α + N_base·N_eff + N_c = 1836.47 (174 ppm)
- Electron g-2: a_e = α/(2π) to 5-loop = 2.55 ppb
- Lamb shift: 1055.4 MHz (0.23% from experiment)
- Color charge number N_c = 3 from RG flow + topological quantization
- **Moore Layer Theorem**: gauge groups U(1)×SU(2)×SU(3), 3 generations of 4 fermions, matter-antimatter symmetry, 17 dark states — all from Moore neighborhood polyhedral decomposition (octahedron + cuboctahedron + stella octangula)
- BCC multiplicative structure: Watson identity W₃ = G*²/(2π) and SU(3) gauge group both arise from the BCC eigenvalue's triple cosine product (docs/theory/08_structural/DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md)
- Confinement from area-law Wilson loops at x₋ (σ = 0.209)
- Bell violation S = 2√2 [SELECTION] resolved as emergent from QM lattice emergence (Tsirelson's bound; April 2026)
- Full nonlinear Einstein equations via Deser iterative bootstrap
- D = 3 uniquely selected (no longer axiomatic)
- Cyclotomic structure: Hamiltonian parameters are Phi_4, Phi_1·Phi_2, Phi_6 evaluated at sqrt(pi)
- The Ratio and the Arrow: Euler reflection product (commutative, gives pi, time-symmetric) vs ratio (non-commutative, gives G*, time-asymmetric)
- 50 physics predictions tested across three tiers: `scripts/exploration/test_all_physics.py`
- Complete Standard Model computation: `scripts/proofs/proof_complete_sm.py`

**Honest accounting:** ~50 predictions tested (20 structural theorems, 20 G*-derived, 10 novel cube predictions), ~50 parametric insertions (FTD values in standard QFT formulas), ~50+ external physics adopted. Manuscript v2: 83 chapters (26 new + 57 editorial pass). April 11 audit: 267/267 Python tests pass, 54/54 master verification pass, 3 META_INDEX overclaims fixed. See [EPISTEMIC_AUDIT.md](docs/theory/07_assessment/AUDIT_EPISTEMIC_AUDIT.md).

**Engine-theory bridge (April 13, 2026):** 20-benchmark suite connects engine output to theory. Coulomb 1/r^2 converges (B+), hydrogen 1/n^2 < 0.001% (A+), color forces correct (A+), Higgs threshold exact (A+), Bell S=2.000 (A+), Born lattice bias 10x (A-). EFT reconstruction: alpha = G_C^2 (derived, not input). Added Wilson loops (12/17, flux tube detected), gluon dynamics (7/11, linear E(r)), budget equation (0.2% at r=6). LATENCY FIX unlocked GR: time dilation 0.004% match, BH gravitational wells L_peak=0.62. Three theorem papers: continuum limit -> QED, singlet from void event, N_c from topology. WASM rebuilt and deployed. 148/166 CTest passing. Scientific status: C+ -> B+.

---

## Project Structure

```
ftd/                                     # Project root
├── docs/
│   ├── SPEC_FTD.md              # THE authoritative FTD specification (single source of truth)
│   ├── theory/                   # 115 core theory documents (10 categories)
│   │   ├── META_INDEX.md         # Complete catalog
│   │   ├── 01_reference/         # Master references and proofs
│   │   ├── 02_foundations/       # Ontological emergence
│   │   ├── 03_derivations/       # Core physics derivations
│   │   ├── 04_coupling/          # Coupling constants
│   │   ├── 05_particles/         # Particle physics
│   │   ├── 06_consciousness/     # Consciousness and measurement
│   │   ├── 07_assessment/        # Epistemic audits
│   │   ├── 08_structural/        # Geometry and information theory
│   │   ├── 09_mathematical/      # Number theory and connections
│   │   └── archive/              # Superseded/historical documents
│   ├── reference/                # REF_EPISTEMIC_LABELS, REF_SYMBOL_GLOSSARY, etc.
│   ├── papers/                   # Published/submitted PDFs and TeX sources
│   └── internal/                 # Session summaries, exploration scripts
├── engine/                       # C++ simulation engine (v2.14)
│   ├── SPEC_ENGINE.md            # Engine reference document
│   ├── include/ftd/              # 29 headers (ontic.h, voxel.h, lattice.h, scenarios.h, etc.)
│   ├── src/                      # 14 source files
│   ├── tests/                    # 169+ test files (120 unit + 49 campaign + 4 GPU)
│   ├── cuda/                     # GPU acceleration
│   ├── wasm/                     # Emscripten bindings
│   └── web/                      # Browser dashboard (Three.js, modular JS)
├── scripts/                      # ALL Python scripts (~149 scripts)
│   ├── constants.py              # Canonical shared constants (single source of truth)
│   ├── verification/             # Formal derivation verification (40 scripts)
│   ├── proofs/                   # Formal mathematical proofs with error bounds (57 scripts)
│   ├── experiments/              # Bell tests, CERN analysis, physics sims (17 scripts)
│   ├── exploration/              # Focused research investigations (25+ scripts)
│   ├── tests/                    # Python test suites — pytest (11 scripts)
│   │   └── comprehensive/        # 7-tier verification framework
│   ├── visualization/            # Publication figure generation (11 scripts)
│   └── runners/                  # Test protocol runners (2 scripts)
├── evaluation/                   # Multi-domain assessment & certification
├── dissemination/                # All publication/outreach content
│   ├── manuscript/               # 96-chapter Quarto book (src/ + media/images/)
│   ├── manuscript_v2/            # 83-chapter physicist-targeted rewrite
│   ├── whitepaper/               # LaTeX whitepaper + figures
│   ├── book/                     # "The Golden Thread" narrative (53 .qmd files)
│   ├── notebooks/                # 12 Jupyter pedagogy notebooks
│   └── interactive/              # 8+ standalone HTML simulations (forces, photon, Hamiltonian bridge, ternary cube)
├── models/                       # Physics derivation package (gitignored)
├── archive/                      # Curated historical record (gitignored; see docs/theory/archive/ for archived theory docs)
├── META_DOCUMENTATION_MAP.md     # Master catalog / card catalog
└── META_PROJECT_ATLAS.md         # AI agent navigation guide
```

---

## C++ Engine

**Build**: `cmake -S engine -B engine/build && cmake --build engine/build --config Release`
**Test**: `cd engine/build && ctest --output-on-failure -C Release`
**WASM**: `engine\build_wasm.bat` (Windows wrapper; runs emcmake/emmake + deploys to `engine/web/wasm/`). Manual: `emcmake cmake -S engine -B engine/build_wasm -DCMAKE_BUILD_TYPE=Release && emmake cmake --build engine/build_wasm --target ftd_wasm`
**Web UI**: `python engine/web/serve.py 8080` (no-cache dev server — emits `Cache-Control: no-store` on every response so JS edits hit the browser without manual hard-refresh). Plain fallback: `python -m http.server 8080 -d engine/web` (caches aggressively; expect to bounce + hard-refresh after edits).

### Key Constants (all derived from D=3 + varpi via `ontic.h`)

| Constant | Value | Origin |
|----------|-------|--------|
| G* (lemniscatic) | 2.95868... | Γ(1/4)/Γ(3/4) |
| α (fine structure) | 1/137.036 | Master quadratic x₊ |
| N_c (colors) | 3 | Master quadratic x₋ |
| K_B (manifestation) | 0.511 | m_e = m_P·√(2π)·(16/3)·α¹¹ |
| C_SPEED | 1/√3 | CFL stability on cubic lattice |
| G_N (gravity) | 0.01 | 1/(b₃+N_c)² |

### Engine Philosophy

Logic-first: only 6 rules derived from axioms. All phenomenological features are toggle-gated extensions (default OFF).

**Tick cycle:** phase_read → phase_write → gauss_project → phase_forces → phase_movement → tick++

---

## Key Navigation Documents

- **Full FTD spec**: `docs/SPEC_FTD.md`
- **Algebraic spine (canonical theorems-only reference, 2026-04-27)**: `docs/theory/01_reference/SPEC_ALGEBRAIC_SPINE.md` — citation target for paper drafts; states the seven [THEOREM] claims (G* identity, master quadratic, CM uniqueness, coefficient 16, Watson identity, Phase G geometric Coulomb, Phase J ultralocality) independent of any physics interpretation. Read this before claiming anything load-bearing about FTD's algebraic content.
- **Engine spec**: `engine/SPEC_ENGINE.md`
- **Theory catalog**: `docs/theory/META_INDEX.md`
- **Documentation map**: `META_DOCUMENTATION_MAP.md`
- **Epistemic audit**: `docs/theory/07_assessment/AUDIT_EPISTEMIC_AUDIT.md`
- **Parametric insertions catalog** (April 19, 2026): `docs/theory/07_assessment/CATALOG_PARAMETRIC_INSERTIONS.md` (~162 rows enumerated: ~23 [DERIVED]/[THEOREM], ~129 [PARAMETRIC], ~10 [IMPOSED]/[SELECTION])
- **EFT Recovery Program** (April 19, 2026, COMPLETE Phase 0 → F + Phase G reframe): `docs/theory/10_eft_program/SPEC_EFT_RECOVERY_PROGRAM.md` — pre-registered 7-phase campaign. **Phase-F measurement:** a lattice-α plateau at ~1.8× α_ref (classical convention; 3.6× under engine-internal energy convention) across L ∈ {64, 128, 256, 384} GPU scan. **Phase-G reframe (2026-04-19):** the plateau is the zero-free-parameter periodic lattice Poisson Green's function `α_r(r, L) = 2 · r · G_L(r)`; R² = 1.0000 at L=384, median 0.07% residual in the Coulomb tail. **This is not a QED deviation** — it is lattice geometry with zero fine-structure content. See `AUDIT_ALPHA_EXTRACTION.md` and `DERIV_EMERGENT_COULOMB_GEOMETRIC.md`. Day-2 interim "1.23×" claim **RETRACTED** (ticks=100 under-equilibrated). Day-2 shipped: matched-stencil CG Poisson (Ward floor 1% → 1e-8), EWSB sharp first-order transition at amp ∈ (0.6, 0.7), condensate m ≈ 0.18 (flux/charge channels agree 3%), Rutherford α = 0.042 ± 0.005 independent cross-check. WSL2 + CUDA 13 path unblocks RTX 5090 (30× speedup). Pipeline<Backend> architecture with CPU/GPU parity. Paper: `dissemination/papers/PAPER_FTD_AS_WILSONIAN_EFT.tex`. Day-2 doc: `docs/theory/10_eft_program/DERIV_DAY2_CAMPAIGN.md`. Plan: `C:\Users\cpaci\.claude\plans\vivid-marinating-pudding.md`
- **Engine callstack audit**: `docs/theory/07_assessment/AUDIT_ENGINE_CALLSTACK.md` (CPU/GPU parity, toggle gaps, 10 findings)
- **Open items tracker**: `docs/theory/07_assessment/TRACKER_OPEN_ITEMS.md` (every `[OPEN]` across code + theory, one place)
- **Infinity reframe** (April 19, 2026): `docs/theory/07_assessment/AUDIT_INFINITY_REFRAME.md` — undefined-boundary ontology triage; foundational replacement for completed-infinity ℤ³ framing
- **a_phys open problem** (April 19, 2026): `docs/theory/10_eft_program/OPEN_A_PHYS_DERIVATION.md` — load-bearing problem the reframe creates: derive `a_phys` (lattice→physical length) from Axiom-Zero invariants or declare it empirical. Three derivation candidates analysed
- **Mechanism γ attempt** (April 19, 2026): `docs/theory/10_eft_program/DERIV_A_PHYS_MECHANISM_GAMMA_ATTEMPT.md` — gravitational `a_phys` derivation attempted and **closed as candidate** (negative result; recommendation: declare `a_phys ≡ ℓ_P` in `SPEC_FTD.md`)
- **Master quadratic (rewritten)** (April 19, 2026): `docs/theory/03_derivations/DERIV_MASTER_QUADRATIC_GAP_EQUATION.md` — full rewrite as algebraic identity + physical match (CM-curve uniqueness + dual match); gap-equation/thermodynamic-limit narrative withdrawn
- **Reframe deployment package** (April 19, 2026): `docs/theory/07_assessment/reframe_deployment/` — `CANONICAL_REFRAME.md` (single source of truth for what the reframe means; agent-facing) + `DEPLOYMENT_GUIDE.md` (7-phase plan) + `agents/` (9 agent prompts) + `templates/` + `checklists/`. Read CANONICAL_REFRAME.md before any reframe-related work
- **Master claim ledger** (April 19, 2026; extended April 20): `docs/theory/07_assessment/LEDGER.md` — 52 load-bearing claims with tag history, dependencies, reframe status. **Single source of truth for claim status** — papers cite tags from here; if they disagree, the ledger wins. Rows FTD-0050/0051/0052 (Link 8 closure + Langevin infrastructure + deferred s-Metropolis) added 2026-04-20.
- **Link 8 closure audit** (April 20, 2026): `docs/theory/10_eft_program/AUDIT_LINK8_CLOSURE.md` — full closure report on "master quadratic as RG-step characteristic polynomial" hypothesis. Three independent tests (Kadanoff blocking, Watson-integral analytical, thermalized |J|² correlator) all NEGATIVE for structurally consistent reasons (engine stencil is (SC+FCC)/2, BCC-orthogonal; master quadratic lives on BCC Watson integral). FTD-0001/0013/0014 UNAFFECTED.
- **Langevin thermostat** (April 20, 2026): `engine/src/render_bridge.cpp` + `engine/include/ftd/term_toggles.h` — CPU single-substrate OU update on wave_vel, toggle-gated. Validated by `engine/tests/test_langevin_equipartition.cpp` (equipartition to 4%). Unblocks non-zero-T matched-stencil β, condensate ensemble measurements, fluctuation-dissipation tests.
- **Reframe changelog** (April 19, 2026, append-only): `docs/theory/07_assessment/CHANGELOG_REFRAME.md` — every decision and change made under the reframe deployment
- **Devil's advocate report** (April 19, 2026): `docs/theory/07_assessment/archive_session_outputs/DEVILS_ADVOCATE_REPORT.md` — falsification pass on 6 substantive rewrites; 3 blocking bugs found and fixed same-day, 5 PASS-WITH-NOTES queued
- **Engine reframe audit** (April 19, 2026): `docs/theory/07_assessment/archive_session_outputs/ENGINE_AUDIT_REFRAME.md` — C++/CUDA/JS sweep for completed-infinity + hidden α; 3 HIGH (2 fixed same-day, 1 deferred for owner: `α_inf` rename across CSV/Python/TeX), 6 MEDIUM, 9 LOW. Parameter-free claim status: CONDITIONAL
- **Portfolio inventory** (April 19, 2026): `docs/theory/07_assessment/archive_session_outputs/INVENTORY_PORTFOLIO.md` — 280 artifacts cataloged outside `docs/theory/`; 267 editable, 13 PDF-only; manuscript_v1↔v2 share ~57 chapters that must be propagated together
- **Paper classification** (April 19, 2026): `docs/theory/07_assessment/archive_session_outputs/FLAGGED_PASSAGES_PAPERS.md` — 34 TeX/MD papers in `docs/papers/` classified; 10 clean, ~37 proscribed passages in 7 files. Top-7 priority list inside
- **Session wrapup** (April 19, 2026, evening): `docs/theory/07_assessment/SESSION_WRAPUP_2026_04_19.md` — **read first when resuming reframe work.** Lists what ran in your absence, what same-day fixes landed, and 7 decisions awaiting owner sign-off
- **PDF-only papers tracker** (April 19, 2026): `docs/theory/07_assessment/archive_session_outputs/TRACKER_PDF_ONLY_PAPERS.md` — 13 papers without TeX source; 2 HIGHEST priority (`FTD_Thermodynamic_Limit`, `DERIV_THERMODYNAMIC_REFLEXION`); recovery options + recommended action sequence
- **YM/NS RE-DERIVE assessment** (April 19, 2026): `docs/theory/07_assessment/archive_session_outputs/REDERIVE_REPORT_YM_NS.md` — both speculative Clay-aimed papers lose post-reframe; YM has 1 surviving theorem (per-voxel mass gap), NS has none; SPLIT/DEMOTE/RETRACT options laid out per paper
- **Manuscript propagation rule** (April 19, 2026): `dissemination/manuscript_v2/PROPAGATION_RULE.md` — authoritative rule for v1↔v2↔vol1↔vol2 chapter editing. **Mandatory before any chapter edit** (vol1/vol2 are NOT symlinks; already diverged)
- **a_phys ≡ ℓ_P calibration** (April 19, 2026, declared in SPEC_FTD.md): one voxel ≡ Planck length; one tick ≡ √3·ℓ_P/c ≈ 9.34×10⁻⁴⁴ s; mass-unit ≡ m_e/K_B = 1 MeV/c². **Every dimensional FTD prediction is conditional on this calibration; dimensionless predictions (α, mass ratios, mixing angles) are calibration-independent and constitute the falsifiable spine.**
- **Changelog (engine + project)**: `CHANGELOG.md`
- **Complete SM**: `scripts/proofs/proof_complete_sm.py`
- **Motivic proof**: `scripts/proofs/proof_motivic_master_quadratic.py`
- **Moore Layer Theorem**: `docs/theory/08_structural/THEOREM_MOORE_LAYER_DECOMPOSITION.md`
- **Phase lattice**: `docs/theory/08_structural/EXPLR_PHASE_LATTICE_MOORE.md`
- **50-test battery**: `scripts/exploration/test_all_physics.py`
- **Loop derivations**: `scripts/exploration/compute_c2.py`, `derive_all_loops.py`, `gauge_loops.py`
- **Arrow paper**: `docs/papers/PAPER_RATIO_AND_THE_ARROW.tex`
- **Engine coupling test**: `engine/tests/test_intervoxel_coupling.cpp`
- **Complete Chain** (April 2026): `docs/theory/01_reference/SPEC_FTD_COMPLETE_CHAIN.md`
- **QM as Statistics** (April 2026): `docs/theory/03_derivations/DERIV_QM_FROM_LATTICE.md`
- **Lattice Physics Reference** (April 2026): `docs/theory/02_foundations/FOUND_LATTICE_PHYSICS_INTUITIONS.md`
- **Stellar Lifecycle** (April 2026): `docs/theory/03_derivations/DERIV_STELLAR_LIFECYCLE_LATTICE.md`
- **Master Verification** (April 2026): `scripts/proofs/proof_master_verification.py` (54/54 checks)
- **BCC Unification** (April 2026): `docs/theory/08_structural/DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md`
- **Observer Formalism** (April 2026): `docs/theory/02_foundations/FOUND_POTENTIAL_CORE_AND_GENERATIVE_INTERIOR.md` (Part II: 3³ lattice grounding)
- **Manuscript v2**: `dissemination/manuscript_v2/CHECKLIST.md` (83 chapters, physicist-targeted)
- **Engine-Theory Bridge** (April 13, 2026): `engine/tests/benchmark_engine_theory.cpp` (20 benchmarks)
- **Emergent Alpha** (April 13, 2026): `engine/tests/benchmark_emergent_alpha.cpp` (6 EFT experiments)
- **Benchmark Harness**: `scripts/benchmarks/benchmark_engine_vs_theory.py` (Python analysis)
- **Convergence Analysis**: `scripts/benchmarks/analyze_convergence.py` (20-benchmark report + plots)
- **Benchmark Results**: `scripts/benchmarks/results/` (reports, plots, CSV)
- **Wilson Loops** (April 13, 2026): `engine/tests/benchmark_wilson_loops.cpp` (12/17 pass, flux tube detected)
- **Gluon Dynamics** (April 13, 2026): `engine/tests/campaign_gluon_dynamics.cpp` (7/11 pass, linear E(r))
- **Einstein Equations** (April 13, 2026): `engine/tests/test_einstein_equations.cpp` (time dilation 0.004% match after latency fix)
- **BH Thermodynamics** (April 13, 2026): `engine/tests/benchmark_black_hole_thermo.cpp` (L_peak 0.62, proper time dilation)
- **Budget Equation** (April 13, 2026): `engine/tests/benchmark_budget_equation.cpp` (x/K+G*/x=1 to 0.2%)
- **Continuum Limit -> QED** (April 13, 2026): `docs/theory/03_derivations/DERIV_CONTINUUM_LIMIT_QED_EQUIVALENCE.md` (x+ = 1/alpha conditional [THEOREM])
- **Singlet from Void** (April 13, 2026): `docs/theory/03_derivations/DERIV_SINGLET_FROM_VOID_EVENT.md` (Bell loop closed via 5 lemmas)
- **N_c from Topology** (April 13, 2026): `docs/theory/03_derivations/DERIV_NC_FROM_TOPOLOGY.md` (N_c = 3 from 4 independent routes)
- **Web refactor spec** (April 18-19, 2026): `engine/web/docs/SPEC_REFACTOR_LARGE_FILES.md` (Waves 0-3 split of viewport/wasm-bridge-dag/app_dag + RF-1/3/4/5/6/7/8/10 post-audit cleanup; Ticket 14 + RF-9-full deferred). Final: viewport 5325→3900, wasm-bridge-dag 5736→2132, app_dag 1898→1723 (−5204 LOC, −40% across three files)
- **Whole-project extraction** (April 19, 2026, v2.15.0): 16-agent parallel refactor split every file ≥500 LOC into discrete-responsibility modules. C++: render_bridge.cpp 2139→1097, constructors.cpp 1245→0 (deleted, 5 split files), scenarios.cpp 1241→79, ftd_wasm.cpp 1224→607, cosmic_engine 1193→500, atom_engine 1029→325, main.cpp 938→74, ontic.h 806→45 (+6 theme-headers), ws_server 831→496. JS: mock-scale5 1903→313, consciousness triad −1711, scale controllers −1248, backgrounds 846→178, field-overlays 976→455. Python: 3 common helpers extracted. Total: ~13800 LOC redistributed across ~97 new files, every module nameable in ≤5 words
- **Scenario library (C++)** (April 18, 2026): `engine/include/ftd/scenarios.h` + `engine/src/scenarios/{flux,light,quantum,s0_seed,s0_field}.cpp` (all 83 Scale-0 scenarios ported from JS MockBridge; 84/84 Playwright coverage, 5/5 parity CI guard)
- **Web power-user guide** (April 18-19, 2026): `engine/web/docs/USER_GUIDE.md` (15-section reference for dashboard + console workflows)
- **Scenario parity CI guard** (April 19, 2026): `engine/web/tests/scenario-parity.spec.js` (5 assertions covering JS↔C++ scenario name drift; runs in <1s)
- **Viewport extracted modules** (April 19, 2026): `engine/web/js/viewport/{color-ramps,molecular-renderer,boundary-geometry,topology-sheet-renderer}.js` (own their Three.js concerns; viewport.js keeps thin delegators)

---

## EFT Reconstruction (April 13, 2026)

Alpha is now a DERIVED quantity in the engine:
- `ALPHA_EFT = G_C * G_C` defined in `constants.h` with compile-time `static_assert`
- G_C (wave equation coupling) is the fundamental lattice parameter
- All force computations use `ALPHA_EFT` (= G_C²), not hardcoded `ALPHA`
- New toggle `emergent_forces` computes force from flux gradient without Poisson solver
- 20-benchmark suite validates: Coulomb convergence (B+), hydrogen spectrum (A+), color forces (A+), Higgs threshold (A+), Bell S=2 (A+), Born lattice bias (A-), 139/179 CTest passing

---

## Naming Conventions

- Markdown files: `UPPER_SNAKE_CASE` with semantic prefix
- Prefixes: `SPEC_` (specifications), `DERIV_` (derivations), `FOUND_` (foundations), `AUDIT_` (assessment), `EXPLR_` (exploratory), `REF_` (reference), `ARCH_` (archived), `META_` (meta-documentation)
- Engine: C++17, snake_case functions, CamelCase types

---

## Environment Notes

- Platform: Windows 11. No `rsync` — use `cp -r` for directory copies
- **GPU execution MUST go through WSL2 Ubuntu-22.04, not Windows-native CUDA.** RTX 5090 speedup (~30×) is only available via the WSL2 build at `engine/build_wsl`. Windows-native CUDA builds from `engine/build/` technically run but are pathologically slow (observed 19 minutes wall for a single L=64 density=0.1 seed). Invocation pattern:
  ```
  wsl.exe -d Ubuntu-22.04 -- bash -c "cd /mnt/c/Users/cpaci/Desktop/ftd && \
      engine/build_wsl/benchmark_foo --args"
  ```
  Windows-native CUDA is acceptable for compile-time checks and single-tick correctness tests only. Any measurement campaign, sweep, or multi-seed run goes through WSL2.
- Python tests: `scripts/tests/` (pytest). C++ tests: `engine/tests/` (CTest). No overlap between them
- `scripts/constants.py` is the canonical shared constants module imported by 20+ scripts
- Build `.bat` files live in `engine/` — use `vswhere.exe` for portable VS detection
- `dissemination/media/`, `models/`, and `archive/` are gitignored — they exist on disk but not in git
- `docs/internal/` is gitignored — session summaries and explorations are local-only
