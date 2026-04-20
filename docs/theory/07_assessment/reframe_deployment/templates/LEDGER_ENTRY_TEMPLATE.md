# Ledger Entry Template

The master ledger contains one entry per load-bearing claim in the FTD portfolio. Use YAML format for machine-readability; Markdown is acceptable if YAML is not convenient.

## YAML format (recommended)

```yaml
- claim_id: FTD-0001
  short_name: "Master Quadratic Algebraic Identity"
  statement: >
    The polynomial x^2 - 16 G*^2 x + 16 G*^3 = 0 has exactly two real roots:
    x_+ ≈ 137.036171458... and x_- ≈ 3.023963916...; the identity is verified
    to 50-digit precision.
  tag: THEOREM
  tag_history:
    - date: "2026-04-01"
      tag: "CONJECTURE"
      reason: "Initial proposal"
    - date: "2026-04-15"
      tag: "SELECTION PRINCIPLE"
      reason: "Numerical verification at 50-digit precision"
    - date: "2026-04-19"
      tag: "THEOREM"
      reason: "Pure algebra; no completed-infinity steps; Phase I audit verified"
  proof_status: COMPLETE
  proof_location:
    - "MATH_MASTER_QUADRATIC.md"
    - "proof_motivic_master_quadratic.py"
  dependencies:
    - FTD-0003  # G* = Gamma(1/4)/Gamma(3/4) definition
    - FTD-0012  # Coefficient 16 = |Aut(E_i)|^2 at D=3
  dependents:
    - FTD-0021  # Vieta identity 1/x_+ + 1/x_- = 1/G*
    - FTD-0022  # Koide-like identity for the pair
    - FTD-0034  # alpha_* as geometric mean
  last_reviewed: "2026-04-19"
  reframe_status: UNAFFECTED
  reframe_notes: "Pure algebra; no completed-infinity invocation in proof or statement."
  citations:
    - paper: "MATH_MASTER_QUADRATIC.md"
      location: "Section 3"
    - paper: "AUDIT_MASTER_QUADRATIC.md"
      location: "Full document"
```

## Markdown format (acceptable)

```markdown
### FTD-0001: Master Quadratic Algebraic Identity

**Statement**: The polynomial x^2 - 16 G*^2 x + 16 G*^3 = 0 has exactly two real roots: x_+ ≈ 137.036171458... and x_- ≈ 3.023963916...; the identity is verified to 50-digit precision.

**Tag**: THEOREM

**Tag history**:
- 2026-04-01: CONJECTURE (initial proposal)
- 2026-04-15: SELECTION PRINCIPLE (numerical verification at 50-digit precision)
- 2026-04-19: THEOREM (pure algebra; no completed-infinity steps)

**Proof status**: COMPLETE
**Proof location**: MATH_MASTER_QUADRATIC.md, proof_motivic_master_quadratic.py

**Dependencies**: FTD-0003, FTD-0012
**Dependents**: FTD-0021, FTD-0022, FTD-0034

**Last reviewed**: 2026-04-19
**Reframe status**: UNAFFECTED
**Reframe notes**: Pure algebra; no completed-infinity invocation in proof or statement.

**Citations**:
- MATH_MASTER_QUADRATIC.md, Section 3
- AUDIT_MASTER_QUADRATIC.md, full document
```

## Field Definitions

- **claim_id**: Stable identifier. Format: FTD-NNNN. Once assigned, never changes.
- **short_name**: Human-readable label, <60 characters.
- **statement**: The claim in its current form, self-contained.
- **tag**: One of THEOREM, SELECTION PRINCIPLE, HYPOTHESIS, CONJECTURE, RETRACTED.
- **tag_history**: Ordered list of tag transitions with dates and reasons.
- **proof_status**: COMPLETE | OUTLINED | SKETCH | OPEN | FAILED.
- **proof_location**: Paper and location where the proof lives.
- **dependencies**: claim_ids this claim depends on.
- **dependents**: claim_ids that depend on this one.
- **last_reviewed**: Date of last review.
- **reframe_status**: AFFECTED | UNAFFECTED | RESOLVED under the current reframe.
- **reframe_notes**: Brief explanation.
- **citations**: Where the claim is cited in the portfolio.

## Maintenance Rules

1. Never delete a row. Retracted claims stay in the ledger with tag RETRACTED.
2. Never change a claim_id. If renaming, change only short_name.
3. Dependencies and dependents must be symmetric: if A depends on B, B's dependents must include A.
4. Every tag change must have a corresponding tag_history entry.
5. last_reviewed must be updated whenever any field changes.
```
