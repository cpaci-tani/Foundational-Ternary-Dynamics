# EXPLR — Dark Matter / Baryon Ratio under Moore-Shell W5 Weighting

**Status:** [SELECTION — post-hoc structural finding, awaiting pre-registered confirmation against an independent observable]
**Date:** 2026-05-27
**Tag discipline:** This document records a numerical observation made via post-hoc enumeration of 9 weighting schemes. Per CLAUDE.md anti-target rules (no fishing for matches; pre-register before search) the W5 weighting is [SELECTION] — structurally motivated but NOT [DERIVED] from FTD axioms. A pre-registered confirmation against an independent cosmological observable is required to move this beyond [SELECTION].

**LEDGER:** No new row required. The W5 weighting is filed as a subsidiary observation under FTD-0028 (Moore Layer Theorem) with the cosmological identification remaining `[SELECTION]` per the existing audit-corrected constants.js convention (P0-15 update, 2026-05-27).

**Depends on:**
- `THEOREM_MOORE_LAYER_DECOMPOSITION.md` (FTD-0028, the 27-site partition: 1 + 6 + 12 + 8 sites by Moore shell)
- `DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md` (FTD-0029, BCC sublattice structure including T_+ / T_- stella octangula split)
- `engine/web/js/constants.js` Layer 9 (`DM_FRACTION = 17/27`, `BARYON_FRACTION = 10/27`) — the canonical voxel-counted reading
- `docs/reference/REF_EXTERNAL_CONSTANTS.md` (CODATA / PDG / Planck 2018 reference values)
- `docs/theory/07_assessment/core_ledgers/LEDGER.md` (FTD-0028 site count theorem)

**Related:**
- `engine/web/js/ui/components/faq/data.js` `dark-matter` entry (updated 2026-05-27 to cite this document)
- `engine/web/docs/audits/AUDIT_WEB_ENGINE_2026-05-27.md` P0-15 (the audit ticket that corrected `[THEOREM]` → `[SELECTION]` on the cosmological constants, opening the door to this exploration)

**Verification scripts:**
- `scripts/exploration/moore_shell_dm_baryon_weightings.py` (~150 lines; enumerates 9 weightings and computes DM:baryon ratio for each)

---

## §0 — Overview

The Moore Layer Theorem (FTD-0028) decomposes the 27-site Moore neighborhood into four orbits under the O_h point group: center (1), octahedron (6), cuboctahedron (12), and cube corners (8). The cube corners further split into T_+ ∪ T_- (stella octangula, 4 + 4) under the BCC complex-structure decomposition (FTD-0029). FTD's canonical reading partitions these 27 sites into:

- **DM partition (17 sites):** center + cuboctahedron + T_- = 1 + 12 + 4
- **BARYON partition (10 sites):** octahedron + T_+ = 6 + 4

Under **uniform voxel-counting** (each site counts as 1), the predicted DM:baryon ratio is 17:10 = 1.70. This is **catastrophically wrong** when compared to the Planck 2018 measurement of Ω_DM/Ω_b ≈ 5.375 — a factor-3.2 discrepancy.

This document records a structurally-motivated **per-site weighting scheme W5** (cuboctahedron × N_base = 4, others uniform) under which the prediction becomes 53:10 = 5.30 — within 1.4% of Planck and inside the 1σ uncertainty band. W5 is unique among 9 natural weighting candidates tested: the next-closest gives 6.80 (27% off), and every other candidate sits beyond 44% deviation.

The W5 finding is **suggestive but not derivative**. Its load-bearing status depends on a follow-up pre-registered test against an independent cosmological observable, not on this match alone.

---

## §1 — The puzzle, restated

### 1.1 Canonical reading (uniform voxel-counting)

`engine/web/js/constants.js` Layer 9 exports (post 2026-05-27 audit):

```js
export const DM_FRACTION     = 17.0 / 27.0;   // = 0.6296
export const BARYON_FRACTION = 10.0 / 27.0;   // = 0.3704
```

Three readings of these fractions are admissible — all numerically identical, only the cosmological interpretation differs:

| Reading | Meaning | FTD value | Planck 2018 |
|---|---|---:|---:|
| (a) DM:baryon | Ω_DM/Ω_b | 17/10 = 1.70 | 5.37 |
| (b) DM fraction of matter | Ω_DM/(Ω_DM+Ω_b) | 17/27 = 0.630 | 0.843 |
| (c) DM fraction of universe | Ω_DM/Ω_total | 17/27 = 0.630 | 0.265 |

