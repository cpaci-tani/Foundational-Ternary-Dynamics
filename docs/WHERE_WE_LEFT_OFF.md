# Current State of FTD

A single-page orientation to where the framework stands and where its canonical truth lives. This page carries **no change log** — what changed and when lives in `CHANGELOG.md` and git history; per-claim status lives in the LEDGER.

Foundational Ternary Dynamics is a philosophy-of-mathematics project: a discrete, finite, deterministic ternary-lattice ontology, the mathematics it forces, and the physics that mathematics suggests, ordered **Ontology > Logic > Math > Physics**. The project's aim is the Number-One Goal in `CLAUDE.md` (amended 2026-07-12, FTD-0383) — set the smallest honest set of types a discrete ontology can speak from; build the content forward, sector by sector, until every physical structure is either forced content or a marked-and-priced import; **drive** every priced line to retirement, a theorem-grade no-go, or a sharper falsifier — never leaving a line merely booked; and where a line provably resists retirement, search deliberately for the next honest type whose declared adoption converts it into content at a minimal, falsifiable price (the boundary marked *qualitatively* by the modulus/argument frontier, *quantitatively* by the priced-import ledger FTD-0371, and *driven* by the Consumption Program charter, `01_reference/SCOPE_CONSUMPTION_PROGRAM.md`).

## Where the canonical truth lives (in precedence)

**LEDGER > constitution > all other prose.** When two documents disagree, the higher one wins.

- **Constitution** — `docs/theory/01_reference/SPEC_FTD_FRAMEWORK_V1.md`. The framework statement of record: Postulates P1–P5 (frozen), Framework Commitments (FC-0, FC-1, FC-2, FC-3, FC-W — `[AXIOM]`-class declarations), and Calibrations.
- **LEDGER** — `docs/theory/07_assessment/core_ledgers/LEDGER.md`. Per-claim tags, dependencies, provenance, and the live `FTD-NNNN` id register. The single source of truth for claim status.
- **Bedrock tracker** — `docs/theory/07_assessment/core_ledgers/TRACKER_ONTIC_TRUTH.md`. The five truth tiers (T1 rock-solid → T5 conjecture); read before defending any FTD math claim.
- **Doctrine ledger** — `docs/theory/01_reference/SPEC_DOCTRINE_LEDGER.md`. Single-page status map rolling up the above.
- **Open items** — `docs/theory/07_assessment/core_ledgers/TRACKER_OPEN_ITEMS.md`. Every `[OPEN]` across code and theory.

## What is load-bearing — the algebraic spine

The theorem-grade core, independent of any physics interpretation, lives in `docs/theory/01_reference/SPEC_ALGEBRAIC_SPINE.md`. It is genuine mathematics, **conditional on Chudnovsky 1976** (algebraic independence of π and Γ(1/4)):

- `G* = Γ(1/4)/Γ(3/4) ≈ 2.95868` (the lemniscatic ratio — **not** the Bernoulli/Gauss constant ϖ ≈ 2.6221).
- The master quadratic `x² − 16G*²x + 16G*³ = 0`, its roots, and Vieta structure.
- The Watson identity `W₃ = G*²/(2π)` via Chowla–Selberg; CM-curve uniqueness among class-number-1 fields; the coefficient `16 = |Aut(E)|²`.
- `ℚ(G*)` as a maximal π-free subfield of `ℚ(π, Γ(1/4))`; the harmonic-invariant tower.

Of the nine numbered results, seven are theorem-grade and two are honestly tiered below theorem grade (see §0 of the spine).

## What is suggestive — physics at LEDGER status

Physics identifications ride at their actual tags and are never promoted by rhetorical momentum:

