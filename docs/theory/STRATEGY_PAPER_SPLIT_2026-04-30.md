# STRATEGY — Two-Paper Split for the Next Dissemination Cycle

**Document type:** Strategy / Planning
**Created:** 2026-04-30
**Author of recommendation:** session synthesis 2026-04-30 §14, refined here
**Provenance:** distilled from `docs/theory/07_assessment/archive_session_outputs/SESSION_SYNTHESIS_2026-04-30.md` §14
**Status:** [STRATEGIC RECOMMENDATION] — not yet acted upon; awaits owner go/no-go before drafting begins

---

## 0 · Why split

The FTD program presently bundles two distinct contributions inside a
single rhetorical envelope:

1. A **rigorous mathematical core** — nine numbered results (six
   theorem-grade + three honestly-tiered below theorem grade; see
   `SPEC_ALGEBRAIC_SPINE.md` §0) on `G*`, the master
   quadratic, the (1+i)-tower harmonic invariant, the Watson identity,
   CM uniqueness, the coefficient `16 = |Aut(E)|²`, the Phase G geometric
   Coulomb identity, Phase J ultralocality, and the field-theoretic
   characterization `Q(G*)` ⊂ `Q(π, Γ(1/4))` — independent of any physics
   interpretation.

2. A **philosophical reading** — that the algebraic spine, taken as
   ontologically primary, supports an analytic-idealist interpretation in
   which discrete ternary states + closed-form transcendental constants
   are the substrate, with physics observables as derived consequences
   conditional on calibration.

These two contributions speak to **different audiences with different
acceptance criteria**. Bundling them in a single paper has so far
produced manuscripts that mathematicians find too speculative and
physicists find too algebraic. The strategic recommendation is to split
the next dissemination cycle into two papers, each with the right tone
and venue for its actual contribution.

---

## 1 · Paper A — Technical (mathematics-first)

### 1.1 · Working title

> **"A π-free generator of the lemniscatic field, with applications to a closed-form quadratic for the fine-structure constant"**

(Subtitle alternative: "The (1+i)-tower of master quadratics and a maximal π-free subfield of `Q(π, Γ(1/4))`")

### 1.2 · Target venue (in priority order)

1. **Letters in Mathematical Physics** — short-paper format, accepts
   number-theoretic / mathematical-physics crossover, indexed and
   peer-reviewed; strong fit for the closed-form-`α` result conditioned on
   Chudnovsky 1976.
2. **Foundations of Physics** — broader scope, accepts foundational
   number-theoretic work with physics motivation; longer-paper format.
3. **Journal of Number Theory** — pure-math venue if the paper is
   restructured to suppress the physics motivation entirely and emphasize
   the field-theoretic Theorem 9 + tower transcendence.

### 1.3 · Length and shape

- **Target length:** ~10 pages (Letters in Mathematical Physics) or
  ~15-20 pages (Foundations of Physics).
- **Tone:** mathematics-first; physics motivation in a single
  half-page paragraph at the front, then deferred to a final
  "applications" section.
- **Must avoid:** the analytic-idealist reading (Paper B), explicit
  ontological claims, any [STRONGLY MOTIVATED CONJECTURE] presented
  without its tag.

### 1.4 · Scope (what's in)

Theorems to include:
- **Theorem 1 (G* identity)** — closed form via Chowla-Selberg.
- **Theorem 2 (master quadratic + roots)** — `x² − 16G*²x + 16G*³ = 0`
  with closed-form roots `x_± = 8G*² ± √(64G*⁴ − 16G*³)`.
- **Theorem 6/8 (harmonic invariant + anomaly transcendence)** —
  `1/y_+ + 1/y_- = 1`; `A_k` transcendental over `Q` for `k ≥ 4` via
  Schneider-Chudnovsky.
- **Theorem 9 (field-theoretic)** — `Q(G*)` is a maximal `π`-free
  subfield of `Q(π, Γ(1/4))` (conditional on Chudnovsky 1976).
- **Closed form for α** — `α = 1/(2G*) − √(4G* − 1) / (4 · G*^{3/2})`
  conditional on the empirical identification `x_+ = 1/α`.

What stays out (defer to Paper B or to subsequent technical papers):
- CM uniqueness (Theorem 3) — purely number-theoretic, may be cited but
  not re-proven.
