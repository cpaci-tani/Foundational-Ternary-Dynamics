# AUDIT — Provisional Two-Specialist Review (Mathematics + Physics), 2026-07-01

**Tag:** `[CRITIC SYNTHESIS]` — adversarial review record, not a theory claim.
**LEDGER id:** FTD-0347.
**Targets:** the algebraic spine (`SPEC_ALGEBRAIC_SPINE.md`, `TRACKER_ONTIC_TRUTH.md`, the K-BIND /
route-invariance / FC-W carrier-narrowing chain) and the boundary/frontier docs
(`AUDIT_BOUNDARY_MAP.md`, `FOUND_MODULUS_ARGUMENT_FRONTIER.md`,
`DERIV_SPIN2_BOUNDARY_THEOREM_FREE_THEORY.md`, `CATALOG_PARAMETRIC_INSERTIONS.md`,
the engine-theory benchmark scorecards, `CLAUDE.md`'s headline claims).

---

## §0 · Mandatory disclosure — what this review is and is not

**This review, like every prior adversarial pass in this corpus, was performed entirely by AI**
(see `TRACKER_OPEN_ITEMS.md` §0, the standing external-review-status item): two specialist
reviewer personas — an outside-pure-mathematician role and an outside-theoretical-physicist role,
defined in `.claude/agents/ftd-math-redteam.md` and `.claude/agents/ftd-physics-redteam.md`
(both specified `model: fable`; this run executed on Opus 4.8 because Fable was gated at run
time) — each instructed to form judgments from primary sources only, to recompute every
high-precision numerical claim rather than trust stated digits, and to search the corpus before
accusing. Every raw finding was then re-checked by a separate, independent AI skeptic instructed
to **refute** it (default verdict "refuted" unless the defect could be independently confirmed
from the cited files). 17 agents total (2 reviewers + 15 verifiers), workflow `wf_5799fb83-615`.

This is a real internal-consistency check with genuinely adversarial structure. It is **not**
external human validation, and it does not discharge the standing external-review item. A real
outside mathematician or physicist may catch things these reviewers cannot (blind spots shared
with the system that produced the corpus) and may dismiss concerns raised here as unfounded.

---

## §1 · Headline verdicts (both specialists, independently)

- **Mathematics:** "the load-bearing ALGEBRA is genuinely solid, and the project's epistemic
  hygiene around the physics identification is unusually disciplined — I could not find a
  promotion of x₊=1/α to [THEOREM]/[DERIVED] anywhere, and the G\*/ϖ (FTD-0117) conflation is
  correctly handled throughout." Verified by independent recomputation at 40–60 digits: the G\*
  identity, the master-quadratic roots (1.257 ppm vs CODATA-2022, matching the stated 1.26 ppm),
  the closed-form α_tree, Watson W₃, the CM-uniqueness arithmetic theorem ("three-case proof is
  airtight"), and the K-BIND transcendence core (sound conditional on Chudnovsky 1976; the
  Chudnovsky-vs-Nesterenko attributions resolve cleanly — Nesterenko's 3-element theorem strictly
  implies Chudnovsky's 2-element result, both documented correctly).
- **Physics:** "FTD's physics program, read at the primary-source level, is more honest than its
  reputation for numerology would suggest — per-claim epistemic tagging inside docs/theory/ is
  genuinely, unusually disciplined." g-2/Lamb-shift correctly `[PARAMETRIC]`; the spin-2 boundary
  scrupulously scoped PROVEN-free-theory vs ATTEMPTED-interacting; the 6th-postulate-family
  boundary arguments survive scrutiny as physics with concrete falsifiers.
- **Shared diagnosis:** the surviving defects are **bookkeeping and propagation, not
  mathematics** — roll-up counts drift from their per-claim sources, and corrections applied in
  one file fail to reach every copy (the same two systemic patterns
  `AUDIT_REDTEAM_DISSECTION_2026-07-01.md` §5 named).

---

## §2 · Findings that survived independent verification

| # | Sev | Verdict | File | Defect (one line) | Status |
|---|-----|---------|------|--------------------|--------|
| 1 | HIGH | CONFIRMED | `CLAUDE.md` :296, :120 | "hydrogen spectrum (A+) … Bell S=2 (A+)" still graded as validated physics — the exact benchmark inversions FTD-0345 fixed in `analyze_convergence.py` / `benchmark_engine_theory.cpp` but never propagated to the file every agent reads first; verifier traced commit `435f5a98` editing adjacent lines and skipping these | **FIXED (FTD-0347)** |
| 2 | MEDIUM | CONFIRMED | `SPEC_ALGEBRAIC_SPINE.md` :4/:18 vs :205/:308/:737-9/:808 | Self-contradicts its own headline count — "seven theorem-grade + two tiered" (§0) vs "six + three" (§4/§12/§14) — because Theorem 3 bundles a genuine arithmetic `[THEOREM]` (\|μ_K\|=\|disc(K)\| unique to ℚ(i), OT-1.9 T1) with a demoted physics `[NUMERICAL FACT]` (the d=−4 dual-match privilege, which flips under the rational-multiplier criterion) | **FIXED (FTD-0347)** — count unified at 7+2 with the Theorem-3 split stated explicitly at every count site |
| 3 | MEDIUM | CONFIRMED | `CLAUDE.md` :110 | "Bell violation S = 2√2" overstates the canonical source, which states outright "FTD does not violate Bell" — the 2√2 is imported QM conditional on the `[SELECTION]` singlet, and the substrate's own falsifier (FC-1) *requires* S≤2 natively | **FIXED (FTD-0347)** |
| 4 | MEDIUM | **PLAUSIBLE — OPEN, owner judgment required** | `FOUND_OPERATOR_CALCULUS_AXIOMATIZATION.md` §1 | K-BIND's `[THEOREM]` closes a generator set whose members are each chosen with invariants in ℚ(G\*) — Lemma 1 is near-tautological relative to that choice. The verifier found the doc **already self-scopes** ("relative to the admissible FTD-native construction set") and that FTD-0314 treats FTD-0244 as *extended, not contradicted* — so the tag is defensible within its stated scope — but a genuine residual tension stands: §5's boxed "K-BIND [OPEN] → [CLOSED THEOREM-NEGATIVE]" transition over-reads for a reader of FTD-0244 alone, while FTD-0314 keeps the analytic-period sub-route `[OPEN]`. **Whether the axiomatized generators are *representative* of what the substrate can construct is a question for a working Galois/transcendence theorist, not resolvable by another documentation pass.** | **NOT FIXED — recorded as open flag** |
| 5 | LOW | CONFIRMED | `LEDGER.md` :325 (FTD-0336 row) | The row still carries "five-fold independent mathematical convergence" — the framing its own source doc (`FOUND_MODULUS_ARGUMENT_FRONTIER.md` §5) corrected to "one structural fact, five vocabularies" under FTD-0346; under LEDGER>prose precedence the canonical record asserted what the source disavows | **FIXED (FTD-0347)** |
| 6 | LOW | CONFIRMED | `analyze_convergence.py` :32, :67 | Benchmark B2 ("alpha recovered 2.4%") is circular — α is the input by construction (`ALPHA_EFT = G_C²`, static_assert-pinned; the force code's Coulomb amplitude IS α in every mode) — self-disclosed only in a buried REMAINING-GAPS line while the B2 scorecard row and EM domain-grade carry no inline caveat, unlike the freshly-caveated B7/B16 | **FIXED (FTD-0347)** |
| 7 | LOW | CONFIRMED | `lean/FTD/Axioms.lean` | Every axiom body is the trivial `True` with the real statement in a comment — the file is citation bookkeeping wearing Lean syntax, not machine-checked formalization; nothing downstream consumes these axioms to prove a nontrivial theorem (the genuinely rigorous artifact is the separate `FtdNoGo/Standalone.lean` tree); the "sorry-debt: 10 proven in literature" report overstates what Lean verifies | **FIXED (FTD-0347)** — header disclosure added, no axiom changed |

## §3 · Attacked and survived (reported for transparency)

Findings the reviewers raised and the independent skeptics **refuted** — i.e. things
specifically attacked that held up:

- **The G\*/ϖ distinction and all core numerics** — every high-precision claim in
  `SPEC_ALGEBRAIC_SPINE.md` reproduced independently at 60 digits; the FTD-0117 warning is
  handled correctly; plugging ϖ into the master quadratic gives 107.3, exactly as the doc warns.
- **The CM-uniqueness arithmetic theorem** (three-case proof re-derived independently; the
  companion unit test re-run, 1 passed) — and the doc honestly separates the arithmetic
  `[THEOREM]` from the physics `[NUMERICAL FACT]`.
- **Chudnovsky-1976 vs Nesterenko-1996 attributions** — mathematically consistent (stronger
  implies weaker), correctly documented in both the bibliography and the Lean comment.
- **The spin-2 boundary scoping** — the PROVEN/ATTEMPTED split is respected everywhere checked;
  the verifier found the doc actually *under-claims* current empirical coverage (L=128 resolved
  in the report but the DERIV still says deferred — a conservative-direction lag).
- **The QM-non-commutativity boundary (§2a)** — genuinely structural, multi-angle, self-flags
  its weakest angle (ℤ/3 "apophenia"), correctly scoped to the *stated* algebra with a concrete
  falsifier mirrored in the LEDGER.
- **g-2 / Lamb-shift `[PARAMETRIC]` filing** — the catalog's note is "exactly the correct
  physics"; consistent across `CATALOG_PARAMETRIC_INSERTIONS.md` §10, `CLAUDE.md`, and the
  atomic-dynamics audit.
- **The frontier's §6 remediated framing** — the FTD-0346 fix verified as structural, not
  cosmetic (old "most defensible asset" phrasing gone from the live doc; per-instance falsifiers
  present; meta-conjecture honestly tagged).
- **TRACKER_ONTIC_TRUTH's 9-vs-7 accounting** — refuted as a defect: the two counts partition
  along different axes (tier membership vs spine membership) and both are internally correct;
  T2 "conditional" means conditional on a published theorem, not on a conjecture. (The skeptic
  did note a real but separate staleness: the Tier-2 header says "(3)" over a 7-row table —
  minor, logged here, not in scope.)

---

## §4 · What this triggered

The six CONFIRMED items were fixed under **FTD-0347** (same pass as this capture doc; see the
LEDGER row and `CHANGELOG_REFRAME.md` 2026-07-01 entry). The one PLAUSIBLE item (K-BIND
generator representativeness, finding 4) is **deliberately not fixed** — it is exactly the kind
of question the standing external-review item (`TRACKER_OPEN_ITEMS.md` §0) exists for, and
another AI pass editing the doc would simulate resolution without providing it.

**Zero promotions:** `x₊=1/α` stays `[STRONGLY MOTIVATED CONJECTURE]`; MC-T4.3 stays
`[FOUNDATIONAL OBSTRUCTION]`; no α derived; golden hash `0xb604d81a3d79366e` untouched. Every
fix downgrades an overclaim, reconciles a count to its per-claim sources, or discloses a gap.
