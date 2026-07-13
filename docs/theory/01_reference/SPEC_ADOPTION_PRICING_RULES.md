# SPEC — Adoption Pricing Rules (Unification Annex Stage U0)

**Tag:** [SCOPE / METHODOLOGY — DRAFT awaiting owner ratification at D5/D6/D7] — proposes the rules by which a candidate axiom is priced and judged for adoption. Ratifies nothing, adopts nothing, promotes no claim; no purchase may cite these rules as settled until the owner rulings are booked.
**LEDGER id:** FTD-0387 · **Date:** 2026-07-13 · **Charter:** `SCOPE_CONSUMPTION_PROGRAM.md` §5.5 (the Unification Annex), Stage U0 — the pricing rules that gate every Stage-U2 purchase.
**Data:** [`adoption_pricing.json`](adoption_pricing.json) (canonical) · **Verifier:** `scripts/proofs/proof_adoption_pricing.py`.
**Precedence:** LEDGER > constitution > this document > other prose. On existing prices and falsifiers, `import_ledger.json` (FTD-0371) is authoritative; this document adds a decision rule over those prices and revises none of them.

---

## §0 · Scope and status

Stage U1 assembled the register of types in force (FTD-0386). Stage U2 proposes to extend that register by adopting new axioms, each priced. An adoption cannot be judged honestly until the pricing rules are fixed in advance; otherwise a candidate's cost and yield are negotiable after the fact, which is the failure this stage exists to prevent. This document proposes three rules, corresponding to the three owner sittings the charter schedules: **D5**, the currency in which cost is measured (§1); **D6**, the compression predicate that relates cost to explanatory yield (§2); **D7**, the application of both to FC-W, the one axiom already adopted, which calibrates the rule against a real datum (§3).

Everything below is a proposal. The bit-equivalents, weights, and margins are declared methodology parameters, not derived quantities; each is a choice the owner ratifies, revises, or rejects at the corresponding sitting. Until then no candidate is priced under these rules and no purchase proceeds. The rules bind the *process* of adoption; they introduce no physical claim and move no epistemic tag.

## §1 · D5 — The currency

The import ledger already denominates cost in five currencies (`import_ledger.json` meta): the **adopted-bit** (a binary structural choice the substrate cannot force), the **selected-type** (a declared selection among discrete alternatives), the **named-result** (an external mathematical result the framework leans on), the **calibration** (a dimensional scale anchor), and the **empirical-identification** (a "this mathematics is this physics" bridge). The proposed D5 ruling fixes how these enter an adoption decision.

**Reporting stays stratified.** The ledger continues to report cost as a vector across the five currencies with no headline scalar, preserving the standing reading guard that the single adopted bit is never cited as the total physics import. Collapsing the vector to one number is the abuse the ledger was built to prevent, and D5 does not license it for reporting.

**The predicate uses a conservative bit-equivalent, declared for the decision only.** A compression predicate must relate cost to yield, which requires a cost scalar. D5 proposes a bit-equivalent conversion used solely inside the §2 predicate and never exported as a headline cost:

| Currency | Proposed bit-equivalent (structural cost) | Rationale |
|---|---|---|
| adopted-bit | 1 bit | definitional — a binary structural choice |
| selected-type | log₂(\|alternatives\|); 1 bit for a binary selection; flagged-unknown for an open alternative set | a selection among N discrete options carries log₂ N bits of choice |
| named-result (proven) | 0 bits of adoption cost; tracked as a dependency | a proven external theorem is a fact, not a choice the framework makes; it constrains but does not cost |
| named-result (open) | 0 bits of cost; tracked as a **risk** | an open result is a conditionality the yield inherits, recorded on the risk ledger rather than the cost side |
| calibration | excluded from the predicate (irreducible floor) | A2 grade-0 closure (FTD-0368; `FOUND_DIMENSIONAL_GRADE_CLOSURE.md`) shows at least one dimensional anchor is irreducible; a forced floor cannot be compressed away and is not weighed against yield |
| empirical-identification | not a cost — counted on the yield side | a bridge is content the axiom licenses, measured in §2, not a cost the axiom incurs |

The predicate therefore weighs **structural adoption cost only** — adopted-bits and selected-types in bit-equivalents, plus any adopted dynamics posit at its own declared bit-equivalent. Calibrations sit on a separate irreducible floor; named-results are dependencies or risks; empirical-identifications are yield.

