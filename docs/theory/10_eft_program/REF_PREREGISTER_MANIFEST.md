# Pre-Registration Manifest

**Purpose:** single authoritative table mapping every pre-registered
FTD measurement to (a) the git tag committed BEFORE the run, (b) the
commit SHA the tag points at, (c) the script and any flags used, (d)
the output directory the campaign emits to, and (e) the analysis
document that interprets the result.

**Why it lives here:** the `engine/results/` gitignore default makes
new campaign outputs **local-only** by default — analysis docs cite
result paths that won't exist in a fresh clone. This manifest gives
posterity a recipe for reproducing each campaign from a tagged
commit.

**Discipline:** SHA256 of every pre-registered measurement script is
recorded in the corresponding analysis document (e.g.
`AUDIT_LOOK_ELSEWHERE_RESULTS.md`). The git tag locks the SHA at
pre-registration time. To verify a tag's commit hasn't drifted, run:

```sh
git rev-list -n1 <tag-name>     # commit SHA
git tag -l <tag-name>            # tag listing
```

---

## Pre-registered campaigns (2026-04-27 / 2026-04-28 cycle)

| FTD ID | Pre-reg tag | Commit | Script | Flags | Output dir | Analysis doc |
|---|---|---|---|---|---|---|
| **FTD-0097** look-elsewhere scan | `preregister-look-elsewhere-scan-v1` | `f11dcaa` | `tools/scan_look_elsewhere.py` | `--epsilon 1e-3,1e-4` | `engine/results/look_elsewhere_2026-04-27/` | [`AUDIT_LOOK_ELSEWHERE_RESULTS.md`](../07_assessment/AUDIT_LOOK_ELSEWHERE_RESULTS.md) |
| **FTD-0105** lemniscatic 2-sphere test | `preregister-lemniscatic-v1` | `7bc2185` | `engine/build_wsl/benchmark_black_hole_thermo` | `--lemniscatic-mode` | `engine/results/lemniscatic_*` | LEDGER row FTD-0105 |
| **FTD-0106** G\*/π asymmetry scan | `preregister-gstar-asymmetry-v1` | `edd1349` | (theory-only catalog committed; engine measurements deferred) | n/a | n/a yet | LEDGER row FTD-0106 |
| **FTD-0107** emergent-spectrum L=64 G1 | `preregister-emergent-spectrum-g1` | `37ea371` | `engine/build/campaign_emergent_spectrum_2026-04-27` | `--L 64 --output-dir=engine/results/emergent_spectrum_2026-04-27_L64 --N-samples 5 --N-seeds 5` | `engine/results/emergent_spectrum_2026-04-27_L64/` | [`ANALYSIS_EMERGENT_SPECTRUM_G1.md`](archive/campaign_complete/ANALYSIS_EMERGENT_SPECTRUM_G1.md) |
| **FTD-0107** emergent-spectrum L=128 G2 | `preregister-emergent-spectrum-g2` | (this commit) | `engine/build_wsl/campaign_emergent_spectrum_2026-04-27` | `--L=128 --seeds=5 --samples=50 --burn=200 --stride=50 --output-dir=engine/results/emergent_spectrum_2026-04-28_L128/` | `engine/results/emergent_spectrum_2026-04-28_L128/` | [`PROTOCOL_EMERGENT_SPECTRUM_G2.md`](archive/campaign_complete/PROTOCOL_EMERGENT_SPECTRUM_G2.md) (analysis pending) |

The launcher script `engine/tools/run_emergent_spectrum_g1.sh` wraps
the FTD-0107 invocation; see `commit a0983ca` for the script body.

## FQCR (Finite Quarter-Conjugacy Recurrence) Model IV uniqueness scan (2026-05-06; scan queued)

| FTD ID | Pre-reg tag | Commit | Script | Flags | Output dir | Analysis doc |
|---|---|---|---|---|---|---|
| **FTD-0143** FQCR (4,6;3,2) uniqueness scan | `preregister-fqcr-quotient-uniqueness-v1` | `557593e` | (scan-runner not yet authored; sketched in pre-reg §5 — extends `tools/scan_look_elsewhere.py` with FQCR-readout inner loop) | (k, d, ℓ, m) ∈ {2,...,8}^4; tolerances {1e-3, 1e-4, 1e-5, 1e-6}; targets = 20 FTD-0097 spine targets | `engine/results/fqcr_quotient_uniqueness_2026-05-06_l_scan/` | `PREREG_FQCR_QUOTIENT_UNIQUENESS_v1.md` (pre-reg) → `ANALYSIS_FQCR_QUOTIENT_UNIQUENESS.md` (post-launch) |

Pre-reg SHA256: `94bc4cd74cbf90017996bf90a19f0bbeaae7937f8c47a6317b3409f58c268a1f`.

Backend: pure Python via mpmath (no engine GPU required). Scan execution ~1-2 hours wall on a single CPU core.

When launching: confirm `git rev-list -n1 preregister-fqcr-quotient-uniqueness-v1` resolves to `557593e` and that the scan-runner's content hash is recorded against this anchor at runtime per FTD-0097's precedent.

## Alpha arithmetic generativity Test 4 (2026-05-20; candidate inventory queued)

| FTD ID | Pre-reg tag | Commit | Script | Flags | Output dir | Analysis doc |
|---|---|---|---|---|---|---|
| **FTD-0185** alpha arithmetic generativity | `preregister-alpha-arithmetic-generativity-v1` | (pending commit/tag) | none; desk-audit target declaration gate | No numerical search. Candidate must publish target declaration before comparison; `x_- ≈ N_c` excluded as the prize | n/a until a candidate declaration exists | `PREREG_ALPHA_ARITHMETIC_GENERATIVITY_v1.md` → candidate declaration or no-candidate report |

Pre-reg SHA256: `b222c2a0873fa21dcf28b87111ecab5de8753ec3a4a38e3074d038b6f3d06a27`. This pre-registration locks the rules for Test 4, not a measurement script.

## Derive-QM / epistemic arc — desk pre-regs (2026-05-29; closure attempts complete)

Desk pre-registrations (in-session SHA256 lock recorded **before** each analysis; no engine GPU; commit deferred per owner, integrated this commit). Per the FTD-0224 alpha-readout precedent, the lock is the pre-reg file's SHA256 recorded in-session, not a `preregister-*` git tag anchored before a separate engine run.

