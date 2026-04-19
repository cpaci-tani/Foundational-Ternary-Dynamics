# DERIV_<NAME> — <One-line title>

> **How to use:** copy this file to `docs/theory/03_derivations/DERIV_<NAME>.md`, rename it, fill in every section. Remove these instructional callouts before committing.

## Epistemic summary

> State the status in one line. Example: "[THEOREM] derives m_μ/m_e = 207 from {b_3, N_c}."

- **Claim:** `<what this document proves or proposes>`
- **Tag:** `[AXIOM] / [THEOREM] / [SELECTION] / [CONJECTURE] / [IMPOSED] / [EMERGENT] / [OPEN]`
- **Depends on:** `<list of prior theorems, axioms, or constants>`
- **Provides:** `<what downstream docs can cite as established>`

## Motivation

> 2-3 sentences. Why does this matter? What does it unlock? What question does it answer?

## Prerequisites

> Bullet list of upstream results the reader must know. Link each to its doc.

- `<THEOREM A>` — see `docs/theory/.../THEOREM_A.md`
- `<Definition of X>` — see `docs/reference/REF_SYMBOL_GLOSSARY.md`
- `<Constant c>` — see `scripts/constants.py`

## Setup

> Formal setup. Define the objects, state the assumptions, pin down notation.

```
<definitions>
<notation>
<assumptions>
```

## Main result

> The clean statement of what's proven.

```
Theorem (<NAME>). Given {<prerequisites>}, the following holds:

    <formal statement>
```

## Proof

> Tag each step. Cite each rule.

**Step 1.** `<description>`
> (1.1) `<algebraic step>`
> (1.2) `<algebraic step>`
> By [RULE X], (1.1) implies (1.2).

**Step 2.** `<description>`
> …

**QED** (or "modulo `<lemma>`, which is [OPEN]").

## Numerical verification

> If the theorem produces a specific number, check it against `scripts/constants.py` or a proof script.

```
From scripts/constants.py: <expression> = <value>
Experimental (PDG 2024):  <value>
Error:                    <ppm / %>
```

Point to the verification script:

```
python scripts/proofs/proof_<NAME>.py
```

Expected output: `<NAME>: PASS` (or describe what "pass" means here).

## Sanity checks

> Two or three things that must be true if this theorem is right; confirm each.

1. **Limiting case:** when `<parameter> → <limit>`, the result reduces to `<known formula>`. ✓
2. **Dimensional check:** both sides have units `<units>`. ✓
3. **Cross-theorem:** result is consistent with `<OTHER_THEOREM>`. ✓

## What this does not say

> Be explicit about scope. Head off the "this proves everything!" failure mode.

- This does **not** derive `<X>`.
- This assumes `<Y>` which is tagged `[IMPOSED]`.
- The step at `<line>` is sensitive to `<Z>` — not a universal result.

## Open questions

> Honest accounting of what's left.

- **[OPEN]** `<question>` — why it matters, what would resolve it.

## Cross-references

- Upstream: `<list theorem docs this depends on>`
- Downstream: `<what uses this>` (may be empty when first written)
- Code: `scripts/proofs/proof_<NAME>.py`, `scripts/verification/verify_<NAME>.py`
- Related experimental data: `scripts/experiments/<related>.py`
- Audit ledger: `docs/theory/07_assessment/AUDIT_EPISTEMIC_AUDIT.md` (add the new claim to the ledger)

## Changelog

- `YYYY-MM-DD` — initial draft
- `YYYY-MM-DD` — verified numerically; proof script added
- `YYYY-MM-DD` — reviewed; tag confirmed as [THEOREM] / demoted to [SELECTION] / etc.

---

## Checklist before committing

- [ ] Epistemic tag matches actual proof status (not aspirational)
- [ ] Every numeric value traces to `scripts/constants.py` or experimental reference
- [ ] Proof has no `<TBD>` / `<fill in>` placeholders
- [ ] Sanity checks run and pass
- [ ] Cross-references point to real files
- [ ] Added to `docs/theory/META_INDEX.md`
- [ ] Added entry to `CHANGELOG.md` if this is a major result
- [ ] Parametric insertions are tagged `[IMPOSED]`, not hidden inside a "derivation"
