# PRE-REGISTRATION — Adversarial Look-Elsewhere Scan (FTD-0189)

**Tag:** [PRE-REGISTRATION]. Locks the methodology of a measurement **before** the measurement is run.
**Date:** 2026-05-21.
**LEDGER row:** FTD-0189 (to be added when the campaign is registered/run).

**2026-05-22 v1.4 cleanup-taxonomy annotation (outside frozen scope).** This pre-registration was hash-locked on 2026-05-21 under the framework state then in force, which included the `x_- ↔ N_c` identification (LEDGER FTD-0014, [STRONGLY MOTIVATED CONJECTURE]). On 2026-05-22, **after this pre-registration was locked**, FTD/FQCR Cleanup Taxonomy v1.4 §5 retired the `x_- ↔ N_c` identification (LEDGER FTD-0014 removed in commit `ca7eb61`). Per pre-registration discipline (§0), the frozen §§2–5 content — including the target pair `(1/α, N_c)`, the dual-matcher definition, and the decisive statistics — is preserved verbatim at registration-time wording. The scan's outcome (Outcome A, achieved) speaks to the polynomial-template-uniqueness of the master quadratic across an FTD-undesigned 18-constant basket, which is a fact **independent of the target identification**: it remains a statement about which polynomial-template-plus-constant matches the *numerical pair* `(137.036…, 3.024)` at the declared tolerances. The interpretation of the second target as "N_c" reflects the pre-v1.4 framework state; the scan's structural-uniqueness finding stands and is now the canonical structural-uniqueness evidence for the single live identification `x_+ ↔ 1/α` (FTD-0013).

> **ID-renumber note (2026-05-21).** This campaign was originally numbered **FTD-0187** — the ID under which it was hash-locked (commit `9e5ad8f`, git tag `preregister-adversarial-look-elsewhere-v1`). That same ID had been concurrently committed in `LEDGER.md` to an unrelated claim (the Born-rule derivation-status consolidation row, ~12 inbound citations), creating a collision. The look-elsewhere scan was therefore renumbered to **FTD-0189** (FTD-0188 is the κ_ψ = 4π audit). The immutable git commit/tag and the hash-locked runner `tools/scan_adversarial_look_elsewhere.py` retain the literal string "FTD-0187" as frozen registration-time provenance — the runner's SHA256 (§6) is **deliberately not edited**, so the hash-lock stays intact and verifiable. **FTD-0189 is the canonical LEDGER ID for this scan.**
**Runner:** `tools/scan_adversarial_look_elsewhere.py`, SHA256 `764a6a8c4d27fce6bd45d42e1c5714fcdfaf1ca72fc10fde8e98d189e40a27d9`.
**Hash-lock status:** **PENDING** (see §7). This pre-registration is **not in force** until the git tag `preregister-adversarial-look-elsewhere-v1` is created over a commit containing this file and the runner. **The scan must NOT be run before that tag exists** — a pre-run measurement voids the pre-registration.

---

## §0 — Pre-registration discipline

Everything in §§2–5 below — the family, the constant basket, the coefficient grids, the targets, the dual-matcher definition, the decisive statistics, and the outcomes — is **fixed now, before measurement**. The git tag locks the runner's SHA256 at registration time. Any edit to §§2–5 invalidates v1 and forces a fresh v2 (a new hash-lock + a re-run); the result of a v2 wording cannot be retro-credited to v1. The scan's outcome is reported as it returns — **including a result unfavourable to FTD** — with no reinterpretation.

## §1 — Purpose and motivation

The master quadratic `x² − 16 G*² x + 16 G*³` has roots x₊ ≈ 137.036 (claimed ≈ 1/α) and x₋ ≈ 3.024 (claimed ≈ N_c = 3). FTD-0121 reports a **~4×10⁵:1 Bayes weight** for that dual-match, from `scripts/proofs/proof_polynomial_look_elsewhere_extended.py` (a 2.87M-polynomial scan).

**The problem.** Every polynomial in that 2.87M scan has the form `x² − c₁·G*ᵖ·x + c₂·G*ᵍ` — **G\* is the only constant in the entire family.** A Bayes factor conditioned on a family FTD designed around G* is not evidence that G* is special; it measures only that, among G*-polynomials, the master quadratic is the rare dual-matcher. `CATALOG_PARAMETRIC_INSERTIONS.md` already concedes "the interpretation as Bayes evidence depends on the prior choice of family." A physics-panel review (2026-05-20, recorded in the forward plan as priority P1) made the demand explicit: **test the dual-match against a polynomial family FTD did not design.**

**This scan.** It runs the identical polynomial template over a frozen basket of 18 standard mathematical constants — G* among them on identical footing — and counts dual-matchers per constant. If G* is genuinely special, the master quadratic stands alone; if the dual-match is a look-elsewhere artifact, other constants reproduce it.

