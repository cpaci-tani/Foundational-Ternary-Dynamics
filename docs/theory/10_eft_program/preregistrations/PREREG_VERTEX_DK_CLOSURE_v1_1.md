# PREREG — Vertex Program v1.1: Corrected-Operator, Free-Scale Dirac–Kähler Evolution Re-Test

**Tag:** [PRE-REGISTRATION] — expectations committed BEFORE the measurement runs
**LEDGER id:** extends FTD-0379 (M1 v1.1; no new id — same claim, corrected instrument)
**Date locked:** 2026-07-10
**Supersedes nothing; complements:** [`PREREG_VERTEX_DK_CLOSURE_v1.md`](PREREG_VERTEX_DK_CLOSURE_v1.md) §2 (the v1 M1, which is left untouched as locked provenance) and its analysis `ANALYSIS_VERTEX_DK_CLOSURE_v1.md`.

## 0 · Why a v1.1

The post-run adversarial math review of M1 v1 established two instrument defects, both traceable to executing FTD-0089 §A1.3/§A1.5 *literally*:

1. **The §A1.3 system is not the Dirac–Kähler operator.** Its divergence convention gives δ ≠ d\* (adjointness signs alternate −,+,− across grade sectors), so D = d−δ is not skew-adjoint and D² ≠ −Δ_Hodge — the tested first-order system is provably not a Dirac-type square root of the wave operator. The v1 [CLOSED NEGATIVE] is therefore against *that variant*: grades 1 and 3 are convention-invariant (their v1 residuals stand), grade 0 provably fails at least as hard under correction (ρ′ ≥ 0.987 from the printed sums), but grade 2's band is not computable from v1's printed output.
2. **Unit operator speed.** v1 locked coefficient 1 on (d−δ) while the engine's characteristic speed is 1/√3; a genuine first-order mode at native speed would have registered ρ ≈ 0.73 → misclassified STATIC-ONLY. (v1's verdict is nonetheless salvageable via the measured gross non-uniformity of per-grade residuals, incompatible with any single-speed DK — but that argument was not pre-registered.)

v1.1 closes both: the **true DK operator** (uniform δ = d\*, making D skew-adjoint with D² = −Δ_H, verified numerically as harness gates) with a **freely fitted scalar speed** a and mass m, plus **per-grade weighting** in the joint fit (1/Σ‖∂ₜφ⁽ᵏ⁾‖² per grade) so the A-degree inhomogeneity (V ~ A, S,P ~ A², T ~ A³) cannot let one grade dominate the objective.

## 1 · The corrected system

Same grade fields, d operators, protocol, seeds, pairs, configs, and fit window as v1 §2. Codifferential replaced by the true adjoints (uniform ⟨dφ,ψ⟩ = ⟨φ,δψ⟩, asserted numerically to 10⁻¹² together with skew-adjointness ⟨DΦ,Ψ⟩ = −⟨Φ,DΨ⟩ and D² = −Δ componentwise on random fields):

- δ¹V = −(∇⁻ₓVₓ + ∇⁻ᵧVᵧ + ∇⁻_zV_z)
- δ²P = as v1 (already the true adjoint)
- δ³T: (δ³T)ₓᵧ = −∇⁻_zT, (δ³T)ₓ_z = +∇⁻ᵧT, (δ³T)ᵧ_z = −∇⁻ₓT

Net vs v1: eq0's sign and eq2's T-coupling sign flip; eq1, eq3 unchanged.

**Dynamical test:** ∂ₜΦ = a·(d−δ)Φ − mΦ, midpoint discretization; (a, m) fitted jointly by weighted least squares over all grades, sites, window ticks, seeds per (config, pair); per-grade weights 1/D_k, D_k = Σ‖∂ₜφ⁽ᵏ⁾‖². Report (a\*, m\*), per-grade ρ_k at the joint fit (unweighted within grade), ρ_all, plus the v1 KG comparator unchanged (both models now have ≥ 2 fitted parameters — the v1 comparator asymmetry is removed).

## 2 · Pre-registered outcomes (same taxonomy as v1 §2.5, criteria on the corrected fit)

| Outcome | Criterion (CONFIG-N, aggregated) |
|---|---|
| DK-DYNAMICAL | ρ_all < 0.15 AND every ρ_k < 0.25 AND fitted a\* consistent across the three pairs (relative spread < 30%) |
| DK-PARTIAL | ≥ 1 grade with ρ_k < 0.15 while ρ_all ≥ 0.15 |
| DK-STATIC-ONLY | all ρ_k ≥ 0.50 |
| UNDETERMINED | anything between |

**Priors (stated now):** DK-STATIC-ONLY 65%, DK-PARTIAL 15%, UNDETERMINED 15%, DK-DYNAMICAL 5%. Rationale: grades 1/3 carry over from v1 unchanged in operator (their residuals were ≥ 2.3 and a free scale can at best rescale, not restructure, them — and no single a fits ρ₁ = 8.6 alongside ρ₀ ≈ 1); grade 0 is bounded worse; only grade 2 is genuinely open.

**Sanity anchor (inherited):** CONFIG-M grade 1 must be KG-FORM or TIE.

## 3 · Artifacts

| Item | Path | SHA256 |
|---|---|---|
| v1.1 runner | `engine/tests/test_dk_evolution_v11.cpp` | `cdd8f6c7b50ee8996a202341bb08e30aefe94ef2a76bc59ca7c6740fda3f6205` |

CTest `dk_evolution_v11` (LABELS native eft vertex). Output to `engine/results/vertex_dk_closure_2026-07-10/m1_dk_evolution_v11.log`. Verdict impact: whatever lands updates the FTD-0379 row's scope — from "the §A1.3 literal variant at unit scale" to the corrected operator at fitted scale; no other tag can move. Ramification grade 0 (unchanged).