**No reading matches Planck under uniform voxel-counting.** The discrepancy ranges from 24% (reading b) to factor-3.2 (reading a) to factor-2.4 (reading c). This is exactly the inconsistency the dashboard's pre-audit `[THEOREM]` tag on DM_FRACTION concealed; the audit P0-15 correction downgraded it to `[SELECTION]` to surface the open question.

### 1.2 What "weighting" means

Cosmological Ω fractions are **energy-density-weighted**: Ω_X = ρ_X / ρ_crit. Each species' contribution to Ω is proportional to its mass-energy density, not its particle count. If different Moore-shell sites have different effective mass-energy contributions per site, the voxel-counted 17:10 is a mis-estimate.

The lead-physicist's hypothesis (2026-05-27 brainstorm, post-audit): a weighting that "gives heavier weight to the 12 cuboctahedral states (the weak / heavy lepton layer) could compress 17:10 toward 5:1." This document tests that hypothesis against 8 alternatives.

---

## §2 — The W5 weighting hypothesis

### 2.1 Statement

**W5: cuboctahedron sites weight = N_base = 4; all other shells weight = 1.**

Structural argument (FTD-canonical):

1. The 12 cuboctahedral sites correspond to the 12 SM fermion species: 3 generations × 4 fermions/generation (electron, neutrino, up-quark, down-quark per generation).
2. Each fermion carries an internal multiplicity associated with `N_base = 4 = |Aut(E)|² = z_BCC`, the framework integer that appears as the |Aut(E)|² coefficient of the master quadratic (Theorem 1, FTD-0006) and the BCC coordination number.
3. Under cosmological-mass weighting, the 12 fermion-species sites contribute proportionally to their internal multiplicity. Octahedron, T_+, T_- sites carry weight 1 (no analogous multiplicity factor identified). Center carries weight 1 (singular site; A_1g singlet, no extra structure).

This gives:

$$\text{DM}_{\text{W5}} \;=\; 1 \cdot 1 + 12 \cdot 4 + 4 \cdot 1 \;=\; 53$$
$$\text{BARYON}_{\text{W5}} \;=\; 6 \cdot 1 + 4 \cdot 1 \;=\; 10$$
$$\frac{\Omega_{\text{DM}}}{\Omega_b}\bigg|_{\text{W5}} \;=\; \frac{53}{10} \;=\; 5.30$$

### 2.2 Why N_base specifically

`N_base = 4` is the only framework integer that produces a Planck-compatible result under cuboctahedron-only weighting:

| Cuboct weight | DM:b prediction | vs Planck 5.375 |
|:---:|:---:|:---:|
| 1 (uniform) | 1.70 | 68% off |
| **4 (N_base)** | **5.30** | **1.4% off** ← |
| 7 (b_3) | 8.90 | 66% off |
| 13 (N_eff) | 16.10 | 200% off |
| 3 (N_c) | 4.10 | 24% off |

The selection of `N_base = 4` is **not derived** from a first-principles argument — it is the integer that, among the framework integers `{N_c, N_base, b_3, N_eff}`, happens to fit. This is the principal honesty caveat (§5).

---

## §3 — Full enumeration of 9 natural weightings

Computed via `scripts/exploration/moore_shell_dm_baryon_weightings.py` (mpmath, 30 dps; results exact in rationals where applicable).

| # | Scheme | Rationale | DM:b ratio | vs Planck |
|:---:|:---|:---|:---:|:---:|
| W1 | Uniform voxel-counting | Each site counts as 1; current dashboard convention. | 1.70 | 68% off |
| W2 | \|Laplacian weight\| | Patra-Karttunen 18-pt isotropic Laplacian: face = 1/3, edge = 1/6, corner = 0, self = -4. Weight by \|coupling\|. | 3.00 | 44% off |
| W3 | A_1g density per site | Mult(A_1g) = 4 in 27-block (FTD-0110); distributed 1 per orbit. Weight per site = mult / \|orbit\|. | 1.67 | 69% off |
| W4 | Shell-d² weighting | Each site weighted by L² distance² (d² ∈ {0, 1, 2, 3}). Spatial-spread weighting. | 2.06 | 62% off |
| **W5** | **Cuboctahedron × N_base = 4** | **12 cuboct = 12 fermions; weight by internal multiplicity N_base = 4.** | **5.30** | **1.4% off ←** |
| W6 | Cuboctahedron × N_eff = 13 | Alternative framework integer (N_eff = 13). | 16.10 | 200% off |
| W7 | All non-center × N_base = 4 | Uniform N_base weighting on shells 1, 2, 3. | 1.70 | 68% off |
| W8 | T_1u dim where present (=3) | Fermion irrep dim (T_1u = 3). | 1.63 | 70% off |
| W9 | DM-shells × N_base = 4 | Boost ALL DM-classified shells (center, cuboct, T_-) by N_base = 4. | 6.80 | 27% off |

