import os

ledger_path = r"c:\Users\cpaci\Desktop\ftd\docs\theory\07_assessment\TRACKER_OPEN_ITEMS.md"
content_to_append = """
## §10 Epistemic Integrity Gaps

The following action items address tag inflations identified during the recent epistemic audit:

- **[OPEN] Lepton Mass Ratios:** Demote the $m_\\mu/m_e = 207$ and $m_\\tau/m_e = 3477$ formulas in `FOUND_AXIOM_ZERO.md` from `[THEOREM]` to `[PARAMETRIC INSERTION]` or `[CONJECTURE]`, as they lack a rigorous derivation from the core FTD lattice Lagrangian.
- **[OPEN] Fine-Structure Constant Precision Polynomials:** Demote the 4-term and 7-term $\\alpha$ precision polynomials (`ALPHAP-1`, `ALPHAP-1b`) in `DERIV_ALPHA_PRECISION_FORMULA.md` from `[THEOREM]` to `[SELECTION]` or `[IMPOSED]` parameter fits, since they are derived via numerical search rather than geometric proofs.
- **[OPEN] Downstream Derivations (SU(2) Weak):** Correct the summary tables in `DERIV_LATTICE_SU2_WEAK.md` that improperly upgrade 50 decay rates to `[THEOREM]†`. Ensure they are marked consistently with the text as `[PARAMETRIC INSERTION]` due to their reliance on imported functional forms.
"""

with open(ledger_path, "a", encoding="utf-8") as f:
    f.write(content_to_append)

print("Successfully appended to ledger.")