| FTD ID | Pre-reg doc (`10_eft_program/`) | In-session SHA256 | Verifier (passes) | Verdict |
|---|---|---|---|---|
| **FTD-0225** Route B modular-time algebra type (B1) | `PREREG_MODULAR_TIME_ALGEBRA_TYPE_v1.md` | `f8a3e960c400863677e631abba898e13d73ef64023e9da9ea51fe088b63606e5` | `scripts/proofs/proof_modular_time_algebra_type.py` (4/4) | CLOSED-NEGATIVE (type I) |
| **FTD-0226** manifestation non-commutativity (B-QM-1) | `PREREG_MANIFESTATION_NONCOMMUTATIVITY_v1.md` | `fefcd6ad26320ed4f2b3e8a46144080894c3eceb07bf90378295cd3a3386d91b` | `scripts/proofs/proof_manifestation_noncommutativity.py` (5/5) | CLOSED-NEGATIVE (Boolean) |
| **FTD-0227** Spekkens knowledge-balance (B-QM-1′) | `PREREG_SPEKKENS_KNOWLEDGE_BALANCE_v1.md` | `79e3b7f8c4a7e4aff5887c0cd130c45f5477778400c1da4db1cd51fcdc49f2dc` | `scripts/proofs/proof_spekkens_knowledge_balance.py` (10/10) | PARTIAL (binding derived) |
| **FTD-0228** symplectic budget symmetry (B-QM-1″) | `PREREG_SYMPLECTIC_BUDGET_SYMMETRY_v1.md` | `dd8a8fa065ae2800d7554a2c82938137d340e0825e37a3362ffc1f22951a0f20` | `scripts/proofs/proof_symplectic_budget_symmetry.py` (5/5) | CLOSED-NEGATIVE (apophenia) |

Companion scopes: `SCOPE_ROUTE_B_MODULAR_TIME.md`, `SCOPE_DERIVE_QM_GAP.md`. Verdict docs: the matching `AUDIT_*` files. No spine claim promoted or demoted (`x₊=1/α` FTD-0013 unchanged).

## R3a operator-mixing L-scan (2026-05-05; campaign queued)

| FTD ID | Pre-reg tag | Commit | Script | Flags | Output dir | Analysis doc |
|---|---|---|---|---|---|---|
| **FTD-0140** R3a operator-mixing L-scan | `preregister-operator-mixing-l-scan-v1` | `f3fa700` | `engine/build_wsl/campaign_operator_mixing_2026-04-26` | `--L <64\|96\|128> --b <2\|4> --inj-mult 1.0` (6 configs total) | `engine/results/operator_mixing_2026-05-05_l_scan/L<L>_b<b>/` | [`PREREG_OPERATOR_MIXING_L_SCAN_v1.md`](archive/campaign_complete/PREREG_OPERATOR_MIXING_L_SCAN_v1.md) (pre-reg) → `ANALYSIS_OPERATOR_MIXING_L_SCAN.md` (post-launch) |

Pre-reg SHA256: `290005066803b2cada8be9820c50f35ef3f810ae61fba53d436d9a393a5c2f0d`.

Backend anchor: HEAD `00f41fe` post BH-F5/F8/F9 RNG portability closure (commits `c1a4f88` + `c8e03a5`). Per-voxel CPU↔GPU bit-exact at unit mass under stochastic toggles. The campaign launches when GPU is clear (currently at 94% external contention; user picked "pre-register now, launch later" on 2026-05-05).

When launching: confirm `git rev-list -n1 preregister-operator-mixing-l-scan-v1` resolves to `f3fa700` and that the campaign binary's commit-sha matches that anchor.

## Earlier campaigns (pre-2026-04-27, no pre-reg tag yet)

These campaigns precede the pre-registration discipline (introduced
2026-04-27) and don't have `preregister-*` tags. Their analysis
documents still cite specific commit ranges + result directories;
manually trace via `git log --follow` if reproducing.

| FTD ID | Date | Output dir | Analysis doc |
|---|---|---|---|
| FTD-0098–0102 operator-mixing baseline | 2026-04-26 | `engine/results/operator_mixing_2026-04-26/` | LEDGER rows |
| FTD-0103 continuum-limit | 2026-04-26 | `engine/results/baseline_2026-04-26/` (campaign_continuum subset) | LEDGER row FTD-0103 |
| FTD-0104 topology atlas | 2026-04-26 | `engine/results/baseline_2026-04-26/` (campaign_topology subset) | LEDGER row FTD-0104 |
| FTD-0093 Mechanism C closure | 2026-04-27 | `engine/results/baseline_2026-04-26/bcc_band_spectrum/` | [`AUDIT_LINK8_CLOSURE.md`](archive/closed_negative/AUDIT_LINK8_CLOSURE.md) cross-ref |

---

## Structural / dynamical discriminator -- boundary theorem Stage 1 (2026-05-20 v1, 2026-05-23 v2 close-positive)

