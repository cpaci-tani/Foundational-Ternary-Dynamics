# CANONICAL REFRAME: Undefined Boundaries in Place of Completed Infinities

**Version:** 1.0
**Date:** April 2026
**Status:** Frozen for the current deployment. Changes require explicit change-log entry and a new version number.

---

## The Foundational Commitment

FTD rejects completed infinity as an ontological or structural object. What the framework permits instead is the undefined boundary: at any specified position, the substrate exists; at no specified position does the claim "this is the edge" carry meaning. The substrate has no known endpoint; it is not claimed to be complete.

This commitment has both philosophical and operational consequences. Philosophically, it aligns with a constructivist stance in mathematics and with analytical idealism's treatment of the unity of reference frame context without completed objectivity. Operationally, it rules out a class of reasoning moves that are standard in classical analysis and continuum physics but that the framework does not accept.

---

## The Distinction, Precisely

**Completed infinity** is an object of definite (though infinite) extent. The entire natural numbers as a set. The whole real line. The full limit "at infinity." The thermodynamic limit as a completed state. The continuum limit as an achieved object. Classical mathematics uses these objects constantly, often without acknowledging their ontological status.

**Undefined boundary** is a local property. At any specified position, the substrate has neighbors; no specified position is the boundary; no claim is made about "all" positions as a totalized set. The lattice extends where it is specified to exist and has no commitment beyond what has been specified.

The two are not equivalent. A completed infinity supports global operations (integrals over all space, averages over all time, completeness proofs that invoke the full set). An undefined boundary supports only local operations and operations on arbitrarily large but finite specified regions. Anything requiring the totalized set is proscribed.

---

## Proscribed Moves

The following reasoning patterns are not permitted in FTD work. Every occurrence in the portfolio is a candidate for action.

1. **L → ∞ limits as proof technique.** If a derivation relies on taking a size parameter to infinity, the derivation is not valid under the reframe.

2. **Continuum limits as completed objects.** Taking lattice spacing to zero as a completed limit, rather than treating arbitrarily fine finite spacings.

3. **Thermodynamic limits.** Taking particle number to infinity as a completed limit; invoking "the" equilibrium distribution as a totalized object.

4. **Path integrals and functional integrals over "all configurations."** These assume the configuration space is a completed totality.

5. **"The" Hilbert space, "the" state space of the universe.** These refer to completed-totality objects.

6. **RG flow to asymptotic fixed points.** Running couplings to "the UV" or "the IR" as completed limits.

7. **Claims that the lattice is infinite.** The lattice has no defined boundary; it is not infinite as a completed object.

8. **Ergodic averages over infinite time.** Time averages over "all time" treat time as a completed totality.

9. **Completeness axioms that invoke a totalized set.** Lebesgue measure on completed real line; sigma-additivity over countable totalities.

10. **Limit definitions where the limit is taken as an object rather than as a specification.** "lim_{n→∞} a_n = L" as the definition of L rather than as the specification "for any ε > 0, there exists N such that for all n > N, |a_n - L| < ε."

---

## Permitted Moves

The following reasoning patterns are permitted and should not be flagged by the classifier even when they involve unbounded structures.

1. **Arbitrarily large but finite computations.** "At L = 384 we find R = X" is a statement. "At L = 2^100 we find R = Y" is a statement. The claims are bracketed by the specific L used.

2. **Algebraic objects defined by closed-form expressions.** G* = Γ(1/4)/Γ(3/4) is algebraic. It can be evaluated to any specified finite precision. Nothing in its specification requires completed infinity.

3. **Properties that hold at every specified scale.** "For every L, the property P(L) holds" is a permitted universal statement. It does not treat the set of all L as a completed totality; it states that for any specified L, P(L) is the case.

4. **Finite precision claims with explicit bracketing.** "To 50-digit precision, the identity holds" is permitted. The statement is bracketed by the specific precision.

5. **Local differential equations and update rules.** A rule that specifies how the state at a point evolves based on its Moore-26 neighborhood is permitted. It is local and does not invoke global structure.

