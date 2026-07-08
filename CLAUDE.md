# Foundational Ternary Dynamics (FTD) — Project Instructions

**These are the project working instructions.** Foundational Ternary Dynamics (FTD) is a discrete, finite, deterministic ternary-lattice ontology, the mathematics it forces, and the physics that mathematics suggests; the project's aim is stated in the Number-One Goal below.

Canonical status lives in three places, in this precedence — **LEDGER > constitution > all other prose**: the **LEDGER** ([`docs/theory/07_assessment/core_ledgers/LEDGER.md`](docs/theory/07_assessment/core_ledgers/LEDGER.md)) carries per-claim tags and provenance; the **constitution** ([`docs/theory/01_reference/SPEC_FTD_FRAMEWORK_V1.md`](docs/theory/01_reference/SPEC_FTD_FRAMEWORK_V1.md)) declares the Postulates / Framework Commitments / Calibrations; the **bedrock tracker** ([`docs/theory/07_assessment/core_ledgers/TRACKER_ONTIC_TRUTH.md`](docs/theory/07_assessment/core_ledgers/TRACKER_ONTIC_TRUTH.md)) ranks rock-solid vs conjectural; and the **algebraic spine** ([`docs/theory/01_reference/SPEC_ALGEBRAIC_SPINE.md`](docs/theory/01_reference/SPEC_ALGEBRAIC_SPINE.md)) holds the theorem-grade core. Project history lives in `CHANGELOG.md` and git, not in these pages.

**Full specification:** [`docs/SPEC_FTD.md`](docs/SPEC_FTD.md)
**Start here:** [`docs/WHERE_WE_LEFT_OFF.md`](docs/WHERE_WE_LEFT_OFF.md) — current-state pointer (what the framework is, where status lives, where the boundary is).
**Architecture navigation:** [`META_PROJECT_ATLAS.md`](META_PROJECT_ATLAS.md) — task→file table, directory tree, subsystem dependency graph.
**Cross-module contracts:** [`CONTRACTS.md`](CONTRACTS.md) — bridge state, capability factories, scale context, scenarios, toggles, energy convention, constants chain, telemetry, and the golden-tick gate.
**Architectural decisions:** [`docs/adr/INDEX.md`](docs/adr/INDEX.md) — the ADRs governing engine patterns.
**Audit ledgers:** [`docs/audits/INDEX.md`](docs/audits/INDEX.md) — sweep and audit ledgers.

---

## Number-One Goal

> **Set the smallest honest set of types from which a discrete ontology can speak; build the mathematics and physics forward as the content those types make meaningful; and rigorously mark and price which types the ontology sets for itself and which it must import.**

This is the project's single north star, and it rests on one principle — **type-priority** (`02_foundations/FOUND_TYPE_PRIORITY_PRINCIPLE.md`, FTD-0339): a *context* (a type) is prior to, and the precondition for, the *value* of any *content* (a token); content cannot bootstrap context. The goal has two inseparable faces. To **derive** is to build the content a set of types makes meaningful — a strict chain from the postulates to a `[THEOREM]` or `[DERIVED]` result. To **establish the boundary** is to mark — and to *price* — rigorously, which types the discrete ontology cannot set for itself and must instead import: to name each imported type, count it in a common currency, and attach its falsifier (the priced-import ledger, `01_reference/SPEC_IMPORT_LEDGER.md`, FTD-0371). The algebraic spine, the engine, the pre-registered tests, the LEDGER, and the manuscripts each serve this goal or are subordinate to it.

At its honest altitude FTD is **a philosophy-of-mathematics project with a rigorous algebraic core and suggestive — not derived — physics connections**, ordered **Ontology > Logic > Math > Physics**: the discrete ontology sets the types, mathematics is built forward as their meaningful content, and physics is a *constraint*, not the sole arbiter. The load-bearing, theorem-grade part is the algebraic spine (G\*, the master quadratic, the Watson/Chowla–Selberg identities, CM-uniqueness, the D=3 *arithmetic* uniqueness — its dimension-forcing is `[SELECTION — declared]`, FTD-0355); physics identifications ride at their actual LEDGER status and are never promoted by rhetorical momentum.

Read precisely:

- **"A discrete ontology"** — FTD's five postulates: a finite, undefined-boundary lattice (no completed infinity, no primitive continuum); discrete time; ternary states {−1, 0, +1}; local Moore-neighbour causality; determinism. These, with the Framework Commitments (FC-0…FC-W), are the **types** — the chosen context, *set first*, that the dynamics presuppose. The ternary cubic lattice is the current concrete model; discreteness and finiteness are non-negotiable.
- **"Set the types"** — the Framework Commitments are **precondition-types: adopted, not derived** (a context cannot be derived from its content). Adopting the *smallest honest set* of them is the discipline; each is a declared `[AXIOM]`-class commitment with a stated falsification criterion, never a smuggled derivation.
- **"Build the mathematics forward"** — a strict, explicit chain from the postulates and set types to the result: `[THEOREM]` or `[DERIVED]`. A `[PARAMETRIC]` insertion (a standard physics formula filled with FTD numbers) is **not** a derivation; a `[STRONGLY MOTIVATED CONJECTURE]` is a *match*, not a derivation. A claim's LEDGER tag is the measure of whether it serves the goal.
- **"Mark and price which types must be imported"** — the boundary is itself a deliverable, and FTD's sharpest. Rigorously showing that the ontology **cannot set** a given type — that it must be imported — is as much a result as a derivation. The canonical statement is the modulus/argument frontier (`02_foundations/FOUND_MODULUS_ARGUMENT_FRONTIER.md`, FTD-0336): the substrate natively sets the *forced/modulus* types and must import the *chosen/argument* ones. **α is the worked example, not the goal** — its value is an imported type (the chosen adjoint δ, FC-W), exactly the status physics itself gives α (no theory derives it). Closed-negatives map how far the ontology's own type-setting reaches. Where FTD-0336 draws that boundary *qualitatively*, the priced-import ledger (`01_reference/SPEC_IMPORT_LEDGER.md`, FTD-0371 `[SYNTHESIS]`) draws it *quantitatively* — every imported type counted in a common currency (adopted bits, selected types, named results, calibrations, empirical identifications), each line carrying its own falsifier, with the substrate's self-set credits and its two declined bets (M, reversibility) booked alongside. Pricing the boundary this way — auditable, per-line falsifiable, tag-preserving — is itself a deliverable of this face; it moves no tag (`x₊=1/α` stays `[SMC]`, FC-W stays `[AXIOM]`, the calibrations stay `[IMPOSED]`) and reclassifies nothing as derived.