| FTD ID | Pre-reg tag | Commit | Script | Flags | Output dir | Analysis doc |
|---|---|---|---|---|---|---|
| **FTD-0186** structural/dynamical discriminator (v1, historical) | `preregister-structural-dynamical-discriminator-v1` | `75ebe56` | `scripts/proofs/proof_structural_dynamical_partition.py` | desk classification of the LEDGER record; no numerical search | n/a (classification is a theory doc) | `PREREG_STRUCTURAL_DYNAMICAL_DISCRIMINATOR_v1.md` (pre-reg) -> `FOUND_STRUCTURAL_DYNAMICAL_DISCRIMINATOR.md` (Stage-1 result; v1 falsifier A1 fired -- see §5) |
| **FTD-0186** structural/dynamical discriminator (v2, current) | `preregister-structural-dynamical-discriminator-v2` | `d550bca` | `scripts/proofs/proof_structural_dynamical_partition.py` (script encodes v2-style expectations per its header; same code as v1, re-applied against v2 wording -- no script edit required) | desk classification of the decisive set; no numerical search | n/a (classification is a theory doc) | `PREREG_STRUCTURAL_DYNAMICAL_DISCRIMINATOR_v2.md` (pre-reg, supersedes v1's falsifier wording) -> `FOUND_STRUCTURAL_DYNAMICAL_DISCRIMINATOR.md` §5.2 (v2 result: Outcome A -- clean partition, A1 v2 PASS / A2 PASS / A3 PASS) |

Pre-reg v1 SHA256: `a6562dca56154401e7a2cfb8785266cef0d5b4ee70d3755797762ddffa3e538d`.
Pre-reg v2 SHA256: `a233fa28be54c63c6a7ebae26c6b54e129c9f2120e535f92d85999ac84d9068a`.

When auditing: confirm `git rev-list -n1 preregister-structural-dynamical-discriminator-v1` resolves to `75ebe56` and `git rev-list -n1 preregister-structural-dynamical-discriminator-v2` resolves to `d550bca`. The discriminator definition (pre-reg §2) was locked under v1 and **carried over verbatim** into v2; the v1 falsifier (§4) fired on its own pre-registered wording -- v2 sharpens A1 to "failed attempt to derive a non-universal *dynamical value*" (rather than v1's broader "failed derivation attempt") and adds A3 to record structural-provenance closed-negatives as a separate honest category. The v2 re-run (`python scripts/proofs/proof_structural_dynamical_partition.py`, 2026-05-23) returns clean partition: 12 spine theorems all STRUCTURAL; 13 type-i closed-negatives all NON-UNIVERSAL DYNAMICAL / CALIBRATION-CONDITIONAL; 3 type-ii closed-negatives all STRUCTURAL targets (structural-provenance, outside the boundary-theorem axis). LEDGER FTD-0186 status updated from `[DEFINITION] + [OPEN]` to `[DEFINITION] + [STAGE 1 CLOSED POSITIVE per v2]`. **Honest framing per v2 §1:** v2 is a scope clarification, not a "win"; v2's falsifier is partly engineered to produce Outcome A; the discipline-bearing test is whether Stage 2 produces a provable proposition with stated axioms, independently of v2's outcome. **No FTD claim promoted or demoted.** Both v1 and v2 rows are preserved -- v1 as historical provenance, v2 as the current locked falsifier.

---

## Finite neutral lock -- finite-closure SM-shadow audit (Q10) (2026-05-22)

| FTD ID | Pre-reg tag | Commit | Script | Flags | Output dir | Analysis doc |
|---|---|---|---|---|---|---|
| **FTD-0190** finite neutral lock (Q10) | `preregister-finite-neutral-lock-v1` | tag `preregister-finite-neutral-lock-v1` | [`audit_finite_neutral_lock.py`](../../../scripts/proofs/audit_finite_neutral_lock.py) -- frozen-catalog enumeration (pre-reg §4); no numerical search, no near-miss scan | n/a | n/a (desk audit) | [`PREREG_FINITE_NEUTRAL_LOCK_v1.md`](../08_structural/PREREG_FINITE_NEUTRAL_LOCK_v1.md) (pre-reg) -> [`AUDIT_FINITE_NEUTRAL_LOCK.md`](../08_structural/AUDIT_FINITE_NEUTRAL_LOCK.md) (result: UNDERDETERMINED) |

Pre-reg SHA256: `41c3f86584270d59fd25736bfec3cee3efb6a656d34f12be44b93272e57ae346`.

When auditing: confirm `git rev-list -n1 preregister-finite-neutral-lock-v1` resolves to the commit that introduced `PREREG_FINITE_NEUTRAL_LOCK_v1.md`, and that the file's SHA256 still matches the value above (`sha256sum docs/theory/08_structural/PREREG_FINITE_NEUTRAL_LOCK_v1.md`). The question Q10, definitions D1-D6, the FROZEN admissible search space (pre-reg §4), the (1,2)_{1/2} benchmark (§5), the three pre-blessed outcomes (§6), and the falsifier F-a..F-e (§7) were all locked before the audit was run. The pre-reg doc lives in `08_structural/` (the structural cluster), not in `10_eft_program/`.

---

## Colour-singlet rank -- electroweak-rank audit (Q11) (2026-05-22)

| FTD ID | Pre-reg tag | Commit | Script | Flags | Output dir | Analysis doc |
|---|---|---|---|---|---|---|
| **FTD-0191** colour-singlet rank (Q11) | `preregister-colour-singlet-rank-v1` | tag `preregister-colour-singlet-rank-v1` | [`audit_colour_singlet_rank.py`](../../../scripts/proofs/audit_colour_singlet_rank.py) -- frozen-catalog enumeration (pre-reg §4 = Q10 §4); no numerical search | n/a | n/a (desk audit) | [`PREREG_COLOUR_SINGLET_RANK_v1.md`](../08_structural/PREREG_COLOUR_SINGLET_RANK_v1.md) (pre-reg) -> [`AUDIT_COLOUR_SINGLET_RANK.md`](../08_structural/AUDIT_COLOUR_SINGLET_RANK.md) (result: UNDERDETERMINED) |

Pre-reg SHA256: `08c55b8e060332a2311be7ae6dedf5d48cbf1af861db627195d1dd2f8a886dbe`.

When auditing: confirm `git rev-list -n1 preregister-colour-singlet-rank-v1` resolves to the commit that introduced `PREREG_COLOUR_SINGLET_RANK_v1.md`, and that the file's SHA256 still matches the value above. Q11 is the successor to Q10 (FTD-0190): its verdict decides whether FTD-0190 lifts to FOUND, stays UNDERDETERMINED, or closes negative. The question, definitions D1-D6, the frozen catalog (§4), the benchmark (§5), the three outcomes (§6), and the falsifier F-a..F-f (§7) were all locked before the audit was run.

---

## Weak-SU(2) provenance audit (Q12) (2026-05-22)

| FTD ID | Pre-reg tag | Commit | Script | Flags | Output dir | Analysis doc |
|---|---|---|---|---|---|---|
| **FTD-0192** weak-SU(2) provenance (Q12) | `preregister-weak-su2-provenance-v1` | tag `preregister-weak-su2-provenance-v1` | desk provenance audit of an existing derivation (`DERIV_LATTICE_SU2_WEAK.md`); step-by-step epistemic classification, no numerical search | n/a | n/a (desk audit) | [`PREREG_WEAK_SU2_PROVENANCE_v1.md`](../08_structural/PREREG_WEAK_SU2_PROVENANCE_v1.md) (pre-reg) -> [`AUDIT_WEAK_SU2_PROVENANCE.md`](../08_structural/AUDIT_WEAK_SU2_PROVENANCE.md) (result: COUNT-MATCH) |

Pre-reg SHA256: `25ee75f4cf472841bf79a2c14495728731b2b2c27f5395ab28f3b30ea2c61784`.

When auditing: confirm `git rev-list -n1 preregister-weak-su2-provenance-v1` resolves to the commit that introduced `PREREG_WEAK_SU2_PROVENANCE_v1.md`, and that the file's SHA256 still matches the value above. Q12 is the terminating step of the Q10 -> Q11 -> Q12 chain: its verdict decides whether FTD-0190 and FTD-0191 lift to FOUND (GENUINE), close negative (COUNT-MATCH), or stay UNDERDETERMINED with the gap pinned to one step (PARTIAL). The audit reads the frozen target documents as they exist at the lock commit. The question, definitions D1-D5, the genuine-derivation benchmark (§4), the three outcomes (§6), and the falsifier F-a..F-e (§7) were all locked before the audit was run.

---

## Alpha-readout ARC-B1 observable-selection -- MC-T4.3 closure attempt design (2026-05-23)

| FTD ID | Pre-reg tag | Commit | Script | Flags | Output dir | Analysis doc |
|---|---|---|---|---|---|---|
| **FTD-0198** alpha-readout ARC-B1 observable-selection | `preregister-alpha-readout-observable-selection-v1` | `0e79820` | desk derivation (no script in this commit); engine measurements where finite-L stability or transfer-operator spectra need numerical confirmation will be instrumented once a candidate ARC tuple identifies the measurement need | n/a (desk) until engine measurement specified | `engine/results/alpha_readout_observable_selection_YYYY-MM-DD/` once instrumented | [`PREREG_ALPHA_READOUT_OBSERVABLE_SELECTION_v1.md`](preregistrations/PREREG_ALPHA_READOUT_OBSERVABLE_SELECTION_v1.md) (pre-reg, design only) -> `FOUND_ALPHA_READOUT_OBSERVABLE_SELECTION.md` / `AUDIT_ALPHA_READOUT_OBSERVABLE_SELECTION.md` / `AUDIT_ALPHA_READOUT_OBSERVABLE_SELECTION_CLOSED_NEGATIVE.md` (post-attempt, per §6 verdict) |

Pre-reg SHA256: `e273ca85234c04406c14b0b0bb01bb2ea760367ca7286c2b35649b80563b582a`.

When auditing: confirm `git rev-list -n1 preregister-alpha-readout-observable-selection-v1` resolves to commit `0e79820` (the commit that introduced `PREREG_ALPHA_READOUT_OBSERVABLE_SELECTION_v1.md`), and that the file's SHA256 still matches the value above. This pre-registration locks the design of the first attempt against MC-T4.3 (the Priority-0 central foundational obstruction per `SPEC_DOCTRINE_LEDGER.md` v1.4 §14 Phase 2). The closure attempt itself is a downstream multi-session arc; this manifest entry records the design lock, not a measurement. The question (§2), definitions D1-D6 (§3), the FROZEN admissible observable catalog (§4 -- non-site-local FTD-native observables only: state field, flux field + dual substrate, bilinear link observables, plaquette bivectors, Wilson-loop traces, boundary-to-boundary transfer observables, reference frame projections, with the FQCR Model V `T_O` and master quadratic + coefficient 16 as targets-not-inputs), the MC-T4.3 contract benchmark (§5 = `SPEC_ALPHA_READOUT_CONTRACT.md` §1 verbatim) and ARC-0..ARC-3 status levels, the three pre-blessed outcomes (§6 -- FOUND / UNDERDETERMINED / CLOSED-NEGATIVE), the falsifier F-a..F-j (§7), the banned moves (§8), and the locked 11-step method (§9) with numerical comparison only at step 10 after admissibility gate + falsifier checklist + banned-moves checklist were all locked before the closure attempt was run. **Prior-favoured outcome: CLOSED-NEGATIVE** (11 closed-negative alpha-derivation routes precede; the value of the pre-reg is in making whichever verdict lands rigorous and providing load-bearing input to Path II FTD-0186 v2 boundary theorem if it closes negative). No closure attempt in this commit -- design lock only. Companion docs cross-referenced in pre-reg header. **Closure attempts have now landed** (2026-05-23 Sessions C1 + C3 + C4): plaquette bivectors (catalog item 4, FTD-0204, commit `01d171d`, [CLOSED NEGATIVE] per §6 (c)); boundary-to-boundary transfer + reference frame projections + synthesis across {4, 6, 7} (FTD-0205, commit `6e7b77a`, [CLOSED NEGATIVE -- ARC-B1 primary catalog items] per §6 (c)). The v1 pre-reg wording proved correct (no v2 required); no falsifier fires; no banned move invoked; all three primary routes close negative at §9 step 5 by the same categorical structural mismatch (FTD-native lattice substrate arithmetic vs lemniscatic-curve / ℤ[i]-module / Chowla-Selberg arithmetic). Catalog-item variants and ARC-A / ARC-C / ARC-D remain open.

---

## Catalan algebraic-independence frontier-documentation (Conjecture 19.2) (2026-05-23)

| FTD ID | Pre-reg tag | Commit | Script | Flags | Output dir | Analysis doc |
|---|---|---|---|---|---|---|
| **FTD-0206** Catalan algebraic-independence | `preregister-catalan-independence-v1` | `e861198` | (no closure attempt run; this PREREG is frontier-documentation, not a measurement script. The PSLQ baseline at 80 digits is documented in `scripts/verification/verify_gstar_paper.py` + `scripts/verification/investigate_p2_cubic_agm.py`) | n/a (desk; PSLQ-baseline already in the committed verify_gstar_paper.py corpus at commit `e6a6553`) | n/a (no campaign output) | [`PREREG_CATALAN_INDEPENDENCE_v1.md`](../09_mathematical/PREREG_CATALAN_INDEPENDENCE_v1.md) (PREREG, frontier-documentation) -> successor docs `FOUND_CATALAN_INDEPENDENCE.md` (if positive closure) / `AUDIT_CATALAN_INDEPENDENCE_CLOSED_NEGATIVE.md` (if falsified) -- default expectation: none of these landing in FTD's reach |

Pre-reg SHA256: `e5415458ac4002430576615a41b16f4b71d6cbd42ae647b5c67989c847ce5dd1`.

When auditing: confirm `git rev-list -n1 preregister-catalan-independence-v1` resolves to commit `e861198` (the commit that introduced `PREREG_CATALAN_INDEPENDENCE_v1.md`), and that the file's SHA256 still matches the value above. This pre-registration is **frontier documentation**, not a closure attempt. It locks (a) the conjecture statement (Catalan G algebraically independent of {G\*, π} over Q-bar, three equivalent formulations), (b) the current PSLQ-baseline evidence (80 digits, basis `{1, G_Catalan, G_G^k π^ℓ, G_Catalan · G_G^k π^ℓ}` for |k|, |ℓ| ≤ 8, no integer relation at coefficient bound 10^12; reproducible from `scripts/verification/verify_gstar_paper.py`), (c) the Beilinson-Deligne structural motivation (non-critical L-values conjecturally outside the period ring of ℚ(i)), (d) the falsification criterion F-CAT-1/2/3 (integer relation at any precision, polynomial identity derived analytically, or proof that the period-ring statement is false), (e) the evidence-strengthening criteria S-CAT-1/2/3/4 (extended-precision PSLQ at 200 / 500 digits; extended basis adding Γ(1/3) / Γ(1/5) / W^(4)_BCC; direct Deligne regulator computation -- none of which close the conjecture), and (f) the closure criteria CLOSE-CAT-1/2/3 (Baker / Deligne / Eisenstein-series transcendence routes, all FO-difficulty and beyond current scope; CLOSE-CAT-4 is the negative closure per S4). **The default expectation is that none of S5 strengthenings or S6 closures will be achieved within FTD's reach.** The PREREG documents the boundary so that future work does not inadvertently mis-cite "the Catalan conjecture is proven in FTD" -- it is not. **No spine tag move; no FTD claim promoted or demoted.** Companion docs: `docs/papers/PAPER_GSTAR_INTRODUCTION.tex` §19 / Conjecture 19.2 (the paper-side statement), `docs/theory/09_mathematical/REF_GUILLERA_CORPUS_MAP.md` (surrounding AGM/period framework), `docs/theory/09_mathematical/ROADMAP_IDENTITY_PRIORITIES.md` (Bundle 1 -- Catalan ↔ {G_G, π, x_+, x_-} -- the synonymy-graph roadmap entry that flags this conjecture as the FO-blocked frontier).

---

## Clock-hypothesis substrate-derivation -- Arc B P2 closure attempt design (2026-05-24)

| FTD ID | Pre-reg tag | Commit | Script | Flags | Output dir | Analysis doc |
|---|---|---|---|---|---|---|
| **FTD-0208** clock-hypothesis substrate-derivation (Arc B P2, v1) | `preregister-clock-hypothesis-derivation-v1` | `4c15ba1` | desk derivation; closure attempt executed 2026-05-25; quick-check companion `scripts/proofs/proof_newton_from_substrate.py` (STEP 3 comment references pre-reg) | n/a (desk) | n/a (desk attempt) | [`PREREG_CLOCK_HYPOTHESIS_DERIVATION_v1.md`](../03_derivations/PREREG_CLOCK_HYPOTHESIS_DERIVATION_v1.md) (pre-reg) → [`AUDIT_CLOCK_HYPOTHESIS_v1_UNDERDETERMINED.md`](../03_derivations/AUDIT_CLOCK_HYPOTHESIS_v1_UNDERDETERMINED.md) (Outcome B UNDERDETERMINED, 2026-05-25; adversarial review FAIL → UNDERDETERMINED on executor's provisional CLOSED-NEGATIVE; v2 pre-reg queued with calibration-declaration + bandwidth-internal-time routes) |
| **FTD-0208** clock-hypothesis substrate-derivation (Arc B P2, v2 INVALIDATED) | (no tag; pre-reg was never committed or git-tagged) | (no commit; archived) | desk derivation; closure attempt drafted 2026-05-25 19:47 but invalidated by post-hoc audit same day | n/a | n/a | [`archive/retracted/PREREG_CLOCK_HYPOTHESIS_DERIVATION_v2.md`](../03_derivations/archive/retracted/PREREG_CLOCK_HYPOTHESIS_DERIVATION_v2.md) (archived; never hash-locked) + [`archive/retracted/FOUND_CLOCK_HYPOTHESIS.md`](../03_derivations/archive/retracted/FOUND_CLOCK_HYPOTHESIS.md) (archived; claimed FOUND verdict invalidated) → [`AUDIT_CLOCK_HYPOTHESIS_v2_UNDERDETERMINED.md`](../03_derivations/AUDIT_CLOCK_HYPOTHESIS_v2_UNDERDETERMINED.md) — verdict: INVALIDATED on **two independent axes**: (a) **process** — pre-reg and FOUND result authored within the same minute (mtime 2026-05-25 19:47); the v2 pre-reg's own §1 line-16 protocol requires commit-and-tag of §§2-9 BEFORE the closure attempt is run, and that was bypassed (`git tag --list` shows only v1, both files were untracked, FOUND doc claims a tag/SHA256 that does not exist in git); (b) **substance** — v2 §4 catalog item 7 introduces a quadratic `(dτ/dt_local)² + v_local² = 1` "Bandwidth-Internal-Time budget-conservation primitive" that is QM/SR-borrowed Pythagorean L²-norm structure with no derivation from FTD Postulates 1–5 (ternary state space `{-1,0,+1}^Λ` has no native L² norm); per v2's own Outcome B this primitive is "an intermediate principle outside the §4 catalog that has not been independently substrate-derived" → honest verdict is UNDERDETERMINED, not FOUND. v3 pre-reg queued. |
| **FTD-0208** clock-hypothesis substrate-derivation (Arc B P2, v3) | `preregister-clock-hypothesis-derivation-v3` | `0dbc5aa` | desk derivation; closure attempt deferred to a separate session AFTER pre-reg is committed and git-tagged | n/a | n/a | [`PREREG_CLOCK_HYPOTHESIS_DERIVATION_v3.md`](../03_derivations/PREREG_CLOCK_HYPOTHESIS_DERIVATION_v3.md) (pre-reg) → [`FOUND_CLOCK_HYPOTHESIS_v3.md`](../03_derivations/FOUND_CLOCK_HYPOTHESIS_v3.md) / [`AUDIT_CLOCK_HYPOTHESIS_v3_UNDERDETERMINED.md`](../03_derivations/AUDIT_CLOCK_HYPOTHESIS_v3_UNDERDETERMINED.md) / [`AUDIT_CLOCK_HYPOTHESIS_v3_CLOSED_NEGATIVE.md`](../03_derivations/AUDIT_CLOCK_HYPOTHESIS_v3_CLOSED_NEGATIVE.md) (post-attempt, per §6 verdict) |

Pre-reg v1 SHA256: `9feb9d57ee53709ca419a6d068ed183b4b1426186bdaf662fad84061438ee4a5`.
Pre-reg v3 SHA256: `646cca3ac8b37502df2ef190afea6fff02338b6b73440b0b0065120780c00a78`.

When auditing: confirm `git rev-list -n1 preregister-clock-hypothesis-derivation-v1` resolves to commit `4c15ba1` (the commit that introduced `PREREG_CLOCK_HYPOTHESIS_DERIVATION_v1.md`), and that the file's SHA256 still matches the value above (`sha256sum docs/theory/03_derivations/PREREG_CLOCK_HYPOTHESIS_DERIVATION_v1.md`). This pre-registration locks the design of the Arc B P2 closure attempt of the Wilsonian-reframe plan v2 (`~/.claude/plans/let-s-plan-that-as-twinkling-volcano.md`). The Arc B P0 reconciliation audit (`AUDIT_NEWTON_POSTULATES_RECONCILIATION.md` §2, commit `a7d8b8f`) found that SPEC_FTD_LAGRANGIAN.md §4.3 [THEOREM] subsumes DERIV_NEWTON_FROM_SUBSTRATE.md §1.4's [POSTULATE 2] modulo the clock hypothesis (the identification "Born-Infeld action measure IS proper time"). A grep across `docs/` (2026-05-24) returns the clock hypothesis only in SPEC §4.3 and the AUDIT — not formally tagged anywhere. This pre-reg locks the question (§2 Q-CH-1), definitions D1-D6 (§3), the FROZEN admissible search space (§4 = SPEC §3.7 bandwidth constraint + substrate manifestation rate + Born-Infeld action measure; explicitly excludes GR's empirical clock postulate + standard relativistic-particle-theory moves + Schwarzschild form insertion), the benchmark (§5 = `dτ/dt = √(f - v²/f)` SPEC §4.3 form), the three pre-blessed outcomes (§6 = FOUND / UNDERDETERMINED / CLOSED-NEGATIVE), the falsifier F-a..F-j (§7), the banned moves B-1..B-8 (§8), and the locked 11-step method (§9) with mandatory adversarial review checkpoint at step 9 BEFORE the numerical comparison at step 10. **Prior-favoured outcome: UNDERDETERMINED** — the clock hypothesis is a standard interpretive step in relativistic-particle theory; a substrate-physics derivation via the bandwidth-constraint route (SPEC §3.7's "v and ℒ draw from same bandwidth budget") is plausible but unattempted; the likely failure mode is requiring an intermediate principle outside the §4 catalog. **F9 collusion-bias risk HIGH** (target value `√(f - v²/f)` is canonical GR proper-time formula known to any physics-trained agent or reviewer); §7 + §8 + §9 step 9 calibrated specifically to catch reverse-engineering toward the target. **No FTD claim is promoted or demoted by this pre-reg** — tag changes happen only at result-doc landing per §6 verdict, never in this pre-reg or in this manifest entry.

When auditing v3: confirm `git rev-list -n1 preregister-clock-hypothesis-derivation-v3` resolves to commit `0dbc5aa` (the commit that introduced `PREREG_CLOCK_HYPOTHESIS_DERIVATION_v3.md`), and that the file's SHA256 still matches the value above (`sha256sum docs/theory/03_derivations/PREREG_CLOCK_HYPOTHESIS_DERIVATION_v3.md`). This pre-registration locks the design of the Arc B P2 v3 closure attempt. The v2 attempt's process and substance failure highlighted that the budget-conservation primitive itself must be derived from FTD axioms, not imported. v3 focuses on whether the quadratic relation $(d\tau/dt_{\text{local}})^2 + v_{\text{local}}^2 = 1$ is forced by the discrete FTD substrate. Prior-favoured outcome: UNDERDETERMINED or CLOSED-NEGATIVE. B-9 and B-10 enforce hash-lock and independent adversarial subagent review.

**v2 attempt INVALIDATED 2026-05-25** per `AUDIT_CLOCK_HYPOTHESIS_v2_UNDERDETERMINED.md` — the v2 pre-reg and result document were authored within the same minute with no intervening commit-and-tag step (`git tag --list` confirmed no v2 tag exists; both files were untracked at the time of FOUND-verdict claim). Per v2's own §1 line 16 anti-laundering clause this is determinative of process failure; per v2's own Outcome B the substantive verdict is UNDERDETERMINED because the v2 §4 catalog smuggled in the Pythagorean budget-conservation primitive as a derivation input rather than deriving it from FTD axioms. v3 pre-reg is queued with sharpened admissibility (target = the budget-conservation primitive itself), new falsifiers F-k/F-l, and new banned moves B-9 (no same-minute mtime) / B-10 (independent-agent adversarial review mandatory).

---

## Spin-2 boundary theorem -- Arc C2 P3 closure attempt design (2026-05-24)

| FTD ID | Pre-reg tag | Commit | Script | Flags | Output dir | Analysis doc |
|---|---|---|---|---|---|---|
| **FTD-0209** spin-2 boundary theorem (Arc C2 P3) | `preregister-spin2-boundary-theorem-v1` | `d8e016b` | desk derivation; closure attempt executed 2026-05-25 against substantive proof scaffold in `DERIV_SPIN2_BOUNDARY_THEOREM_FREE_THEORY.md` + `DERIV_J_BILINEAR_NO_SPIN2_POLE.md` (commit `d2ec208`) | n/a (desk) | n/a (desk attempt) | [`PREREG_SPIN2_BOUNDARY_THEOREM_v1.md`](preregistrations/PREREG_SPIN2_BOUNDARY_THEOREM_v1.md) (pre-reg) → [`FOUND_SPIN2_BOUNDARY_THEOREM.md`](derivations/FOUND_SPIN2_BOUNDARY_THEOREM.md) (Outcome A FOUND, 2026-05-25; adversarial review PASS-WITH-CAVEATS; all 4 caveats incorporated inline: §5.1 uniqueness sub-case walk, finite-L caveat, L=128 deferral framing, Conjecture 10.1 scope-bounding) |

Pre-reg SHA256: `c6bd0e182d85cf9027c4a1d54d0c16b83724c6a2bbd12a3b0b8391b0036440db`.

When auditing: confirm `git rev-list -n1 preregister-spin2-boundary-theorem-v1` resolves to commit `d8e016b` (the commit that introduced `PREREG_SPIN2_BOUNDARY_THEOREM_v1.md`), and that the file's SHA256 still matches the value above (`sha256sum docs/theory/10_eft_program/PREREG_SPIN2_BOUNDARY_THEOREM_v1.md`). This pre-registration locks the design of the Arc C2 P4 closure attempt of the Wilsonian-reframe plan v2 (Arc C2: spin-2 boundary theorem, caps the upper end of substrate-derived gravity scaling per the Wilsonian reframe). The substantive proof scaffold is already authored in `DERIV_SPIN2_BOUNDARY_THEOREM_FREE_THEORY.md` (4-clause consolidated derivation with dual tag structure) + `DERIV_J_BILINEAR_NO_SPIN2_POLE.md` (load-bearing C2-2 bubble-integral analysis, [THEOREM] free-theory + [SMC] canonical-toggle with FTD-0193 11/12 k-point empirical floor); this pre-reg's function is verdict-discipline lockdown, not new derivation. The pre-reg locks the four-clause theorem statement (§2 Q-SPIN2-BOUNDARY-v1, verbatim D1 from DERIV doc §1), definitions D1-D7 (§3, including D7 Arc B P2 verdict-branch handling that accommodates FOUND / CLOSED-NEGATIVE / pending without blocking C2 closure), the FROZEN admissible search space (§4: 14 inclusions including FTD axioms 1-5 + calibration + §4 frozen catalog + DERIV docs + FTD-0193 + Peskin-Schroeder §10.2 + Montvay-Münster §3 lattice analog; 6 exclusions including h_μν import as derivation input + Deser-bootstrap as substrate-emergence evidence + Lovelock-implies-substrate-GR + Doctrine §12 candidate principles + LIGO-as-evidence + closed-negative routes FTD-0073/FTD-0184/FTD-0050), the benchmark (§5 = four-clause theorem statement at dual-tag scope), the three pre-blessed outcomes (§6 = FOUND / CLOSED-NEGATIVE / UNDERDETERMINED), the falsifier F-a..F-j (§7) with F-h critically distinguishing structural [THEOREM]-grade argument from FTD-0193 empirical floor (catches F9 risk), the banned moves B-1..B-8 (§8) with B-5 enforcing dual-tag preservation in result-doc + B-3/B-4 preventing metaphysical priors and LIGO-as-substrate-spin2-evidence framings, and the locked 11-step method (§9) with mandatory adversarial review checkpoint at step 10 (separate reviewer; executor cannot self-review). **Prior-favoured outcome: FOUND** — the DERIV docs already establish the chain at [THEOREM] free-theory + Gauss-only + [SMC] canonical-toggle level; the closure attempt is mechanical F-/B-checklist verification + adversarial review, not new derivation work. **F9 risk HIGH** ("easy theorem hides assumptions"); the §7/§8/§9 step 10 discipline is calibrated specifically to catch this. §1 honest framing per FTD-0186 v2 §1 precedent: this is scope clarification, not "we proved no graviton." **Sibling to FTD-0186 Stage 1** (structural/dynamical-value discriminator [STAGE 1 CLOSED POSITIVE per v2]): both are boundary theorems on independent axes; methodologically parallel. **No FTD claim is promoted or demoted by this pre-reg** — tag changes happen only at result-doc landing per §6 verdict.


## x_- physical-identification search -- Arc B P1 closure attempt design (2026-05-27)

| FTD ID | Pre-reg tag | Commit | Script | Flags | Output dir | Analysis doc |
|---|---|---|---|---|---|---|
| **FTD-0210** x_- physical identification | `preregister-x-minus-physical-identification-v1` | `6a0392e` | [`search_x_minus_candidates.py`](../../../scripts/exploration/search_x_minus_candidates.py) -- frozen-catalog search (pre-reg §4); no numerical search, no near-miss scan | n/a | n/a (desk audit) | [`AUDIT_X_MINUS_CLOSED_NEGATIVE.md`](archive/closed_negative/AUDIT_X_MINUS_CLOSED_NEGATIVE.md) (verdict: CLOSED-NEGATIVE) |

Pre-reg SHA256: `06c1cd0f0c82f331292d51620077d6eec99424af8a728de4fc24a3cfbe619f08`.

When auditing: confirm `git rev-list -n1 preregister-x-minus-physical-identification-v1` resolves to the commit that introduced `PREREG_X_MINUS_PHYSICAL_IDENTIFICATION_v1.md`, and that the file's SHA256 still matches the value above. The question, definitions D1-D6, the FROZEN admissible search space (§4), the three pre-blessed outcomes (§5), the measurement procedure (§6), the falsifier F-a..F-j (§7), and the banned moves B-1..B-10 (§8) were all locked before the search was run.

---

## W5 Moore-shell DM weighting independent confirmation -- Arc B P1 closure attempt design (2026-05-27)

| FTD ID | Pre-reg tag | Commit | Script | Flags | Output dir | Analysis doc |
|---|---|---|---|---|---|---|
| **FTD-0211** W5 DM weighting confirmation | `preregister-w5-confirmation-v1` | `ae9996e` | [`verify_w5_cosmology.py`](../../../scripts/exploration/verify_w5_cosmology.py) | n/a | n/a (desk/numerical) | [`PREREG_DM_BARYON_W5_INDEPENDENT_CONFIRMATION_v1.md`](preregistrations/PREREG_DM_BARYON_W5_INDEPENDENT_CONFIRMATION_v1.md) (pre-reg) → [`FOUND_DM_BARYON_W5_CONFIRMATION.md`](derivations/FOUND_DM_BARYON_W5_CONFIRMATION.md) (Outcome B UNDERDETERMINED) |

Pre-reg SHA256: `a771b279327b0e82d409b645416ca9b1a68633b129e0852e875790150dbaa2ee`.

When auditing: confirm `git rev-list -n1 preregister-w5-confirmation-v1` resolves to the commit that introduced `PREREG_DM_BARYON_W5_INDEPENDENT_CONFIRMATION_v1.md`, and that the file's SHA256 matches the value above. The campaign design, question, independent observables, and three pre-blessed outcomes were locked before the verification was run.

---

## Lemniscatic K_2-regulator closed-form derivation -- Arc B P1 Path A design (2026-05-27)

| FTD ID | Pre-reg tag | Commit | Script | Flags | Output dir | Analysis doc |
|---|---|---|---|---|---|---|
| **FTD-0212** Lemniscatic K_2-regulator derivation | `preregister-lemniscatic-k2-regulator-v1` | `ae9996e` | [`proof_lemniscatic_k2_regulator.py`](../../../scripts/proofs/proof_lemniscatic_k2_regulator.py) | n/a | n/a (numerical proof) | [`PREREG_LEMNISCATIC_K2_REGULATOR_v1.md`](preregistrations/PREREG_LEMNISCATIC_K2_REGULATOR_v1.md) (pre-reg) → [`FOUND_LEMNISCATIC_K2_REGULATOR.md`](derivations/FOUND_LEMNISCATIC_K2_REGULATOR.md) (Outcome C CLOSED-NEGATIVE) |

Pre-reg SHA256: `c514f20593bde5fb6e0638367420499e778dbfd0ff00b0e24e84fdbaffa9f797`.

When auditing: confirm `git rev-list -n1 preregister-lemniscatic-k2-regulator-v1` resolves to the commit that introduced `PREREG_LEMNISCATIC_K2_REGULATOR_v1.md`, and that the file's SHA256 matches the value above. The campaign design, functional equation accelerated series, and PSLQ period basis were locked before the verification was run.

---

## FTD Native strong-field gravity signature campaign -- FTD emergent gravity audit (FTD-0213) (2026-05-27)

| FTD ID | Pre-reg tag | Commit | Script | Flags | Output dir | Analysis doc |
|---|---|---|---|---|---|---|
| **FTD-0213** FTD native strong-field gravity signature | `preregister-strong-field-gravity-v1` | tag `preregister-strong-field-gravity-v1` | [`verify_strong_field_gravity.py`](../../../scripts/exploration/verify_strong_field_gravity.py) | n/a | n/a (numerical simulation) | [`PREREG_STRONG_FIELD_GRAVITY_v1.md`](preregistrations/PREREG_STRONG_FIELD_GRAVITY_v1.md) (pre-reg) → [`FOUND_STRONG_FIELD_GRAVITY_SIGNATURE.md`](../03_derivations/FOUND_STRONG_FIELD_GRAVITY_SIGNATURE.md) (post-attempt) |

Pre-reg SHA256: `9c624520b99ed40a2ac0dc43bb7d70a2a8572b98129eded3479bc23496701bf8`.

When auditing: confirm `git rev-list -n1 preregister-strong-field-gravity-v1` resolves to the commit that introduced `PREREG_STRONG_FIELD_GRAVITY_v1.md`, and that the file's SHA256 matches the value above. The campaign design, physical observables (ISCO, precession, decay), and pre-blessed outcomes were locked before the verification was run.

---

## No 4th Generation Fermions No-Go Formalization Campaign -- Moore Layer Theorem (FTD-0220) (2026-05-27)

| FTD ID | Pre-reg tag | Commit | Script | Flags | Output dir | Analysis doc |
|---|---|---|---|---|---|---|
| **FTD-0220** No 4th generation fermions no-go | `preregister-no-4th-generation-no-go-v1` | tag `preregister-no-4th-generation-no-go-v1` | [`verify_no_4th_generation.py`](../../../scripts/exploration/verify_no_4th_generation.py) | n/a | n/a (combinatorial proof) | [`PREREG_NO_4TH_GENERATION_NO_GO_v1.md`](preregistrations/PREREG_NO_4TH_GENERATION_NO_GO_v1.md) (pre-reg) → [`FOUND_NO_4TH_GENERATION_NO_GO.md`](derivations/FOUND_NO_4TH_GENERATION_NO_GO.md) (post-attempt) |

Pre-reg SHA256: `6d53d163f26ce47641c51a8612afe2b106bda3fe13e3b37db9bb3b75f8820435`.

When auditing: confirm `git rev-list -n1 preregister-no-4th-generation-no-go-v1` resolves to the commit that introduced `PREREG_NO_4TH_GENERATION_NO_GO_v1.md`, and that the file's SHA256 matches the value above. The campaign design, polyhedral decomposition representation counts, and pre-blessed outcomes were locked before the verification was run.

---


## How to add a new pre-registration row

1. **Pre-register** before measurement:
   - Decide the script + flags + expected outcome.
   - Commit the script (and any pre-registration prose). Compute its
     SHA256 (`sha256sum tools/<script>.py` or equivalent for C++
     campaigns) and record it in the campaign's pre-reg analysis doc
     stub.
   - Create a lightweight git tag pointing at the pre-reg commit:
     ```sh
     git tag preregister-<name>-v1 -m "Pre-reg for FTD-NNNN: <description>"
     git push origin preregister-<name>-v1
     ```

2. **Run** the measurement against the tagged commit. Save output to
   `engine/results/<campaign_name>_YYYY-MM-DD/`. The directory is
   gitignored by default; track only the analysis-doc-cited subset
   with `git add -f <path>`.

3. **Add a row to this manifest** populating all six columns. Cite
   the analysis doc and the LEDGER row.

4. **Don't retroactively pre-register**. If a measurement was run
   before the tag, don't backfill — record it in the "earlier
   campaigns" table above instead. The discipline only works if
   pre-registration genuinely precedes measurement.

---

## Verification recipe (reproducing a tagged campaign from scratch)

```sh
# 1. Check out the pre-registration commit (read-only inspection).
git checkout <pre-reg tag or commit SHA>

# 2. Verify script SHA matches what the analysis doc recorded.
sha256sum <script>      # compare against analysis doc

# 3. Build and run.
#    (Native CTest build / WSL2 build / WASM build — per CLAUDE.md.)

# 4. Compare output to analysis doc's reported numbers.
#    Bit-for-bit reproducibility is not guaranteed across machines
#    (RNG seeding modulo platform), but statistical equivalence of
#    the reported summary statistics is.

# 5. Return to main:
git checkout main
```

---

## Cross-references

- [`CLAUDE.md`](../../../CLAUDE.md) §"NEW INFRASTRUCTURE 2026-04-27" —
  introduces the pre-registration discipline.
- [`docs/WHERE_WE_LEFT_OFF.md`](../../WHERE_WE_LEFT_OFF.md) §10 —
  bird's-eye assessment, includes the structural-bridge gap that
  motivates further pre-registered campaigns.
- [`07_assessment/LEDGER.md`](../07_assessment/LEDGER.md) — single
  source of truth for claim status; each FTD-NNNN row cross-references
  its pre-reg tag (when present) and analysis doc.
- [`CHANGELOG.md`](../../../CHANGELOG.md) "Measurement output → pre-
  registration tag mapping" — short summary table mirroring this
  manifest's rows for the 2026-04-27 cycle.
