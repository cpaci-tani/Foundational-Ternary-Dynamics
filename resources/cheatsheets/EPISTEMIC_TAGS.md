# Epistemic Tags Cheatsheet

FTD is epistemically disciplined. Every claim wears a tag. Tags are mandatory — untagged claims are bugs.

## The tags

| Tag | Meaning | Reviewer expectation | Example |
|---|---|---|---|
| **[AXIOM]** | Structural postulate. Not derived from anything else. | Accept as model definition. Argue about its choice, not its truth. | "Space is a 3D cubic lattice." |
| **[THEOREM]** | Rigorously proven from axioms + prior theorems. | Check the proof. Demand full derivation chain. | "`x₊` is a root of `x² − 16 G*² x + 16 G*³ = 0`." |
| **[SELECTION]** | Argued from consistency, coherence, or minimality — but not uniquely proven. | Critique the argument. Ask "why not the alternative?" | "Lattice spacing `a = 2/D` is chosen as the boundary-to-bulk ratio." |
| **[CONJECTURE]** | Proposed interpretation requiring external validation. | Demand evidence. Treat as hypothesis. | "The Bell violation `S = 2√2` emerges from ternary-state QM." |
| **[IMPOSED]** | Parameter choice or calibration — an input, not an output. | Note as input. Never cite as a "derivation". | "`K_B = 0.511 MeV` is set equal to the electron mass." |
| **[EMERGENT]** | Behavior arising from dynamics; not designed in. | Verify in simulation. Watch for confirmation bias. | "Confinement strings appear at separation `x₋` in scenarios." |
| **[OPEN]** | Unresolved question. | Research opportunity. | "What is the CKM mixing matrix in FTD?" |

## Common anti-patterns (bugs)

### 1. Labeling a parametric insertion as a derivation
**Wrong:**
> "FTD derives the muon mass: `m_μ = 3 b_3 (b_3 + N_c) · m_e − N_c · m_e = 105.67 MeV`."

**Right:**
> "[THEOREM] The ratio `m_μ/m_e = 3 b_3 (b_3 + N_c) − N_c = 207` follows from {b_3, N_c}. [IMPOSED] Using `m_e = 0.511 MeV` as the scale input, `m_μ = 105.77 MeV`."

The ratio is derived; the mass scale is inserted. Keep them labelled separately.

### 2. Running numerical search and calling near-misses "predictions"
The epistemic-rigor rule in `CLAUDE.md` is explicit: **do not run numerical search scripts looking for near-misses or coincidences**. Finding `e^π − π ≈ 20` by grep is not a derivation; identifying it as `b_3 + N_eff` from the Moore decomposition is.

### 3. Substitution identities
Plugging FTD values into a standard formula and presenting the result as "FTD derives X" is a parametric insertion, not a derivation. Tag it `[IMPOSED]` + the standard formula's source.

### 4. Missing provenance on `[OPEN]`
"[OPEN] the CKM matrix" is a placeholder. "[OPEN] the CKM matrix — hypothesized route via BCC multiplicative structure, see `docs/theory/08_structural/DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md §4`" is useful.

## Tag downgrade rules

If a claim's status weakens, tags change:

- A [THEOREM] that turns out to rely on an unproven lemma → demote to [SELECTION] or [CONJECTURE] until the lemma is proven.
- An [EMERGENT] claim that turns out to depend on a tuned parameter → re-tag the parameter [IMPOSED] and re-test the claim.

## Tag upgrade rules

- [CONJECTURE] + experimental confirmation ≠ [THEOREM]. It's [SELECTION] or [EMPIRICAL-VALIDATED] depending on whether the derivation chain is closed.
- [SELECTION] → [THEOREM] only when the argument becomes a formal proof.

## Where to see tags in action

- Every derivation doc in `docs/theory/03_derivations/` opens with an epistemic summary.
- `docs/theory/07_assessment/AUDIT_EPISTEMIC_AUDIT.md` is the project-wide ledger of tag decisions.
- `docs/theory/07_assessment/core_ledgers/TRACKER_OPEN_ITEMS.md` aggregates every `[OPEN]` in one place — engine code stubs, theory gaps, and roadmap items — so contributors can pick work without grepping the whole repo.
- The `epistemic-auditor` agent scans for untagged claims and mislabeled derivations.

## Quick test

Before writing "FTD predicts X":

1. Can I point to the axioms X follows from?
2. Is the derivation chain closed (no [OPEN] links)?
3. Are all inserted values tagged [IMPOSED]?

If any answer is no, the claim is not a prediction — it's a [CONJECTURE] or [SELECTION]. Tag honestly.