## §2 — The polynomial family (mechanical exhaustivity rule)

The family is defined by a **mechanical rule with no designer discretion**: every (constant, coefficient, exponent) combination over the stated grids is included — nothing selected, nothing excluded.

### §2.1 The constant basket — FROZEN, 18 entries

`G_star`≈2.958675, `pi`≈3.141593, `e`≈2.718282, `sqrt2`≈1.414214, `sqrt3`≈1.732051, `sqrt5`≈2.236068, `golden_phi`≈1.618034, `euler_gamma`≈0.577216, `ln2`≈0.693147, `apery_zeta3`≈1.202057, `catalan`≈0.915966, `varpi_lemn`≈2.622058, `gauss_G`≈0.834627, `sqrt_pi`≈1.772454, `gamma_1_3`≈2.678939, `R3_equianh`≈1.978363, `khinchin`≈2.685452, `glaisher`≈1.282427.

Computed at 50-digit precision by mpmath in the runner's `_build_constants()`. **G\* enters as entry 0 with no privilege**; the runner is constant-agnostic. The basket is standard named mathematical constants — it contains no FTD framework integers, no α-powers, no FTD-specific quantities.

### §2.2 The templates — FROZEN

- **Degree 2:** `x² − c₁·Kᵃ·x + c₂·Kᵇ`, with c₁,c₂ ∈ [1,64] and a,b ∈ [0,5]. The coefficient grid is **identical to FTD-0121 EXT-A** — so each constant's subfamily is directly comparable to the FTD-designed G*-scan. Family size: 18 × 64² × 6² = **2,654,208** polynomials.
- **Degree 3 (secondary):** `x³ − c₁·Kᵃ·x² + c₂·Kᵇ·x − c₃·Kᶜ`, c ∈ [1,12], exponents ∈ [0,4].

### §2.3 The F9 (collusion-bias) guard

The family is mechanical; G* is one of 18 constants on identical footing; the runner does not know which constant FTD cares about. **Recommendation:** before hash-lock, the constant basket (§2.1) should be reviewed by someone outside the FTD project, to confirm it is not gerrymandered.

### §2.4 The magnitude footnote (honest scope)