**Revision trigger.** A candidate whose selected-type has an open (non-enumerable) alternative set cannot be priced until the set is bounded; such a candidate routes back to the owner rather than through the predicate.

## §2 · D6 — The compression predicate

The charter states the predicate informally: an axiom may be adopted only if it explains more than its own degrees of freedom. D6 proposes the formal statement.

**Cost.** c(A) is the structural adoption cost of candidate A in D5 bit-equivalents (§1), excluding the calibration floor and external dependencies.

**Yield.** y(A) is the content A licenses that was not licensed before, counted in **distinct gap-classes** rather than raw rows, and weighted by structural strength:

- a licensed `[CONDITIONAL THEOREM]`, `[DERIVED]`, or a retired `[OPEN]`→`[THEOREM]` wall: weight 1 per distinct gap-class;
- a licensed `[MEASURED]` engine signature at a stated protocol: weight ½ per distinct gap-class;
- a licensed `[PARAMETRIC]` match: weight 0 — a borrowed functional form filled with framework numbers adds no structural content, and a large catalogue makes such matches unsurprising (the standing F10 caution). Parametric rows are recorded as consequences but contribute nothing to yield.

The unit weight credits the conditional content the axiom licenses given itself — the quantity the compression accounting is meant to measure — not substrate-level theorems; it does not assert that a `[CONDITIONAL THEOREM]` given a new axiom has the standing of a substrate `[THEOREM]`.

The gap-class unit is the anti-gaming guard: rows that share one mechanism count as one gap-class, so buying ninety hadron rows that all follow from one posited mechanism counts as one gap-class, not ninety, matching the program metric that rows are counted only with distinct gap-classes named.

**The predicate.** Adopt A only if y(A) > c(A) — the licensed structural content, in distinct weighted gap-classes, strictly exceeds the adoption cost in bit-equivalents. Equality is a marginal outcome, admissible only when the single gap-class is high-value (independently checkable and itself free of tunable parameters), and is flagged as marginal on the record. The quantity y(A) − c(A), and the index y(A)/c(A) used for ordering in §4, are **decision heuristics, not information-theoretic measures**: cost is in bit-equivalents and yield in weighted distinct gap-classes, so the two sides are not commensurable and the index carries no compression-theoretic meaning beyond ranking. It operationalizes the charter's "explain more than its own degrees of freedom" as a declared rule, and its parameters are what the owner ratifies.

**Two guards preserve honesty.** First, a candidate whose entire yield is parametric fails by construction (y = 0), which blocks the route of adopting an axiom that merely relabels fitted numbers as licensed content. Second, yield counts only content the candidate licenses that is *independent* of the candidate's own free parameters; a candidate cannot pay for itself by licensing restatements of its own posit.

## §3 · D7 — The FC-W calibration

FC-W is the one axiom already adopted. D7 applies §1–§2 to it. The retro-test does not adjudicate FC-W: an owner-declared `[AXIOM]` is not subject to disqualification by these rules, and because the marginal-outcome clause is defined to fit the FC-W point, the exercise is calibration by construction, not independent validation. Its purpose is to anchor the predicate's floor to the one existing adoption so that future candidates are judged against a real datum rather than an arbitrary threshold.

**Cost.** FC-W is one adopted-bit — the δ branch, a binary ℤ/2 orientation the substrate cannot force. c(FC-W) = 1 bit, the minimal possible structural cost.

**Yield.** FC-W licenses one gap-class in the electromagnetic-coupling sector. Two statements are kept separate and both stand: given W, the selected master-quadratic root satisfies x₊ = 1/α as a `[CONDITIONAL THEOREM given W]` (constitution §3.5); the physical identification of x₊ with the measured fine-structure constant remains `[SMC]` (FTD-0013, FTD-0386 U-4). That gap-class is high-value — it is the framework's single scan-rigid identification (FTD-0319, a tolerance-conditioned `[NUMERICAL FACT]`, not a structural or Bayesian result) — but it is one gap-class, and the constitution records that on present evidence W does no work beyond the α-root. y(FC-W) = 1 high-value gap-class.

**Floor.** y = c = 1: FC-W sits at the marginal point, which the rule takes as the floor. This is consistent with — not a restatement or revision of — FC-W's standing of record, which the constitution already marks declared-but-conditional, with full commitment gated on the `[OPEN]` condition that W's carrier force independent structural content. In the predicate's terms, that open condition is exactly what would raise y(FC-W) above one gap-class. FC-W therefore fixes the scale: one bit buying one high-value, scan-rigid gap-class is the weakest adoption the framework has on record, and a candidate costing more than one bit must license proportionally more distinct gap-classes to reach the same point.