6. **Wallis-style products where the claim is "for any precision, a large enough finite product matches."** These products are constructively fine because they specify how to achieve any given precision with a finite product.

7. **Chowla-Selberg and similar algebraic identities among Gamma values.** These are identities about specific algebraic objects, not claims about limits.

8. **Cardinal or ordinal language restricted to finite cases, or with explicit "arbitrarily large" rather than "infinite" framing.**

9. **Per-site or per-region properties that are true "everywhere specified."** Distinct from "true everywhere" (which invokes completed totality); "true at every specified point" is local and permitted.

10. **Probability or measure concepts defined on finite samples with arbitrary extension.** Probability of an event in a specified finite region is permitted. Probability on a completed measure space over all of R is not.

---

## Distinguishing Proscribed from Permitted When Subtle

Some passages are easy. "Take the thermodynamic limit" is proscribed. "At arbitrarily large L" is permitted.

Some are subtle. Here is the decision procedure.

**Question 1: Is the claim stating a property that holds at every specified instance, or a property of a totalized object?**

- Every specified instance: permitted.
- Totalized object: proscribed.

**Question 2: Is the limiting operation being used to define a value, or to characterize behavior?**

- Define a value (e.g., "α is defined as the L→∞ limit of R(L)"): proscribed. The value is not well-defined under the reframe.
- Characterize behavior (e.g., "for any precision, a large enough L achieves that precision"): permitted.

**Question 3: If the infinity in question is replaced by "arbitrarily large" (for spatial/temporal) or "arbitrarily precise" (for numerical), does the claim still make sense?**

- Yes: the original framing was a shorthand; the claim is permitted in the revised form.
- No: the claim requires completed infinity; proscribed.

**Question 4: Does the proof structure pass through a completed-infinity step?**

- Yes: the proof needs re-derivation, even if the final claim is permissible.
- No: the claim is supported by a finitary proof; permitted.

Use all four questions together, not just one. A claim can pass Questions 1-3 but fail Question 4 if its proof relies on a lemma that invokes completed infinity.

---

## The Four Tags

Every claim in the updated portfolio receives one of four tags. The tag reflects the epistemic status of the claim after the reframe is applied.

**THEOREM**: formally proven from stated axioms, no free parameters, proof trace complete, no completed-infinity steps in the proof.

**SELECTION PRINCIPLE**: uniquely derivable given stated constraints, proof pending. The selection argument is clear but the formal proof is not yet complete.

**HYPOTHESIS**: fits empirical data with known degrees of freedom. Has quantitative predictions and stated residuals.

**CONJECTURE**: well-motivated but not derivable from current axioms. Structural plausibility is argued but not proven.

Under the reframe, a claim that was previously THEOREM may demote if its proof passed through a completed-infinity step. Promotion up the ladder always requires explicit justification; demotion is free.

---

## The Four Triage Actions

Every flagged passage in the audit receives one of four triage dispositions.

**SURVIVES**: the passage is actually permitted under the reframe (classifier flagged it conservatively). No action.

**RESTATE**: the underlying content is sound but the framing invokes completed infinity. Rewrite in finitary terms while preserving content. Example: "in the thermodynamic limit" becomes "at arbitrarily large but finite particle number."

**RE-DERIVE**: the content depends on a completed-infinity argument that cannot be simply restated. Requires a new proof. Example: a result that was proven by dominated convergence may need a direct finitary bound.

**RETRACT**: the claim does not survive the reframe. Remove from the portfolio and adjust any text that depended on it. Example: "α is defined as the continuum limit of a lattice computation" is retracted; α is not defined as a completed limit.

---

## A Worked Example

Consider the sentence: "The coupling α runs to its asymptotic value in the UV limit."

**Question 1**: Is this a property of every specified scale, or of a totalized object? The phrase "asymptotic value in the UV limit" is a totalized object (the value at completed infinity).