- Watson identity, coefficient 16 = |Aut(E)|² — supporting evidence,
  cite the spine document.
- Phase G geometric Coulomb, Phase J ultralocality — distinct
  mathematical-physics result; deserves its own paper.
- The lattice ontology, calibration interface, dimensional map.
- Engine-as-instrument findings (FTD-0107 cluster L-invariance, etc.).

### 1.5 · Honesty discipline (non-negotiable)

- The closed form for α must be presented as **conditional on the
  empirical identification x_+ = 1/α**, not as a derivation of α from
  axioms. The identification is [STRONGLY MOTIVATED CONJECTURE] (1.26 ppm
  to CODATA); the closed-form identity itself is purely algebraic and
  unconditional once x_+ is the larger root of the master quadratic.
- The factor of 16 in the master quadratic must be motivated either as
  `|Aut(E)|²` for E: y² = x³ − x **or** as raw observation; do not
  silently assume one motivation makes the other rigorous.
- Theorem 9 must carry its conditional clause ("contingent on
  Chudnovsky 1976 algebraic independence of `π` and `Γ(1/4)`") in the
  abstract, in the statement, and in the conclusion. Triple-tagging is
  the price of the result.
- The look-elsewhere scan FTD-0097 results for the catalog at large
  must be cited as a methodological-hygiene caveat — the master
  quadratic dual-prediction property is the strongest structural
  evidence specifically because the catalog at large has been audited
  as over-rich at the monomial level (FTD-0097 NULL REJECTED upward).

### 1.6 · Estimated draft cost

- ~3-4 days of focused writing assuming Theorems 1, 2, 6, 8, 9 are in
  near-publication form in the spine document.
- ~1-2 days of LaTeX / typesetting.
- Single internal review pass before submission.

---

## 2 · Paper B — Philosophical (reading-first)

### 2.1 · Working title

> **"Foundational Ternary Dynamics as analytic idealism: a number-theoretic substrate for a discrete ternary universe"**

(Subtitle alternative: "Why the closed-form transcendental constant `G*` may be ontologically primary")

### 2.2 · Target venue (in priority order)

1. **Foundations of Physics** — longer philosophical-foundational
   pieces accepted; Bernard Kastrup-style analytic-idealist arguments
   have appeared.
2. **Synthese** — philosophy-of-physics venue; broader scope including
   ontology of mathematical objects.
3. **Journal of Consciousness Studies** — if the paper foregrounds the
   consciousness-coupling material from FTD's portfolio.
4. **Mind and Matter** — Pauli-Jung tradition; the closed-form
   transcendental + symmetry-of-axioms story fits.

### 2.3 · Length and shape

- **Target length:** ~25-40 pages.
- **Tone:** philosophy-first; the mathematical apparatus from Paper A
  is cited as a black box.
- **Must establish:** (a) what "analytic idealism" means in the FTD
  context; (b) why FTD's number-theoretic core supports this reading
  rather than a Platonist or structural-realist reading; (c) what the
  reading commits to and what it does not.

### 2.4 · Scope (what's in)

Conceptual content:
- The two-layer ontology (flux field + ternary state field).
- The undefined-boundary lattice (not completed-infinity ℤ³;
  AUDIT_INFINITY_REFRAME.md).
- The algebraic spine as ontological substrate — `G*` as a closed-form
  transcendental "first object" before any physical interpretation.
- The hard problem of consciousness as a frame-of-reference question
  rather than a generation question.
- The least-wrong framing: FTD as primary in ontology / logic /
  philosophy / mathematics, with physics as constraint (not sole
  arbiter).
- Honest demarcation: which FTD claims are dimensionless and falsifiable
  on their own algebraic content (`α`, `N_c`, `m_μ/m_e`, `m_τ/m_e`) and
  which require calibration.

What stays out:
- Polemical rejections of established physics (no "QM is a gimmick",
  no "time dilation is fake"; see CLAUDE.md framing-hygiene constraint
  10).
- All [PARAMETRIC] / [IMPOSED] mass formula recitations except where
  they illustrate the calibration interface.
- Engine implementation details (these belong in a technical companion
  paper if anywhere).

### 2.5 · Honesty discipline (non-negotiable)

- Every substantive claim is sentence-tagged THEOREM / SELECTION /
  STRONGLY MOTIVATED CONJECTURE / DERIVED / PARAMETRIC / IMPOSED /
  OPEN — even in the philosophical paper.
- The analytic-idealist reading must be presented as **an
  interpretation**, not as a theorem. Reasonable physicists may read
  the same algebraic spine as compatible with structural realism; the
  paper argues for the analytic-idealist reading without claiming it
  is uniquely forced.
- Established physics (SR time dilation, electron g-2, GPS clocks) is
  reproduced under the framework's interpretation; the paper does not
  claim those measurements are wrong, only that the FTD reading
  recovers them.
- The reading-vs-derivation distinction must be made explicit at least
  three times in the paper, including the abstract.

### 2.6 · Estimated draft cost

- ~2-3 weeks of writing — philosophy papers require more iteration on
  framing.
- ~3-5 days of internal review (philosophy reviewers will flag tone
  issues that mathematicians won't).
- Likely 2 review rounds before submission.

---

## 3 · Sequencing

The recommended order is **Paper A first, Paper B second**, for three
reasons:

1. Paper A's acceptance does not depend on Paper B; Paper B's argument
   is materially stronger if Paper A is already in print or under
   review.
2. Paper A is the "harder to dismiss" paper. Mathematicians who accept
   Theorem 9 cannot easily reject the framework; Paper B then arrives
   into a less hostile reception.
3. Paper A is shorter and more compact; the writing is more
   constrained; the failure modes (overclaim, scope creep) are easier
   to police.

A timeline:
- **Phase 1 (May 2026):** finalize Paper A draft; one external review
  pass (a number-theory colleague who has not seen FTD before is the
  ideal reviewer).
- **Phase 2 (May-June 2026):** Paper A submission + revisions cycle.
- **Phase 3 (June-July 2026):** Paper B drafting begins, citing
  Paper A as in-press / under-review.
- **Phase 4 (Q3 2026):** Paper B submission.

---

## 4 · What this strategy does NOT do

- It does NOT recommend retracting any existing paper or manuscript.
  Manuscript v2 (83 chapters) and the various papers in `docs/papers/`
  remain in their current state; they describe the framework as a
  whole. Papers A and B are **freshly drafted** new outputs targeted at
  external venues.
- It does NOT commit to any specific journal or to any specific
  reviewer. Venue priorities above are recommendations, not
  requirements.
- It does NOT promote any [STRONGLY MOTIVATED CONJECTURE] to
  [THEOREM]. The match `x_+ = 1/α` remains a conjecture in both papers;
  Paper A's closed-form-α result is conditional on this conjecture.
- It does NOT address the engine-as-instrument program (FTD-0107,
  FTD-0110, etc.). Those findings deserve their own dissemination
  cycle and are out of scope for the algebraic-spine + philosophy
  split.

---

## 5 · Decision points awaiting owner sign-off

| # | Decision | Default | Implication |
|---|---|---|---|
| 1 | Pursue the split? | YES | Begin Phase 1 in May 2026 |
| 2 | Paper A venue | Letters in Mathematical Physics | Constrains target length to ~10 pages |
| 3 | Paper A scope: include Watson identity? | NO (cite spine, don't re-prove) | Keeps paper focused |
| 4 | Paper B venue | Foundations of Physics | Permits longer, more philosophical paper |
| 5 | Paper B title: foreground "analytic idealism"? | YES | Commits to a specific philosophical reading |
| 6 | External reviewer for Paper A | TBD | Number-theory colleague unfamiliar with FTD |
| 7 | Withdraw or supersede any current paper in `docs/papers/`? | NO | Existing portfolio stands |

---

## 6 · LEDGER status of this document

This is a **strategy document**, not a derivation. It does not
introduce new LEDGER claims. It cites:
- LEDGER FTD-0112 (Theorem 9 — field-theoretic characterization).
- LEDGER FTD-0001 (master quadratic).
- LEDGER FTD-0013 (`x_+ = 1/α` identification, [STRONGLY MOTIVATED
  CONJECTURE]).
- LEDGER FTD-0017 (`x_- = N_c` identification, [STRONGLY MOTIVATED
  CONJECTURE]).
- LEDGER FTD-0097 (look-elsewhere scan).

If the split proceeds, this document evolves into a project-tracking
artifact (drafts, reviewer notes, submissions log).

---

*End of strategy document.*