- `x₊ ≈ 137.036 = 1/α` to 1.26 ppm is a `[STRONGLY MOTIVATED CONJECTURE]` (FTD-0013) — the algebra is a theorem; the *physical identification* is not derived. The structural-uniqueness evidence is the FTD-0319 look-elsewhere scan (a `[NUMERICAL FACT]`).
- Mass formulas, gauge ratios, and the gravity/QM identifications are `[PARAMETRIC]` or `[SMC]` (see the LEDGER and `CATALOG_PARAMETRIC_INSERTIONS.md`).
- **α is not derived anywhere.** Whether the discrete ontology forces the EM coupling is the central open obstruction, **MC-T4.3**, a `[FOUNDATIONAL OBSTRUCTION]`: 0/4 FTD-native routes force the master-quadratic operator assembly (FTD-0242), so α is dynamical, not structural.
- **FC-W** is the framework's one *adopted* import — an external order-2 twist realizing `δ = √(G*(4G*−1))`; under it `x₊ = 1/α` is a `[CONDITIONAL THEOREM given W]`, explicitly not `[DERIVED]` (FTD-0314/0315).

## The standing boundary

The map of what the discrete ontology can and cannot set for itself: `docs/theory/07_assessment/AUDIT_BOUNDARY_MAP.md`.

- **The modulus/argument frontier** (`docs/theory/02_foundations/FOUND_MODULUS_ARGUMENT_FRONTIER.md`): a finite, discrete, deterministic, forward-only substrate owns the forced/modulus/even half (π) and cannot self-supply the chosen/argument/odd half (G*, the chosen adjoint δ).
- **The priced-import ledger** (`docs/theory/01_reference/SPEC_IMPORT_LEDGER.md`, FTD-0371): the frontier made *quantitative* — every imported type in a common currency with a falsifier each: 1 adopted bit (FC-W/δ), 4 selected types (incl. **D=3 — [SELECTION — declared], FTD-0355**; A_μ=𝒫_T J_μ minted as IMP-S4 2026-07-12), 4 named results (Chudnovsky + CM-h=1/E1/E\*/E\*\*), 3 calibrations (a_phys/t_phys/K_B), the empirical bridges, and 2 *declined* bets (M, reversibility). ⚠ Reading guard: the "1 bit" is the α-branch choice only, not the total physics import.
- **The type-priority principle** (`docs/theory/02_foundations/FOUND_TYPE_PRIORITY_PRINCIPLE.md`): context (a type) is prior to and the precondition for the value of content (a token); the Framework Commitments are precondition-types, which is why they are adopted rather than derived.
- Which predictions are dimensionless vs calibration-conditional: `docs/theory/01_reference/SPEC_DIMENSIONAL_MAP.md`. The **default calibration is electron-primary** (FTD-0137, `FOUND_ELECTRON_PRIMARY_GAUGE.md`, declared in `SPEC_FTD.md`): import `{ℏ, c, m_e}`; derive `a_phys = ℓ_P` `[DERIVED ~0.19%]`, one tick `t_phys = ℓ_P/(√3·c) = t_P/√3 ≈ 3.11×10⁻⁴⁴ s`, and Newton `G` as an `[SMC]` output. The dimensionless spine (α, mass ratios, mixing angles) is calibration-independent; every dimensional prediction is conditional on this gauge. How the gate carries dimensionless → dimensionful: `docs/theory/03_derivations/foundational_mechanics/DERIV_DIMENSIONAL_GATE.md`.

## The engine

A logic-first C++/CUDA simulation of the substrate (`engine/SPEC_ENGINE.md`); only 6 rules are derived from the axioms, all phenomenological features are toggle-gated and default OFF. Determinism is pinned by the golden hash **`0xb604d81a3d79366e`** at L=17 (rebuild and compare to verify a change is physics-neutral). The engine is **not** a quantum-dynamics engine — it has no ℏ and no Pauli exclusion, and the lattice's dispersion is not the Schrödinger one (FTD-0270); it does not derive atomic spectra.

## Start here

- New to the project: `README.md`, then the constitution.
- Working instructions, navigation, and discipline: `CLAUDE.md`.
- Architecture and task→file navigation: `META_PROJECT_ATLAS.md`; theory catalog: `docs/theory/META_INDEX.md`.
