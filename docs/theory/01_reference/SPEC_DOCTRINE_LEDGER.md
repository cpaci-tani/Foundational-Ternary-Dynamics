# SPEC · FTD/FQCR Doctrine Ledger v1.4

**Tag:** [REFERENCE] — single-page status map. Per-element tags within (see §0 status key + §13 audit).
**Date:** 2026-05-20
**Version:** 1.4 (ontic-system v0.2 intake reconciliation; branch-compliance/Yilmaz gravity fenced per FTD-0184; no tag promotion).
**LEDGER:** FTD-0145 [SYNTHESIS] — claim-aggregation roll-up; introduces no new theorems.
**Companion docs:**
- [`SPEC_ALGEBRAIC_SPINE.md`](SPEC_ALGEBRAIC_SPINE.md) — canonical 9-theorem reference (theorem-only, physics-free)
- [`SPEC_FQCR.md`](SPEC_FQCR.md) — FQCR capstone reference (Models I–V)
- [`SPEC_MATH_FIRST_ONTOLOGY.md`](SPEC_MATH_FIRST_ONTOLOGY.md) — canonical math-first ontology / readout ordering principle
- [`SPEC_PHYSICS_BRIDGE.md`](SPEC_PHYSICS_BRIDGE.md) — physics-bridge synthesis (FTD-0121, structural-uniqueness scans)
- [`SPEC_ALPHA_READOUT_CONTRACT.md`](SPEC_ALPHA_READOUT_CONTRACT.md) — MC-T4.3 operational-readout closure contract
- [`SPEC_OPEN_MATH_BY_SECTOR.md`](SPEC_OPEN_MATH_BY_SECTOR.md) — 18-item bridge-complete roadmap (Phase 2 hardening targets)
- [`../07_assessment/TRACKER_ONTIC_TRUTH.md`](../07_assessment/TRACKER_ONTIC_TRUTH.md) — bedrock truth-tier tracker (T1–T5; OT-N.M IDs)
- [`../07_assessment/LEDGER.md`](../07_assessment/LEDGER.md) — atomic per-claim provenance (FTD-NNNN IDs)

---

## §0 · What this document is and is not

**This document IS:** a single-page status map, organized as fourteen sections covering FTD's algebraic spine (§1–6), candidate physics-bridge sectors (§7–12), a non-circularity audit (§13), and a compressed forward roadmap (§14). Each row points back at a canonical source — `LEDGER` row, `TRACKER_ONTIC_TRUTH` tier, `SPEC_*` section, or proof script — and carries the **same epistemic tag the source carries**. The doctrine is a roll-up.

**This document IS NOT:** a derivation, a new tracker tier, a replacement for `LEDGER.md` or `TRACKER_ONTIC_TRUTH.md`, or a vehicle for tag promotion. If a reader believes this document promotes a claim above its canonical tag, they have found a bug — please file an issue and cite the source row.

**Why the doctrine exists.** FTD's canonical claim infrastructure has accumulated nine theorems, ~150 LEDGER rows, fifteen T1–T5 tracker entries, an 18-item math-completion checklist, and a five-model FQCR capstone. The doctrine compresses this into a single navigation surface that exposes which derivation chains are non-circular, which are imported scaffolding, and which are pending hardening. It prevents cycling — repeatedly re-litigating the same claim because the per-claim status was buried in a 700-line ledger.

**v1.3 physicist consolidation.** The 2026-05-18 intake of `complete_ftd_chain_v1.md` adds no canonical claim and no new theorem. It sharpens the physics-facing reading: FTD is presently defensible as a finite-invariant algebraic reconstruction program whose strongest physical conjecture is the operational identification of the master-quadratic/FQCR dominant branch with the electromagnetic coupling. The mathematical object is solid; the physical readout is the open problem. `SPEC_MATH_FIRST_ONTOLOGY.md` now names this ordering explicitly: primitives -> invariants -> admissible readouts -> operational physics.

**v1.4 ontic-system reconciliation.** The 2026-05-20 intake of `FTD_Ontic_System_v0_2_Agent_Brief.md` records useful external finite-closure/FQCR vocabulary but does not import a new ontology or gravity sector. Per FTD-0184, the branch-compliance exponential readout metric is the closed-negative Yilmaz route. The surviving gravity task is substrate-side strong-field GR / Schwarzschild-Kerr-horizon derivation, not reusing the exponential metric as a shortcut.

---

### §0.1 · Status key (canonical LEDGER tags)

This doctrine uses canonical tags exclusively. The right-hand column is what you'll see in `LEDGER.md`.

| Tag                             | Meaning                                                                                                            |
|---------------------------------|--------------------------------------------------------------------------------------------------------------------|
| **[THEOREM]**                   | Rigorously proven from FTD axioms or external classical theorems with explicit citation. Cannot be wrong without arithmetic mistake. |
| **[AXIOM]**                     | Structural postulate; not derivable.                                                                               |
| **[DERIVED]**                   | Established from axioms / prior theorems by an explicit chain reproduced in the cited doc; weaker than [THEOREM] when the chain has non-trivial assumptions. |
| **[SELECTION]**                 | Argued from consistency or naturalness; not uniquely proven. Reviewer expectation: critique the consistency argument. |
| **[STRONGLY MOTIVATED CONJECTURE]** | [CONJECTURE] with substantial structural and/or empirical evidence (uniqueness scan, multi-route convergence, sub-ppm match) but no derivation chain. |
| **[CONJECTURE]**                | Proposed interpretation requiring validation; weaker than [SMC] (no structural-uniqueness backing).                |
| **[NUMERICAL FACT]**            | Verified by exhaustive computation across an explicitly stated finite domain. NOT a structural theorem.            |
| **[IMPORTED]**                  | Standard physics or external mathematics adopted as bridge or scaffolding. May carry [PARAMETRIC] when the import provides the formula and FTD provides the numbers. |
| **[PARAMETRIC]**                | Standard physics formula filled with FTD constants; numbers fit but mechanism is borrowed. Not a derivation.       |
| **[IMPOSED]**                   | Calibration declaration (e.g., `a_phys ≡ ℓ_P`, `K_B = m_e`).                                                       |
| **[OPEN]**                      | Unresolved question; research opportunity.                                                                         |
| **[CLOSED NEGATIVE]**           | Hypothesis tested and falsified; preserved for provenance to prevent re-attempt.                                   |
| **[SYNTHESIS]**                 | Cross-document integration of multiple lower-level claims into a single externally-defensible package; not a new theorem. |

### §0.2 · v1.2 → canonical tag map

Earlier drafts of this doctrine used a parallel vocabulary. For continuity, the equivalences are:

| v1.2 draft tag                  | Canonical tag used here                                                                                            |
|---------------------------------|--------------------------------------------------------------------------------------------------------------------|
| THEOREM                         | [THEOREM]                                                                                                          |
| SELECTION / PRINCIPLE           | [SELECTION]                                                                                                        |
| NUMERICAL RECONSTRUCTION        | [STRONGLY MOTIVATED CONJECTURE] (when structural evidence exists) or [NUMERICAL FACT] (raw scan)                   |
| CONJECTURE / PHYSICAL ID        | [CONJECTURE] or [STRONGLY MOTIVATED CONJECTURE] (per evidence level)                                               |
| IMPORTED PHYSICS                | [IMPORTED] or [PARAMETRIC] (when the import is filled with FTD numbers)                                            |
| OPEN / HARDENING                | [OPEN]                                                                                                             |
| REJECTED / TOO STRONG           | [CLOSED NEGATIVE]                                                                                                  |
| CONDITIONAL THEOREM             | [THEOREM, conditional on X] (with X named)                                                                         |

---

### §0.3 · Physicist-facing consolidation from `complete_ftd_chain_v1.md`

The useful content of the external chain draft is the following blackboard-safe posture:

1. **Defensible core.** FTD has a finite algebraic spine: quarter conjugacy, `G*`, finite-N convergence, determinant-one recurrence, Casimir invariant, transfer-matrix restatement, and the master quadratic. These are mathematical claims with explicit proof anchors.
2. **Defensible physical observation.** The master-quadratic roots land near `1/alpha` and `N_c` with strong structural-uniqueness evidence. This supports `[STRONGLY MOTIVATED CONJECTURE]`, not `[THEOREM]`.
3. **Not yet defensible as derivation.** QED, Dirac, Standard Model, flavor, confinement, full gravity, Born-rule, and noetic/memory-clock layers do not currently follow from the FTD substrate. They are `[IMPORTED]`, `[PARAMETRIC]`, `[SELECTION]`, `[CONJECTURE]`, or `[OPEN]` as tagged below.
4. **Physics bottleneck.** The central missing object is an operational readout rule: why the algebraically distinguished branch is the electromagnetic coupling measured by matter. This is MC-T4.3. More algebraic near-miss scanning is not the main path forward.
5. **External claim discipline.** The strongest honest public sentence is: FTD constructs a rigid finite algebraic object whose dominant branch is a structurally unique candidate for `1/alpha`; it does not yet derive the physical coupling without an added readout principle.

This subsection is a consolidation rule for future writing. It does not override any per-claim row below.

---

# §1 · Core quarter-duality layer

**Primitive.** The conjugacy operator `J` with `J² = −I` generates a cyclic group of order 4 with elements `{I, J, −I, −J}` and eigenvalues `±i = e^{±2πi/4}`. This is the algebraic source of the quarter split that pervades the FTD spine: the i-cycle ontology, the framework integer `N_base = 4`, the (1+i)-tower of Theorem 8, and FQCR Model I.