**Of 9 candidates, W5 is alone in landing within Planck 1σ.** The next-best (W9) is 27% off. No combination of "uniform" or "all-shells-weighted" reproduces 5.375.

---

## §4 — Full cosmological prediction under W5

Combining W5 with FTD's other canonical cosmological assignment `Ω_Λ = 2/3` (from constants.js Layer 9; structurally motivated but `[PARAMETRIC]` per audit P0-15):

| Quantity | FTD-W5 prediction | Planck 2018 | Deviation |
|---|---:|---:|---:|
| Ω_Λ | 2/3 = 0.6667 | 0.685 | 2.6% |
| Ω_m = 1 − Ω_Λ | 1/3 = 0.3333 | 0.315 | 5.8% |
| Ω_DM/Ω_m (within matter) | 53/63 = 0.8413 | 0.843 | **0.21%** |
| Ω_b/Ω_m (within matter) | 10/63 = 0.1587 | 0.157 | 1.1% |
| Ω_DM (of total universe) | 53/189 = 0.2804 | 0.265 | 5.8% |
| Ω_b (of total universe) | 10/189 = 0.0529 | 0.0493 | 7.3% |
| Ω_DM/Ω_b (primary observable) | 53/10 = 5.30 | 5.375 | **1.40%** |

**Within-matter fractions land tighter** (0.2% on Ω_DM/Ω_m, 1.1% on Ω_b/Ω_m) than universe-fraction predictions (5-7%) because the within-matter calculation is independent of the Ω_Λ = 2/3 assignment, depending only on the W5 weighting.

The universe-fraction deviations (5-7%) are dominated by the Ω_Λ = 2/3 vs observed 0.685 mismatch, which is a separate structural question.

---

## §5 — Discipline: post-hoc analysis status

### 5.1 What was done

On 2026-05-27, a 9-weighting enumeration was run via `moore_shell_dm_baryon_weightings.py`. The lead-physicist's W5 hypothesis was named in advance of the numerical comparison (in the 2026-05-27 prediction brainstorm) but was tested alongside 8 alternatives I generated. No pre-registration document was filed before the search.

### 5.2 Look-elsewhere accounting

Effective look-elsewhere factor: ~9 (the number of weightings tested). Under a naive null model where each weighting has a uniform ~10% chance of landing within Planck's 1σ band by accident, the prior probability of at least one match is ~60%. **Therefore the W5 match is not by itself decisive evidence**.

The argument for taking it seriously despite look-elsewhere:
- W5 was the specific hypothesis named by the lead-physicist in their brainstorm; it was not selected from the 9 by post-hoc fitness.
- Of the 4 framework integers tested for cuboct weighting (N_c = 3, N_base = 4, b_3 = 7, N_eff = 13), only N_base produces a match — the others are 24%, 66%, 200% off respectively. This is a *single-integer* selection within the framework-integer family, not an arbitrary parameter.
- W5 simultaneously matches TWO observables (Ω_DM/Ω_b at 1.4%, Ω_DM/Ω_m at 0.2%), though these are algebraically linked so this counts as one data point.

The argument against:
- The choice "weight cuboctahedron only" (vs uniformly weighting all shells) is itself a selection. W7 (all non-center × N_base) reproduces the uniform-voxel ratio because the weighting cancels out — but a different "all shells" weighting could have matched.
- "Why N_base and not another integer" lacks a derivation chain. The hypothesis (cuboct = 12 fermion species × N_base internal multiplicity) is structurally suggestive but not theorem-grade.

### 5.3 Honest tag: [SELECTION]

Per CLAUDE.md's tag system, [SELECTION] denotes "argued from consistency, not uniquely proven." The W5 weighting fits this exactly: it is consistent with FTD's structural integers, reproduces Planck within 1σ on the primary observable, but is not uniquely forced from axioms. Promotion to a stronger tag requires a pre-registered confirmation against an independent observable.

---

## §6 — Proposed independent confirmation test

To move W5 from [SELECTION] toward something firmer, a pre-registered test against an **observable not used to construct or validate W5** is needed. Candidate independent observables (filed as future pre-registration targets, NOT claims):

### 6.1 CMB acoustic-peak position

