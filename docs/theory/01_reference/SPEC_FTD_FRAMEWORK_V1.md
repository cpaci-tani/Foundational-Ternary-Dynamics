# SPEC — The FTD Framework, Version 1 (the Constitution)

**Tag:** `[SYNTHESIS]` (re-states canonical claims at their canonical tags) **+ five `[AXIOM]`-class framework-commitment declarations (FC-0, FC-1, FC-2, FC-3, FC-4)** + `[SELECTION]` (the register and decomposition choices made here).
**LEDGER:** FTD-0254 (this document); FC-1 = FTD-0255; FC-2 = FTD-0256; the two-field formalization = FTD-0257; the deviation ledger = FTD-0258; FC-3 = FTD-0304; the carrier-narrowing theorem = FTD-0314; FC-4 (FC-W) = FTD-0315.
**Status line (read first):** **Nothing is promoted by this document.** `x₊ = 1/α` (FTD-0013) stays `[STRONGLY MOTIVATED CONJECTURE]`; MC-T4.3 stays a `[FOUNDATIONAL OBSTRUCTION]`; FTD-0208 stays `[CLOSED NEGATIVE]`; the algebraic spine count (7 theorem-grade + 2 honestly tiered) is unchanged. The new content of this document is a set of **declarations** — choices of model, not derivations — plus the assembly of existing results into one constitution.
**Companion:** [`SPEC_PREDICTION_LEDGER_DEVIATIONS.md`](SPEC_PREDICTION_LEDGER_DEVIATIONS.md) (FTD-0258, the falsifiable deviation spine).

---

## §0 · Charter

### 0.1 What this document is

This is the **constitution of Foundational Ternary Dynamics as a standalone framework**: the single canonical statement of its postulates, its framework commitments, its calibrations, its mathematical core, its observer layer, its computational effective field theory, and its falsification spine — ordered by the framework's chain of priority:

> **Ontology > Logic > Mathematics > Philosophy > Physics > Science.**

The ordering is a methodological commitment, not a ranking of importance: each layer is *accountable to* the layers after it (a constitution whose physics layer ignored measurement would be worthless), but *constructed from* the layers before it. It instantiates the math-first ordering of [`SPEC_MATH_FIRST_ONTOLOGY.md`](SPEC_MATH_FIRST_ONTOLOGY.md) (FTD-0153 `[SYNTHESIS]`): *ontology = finite mathematical invariant structure; physics = operationally stable readout of that structure.*

This document serves the project's Number-One Goal in all three clauses (amended 2026-07-12, FTD-0383): **derive everything we can from a discrete ontology — rigorously mark and price what we cannot — and drive every priced import toward retirement, a theorem-grade no-go, or a declared minimal adoption, never leaving a line merely booked.** The framework commitments declared here (§2.4, §2.6) sit exactly on that boundary: they are the framework's chosen answers to questions its own theorems prove the postulates *do not* answer. Ontology-first is therefore **not** evidence-optional — §6 makes the commitments themselves falsifiable, and the drive clause makes the priced lines standing work items (program charter: `SCOPE_CONSUMPTION_PROGRAM.md`).

### 0.2 The three registers

This constitution separates three kinds of foundational input that the corpus has historically discussed in one breath:

