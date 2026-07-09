# INDEX · Coupling Constants & Precision

**Tag:** [REFERENCE]
**Status:** [REFERENCE] — local navigation index for `docs/theory/04_coupling/`.
**Purpose:** This cluster holds the derivations that turn the algebraic spine (G\*, the master quadratic) into coupling constants and precision predictions — the fine-structure constant α and its loop corrections, Λ_QCD, the cosmological constant, the Planck-mass scale-setting relation, and the G\*-decomposition bridges (Watson identity, ϖ/√PF). Read it when you need the chain from lemniscate geometry to a numerical coupling, or when checking the epistemic status of an α-precision claim.

## Read first

1. [DERIV_GSTAR_PF_BRIDGE.md](DERIV_GSTAR_PF_BRIDGE.md) — how G\* = ϖ/√(PF) enters and exits every physics sector; the foundational decomposition.
2. [DERIV_ALPHA_LATTICE_MECHANISM.md](DERIV_ALPHA_LATTICE_MECHANISM.md) — the supporting chain ℤ³ → cuboctahedron → CM curve → G\* → α (honest: not a single theorem).
3. [DERIV_PHI3_EXACT_EFT.md](DERIV_PHI3_EXACT_EFT.md) — the master cubic potential and the algebraic EFT it generates.
4. [DERIV_WATSON_GSTAR_IDENTITY.md](DERIV_WATSON_GSTAR_IDENTITY.md) — G\*²/(2π) = Watson's BCC integral I₁; the exact identity behind the BCC selection.

## α and its precision corrections

| File | Tag | Purpose |
|---|---|---|
| [DERIV_PHI3_EXACT_EFT.md](DERIV_PHI3_EXACT_EFT.md) | [THEOREM] (algebraic EFT) + [SELECTION] (x₊ = α⁻¹) | Master cubic potential V(x) = x³/3 − 8G\*²x² + 16G\*³x; algebraic EFT from the master quadratic. |
| [DERIV_ALPHA_LATTICE_MECHANISM.md](DERIV_ALPHA_LATTICE_MECHANISM.md) | derivation chain; x₊ = 1/α is [STRONGLY MOTIVATED CONJECTURE] | Five-step supporting chain from the cubic lattice to α; physical matching principle remains conjectural. |
| [DERIV_ONE_LOOP_LATTICE_ALPHA.md](DERIV_ONE_LOOP_LATTICE_ALPHA.md) | [SELECTION] / scheme-conditional | One-loop Brillouin-zone tadpole correction; "9.6 ppb" is a Structure-1 fixed-regularization outcome, not scheme-independent. |
| [DERIV_ALPHA_PRECISION_FORMULA.md](DERIV_ALPHA_PRECISION_FORMULA.md) | [CONJECTURE] | 7-term precision series matching CODATA to 24 digits as an algebraic identity; algebraically strong, experimentally weak (untestable below digit ~11). |
| [EXPLR_A_OVER_D_AUDIT.md](EXPLR_A_OVER_D_AUDIT.md) | [EMERGENT] — audit, not derivation | Tests whether the lattice spacing a = 2/D is forced from first principles or merely close to the gap-closing value. |

## Other couplings & scales

| File | Tag | Purpose |
|---|---|---|
| [DERIV_LAMBDA_QCD_DERIVATION.md](DERIV_LAMBDA_QCD_DERIVATION.md) | [SELECTION] (loop closed, up from [PARAMETRIC]) | Non-circular derivation of Λ_QCD via dimensional transmutation; breaks the α → v → Λ_QCD circularity. |
| [DERIV_PLANCK_MASS_AND_LAMBDA_QCD.md](DERIV_PLANCK_MASS_AND_LAMBDA_QCD.md) | [THEOREM] (Λ_QCD) + [SELECTION] (M_P self-consistency) | Closes the external-input loop: M_P is axiomatic scale-setting; a single measured mass fixes it; Λ_QCD consolidated at 218 MeV. |
| [DERIV_COSMOLOGICAL_CONSTANT.md](DERIV_COSMOLOGICAL_CONSTANT.md) | [PARAMETRIC] numerology (source [OPEN], value [BOUNDARY]) | The ρ_Λ = m_e⁴·α¹⁶·G*² (α⁵⁷) form is a value-match, not a source: FTD's classical vacuum is zero-energy (FC-1) ⇒ Λ=0; honest small-Λ mechanism is the scale-covariant holographic ratio in DERIV_LAMBDA_SCALE_COVARIANT.md (FTD-0331). |

## G\*-decomposition bridges

| File | Tag | Purpose |
|---|---|---|
| [DERIV_GSTAR_PF_BRIDGE.md](DERIV_GSTAR_PF_BRIDGE.md) | three-layer algebraic reduction | G\* = ϖ/√(PF) decomposition; PF = π/4 cancels in every dimensionless observable, survives only in scale-setting quantities. |
| [DERIV_WATSON_GSTAR_IDENTITY.md](DERIV_WATSON_GSTAR_IDENTITY.md) | [THEOREM] (algebraic identity) + corrected interpretation | G\*²/(2π) = Watson's I₁, the BCC (not SC) lattice self-energy; ties G\* to the 8 corner neighbours. |
| [DERIV_DISCRETE_CONTINUOUS_BRIDGE.md](DERIV_DISCRETE_CONTINUOUS_BRIDGE.md) | [THEOREM] (identities) + [SELECTION] (interpretation) + [CONJECTURE] (Fourier self-duality) | The master quadratic as connector between lattice arithmetic and lemniscate analysis. |
| [DERIV_LEMNISCATE_HIERARCHY_WHITEPAPER.md](DERIV_LEMNISCATE_HIERARCHY_WHITEPAPER.md) | [THEOREM]/[CONTEXT] mixed (white paper) | Lemniscate Hierarchy white paper: nested algebraic curves encoding the four forces; 137-lobe curve and 2πα moiré period. |

## Research programs

| File | Tag | Purpose |
|---|---|---|
| [SCOPE_DISCRETE_FEYNMAN_PROGRAM.md](SCOPE_DISCRETE_FEYNMAN_PROGRAM.md) | [SCOPE] — promotes nothing | Discrete-Feynman-integral program: the one-loop lattice integral (return Green's function) is pinned to a CM point by the structure function — BCC→ℤ[i]/Γ(1/4)=G\*, FCC→ℤ[ω]/Γ(1/3), SC→disc −24 (a [THEOREM] per DERIV_WATSON_GSTAR_IDENTITY §7.4, confirmed by `lattice_period_map.py`). M2 (open): does the **two-loop** BCC period stay lemniscatic or climb like the continuum sunrise? Ceiling: cannot force α (MC-T4.3); a deterministic resolvent series, not a path integral; studies periods, not literal self-energies. |

---

13 active docs in this cluster (+ 1 archived: `archive/retracted/DERIV_ALPHA_READOUT_RESOLUTION.md` — retracted).