**Quarter sectors.** The eigenphases `1/4` and `3/4` are the two non-trivial residue classes mod 4. Restricted to primes, they are the split and inert prime classes of `Z[i]` (Fermat's two-square theorem). G* is the regularized asymmetry between them (FQCR Model I, FTD-0141; OT-1.7).

| Claim                                                                | Tag                              | Source                                                                                  |
|----------------------------------------------------------------------|----------------------------------|-----------------------------------------------------------------------------------------|
| `J² = −I ⇒ J⁴ = I`                                                   | [THEOREM]                        | Standard linear algebra; SPEC_FQCR.md §1 Def 1                                          |
| Quarter eigenphases `1/4`, `3/4` (residue classes mod 4)             | [THEOREM]                        | SPEC_FQCR.md §1 Def 2; OT-1.7                                                            |
| `J` as ontic primitive of finite conjugacy                            | [SELECTION]                      | SPEC_FTD.md §1.1 (graded monism); FOUND_AXIOM_ZERO.md                  |
| Physical reality is finite-trace dynamics (interpretive ontology)    | [CONJECTURE — interpretive]      | Discharged algebraically by FQCR Model II (FTD-0142, OT-1.8); does not commit ontology |

---

# §2 · G* layer

**Definition.** `G* := Γ(1/4)/Γ(3/4) = Γ(1/4)²/(π√2) ≈ 2.95867512...`

**Notational warning.** G* (project canonical, ≈ 2.959) and the Bernoulli/Gauss lemniscate constant `ϖ = Γ(1/4)²/(2√(2π)) ≈ 2.622` are sometimes both called "the lemniscate constant" in informal usage. They are distinct: the master quadratic produces `x_+ = 137.036` only at `G* = 2.959`, not at `ϖ`. See `SPEC_ALGEBRAIC_SPINE.md` §1 (FTD-0117 typo audit).

| Claim                                                                | Tag                              | Source                                                                                  |
|----------------------------------------------------------------------|----------------------------------|-----------------------------------------------------------------------------------------|
| `G* = Γ(1/4)/Γ(3/4)`                                                 | [THEOREM]                        | SPEC_ALGEBRAIC_SPINE.md §1 Theorem 1; OT-1.2; FTD-0001                                  |
| Reflection identity `Γ(1/4)·Γ(3/4) = π√2`                            | [THEOREM]                        | Γ-function reflection                                                                   |
| Finite-N attractor `G_N* → G*` at `O(1/N²)`                          | [THEOREM]                        | SPEC_FQCR.md §2 Prop 2; FTD-0142; OT-1.8; verified by `proof_fqcr_convergence.py`        |
| Operator-theoretic provenance `det_ζ D_{3/4}/det_ζ D_{1/4} = G*`     | [THEOREM]                        | SPEC_FQCR.md §2 Prop 1; FTD-0141; OT-1.7; via Lerch's formula                            |
| `G* > 2 ⇒ ∃ unique χ_G > 0 with G* = 2 cosh(χ_G)`                    | [DERIVED — parametric reformulation] | Inline: `cosh` is strictly monotone on `[0, ∞)` with image `[1, ∞)`; `G* > 2` gives unique `χ_G > 0`. Notational only; no new physics. |
| `q* := e^{−χ_G} = (G* − √(G*² − 4))/2`                               | [DERIVED — parametric reformulation] | Inline: `2 cosh χ_G = G*` ⇒ `e^{χ_G} + e^{−χ_G} = G*` ⇒ `q*` is the smaller root of `z² − G*z + 1 = 0`. Same root as Möbius reduction §3. |
| `W_BCC = G*²/(2π)` (Watson identity)                                 | [THEOREM, conditional on Watson 1939] | SPEC_ALGEBRAIC_SPINE.md §5 Theorem 5; OT-2.1; verified at 100-digit precision in PARI |
| Distinction `W_BCC ≠ G*` (one-loop / tadpole prefactor, not bridge constant) | [THEOREM]                | ../08_structural/DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md                                   |

---

# §3 · Finite trace mechanics

**Primitive recurrence (FQCR Model III).** `u_{m+1} + u_{m−1} = s_m u_m`. Transfer matrix
`T_m = [[s_m, −1], [1, 0]]`, `det T_m = 1`. Casimir invariant
`I_m := u_m² + u_{m−1}² − s u_m u_{m−1}`. Projective Möbius reduction `z_{m+1} = s_m − 1/z_m`.
For constant `s`, fixed-point quadratic `z² − sz + 1 = 0` with reciprocal roots `z_+ z_− = 1`.

| Claim                                                                | Tag                              | Source                                                                                  |
|----------------------------------------------------------------------|----------------------------------|-----------------------------------------------------------------------------------------|
| Symmetric recurrence `u_{m+1} + u_{m−1} = s_m u_m`                   | [THEOREM]                        | SPEC_FQCR.md §1 Def 3 (after reparameterisation)                                         |
| `det T_m = 1`                                                        | [THEOREM]                        | Direct: `det T_m = s_m·0 − (−1)·1 = 1`                                                   |
| Casimir invariant `I_m = u_m² + u_{m−1}² − s u_m u_{m−1}` conserved  | [THEOREM]                        | SPEC_FQCR.md §2 Prop 3                                                                   |
| Projective map `z_{m+1} = s_m − 1/z_m`                               | [THEOREM]                        | SPEC_FQCR.md §2 Prop 4                                                                   |
| Regime split `\|s\| < 2` / `s = 2` / `s > 2` (oscillatory / null / hyperbolic) | [THEOREM]                | Discriminant of `z² − sz + 1` is `s² − 4`; sign analysis is direct                       |
| `\|s\| < 2` reading as wave/phase regime                             | [SELECTION — physical identification] | Mode of FQCR finite trace; reading is interpretive                                  |
| `s = 2` as null/inertial regime                                      | [SELECTION — physical identification] |                                                                                      |
| `s > 2` as mass-gap / branch-selection regime                        | [SELECTION — physical identification] |                                                                                      |
| `μ² := s − 2` (mass parameter from regime offset)                    | [SELECTION — bridge identification]   | Hardening target: substrate-derive a mass observable from the regime parameter `s`. Distinct from MC-T3.4 (Bridge Functional `M(x_+, x_-)` arithmetic-mean / FTD-0095) — both are mass-from-algebra identifications but use different algebraic input (regime parameter `s` here vs root pair `(x_+, x_-)` for MC-T3.4); no canonical cross-link. |

---

# §4 · Quarter-rotation split geometry

**Quarter rotation.** Under `y ↦ Jy` with `J² = −I`, the Euclidean form `x² + y²` becomes `x² − y²` — Lorentzian signature on a 2-plane, light-cone boundary at `x² − t² = 0 ⇔ x = ±t`. The lemniscatic kernel `cos(2θ) = cos²θ − sin²θ = x² − y²` on the unit circle realises the same algebraic structure radially.

**Honest scoping.** The statement *"the twisted circle literally becomes the Bernoulli lemniscate"* is **REJECTED / TOO STRONG**. The lemniscate `r² = a² cos(2θ)` is one specific *radial* realisation of the split kernel; the algebraic identity `x² + y² ↦ x² − y²` does not by itself produce the lemniscate without additional radial choice.

**G*-hyperbola point.** From the Möbius fixed-point quadratic `z² − sz + 1 = 0` (§3) with `s = G*`, the positive-discriminant point is
`P_G = (G*/2, √(G*² − 4)/2)` (substituting `x = G*/2`, solving for `y`).

| Claim                                                                | Tag                              | Source                                                                                  |
|----------------------------------------------------------------------|----------------------------------|-----------------------------------------------------------------------------------------|
| `x² + y² ↦ x² − y²` under `y ↦ Jy`, `J² = −I`                        | [THEOREM]                        | Direct algebra                                                                           |
| Light-cone boundary `x² − t² = 0 ⇒ x = ±t`                           | [THEOREM]                        | Direct algebra                                                                           |
| Trig identity `cos(2θ) = x² − y²` on the unit circle                  | [THEOREM]                        | External; standard double-angle formula                                                  |
| Bernoulli lemniscate `r² = a² cos(2θ)` as one radial realisation     | [SELECTION — geometric reading]  | One of many radial realisations of the split kernel                                      |
| "Twisted circle becomes lemniscate" as forced consequence            | **[CLOSED NEGATIVE]**            | Algebra does not force this realisation                                                  |
| `P_G = (G*/2, √(G*²−4)/2)` is the positive-discriminant `s=G*` point | [DERIVED — corollary, inline]    | Substitute `x = G*/2` into `z² − G*z + 1 = 0` and solve. Notational only.               |

---

# §5 · FQCR-EM / α⁻¹ branch quadratic

This is the most heavily annotated section because the physical identification of `α⁻¹` is the framework's most-cited (and most-interrogated) claim.

**Modular shape factor (FQCR §3.1, §3.3).** Define
`Ψ_N(t) := ∏_{n=1}^N (1 − Q^{4n})^6 / (1 − Q^{3n})^2`, `Q := e^{−2πt}`.
The exponent quadruple `(k, d; ℓ, m) = (4, 6; 3, 2)` is a structural choice; pre-registered uniqueness scan in `PREREG_FQCR_QUOTIENT_UNIQUENESS_v1.md` (FTD-0143, awaiting execution). Define
`A_N(t) := (1/3) d/dt log Ψ_N(t)` (anomaly pressure),
`λ_N(4it) := (θ_{2,N}(4it) / θ_{3,N}(4it))⁴` (truncated modular lambda),
`R_N(t) := 1 + λ_N(4it) + A_N(t)` (combined renormalisation).

**Branch quadratic (FQCR Model V, Prop 5).**
`x² − 16 (G_N*)² x + 16 (G_N*)³ R_N(t) = 0`
At `R_N(1) = 1` and `N → ∞`, this reduces *exactly* to the spine master quadratic `x² − 16 G*² x + 16 G*³ = 0` (Theorem 2; OT-1.1; FTD-0001). *(Note: prior reference to FTD-0014 here is removed — that LEDGER row, which carried the `x_- ↔ N_c` identification, was retired per v1.4 §5 and removed in commit `ca7eb61`.)*

**Self-dual readout.** `t = 1` is the fixed point of the modular involution `t ↔ 1/t`. The base coupling
`α_FTD⁻¹ := lim_{N→∞} α_N⁻¹(1) ≈ 137.035999177`
where `α_N⁻¹(t) = 8(G_N*)² + 4(G_N*)^{3/2} √(4G_N* − R_N(t))` is the dominant root.

| Claim                                                                | Tag                              | Source                                                                                  |
|----------------------------------------------------------------------|----------------------------------|-----------------------------------------------------------------------------------------|
| `Ψ_N(t)` exponent-quadruple form                                     | [SELECTION]                      | SPEC_FQCR.md §3.1 (pending FTD-0143 uniqueness scan)                                    |
| `(4,6;3,2)` interpretation as primitive antisymmetric / projected transverse sectors | [SELECTION]              | SPEC_FQCR.md §3.1                                                                        |
| `A_N = (1/3) d/dt log Ψ_N` once `Ψ_N` is selected                    | [DERIVED]                        | SPEC_FQCR.md §3.3                                                                        |
| `λ_N(4it)` shape term as truncated modular lambda                    | [SELECTION]                      | SPEC_FQCR.md §3.3                                                                        |
| `R_N = 1 + λ_N + A_N` additive combination                           | [SELECTION]                      | SPEC_FQCR.md §3.3 (one of several plausible combinations)                                |
| Coefficient 16 = `\|Aut(E)\|²` for `E: y² = x³ − x`                  | [CONJECTURE] (T4: value-equality holds, structural necessity unproven) | SPEC_ALGEBRAIC_SPINE.md §4 Theorem 4; OT-4.1 (T4 — true at value level; structural identification conjectural) |
| `t = 1` fixed under modular involution                                | [THEOREM]                        | Direct: `1/t = t ⇔ t² = 1 ⇔ t = ±1`                                                     |
| Base coupling evaluated at `t = 1`                                    | [SELECTION — physical principle] | SPEC_FQCR.md §3.2 (a-priori interpretation of `t` is open)                               |
| Branch quadratic `x² − 16(G_N*)²x + 16(G_N*)³R_N(t) = 0` (notational) | [THEOREM — notational identity] | SPEC_FQCR.md §2 Prop 5; reduces to spine master quadratic at `R_N(1)=1, N→∞`             |
| `α_FTD⁻¹ ≈ 137.035999177` at `t = 1`                                  | [STRONGLY MOTIVATED CONJECTURE]  | OT-5.1; FTD-0013. Structural evidence: ~4×10⁵:1 Bayes (OT-3.3, 2.87M-poly scan with 0 Eisenstein dual-matchers); 63-discriminant Γ-product null at `h ≥ 2` (OT-3.2); Z[i] structural unification of CM Aut count and tower level k=4 (OT-1.5, FTD-0122). |
| Identification with physical `α⁻¹` (CODATA 2022 `137.035999177(21)`)       | [STRONGLY MOTIVATED CONJECTURE]  | OT-5.1; agreement 1.26 ppm; not [DERIVED] absent a non-action injection mechanism (MC-T4.3, foundational obstruction). |

---

# §6 · Minimal operator stack

**Current canonical stack (FQCR §2).**
`T_N(t) = (L_{1/4,N}, L_{3/4,N}, Ψ_N(t), λ_N(4it), T_N(t))`
with `G_N* = (N+1)^{−1/2} · det L_{3/4,N} / det L_{1/4,N}` (Lerch's formula, FTD-0141), `κ_N(t) = R_N(t)/(16 G_N*)`, and the transfer matrix
`T_N(t) = [[1, −κ_N(t)], [1, 0]]` — eigenvalue equation is exactly the §5 branch quadratic after rescaling.

| Claim                                                                | Tag                              | Source                                                                                  |
|----------------------------------------------------------------------|----------------------------------|-----------------------------------------------------------------------------------------|
| `G_N*` as `det_ζ` ratio                                              | [THEOREM]                        | SPEC_FQCR.md §2 Prop 1; FTD-0141; OT-1.7                                                |
| Stack `(L_{1/4,N}, L_{3/4,N}, Ψ_N, λ_N, T_N)` as minimal FQCR canon  | [SELECTION — current canon]       | SPEC_FQCR.md §1–§3                                                                       |
| Stack yields `G_N*, q*, R_N, x_±` algebraically                       | [DERIVED]                        | Composition of §1–§5 results                                                             |
| Stack yields physical coupling                                        | [STRONGLY MOTIVATED CONJECTURE]  | Inherits OT-5.1 / FTD-0013 tag                                                           |

---

# §7 · Lorentz / EM / QED / Dirac scaffolding

> **Scope flag.** The bivector / Dirac / QED scaffolding in this section is external mathematical machinery (Maxwell duality on `Λ²(R^{3,1})`, Clifford algebra, standard tree-level QED) that FTD's bridge program is exploring. **All physics-bridge claims here are tagged [OPEN] pending a derivation chain that does not conflict with the closed-negative result FTD-0073.** FTD-0073 (mode-erasure capstone, 2026-04-24) proved that **site-local 0-form state-field readout cannot support Clifford on any finite block under pointwise-threshold dynamics**. The bivector reading in this section is a candidate *non-site-local* scaffold; its compatibility with FTD-0073 has not been established. Pure linear-algebra facts about external Maxwell duality remain [THEOREM] but do not by themselves bridge to the FTD lattice.

**Bivector frame.** `B = (E, H) ∈ Λ²(R^{3,1}) ≅ R⁶`. The Hodge dual `*: Λ² → Λ²` defines `J(E, H) = (H, −E)` with `J² = −I` — the same algebraic primitive as §1.

**Charged holonomy.** `e^{iqA_e}` and the Dirac operator `D_A ψ(v) = Σ_μ γ^μ ∇_μ^A ψ(v)` with Clifford square `D_A² ≈ ∇_A^† ∇_A + i q Σ^{μν} F_{μν}`. Standard tree-level `g = 2` and one-loop `a^{(1)} = α/(2π)` are external QED results.

| Claim                                                                | Tag                              | Source                                                                                  |
|----------------------------------------------------------------------|----------------------------------|-----------------------------------------------------------------------------------------|
| `dim Λ²(R⁴) = 6`                                                     | [THEOREM]                        | External linear algebra (`C(4,2) = 6`)                                                   |
| Bivector duality `J(E,H) = (H, −E)`, `J² = −I`                        | [THEOREM]                        | External Maxwell duality                                                                 |
| Lorentzian signature emerges from bivector duality on the FTD lattice | **[OPEN]**                       | LEDGER FTD-0073: site-local Clifford **closed-negative** on finite blocks under pointwise-threshold dynamics. The bivector reading is a candidate non-site-local scaffold; bridging to FTD substrate requires a non-site-local construction not yet built. |
| Photon as finite edge connection / `F = dA` lattice gauge structure  | **[OPEN — IMPORTED scaffold]**   | Standard lattice-gauge formalism. FTD-0074 (flux 1-form readout) closed-negative for Clifford. |
| Dirac Clifford square gives Pauli term `i q Σ^{μν} F_{μν}`            | **[OPEN — IMPORTED scaffold]**   | External Clifford structure. Cite FTD-0073 mode-erasure for the FTD-side obstruction.    |
| Tree-level `g = 2`                                                   | **[OPEN — IMPORTED scaffold]**   | External QED, awaiting FTD-side derivation                                               |
| One-loop `a^{(1)} = α_FQCR/(2π)`                                     | **[OPEN — IMPORTED bridge]**     | Requires `α_FQCR ↔ α` physical identification, which is itself FTD-0013 [STRONGLY MOTIVATED CONJECTURE]. Cannot be promoted above its dependencies. |
| Full QED `g − 2` precision                                           | [OPEN]                           | MC-T4.4 in SPEC_OPEN_MATH_BY_SECTOR.md                                                    |

---

# §8 · Electroweak / SM scaffold

**Internal frame.** `H_int = C³ ⊗ C² ⊗ C_Y`. **Gauge group.** `G_SM = (SU(3)_c × SU(2)_w × U(1)_Y) / Z_6` with center closure `Y_6 + 2r + 3w ≡ 0 mod 6`. **Charge.** `Q = T_3 + Y` (standard SM construction).

**Stiffness convention.** `X_2 = 4π/g_2²`, `X_Y = 4π/g_Y²`, `X_EM = X_2 + X_Y`.

**Lock ratio at GUT scale (SU(5) prediction).** `X_2 : X_Y = 3 : 5` ⇒ `sin²θ_W^lock = 3/8 = 0.375`. This is the **standard SU(5) GUT-scale value at the unification scale**, *not* the M_Z physical value. Comparison to experiment requires RG running. **Honest framing (W2.3 audit, 2026-05-08):** the 3:5 ratio is purely **imported** from standard SU(5) trace-normalisation (`5̄ = (3̄,1)_{1/3} ⊕ (1,2)_{−1/2}` forces the hypercharge trace) — no FTD substrate ingredient enters its derivation. The 3:5 → 3/8 step is therefore IMPORTED PHYSICS dressed in FTD vocabulary; cite FTD-0149 [IMPORTED] for the SU(5) trace-normalisation anchor.

**RG running to M_Z.** Standard SM running takes `sin²θ_W^lock = 0.375` at `M_GUT` to `sin²θ_W ≈ 0.231` at `M_Z`. The substrate-derivation of the running coefficients (`b_Y = 41/6`, `b_2 = −19/6`) from FTD finite spectra is **[OPEN / HARDENING]** (cross-link MC-T3.6, new ID 2026-05-08).

**Canonical IR fit.** FTD-0018: `sin²θ_W = 3/13 = 0.2308` is **[PARAMETRIC]** at M_Z (downgraded 2026-04-19; 3.5% off experimental 0.2229). The 3/13 fit and the 3/8 GUT-scale value are **distinct claims at distinct scales**; they coexist in canon — neither subsumes the other.

| Claim                                                                | Tag                              | Source                                                                                  |
|----------------------------------------------------------------------|----------------------------------|-----------------------------------------------------------------------------------------|
| SM group quotient `G_SM = (SU(3) × SU(2) × U(1))/Z_6`                | [IMPORTED — structural match]    | Standard SM construction                                                                 |
| Z_6 center closure `Y_6 + 2r + 3w ≡ 0 mod 6`                         | [THEOREM within scaffold]        | Group-theoretic computation                                                              |
| `Q = T_3 + Y`                                                        | [IMPORTED]                       | Standard SM                                                                              |
| Stiffness convention `X_a = 4π/g_a²`                                 | [DEFINITION]                     |                                                                                          |
| Lock ratio `X_2 : X_Y = 3 : 5` at GUT scale                          | **[IMPORTED]** (FTD-0149) — standard SU(5) trace normalisation | No FTD substrate ingredient enters; pure GUT-scale gauge-theory adoption |
| `sin²θ_W^lock = 3/8` at GUT scale                                    | [THEOREM once 3:5 imported]      | Direct algebra: `g_Y²/(g_2² + g_Y²) = X_2/(X_Y + X_2) = 3/8`. Honest reading: `IMPORTED` content, not FTD content |
| RG running of `sin²θ_W^lock` from GUT to M_Z                         | [OPEN / HARDENING]               | MC-T3.6 (β-coefficient substrate-derivation [OPEN]; new ID introduced 2026-05-08 to resolve W2.5 MC-T3.5 collision — old MC-T3.5 = FTD-0110 multi-scale boundary correction is sector-tracker §9, distinct research arc)                                       |
| `sin²θ_W ≈ 3/13` at M_Z (canonical IR fit)                           | [PARAMETRIC]                     | LEDGER FTD-0018 (downgraded 2026-04-19); 3.5% off CODATA                                 |
| Neutral-Higgs lock preserves `U(1)_EM`                               | [THEOREM within scaffold]        | Standard SM                                                                              |
| Substrate-derivation of beta coefficients from finite spectra        | [OPEN / HARDENING]               | **MC-T3.6** (β-coefficient arc; new ID 2026-05-08 to resolve W2.5 collision; *not* the same arc as sector-tracker MC-T3.5 = FTD-0110 multi-scale boundary correction) |

---

# §9 · Weak / Top / Higgs scaffold

**Weak scale.** `v² = Λ_h² / W_BCC` with `Λ_h = m_t · √(2 W_BCC) = m_t · G* / √π` (top saturation principle). At bare lock: `v = √2 · m_t`. **Honest framing:** `v = √2 m_t` follows from `y_t ≈ 1` (textbook top Yukawa); this is borrowed empirical input, not a novel FTD prediction.

**Higgs radial mode.** `m_H² = W_BCC · χ_H · v²` with `χ_H = 2 − 3 Ξ_t + Ξ_bos` (proposed scaffold). Computing `χ_H` from FTD substrate is **[OPEN / HARDENING]**.

**Equality to physical pole masses.** Equating bare-lock predictions directly to physical pole masses (without RG running, threshold corrections, etc.) is **[CLOSED NEGATIVE]** as a claim of strict identity.

| Claim                                                                | Tag                              | Source                                                                                  |
|----------------------------------------------------------------------|----------------------------------|-----------------------------------------------------------------------------------------|
| `v² = Λ_h² / W_BCC` (scale principle)                                | [SELECTION — scale principle]    | Internal FTD scaffold                                                                    |
| `Λ_h = m_t · G* / √π` (top saturation)                               | [SELECTION — top saturation]     | Internal FTD scaffold                                                                    |
| `v = √2 · m_t` at bare lock                                          | [BORROWED EMPIRICAL]             | Textbook `y_t ≈ 1`; numerical coincidence, not novel                                     |
| Equality to physical pole masses                                     | [CLOSED NEGATIVE]                | Bare lock ≠ pole mass without RG + threshold corrections                                 |
| Higgs `m_H² = W_BCC · χ_H · v²` with `χ_H = 2 − 3 Ξ_t + Ξ_bos`       | [SELECTION — scaffold]           | Cross-ref FTD-0017 (Higgs `(N_eff/α²)·m_e`, [STRUCTURALLY MOTIVATED PARAMETRIC])        |
| Computing `χ_H` from substrate                                        | [OPEN / HARDENING]               | No canonical anchor yet                                                                  |

---

# §10 · Flavor scaffold

**Base branch.** `q* = (G* − √(G*² − 4))/2` (smaller root of `z² − G*z + 1 = 0`; same as `e^{−χ_G}` of §2).

**Depth matrices (proposed candidate scaffolding).**
- `N_E = diag(9, 3, 0)` (charged leptons)
- `N_U = diag(12, 5, 0)` (up-type quarks)
- `N_D = diag(7, 4, 0)` (down-type quarks)

These integer-depth matrices are **proposed candidate scaffolding**. They have **no canonical derivation** in current FTD documentation. They are reverse-engineered from observed mass hierarchies via `q*`-power fits and tagged here at their honest level.

**Mass formula.** `D_F = G_F [m_{F,3} · C_F · q*^{N_F}] H_F†` (proposed unitary structure). Per-fermion projection corrections `C_F` are also **[PARAMETRIC candidate scaffold]**.

| Claim                                                                | Tag                              | Source                                                                                  |
|----------------------------------------------------------------------|----------------------------------|-----------------------------------------------------------------------------------------|
| `q*`-power hierarchy as flavor structural principle                  | [SELECTION — structural reading] | Internal FTD scaffold                                                                    |
| Depth matrices `N_E = diag(9,3,0)`, `N_U = diag(12,5,0)`, `N_D = diag(7,4,0)` | **[PARAMETRIC candidate scaffold]** | No canonical derivation; reverse-engineered from mass-ratio fits                  |
| Mass scaffold `D_F = G_F[m_{F,3} C_F q*^{N_F}] H_F†`                 | [SELECTION — scaffold]           | Internal FTD scaffold; projection corrections `C_F` candidate-only                       |
| CKM estimates from `q*` powers                                       | [NUMERICAL / STRUCTURAL APPROXIMATION] | Order-of-magnitude only; cross-ref CATALOG_PARAMETRIC_INSERTIONS.md                |
| Explicit transfer matrices forcing depths                            | [OPEN / HARDENING]               | No canonical anchor — would be a new derivation chain                                    |

---

# §11 · QCD scaffold

**Color frame.** `C³`. **Color group.** `SU(3)_c`. **Triality.** `r ∈ Z_3`.

**QCD beta coefficient (one loop).** `b_3 = (11 N_c − 2 n_f)/3`. For `N_c = 3, n_f = 6`: `b_3 = 7`. **The formula is imported standard QCD**; the integer `b_3 = 7` follows from substituting `N_c = 3` (independently sourced — see `DERIV_NC_FROM_TOPOLOGY.md` (four routes) and the Moore Layer Theorem; the historical identification `N_c = \lfloor x_- \rfloor` is **RETIRED** per v1.4 §5, LEDGER FTD-0014 removed in commit `ca7eb61`) and `n_f = 6` (SM input).

**Distinct claim.** FTD-0020 asserts `α_s = 7/59` as **[PARAMETRIC]** (downgraded 2026-04-19; rational fit with `7` matching `b_3` numerator coincidence). This is *not* the same claim as importing `b_3 = 7`; the LEDGER demotion of FTD-0020 does not affect the scaffold-level adoption of the standard one-loop formula.

**Confinement and strong CP.**
- Confinement as "non-abelian finite color transport creates trace-gap / flux closure" — **[CONJECTURE / DOCTRINE]**. The 2026-05-03 night audit of FTD-0025 explicitly recorded the structural obstruction: confinement is intrinsically non-classical (lives in `Z = ∫ dU exp(−S)`), and FTD's substrate is deterministic — there is **no Phase-G analog for area-law behavior**. The compact-U(1) link-variable formulation is *imported wholesale* from textbook lattice gauge theory and not substrate-derived. Internal `[THEOREM]` tags within `DERIV_CONFINEMENT_FROM_GAP_EQUATION.md` should each read `[THEOREM-within-compact-U(1)-LGT framework, PARAMETRIC at FTD-substrate level]`.
- Strong CP `θ_QCD = 0` by finite discrete orientation closure — **[CONJECTURE / NEEDS THEOREM PACKAGING]**.

| Claim                                                                | Tag                              | Source                                                                                  |
|----------------------------------------------------------------------|----------------------------------|-----------------------------------------------------------------------------------------|
| `N_c = 3` from `C³` / `Z_3` triality / `Z_6` closure                 | [SELECTION] (algebraic) — `N_c = 3` independently sourced; the historical `x_- ↔ N_c` identification is **RETIRED** (v1.4 §5; LEDGER FTD-0014 removed in commit `ca7eb61`) | `DERIV_NC_FROM_TOPOLOGY.md`; Moore Layer Theorem                              |
| `SU(3)_c` as internal-frame symmetry                                 | [IMPORTED — structural match]    | Standard SM                                                                              |
| `b_3 = (11 N_c − 2 n_f)/3` formula                                   | [IMPORTED COEFFICIENT]           | Standard one-loop QCD                                                                    |
| `b_3 = 7` for `N_c = 3, n_f = 6`                                     | [THEOREM once formula imported, conditional on independent `N_c = 3` source + SM `n_f = 6` input] | Direct substitution                                            |
| `α_s = 7/59` (distinct claim)                                        | [PARAMETRIC]                     | LEDGER FTD-0020 (downgraded 2026-04-19); rational-fit coincidence                        |
| Confinement as trace-gap / flux closure                              | [CONJECTURE / DOCTRINE]          | LEDGER FTD-0025 (compact-U(1)-LGT framework imported, not substrate-derived)             |
| Color singlets `q-q̄`, `qqq` as observable states                    | [IMPORTED — structural match]    | Standard SM                                                                              |
| Strong CP `θ_QCD = 0` by finite orientation closure                  | [CONJECTURE / NEEDS THEOREM PACKAGING] | Internal FTD scaffold                                                                |

---

# §12 · Gravity / curvature

**Partial closure landed (FTD-0131, 2026-05-03; reconciled 2026-05-24 per `../03_derivations/AUDIT_NEWTON_POSTULATES_RECONCILIATION.md`).** `DERIV_NEWTON_FROM_SUBSTRATE.md` derives Schwarzschild's leading-order behaviour from FTD substrate via Phase G's lattice Poisson Green's function (FTD-0004 [THEOREM]) plus FTD-0110's cluster-mass identification ([DERIVED at linear level]) plus FTD-0015's α¹¹ mass formula ([STRONGLY MOTIVATED CONJECTURE]) plus **one flagged interpretive step (the clock hypothesis used in `SPEC_FTD_LAGRANGIAN.md` §4.3)** — the original two flagged postulates (gravitational coupling form, linearized tick-rate response coefficient `2/c²`) are subsumed by SPEC §4.2 + §4.3 [THEOREM]s per the 2026-05-24 reconciliation (Reading A confirmed; the linearized `2/c²` postulate's factor-of-2 was a `g_00`-vs-`dτ/dt` convention difference). Arc B P2 v1 closure attempt UNDERDETERMINED (`AUDIT_CLOCK_HYPOTHESIS_v1_UNDERDETERMINED.md`); v2 attempt INVALIDATED on process + substance axes (`AUDIT_CLOCK_HYPOTHESIS_v2_UNDERDETERMINED.md`, 2026-05-25); v3 pre-reg queued (target: substrate-derivation of quadratic L²-norm bandwidth-budget-conservation primitive).

**Substrate prediction.**
`α_G(e,e) = (m_e / m_P)² = [√(2π) · (16/3) · α¹¹]² ≈ 1.745 × 10⁻⁴⁵`
matches measured `1.752 × 10⁻⁴⁵` to **0.38%** (within FTD-0015's existing precision envelope).

**Closed-negative finding.** The framework-integer claim `G_N = 1/(b_3 + N_c)² = 1/100 in lattice units` is **[CLOSED NEGATIVE]** as identification with physical `G_N`. The substrate-derived `α_G` differs from `1/100` by `~10²⁰` (K_B = m_e calibration), `~300` (K_B = m_P calibration), or `~10⁴³` (dimensionless `α_G(e,e)`) — at minimum a 2.5-order discrepancy under any natural calibration.

**v1.4 closed-negative guardrail (FTD-0184).** The external ontic-system v0.2 branch-compliance/source-law stack proposes the exponential readout metric `dτ=e^{-U}`, `d ell_eff=e^U d ell`, `n_γ=e^{2U}`, and `dτ_m^2=e^{-2U}dt^2-e^{2U}d ell^2/c^2`. This is the Yilmaz-style route: useful as red-team provenance, but **[CLOSED NEGATIVE]** as a replacement FTD gravity sector because it diverges from GR beyond leading weak-field order and removes literal horizons. `Action-Closure Duality`, `ell_F`, `m_F`, and mass-depth notation receive **no promotion**; they are Planck-scale/substitution-identity bookkeeping under the existing calibration.

**v1.2 candidate principles.** The proposed Phase-3 program lists three candidate principles for substrate-deriving the remaining gravitational structure:
1. Finite trace curvature (curvature from `s_m` variation in §3 finite-trace mechanics)
2. Graph spectral curvature (curvature from lattice Laplacian eigenvalue structure)
3. Emergent spacetime curvature from finite adjacency deformation

These are **[CANDIDATE PRINCIPLE]** alternatives within the §14 Phase-3 program; none is currently a derivation chain.

| Claim                                                                | Tag                              | Source                                                                                  |
|----------------------------------------------------------------------|----------------------------------|-----------------------------------------------------------------------------------------|
| `G_+(r) → 1/(4π r)` at large r (Phase G)                             | [THEOREM]                        | SPEC_ALGEBRAIC_SPINE.md §6 Theorem 6; OT-1.4; FTD-0004                                  |
| Cluster mass `M = N · m_e` (linear regime)                            | [DERIVED at linear level]        | FTD-0110; OT-3.4                                                                        |
| Schwarzschild leading-order `dτ/dT = 1 + 2φ_g/c²` recovered           | [DERIVED, conditional on 1 flagged interpretive step (clock hypothesis)] | FTD-0131; DERIV_NEWTON_FROM_SUBSTRATE.md §1; reconciled 2026-05-24 (`../03_derivations/AUDIT_NEWTON_POSTULATES_RECONCILIATION.md`)            |
| `α_G(e,e) = (m_e/m_P)² ≈ 1.745 × 10⁻⁴⁵`, 0.38% match                | [STRONGLY MOTIVATED CONJECTURE] (floor inherited from FTD-0015) + [DERIVED chain] | FTD-0131; the 0.38% precision is squared FTD-0015 precision (mechanical, not new evidence); chain steps 1.1–1.5 derived from substrate; 1 flagged interpretive step (clock hypothesis) post-reconciliation 2026-05-24; v1+v2 closure attempts UNDERDETERMINED/INVALIDATED, v3 queued |
| `G_N = 1/(b_3 + N_c)² = 1/100` framework-integer reading              | **[CLOSED NEGATIVE per FTD-0131]** | Off by `~10²⁰` to `~10⁴³` under any natural calibration                              |
| Branch-compliance/Yilmaz exponential metric route (`dτ=e^{-U}`, `n_γ=e^{2U}`) | **[CLOSED NEGATIVE per FTD-0184]** | Agrees at low weak-field order but conflicts with standing GR/black-hole sector; see LEDGER FTD-0184 |
| `Action-Closure Duality`, `ell_F`, `m_F`, mass-depth notation          | [NO NEW CLAIM]                    | Planck-length/Planck-mass reparameterization and substitution identity per FTD-0184      |
| Gravitational coupling form `ρ_g = K_B^grav · 1_manifested`           | [POSTULATE 1, flagged]           | DERIV_NEWTON_FROM_SUBSTRATE.md §1.2                                                      |
| Linearized tick-rate response `tick = 1 + 2φ_g/c²`                    | [POSTULATE 2, flagged]           | Matches GR linearization; substrate-derivation [OPEN]                                    |
| Beyond-leading-order GR (Mercury perihelion, light bending, GW)       | [OPEN]                           | DERIV_NEWTON_FROM_SUBSTRATE.md §5                                                        |
| Substrate-side strong-field GR / Schwarzschild-Kerr-horizon derivation | [OPEN]                           | Genuine open item surfaced by FTD-0184; do not use the exponential-metric shortcut       |
| Equivalence-principle analogue from substrate                         | [OPEN]                           | No canonical anchor                                                                      |
| Mass-gap to curvature source                                          | [OPEN]                           | No canonical anchor                                                                      |
| Three v1.2 candidate principles (trace curvature / graph spectral / adjacency deformation) | [CANDIDATE PRINCIPLE] | Phase-3 hardening targets; not derivations                                          |

---

# §13 · Non-circularity audit (roll-up)

This audit is a roll-up of the canonical LEDGER tags into four layers. **It introduces no new categorisation.** The canonical tag system (§0.1) already distinguishes derived from imported from numerical from open. This section reads them out at the doctrine level for navigation.

### §13.1 · Cleanly derived from internal primitives ([THEOREM] / [DERIVED] from axioms or prior theorems with explicit chain)

These have proof chains entirely inside FTD's axiom set or its rigorous algebraic spine:

`J² = −I`, `J⁴ = I`; `G* = Γ(1/4)/Γ(3/4)`; `G* = Γ(1/4)²/(π√2)`; `G_N* → G*` at `O(1/N²)`; `det_ζ D_{3/4}/det_ζ D_{1/4} = G*`; `G* = 2 cosh(χ_G)`; `q* = e^{−χ_G}`; symmetric recurrence; transfer-matrix `det = 1`; Casimir invariant `I_m`; Möbius reduction `z_{m+1} = s − 1/z_m`; regime split `\|s\| < 2 / = 2 / > 2`; `x² + y² ↦ x² − y²` under `y ↦ Jy`; light-cone boundary; `cos(2θ) = x² − y²`; `P_G = (G*/2, √(G*²−4)/2)`; coefficient 16 = `\|Aut(E)\|²`; tower harmonic invariant `1/y_+ + 1/y_- = 1`; `Q(G*) ∩ Q(π) = Q` (conditional on Chudnovsky 1976); per-voxel mass gap; D = 3 from `\|Aut(E)\|² = 2^D(D−1)!`; Moore integers `{N_c=3, N_base=4, b_3=7, N_eff=13}`; `a_phys ≡ ℓ_P` no-go theorem.

**Anchor:** `SPEC_ALGEBRAIC_SPINE.md` 9 theorems + subsidiaries; `TRACKER_ONTIC_TRUTH.md` T1 + T2 + T3.4.

### §13.2 · Selected (consistency-argued, not uniqueness-proven) ([SELECTION])

`R_N(t) = 1 + λ_N + A_N`; `(4, 6; 3, 2)` exponent quadruple; `t = 1` base point; `μ² = s − 2`; Bernoulli lemniscate as one radial realisation; `v² = Λ_h²/W_BCC`; top saturation `Λ_h = m_t G*/√π`; `q*`-power flavor depths; mass scaffold `D_F = G_F[m_{F,3} C_F q*^{N_F}] H_F†`; `X_2:X_Y = 3:5` GUT lock; Higgs `χ_H = 2 − 3Ξ_t + Ξ_bos`; Bell `S = 2√2`; loop coefficients `c_1 = 9/47`, `c_2 = 5/64`, `c_3 = 4/141`; Einstein equations from Deser bootstrap; cyclotomic Hamiltonian parameters; Moore Layer Theorem; BCC multiplicative structure (`W₃ + SU(3)` from same eigenvalue).

**Anchor:** `LEDGER.md` per-claim; `CATALOG_PARAMETRIC_INSERTIONS.md` row-by-row.

### §13.3 · Imported from known physics ([IMPORTED] / [PARAMETRIC])

`G_SM = (SU(3) × SU(2) × U(1))/Z_6`; `Q = T_3 + Y`; `b_Y = 41/6`, `b_2 = −19/6`, `b_3 = (11 N_c − 2 n_f)/3`; Dirac/Clifford formalism (with FTD-0073 closure on site-local Clifford recorded); standard QED `β` and `g − 2` structures; SU(3) confinement framework (compact-U(1)-LGT); `θ_QCD = 0` adopted from standard SM.

**Anchor:** `LEDGER.md` rows tagged [IMPORTED] / [PARAMETRIC]; FTD-0018, FTD-0019, FTD-0020, FTD-0021, FTD-0022 demotions (2026-04-19).

### §13.4 · Numerically reconstructed ([STRONGLY MOTIVATED CONJECTURE] / [NUMERICAL FACT])

`α_FTD⁻¹ ≈ 137.035999177` (1.26 ppm; `x_+ ↔ 1/α`, FTD-0013); `α_G(e,e) = (m_e/m_P)² ≈ 1.745 × 10⁻⁴⁵` (0.38%, postulate-conditional); CKM estimates from `q*` powers; structural uniqueness scans (2.87M-poly with 0 Eisenstein dual-matchers under the historical `(1/α, N_c)` target pair; 63-discriminant Γ-product null at `h ≥ 2`; ~4×10⁵:1 Bayes weight; FTD-0189 adversarial 2.65M-polynomial scan over an 18-constant FTD-undesigned basket: 0 non-G\* dual-matchers, rank 1 by ~130×). *(Note: `x_- = 3.024 ≈ N_c = 3` (0.80%) framing is **retired** per v1.4 §5; LEDGER FTD-0014 removed in commit `ca7eb61`. The `x_-` value is a mathematical artifact of $P(x)$; `N_c = 3` is independently sourced — see `DERIV_NC_FROM_TOPOLOGY.md`.)*

**Anchor:** `TRACKER_ONTIC_TRUTH.md` T3 + T5; `SPEC_PHYSICS_BRIDGE.md` (FTD-0121 SYNTHESIS).

### §13.5 · Main hardening targets ([OPEN] research arcs)

1. **Non-action alpha-readout mechanism** — MC-T4.3, central foundational obstruction. The target is not another match to `1/alpha`; it is a rule that maps the algebraic/FQCR branch to an operational electromagnetic coupling measured by matter, without inserting alpha. `SPEC_ALPHA_READOUT_CONTRACT.md` defines the required tuple `(P, A_obs, O_EM, R, C)`, exclusion rules, and ARC-0 to ARC-3 status levels. Candidate classes: boundary-condition readout, observable-selection readout, quantization/readout rule, discrete-native measurement path. Lead-physicist diagnosis: closure may require ontology extension beyond the 5 axioms.
2. **Derive `R_N(t)` from a variational / operator principle** — MC-T3.1 (FTD-0110 nonlinear bridge, NOT CLOSED, ~2.5× slope mismatch); FTD-0143 uniqueness scan PRE-REGISTERED. This is useful only if it feeds an operational readout or removes a selection from FQCR Model IV.
3. **Derive activation kernels and beta coefficients from finite spectra** — MC-T3.6 (new ID 2026-05-08; not the same arc as MC-T3.5 = FTD-0110 multi-scale boundary).
4. **Derive Higgs residual curvature `χ_H`** — no canonical anchor.
5. **Force flavor depth matrices from explicit transfer matrices** — no canonical anchor (depth matrices §10 [PARAMETRIC scaffold]).
6. **Substrate-derive QCD trace-gap confinement** — FTD-0025 night-2026-05-03 audit recorded structural obstruction (no Phase-G analog for area-law behavior); the strong-sector substrate-derivation gap is real and structural.
7. **Build gravity as finite trace curvature** — partial closure FTD-0131 (`α_G` to 0.38%, **1 interpretive step flagged: clock hypothesis** post-2026-05-24 reconciliation; original 2 postulates subsumed by SPEC §4.2 + §4.3 [THEOREM]s); FTD-0184 closes the branch-compliance/Yilmaz shortcut negative; FTD-0193 substrate spin-2 search [CLOSED NEGATIVE] at L≤64; **Arc C2 spin-2 boundary theorem free-theory derivation landed 2026-05-24** (`../10_eft_program/DERIV_SPIN2_BOUNDARY_THEOREM_FREE_THEORY.md` + `../10_eft_program/DERIV_J_BILINEAR_NO_SPIN2_POLE.md`); Arc C2 P3 pre-reg (`preregister-spin2-boundary-theorem-v1`, FTD-0209) hash-locked. Substrate-side strong-field GR / horizon derivation remains [OPEN] in the dual sense per the boundary theorem framework (full nonlinear GR via Deser-bootstrap of POSITED h_μν per FTD-0189).

---

# §14 · Compressed roadmap

### Phase 1 — Doctrine ledger and audit (this document)

**Status: COMPLETE.** This v1.4 document compresses FTD's claim infrastructure into a single navigation surface. Every claim is rolled up at canonical tag from `LEDGER.md`, `TRACKER_ONTIC_TRUTH.md`, `SPEC_ALGEBRAIC_SPINE.md`, `SPEC_FQCR.md`, `SPEC_OPEN_MATH_BY_SECTOR.md`, or `CATALOG_PARAMETRIC_INSERTIONS.md`. The non-circularity audit (§13) reads out the four-layer structure (derived / selected / imported / numerical) without inventing new categories. The 2026-05-20 ontic-system reconciliation adds the FTD-0184 gravity guardrail without changing any claim tag.

### Phase 2 — Hardening

| Priority | Hardening target                                  | Canonical anchor                                                        | Status                          |
|----------|---------------------------------------------------|-------------------------------------------------------------------------|---------------------------------|
| 0        | Operational alpha-readout mechanism               | MC-T4.3 (`SPEC_OPEN_MATH_BY_SECTOR.md` §10.1)                            | CENTRAL FOUNDATIONAL OBSTRUCTION |
| 1        | `R_N(t)` variational/operator derivation          | MC-T3.1 (`SPEC_OPEN_MATH_BY_SECTOR.md`); FTD-0143 PRE-REG                | NOT CLOSED, slope mismatch ~2.5× |
| 2        | Activation kernels and beta coefficients          | MC-T3.6 (new ID 2026-05-08; β-coefficient arc separate from MC-T3.5 FTD-0110 multi-scale) | OPEN |
| 3        | Higgs residual curvature `χ_H`                    | NEW — no canonical anchor                                                | OPEN                             |
| 4        | Flavor transfer matrices forcing depths           | NEW — no canonical anchor                                                | OPEN                             |
| 5        | QCD trace-gap confinement substrate derivation    | FTD-0025 night-audit (2026-05-03); FTD-0131 cross-ref                    | OPEN, structural obstruction recognized |
| 6        | Gravity beyond Newtonian limit (substrate-side GR)| FTD-0131 partial; FTD-0184 closes branch-compliance/Yilmaz route negative | OPEN, partial closure landed; shortcut closed negative |

### Phase 3 — Gravity (subsumed by Phase 2 priority 6 + new candidate principles)

Develop substrate-derivation routes for full gravitational structure. The three candidate principles (finite trace curvature; graph spectral curvature; emergent spacetime curvature from finite adjacency deformation) are tagged [CANDIDATE PRINCIPLE]; none currently has a derivation chain. Core target:

`s_m variation ⇒ curvature ⇒ effective attraction / geodesic deviation`

This connects §3 finite-trace mechanics (where `s_m` lives) to §12 gravity. **Status: OPEN.**

### Phase 4 — Master compression

Aspirational target: a compact object `T_FTD` (an extended operator stack in the spirit of §6) such that

`T_FTD ⇒ G*, α, q*, SU(3) × SU(2) × U(1), running, masses, flavor, confinement, curvature.`

This is the strongest possible FTD self-claim. **Achievability is conditional on Phase 2 priority 0 (MC-T4.3 operational alpha-readout) and priority 5 (confinement substrate derivation, structurally obstructed).** Honest assessment: closure may not be achievable without ontology extension beyond the 5 axioms. The framework's external defensibility is fully achievable without `T_FTD` closure; the strongest possible self-claim ("derived 1/α + entire SM") requires it.

---

# §15 · Cross-references

| Doctrine §  | Primary anchor                                                                                      | LEDGER row(s)                                                  | Tracker row(s)               |
|-------------|-----------------------------------------------------------------------------------------------------|----------------------------------------------------------------|------------------------------|
| §1          | SPEC_FQCR.md §1; ../02_foundations/FOUND_AXIOM_ZERO.md                            | FTD-0141                                                       | OT-1.7                       |
| §2          | SPEC_ALGEBRAIC_SPINE.md §1, §5; SPEC_FQCR.md §2 Props 1, 2                                          | FTD-0001, FTD-0141, FTD-0142, FTD-0117                         | OT-1.2, OT-1.7, OT-1.8, OT-2.1|
| §3          | SPEC_FQCR.md §1, §2 Props 3, 4 (Model III)                                                          | FTD-0141, FTD-0142                                             | (algebraic; standard rep theory)|
| §4          | ../08_structural/DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md; SPEC_ALGEBRAIC_SPINE.md §1                  | FTD-0001                                                       | OT-1.2                       |
| §5          | SPEC_ALGEBRAIC_SPINE.md §2 (Theorem 2); SPEC_FQCR.md §2 Prop 5, §3                                  | FTD-0001 (master quadratic), FTD-0013 (`x_+ ↔ 1/α`), FTD-0143 PRE-REG; FTD-0014 (`x_- ↔ N_c`) **RETIRED** v1.4 §5, row removed in commit `ca7eb61` | OT-1.1, OT-3.3, OT-5.1 |
| §6          | SPEC_FQCR.md §1, §2, §3                                                                             | FTD-0141, FTD-0142                                             | OT-1.7, OT-1.8               |
| §7          | ../09_mathematical/DERIV_MODE_ERASURE_AND_SPIN_ALGEBRA.md                                           | **FTD-0073 (CLOSED NEGATIVE for site-local Clifford)**, FTD-0074 | (no T-tier; closed-negative)|
| §8          | LEDGER.md (FTD-0018 sin²θ_W demotion); standard SU(5) GUT                                          | FTD-0018 (PARAMETRIC at M_Z)                                   | (T5/parametric)              |
| §9          | LEDGER.md (FTD-0017 Higgs)                                                                          | FTD-0017                                                       | (parametric)                 |
| §10         | CATALOG_PARAMETRIC_INSERTIONS.md (flavor section)                                                   | (catalog rows)                                                 | (parametric scaffold)        |
| §11         | LEDGER.md (FTD-0020 α_s; FTD-0025 confinement night-audit); standard QCD; `DERIV_NC_FROM_TOPOLOGY.md` (independent `N_c = 3` routes) | FTD-0020, FTD-0025 (annotated 2026-05-03); FTD-0014 (`x_- ↔ N_c`) **RETIRED** v1.4 §5 | OT-5.2 (`x_- ↔ N_c`) **retired** |
| §12         | ../03_derivations/DERIV_NEWTON_FROM_SUBSTRATE.md; SPEC_ALGEBRAIC_SPINE.md §6 (Phase G) | **FTD-0131** ([SMC] for prediction inherited from FTD-0015 + [DERIVED chain]), **FTD-0184** (branch-compliance/Yilmaz route [CLOSED NEGATIVE]; strong-field substrate gravity [OPEN]), FTD-0004, FTD-0110, FTD-0015 | OT-1.4, OT-3.4 |
| §13         | LEDGER.md (full); CATALOG_PARAMETRIC_INSERTIONS.md; TRACKER_ONTIC_TRUTH.md (T1–T5)                  | (roll-up)                                                      | (roll-up)                    |
| §14         | SPEC_OPEN_MATH_BY_SECTOR.md (MC-T4.3, MC-T3.1, MC-T3.6, T4.4)                                        | (roadmap)                                                      | (roadmap)                    |

---

# §16 · Refresh policy

This v1.4 document is refreshed when:

- A LEDGER row's tag changes for any claim cited in §1–§13.
- `SPEC_ALGEBRAIC_SPINE.md` adds, removes, or retags a theorem.
- `SPEC_FQCR.md` updates Models I–V (e.g., upon FTD-0143 scan execution → Model IV [SELECTION] upgrade or rejection).
- A new partial closure lands in §7 (bivector / Dirac bridge), §9 (`χ_H`), §10 (depth matrices), §11 (confinement), or §12 (gravity beyond leading order).
- `TRACKER_ONTIC_TRUTH.md` adds an OT-N.M entry that this doctrine should cross-reference.
- Phase 2 priority 0–6 items (§14) close.

When refreshing, increment the `Version` line in the header (v1.3 → v1.4 etc.) and add a one-line entry to `LEDGER.md` row FTD-0145 noting what changed. **No tag promotion in this document is permitted without a corresponding canonical-source change.**

---

# §17 · Single-line summary

**FTD's algebraic spine is seven theorem-grade results plus two honestly-tiered subsidiary results (nine numbered; see `SPEC_ALGEBRAIC_SPINE.md` §0) centered on `G* = Γ(1/4)/Γ(3/4) ≈ 2.9587`; the FQCR capstone (Models I–V) lands operator-theoretic provenance and the finite-N reframe-compatible restatement; the master quadratic's roots match `1/α` to 1.26 ppm and `N_c` to 0.80% as [STRONGLY MOTIVATED CONJECTURE]; the strongest physics-facing claim is therefore a rigid candidate readout, not a finished derivation; gravity has a partial closure (`α_G(e,e) = (m_e/m_P)²` to 0.38%, FTD-0131, postulate-conditional), while the branch-compliance/Yilmaz shortcut is closed negative (FTD-0184) and substrate-side strong-field GR remains open; the §7 bivector/Dirac/QED bridge sector is [OPEN] pending a non-site-local construction compatible with FTD-0073's mode-erasure closure; §8's GUT-lock `sin²θ_W = 3/8` and IR `sin²θ_W = 3/13` [PARAMETRIC] coexist in canon at distinct scales; the central foundational obstruction (MC-T4.3, operational alpha-readout mechanism) may require ontology extension beyond the 5 axioms.**
