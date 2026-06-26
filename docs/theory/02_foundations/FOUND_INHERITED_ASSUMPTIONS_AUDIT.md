# FOUND — The Inherited-Assumptions Audit

**Tag:** `[SYNTHESIS]` (a methodology + a status-map; re-states existing FTD claims at their canonical tags, introduces **no new theorem and no promotion**)
**Date:** 2026-06-26 · **LEDGER:** FTD-0329 · **Companion to:** the constitution (`SPEC_FTD_FRAMEWORK_V1.md`, FTD-0254)

> **Purpose.** The formalism of 20th-century physics was assembled fast, pragmatically, with the mathematics available in ~1900–1935, and a philosophy (operationalism / Copenhagen) was bolted on because the math worked and no one knew why. A large fraction of what is treated as bedrock is **historically-contingent scaffolding, not discovered truth.** This document is the founding artifact of FTD's *realist-reconstruction* program: it audits the load-bearing inherited objects of physics, classifies each, and records — honestly, at canonical tags — where FTD's discrete logical core rebuilds it, where it declines it, and where it simply hits a boundary. **The audit is as much a map of what FTD does *not* reach as of what it does.**

---

## 0 · The thesis (and the precise claim)

Read precisely, the program is **not** "the math is wrong" and **not** "rebuild physics on intuition." It is:

> Distinguish, for each load-bearing object, whether it is a **logical/ontological primitive**, a **derived theorem**, or a **postulate of 1925-convenience** kept because it was the available mathematics — and, where it is convenience, find the **cleaner primitive that re-derives it as an aggregate/emergent/symmetry shadow while still recovering every empirical consequence.**

This is the constitution's ordering made operational: **Ontology > Logic > Math > Physics**. The math is correct and predictive; it is *ontologically agnostic*. The rebuild supplies the ontology the formalism is silent about, and demands the formalism fall out as the aggregate description.

---

## 1 · The method (the three-question audit)

For every inherited object, ask:

1. **What is it?** A primitive (irreducible commitment), a theorem (forced by the primitives), or a convenience postulate (fitted, not forced)?
2. **If convenience: necessary or contingent?** Was it kept because it is *logically forced*, or because it was the math on hand in 1925?
3. **If contingent: what re-derives it?** Which cleaner primitive produces it as a *shadow* — an aggregate, an emergent IR limit, a symmetry sub-structure — **while recovering the data**?

---

## 2 · The discipline (what separates this from crankery — non-negotiable)

"Rebuild on logic, not just math" is also the opening line of every crank manifesto. The line between *serious reconstruction* and *freestyle* is exactly five constraints, all enforced by FTD's existing machinery (the LEDGER, the tags, the deviation ledger, pre-registration):

- **R1 — Recover the old math as a limit/theorem, never discard it.** If a rebuild *loses* a working prediction, it is wrong, not deep. (Born must come back as the aggregate; non-commutativity must come back as the spin-cover shadow.)
- **R2 — Be *more* constrained, not less.** "Logic not math" is a demand for *fewer, sharper* primitives that force the same structure — not a license to add freedom.
- **R3 — The empirical floor is untouchable.** Logic untangles *scaffolding*, never *data*. Measured Bell violations stay; the rebuild must *recover* them, it cannot reason them away.
- **R4 — Primitives must be explicit, minimal, and named.** Appeals to "common sense" drift into crankery; the rebuild is only as good as its *declared* bedrock.
- **R5 — Tag honestly.** Every cell below carries its canonical tag. A rebuild that is `[SELECTION]` is not written as `[DERIVED]`; a declined import is not written as a triumph.

---

## 3 · FTD's declared primitives (the logical bedrock the rebuild rests on)

R4 requires these be stated, not assumed:

- **P1–P5** — discrete finite (undefined-boundary) lattice; discrete time; ternary state `{−1,0,+1}`; local (26-neighbour Moore) causality; determinism.
- **The first distinction** — the ternary `{−1,0,+1} = {i², 0, |i²|}`, the real projection of `ℤ[i]^× ∪ {0}` (FTD-0128).
- **FC-0** — the order-4 planar symmetry read as `ℤ[i]`; the `i`-act, `√(−1)`, the one *generative* act of intent (FTD-0322).
- **The Framework Commitments** — FC-1 (declines the measurement map M), FC-2 (native arrow; emergent metric), FC-3 (scale-covariance), FC-W = FC-4 (the adopted α-binding law). Each is an `[AXIOM]`-class **declaration**, not a derivation.

Everything in §4 is rebuilt *from* these, recovered *as* the old math, or honestly marked as still-imported / boundary.

---

## 4 · The audit