## §4 · Illustrative candidate ordering (non-binding)

Applying the proposed predicate to the five Stage-U2 candidates, pricing each headline posit as one structural choice (except the UV distribution, whose degrees of freedom are counted explicitly), produces the ordering below. It is illustrative only: the estimates are not owner-ratified, no candidate has its narrowing theorem or independence proof, each stated yield is itself conditional on proofs not yet built, and nothing here authorizes a purchase. Outcomes are taken against the FC-W floor — index y/c above 1 passes, equal to 1 is marginal, below 1 fails.

| Rank | Candidate | Cost | Yield (distinct gap-classes) | y/c | Outcome (as estimated) |
|---|---|---|---|---|---|
| 1 | P6C-C (compact SU(3) links) | 1 | 1 — confinement and the QCD scaffold (one mechanism) | 1.0 | marginal |
| 2 | P6C-G (spin-2) | 1 | 1 — the gravity sector (full nonlinear EFE via Deser, itself conditional on Conjecture 10.1) | 1.0 | marginal |
| 3 | P6C-M (ℤ/3 sharpness) | 1 | 1 — Born-sharpness | 1.0 | marginal |
| 4 | P6C-U (UV/initial conditions) | 8 | 1 — the cosmology apparatus, heavily conditional | 0.125 | fail |
| 5 | P6C-F (matter axiom) | 1 | 0 — consolidates already-priced imports (IMP-S4, IMP-E1∘E3) without new independent content | 0.0 | fail |

Two outcomes are counterintuitive and honest. The candidate covering the most physics (P6C-U) is the weakest, because an initial-condition distribution is many degrees of freedom for one soft, heavily conditional gap-class. And the tied-cheapest candidate (P6C-F) fails outright: consolidating already-priced imports into one declared type licenses no *new* independent content and so, under the independence guard, yields zero — consolidation may still be desirable for bookkeeping, but it does not pay for itself as structural yield. Neither result is visible from cost or from headline scope alone.

**The yield-individuation question — what D6 must settle.** Under the conservative default proposed here (one posit priced as one gap-class, no weight for sector breadth), every single-posit candidate lands at the marginal point, the same grade as FC-W: P6C-C, P6C-G, and P6C-M are indistinguishable at y/c = 1. The predicate cleanly separates the two failures but does not, by itself, rank the three viable candidates. Making it discriminate requires one or both of two declared design choices, each of which D6 must settle and neither of which the metric can settle for itself. The first is **yield individuation**: P6C-C's confinement and hadron spectrum both follow from compact SU(3) gauge dynamics, so the anti-gaming rule counts them as one gap-class; whether distinct *phenomena* from a single mechanism may count separately is a ruling, not a fact. The second is **sector scope**: P6C-G buys the entire gravity sector yet scores as one gap-class, the same as Born-sharpness; whether breadth of a single sector counts toward yield is likewise a ruling, and any scope weight introduces its own gaming risk. Left unweighted, the predicate rewards a multiplicity of distinct mechanisms over the breadth of any one, and reports the three viable candidates as a tie — which is the honest state of the estimate until D6 fixes the individuation rule.

## §5 · Verification and the ratification gate

`python scripts/proofs/proof_adoption_pricing.py` recomputes the FC-W calibration from the declared cost and yield, checks that the predicate's floor equals the FC-W point, checks that the illustrative ordering is monotonic in the yield-to-cost index and that each stated outcome matches the floor rule, and asserts that the document claims no ratification and adopts nothing. **Ratification gate:** these rules take effect only when the owner books D5 (currency), D6 (predicate, including the yield-individuation and sector-scope rulings §4 surfaces), and D7 (acceptance of the FC-W calibration) in the LEDGER; until then the document stands at `[DRAFT awaiting ratification]` and no Stage-U2 candidate is priced under it. A ratified rule set becomes the pricing authority the register's §4 maintenance trigger points to when a purchase moves from candidate to adopted.

*Standing invariants: x₊ = 1/α remains [SMC]; MC-T4.3 remains [FOUNDATIONAL OBSTRUCTION]; FC-W remains [AXIOM] (the calibration confirms its standing, it does not move it); D=3 remains [SELECTION — declared]; no tag moves; this document prices the process of adoption and introduces no claim.*