| Register | Contents | Tag class | What they do |
|---|---|---|---|
| **Postulates** (P1–P5) | Discrete space, discrete time, ternary states, Moore locality, determinism | `[AXIOM]` | **Generative** — they define the dynamics. Frozen; hundreds of documents and the machine-checked theorems ([`lean/Standalone.lean`](../../../lean/Standalone.lean), FTD-0243, FTD-0253) quantify over exactly these five. |
| **Framework Commitments** (FC-0, FC-1, FC-2, FC-3, FC-4) | The ℤ[i] reading; the declination of the measurement map M; the native arrow / emergent metric / space⊥time; scale-ratio-covariance (only internal ratios physical; `a`, `L` are observation scales); **FC-W — the *adopted* external α-binding law W (the framework's one adopted import, vs the declined M / reversibility; §3.5)** | `[AXIOM]`-class **declarations** | **Model-fixing** — where a theorem proves the postulates leave a fork open, a commitment picks the branch FTD asserts the world is. A declaration, not a derivation. |
| **Calibrations** | **electron-primary default** (FTD-0137 §4.5): import `{ℏ, c, m_e}`; single beyond-universal anchor `m_e` (`M_REST`, formerly `K_B`). Derived: `a_phys = ℓ_P` [DERIVED ~0.19%], `t_phys = ℓ_P/(√3·c)`, `G` [SMC output] | `[DECLARED]` | **Unit-fixing** — FTD-0059/0096 prove no length/mass derivable from Axiom Zero *alone*, so one import is irreducible (grade-0 closure FTD-0368); electron-primary makes it the measurable `m_e`. Legacy Planck-primary (`a_phys ≡ ℓ_P` declared) + 3 other gauges per [`FOUND_LATTICE_SPACING_GAUGE_FREEDOM.md`](../02_foundations/FOUND_LATTICE_SPACING_GAUGE_FREEDOM.md) (FTD-0137). Dimensionless predictions are calibration-invariant. |

**Why the commitments are not "P6/P7".** The corpus's standing term "6th-postulate-class input" names something FTD would have to *add* to obtain quantum non-commutativity ([`THEOREM_COMMUTATIVITY_INDEPENDENCE.md`](../10_eft_program/derivations/THEOREM_COMMUTATIVITY_INDEPENDENCE.md) §1: "$M$, Postulate 6") or the Lorentzian metric ([`FOUND_SPACETIME_FORCING_BOUNDARY.md`](../02_foundations/FOUND_SPACETIME_FORCING_BOUNDARY.md) §4: reversibility). FC-1 and FC-2 are **declinations** of those additions — numbering a refusal as a postulate would invert its content, and would collide with the separate `[CONJECTURE]`-grade proposal already titled "Postulate Six" in [`FOUND_EPISTEMIC_SYMMETRIES_AND_CHIRALITY.md`](../02_foundations/FOUND_EPISTEMIC_SYMMETRIES_AND_CHIRALITY.md) (FTD-0248 — an *operational chirality* postulate, outside this kernel; a rename of that document is queued, §6.4). P1–P5 stay frozen.

### 0.3 The epistemic-tag contract

Every substantive claim in this document carries an epistemic tag, with [`LEDGER.md`](../07_assessment/core_ledgers/LEDGER.md) as the single source of truth: **if this document and the LEDGER ever disagree on a tag, the LEDGER wins.** Conflict precedence corpus-wide: LEDGER > this constitution > [`docs/SPEC_FTD.md`](../../SPEC_FTD.md) (the readable overview). The tag legend is the LEDGER's; the declarations introduced here use `[AXIOM]`-class tags with the reviewer expectation *"accept as model definition"* — and the explicit understanding that **a tag is a label, not a resolution** (the F10 discipline): declaring FC-1 does not make non-commutativity derivable, it makes FTD's refusal to import it official.

### 0.4 Division of labor

- [`MONOGRAPH_FTD_CONSTRUCTION.md`](MONOGRAPH_FTD_CONSTRUCTION.md) (FTD-0249) is the **bottom-up construction story** — how the mathematics is built and where its provable boundary lies. This constitution **cites it and never duplicates its proofs**.
- [`SPEC_ALGEBRAIC_SPINE.md`](SPEC_ALGEBRAIC_SPINE.md) is the **theorems-only core**; [`TRACKER_ONTIC_TRUTH.md`](../07_assessment/core_ledgers/TRACKER_ONTIC_TRUTH.md) is the **tier table**; [`SPEC_DOCTRINE_LEDGER.md`](SPEC_DOCTRINE_LEDGER.md) is the **status map**.
- [`docs/SPEC_FTD.md`](../../SPEC_FTD.md) remains the readable framework overview; its full editorial alignment to this constitution is queued (§6.4), and until then §0.3's precedence rule governs.
- This constitution is the **forward-facing statement of what FTD is** — the document a physicist, philosopher, or mathematician reads to learn what the framework asserts, what it declines, and how to kill it.

---

## §1 · Ontology

### 1.1 The five postulates `[AXIOM]`

The substrate of FTD (canonical statements: [`docs/SPEC_FTD.md`](../../SPEC_FTD.md) Ch. 1; [`FOUND_AXIOM_ZERO.md`](../02_foundations/FOUND_AXIOM_ZERO.md)):

- **P1 — Discrete space.** A 3-dimensional cubic lattice of voxels with **undefined boundary**: at every specified position, axis-adjacent sites exist; the lattice is *not* a completed-infinity ℤ³ totality (`AUDIT_INFINITY_REFRAME.md`). Arbitrarily large finite computations are well-posed; completed limits are not.
- **P2 — Discrete time.** Dynamics proceeds in global ticks. Simultaneity at the substrate level is absolute (a privileged foliation — the neo-Lorentzian reading, §4.2).
- **P3 — Ternary states, J-primary.** Each voxel carries a continuous flux vector `J ∈ ℝ³` (dispositional) and a ternary state `s ∈ {−1, 0, +1}` (actual). `J` is primary; `s` is the manifestation projection of `J` via the genesis threshold rule ([`docs/SPEC_FTD.md`](../../SPEC_FTD.md) §1.1: "treating s as primary would double-count"). The ternary value set is grounded in the ℤ[i] reading: `s ∈ {i², 0, |i²|}` ([`FOUND_TERNARY_STATE_FROM_I.md`](../02_foundations/FOUND_TERNARY_STATE_FROM_I.md), FTD-0128 `[SYNTHESIS]`).
- **P4 — Local causality.** Updates depend only on the 26-neighbour Moore neighborhood at the previous tick; information moves ≤ 1 voxel per tick.
- **P5 — Determinism.** The update map is a function. **Determinism is not reversibility** (§2.1) — a deterministic map may be many-to-one.

### 1.2 Graded monism, and FC-0

The ontology is **graded monism** ([`docs/SPEC_FTD.md`](../../SPEC_FTD.md) §1.1): one substance (the void), whose **dispositions** are the flux `J`, whose **manifestations** are the actualized states `±1`, and whose **properties** (charge, mass) are emergent from manifestation patterns.

> **FC-0 (framework commitment, `[AXIOM]`-class declaration).** *The cubic lattice's order-4 planar symmetry is read as the arithmetic of the Gaussian integers ℤ[i].* This reading is a **modelling choice**: the algebraic spine (§3.1) is forced by P1–P5 **together with** FC-0, not by the discrete ontology alone — the honesty correction the construction monograph's red-team installed (FTD-0249, LEDGER row), here canonized as the register's zeroth commitment. Declaring it does not derive it.

### 1.3 The two orthogonal fields `[SELECTION + SYNTHESIS — FTD-0257]`

FTD's field content is **two orthogonal fields, nested**:

**Primary pair — Flux ⊥ State (dispositional ⊥ actual).** `J ∈ ℝ³` is continuous, reversible-in-isolation (§2.6), and carries the wave dynamics; `s ∈ {−1,0,+1}` is discrete, finite, and irreversible (manifestation is many-to-one). They are *orthogonal* in the precise sense that they inhabit different ontological grades (disposition vs actuality) and **interact through exactly two channels and no others**:

1. **Genesis / evaporation (downward, J → s):** at a void site, `|J|` crossing the manifestation threshold projects a state (`s = sign`-rule); falling below the evaporation threshold reverts it. The kinetics scale is `K_MANIFEST`, the trigger `K_GENESIS = N_c·K_MANIFEST` (type-separated from the mass quantum `M_REST` in the unified-mass arc, FTD-0130/FTD-0250 Phase 0).
2. **Gauss sourcing (upward, s → J):** manifested states source the flux through the lattice Gauss constraint `∇·J = ρ_s` (the projection step of the tick cycle; engine canon in [`MAP_LAGRANGIAN_TO_ENGINE.md`](MAP_LAGRANGIAN_TO_ENGINE.md)).

Everything else in the framework is dynamics *within* one field or the other. This two-channel coupling structure is the engine's actual architecture (`phase_write` genesis; `gauss_project` sourcing) — the formalization here packages it as the canonical ontology. `[SELECTION — a canonical decomposition choice; no new dynamical claim]`

**Nested pair — the symplectic quadratures (q, p) inside the flux layer.** The flux sector is a second-order field, so its true arena is phase space: per mode, the amplitude `q` and its rate `p = ∂_t J` (engine: `wave_vel`) form a symplectic pair. The substrate's **only native dynamical angle** is the quadrature phase `arg(q + ip)`, winding at the dispersion frequency `ω(k) = 2·C_WAVE·|sin(k/2)|`, `C_WAVE = 1/√3` — measured: multi-tick winding matches the single-tick eigenvalue to 0.10–0.98 % (modes n = 1, 2, 4; the residual growing as the leapfrog `O((ωΔt)²)` signature), while the transverse spatial orientation `arg(J_x + iJ_y)` is dynamically **frozen** (leakage ≈ 1.6×10⁻¹⁶, machine zero) and the dual-substrate L/R channels stay an exact mirror (`|J_L − J_R| = 0`) ([`EXPLR_SUBSTRATE_NATIVE_ANGLE.md`](../06_reference_frames_and_measurement/EXPLR_SUBSTRATE_NATIVE_ANGLE.md), FTD-0251 `[MEASURED]`). The pair is symplectically non-trivial but observably commutative: `{q,p} ≠ 0` (Poisson) yet `[q,p] = 0` (observable commutator) — the Crucial Distinction of FTD-0243 §3. The quadrature **clock** is the substrate's native time-keeper (§5.2); identifying it with "the measurement angle" is interpretive `[SELECTION]`, and the *incompatibility* that would make choosing a quadrature a quantum measurement is exactly what FC-1 declines to import.

**Why "orthogonal."** The primary pair is orthogonal at the grade level (no state-state dynamics; no flux-flux manifestation; two coupling channels only). The nested pair is orthogonal in the symplectic sense (conjugate quadratures of one oscillation). The two orthogonalities live at different levels and do not compete — the (q,p) structure is *internal to* the flux member of the primary pair.

### 1.4 More fields, not more dimensions `[SYNTHESIS]`

`D = 3` is a **[SELECTION — declared]** given FC-0 (FTD-0355): the arithmetic identity `|Aut(E)|² = 2^D·(D−1)!` has D = 3 as its unique solution `[THEOREM]`, but the *dimension-forcing* is not forced — the RHS target `16 = |O_h|/3` presupposes D = 3, a circularity named in FTD-0355; the earlier "forces D = 3 — FTD-0010 `[THEOREM]`" is demoted. Everything beyond the primary pair is a **derived or imposed decomposition over the same three dimensions**, never a new dimension:

| Field/decomposition | Status | Provenance |
|---|---|---|
| `wave_vel` (= p quadrature) | the flux sector's canonical momentum — part of the nested pair, §1.3 | engine voxel state; FTD-0251 |
| Latency `L(x)` (time-rate / gravity potential) | `[IMPOSED]` — solves a lattice Poisson problem sourced by rest mass; gravitational phenomenology §5.1 | `SPEC_FTD_LAGRANGIAN.md` §3.3/§4.2 |
| Dual substrate L/R (chirality split) | `[IMPOSED]` — a parametrized split of `J`, not an independent field; the parity-violating coupling is hand-set | [`AUDIT_DUAL_SUBSTRATE_PROVENANCE.md`](../07_assessment/AUDIT_DUAL_SUBSTRATE_PROVENANCE.md); FTD-0248 |
| Charge density, force accumulators | derived per-tick functionals | engine diagnostics |

The closed generator inventory of the observable algebra is `{s, J, wave_vel, L}` (FTD-0243 §2). A future field proposal enters this constitution only by amendment (§7).

### 1.5 Space ⊥ time at the base `[AXIOM-level structure + FC-2 forward-pointer]`

P1 (the lattice) and P2 (the tick) are **separate primitives**: FTD's base ontology has a 3-space and a 1-time, not a 4-geometry. Nothing at the postulate level mixes them; the *causal cone* that relates them is a theorem of locality (§5.2), and the Lorentzian *metric* that would weave them into Minkowski spacetime is **not forced** by P1–P5 (FTD-0253). FC-2 (§2.6, §5.2) declares FTD's position: the mixing is **emergent and sector-scoped**, the separation fundamental.

---

## §2 · Logic

### 2.1 Determinism ≠ reversibility `[SYNTHESIS]`

P5 says the update map is a function; it does **not** say the map is invertible. A deterministic map may be many-to-one — diffusion is deterministic *and* irreversible ([`FOUND_SPACETIME_FORCING_BOUNDARY.md`](../02_foundations/FOUND_SPACETIME_FORCING_BOUNDARY.md) §2). This distinction carries the entire weight of FC-2.

### 2.2 The substrate's logic is classical `[THEOREM]`

The observable algebra `A₅` — all real-valued functionals of the configuration `(s, J, wave_vel, L)` closed under pointwise products, Moore sums, and composition with the update map — is **strictly commutative** ([`THEOREM_COMMUTATIVITY_INDEPENDENCE.md`](../10_eft_program/derivations/THEOREM_COMMUTATIVITY_INDEPENDENCE.md), FTD-0243 `[THEOREM]`; algebraic core machine-checked in [`lean/Standalone.lean`](../../../lean/Standalone.lean)). A commutative algebra has a distributive (Boolean) event lattice (Birkhoff–von Neumann), hence joint probability distributions always exist. The substrate's logic is classical logic.

### 2.3 The closed routes (the non-commutativity wall) `[CLOSED NEGATIVE × 4]`

Every attempted derivation of quantum non-commutativity from P1–P5 has been closed:

| Route | Result | Source |
|---|---|---|
| Modular time (Tomita–Takesaki type III₁ from the substrate algebra) | commutative → type I → trivial modular flow | FTD-0225 `[CLOSED NEGATIVE]` ([archive](../10_eft_program/archive/closed_negative/AUDIT_MODULAR_TIME_ALGEBRA_TYPE_CLOSED_NEGATIVE.md)) |
| Manifestation as quantum measurement (genesis + Gauss back-reaction) | deterministic function of commuting flux → Boolean event lattice → classical coarse-graining | FTD-0226 `[CLOSED NEGATIVE]` ([archive](../10_eft_program/archive/closed_negative/AUDIT_MANIFESTATION_NONCOMMUTATIVITY_CLOSED_NEGATIVE.md)) |
| S₃ budget symmetry (`N_c = 3` body-diagonal C₃ as the epistemic ℤ/3) | `{J_x, J_y, J_z}` commute — co-measurable, not complementary; same count, wrong kind | FTD-0228 `[CLOSED NEGATIVE]` ([archive](../10_eft_program/archive/closed_negative/AUDIT_SYMPLECTIC_BUDGET_SYMMETRY_CLOSED_NEGATIVE.md)) |
| The Bell wall | local deterministic substrate ⇒ S ≤ 2 (Bell's theorem applies — realism, locality, statistical independence all hold); engine measures S ≈ 1.95–2.00; `S = 2√2` appears only after the complexification step `J_x + iJ_y ↦ ψ`, which **is** an instance of M `[SELECTION]` | [`AUDIT_BELL_ANALYSIS.md`](../07_assessment/AUDIT_BELL_ANALYSIS.md); FTD-0243 §4 |

### 2.4 FC-1 — the framework declines the measurement map `[AXIOM-class declaration — FTD-0255]`

FTD-0243 proves the fork exists: non-commutativity is **logically independent** of P1–P5 — both `{P1..P5} ∪ {M}` and `{P1..P5} ∪ {¬M}` are consistent. The theorem cannot choose the branch; only a commitment can.

> **FC-1 (framework commitment, `[AXIOM]`-class declaration).** *The commutative observable algebra `A₅` is complete: FTD declines the measurement-map import `M`. Non-commutativity, Hilbert-space state vectors, and operator-valued observables are not part of FTD's model of the world. Where quantum mechanics' non-commutative formalism makes structural predictions that differ from the commutative substrate's, FTD predicts the substrate.*

**This is a declaration, not a derivation** — the theorem proves the fork; the commitment picks the branch. Its consequences: measurement is re-located to the observer layer as frame-relative epistemic restriction (§4.1); the wave-face of quantum phenomenology is carried by the classical flux (§5.3); and the structural deviations from the quantum formalism become FTD's falsifiable spine (§6.1, [`SPEC_PREDICTION_LEDGER_DEVIATIONS.md`](SPEC_PREDICTION_LEDGER_DEVIATIONS.md)). What kills FC-1 is stated in §6.2. *Lineage note (framing hygiene): FC-1 places FTD in the ψ-epistemic tradition (Spekkens' epistricted theories) with a classical ontic substrate — a respectable, actively-studied research line — while going beyond it: epistricted theories aim to reproduce a fragment of QM; FTD declines the reproduction target itself.*

### 2.5 The epistemic horizon, honestly split `[THEOREM + OPEN — FTD-0227 PARTIAL]`

What FTD *does* derive about observation ([`AUDIT_SPEKKENS_KNOWLEDGE_BALANCE_PARTIAL.md`](../07_assessment/audits/AUDIT_SPEKKENS_KNOWLEDGE_BALANCE_PARTIAL.md)):

- **The binding half `[THEOREM]`:** a finite internal observer — a subsystem with M pointer states modelling a total of N > M states, itself included — necessarily loses access to part of its own state (classical finite self-reference / pigeonhole; non-circular, no quantum input). Every internal frame carries a **self-blind-spot**.
- **The sharp half `[OPEN]`:** the *Spekkens knowledge-balance* — the blind-spot rotating symmetrically over complementary bases — requires the full symplectic `S₃ = ℤ/2 ⋊ ℤ/3` action. The ℤ/2 is FTD-native (`J² = −I` quarter-conjugacy); the ℤ/3 is **missing** (the `N_c = 3` candidate was apophenia, FTD-0228). Binding-without-sharpness is the framework's honest state.
- **Consequence, flagged post-hoc:** binding-without-sharpness *post-hoc* accounts for the engine's measured detection statistics being Rice/Gaussian rather than Born (§6.1 PL-1, FTD-0199/0200) — a classical restriction without the symmetric balance gives classical-with-noise statistics, not `|ψ|²`. This account was constructed after the measurement; it is an explanation candidate, not a pre-registered prediction.

### 2.6 FC-2, arrow half — the arrow is native `[AXIOM-class declaration — FTD-0256]`

FTD-0253 maps the gap: reversibility is **absent from P1–P5**, and the genuinely *finite* sector of the substrate — the ternary state field — is **irreversible** (manifestation is many-to-one, provably unrecoverable), while the reversible part is the continuous flux, on which the finiteness commitment has no purchase. Finiteness therefore *opposes* a global reversibility postulate ([`FOUND_SPACETIME_FORCING_BOUNDARY.md`](../02_foundations/FOUND_SPACETIME_FORCING_BOUNDARY.md) §6).

> **FC-2 (framework commitment, `[AXIOM]`-class declaration) — arrow half.** *The arrow of time is native: the manifestation/state sector is fundamentally irreversible, and FTD declines global reversibility as a postulate. Reversibility holds only as a sector property of the weakly-coupled flux wave dynamics — and with it everything that rides on reversibility, including the Lorentzian metric (§5.2).*

**This is a declaration, not a derivation** — FTD-0253 proves the postulates do not force reversibility (and FTD's finiteness argues against it); FC-2 commits to the irreversible reading as the framework's official model. The 2nd-order Born-Infeld action term `(Δ_t J)²` keeps its existing `[AXIOM]` tag, now explicitly **scoped to the flux wave sector** rather than read as a global time-reversal symmetry of the world. FTD-0208's `[CLOSED NEGATIVE]` (no exact continuous proper-time budget from the discrete substrate) stands untouched.

---

## §3 · Mathematics

### 3.1 The algebraic spine at its honest count `[THEOREM-grade ×7 + tiered ×2]`

The framework's rigorous mathematical core is stated once, in [`SPEC_ALGEBRAIC_SPINE.md`](SPEC_ALGEBRAIC_SPINE.md), and tiered in [`TRACKER_ONTIC_TRUTH.md`](../07_assessment/core_ledgers/TRACKER_ONTIC_TRUTH.md): **nine numbered results, of which seven are theorem-grade** (the G* identity `G* = Γ(1/4)/Γ(3/4) = 2.95867512…`; the master quadratic `x² − 16G*²x + 16G*³ = 0` with roots `x₊ = 137.0361714582…`, `x₋ = 3.0239639163…`; CM-curve uniqueness at h = 1; the Watson identity `W₃ = G*²/(2π)`; Phase G geometric Coulomb; the (1+i)-tower harmonic invariant `1/y₊ + 1/y₋ = 1`; the field-theoretic characterization of `Q(G*)`), **and two honestly tiered below theorem grade** (coefficient 16 — a value-level identity whose structural *necessity* is `[CONJECTURE]`; Phase J ultralocality — `[THEOREM at L = 2]` only). Per FC-0, the spine is forced by P1–P5 **together with** the ℤ[i] reading. This constitution adds nothing to the spine and re-proves nothing.

### 3.2 Forced versus free — the coupling inventory `[SYNTHESIS]`

The framework's honest map of which numbers its ontology fixes:

| Quantity | Status | Source |
|---|---|---|
| `c = 1/√3` (causal speed) | **Forced** `[THEOREM]` (P4 + CFL on the cubic lattice) | `FOUND_AXIOM_ZERO.md` §2.2(d) |
| `D = 3` | **[SELECTION — declared]** (FTD-0355; arithmetic uniqueness of `2^D·(D−1)! = 16` is `[THEOREM]`, dimension-forcing is `[SELECTION]` — circularity named) | FTD-0355 / FTD-0010 |
| `G*` | **Forced** `[THEOREM]` (given FC-0) — four independent constructions converge | spine Theorem 1; monograph §I.2 |
| `N_c = 3` | **Forced** `[THEOREM]` from lattice topology (four independent routes) — **not** from `x₋` | [`DERIV_NC_FROM_TOPOLOGY.md`](../03_derivations/standard_model/DERIV_NC_FROM_TOPOLOGY.md) |
| `N_base = 4`, `N_eff = 13`, `b₃ = 7` | **Forced** `[THEOREM]` (Moore-neighborhood integers) | FTD-0008 |
| **α** | **Dynamical, not structural.** No FTD-native route forces the readout assembly `(Tr, Det) = (16G*², 16G*³)` — route-invariantly (0/4, FTD-0242; conditional theorem FTD-0243: `𝔉` does not force α unless a binding law W natively realizes `√(G*(4G*−1))`; **K-BIND `[CLOSED THEOREM-NEGATIVE] (FTD-0244)**). The identification `x₊ = 1/α` (1.26 ppm) stays `[STRONGLY MOTIVATED CONJECTURE]` (FTD-0013, OT-5.1) — its uniqueness evidence (FTD-0319 scan, formerly cited as FTD-0189: sole dual-matcher among 2.65 M polynomials) is evidence, not derivation. **The surviving exit is now precisely pinned:** FTD-0314 (carrier-narrowing `[THEOREM]`) proves W must be an external ℤ/2 twist on a G\*-bearing analytic carrier realizing `√(G*(4G*−1))`, and no native object supplies it; FTD **adopts** it as **FC-W** (§3.5) ⇒ `x₊ = 1/α` becomes `[CONDITIONAL THEOREM given W]`, still **not** `[DERIVED]`. | [`AUDIT_ALPHA_OPERATOR_FORCING_ROUTE_INVARIANCE.md`](../07_assessment/audits/AUDIT_ALPHA_OPERATOR_FORCING_ROUTE_INVARIANCE.md); [`AUDIT_RSI_LEG3_CONDITIONAL_THEOREM.md`](../07_assessment/audits/AUDIT_RSI_LEG3_CONDITIONAL_THEOREM.md) |
| `x₋` | **No physical correspondent.** The `x₋  N_c` identification is RETIRED; the pre-registered 25-observable search returned `[CLOSED NEGATIVE]` (FTD-0210) — `x₋` is a coordinate/chirality artifact of the quadratic. | LEDGER FTD-0210 |
| `g_c`, `sin²θ_W = 3/13`, `α_s = 7/59` | `[PARAMETRIC]` terminal — all first-principles routes closed negative | LEDGER FTD-0031/0093 |
| `K_GENESIS = N_c·K_MANIFEST` | engine kinetics scale; threshold dynamics `[IMPOSED]`, EWSB transition `[MEASURED]` §5.1 | FTD-0130/FTD-0250 Phase 0 |

### 3.3 The calibration register `[DECLARED]`

Per the calibration-discipline block of [`docs/SPEC_FTD.md`](../../SPEC_FTD.md) (§ "Lattice  Physical Calibration") and the dimensional map [`SPEC_DIMENSIONAL_MAP.md`](SPEC_DIMENSIONAL_MAP.md):

- **Default gauge: electron-primary** (FTD-0137 §4.5, [`FOUND_ELECTRON_PRIMARY_GAUGE.md`](../02_foundations/FOUND_ELECTRON_PRIMARY_GAUGE.md)) — import `{ℏ, c, m_e}`; the single beyond-universal anchor is `m_e`, and `a_phys = ℓ_P = ƛ_C·Kα¹¹` is **derived** (≈ Planck length to 0.19%, [DERIVED]). FTD-0059/0096 forbid deriving length/mass from Axiom Zero *alone*, so one import is irreducible; electron-primary makes it the measurable `m_e` and derives `ℓ_P`/`G` from it. Legacy Planck-primary declares `a_phys ≡ ℓ_P` exactly instead (a valid gauge).
- `M_REST = m_e` (the rest/inertial/gravitational mass quantum; formerly the mass-anchor role of `K_B`, type-separated in the unified-mass arc) and `t_phys = ℓ_P/(√3·c)` (= t_P/√3 ≈ 3.11×10⁻⁴⁴ s; derived from `a_phys` + `c_lat=1/√3`, corrected 2026-07-08 from `√3·ℓ_P/c`).
- **Discipline:** every *dimensional* prediction is conditional on this register; **dimensionless** predictions (α, mass ratios, mixing angles, the deviation spine of §6) are calibration-invariant and constitute the falsifiable core. Engine-native results quoted in §5/§6 are substrate statements first; their physical readings inherit this conditionality.

### 3.4 The π/G* split is the algebra of FC-2 `[SYNTHESIS]`

The reversible/irreversible divide that FC-2 commits to is already encoded in the framework's own constant-generating structure: the Euler reflection formula's **product branch** `Γ(z)Γ(1−z)` is commutative, yields **π**, and governs the time-reversible (wave/Lagrangian) face; the **ratio branch** `Γ(z)/Γ(1−z) = G*` is order-sensitive, yields **G***, and carries the fractional/diffusive `D^{−1/2}` operator — the arrow ([`DERIV_HEAT_EQUATION_FROM_RATIO.md`](../03_derivations/foundational_mechanics/DERIV_HEAT_EQUATION_FROM_RATIO.md) `[THEOREM]` for the operator content; framing `[SYNTHESIS]`, FTD-0253 §3). FTD's spacetime phenomenology lives on the π-face; its arrow and manifestation sector on the G*-face. FC-2 is the commitment that **the G*-face is fundamental and the π-face sector-scoped** — stated here as the mathematical face of the declaration, not as a derivation of it.

### 3.5 FC-W — the framework adopts the external α-binding law `[AXIOM-class declaration — FTD-0315]`

FTD-0243 proves the fork exists: the operator assembly that would force α is **logically independent** of P1–P5 — both `𝔉 ∪ {W}` and `𝔉 ∪ {¬W}` are consistent. FTD-0314 (the carrier-narrowing theorem, [`AUDIT_W_CARRIER_NARROWING.md`](../07_assessment/audits/AUDIT_W_CARRIER_NARROWING.md), `[THEOREM]`) then proves *what W must be and that the substrate cannot supply it*: the distinguishing surd `√(G*(4G*−1))` is **transcendental over ℚ**, so every finite-symmetry carrier and every native operator (invariants in `Q^ab` or `Q(G*)`) is excluded, and the three natural G\*-bearing analytic carriers (BCC-Watson twist, second Watson, CM period) all close — W is external (~85%; one loophole `[OPEN]`). The theorem cannot pick the branch; only a commitment can.

> **FC-W (framework commitment, `[AXIOM]`-class declaration).** *The substrate is extended by an external order-2 (ℤ/2) twist on a G\*-bearing analytic structure that realizes the degree-2 invariant `√(G*(4G*−1))` and breaks the master-quadratic root-swap `x₊ ↔ x₋`. FTD adopts this binding law W as its α-sector commitment.*

**This is a declaration, not a derivation** — the theorem proves the fork; the commitment picks the branch. Its content is **fully pinned by FTD-0314**: FC-W is not "some α-mechanism" but *exactly* a surd-realizing ℤ/2 twist on an analytic carrier, and the theorem proves no cheaper object (finite group, operator, second Watson period, CM period) can be it. Its consequence: **`x₊ = 1/α` becomes a `[CONDITIONAL THEOREM given W]` / `[CONDITIONAL — DERIVED-GIVEN-IMPOSED]`** — never `[DERIVED]` from the bare substrate.

**FC-W is the framework's first *adopted* import — and that asymmetry is stated, not hidden.** FC-1 and FC-2 *decline* imports (the measurement map M, global reversibility) and thereby *buy* the falsifiable deviation spine (§6.1). FC-W instead *adopts* an import, and on present evidence it does **no work beyond the α-root** — it selects `x₊ = 1/α` and nothing else. It earns full commitment status only if W's carrier also forces independent structural content (currently `[OPEN]`, `AUDIT_W_CARRIER_NARROWING.md` §4). Until then FC-W is a **declared-but-conditional** commitment: honest as a precisely-pinned choice, not a load-bearing derivation, and explicitly **not** a closure of MC-T4.3 (which stays a `[FOUNDATIONAL OBSTRUCTION]` — W is an external axiom, not a substrate theorem). What kills FC-W is stated in §6.2.

---

## §4 · Philosophy

### 4.1 The observer layer: frames, readouts, and the blind spot `[SYNTHESIS + OPEN]`

FTD's account of observation uses the canonical vocabulary of [`REF_REFERENCE_FRAME_VOCABULARY.md`](REF_REFERENCE_FRAME_VOCABULARY.md) exclusively — *reference frame structure* (a structure containing a model of itself), *local reference frame* (canonically: the center of a 27-block), *reference frame projection* (the noumenal 3³ → phenomenal 2³ restriction), *frame-relative readout* (what a structure can know of itself), *frame-relative integration* (updating that readout in time), and *active frame dynamics* (trajectory selection driven by the frame's own readout).

The layered picture: the **substrate frame** (god's-eye, noumenal) holds the full configuration, evolving deterministically under P1–P5 with absolute simultaneity; **internal frames** (phenomenal) are subsystems whose access to the world — and to themselves — is restricted by the reference frame projection, with the **binding self-blind-spot of §2.5 as a theorem**. What QM treats as the measurement problem, FTD treats as the epistemics of internal frames embedded in a classical substrate: *measurement is frame-relative readout under a derived restriction, not a dynamical collapse and not an algebra deformation* (FC-1).

Honest gaps, kept open: the **sharpness** of the restriction (§2.5, `[OPEN]`); the **active-frame threshold** (which configurations realize active frame dynamics — an open structural problem, vocabulary §5.1); and a dedicated formalization of frame self-identification (the GUID+XYZ construction) does not yet exist as a `FOUND_` document (§6.4).

### 4.2 Position among the alternatives — the supersession matrix `[SYNTHESIS; per-cell tags as marked]`

FTD's posture toward the existing interpretive and foundational frameworks is **structural, not rhetorical**: each row below is a respectable research tradition; FTD differs by *declining imports they make* and by *offering deviations where they reproduce the standard formalism exactly*.

| Framework | Ontic substrate | Deterministic? | Dynamically local? | Measurement | Non-commutativity | Metric / spacetime | Arrow of time | Headline imports (not derived internally) |
|---|---|---|---|---|---|---|---|---|
| **Copenhagen QM** | none specified (ψ is not ontic) | no | n/a (no dynamics below ψ) | primitive collapse postulate | **fundamental** | imported (background) | from measurement irreversibility | Hilbert space, Born rule, collapse, classical/quantum cut |
| **Many-Worlds** | universal ψ | yes (unitary) | no 3-space locality of outcomes | branching (decoherence) | **fundamental** | imported (background) | decoherence-thermodynamic | Hilbert space, preferred basis problem, Born weights |
| **RQM (Rovelli)** | relational facts only | no | relational | relative-state actualization | **fundamental** | imported / relational | perspectival | Hilbert space; no observer-independent state |
| **Bohmian mechanics** | particle configuration + ψ | yes | **no** (explicitly nonlocal) | effective (conditional ψ) | fundamental (via ψ) | imported (background) | quantum-equilibrium statistical | Hilbert space, pilot wave, quantum equilibrium |
| **'t Hooft CA interpretation** | classical cellular automaton | yes | yes | template-basis map onto QM | **imported deliberately** (the CA→QM map) | imported | aims at reversible automata | the QM reproduction target itself; superdeterministic correlations |
| **Spekkens epistricted** | classical ontic states | yes (toy) | yes (toy) | epistemic restriction (postulated) | reproduced *fragment* | n/a (toy) | n/a | the knowledge-balance principle (postulated, not derived) |
| **String theory** | strings/branes on a target manifold | quantum | QFT-local | standard QM | **fundamental** | dynamical but presupposes continuum + extra dimensions | thermodynamic | continuum, SUSY, extra dimensions, landscape choices |
| **FTD** | ternary lattice + flux (P1–P5, FC-0) | **yes** (P5) | **yes** (P4) | frame-relative readout; binding restriction `[THEOREM]`, sharpness `[OPEN]` | **declined** (FC-1; independence is `[THEOREM]`) | cone forced `[THEOREM]`; metric emergent-IR, sector-scoped (FC-2); space ⊥ time fundamental | **native** (FC-2; manifestation many-to-one) | FC-0 (ℤ[i] reading); calibrations; the flux field `[SELECTION]`; per-sector physics imports at their LEDGER tags |

Three structural contrasts carry the supersession claim:

1. **Against the QM family (Copenhagen/MWI/RQM/Bohm):** all four import the non-commutative Hilbert-space formalism as fundamental. FTD proves its substrate cannot generate that formalism (FTD-0243 `[THEOREM]`) and **declines** it (FC-1) — so where they reproduce textbook QM exactly, FTD predicts *structural deviations* (§6.1). FTD is thereby **more falsifiable, not more general**: it stakes outcomes they cannot stake.
2. **Against 't Hooft's CA program — the nearest relative:** both posit a deterministic, local, discrete classical substrate. 't Hooft's program *aims to reproduce QM exactly* via a template-basis map (an instance of M) and hopes for reversible automata; FTD declines M and — decisively — declines reversibility too, since its own finite sector argues against it (FTD-0253 §6). Same substrate class, opposite commitments on both forks.
3. **Against string theory:** FTD's structural null-predictions are direct contradictions of string theory's characteristic imports — **no SUSY partners, no extra dimensions, no monopoles** (`[THEOREM]`-grade nulls, §6.1 PL-6). Continued null results at colliders are evidence *for* FTD's ontology and *against* the imports. (Proton stability is **not** among these `[THEOREM]` nulls: FTD-0301 re-tags `τ_proton = ∞` as `[SELECTION]/[BOUNDARY]` — the substrate carries no baryon/B−L current and its own weak channel decays the proton, so a continued proton-decay null is consistent with FTD but not a forced FTD prediction.)

**What FTD has not yet delivered (mandatory honesty row).** FTD currently does **not** derive: the Born rule (FTD-0187 `[OPEN]`; FTD-0199/0200 `[CLOSED NEGATIVE]` for the tested constructions); laboratory Bell violations S > 2 (the observer-layer account, CLAIM.8, is `[SELECTION]`-grade — the substrate bound is S ≤ 2 and lab experiments measure S > 2, an **accepted open burden**, §6.1 PL-2); interference/double-slit phenomenology at observer level; QFT scattering amplitudes; α (**dynamical, not structural** — §3.2; `x₊ = 1/α` stays `[SMC]`); and the sharp epistemic balance (§2.5 `[OPEN]`). The supersession posture is a *program with a falsifiable spine*, not a completed replacement.

### 4.3 Why ontology-first is more falsifiable, not less `[SYNTHESIS]`

The chain Ontology > Logic > Math > Philosophy > Physics > Science would be vacuous if "Science" merely decorated the front of the chain. The constitution's discipline is the reverse: because the early layers are *committed* (P1–P5, FC-0/1/2), the framework cannot retreat when measurements arrive — §6.2 states, in advance, the observations that kill each commitment. A framework that reproduces QM/SR exactly can never be separated from them by experiment; FTD, by declining the imports, **can**. Rejecting falsifiability-as-sole-criterion (the framework's philosophical stance) does not license ignoring measured phenomena: every measured phenomenon either appears in the IR catalog (§5.1), in the deviation ledger (§6.1), or in the not-yet-delivered row (§4.2) — nothing is waved away.

---

## §5 · Physics

### 5.1 The computational EFT — the IR catalog v1 `[per-row tags]`

FTD's physics layer is a **computational effective field theory**: the IR behavior of the substrate as *measured and derived through the engine as instrument*, under the pre-registration discipline of §6.3. The catalog (engine-native statements; physical readings conditional per §3.3):

| # | IR result | Status | Content + source |
|---|---|---|---|
| 1 | **Causal cone** | `[THEOREM]` | `c = 1/√3`, forced by P4; engine demo: cone bit-identical between 2nd-order and 1st-order dynamics (both fronts at 7.211 at t = 8) — `test_spacetime_forcing_demo` 9/9 (FTD-0253) |
| 2 | **Geometric Coulomb (Phase G)** | `[THEOREM]` | `α_r(r, L) = 2·r·G_L(r)` — the emergent static potential *is* the periodic lattice Poisson Green's function, zero free parameters; R² = 1.0000 at L = 384, median 0.07 % residual in the tail ([`DERIV_EMERGENT_COULOMB_GEOMETRIC.md`](../10_eft_program/derivations/DERIV_EMERGENT_COULOMB_GEOMETRIC.md), FTD-0004) |
| 3 | **Lorentz/isotropy recovery** | `[MEASURED]` | anisotropy exponent `p = 4.0008 ± 0.0006` (R² = 1.000000): the rotation-breaking operator scales `δ ∝ k⁴` — strongly irrelevant in the IR ([`AUDIT_LORENTZ_ANISOTROPY.md`](../10_eft_program/archive/campaign_complete/AUDIT_LORENTZ_ANISOTROPY.md)) |
| 4 | **Time dilation / γ-emergence** | `[MEASURED, scoped]` + `[OPEN]` | departure from exact γ vanishes as `R ∝ L⁻¹·⁹⁸ ≈ L⁻²` (∝ k²) on ⟨100⟩ at v ≲ 0.85 (34–94× shrink to L = 193); ultra-relativistic diagonals unconverged at L ≤ 193 `[OPEN]`; clock hypothesis stays `[AXIOM]` ([`ANALYSIS_DYNAMICAL_TIME_DILATION.md`](../03_derivations/foundational_mechanics/ANALYSIS_DYNAMICAL_TIME_DILATION.md), FTD-0252) |
| 5 | **Per-voxel mass gap** | `[THEOREM]` | Gauss constraint + lattice topology force a nonzero per-voxel excitation cost (FTD-0044) |
| 6 | **Cluster mass = N, coefficient k = 1/4** | linear `[DERIVED]`; nonlinear `[OPEN]`; identification `[SMC]` | `mult(A₁g) = 4` in the 27-block `[THEOREM]` → mean-energy coefficient 1/4 at linear level ([`DERIV_K_FROM_OH_A1G_MULTIPLICITY.md`](../03_derivations/foundational_mechanics/DERIV_K_FROM_OH_A1G_MULTIPLICITY.md)); the nonlinear-pipeline upgrade was **reverted by audit** ([`AUDIT_FTD0110_2026-05-27_RESOLUTION.md`](../07_assessment/audits/AUDIT_FTD0110_2026-05-27_RESOLUTION.md)) — empirical drift `k(A) ≈ ¼(1 − 0.030 ln(A/2))`; closure route = Mechanism α multi-block perturbation (FTD-0203) |
| 7 | **Cluster transport inertia = N·M_REST** | `[IMPOSED]`; reduction `[OPEN]` | the engine honors the action's equivalence principle (EP demonstration CI-5: unequal-N clusters free-fall identically while F ∝ N — conditional on the imposed claim); the collective-coordinate reduction is the dynamical twin of row 6's `[OPEN]` bridge (FTD-0250) |
| 8 | **EWSB / condensate** | `[MEASURED]` | sharp first-order manifestation transition at amplitude ∈ (0.6, 0.7); condensate mass gap `m_flux = 0.181`, `m_charge = 0.186` (channel ratio 0.97) (Day-2 campaign, FTD-0106 lineage) |
| 9 | **Confinement signature** | `[MEASURED at an inserted coupling [SELECTION]]` | Wilson-loop area law with string tension σ ≈ 0.209 at β = x₋ (the coupling insertion is a selection, not a derivation) ([`DERIV_YANG_MILLS_CONFINEMENT.md`](../03_derivations/DERIV_YANG_MILLS_CONFINEMENT.md); engine `benchmark_wilson_loops`) |
| 10 | **Classical electrodynamics suite** | `[DERIVED/MEASURED]` | retarded radiation (FTD-0113), Bianchi identities (FTD-0114), boosted Coulomb + lattice Cherenkov pole (FTD-0115), extended sources, Larmor (FTD-0120) — the lattice ED corpus |
| 11 | **Ward identities** | `[MEASURED]` | Gauss residual ≤ 10⁻⁸ (matched-stencil CG Poisson) |
| 12 | **Gravity ratio** | `[DERIVED with 1 flagged interpretive step]` | `α_G(e,e) = (m_e/m_P)² ≈ 1.745×10⁻⁴⁵` predicted vs 1.752×10⁻⁴⁵ measured (0.38 %) — via Phase G + cluster mass + the clock hypothesis (the flagged `[AXIOM]` step) ([`DERIV_NEWTON_FROM_SUBSTRATE.md`](../03_derivations/gravity_and_cosmology/DERIV_NEWTON_FROM_SUBSTRATE.md), FTD-0131) |
| 13 | **Hydrogen spectrum** | `[MEASURED — engine benchmark]` | bound-state level structure `1/n²` to < 0.001 % in the engine benchmark suite (`benchmark_engine_theory`) |
| 14 | **Bell bound** | `[THEOREM + MEASURED]` | substrate S ≤ 2 (FTD-0243 + Bell's theorem); engine S ≈ 1.95–2.00; ternary detection loophole gives S ≈ 3.6 at ~49 % efficiency — a known artifact, not a violation ([`AUDIT_BELL_ANALYSIS.md`](../07_assessment/AUDIT_BELL_ANALYSIS.md)) |
| 15 | **Detection statistics** | `[CLOSED NEGATIVE for Born]` + `[NUMERICAL FACT]` | threshold-crossing event rates follow Rice's upcrossing law (R² = 0.9923) not Born `|J|²` (R² = 0.7137) in the pre-registered 6-neighbour construction (FTD-0200; companion FTD-0199) |

### 5.2 FC-2, metric half — emergent relativity, sector-scoped `[AXIOM-class declaration — FTD-0256]`

The relativistic structure decomposes exactly as FTD-0253 proved:

- **Forced:** the causal cone (`[THEOREM]`, row 1) — for *any* local dynamics, reversible or not.
- **Posited:** the second-order wave action `(Δ_t J)²` (`[AXIOM]`, scoped to the flux sector per FC-2's arrow half), which carries clocks, `γ`, and Lorentzian structure.
- **Measured:** the IR emergence — anisotropy dying as `k⁴` (row 3), γ-departure dying as `L⁻²` on the principal axis (row 4), the quadrature clock winding at `ω(k)` (FTD-0251).

> **FC-2 — metric half.** *FTD commits to the Lorentzian metric as an **emergent IR property of the weakly-coupled flux wave sector only** — not a postulate of the substrate. Space (P1) and time (P2) are fundamentally separate; Minkowski mixing is the IR shadow of second-order flux dynamics; the substrate frame's simultaneity is absolute (neo-Lorentzian positioning — empirically respectful of every SR confirmation in the IR, structurally committed to a preferred foliation at the substrate level). UV departures from exact Lorentz invariance are therefore **native predictions**, not embarrassments (§6.1 PL-4/PL-5).*

**Declaration, not derivation:** FTD-0253 maps the boundary; FTD-0252 measures the IR approach on its scoped axis; FC-2 picks the reading. The clock hypothesis keeps its coordinate-level `[AXIOM]` tag with measured IR-emergent support (⟨100⟩, `R ∝ L⁻²`) — nothing stronger.

### 5.3 Quantum phenomenology, scoped `[SYNTHESIS]`

Under FC-1, the framework's account of "quantum" phenomena splits into three honest strata:

1. **Native (substrate, derived/measured):** the wave face — superposition, dispersion, the quadrature clock, cosine correlations from the Gauss constraint ([`DERIV_BELL_COSINE_FROM_GAUSS.md`](../03_derivations/quantum_mechanics/DERIV_BELL_COSINE_FROM_GAUSS.md)), singlet anticorrelation from void events ([`DERIV_SINGLET_FROM_VOID_EVENT.md`](../03_derivations/quantum_mechanics/DERIV_SINGLET_FROM_VOID_EVENT.md)) — all classical-flux phenomena, S ≤ 2.
2. **Observer-layer (partially derived):** the epistemic restriction of internal frames — binding `[THEOREM]`, sharpness `[OPEN]` (§2.5). This is where FTD's replacement for the measurement problem lives.
3. **Declined (FC-1):** the non-commutative formalism itself. Where the formalism's predictions require M (Born statistics, S > 2 correlations, quadrature complementarity), FTD stakes the substrate's side as a prediction (§6.1) — and openly carries the burden that several such phenomena are experimentally established (the §4.2 honesty row): the framework's task is to produce them *at the observer layer or not at all*, and it accepts the falsification risk.

### 5.4 The forward program `[OPEN PROGRAM]`

What completing the computational EFT requires (the open queue at sector level lives in [`SPEC_OPEN_MATH_BY_SECTOR.md`](SPEC_OPEN_MATH_BY_SECTOR.md) and [`TRACKER_OPEN_ITEMS.md`](../07_assessment/core_ledgers/TRACKER_OPEN_ITEMS.md)):

- **S_eff[J, s] after blocking** — the explicit native effective action (deliverable R3 of the EFT roadmap; blocking-diagonal identities `M_JJ = 16`, `M_J⁴ = 256` are already `[THEOREM]`).
- **Multi-L Wilson coefficients** — extend the operator-mixing matrix beyond L = 16–32 to L ∈ {64, 96, 128} to resolve marginal/irrelevant tiers.
- **Fixed-point classification** and a pre-declared continuum-scaling protocol.
- The Born/observer-layer program: the sharpness gap (§2.5) and the active-frame threshold (§4.1).

---

## §6 · Science

### 6.1 The deviation spine (summary) `[registry — per-row tags in the companion ledger]`

The framework's falsifiable core: six structural deviations from the QM/SR formalism, fully specified with protocols, scope caveats, and kill conditions in [`SPEC_PREDICTION_LEDGER_DEVIATIONS.md`](SPEC_PREDICTION_LEDGER_DEVIATIONS.md) (FTD-0258):

| PL | Observable | FTD (engine-native) | QM/SR formalism |
|---|---|---|---|
| PL-1 | Threshold detection statistics | Rice upcrossing law (R² = 0.9923 vs Born's 0.7137, scoped construction) | Born `\|ψ\|²` |
| PL-2 | CHSH on M-free correlations | S ≤ 2 (structural); engine 1.95–2.00 — **with the lab-Bell burden stated** | S = 2√2 at optimal settings |
| PL-3 | Quadrature compatibility | all quadratures co-measurable (`[q,p] = 0`; leakage ~10⁻¹⁶) | conjugate incompatibility (ℏ) |
| PL-4 | Moving-clock rate | γ only IR-emergent: departure `∝ L⁻²` (⟨100⟩, v ≲ 0.85); UV bend *below* γ | exact γ at all scales |
| PL-5 | Isotropy | native UV anisotropy, dying as `δ ∝ k⁴` (p = 4.0008 ± 0.0006) | exact rotational invariance |
| PL-6 | Structural nulls | no monopoles; no SUSY; no extra dimensions (`[THEOREM]`); τ_proton = ∞ `[SELECTION]/[BOUNDARY]` (FTD-0301 — not a forced null) | (model-dependent in competitors) |

### 6.2 Falsification criteria for the commitments themselves

The constitution stakes its own commitments, in advance:

**FC-1 is killed by any of:**
1. A substrate-native pair of observables with `[A, B] ≠ 0` derived from P1–P5 (would refute FTD-0243's premise — also the single most direct refutation of this constitution).
2. A P1–P5 derivation of Born statistics (would refute the binding-without-sharpness account and collapse the PL-1 deviation).
3. A substrate-native (M-free) correlation experiment exceeding S = 2 beyond numerical error.

**FC-2 is killed by any of:**
1. Exact γ (or exact Lorentz invariance) at finite L off the IR limit — i.e., a measured *absence* of the predicted UV corrections where the lattice requires them (PL-4/PL-5 returning null at finite scale).
2. A P1–P5 derivation of global reversibility (would refute FTD-0253's boundary).
3. Failure of the measured IR approach: the `L⁻²` law breaking on ⟨100⟩ at larger L, or the k⁴ anisotropy decay reversing — emergence claims that stop emerging.

**FC-0 is killed by:** an inequivalent reading of the lattice symmetry producing the same spine (uniqueness failure), or a spine theorem failing under FC-0 (soundness failure).

**FC-W is killed by any of:**
1. A native carrier IS exhibited — a forward-derived substrate object realizing `√(G*(4G*−1))` with a *forced* ℤ/2 (the FTD-0314 loophole closes *positive*). Then W is not external, FC-W is **superseded by a derivation**, and `x₊ = 1/α` upgrades toward `[SELECTED/DERIVED]` — the one refutation FTD would welcome.
2. The narrowing theorem fails — G\* shown algebraic, or `G*(4G*−1)` a square in `Q(G*)` (would collapse the degree-2 extension; would also refute spine Theorem 9 / Chudnovsky).
3. `x₊ = 1/α` decisively falsified as an identification (α measured to disagree beyond the tree-level tolerance), removing FC-W's sole payoff.

*(Physical-scale honesty: under the §3.3 calibration, PL-4/PL-5 UV effects are Planck-suppressed; their laboratory accessibility is bounded by current Lorentz-violation searches. The engine-native versions are testable now; the physical versions are conditional and long-horizon. The ledger states both.)*

### 6.3 Methodology as institution `[SYNTHESIS]`

The framework's epistemics are operational, not aspirational: **pre-registration with SHA-256 hash-locks and git tags before measurement** (manifest: `REF_PREREGISTER_MANIFEST.md`); **the golden-tick gate** (bit-exact engine hash `0xc13713f0e11a96da` @ L = 17) guarding every physics-bearing change; **independent adversarial review** of campaign verdicts (the FTD-0252 reviewer catches are the recent exemplar); **the LEDGER** as single source of truth with append-only tag history; and **the anti-target discipline** (no near-miss scans, no substitution identities, no derivation-labeling of parametric insertions). This document was produced under those rules: it contains no new measurement and no promotion.

### 6.4 Queued arcs (explicitly not part of this constitution's critical path)

the surviving FTD-0314 loophole (a new forward-derived period — K-BIND's last door; `AUDIT_W_CARRIER_NARROWING.md` §4); the ℤ/3 sharpness gap (§2.5); the FTD-0110 nonlinear bridge via Mechanism α (FTD-0203); the FTD-0250 collective-coordinate reduction; FTD-0252 v3 follow-ups (sign-crossing-robust exponent fit; L = 257 diagonals; git tags); the GUID+XYZ frame-self-identification `FOUND_` doc; the FTD-0189FTD-0243 LEDGER id double-booking cleanup; the FTD-0248 "Postulate Six" title disambiguation; the `docs/SPEC_FTD.md` editorial alignment; `SPEC_DOCTRINE_LEDGER.md` v1.5 refresh carrying the FC register.

---

## §7 · Governance

1. **Amendment rule.** Framework commitments change only by a new `[AXIOM]`-class LEDGER row with full `tag_history` superseding the old one; postulates P1–P5 change only by declaring a successor framework. No document, including this one, may promote a claim — promotions happen in the LEDGER with evidence, and this constitution then *follows*.
2. **Precedence.** LEDGER > this constitution > all other prose (per §0.3).
3. **Registry of rows declared here:** FTD-0254 (this document, `[SYNTHESIS]`); FTD-0255 (FC-1, `[AXIOM]`-class); FTD-0256 (FC-2, `[AXIOM]`-class); FTD-0257 (two-field formalization, `[SYNTHESIS + SELECTION]`); FTD-0258 (deviation ledger, `[SYNTHESIS]` registry); FTD-0314 (carrier-narrowing theorem, `[THEOREM]`); FTD-0315 (FC-4 = FC-W, `[AXIOM]`-class). FC-0 is carried inside FTD-0254/FTD-0249 (the ℤ[i]-reading honesty correction), not as a separate row. **The act-count arc (FTD-0322–0327) reconciled its "proposed FC-4 (δ-act)" INTO FC-W** — the same commitment (both declare the selection of `√(G*(4G*−1))`); no separate FC was minted, and `DRAFT_FC4_DELTA_ACT_DECLARATION.md` is FC-W's act-of-intent reading.
4. **Unchanged by this document:** FTD-0013 `[STRONGLY MOTIVATED CONJECTURE]`; MC-T4.3 `[FOUNDATIONAL OBSTRUCTION]`; FTD-0208 `[CLOSED NEGATIVE]`; FTD-0242/0243 boundaries; the spine count; every `[CLOSED NEGATIVE]` in the corpus. **Nothing promoted.**
