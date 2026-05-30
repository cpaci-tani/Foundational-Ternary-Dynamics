# Analysis Report: Foundational Ternary Dynamics (FTD) Theory Directory

## Observation
- **`proof_master_verification.py` Tests:**
  - Passes 54/54 tests. However, inspecting the source code reveals that sections like "Nuclear Binding (5 Weizsacker coefficients)" and "GR Recovery (Mercury Perihelion)" use standard physics formulas (e.g., $prec = 6\pi GM/pc^2$, Weizsäcker coefficients utilizing $197.3$ for $\hbar c$ and $0.511$ for $m_e$) with FTD parameters inserted.
- **Tag Violations (Parametric Insertions labeled as [THEOREM]):**
  - **`DERIV_HIGGS_FROM_MANIFESTATION.md`**: Section 6.3 labels $M_W = gv/2$ and $M_Z = M_W/\cos\theta_W$ as `[THEOREM]`.
  - **`DERIV_LATTICE_SU2_WEAK.md`**: Section 6.2 labels $G_F = 1/(\sqrt{2}v^2)$ as `[THEOREM]`. Section 7 tables upgrade leptonic, semileptonic, and baryonic decay rates to `[THEOREM]` while explicitly using standard Fermi decay formulas (e.g., $\tau_\mu = 192\pi^3 / (G_F^2 m_\mu^5)$).
- **Epistemic Inconsistencies:**
  - **`SPEC_FTD_LAGRANGIAN.md`**: Section 3.4 tags $K_B = M_P\sqrt{2\pi}(16/3)\alpha^{11} = 0.511$ MeV as `[THEOREM]`. However, `LEDGER.md` (FTD-0015) and `DERIV_NEWTON_FROM_SUBSTRATE.md` rigorously tag this same relationship as a `[STRONGLY MOTIVATED CONJECTURE]`.
  - **`DERIV_EINSTEIN_FIELD_EQUATIONS.md`**: EFE-7 lists $G = \alpha_G \hbar c/m_e^2$ via the $\alpha^{20}$ hierarchy as `[THEOREM]`. But `DERIV_NEWTON_FROM_SUBSTRATE.md` corrects this to an $\alpha^{22}$ hierarchy and identifies the epistemic floor as `[STRONGLY MOTIVATED CONJECTURE]`.
  - **`DERIV_HIGGS_FROM_MANIFESTATION.md`**: In the Claims Table, HIGGS-8 identifies $\lambda = 3/23$ as `[SELECTION]`, but HIGGS-16 identifies the exact same identity $\lambda = N_C/(N_C^3 - N_{BASE}) = 3/23$ as `[THEOREM]`. Section 5A.3 also treats it as `[THEOREM]`.

## Logic Chain
1. The project rules explicitly forbid labeling parametric insertions as derivations: "if standard physics provides the formula and FTD provides the numbers, that is a parametric insertion, not a derivation." 
2. The derivations for $M_W$, $G_F$, and decay rates in the `SU2_WEAK` and `HIGGS` files use standard QFT formulas combined with FTD constants. By definition, these are parametric insertions and should not be tagged `[THEOREM]`.
3. The `proof_master_verification.py` script validates these parametric insertions. The tests "pass" because standard physics equations correctly reproduce physical reality when fed accurate numerical constants. This does not validate the FTD framework's underlying geometry; it merely validates that the numeric inputs are close to experimental values. 
4. The ledger rules stipulate that `LEDGER.md` is the single source of truth. The electron mass ($K_B$) formula and the gravitational coupling ($\alpha_G$) hierarchy are `[STRONGLY MOTIVATED CONJECTURE]`. Their elevation to `[THEOREM]` in the core Lagrangian spec and EFE derivation represents documentation drift and epistemic inflation.

## Caveats
- The verification script `proof_master_verification.py` does technically pass, which means the numerical framework integers (like $N_c = 3$, $b_3 = 7$) do produce values that map closely to experimental constants. The gaps identified here are primarily epistemic and methodological (misclassification of results) rather than mathematical miscalculations.
- I have not audited every single derivation file in the `theory` directory. The ones selected (`HIGGS`, `SU2_WEAK`, `EFE`, `LAGRANGIAN`) are highly representative of the load-bearing claims.

## Conclusion
The FTD theory directory suffers from widespread epistemic inflation, specifically violating the project's core directive against labeling parametric insertions as `[THEOREM]`. Key physical observables ($K_B$, $G_F$, $M_W$) and decay rates are computed using standard physics formulas and improperly elevated to theorem status. The passing status of `proof_master_verification.py` masks this issue by verifying the arithmetic of standard physics equations rather than structural framework derivations. Furthermore, conflicting tags exist between the central `LEDGER.md` and core specification documents.

## Verification Method
1. Inspect `docs/theory/03_derivations/DERIV_LATTICE_SU2_WEAK.md` Section 7 and observe the explicit "New: **THEOREM**" tags alongside standard QFT formulas.
2. Check `docs/theory/01_reference/SPEC_FTD_LAGRANGIAN.md` Section 3.4 for the `[THEOREM]` tag applied to $K_B$, contradicting `LEDGER.md`'s FTD-0015.
3. Review the source of `scripts/proofs/proof_master_verification.py` (specifically `test_nuclear_binding` and `test_gr_recovery`) to verify the use of standard physics formulas with hardcoded constants like $197.3$ ($\hbar c$).