**G\* / ℚ(G\*) is a lever, not the north star** — an acknowledged-but-underexploited mathematical structure that an ontology-first, type-first construction forces into centrality; its orphaned status in mainstream mathematics is a clue worth pursuing, not a claim of discovery.

**Operational test** for any claim, paper, or experiment: does it *set a type* honestly (a declared, minimal, falsifiable commitment), *build content forward* from the set types (`[THEOREM]`/`[DERIVED]`), or *mark and price a boundary* (a type the ontology must import, counted in the ledger's currency with its falsifier) — or is it a match/import still awaiting one of those verdicts? The Epistemic Discipline rules and tag system (below) are *how* the goal is pursued; this goal is *what* is pursued. The standing discipline admits no exceptions: `x₊=1/α` is a `[STRONGLY MOTIVATED CONJECTURE]`, MC-T4.3 is a `[FOUNDATIONAL OBSTRUCTION]`, and the type-priority principle that anchors this goal is itself an *adopted commitment open to external critique*, not a theorem.

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
| **[STRONGLY MOTIVATED CONJECTURE]** | [CONJECTURE] with substantial structural and/or empirical evidence (e.g. structural-uniqueness scans, multi-route convergence, sub-ppm empirical match) but no derivation chain | Critique evidence; expect explicit Bayes-factor, uniqueness, or look-elsewhere argument |
| **[PARAMETRIC]** | Standard physics formula filled with FTD constants; numbers fit but mechanism is borrowed | Treat as calibration input, not output |
| **[SYNTHESIS]** | Cross-document integration of multiple lower-level claims into a single externally-defensible package; not a new theorem but a coherent re-statement of existing claims at their canonical tags | Verify component claims; check that synthesis does not silently promote tags |
| **[CLOSED NEGATIVE]** | Hypothesis was tested and falsified; preserved for provenance to prevent re-attempt | Confirm closure evidence; cite to prevent zombie re-emergence |
| **[DERIVED]** | Established from axioms or prior theorems by an explicit chain that the doc itself reproduces; weaker than [THEOREM] when the chain has non-trivial assumptions | Check the chain; flag any smuggled axioms |

---

## Documentation Cleanup Discipline

> **These rules are mandatory for AI cleanup work.** The goal is persistent consolidation, not one-off tidying that creates future drift.

- **Preserve provenance; move, do not erase.** Superseded, retracted, resolved, and closed-negative theory documents should be archived with `git mv`, not deleted, unless the user explicitly asks for deletion.
- **Keep active directories active.** Documents whose live status is `[CLOSED NEGATIVE]`, `[RETRACTED]`, or `[CLOSED -- RESOLVED]` should live under `docs/theory/archive/` or a local archive such as `docs/theory/10_eft_program/archive/{closed_negative,resolved,retracted}/`.
- **Track cleanup provenance deliberately.** Theory archives used as canonical cleanup provenance must be tracked in git. The top-level `archive/` directory remains ignored. Local cleanup archives such as `docs/theory/10_eft_program/archive/**` are tracked wholesale; the broad `docs/theory/archive/` directory uses explicit `.gitignore` exceptions, so add a matching exception whenever a new canonical top-level archived file is introduced.
- **Update all navigation layers in the same cleanup.** If a file is moved or status-changed, update the relevant index/tracker/spec references in the same commit: `docs/theory/META_INDEX.md`, local sub-indexes such as `docs/theory/10_eft_program/INDEX_FTD_NATIVE_EFT.md`, `docs/theory/07_assessment/core_ledgers/LEDGER.md`, `TRACKER_OPEN_ITEMS.md`, and any project-level maps that link to the file.
- **Open trackers must contain open work.** Do not leave closed, retracted, or resolved items counted as `[OPEN]`. Either remove them from `TRACKER_OPEN_ITEMS.md`, move them to a resolved/provenance tracker, or clearly mark them as "not counted as open" until a resolved tracker exists.
- **Do not promote claims during cleanup.** Cleanup may clarify status, archive provenance, and align links; it must not upgrade epistemic tags or introduce new derivations without a separate audit.
- **Verify before committing.** At minimum run `git diff --check` and `rg` for old active paths after any move. Use documentation/link checks only; do not run numerical near-miss or coincidence searches as part of cleanup.
- **Commit cleanup in small coherent batches.** Prefer one commit per cleanup theme (archive tracking, tracker split, index reconciliation, sector consolidation) so future agents can audit the history.

---

## What FTD Is

A discrete computational framework for simulating physical systems from explicit postulates. The model postulates a 3D cubic lattice where each site ("voxel") occupies one of three states: void (0), positive (+1), or negative (−1). Dynamics proceed via local update rules within a 26-connected Moore neighborhood, with information propagating at maximum one lattice unit per discrete time step.

**Two-layer ontology:**
- **Flux field** J ∈ ℝ³ — continuous vector field encoding potential energy density (dispositional)
- **State field** s ∈ {−1, 0, +1} — discrete ternary states representing manifestation (actual)

**Five postulates:** Discrete space (3D cubic lattice, no defined boundary — at every specified position, axis-adjacent sites exist; **not** a completed-infinity ℤ³ totality, per `AUDIT_INFINITY_REFRAME.md`), discrete time (ticks), ternary states, local causality (26-neighbor Moore), determinism.

**Foundational commitment:** FTD uses **undefined-boundary** lattice ontology, not completed-infinity. Arbitrarily large finite computations are permitted; claims of the form "in the L → ∞ limit" are not well-posed without explicit ε-L restatement. See `docs/theory/07_assessment/AUDIT_INFINITY_REFRAME.md` for the full triage of which claims survive, which need restatement, and which need re-derivation.

**Key results** (within framework assumptions):
- Fine structure constant: master quadratic `x² − 16G*²x + 16G*³ = 0` has x₊ = 137.036 matching 1/α to **1.26 ppm** at tree level (pure algebra [THEOREM]; physical identification of `x₊  1/α` [STRONGLY MOTIVATED CONJECTURE] per `AUDIT_MASTER_QUADRATIC.md`; the structural-uniqueness evidence is the FTD-0319 adversarial look-elsewhere scan (distinct from FTD-0189, the graviton-audit id) — the master quadratic is the unique dual-matcher across 2.65M degree-2 polynomials over an 18-constant basket FTD did not design; **caveat: a `[NUMERICAL FACT]`, not a structural Bayes result — uniqueness is asymmetric-tolerance-conditioned and the "~4×10⁵:1 Bayes" is unsupported by the runner, ~19× scan-size only**). The polynomial's smaller root `x₋ ≈ 3.024` is a mathematical artifact of the quadratic; the **identification `x₋  N_c` is RETIRED** per FTD/FQCR Cleanup Taxonomy v1.4 §5 (FTD-0014 was removed from the LEDGER in commit `ca7eb61`). `N_c = 3` comes from independent structural sources — see the Moore Layer Theorem and `DERIV_NC_FROM_TOPOLOGY.md` below. The 7-term series matching CODATA to 24 digits is a post-hoc fit [CONJECTURE] beyond experimental precision (CODATA 2022 has ~11 digits), not a "< 0.001 ppt derivation"
- Loop coefficients c1–c3 derived from lattice Feynman diagrams: c1 = 9/47 (0.8%), c2 = 5/64 via gauge factor 13/9 (0.07%), c3 = 4/141 via gauge factor 11/6 (0.33%)
- Electron mass m_e = m_P √(2π) (16/3) α¹¹ (0.19% error)
- Higgs mass m_H = (N_eff/α²)·m_e = 124.75 GeV — **−0.36% vs PDG 2024's 125.20 ± 0.11 GeV, a −4.1σ discrepancy at current precision** (corrected 2026-07-01, FTD-0348: the earlier "0.24% error" reproduced only against the superseded PDG-2020 value 125.10; at PDG-2024 precision the exact relation is experimentally excluded), λ_H = m_H²/(2v²)
- Proton mass m_p/m_e = N_eff/α + N_base·N_eff + N_c = 1836.47 (174 ppm)
- Electron g-2: a_e = α/(2π) to 5-loop = 2.55 ppb — **[PARAMETRIC]** (added 2026-07-01, matching the Lamb-shift line's convention below): standard multi-loop QED with FTD's α inserted; the loop functional form is imported, only α is FTD's — genuine evidence for α's *value*, not a substrate derivation of g-2
- Lamb shift: 1055.4 MHz (0.23% from experiment) — **[PARAMETRIC]**: standard QED one-loop (Mohr + Uehling) with FTD's α inserted, NOT a substrate derivation (see `AUDIT_ATOMIC_DYNAMICS_STATUS.md`)
- Color charge number N_c = 3 from RG flow + topological quantization
- **Moore Layer Theorem**: gauge groups U(1)×SU(2)×SU(3), 3 generations of 4 fermions, matter-antimatter symmetry, 17 dark states — all from Moore neighborhood polyhedral decomposition (octahedron + cuboctahedron + stella octangula)
- BCC multiplicative structure: Watson identity W₃ = G*²/(2π) and SU(3) gauge group both arise from the BCC eigenvalue's triple cosine product (docs/theory/08_structural/DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md)
- Confinement from area-law Wilson loops at x₋ (σ = 0.209)
- Bell correlations: S = 2√2 (Tsirelson) is **imported standard QM conditional on the [SELECTION] singlet** (the J→ψ complexification is not forced by the lattice axioms) — "FTD does not violate Bell; FTD produces the singlet, QM handles the rest" (`DERIV_SINGLET_FROM_VOID_EVENT.md`); the substrate itself is local/classical, S ≤ 2 natively (a native S > 2 is an FC-1 **falsifier**, not a prediction)
- Full nonlinear Einstein equations via Deser iterative bootstrap — **[Step-0 correction, FTD-0189]** the bootstrap *completes* a posited massless spin-2 field, it does not derive one; its linearized-EFE input is conditional on Conjecture 10.1 (h_μν posited, not substrate-constructed; spin-2 spatial part is Gap 10.1). Whether the substrate carries an emergent spin-2 mode is [OPEN] — Frontier 4
- D = 3: **[SELECTION — declared]**, not forced (FTD-0355 permanent verdict; the earlier "uniquely selected / no longer axiomatic" forcing claim was demoted — bounded search, circularity named)
- Cyclotomic structure: Hamiltonian parameters are Phi_4, Phi_1·Phi_2, Phi_6 evaluated at sqrt(pi)
- The Ratio and the Arrow: Euler reflection product (commutative, gives pi, time-symmetric) vs ratio (non-commutative, gives G*, time-asymmetric)
- 50 physics predictions tested across three tiers: `scripts/exploration/test_all_physics.py` — **clarified 2026-07-01**: this verifies that ~20 [PARAMETRIC]/[SELECTION] integer-ratio insertions reproduce the values they were fit to, i.e. an internal-consistency check across the catalog, not 50 independently-confirmed predictions
- Complete Standard Model computation: `scripts/proofs/proof_complete_sm.py`

**Honest accounting:** ~21 derived/theorem-grade claims (corrected 2026-07-01, FTD-0348 — the μ/τ mass-ratio rows moved to [STRUCTURALLY MOTIVATED PARAMETRIC] per their demotion of record), ~131 parametric insertions (FTD values in standard QFT formulas), ~10 imposed/selected, ~50+ external physics adopted. 50 physics tests pass across three tiers (see clarification above — reproduction-of-fit, not independent prediction). Test suite: 255/255 Python tests pass, 54/54 master verification pass, 211/211 CTest pass. See [EPISTEMIC_AUDIT.md](docs/theory/07_assessment/AUDIT_EPISTEMIC_AUDIT.md) and [CATALOG_PARAMETRIC_INSERTIONS.md](docs/theory/07_assessment/CATALOG_PARAMETRIC_INSERTIONS.md).

**Engine-theory bridge:** 20-benchmark suite connects engine output to theory. Coulomb 1/r^2 converges (B+), hydrogen 1/n^2 < 0.001% (A+; **classical Kepler check, NOT a quantum eigenvalue derivation** — generic to any 1/r force; see `AUDIT_ATOMIC_DYNAMICS_STATUS.md`), color forces correct (A+), Higgs threshold exact (A+), Bell S=2.000 (**N/A as a QM grade, re-graded 2026-07-01** — this is the *classical* local-hidden-variable bound, measured by a standalone LHV toy that never touches the lattice engine; it confirms the substrate is local/classical, the opposite of a QM confirmation), Born lattice bias 10x (A-). EFT reconstruction: alpha = G_C^2 (an algebraic identity by construction — a consistency check, not a derivation of alpha; see `constants.h` HONEST FRAMING note). Added Wilson loops (12/17, flux tube detected), gluon dynamics (7/11, linear E(r)), budget equation (0.2% at r=6). LATENCY FIX unlocked GR: time dilation 0.004% match, BH gravitational wells L_peak=0.62. Three theorem papers: continuum limit -> QED, singlet from void event, N_c from topology. WASM rebuilt and deployed. 211/211 CTest passing. Scientific status: C+ -> B+.

---

## Project Structure

```
ftd/                                     # Project root
├── docs/
│   ├── SPEC_FTD.md              # Framework overview (defers to the canonical hierarchy in META_STRUCTURE.md)
│   ├── theory/                   # 586 theory documents across 10 categories + 118 archived
│   │   ├── META_INDEX.md         # Curated catalog
│   │   ├── 01_reference/         # Master references, algebraic spine, construction monograph
│   │   ├── 02_foundations/       # Ontological emergence, lattice physics
│   │   ├── 03_derivations/       # Core physics derivations (QM, EM, gravity, SM sectors)
│   │   ├── 04_coupling/          # Coupling constants and precision
│   │   ├── 05_particles/         # Particle physics applications
│   │   ├── 06_reference_frames_and_measurement/ # Frame-relative-projection layer + measurement
│   │   ├── 07_assessment/        # Epistemic audits, ledgers, campaigns
│   │   ├── 08_structural/        # Moore theorem, geometry, BCC structure
│   │   ├── 09_mathematical/      # Number theory, CM curves, L-values
│   │   ├── 10_eft_program/       # Native EFT recovery program + pre-registrations
│   │   └── archive/              # 118 archived documents (provenance preserved)
│   ├── reference/                # REF_EPISTEMIC_LABELS, REF_SYMBOL_GLOSSARY, etc.
│   ├── papers/                   # 66 LaTeX source files + compiled PDFs
│   └── internal/                 # Session summaries, exploration scripts (gitignored)
├── engine/                       # C++ simulation engine (v2.18.0)
│   ├── SPEC_ENGINE.md            # Engine reference document
│   ├── include/ftd/              # 59 headers (ontic.h, voxel.h, lattice.h, scenarios.h, etc.)
│   ├── src/                      # 22 source files
│   ├── tests/                    # 298 test source files (211 active CTest targets)
│   ├── cuda/                     # GPU acceleration (WSL2 + RTX 5090)
│   ├── wasm/                     # Emscripten bindings
│   └── web/                      # Browser dashboard (Three.js, 868 JS files, 5 CSS themes)
├── scripts/                      # 520 Python scripts (139K LOC)
│   ├── constants.py              # Canonical shared constants (single source of truth)
│   ├── verification/             # Formal derivation verification (61 scripts)
│   ├── proofs/                   # Formal mathematical proofs with error bounds (143 scripts)
│   ├── experiments/              # Bell tests, CERN analysis, physics sims (19 scripts)
│   ├── exploration/              # Focused research investigations (195+ scripts)
│   ├── tests/                    # Python test suites — pytest (14 scripts)
│   │   └── comprehensive/        # 7-tier verification framework
│   ├── visualization/            # Publication figure generation (23 scripts)
│   ├── benchmarks/               # Engine vs theory benchmarks (6 scripts)
│   └── runners/                  # Test protocol runners (3 scripts)
├── evaluation/                   # Multi-domain assessment & certification
├── dissemination/                # All publication/outreach content
│   ├── whitepaper/               # LaTeX whitepaper + figures
│   ├── papers/                   # Additional compiled LaTeX papers
│   ├── interactive/              # 31 standalone HTML simulations
│   └── notebooks/                # Jupyter pedagogy notebooks
├── models/                       # Physics derivation package (gitignored)
├── archive/                      # Curated historical record (gitignored; see docs/theory/archive/ for archived theory docs)
├── META_DOCUMENTATION_MAP.md     # Master catalog / card catalog
└── META_PROJECT_ATLAS.md         # AI agent navigation guide
```

---

## C++ Engine

**Build**: `cmake -S engine -B engine/build && cmake --build engine/build --config Release --parallel 32` (Maximize CPU threads on the AMD 9950X3D)
**Test**: `cd engine/build && ctest -j 32 --output-on-failure -C Release` (Always use parallel execution to avoid sequential runs taking forever)
**WASM**: `engine\build_wasm.bat` (Windows wrapper; runs emcmake/emmake + deploys to `engine/web/wasm/`). Manual: `emcmake cmake -S engine -B engine/build_wasm -DCMAKE_BUILD_TYPE=Release && emmake cmake --build engine/build_wasm --target ftd_wasm`
**Web UI**: `python engine/web/serve.py 8080` (no-cache dev server — emits `Cache-Control: no-store` on every response so JS edits hit the browser without manual hard-refresh). Plain fallback: `python -m http.server 8080 -d engine/web` (caches aggressively; expect to bounce + hard-refresh after edits).

### Key Constants (all derived from D=3 + varpi via `ontic.h`)

| Constant | Value | Origin |
|----------|-------|--------|
| G* (lemniscatic) | 2.95868... | Γ(1/4)/Γ(3/4) |
| α (fine structure) | 1/137.036 | Master quadratic x₊ |
| N_c (colors) | 3 | Master quadratic x₋ |
| K_B (manifestation) | 0.511 | m_e = m_P·√(2π)·(16/3)·α¹¹ (current calibration: K_B = m_e mass anchor; role-conflated with engine manifestation threshold — see FTD-0130) |
| C_SPEED | 1/√3 | CFL stability on cubic lattice |
| G_N (gravity) | 0.01 | 1/(b₃+N_c)² — **falsified as identification with physical G_N** (FTD-0131); substrate derivation gives instead the gravitational fine-structure ratio for one electron: α_G(e,e) = (m_e/m_P)² = (√(2π)·(16/3)·α¹¹)² ≈ 1.745×10⁻⁴⁵ (predicted, 0.38% match to measured 1.752×10⁻⁴⁵) — derived via Phase G + FTD-0015 + **1 flagged interpretive step (clock hypothesis used in SPEC_FTD_LAGRANGIAN.md §4.3)** per the reconciliation audit; the two original postulates of DERIV_NEWTON_FROM_SUBSTRATE.md §1.2 + §1.4 are subsumed by SPEC §4.2 + §4.3 [THEOREM]s (`AUDIT_NEWTON_POSTULATES_RECONCILIATION.md`); Arc B P2 closure attempt pre-registered (`preregister-clock-hypothesis-derivation-v1`). See `docs/theory/03_derivations/DERIV_NEWTON_FROM_SUBSTRATE.md`. The "1/100" numerical coincidence has no substrate justification under any natural reading. |

### Engine Philosophy

Logic-first: only 6 rules derived from axioms. All phenomenological features are toggle-gated extensions (default OFF).

**Tick cycle (10 phases):** phase_read → phase_write → pair_production → gauss_project → latency_solve → phase_forces → phase_movement → boundary → weak/triad → proper_time

---

## Key Navigation Documents

- **Full FTD spec**: `docs/SPEC_FTD.md`
- **External comparison constants (canonical edition standard)**: `docs/reference/REF_EXTERNAL_CONSTANTS.md` — the single source for which CODATA/PDG edition every externally-measured comparison value uses (current standard: **CODATA 2022 / PDG 2024**). All future references to α⁻¹, G, ℓ_P, particle masses, etc. cite this; machine-readable mirror is `scripts/constants.py` `Experimental`. Pre-registered/hash-locked artifacts legitimately retain registration-time values (provenance, not drift).
- ** Doctrine ledger v1.2 (single-page status map, FTD-0145 [SYNTHESIS])**: `docs/theory/01_reference/SPEC_DOCTRINE_LEDGER.md` — 14 sections + non-circularity audit + compressed roadmap. Roll-up across LEDGER + TRACKER_ONTIC_TRUTH + SPEC_ALGEBRAIC_SPINE + SPEC_FQCR + CHECKLIST_MATH_COMPLETE. **Introduces no new theorems**; every claim points at a canonical source. v1.2-to-canonical tag map in §0.2. Read BEFORE planning hardening arcs or answering "what is the status of claim X" questions.
- **Priced-import ledger (FTD-0371 [SYNTHESIS])**: `docs/theory/01_reference/SPEC_IMPORT_LEDGER.md` (rendered) + `docs/theory/01_reference/import_ledger.json` (data) + `scripts/proofs/proof_import_ledger.py` (verifier 8/8). The Number-One-Goal "mark the boundary" face made **quantitative**: every imported type priced in a common currency with a falsifier each — **1 adopted bit** (FC-W / the δ branch), **3 selected types** (D=3, singlet, ℭ generator-set), **4 named results** (Chudnovsky proven; CM-h=1 / E1 / E\*/E\*\* open), **3 calibrations** (a_phys / t_phys / K_B), the **empirical bridges** (x₊=1/α [SMC] + ~131 [PARAMETRIC] + ~50 external), **2 declined** (M via FC-1, reversibility via FC-2). The import surface = the *argument* half of the modulus/argument frontier (FTD-0336); the self-set column = the *modulus* half. ⚠ **The "1 adopted bit" is the α-sector branch choice ONLY — never cite it as FTD's total physics import** (the reading guard). Cite for "what does FTD import / what's the price of claim X"; flags RF-1 (constitution §3.3 D=3 "Forced [THEOREM]" is stale vs FTD-0355 [SELECTION — declared]). Introduces no theorem; promotes nothing.
- **Ontic-truth tracker (read this FIRST before defending any FTD math claim)**: `docs/theory/07_assessment/core_ledgers/TRACKER_ONTIC_TRUTH.md` — the single canonical bedrock reference. 5 truth tiers (T1 rock-solid → T5 conjecture); each entry has unique ID `OT-N.M` and points at a verification artifact. The 10 entries that matter are listed at the bottom under "Quick reference". If LEDGER and this tracker disagree on a tier, this tracker is correct on tier-assignment. Anything below T5 (parametric insertions, selection arguments, engine measurements) is NOT in this tracker — it lives in LEDGER and CATALOG_PARAMETRIC_INSERTIONS.
- **Algebraic spine (canonical theorems-only reference)**: `docs/theory/01_reference/SPEC_ALGEBRAIC_SPINE.md` — citation target for paper drafts; states nine numbered results (G* identity, master quadratic, CM uniqueness h=1, coefficient 16, Watson identity, Phase G geometric Coulomb, Phase J ultralocality at L=2, harmonic invariant tower, Q(G*) field-theoretic) of which seven are theorem-grade and two are honestly tiered below theorem grade per §0, independent of any physics interpretation. Theorem 7 honest status: [THEOREM at L=2] + [CONJECTURE for general L]. Read this before claiming anything load-bearing about FTD's algebraic content.
- **Open math by physics sector (FTD-0146 [SYNTHESIS])**: `docs/theory/01_reference/SPEC_OPEN_MATH_BY_SECTOR.md` — sector-aligned canonical research-questions queue, 10 SM-sectors (pure-math / EM-α / EW-Higgs / QCD / flavor / gravity / QM-foundations / cosmology / engine-bridge / cross-cutting). Replaces tier-aligned `CHECKLIST_MATH_COMPLETE.md` (now archived to `docs/theory/archive/ARCH_CHECKLIST_MATH_COMPLETE.md` for provenance). Preserves effort codes (D/W/M/RP/FO) + dependency graph + foundational-obstruction framing. Tier I + II closed (8/8); Tier III 1/5 closed + 3/5 investigated; MC-T4.3 (non-action α-injection mechanism) is the central foundational obstruction.
- **Reference frame structure vocabulary**: `docs/theory/01_reference/REF_REFERENCE_FRAME_VOCABULARY.md` — canonical replacement for "reference frame context" terminology. Two-term core (reference frame structure = structural; frame dynamics = dynamical). Drops qualia commitments without losing conceptual content. Cite this before applying vocabulary in new docs.
- **Chowla–Selberg h≥2 theory note**: `docs/theory/09_mathematical/EXPLR_CHOWLA_SELBERG_HIGHER_H.md` — analytic-machinery list for upgrading Theorem 3 from [NUMERICAL FACT, h=1 only] to a structural theorem covering all CM curves. Closes MC-T2.3.
- **Tier-I/II/III closure proof scripts** (under `scripts/proofs/`):
  - `proof_field_theoretic_qgstar.py` — FTD-0112 / Theorem 9 (T1.3)
  - `proof_per_voxel_mass_gap.py` — FTD-0044 / per-voxel mass gap (T1.4)
  - `proof_phase_j_general_L.py` — Theorem 7 investigation (T1.1)
  - `proof_m_e_exponent_n11.py` — m_e exponent n=11 derivation (T3.2)
  - `proof_scfcc_bcc_bridge.py` — (SC+FCC)/2  BCC investigation (T3.3)
  - `proof_ftd0110_mechanism_gamma.py` — Mechanism γ investigation (T3.1)
  - `proof_bridge_functional_arithmetic_mean.py` — four-mean investigation (T3.4)
  - `proof_polynomial_look_elsewhere_extended.py` — extended scan with pre-registration (T2.1+T2.2)
  - `proof_a1g_dual4_via_zi_units.py` — Z[i]^× structural argument (T1.5+T4.5; superseded by `proof_bcc_complex_structure.py`)
  - `proof_bcc_complex_structure.py` — BCC complex-structure theorem (FTD-0122; T4.5 Roles 1+3 [DERIVED], Roles 2+4 [NO-GO])
- **Dimensionless  Dimensional Map**: `docs/theory/01_reference/SPEC_DIMENSIONAL_MAP.md` (rendered) + `docs/theory/01_reference/dimensional_map.json` (canonical data, 15 entries). Single citation target for "is this prediction dimensionless or calibration-conditional?". Walks the bridge from the 7 algebraic-spine theorems through the 4 dimensionless physical predictions (α, N_c, m_μ/m_e, m_τ/m_e) through the 3 calibration declarations theorem-enforced by FTD-0059 + FTD-0096 (`a_phys ≡ ℓ_P`, `t_phys`, `K_B = m_e`) to one worked dimensional application (m_e in MeV). Renderer: `scripts/proofs/build_dimensional_map.py`. Tests (12 assertions): `scripts/tests/test_dimensional_map.py`.
- **Engine spec**: `engine/SPEC_ENGINE.md`
- **Theory catalog**: `docs/theory/META_INDEX.md`
- **Documentation map**: `META_DOCUMENTATION_MAP.md`
- **Epistemic audit**: `docs/theory/07_assessment/AUDIT_EPISTEMIC_AUDIT.md`
- **Parametric insertions catalog**: `docs/theory/07_assessment/CATALOG_PARAMETRIC_INSERTIONS.md` (~162 rows enumerated: ~23 [DERIVED]/[THEOREM], ~129 [PARAMETRIC], ~10 [IMPOSED]/[SELECTION])
- **EFT Recovery Program** (Phase 0 → F + Phase G reframe): `docs/theory/10_eft_program/scopes_and_specs/SPEC_EFT_RECOVERY_PROGRAM.md` — pre-registered 7-phase campaign. **Phase-F measurement:** a lattice-α plateau at ~1.8× α_ref (classical convention; 3.6× under engine-internal energy convention) across L ∈ {64, 128, 256, 384} GPU scan. **Phase-G reframe:** the plateau is the zero-free-parameter periodic lattice Poisson Green's function `α_r(r, L) = 2 · r · G_L(r)`; R² = 1.0000 at L=384, median 0.07% residual in the Coulomb tail. **This is not a QED deviation** — it is lattice geometry with zero fine-structure content. See `AUDIT_ALPHA_EXTRACTION.md` and `DERIV_EMERGENT_COULOMB_GEOMETRIC.md`. Day-2 interim "1.23×" claim **RETRACTED** (ticks=100 under-equilibrated). Day-2 shipped: matched-stencil CG Poisson (Ward floor 1% → 1e-8), EWSB sharp first-order transition at amp ∈ (0.6, 0.7), condensate m ≈ 0.18 (flux/charge channels agree 3%), Rutherford α = 0.042 ± 0.005 independent cross-check. WSL2 + CUDA 13 path unblocks RTX 5090 (30× speedup). Pipeline<Backend> architecture with CPU/GPU parity. Paper: `dissemination/papers/PAPER_FTD_AS_WILSONIAN_EFT.tex`. Day-2 doc: `docs/theory/10_eft_program/archive/phase_0_f_campaign/DERIV_DAY2_CAMPAIGN.md`. Plan: `C:\Users\cpaci\.claude\plans\vivid-marinating-pudding.md`
- **Engine callstack audit**: `docs/theory/07_assessment/AUDIT_ENGINE_CALLSTACK.md` (CPU/GPU parity, toggle gaps, 10 findings)
- **Open items tracker**: `docs/theory/07_assessment/core_ledgers/TRACKER_OPEN_ITEMS.md` (every `[OPEN]` across code + theory, one place)
- **Infinity reframe**: `docs/theory/07_assessment/AUDIT_INFINITY_REFRAME.md` — undefined-boundary ontology triage; foundational replacement for completed-infinity ℤ³ framing
- **a_phys open problem**: `docs/theory/10_eft_program/archive/resolved/OPEN_A_PHYS_DERIVATION.md` — load-bearing problem the reframe creates: derive `a_phys` (lattice→physical length) from Axiom-Zero invariants or declare it empirical. Three derivation candidates analysed
- **Mechanism γ attempt**: `docs/theory/10_eft_program/archive/closed_negative/DERIV_A_PHYS_MECHANISM_GAMMA_ATTEMPT.md` — gravitational `a_phys` derivation attempted and **closed as candidate** (negative result; recommendation: declare `a_phys ≡ ℓ_P` in `SPEC_FTD.md`)
- **Master quadratic (rewritten)**: `docs/theory/03_derivations/DERIV_MASTER_QUADRATIC_GAP_EQUATION.md` — full rewrite as algebraic identity + physical match (CM-curve uniqueness + dual match); gap-equation/thermodynamic-limit narrative withdrawn
- **Reframe deployment package**: `docs/theory/07_assessment/reframe_deployment/` — `CANONICAL_REFRAME.md` (single source of truth for what the reframe means; agent-facing) + `DEPLOYMENT_GUIDE.md` (7-phase plan) + `agents/` (9 agent prompts) + `templates/` + `checklists/`. Read CANONICAL_REFRAME.md before any reframe-related work
- **Master claim ledger**: `docs/theory/07_assessment/core_ledgers/LEDGER.md` — 52 load-bearing claims with tag history, dependencies, reframe status. **Single source of truth for claim status** — papers cite tags from here; if they disagree, the ledger wins. Rows FTD-0050/0051/0052 (Link 8 closure + Langevin infrastructure + deferred s-Metropolis).
- **Link 8 closure audit**: `docs/theory/10_eft_program/archive/closed_negative/AUDIT_LINK8_CLOSURE.md` — full closure report on "master quadratic as RG-step characteristic polynomial" hypothesis. Three independent tests (Kadanoff blocking, Watson-integral analytical, thermalized |J|² correlator) all NEGATIVE for structurally consistent reasons (engine stencil is (SC+FCC)/2, BCC-orthogonal; master quadratic lives on BCC Watson integral). FTD-0001/0013/0014 UNAFFECTED.
- **Langevin thermostat**: `engine/src/render_bridge.cpp` + `engine/include/ftd/term_toggles.h` — CPU single-substrate OU update on wave_vel, toggle-gated. Validated by `engine/tests/test_langevin_equipartition.cpp` (equipartition to 4%). Unblocks non-zero-T matched-stencil β, condensate ensemble measurements, fluctuation-dissipation tests.
- **Reframe changelog** (append-only): `docs/theory/07_assessment/CHANGELOG_REFRAME.md` — every decision and change made under the reframe deployment
- **Devil's advocate report**: `docs/theory/07_assessment/campaigns/archive_session_outputs/DEVILS_ADVOCATE_REPORT.md` — falsification pass on 6 substantive rewrites; 3 blocking bugs found and fixed same-day, 5 PASS-WITH-NOTES queued
- **Engine reframe audit**: `docs/theory/07_assessment/campaigns/archive_session_outputs/ENGINE_AUDIT_REFRAME.md` — C++/CUDA/JS sweep for completed-infinity + hidden α; 3 HIGH (2 fixed same-day, 1 deferred for owner: `α_inf` rename across CSV/Python/TeX), 6 MEDIUM, 9 LOW. Parameter-free claim status: CONDITIONAL
- **Portfolio inventory**: `docs/theory/07_assessment/campaigns/archive_session_outputs/INVENTORY_PORTFOLIO.md` — 280 artifacts cataloged outside `docs/theory/`; 267 editable, 13 PDF-only; manuscript_v1v2 share ~57 chapters that must be propagated together
- **Paper classification**: `docs/theory/07_assessment/campaigns/archive_session_outputs/FLAGGED_PASSAGES_PAPERS.md` — 34 TeX/MD papers in `docs/papers/` classified; 10 clean, ~37 proscribed passages in 7 files. Top-7 priority list inside
- **YM/NS RE-DERIVE assessment**: `docs/theory/07_assessment/campaigns/archive_session_outputs/REDERIVE_REPORT_YM_NS.md` — both speculative Clay-aimed papers lose post-reframe; YM has 1 surviving theorem (per-voxel mass gap), NS has none; SPLIT/DEMOTE/RETRACT options laid out per paper
- **Manuscript propagation rule**: `dissemination/manuscript_v2/PROPAGATION_RULE.md` — authoritative rule for v1v2vol1vol2 chapter editing. **Mandatory before any chapter edit** (vol1/vol2 are NOT symlinks; already diverged)
- **Calibration — electron-primary default** (FTD-0137 §4.5, `FOUND_ELECTRON_PRIMARY_GAUGE.md`; declared in SPEC_FTD.md): import `{ℏ, c, m_e}` — the single beyond-universal anchor is the electron mass. Derived: `a_phys = ℓ_P` [DERIVED ~0.19%], one tick `t_phys ≡ ℓ_P/(√3·c) = t_P/√3 ≈ 3.11×10⁻⁴⁴ s` (corrected 2026-07-08 from √3·ℓ_P/c — see DERIV_DIMENSIONAL_GATE.md), and Newton `G` [SMC output, `α_G=(m_e/m_P)²`]. Legacy Planck-primary (`a_phys ≡ ℓ_P` declared) remains a valid gauge. **Every dimensional FTD prediction is conditional on the calibration; dimensionless predictions (α, mass ratios, mixing angles) are calibration-independent and constitute the falsifiable spine.**
- **Changelog (engine + project)**: `CHANGELOG.md`
- **Complete SM**: `scripts/proofs/proof_complete_sm.py`
- **Motivic proof**: `scripts/proofs/proof_motivic_master_quadratic.py`
- **Moore Layer Theorem**: `docs/theory/08_structural/THEOREM_MOORE_LAYER_DECOMPOSITION.md`
- **Phase lattice**: `docs/theory/08_structural/EXPLR_PHASE_LATTICE_MOORE.md`
- **50-test battery**: `scripts/exploration/test_all_physics.py`
- **Loop derivations**: `scripts/exploration/compute_c2.py`, `derive_all_loops.py`, `gauge_loops.py`
- **Arrow paper**: `docs/papers/PAPER_RATIO_AND_THE_ARROW.tex`
- **Engine coupling test**: `engine/tests/test_intervoxel_coupling.cpp`
- **Complete Chain**: `docs/theory/01_reference/SPEC_FTD_COMPLETE_CHAIN.md`
- **QM as Statistics**: `docs/theory/03_derivations/DERIV_QM_FROM_LATTICE.md`
- **Lattice Physics Reference**: `docs/theory/02_foundations/FOUND_LATTICE_PHYSICS_INTUITIONS.md`
- **Stellar Lifecycle**: `docs/theory/03_derivations/DERIV_STELLAR_LIFECYCLE_LATTICE.md`
- **Master Verification**: `scripts/proofs/proof_master_verification.py` (54/54 checks)
- **BCC Unification**: `docs/theory/08_structural/DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md`
- **Observer Formalism**: `docs/theory/02_foundations/FOUND_POTENTIAL_CORE_AND_GENERATIVE_INTERIOR.md` (Part II: 3³ lattice grounding)
- **Engine-Theory Bridge**: `engine/tests/benchmark_engine_theory.cpp` (20 benchmarks)
- **Emergent Alpha**: `engine/tests/benchmark_emergent_alpha.cpp` (6 EFT experiments)
- **Benchmark Harness**: `scripts/benchmarks/benchmark_engine_vs_theory.py` (Python analysis)
- **Convergence Analysis**: `scripts/benchmarks/analyze_convergence.py` (20-benchmark report + plots)
- **Benchmark Results**: `scripts/benchmarks/results/` (reports, plots, CSV)
- **Wilson Loops**: `engine/tests/benchmark_wilson_loops.cpp` (12/17 pass, flux tube detected)
- **Gluon Dynamics**: `engine/tests/campaign_gluon_dynamics.cpp` (7/11 pass, linear E(r))
- **Einstein Equations**: `engine/tests/test_einstein_equations.cpp` (time dilation 0.004% match after latency fix)
- **BH Thermodynamics**: `engine/tests/benchmark_black_hole_thermo.cpp` (L_peak 0.62, proper time dilation)
- **Budget Equation**: `engine/tests/benchmark_budget_equation.cpp` (x/K+G*/x=1 to 0.2%)
- **Continuum Limit -> QED**: `docs/theory/03_derivations/DERIV_CONTINUUM_LIMIT_QED_EQUIVALENCE.md` (x+ = 1/alpha conditional [THEOREM])
- **Singlet from Void**: `docs/theory/03_derivations/DERIV_SINGLET_FROM_VOID_EVENT.md` (Bell loop closed via 5 lemmas)
- **N_c from Topology**: `docs/theory/03_derivations/standard_model/DERIV_NC_FROM_TOPOLOGY.md` (N_c = 3 from 4 independent routes)
- **Web refactor spec**: `engine/web/docs/SPEC_REFACTOR_LARGE_FILES.md` (Waves 0-3 split of viewport/wasm-bridge-dag/app_dag + RF-1/3/4/5/6/7/8/10 post-audit cleanup; Ticket 14 + RF-9-full deferred). Final: viewport 5325→3900, wasm-bridge-dag 5736→2132, app_dag 1898→1723 (−5204 LOC, −40% across three files)
- **Whole-project extraction** (v2.15.0): 16-agent parallel refactor split every file ≥500 LOC into discrete-responsibility modules. C++: render_bridge.cpp 2139→1097, constructors.cpp 1245→0 (deleted, 5 split files), scenarios.cpp 1241→79, ftd_wasm.cpp 1224→607, cosmic_engine 1193→500, atom_engine 1029→325, main.cpp 938→74, ontic.h 806→45 (+6 theme-headers), ws_server 831→496. JS: mock-scale5 1903→313, reference frame context triad −1711, scale controllers −1248, backgrounds 846→178, field-overlays 976→455. Python: 3 common helpers extracted. Total: ~13800 LOC redistributed across ~97 new files, every module nameable in ≤5 words
- **Scenario library (C++)**: `engine/include/ftd/scenarios.h` + `engine/src/scenarios/{flux,light,quantum,s0_seed,s0_field}.cpp` (all 83 Scale-0 scenarios ported from JS MockBridge; 84/84 Playwright coverage, 5/5 parity CI guard)
- **Web power-user guide**: `engine/web/docs/USER_GUIDE.md` (15-section reference for dashboard + console workflows)
- **Scenario parity CI guard**: `engine/web/tests/scenario-parity.spec.js` (5 assertions covering JSC++ scenario name drift; runs in <1s)
- **Viewport extracted modules**: `engine/web/js/viewport/{color-ramps,molecular-renderer,boundary-geometry,topology-sheet-renderer}.js` (own their Three.js concerns; viewport.js keeps thin delegators)

---

## EFT Reconstruction

Alpha is restructured (not derived) in the engine — `ALPHA_EFT = G_C²` is an algebraic identity by construction (G_C ≡ √α), a consistency check per `constants.h`'s own HONEST FRAMING note:
- `ALPHA_EFT = G_C * G_C` defined in `constants.h` with compile-time `static_assert` pinning it to the hardcoded `ALPHA` (α is the INPUT; B2's "alpha recovery" measures solver self-consistency, not α)
- G_C (wave equation coupling) is the fundamental lattice parameter
- Force computations use `ALPHA` or `ALPHA_EFT` depending on mode (equal by construction via `static_assert`; `phase_forces.cpp` uses `ALPHA` directly in Poisson/legacy modes, `G_C` in emergent mode)
- New toggle `emergent_forces` computes force from flux gradient without Poisson solver
- 20-benchmark scorecard (re-graded 2026-07-01, see `analyze_convergence.py`): Coulomb convergence (B+), hydrogen spectrum (A+ — classical Kepler check, not a quantum eigenvalue derivation), color forces (A+), Higgs threshold (A+), Bell S=2.000 (N/A — classical LHV bound from a standalone toy, confirms locality, not a QM result), Born lattice bias (A-); 211/211 CTest passing

---

## Naming Conventions

- Markdown files: `UPPER_SNAKE_CASE` with semantic prefix
- Prefixes: `SPEC_` (specifications), `DERIV_` (derivations), `FOUND_` (foundations), `AUDIT_` (assessment), `EXPLR_` (exploratory), `REF_` (reference), `ARCH_` (archived), `META_` (meta-documentation)
- Engine: C++17, snake_case functions, CamelCase types

---

## Environment Notes

- Platform: Windows 11. No `rsync` — use `cp -r` for directory copies
- **Hardware Profile / CPU Parallelization**: The host machine is equipped with an ultra-high-end AMD Ryzen 9 9950X3D (16 cores, 32 threads) and an NVIDIA RTX 5090. When running CPU-based CTests or C++ compilations, **always maximize CPU resource load by passing high concurrency flags** (e.g. `ctest -j 24` or `ctest -j 32`, and `cmake --build ... --parallel 24` or `--parallel 32`). Without parallelization, sequential test execution will run pathologically slow and waste massive compute capacity.
- **GPU execution MUST go through WSL2 Ubuntu-22.04, not Windows-native CUDA.** RTX 5090 speedup (~30×) is only available via the WSL2 build at `engine/build_wsl`. Windows-native CUDA builds from `engine/build/` technically run but are pathologically slow (observed 19 minutes wall for a single L=64 density=0.1 seed). Invocation pattern:
  ```
  wsl.exe -d Ubuntu-22.04 -- bash -c "cd /mnt/c/Users/cpaci/Desktop/ftd && \
      engine/build_wsl/benchmark_foo --args"
  ```
  Windows-native CUDA is acceptable for compile-time checks and single-tick correctness tests only. Any measurement campaign, sweep, or multi-seed run goes through WSL2.
- Python tests: `scripts/tests/` (pytest). C++ tests: `engine/tests/` (CTest). No overlap between them
- **D-module / holonomic CAS (WSL2 Ubuntu-22.04):** SageMath 9.5 (`apt sagemath`) + **ore_algebra 0.5** are installed for differential-operator work (factorization, symmetric powers, local solutions). ⚠ numpy is pinned to **1.24.4** in Sage's pip env (ore_algebra pulls numpy 2.x, which breaks Sage 9.5's compiled Cython modules). Run: `wsl.exe -d Ubuntu-22.04 -- bash -lc "cd /mnt/c/Users/cpaci/Desktop/ftd && sage -python <script>"`. ⚠ ore_algebra's analytic `right_factor()`/`factor()` need **finite singularities** — constant-coefficient operators (e.g. `Dz²−1`) are degenerate and hang; use Fuchsian operators. Example: `scripts/proofs/factor_stencil18_sage.py` (FTD-0372). WolframScript is present but **license-blocked** (unusable).
- `scripts/constants.py` is the canonical shared constants module imported by 20+ scripts
- Build `.bat` files live in `engine/` — use `vswhere.exe` for portable VS detection
- `dissemination/media/`, `models/`, and `archive/` are gitignored — they exist on disk but not in git
- `docs/internal/` is gitignored — session summaries and explorations are local-only