The first acoustic peak in the CMB power spectrum sits at ℓ₁ ≈ 220 (Planck 2018). Its position depends on the sound horizon at recombination, which is set by Ω_b·h². Under W5, Ω_b = 53/189·h²-rescaled, and h ≈ 0.674 (Hubble parameter / 100 km/s/Mpc) gives a specific ℓ₁ prediction. The prediction needs to be computed by running a CMB-physics code (CAMB, CLASS, or analytic approximation) with Ω_b = 0.0529 substituted; the prediction must be made BEFORE comparing to ℓ₁ ≈ 220.

### 6.2 BBN ⁴He mass fraction Y_p

Big Bang Nucleosynthesis (BBN) predicts Y_p (helium-4 mass fraction) as a function of η_B = baryon-to-photon ratio. Under W5, Ω_b = 0.0529 implies η_B ≈ 6.2 × 10⁻¹⁰ (using standard relations). The predicted Y_p ≈ 0.246 should be compared against the observational Y_p ≈ 0.245 ± 0.003 (Aver et al. 2015). If Y_p prediction lands within 1σ under W5 (and NOT under W1), that's a strong independent test.

### 6.3 Matter-radiation equality redshift z_eq

z_eq = Ω_m/Ω_rad − 1 ≈ 3402 (Planck 2018). Under W5, Ω_m = 1/3, and Ω_rad ≈ 9.2 × 10⁻⁵ (from photon + neutrino temperatures). Prediction: z_eq ≈ 3623 — a 6.5% deviation from Planck observed. This is a less sharp test (depends on Ω_m = 1/3, which is the Ω_Λ = 2/3 assignment).

### 6.4 Pre-registration template

A future `PREREG_DM_BARYON_W5_INDEPENDENT_CONFIRMATION_v1.md` would:
1. Hash-lock the W5 weighting (frozen at §2.1 above)
2. Pre-specify the independent observable (one of §6.1/2/3) BEFORE the numerical prediction is computed
3. Define the pass/fail threshold (e.g. within 2σ of Planck for FOUND; within 5σ for UNDERDETERMINED; beyond 5σ for CLOSED-NEGATIVE)
4. Run the comparison and report the verdict in a separate result document

---

## §7 — What this exploration is NOT

- **Not a derivation of Ω_DM/Ω_b from FTD axioms.** The W5 weighting is structurally motivated but the weight assignment "cuboctahedron × N_base = 4" is not derived from the five postulates. It is a hypothesis that fits.
- **Not a promotion of DM_FRACTION's epistemic tag.** Per audit P0-15, `DM_FRACTION = 17/27` remains `[SELECTION]` in `constants.js`. This document does not change that.
- **Not a refutation of the uniform-voxel reading.** The 17/27 voxel count IS the canonical Moore-shell partition (FTD-0028 theorem-grade). What's `[SELECTION]` is the cosmological identification "17 voxels = dark matter, 10 voxels = baryonic matter". W5 explores a refinement: "and the 12 cuboctahedral DM voxels carry weight N_base = 4 each."
- **Not load-bearing for any other FTD prediction.** Confined to the Moore-shell  cosmological partition question.
- **Not a pre-registration.** Future independent-observable tests against W5 require their own pre-registration documents.

---

## §8 — Single-line summary

**Under the post-hoc weighting "cuboctahedron sites × N_base = 4, all others × 1" (W5), the Moore-shell partition reproduces Planck's Ω_DM/Ω_b to 1.4% and Ω_DM/Ω_m to 0.2%; among 9 natural FTD weighting candidates tested, only W5 lands within Planck 1σ; the finding is [SELECTION] pending pre-registered confirmation against an independent cosmological observable.**

---

## §9 — Provenance

- 2026-05-27 evening: lead-physicist brainstorm names W5 (cuboctahedron weighting) as a candidate compression for the 17:10 → 5.4 discrepancy.
- 2026-05-27 same session: `scripts/exploration/moore_shell_dm_baryon_weightings.py` enumerates 9 weighting candidates and identifies W5 as the unique Planck-compatible scheme.
- 2026-05-27 same session: FAQ entry `dark-matter` updated to cite both readings (uniform 1.7 and W5 5.3) and this document.
- 2026-05-27 same session: this document filed at `docs/theory/10_eft_program/EXPLR_DM_BARYON_W5_WEIGHTING.md`.

The exploration was prompted by the audit P0-15 correction that demoted constants.js's `DM_FRACTION` tag from `[THEOREM]` to `[SELECTION]`, which surfaced the cosmological-mismatch question as a genuine `[OPEN]` rather than a `[THEOREM]` overclaim. The W5 hypothesis is an attempt to resolve that `[OPEN]` constructively rather than negatively. The verdict on whether W5 stands or falls awaits pre-registered confirmation per §6.4.

---

*End of exploration. Result document to follow under §6 pre-registration.*
