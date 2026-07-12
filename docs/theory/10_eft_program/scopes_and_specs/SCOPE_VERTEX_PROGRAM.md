# SCOPE — The Vertex Program: an Imported Matter Sector with an Imposed Coupling

**Tag:** [SCOPE / PROGRAM CHARTER] — sets one type (an imposed calibration), marks one boundary; introduces no theorem, promotes no claim
**Date:** 2026-07-10
**LEDGER anchors:** FTD-0379 / FTD-0380 (the campaign that settled the program's branch), FTD-0013 [SMC] (the motivation), MC-T4.3 [FOUNDATIONAL OBSTRUCTION] (the boundary this program does *not* attempt to cross)
**Companions:** [`SPEC_WILSON_DIRAC_FTD.md`](SPEC_WILSON_DIRAC_FTD.md) (the matter-sector specification this program completes), [`ANALYSIS_VERTEX_DK_CLOSURE_v1.md`](../../09_mathematical/algebra/ANALYSIS_VERTEX_DK_CLOSURE_v1.md) (the settling measurements), [`SPEC_ALPHA_READOUT_CONTRACT.md`](../../01_reference/SPEC_ALPHA_READOUT_CONTRACT.md) (the contract any *native* coupling claim must still pass), [`SPEC_IMPORT_LEDGER.md`](../../01_reference/SPEC_IMPORT_LEDGER.md) (where the price is already booked)

---

## 0 · Number-One-Goal position

Under the operational test (set a type / build content forward / mark and price a boundary), this charter does the first and third:

- **It sets a type:** the matter-sector vertex coupling, imposed as a declared, motivated calibration (§2; on the liveness of its falsifiers, see the §2 note).
- **It marks a boundary:** fermion content is treated as an argument-half import. The Branch-A hypothesis — fermionic structure generated dynamically by the substrate — is closed negative at **every protocol tested** (the pre-registered FTD-0379/0380 campaign, extending the closed-negative family FTD-0061/0071–0075/0126). Scope honesty: the negatives are protocol-scoped measurements plus FTD-0073 (whose non-site-local clause is [CONJECTURE]); accessible-but-unrun variants are named in `ANALYSIS_VERTEX_DK_CLOSURE_v1.md` §0/§1.3/§1.4. The boundary is *supported by measurement at the tested scopes* — a working boundary of the FTD-0336 kind is adopted here as the program's operating assumption, not asserted as established.

The "build content forward" face belongs to the program's stages (§4), each of which derives *given the imposed inputs* — the derive-given-imposed pattern, first-class per the project's standing methodology.

## 1 · What the program is

Complete FTD's EFT matter sector honestly. A Wilsonian EFT does not derive its couplings — it takes them as inputs measured at a scale and earns its keep on structure: the interaction term, its symmetries, its RG behavior, its phenomenology. FTD's EFT program stalled on exactly the pillar the substrate **has not supplied** (MC-T4.3 [FOUNDATIONAL OBSTRUCTION] — all *natural* action-level/operator injection routes closed negative through FTD-0244, with two named exits still open: a new W-class framework commitment, or a fresh ARC-D engine-native measurement; an obstruction, not a no-go theorem). This program routes around the obstruction rather than through it:

- **Matter:** imported Wilson–Dirac fermions per `SPEC_WILSON_DIRAC_FTD.md` (Branch-B; the import is priced in the ledger's IMP-E3 category).
- **Coupling:** imposed calibration g²_vertex ≡ 1/x₊ (§2; the identification is priced at IMP-E1).
- **Substrate:** supplies the gauge connection (transverse flux projection), the constraint set (§3), and the falsifiers.

MC-T4.3 stays open as a marked [FOUNDATIONAL OBSTRUCTION]. Its closure would *retire* §2's calibration to the self-set column — the program is structured so that outcome would be a free upgrade, not a rewrite.

## 2 · The imposed input (the type this charter sets)

> **Vertex-coupling calibration [IMPOSED — calibration, conditional].** The electromagnetic coupling of any FTD Branch-B matter sector is *set* to g²_vertex ≡ 1/x₊, where x₊ is the master quadratic's dominant root ([THEOREM] as algebra). This is an act of calibration, not a derivation: it is *motivated* by FTD-0013 (x₊ ↔ 1/α, [STRONGLY MOTIVATED CONJECTURE]) and *not resolved* by it — the tag labels the gap, it does not fill it.

**Pricing (no new ledger line).** This calibration is the **composition of two imports already priced** in `import_ledger.json`: IMP-E1 (the value identification x₊ = 1/α, [SMC]) and IMP-E3 (the imported Dirac/QED functional forms). Minting a separate line would double-count; this charter is the declaration the existing lines were waiting to serve. The δ-branch content of reaching x₊ at all is likewise already priced (IMP-B1, FC-W's one adopted bit); under the ramification checkpoint (`SPEC_ALPHA_READOUT_CONTRACT.md` §2.5) this calibration is **grade-½-by-inheritance, branch-by-inheritance — both inherited from FC-W, with no new ramification act**.

**Falsifiers (each named at declaration, per the ledger's discipline):**
1. An ARC-3 closure of the alpha-readout contract (a native readout returning 1/x₊ without target input) **retires** this calibration to the self-set column — the best possible outcome. (Strictly an upgrade condition, not a falsifier.)
2. A measured *emergent* vertex coupling at matched protocol that disagrees with 1/x₊ **falsifies the calibration choice** and strains FTD-0013 through the same tolerance.
3. The constitution's FC-W kill conditions propagate: α measured outside the tree-level tolerance kills the motivation (IMP-E1's falsifier).

**Liveness note (post-declaration redteam, recorded honestly):** falsifier 2's firing precondition is itself an ARC-D-class native coupling readout — i.e., a partial resolution of MC-T4.3 — and the corpus already contains three emergent-coupling-flavored numbers ≠ 1/x₊ (the Phase-G plateau, Rutherford α ≈ 0.042, FTD-0125's G_C²-free V(r)) that were each, defensibly, diagnosed as geometry/artifact/decoupling rather than falsification. **Any future invocation of falsifier 2 must therefore come with a pre-registered criterion, fixed before the run, for what counts as "the vertex coupling" as opposed to an artifact-dominated readout** — otherwise the diagnosis escape hatch makes the falsifier unfireable. Falsifier 3 is retrospective (α is measured to 0.08 ppb; the tolerance judgment was made at FC-W's declaration). Net: as with any [IMPOSED] input, the declaration is the deliverable; the falsifier list's *live* content today is falsifier 2 under the stated pre-registration condition, and readers should not mistake the list for stronger exposure than that.

**Companion selection — flagged 2026-07-10, PRICED 2026-07-12 as IMP-S4:** the vertex's remaining physical content — the gauge-connection identification **A_μ = 𝒫_T J_μ** (the substrate's transverse flux *is* the U(1) connection the imported fermion minimally couples to), [SELECTION] per `SPEC_WILSON_DIRAC_FTD.md` §6 — was flagged here as having no priced row in `import_ledger.json`; it is now the ledger's **4th selected type (IMP-S4)** with the drafted falsifier of record ("an alternative flux-to-connection map producing inequivalent vertex phenomenology at matched protocol"). The conditional-carry note is retired by the pricing — V2 results cite IMP-S4 directly. History preserved: flagged 2026-07-10 (FTD-0380 redteam) → priced 2026-07-12 (FTD-0371 rev).

**Label correction executed with this charter:** `SPEC_WILSON_DIRAC_FTD.md` §2.2 formerly annotated g_FTD = √(1/x₊) as "[DERIVED from master quadratic, FTD-0125]". That overstated: the *value* is theorem-grade algebra, the *identification* is FTD-0013 [SMC], the *wiring into the vertex* is this [IMPOSED] calibration — and FTD-0125 is a closed-negative diagnostic, not a derivation source. The label now points here.

## 3 · What the substrate natively contributes (the constraint set)

Any Branch-B matter selection must respect all of:

| Constraint | Source | Status |
|---|---|---|
| Kinematic Cl(3,0) 4-grade skeleton (S, Vᵢ, Pᵢⱼ, T) at 2-injection order, 12/12 | FTD-0088 | [MEASURED] — **conditional on the queued Program-F effective-toggle audit** (`ANALYSIS_VERTEX_DK_CLOSURE_v1.md` §1.4); binding once the audit confirms the effective configuration |
| Matching-bivector signature [Êᵢ, Êⱼ] → Pᵢⱼ with Cl(3,0) structure-constant signs | FTD-0086, re-confirmed by FTD-0380's baseline | [MEASURED] |
| NO dynamical Dirac–Kähler evolution in FTD-0089's literal form (grades better described by KG than DK; KG residuals 0.39–0.76) | FTD-0379 | [CLOSED NEGATIVE — tested scope] |
| NO closed native su(2) on plaquette bivectors at protocols tested | FTD-0380 | [CLOSED NEGATIVE — tested scope] |
| NO Clifford from site-local 0-form readout (mode-erasure theorem) | FTD-0073 | [THEOREM] |
| Spontaneous matter is colored single-voxel states, not electrons | FTD-0076 | [MEASURED] |
| Fixed-field Wilson-Dirac g−2 measures the Wilson-r artifact unless controlled | FTD-0126 | [CLOSED NEGATIVE — protocol] |

The first two are positive structural fingerprints an *honest* selection should echo (e.g. the SU(2) doublet structure riding on the bivector plane); the rest are walls.

## 4 · Staged roadmap (each stage pre-registered before running)

- **V1 — Free Wilson–Dirac bring-up + tree-level g = 2.** Implement `SPEC_WILSON_DIRAC_FTD.md` §2.1/§2.4 (free sector, EOM evolution), measure cyclotron vs spin-precession frequencies in a fixed Landau-gauge background. **The FTD-0126 lesson is the gate, and FTD-0126 had *two* failure modes — both must be gated:** (1) the Wilson-r artifact (O(1) at m·a ~ 1): the V1 prereg must define "controlled" numerically *before* locking — an r-scan with r→0 extrapolation recovering ω_s/ω_c = 1 within tolerance, in an m·a ≪ 1 regime, with the residual artifact contribution bounded below a stated fraction of the tolerance; (2) spectral-proxy contamination (FTD-0126's extracted ω_s was an FFT peak of a multi-mode wave packet, not an eigenstate): the V1 prereg must mandate eigenstate-based or matched-filter frequency extraction with a stated mode-purity criterion. Success criterion is tree-level only: ω_s/ω_c = 1 to pre-registered tolerance (g = 2 is Dirac structure, not a loop effect). A run failing either gate is INVALID, not negative. Effort M.
- **V2 — The vertex proper.** Couple the imported fermion to the substrate's transverse flux projection (A_μ = 𝒫_T J_μ) at the §2 imposed coupling. Measure fermion–fermion scattering / bound-state normalization against the Coulomb expectation *given* g². This is a consistency measurement of the imported-vertex EFT, never a derivation of the coupling. Effort M–RP.
- **V3 — The surviving native track.** Quantify the bivector algebra's closure deviation (FTD-0087 Path 2) as a function of protocol — the deviation is now known to be structural (FTD-0380), so it is a *property* of the substrate worth one honest characterization, and the tightest structural constraint available to any future selection argument. Effort W–M.
- **V4 (conditional).** If V1+V2 stand, the one-loop question (§7-loop) remains gated by MC-T4.3 exactly as before — a_e^(1) = α/(2π) with imposed α is [PARAMETRIC] by definition and will be labeled as such if computed.

## 5 · What this program will never claim

Inherited hard exclusions (`SPEC_ALPHA_READOUT_CONTRACT.md` §3) plus the campaign's own: no "FTD derives α"; no "FTD derives fermions" (the Branch-A branch is closed negative at the protocols tested — FTD-0379/0380); no promotion of any [PARAMETRIC] result to [DERIVED] on the strength of vertex phenomenology; no citation of the M1 fitted m* ≈ 0.21 as a discovered mass (see `ANALYSIS_VERTEX_DK_CLOSURE_v1.md` §1.3). External presentation rule: *FTD's matter sector is an imported Wilson–Dirac EFT calibrated to the master-quadratic root, with the identification of that root as 1/α remaining a strongly motivated conjecture; the substrate contributes measured structural constraints, not the fermions.*