Rebuild status legend: **REBUILT** = a canonical `[THEOREM]`/`[DERIVED]` recovers it; **PARTIAL** = recovered in one layer, open/imported in another; **DECLINED** = FC-1/FC-2 refuses the import by design; **BOUNDARY** = the honest limit, not a triumph; **FLOOR** = empirical, cannot be untangled.

| # | Inherited object (≈1900–1935) | Class | FTD rebuild — and what it recovers | Status | Anchor |
|---|---|---|---|---|---|
| 1 | **The continuum** (smooth space, ℝ as substrate) | convenience (calculus/QFT need it) | discrete lattice (P1); continuum physics is the **coarse-grained IR limit** (wave eq, Maxwell, Coulomb all emerge) | **REBUILT** — *the central one* | P1; `AUDIT_INFINITY_REFRAME.md` |
| 2 | **Completed infinity** (ℤ³ totality, `L→∞`) | convenience (real analysis) | undefined-boundary lattice; "L→∞" claims restated as ε-L; arbitrarily-large *finite* only | **REBUILT** | `AUDIT_INFINITY_REFRAME.md` |
| 3 | **Renormalization / UV divergence** | convenience (forced *by* the continuum) | discreteness **is** a physical UV cutoff — no completed infinity ⇒ no UV infinity; the Wilsonian IR EFT is recovered | **REBUILT** (structural) | EFT program; P1 |
| 4 | **The complex amplitude / `i`** | chosen-to-fit | the `i`-act is the **native** `√(−1)` (FC-0); the reconstruction program shows complex (not real) Hilbert space is *forced-given-operational-axioms* | **PARTIAL** — `i` native; the *inner product as observable structure* is M (row 7) | FC-0; Hardy 2001 / CDP 2011 |
| 5 | **The Born rule** (`P=|ψ|²`) | postulate (Born, 1926) | an **aggregate frequency readout**, the weak-field tangent of a Rice threshold-crossing law (`δ = O(ρ²)`); Born is the low-SNR limit | **REBUILT as aggregate** — PL-1 bet: Rice fundamental, Born approximate | FTD-0200; PL-1; the Rice-tangent derivation |
| 6 | **Probability as primitive of nature** | postulate | frequency over **definite ontic events** (the ensemble interpretation); every single-system statistic is an aggregate of definite manifestations | **REBUILT as aggregate** | the ensemble reading; FTD-0200 |
| 7 | **Quantum non-commutativity** (`[A,B]≠0`) | inherited (matrix mechanics) | the **native spin-cover Clifford algebra** (`2O ⊃ Q8` on `ℤ[i]²`, the spin lift of the cubic point group `O_h`) — assembles to `2√2`; but the *observable* algebra `A₅` is **commutative** | **PARTIAL** — native in the *symmetry* layer; promoting it to *observables* = M (declined) | FTD-0243; the spin-cover computation; Program F (FTD-0073/0086–0088) |
| 8 | **The measurement postulate / collapse** | pragmatic add-on (von Neumann) | **FC-1 declines M**; manifestation is the native irreversible event; "collapse" is the aggregate of definite events | **DECLINED** — the one import FTD refuses | FC-1 (FTD-0255) |
| 9 | **The observer / consciousness as collapse-cause** | Copenhagen confusion | FC-1 declines the observer-selection field (M-shaped, `[CLOSED NEGATIVE]`); the observer is a **reference-frame structure**, not a cause | **DECLINED** | FTD-0205/0226; reference-frame vocabulary |
| 10 | **Time's arrow as a boundary condition** (Past Hypothesis on time-symmetric microlaws) | inherited | the arrow is **native** (FC-2): the Euler-reflection *ratio* = the half-derivative `∂_t^{1/2}`; the microlaw is *not* reversible; the thermodynamic arrow is direction-is-causal + a gradient | **REBUILT** (native arrow; global reversibility declined) | FC-2 (FTD-0256); FTD-0323/0324; FTD-0253 |
| 11 | **The field + locality** | inherited | discrete flux ⊥ state fields; Moore-local causality (P4); **classical electromagnetism is `[DERIVED]`** (retarded radiation, Bianchi, boosted Coulomb, Larmor) | **REBUILT** (classical EM) | P4; FTD-0113–0120; Phase-G |
| 12 | **Fundamental spacetime / the Lorentzian metric** | inherited (Minkowski/GR) | space ⊥ time fundamental; the **metric is emergent-IR** (FC-2); causal cone `c=1/√3` `[THEOREM]`; SR/weak-field GR recovered | **PARTIAL** — metric emergent; **full GR + the spin-2 graviton OPEN** | FC-2; FTD-0253; gravity sector (FTD-0189) |
| 13 | **The gauge groups** (`U(1)×SU(2)×SU(3)` imposed) | inherited (observed) | the Moore Layer Theorem geometric grounding (octahedron / cuboctahedron / stella octangula); `N_c=3` from topology | **PARTIAL** — `[SELECTION]`-grade grounding, not forced | Moore Layer Theorem; `DERIV_NC_FROM_TOPOLOGY.md` |
| 14 | **The constants** (`α`, masses as parameters) | inherited (the SM's free parameters) | `α` is **dynamical, not structural** — `x₊=1/α` `[SMC]`, adopted via FC-W, not derived; masses are calibration | **BOUNDARY** — the honest limit, *not* a triumph | MC-T4.3; FTD-0013/0315; `CATALOG_PARAMETRIC_INSERTIONS.md` |
| 15 | **The joint-Bell correlation** (measurement-independence) | **empirical** (the violation is data) | the *one* irreducible thing: a local definite-event substrate matches the Bell-violating *joint* aggregate **only** with a measurement-dependence (context) constraint — FTD **declares** it, does not derive it | **FLOOR / COMMITMENT** — PL-2, the highest-risk open burden | PL-2; the Bell analysis |

---

## 5 · The empirical floor — what cannot be untangled

R3 made concrete. Logic untangles the *scaffolding* of 20th-century physics; it does not touch the *data*. The hard residue, after every convenience-postulate is rebuilt, is **not the Born rule** (an aggregate, row 5) and **not non-commutativity** (a native shadow, row 7) — it is the single empirical fact that the **measured joint two-party correlations violate the Bell inequality**, and a *local, definite-event* substrate reproduces that joint aggregate **only** if the ensemble's measure is context-dependent (measurement-dependence / the superdeterminism horn). That constraint (row 15) is:

- not a logical necessity (the fork is genuinely open — FTD-0243 independence),
- not free (it carries the fine-tuning bill, Wood–Spekkens),
- therefore a **declared commitment**, priced honestly, not a derivation.

This is the line where the program stops being reconstruction and becomes a *choice*. Naming it precisely — *the constants and the joint-Bell context-constraint are the only two things FTD adopts rather than derives* — is itself a deliverable of the Number-One Goal's second clause (map what discreteness does **not** reach).

---

## 6 · The honest scorecard

- **Genuinely rebuilt** (the old math recovered as a limit/theorem): the continuum, completed infinity, renormalization, the Born rule *as aggregate*, fundamental probability, the arrow of time, classical electromagnetism. **Seven of the deepest inherited assumptions are dissolved into a discrete logical core.**
- **Partially rebuilt** (one layer recovered, another open/imported): the complex `i` (native) vs the inner-product-as-observable (M); non-commutativity (native in symmetry) vs as-observable (M); the emergent metric vs full GR/graviton; the gauge groups (`[SELECTION]` grounding).
- **Declined by design** (FC-1/FC-2 refuse the import, and *that refusal buys the falsifiable deviation spine*): the measurement-map M, the observer-as-cause, global reversibility.
- **Boundary, not triumph** (the honest limits): the constants (`α` `[SMC]`, masses `[PARAMETRIC]`).
- **Empirical floor** (the one declared commitment): the joint-Bell measurement-dependence (PL-2).

**Zero promotions.** This document re-states each claim at its canonical tag. `x₊=1/α` stays `[SMC]`; MC-T4.3 stays a `[FOUNDATIONAL OBSTRUCTION]`; FC-1/FC-2/FC-W stay declarations; no α is derived; no tier is upgraded. The audit's content is the *honest accounting itself* — the reframe from "a theory that derives the constants" (its weakest, most-attacked claim) to "the realist reconstruction of physics from a discrete logical core, with its two adopted commitments and its boundaries named."

---

## 7 · Cross-references

- `SPEC_FTD_FRAMEWORK_V1.md` (FTD-0254) — the constitution / the FC register / the deviation criteria.
- `AUDIT_INFINITY_REFRAME.md` — the continuum / completed-infinity rebuild (rows 1–3).
- `SPEC_PREDICTION_LEDGER_DEVIATIONS.md` — PL-1 (Born/Rice, row 5) and PL-2 (joint-Bell, rows 14/15, the floor).
- `TRACKER_ONTIC_TRUTH.md` — the bedrock tier table (what is actually `[THEOREM]`-grade beneath this audit).
- `LEDGER.md` FTD-0243 (commutativity independence), FTD-0253 (spacetime boundary), FTD-0255/0256 (FC-1/FC-2), FTD-0315/0326 (FC-W / native-ℤ/2), FTD-0322–0327 (the act-count arc), FTD-0200 (Rice).
