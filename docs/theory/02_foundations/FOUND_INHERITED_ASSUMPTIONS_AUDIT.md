# FOUND — The Inherited-Assumptions Audit

**Tag:** `[SYNTHESIS]` (a method and a status map; restates existing FTD claims at their canonical tags, and introduces **no new theorem and no promotion**)
**LEDGER:** FTD-0329 · **Companion to:** the constitution (`SPEC_FTD_FRAMEWORK_V1.md`, FTD-0254)

> **Purpose.** The mathematical formalism of early-twentieth-century physics was assembled rapidly and pragmatically, using the analytic tools available between roughly 1900 and 1935, and an interpretive layer (operationalism, the Copenhagen account) was adopted alongside it because the formalism predicted well, independently of any settled account of *why*. A number of structures conventionally treated as foundational are therefore better understood as representational choices fixed by the mathematics of that period than as consequences of a deeper principle — a point developed in the modern reconstruction programme (Hardy 2001; Chiribella–D'Ariano–Perinotti 2011; Masanes–Müller 2011). This document applies that lens systematically within FTD. It audits the load-bearing inherited objects of physics, classifies each, and records — at canonical tags — where FTD's discrete logical core re-derives the object, where it declines the object by design, and where it reaches a boundary. The audit is as much a map of what FTD does **not** reach as of what it does.

---

## 0 · Thesis and precise claim

The claim is **not** that the established mathematics is incorrect, and **not** that physics should be rebuilt on informal intuition. It is the following:

> For each load-bearing object, determine whether it is a **logical or ontological primitive**, a **derived theorem**, or a **postulate of convenience** retained because it was the available mathematics — and, where it is a postulate of convenience, identify the cleaner primitive that re-derives it as an aggregate, emergent, or symmetry-level consequence, **while recovering every empirical prediction of the original**.

This is the constitution's ordering — **Ontology > Logic > Math > Physics** — made operational. The formalism is correct and predictive but ontologically agnostic: it does not specify what exists. The reconstruction supplies that ontology and requires the established formalism to re-emerge as the aggregate description.

---

## 1 · Method: the three-question audit

For each inherited object, ask:

1. **What is it?** A primitive (an irreducible commitment), a theorem (forced by the primitives), or a convenience postulate (fitted rather than forced)?
2. **If convenience: necessary or contingent?** Was it retained because it is logically forced, or because it was the mathematics available at the time of formulation?
3. **If contingent: what re-derives it?** Which cleaner primitive yields it as an aggregate, an emergent infrared limit, or a symmetry sub-structure — while reproducing the data?

---

## 2 · Admissibility criteria for a reconstruction

A reconstruction of this kind is constrained by five criteria, each enforced by existing FTD machinery (the LEDGER, the epistemic tags, the deviation ledger, and pre-registration):

- **R1 — Recover the established mathematics as a limit or theorem; do not discard it.** A reconstruction that fails to reproduce a confirmed prediction is falsified by that failure; the prediction stands. (Born must re-emerge as the aggregate; non-commutativity must re-emerge as the spin-cover structure.)
- **R2 — Be more constrained, not less.** "Logic, not only mathematics" is a demand for fewer and sharper primitives that force the same structure, not a licence to introduce additional freedom.
- **R3 — Empirical results are not subject to revision.** The reconstruction may reorganise the formalism; it may not reinterpret away data. Measured Bell violations are recovered, not explained away.
- **R4 — Primitives must be explicit, minimal, and named.** A reconstruction is determined by its declared commitments; appeals to unstated intuition are not admissible.
- **R5 — Tag honestly.** Every entry below carries its canonical tag. A `[SELECTION]` result is not written as `[DERIVED]`; a declined import is not presented as a derivation.

---

## 3 · FTD's declared primitives

Per R4, the bedrock the reconstruction rests on is stated explicitly:

- **P1–P5** — discrete finite (undefined-boundary) lattice; discrete time; ternary state `{−1, 0, +1}`; local (26-neighbour Moore) causality; determinism.
- **The first distinction** — the ternary set `{−1, 0, +1} = {i², 0, |i²|}`, the real projection of `ℤ[i]^× ∪ {0}` (FTD-0128).
- **FC-0** — the order-4 planar symmetry read as `ℤ[i]`; the `i`-act, `√(−1)`, as the single generative act (FTD-0322).
- **The Framework Commitments** — FC-1 (declines the measurement map M), FC-2 (native arrow; emergent metric), FC-3 (scale-covariance), and FC-W = FC-4 (the adopted α-binding law). Each is an `[AXIOM]`-class **declaration**, not a derivation.

Every entry in §4 is re-derived from these primitives, recovered as the established mathematics, or marked explicitly as still imported or as a boundary.

---

## 4 · The audit

Rebuild-status legend: **REBUILT** = a canonical `[THEOREM]`/`[DERIVED]` recovers it; **PARTIAL** = recovered in one layer, open or imported in another; **DECLINED** = FC-1/FC-2 declines the import by design; **BOUNDARY** = the honest limit of what discreteness fixes; **FLOOR** = empirical, not subject to reconstruction.

| # | Inherited object (≈1900–1935) | Class | FTD treatment — and what it recovers | Status | Anchor |
|---|---|---|---|---|---|
| 1 | **The continuum** (smooth space, ℝ as substrate) | convenience (calculus/QFT require it) | discrete lattice (P1); continuum physics is the **coarse-grained IR limit** (the wave equation, Maxwell, and Coulomb all emerge) | **REBUILT** — the most consequential entry | P1; `AUDIT_INFINITY_REFRAME.md` |
| 2 | **Completed infinity** (ℤ³ totality, `L→∞`) | convenience (real analysis) | undefined-boundary lattice; `L→∞` claims restated in ε–L form; arbitrarily large but **finite** only | **REBUILT** | `AUDIT_INFINITY_REFRAME.md` |
| 3 | **Renormalisation / UV divergence** | convenience (forced *by* the continuum) | discreteness **is** a physical UV cutoff — no completed infinity, hence no UV infinity; the Wilsonian IR EFT is recovered | **REBUILT** (structural) | EFT program; P1 |
| 4 | **The complex amplitude / `i`** | chosen to fit | the `i`-act is the **native** `√(−1)` (FC-0); the reconstruction programme shows complex (not real) Hilbert space is forced given the operational axioms | **PARTIAL** — `i` is native; the *inner product as observable structure* is M (row 7) | FC-0; Hardy 2001 / CDP 2011 |
| 5 | **The Born rule** (`P=|ψ|²`) | postulate (Born, 1926) | an **aggregate frequency readout** — the weak-field tangent of a Rice threshold-crossing law (`δ = O(ρ²)`); Born is the low-SNR limit | **REBUILT as aggregate** — PL-1: Rice fundamental, Born approximate | FTD-0200; PL-1; the Rice-tangent derivation |
| 6 | **Probability as a primitive of nature** | postulate | frequency over **definite ontic events** (the ensemble interpretation); each single-system statistic is an aggregate of definite manifestations | **REBUILT as aggregate** | the ensemble reading; FTD-0200 |
| 7 | **Quantum non-commutativity** (`[A,B]≠0`) | inherited (matrix mechanics) | the **native spin-cover Clifford algebra** (`2O ⊃ Q8` on `ℤ[i]²`, the spin lift of the cubic point group `O_h`) assembles to `2√2`; the *observable* algebra `A₅`, however, is **commutative** | **PARTIAL** — native in the symmetry layer; promoting it to observables is M (declined) | FTD-0243; the spin-cover computation; Program F (FTD-0073/0086–0088) |
| 8 | **The measurement postulate / collapse** | pragmatic addition (von Neumann) | **FC-1 declines M**; manifestation is the native irreversible event; "collapse" is the aggregate of definite events | **DECLINED** — the one import FTD declines | FC-1 (FTD-0255) |
| 9 | **The observer / consciousness as cause of collapse** | a conflation in the Copenhagen account | FC-1 declines the observer-selection field (M-shaped, `[CLOSED NEGATIVE]`); the observer is a **reference-frame structure**, not a cause | **DECLINED** | FTD-0205/0226; reference-frame vocabulary |
| 10 | **The arrow of time as a boundary condition** (Past Hypothesis on time-symmetric microlaws) | inherited | the arrow is **native** (FC-2): the Euler-reflection *ratio* equals the half-derivative `∂_t^{1/2}`; the microlaw is not reversible; the thermodynamic arrow follows from a causal direction plus a gradient | **REBUILT** (native arrow; global reversibility declined) | FC-2 (FTD-0256); FTD-0323/0324; FTD-0253 |
| 11 | **The field and locality** | inherited | discrete flux ⊥ state fields; Moore-local causality (P4); exact free-wave pole/work are derived; FTD-0560–0563 close every fixed finite rigid linear dressing and show finite neutrality supplies no true linear Gauss monopole; FTD-0564 proves orientation topology alone cannot set electric-charge magnitude, while a protected defect plus nonlinear common action, recoil, and physical power remain open | **PARTIAL** — field kinematics, finite-source obstruction, scoped Gauss dichotomy, and topology/magnitude separation rebuilt; reciprocal nonlinear/topological source electrodynamics open | P4; FTD-0113–0120; FTD-0558–0564; Phase-G |
| 12 | **Fundamental spacetime / the Lorentzian metric** | inherited (Minkowski / GR) | space ⊥ time fundamental; the **metric is emergent-IR** (FC-2); causal cone `c = 1/√3` `[THEOREM]`; SR and weak-field GR recovered | **PARTIAL** — metric emergent; **full GR and the spin-2 graviton are OPEN** | FC-2; FTD-0253; gravity sector (FTD-0189) |
| 13 | **The gauge groups** (`U(1)×SU(2)×SU(3)`, imposed) | inherited (observed) | the Moore Layer Theorem's geometric grounding (octahedron / cuboctahedron / stella octangula); `N_c = 3` from topology | **PARTIAL** — `[SELECTION]`-grade grounding, not forced | Moore Layer Theorem; `DERIV_NC_FROM_TOPOLOGY.md` |
| 14 | **The constants** (`α`, masses as parameters) | inherited (the SM's free parameters) | `α` is **dynamical, not structural** — `x₊ = 1/α` `[SMC]`, adopted via FC-W, not derived; masses are calibration | **BOUNDARY** — the honest limit | MC-T4.3; FTD-0013/0315; `CATALOG_PARAMETRIC_INSERTIONS.md` |
| 15 | **The joint-Bell correlation** (measurement-independence) | **empirical** (the violation is data) | the single irreducible item: a local definite-event substrate reproduces the Bell-violating *joint* aggregate **only** under a measurement-dependence (context) constraint, which FTD **declares** rather than derives | **FLOOR / COMMITMENT** — PL-2, the highest-risk open burden | PL-2; the Bell analysis |

---

## 5 · The empirical constraint

Per R3, a reconstruction reorganises the formalism but does not touch the data. After every convenience postulate has been re-derived, the residue is **not the Born rule** (an aggregate, row 5) and **not non-commutativity** (a symmetry-level structure, row 7). It is the single empirical fact that the **measured joint two-party correlations violate the Bell inequality**, together with the consequence that a *local, definite-event* substrate reproduces that joint aggregate **only** if the ensemble's measure is context-dependent (the measurement-dependence horn). That constraint (row 15) is:

- not a logical necessity — the relevant fork is genuinely open (FTD-0243, independence);
- not free — it carries a fine-tuning cost (Wood–Spekkens);
- and therefore a **declared commitment**, priced explicitly, not a derivation.

This is the point at which the programme ceases to be reconstruction and becomes a choice. Stating it precisely — that the constants and the joint-Bell context constraint are the only two items FTD adopts rather than derives — is itself a deliverable of the Number-One Goal's second clause: to map what discreteness does **not** reach.

---

## 6 · Summary assessment

- **Re-derived** (the established mathematics recovered as a limit or theorem): the continuum, completed infinity, renormalisation, the Born rule *as aggregate*, fundamental probability, the arrow of time, and classical electromagnetism — seven of the deepest inherited assumptions, recovered from a discrete logical core.
- **Partially re-derived** (one layer recovered, another open or imported): the complex `i` (native) versus the inner-product-as-observable (M); non-commutativity (native in symmetry) versus as-observable (M); the emergent metric versus full GR and the graviton; the gauge groups (`[SELECTION]`-grade grounding).
- **Declined by design** (FC-1/FC-2 declines the import, which is what buys the falsifiable deviation spine): the measurement map M, the observer-as-cause, and global reversibility.
- **Boundary** (the honest limits): the constants (`α` `[SMC]`, masses `[PARAMETRIC]`).
- **Empirical floor** (the one declared commitment): the joint-Bell measurement-dependence (PL-2).

**Zero promotions.** This document restates each claim at its canonical tag. `x₊ = 1/α` remains `[SMC]`; MC-T4.3 remains a `[FOUNDATIONAL OBSTRUCTION]`; FC-1/FC-2/FC-W remain declarations; no α is derived; no tier is upgraded. The contribution is the accounting itself: it frames FTD as a discrete-ontology reconstruction of physics in which the two adopted commitments and the boundaries are named explicitly, rather than as a programme that claims to derive the constants.

---

## 7 · Cross-references

- `SPEC_FTD_FRAMEWORK_V1.md` (FTD-0254) — the constitution, the FC register, and the deviation criteria.
- `AUDIT_INFINITY_REFRAME.md` — the continuum and completed-infinity rebuild (rows 1–3).
- `SPEC_PREDICTION_LEDGER_DEVIATIONS.md` — PL-1 (Born/Rice, row 5) and PL-2 (joint-Bell, rows 14–15, the floor).
- `TRACKER_ONTIC_TRUTH.md` — the bedrock tier table (what is `[THEOREM]`-grade beneath this audit).
- `LEDGER.md` — FTD-0243 (commutativity independence), FTD-0253 (spacetime boundary), FTD-0255/0256 (FC-1/FC-2), FTD-0315/0326 (FC-W / native ℤ/2), FTD-0322–0327 (the act-count arc), FTD-0200 (Rice).