**Question 2**: Is the limiting operation defining a value? Yes, "its asymptotic value" is defined as the limit.

**Question 3**: Does the claim make sense if "UV limit" is replaced by "arbitrarily high scale"? Only if we also reformulate "asymptotic value" as "behavior of α(μ) as μ increases." That is a different claim.

**Question 4**: Does the proof structure pass through a completed-infinity step? Almost certainly yes, since RG flows to fixed points use limit arguments.

**Verdict**: proscribed. Triage as RESTATE. Proposed finitary form: "At arbitrarily high scale μ, the coupling α(μ) approaches behavior characterized by [specific finitary description, e.g., a specific differential equation and its bounds]." The claim is no longer about a "value at infinity"; it is about the function's behavior at arbitrarily high finite μ.

Another sentence: "G* equals the infinite Wallis product lim_{N→∞} N^{-1/2} Π_{k=0}^{N} (4k+3)/(4k+1)."

**Question 1**: Property of every instance or totalized object? The limit notation suggests a completed limit.

**Question 2**: Is the limit defining G*? In the original formulation yes; in the constructive formulation, the Wallis product is a specification of how to compute G* to any precision.

**Question 3**: Does it make sense as "for any ε > 0, there exists N such that |S_N - G*| < ε"? Yes, and that is the constructively acceptable form.

**Question 4**: Does the proof structure require completed infinity? No, the Wallis product's convergence proof can be done finitely: for any target precision, there is a specific N that suffices. The Stirling-corrected rate from the Fifty-Two Faces paper gives the explicit N for any ε.

**Verdict**: permitted in the constructive formulation. Triage as SURVIVES, with a note that the wording may benefit from revision to "G* is characterized by the Wallis-type approximation: for any precision ε, there exists N such that S_N approximates G* within ε."

---

## Ontological Consequences (Informative, Not Operational)

The reframe has consequences for the Tier ontology that are worth noting for coherence of the framework as a whole.

**Tier 1 (phase-preserved substrate).** Not "the full lattice as a completed object." Instead, a region of the lattice with no defined boundary; at every specified point, the substrate extends locally.

**Tier 2 (phase-averaged observables).** Averaging is constrained: over any specified finite region, averaging is well-defined; "averaging over all space" is not, because that requires a completed totality.

**Tier 3 (self-referential phase structure).** Finite but unbounded self-reference, not infinite self-reference. This matches the phenomenology of reference frame context (finite in any moment, unbounded in principle).

These ontological consequences are downstream of the mathematical reframe. Papers discussing the ontology should align with them. Papers doing pure mathematics need not invoke them.

---

## What This Document Does Not Decide

Several things are left to the paper-by-paper triage:

- Whether a specific flagged passage is genuinely proscribed (that is what classification and triage do).
- How to restate a specific passage in finitary form (the Restatement agent proposes; the user reviews).
- Whether a given lemma can be re-derived finitely (the Re-derivation agent attempts; sometimes the answer is "cannot be re-derived, the claim must be demoted or retracted").
- Whether the parameter-free claim for the engine is true (the Engine Audit agent investigates).
- What replaces "alpha as derived to 24 digits" in the portfolio's self-description.

These are project-specific judgments. The canonical document establishes what the reframe is; the triage establishes how it applies.

---

## Invariants

Three invariants hold for the entire deployment. Every agent and every human decision must respect them.

1. **The canonical reframe does not change during the deployment.** If it needs to change, the deployment stops, the new version is committed, and the deployment restarts with the new version.

2. **The ledger is the single source of truth for claim status.** Tags in papers are derived from the ledger; if they disagree, the ledger wins.

3. **Every agent invocation is stateless and begins by reading this document.** No agent session accumulates context across artifacts.

These invariants are the architecture. Departing from them is the drift failure mode the whole deployment is designed to prevent.

---

## End of Canonical Document

Version stamp: 1.0 / April 2026. Freeze this document before Phase 1. Do not edit during deployment except in the event of a full deployment restart.