The 18 constants span magnitudes ≈0.58 (γ) to ≈3.14 (π). With the c ∈ [1,64] grid, constants of magnitude below ≈1 (`euler_gamma`, `ln2`, `catalan`, `gauss_G`) cannot reach the target root neighbourhood (x₊+x₋ ≈ 140) and will trivially show **0** dual-matchers — for a magnitude reason, not because G* is special. This makes the test **conservative**: a non-G* dual-matcher can only arise from a *magnitude-compatible* constant (π, e, √3, √5, φ, ζ(3), ϖ, √π, Γ(1/3), R₃, khinchin, glaisher — all in G*'s magnitude ballpark), which is exactly the meaningful look-elsewhere signal. The decisive comparison is **G\* vs those ≈12 magnitude-compatible constants**. The magnitude-dead constants are kept in the basket (no exclusion = no discretion); their 0-counts are simply uninformative.

## §3 — Targets and the dual-matcher definition — FROZEN

- Targets: 1/α = 137.035999177 (CODATA 2022, per `REF_EXTERNAL_CONSTANTS.md`); N_c = 3 (exact integer).
- Degree 2: x₊ = larger root, x₋ = smaller root. `resid_+ = |x₊ − 1/α| / (1/α)`; `resid_− = |x₋ − N_c| / N_c`.
- **Dual-matcher:** `resid_+ < 2.0×10⁻⁶` AND `resid_− < 1.0×10⁻²`. (The master quadratic's own precision is resid_+ = 1.26 ppm and resid_− = 0.80%; the thresholds are marginally looser so the master quadratic counts comfortably, not on a knife-edge.)
- Degree 3: a cubic dual-matches if some ordered pair of its real roots satisfies the same. G* cubics whose matching pair equals the master-quadratic roots are flagged **embeddings** (master quadratic × linear factor) and excluded from the genuine count.
- Transparency grid: counts also reported at resid_+ ∈ {10⁻³, 10⁻⁴, 10⁻⁵, 2×10⁻⁶, 10⁻⁶}, x₋ gate fixed at 10⁻².

## §4 — Decisive statistics

- **Per-constant count.** Number of dual-matchers for each of the 18 constants. Every constant's subfamily has identical size (64²×6² = 147,456 degree-2 polynomials), so counts are directly comparable.
- **Global ranking.** All degree-2 polynomials whose x₋ is within 1% of N_c, ranked by resid_+ — with the master quadratic's rank, the gap to rank 2, and the constant-identity of the top entries.
- **Null reading.** Under "G* is not special," dual-matchers spread across magnitude-compatible constants in proportion to subfamily size. The decisive quantities are **D_nonG** (dual-matchers whose constant ≠ G*) and **n_distinct** (distinct non-G* constants producing ≥1).

## §5 — Pre-registered outcomes

- **Outcome A — the dual-match survives.** `D_nonG = 0`. The master quadratic stands alone across the adversarial basket. → FTD-0013 (`x₊ = 1/α`) **retains [STRONGLY MOTIVATED CONJECTURE]**; its evidential basis is **upgraded** — the dual-match now survives a family FTD did not design, and the Bayes argument may be honestly restated over the adversarial family.
- **Outcome B — look-elsewhere artifact.** `D_nonG ≥ 3` OR `n_distinct ≥ 3`. Other constants reproduce the dual-match comparably. → the ~4×10⁵:1 Bayes figure is **retracted as family-conditioned**; FTD-0013 is **honestly demoted toward [CONJECTURE]**; `SPEC_ALGEBRAIC_SPINE.md` §11 and `TRACKER_ONTIC_TRUTH.md` OT-5.1 are updated accordingly.
- **Outcome C — ambiguous.** `1 ≤ D_nonG ≤ 2` and `n_distinct ≤ 2`. Each non-G* dual-matcher is then structurally analysed: if every one is algebraically equivalent to the master quadratic (shared Galois closure / trivial rescaling), Outcome A handling applies; if any is an independent algebraic object, Outcome B handling applies. No tag change until the structural analysis is complete and recorded in the post-run analysis doc.

This pre-registration commits to reporting whichever outcome the scan returns. **Outcome B is a genuine and welcome result** — it would honestly close a methodological question the framework has carried open since FTD-0097.

## §6 — The runner

`tools/scan_adversarial_look_elsewhere.py` — SHA256 `764a6a8c4d27fce6bd45d42e1c5714fcdfaf1ca72fc10fde8e98d189e40a27d9`. Deterministic; no RNG. Output → `engine/results/adversarial_look_elsewhere_2026-05-21/` (`matchers_degree2.json`, `matchers_degree3.json`, `summary.json`). The runner prints the per-constant table, the global ranking, the transparency grid, and the pre-registered Outcome.

## §7 — Hash-lock protocol (user-gated)

Per project commit policy, the commit and tag are explicit user actions. To put this pre-registration in force:

```sh
git add tools/scan_adversarial_look_elsewhere.py \
        docs/theory/10_eft_program/PREREG_ADVERSARIAL_LOOK_ELSEWHERE_v1.md
git commit -m "Pre-register FTD-0189 adversarial look-elsewhere scan"
git tag preregister-adversarial-look-elsewhere-v1 -m "Pre-reg: FTD-0189"
# verify the runner SHA matches §6 BEFORE running:
sha256sum tools/scan_adversarial_look_elsewhere.py
# only then:
python tools/scan_adversarial_look_elsewhere.py
```

Then add the FTD-0189 row to `LEDGER.md`, register the campaign in `REF_PREREGISTER_MANIFEST.md`, and write the post-run analysis doc `ANALYSIS_ADVERSARIAL_LOOK_ELSEWHERE.md`.

## §8 — Honesty notes

- The scan tests one specific, decisive question: is `x₊ = 1/α`'s dual-match special to G*, or an artifact of an FTD-designed family. It does not bear on the *physics* of the identification (that is the separate, deeper MC-T4.3 obstruction); it tests only the *evidential standing* of the structural-uniqueness argument.
- Outcome B does not destroy FTD's algebraic spine. The master quadratic remains a `[THEOREM]` as algebra; the spine theorems stand. Outcome B would demote one [SMC] — `x₊ = 1/α` — to a weaker tag, and that is the honest, correct response to a family-conditioned Bayes factor.
- The magnitude footnote (§2.4) is a known, stated limitation; it makes the test conservative, not unfair.

## §9 — Cross-references

- `FTD-0013` (`x₊ = 1/α`, [SMC]) — the claim under test.
- `FTD-0097` (`tools/scan_look_elsewhere.py`) — the original monomial look-elsewhere scan.
- `FTD-0121` (`scripts/proofs/proof_polynomial_look_elsewhere_extended.py`) — the 2.87M G*-only scan whose family conditioning this scan removes.
- `SPEC_ALGEBRAIC_SPINE.md` §11; `TRACKER_ONTIC_TRUTH.md` OT-5.1 — updated under Outcome B.
- `REF_PREREGISTER_MANIFEST.md` — register the campaign at hash-lock time.
- Forward plan priority **P1** (`.claude/plans/take-the-role-of-fancy-kahn.md`).
