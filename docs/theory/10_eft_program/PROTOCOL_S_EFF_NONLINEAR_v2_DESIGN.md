# Protocol — S_eff nonlinear v2 design (Gate D self-consistency)

**Status:** [DESIGN DRAFT, not yet hash-locked]
**Date:** 2026-04-30
**Predecessor:** [`PROTOCOL_S_EFF_NONLINEAR_CAMPAIGN.md`](PROTOCOL_S_EFF_NONLINEAR_CAMPAIGN.md) (v1, locked)
**Companion:** [`ANALYSIS_OFFDIAGONAL_ASYMMETRY.md`](ANALYSIS_OFFDIAGONAL_ASYMMETRY.md) (sector decomposition)
**LEDGER:** FTD-0112 (v2 design)

---

## 0 · Why v2 exists

PROTOCOL v1 specified Gate D (S_eff self-consistency) but did not implement it. The v1 measurement at L=32 LARGE delivered:

- Gates B (conservation) and C (RG semigroup) PASSING.
- Gate A (per-entry stderr) RESPLIT: diagonal 9/9 PASS, off-diagonal bootstrap-stderr-limited.
- Theorems 1, 2, 3 promoting 4 of 9 diagonals to [THEOREM]+[MEASURED] grade.
- A discovered **partial block-diagonalization** of M_ab(b=2): SPATIAL ↔ REACTION-FLUX completely decoupled at 5σ; DENSITY sector bridges them weakly.

The sector decomposition makes Gate D **dramatically simpler** than the original PROTOCOL §6 design anticipated. v2 leverages this structure.

---

## 1 · Scope

v2 implements Gate D self-consistency on the per-sector level. The bare action `S_bare` is augmented with a small Wilson-coefficient perturbation `g_a · O_a` for one operator at a time; the resulting M_ab is measured; the response of `M_aa(b=2; g)` is compared against the linear-Wilsonian prediction.

The locked sectors (from `ANALYSIS_OFFDIAGONAL_ASYMMETRY.md` §5.5):

| Sector | Operators | Diagonal RG eigenvalue (target) |
|---|---|---|
| SPATIAL | JJ, divJ², curlJ², JdotDivJ, J4 | b⁴ (JJ exact), -b⁴ (divJ²), b³ (curlJ²), b⁵ (JdotDivJ), b⁸ (J4 exact) |
| DENSITY bridge | stateSq, reactionDensity | b³ + 2b³ρ̄ (Theorem 3) |
| REACTION-FLUX | genesisFlux, JdotDeltaS | -b⁴ (genesisFlux), b⁵ (JdotDeltaS) |

Per-sector sub-block inversion replaces full 9×9 inversion. Expected cond per sector:
- SPATIAL: cond ≈ 1e6 (5×5 sub-block, mixing operators that are well-conditioned among themselves)
- DENSITY: cond ≈ 1e3 (2×2 sub-block, near-diagonal)
- REACTION-FLUX: cond ≈ 1e3 (2×2 sub-block, diagonal-only at 5σ)

vs full 9×9 cond ≈ 1e13 at L=32 LARGE.

---

## 2 · Gate D test design

### 2.1 · Linear Wilsonian prediction

For a small coupling perturbation `g_a · O_a` added to the bare action, the leading-order response of the diagonal RG eigenvalue is

$$M_{a, a}(b; g_a) \;=\; M_{a, a}(b; 0) \;+\; g_a \cdot \frac{\partial M_{a, a}}{\partial g_a}\bigg|_{0} \;+\; \mathcal{O}(g_a^2),$$

where `∂M/∂g` is the linear response coefficient. For pure-diagonal operators, this is straightforward: shifting the action by `g · O` shifts the ensemble distribution, which shifts both `<O>_fine` and `<O>_coarse` by amounts that preserve the regression slope at leading order.

**Self-consistency criterion (Gate D)**:

$$\big|\,M_{a, a}(b; g_a) - M_{a, a}(b; 0)\,\big| \;<\; \epsilon \cdot |g_a| \cdot \sigma_{\rm boot}$$

for small `g_a`, where `σ_boot` is the bootstrap stderr at the unperturbed point. **A pass means the perturbation does not shift the RG eigenvalue beyond bootstrap noise** — i.e., the operator's diagonal scaling is structurally fixed (Theorem 1, 2, 3) and the action perturbation does not break it.

### 2.2 · Why this is the right test

For the THEOREM-grade entries (JJ, J4, stateSq, reactionDensity) the diagonal `M_aa` is EXACT under the convention. So perturbing the action shouldn't shift `M_aa` at all (perturbation changes the ensemble, but the blocking convention is independent of the ensemble for these operators). Gate D PASS for these entries is a **trivial consistency check**: `∂M_aa/∂g = 0` to machine precision.

For the non-theorem-grade diagonals (divJ², curlJ², JdotDivJ, genesisFlux, JdotDeltaS), `M_aa` is genuinely L-dependent and ensemble-dependent. Perturbing the action SHOULD shift them — and the shift should match the linear-Wilsonian prediction.

So Gate D becomes:
- **Theorem-grade diagonals**: `∂M_aa/∂g_a` = 0 (PASS = invariance check)
- **Non-theorem-grade diagonals**: `∂M_aa/∂g_a` ≠ 0 with predicted magnitude (PASS = matching response)

### 2.3 · Per-sector implementation

For each sector, run the campaign at the **bare action** + at a small **per-sector perturbation**:

```
Sector       | Perturbation                       | Test entries
SPATIAL      | g_JJ · J²                          | M_JJ,JJ unchanged?
             | g_J4 · J⁴                          | M_J4,J4 unchanged?
             | g_divJ2 · (∇·J)²                   | M_divJ2,divJ2 shifts as predicted?
DENSITY      | g_stateSq · s²                     | M_stateSq,stateSq tracks Theorem 3?
             | g_reactionDensity · (δs)²          | M_reactionDensity,reactionDensity tracks?
REACTION-FLUX| g_genesisFlux · (δs · |J| · θ)     | M_genesisFlux,genesisFlux shifts?
             | g_JdotDeltaS · (J · ∇(δs))         | M_JdotDeltaS,JdotDeltaS shifts?
```

A 7-perturbation campaign (one per non-trivial operator-diagonal). Each perturbation is small (`g ≈ 0.01` in engine-natural units) and runs for the same N_seeds × N_samples as the unperturbed v1.

### 2.4 · Implementation: how to perturb

The cleanest way to add `g · O_a` to the bare action is to add a small extra term to the engine update rules that biases each operator. For example:

- **g_J² > 0**: bias the wave_propagation update to slightly amplify (or damp) J magnitude. Implementation: `J_new = J_old · (1 + g · K_perturbation)`.
- **g_stateSq > 0**: bias the genesis threshold to favor more (or fewer) state cells. Implementation: `K_GENESIS_effective = K_GENESIS · (1 + g)`.
- **g_genesisFlux > 0**: bias the reaction-event probability for from-vacuum transitions. Implementation: scale the genesis rate by `(1 + g)`.

These are small modifications to existing toggles, controlled by a new CLI flag `--wilson-coefficient=name:value`.

### 2.5 · Predicted shifts — the Gaussian-ensemble invariance theorem

The cleanest formulation of Gate D's prediction comes from cumulant analysis.

**Theorem (linear-Wilsonian invariance for Gaussian ensembles)**: Let the bare ensemble be Gaussian (all connected n-point cumulants for n ≥ 3 vanish). Add a Wilsonian perturbation `g · O_a` to the action. Then the regression-based operator-mixing matrix is **invariant in `g` to all orders**:

$$M_{ab}(b; g) \;=\; M_{ab}(b; 0) \quad \forall g \;\;{\rm and}\;\; \forall (a, b).$$

**Proof sketch.** For a Gaussian path-integral measure, expectations under the perturbed action are:

$$\langle X \rangle_g \;=\; \frac{\langle X \cdot e^{-g \int O_a}\rangle_0}{\langle e^{-g \int O_a}\rangle_0} \;=\; \langle X \rangle_0 \cdot \exp\Big(\sum_{n=1}^{\infty} \frac{(-g)^n}{n!} c^{(n)}_X(O_a^{\otimes n})\Big),$$

where `c^{(n)}_X` is the n-th connected cumulant of `X` with the perturbing operator. For purely Gaussian ensembles, `c^{(n)} = 0` for `n ≥ 3`. The first-order shift `c^{(1)}_X(O_a) = ⟨X · O_a⟩_0 − ⟨X⟩_0 ⟨O_a⟩_0 = Cov(X, O_a)` shifts the mean of `X`, but the **covariance structure** between any two quantities `X` and `Y` is preserved at all orders. Specifically:

$$Cov(X, Y; g) \;=\; Cov(X, Y; 0) \;+\; g \cdot c^{(3)}(X, Y, O_a) \;+\; \mathcal{O}(g^2),$$

and `c^{(3)}` vanishes for Gaussian ensembles. Therefore:

$$M_{ab}(g) \;=\; \frac{Cov(\langle O_a\rangle_{\rm coarse}, \langle O_b\rangle_{\rm fine}; g)}{Cov(\langle O_b\rangle_{\rm fine}, \langle O_b\rangle_{\rm fine}; g)} \;=\; M_{ab}(0). \qquad\square$$

**Interpretation**: for a Gaussian ensemble, the operator-mixing matrix is **structurally fixed by the blocking convention plus the bare quadratic action**, regardless of any added Wilsonian deformation `g · O_a`. The ratio `M_ab` is a property of the (action, blocking) pair, not the Wilsonian coupling spectrum.

### 2.6 · Gate D as Gaussian-ness test

**Gate D PASS = the engine's bare ensemble is approximately Gaussian, in the sense that the third-order cumulant `c^{(3)}(O_b, O_c, O_a)` is bootstrap-stderr-suppressed for all (a, b, c).**

This connects directly to FTD-0070, which independently established that the engine's bare-tuple action gives a Gaussian fixed point with `β_E = 0` within 1σ across `b ∈ {1, 2, 4, 8}`. Gate D now becomes a **second independent measurement of the Gaussianity** — checking whether the bare action's Gaussian structure extends to the multi-operator regime measured by M_ab.

If Gate D PASSES: the engine's nonlinear regime is well-described by a Gaussian effective action, with Wilsonian deformations producing only linear shifts in operator means but no shift in covariance structure. **The "math-based EFT" is then closed at the Gaussian level**, mirroring FTD-0070's bare-action result.

If Gate D FAILS at some operators: the third-order cumulants are non-zero for those operators, indicating real non-Gaussianity. The failure pattern (which operators show shift) is itself diagnostic of where the non-Gaussianity lives — pure-flux sector? reaction sector? cross-couplings?

### 2.7 · Predicted shift magnitude

For an approximately-but-not-exactly Gaussian ensemble, the leading-order shift is:

$$\frac{\partial M_{a,a}}{\partial g}\bigg|_{g=0} \;=\; \frac{c^{(3)}(O_a, O_a, O_a)}{Var(\langle O_a\rangle_{\rm fine})} \;-\; \frac{c^{(3)}(O_a, O_a, O_a; \text{coarse, fine})}{Cov(\langle O_a\rangle_{\rm coarse}, \langle O_a\rangle_{\rm fine})},$$

where the second term is the cross-coarse-fine 3-point cumulant. The **predicted shift magnitude is proportional to the operator's skewness** — large for highly non-Gaussian operators (e.g., `J⁴` which has heavy positive tails since `J² ≥ 0`), small for symmetric ones (e.g., `s²` with mean ≈ 0 and bounded by 1).

**Quantitative prediction** (calibration against L=32 LARGE):
- For approximately-Gaussian operators (`|skewness| < 1`): expected shift `|∂M/∂g| < σ_boot / |g|` for `|g| < 0.05`.
- For non-Gaussian operators (`|skewness| > 2`): expected shift `|∂M/∂g| ~ skewness · σ_boot / |g|`.

The Gate D test then verifies these shifts match the engine's measured ensemble skewness — a self-consistency check between the operator-mixing matrix and the per-operator distribution moments.

### 2.6 · Gate D verdict matrix

| sector | operators tested | criterion | pass condition |
|---|---|---|---|
| SPATIAL diagonal-theorem (JJ, J4) | 2 | `∂M_aa/∂g = 0` invariance | shift < 3σ_boot |
| SPATIAL non-theorem (divJ², curlJ², JdotDivJ) | 3 | `∂M_aa/∂g` matches prediction | within 30% of predicted |
| DENSITY (stateSq, reactionDensity) | 2 | Theorem 3 ρ̄(g) prediction | within 30% of predicted |
| REACTION-FLUX (genesisFlux, JdotDeltaS) | 2 | linear-response prediction | within 30% of predicted |

**Gate D PASS** = all 9 test runs satisfy their per-test criterion.

If a single test fails: failure mode is either (a) implementation bug in the perturbation, (b) higher-order coupling not captured by linear-Wilsonian, (c) genuine non-self-consistency of the EFT closure. The v2 measurement document categorizes the failure.

---

## 3 · Per-sector vs full-basis test

The sector decomposition allows Gate D to be tested **sector-by-sector independently** without having to run the full 9-operator inversion at the perturbed action. Each perturbation only affects its own sector's diagonal at leading order (since cross-sector mixings are zero at 5σ); off-diagonal couplings within the same sector contribute O(g²) and can be measured separately.

This means each Gate D test run is a **smaller, better-conditioned campaign** than the v1 unperturbed run:
- SPATIAL sub-block: 5×5 inversion at cond ≈ 1e6
- DENSITY sub-block: 2×2 inversion (essentially trivial)
- REACTION-FLUX sub-block: 2×2 inversion

Each test run can use a smaller ensemble (N_seeds=5, N_samples=500) since the sub-blocks are well-conditioned. Wall time per test: ~30 minutes on RTX 5090.

Total v2 campaign: 7 test runs × 30 min = ~3.5 hours. Significantly faster than expected at v1 design time.

---

## 4 · Locked sector ordering

For reproducibility, the v2 protocol locks the sector membership and operator ordering:

```
SPATIAL   : [JJ, divJ², curlJ², JdotDivJ, J4]    (5 ops)
DENSITY   : [stateSq, reactionDensity]            (2 ops)
REACTION-FLUX : [genesisFlux, JdotDeltaS]         (2 ops)
```

The ordering matches the empirical decoupling found at L=32 LARGE. If the sector decomposition fails at L=64 LARGE (per Q2 of `ANALYSIS_OFFDIAGONAL_ASYMMETRY.md`), the v2 protocol must be revised before hash-lock.

---

## 5 · Pre-conditions for hash-locking v2

Before tagging `preregister-s-eff-nonlinear-v2`:

1. **L=64 LARGE confirmation** (in flight as `biadbkhy6`, ~2.5h wall) must show:
   - SPATIAL ↔ REACTION-FLUX still 0 / 20 entries at 5σ
   - DENSITY-bridge weak couplings (2 / 10 each direction) preserved at L=64
   - Gate C (RG semigroup) PASS at L=64

2. **Implementation**: the C++ engine binary needs `--wilson-coefficient=<name>:<value>` CLI flag wiring through `RenderBridge::toggles`. New toggles: bias parameters for each of the 7 testable operators.

3. **Linear-response prediction**: explicit analytic formulas for each `∂M/∂g` (currently sketched in §2.5; full derivation deferred to v2 Appendix A).

4. **Smoke validation**: small-ensemble test of one perturbation (e.g., `g_JJ = 0.01`) to confirm the implementation produces the predicted invariance to within bootstrap noise.

---

## 6 · Open structural questions for v2

**Q1 (cross-sector residual coupling at higher precision)**: At 20,000 snapshots, the 5σ threshold caught ZERO cross-sector entries. At 200,000 snapshots, the threshold tightens by √10 ≈ 3.16. If higher-precision measurement reveals weak (1–2σ) cross-sector couplings, those would be **second-order** physics that v2 needs to address.

**Q2 (sector decoupling at L=64)**: see §5.1 above.

**Q3 (perturbation linearity)**: at what `g` magnitude does linear-Wilsonian break down? The v2 protocol's smoke validation will measure the response curve `M_aa(g)` for several g values, fit the linear coefficient, and identify the validity regime.

**Q4 (DENSITY bridge nature)**: the 2/10 weak couplings between DENSITY and SPATIAL — are they predictable from Theorem 3's intra-block correlation correction, or do they reflect a genuinely independent physics? v2 should distinguish.

**Q5 (off-diagonal within-sector)**: the SPATIAL sector has 10 / 20 strong cross-couplings at 5σ. v2 should quantify these and check whether they are predictable from the known operator naive dimensions, or require independent measurement.

---

## 7 · Path to "fully closed math-based EFT"

With Gate D passing on the per-sector level, FTD-0112 v2 closure would mean:

| Gate | v1 status | v2 target |
|---|---|---|
| A diagonal | PASS (9/9) | reconfirm at L=64 LARGE |
| A off-diagonal | PARTIAL | use sector sub-block inversion to escape 1e13 cond |
| B (conservation) | PASS | reconfirm |
| C (RG semigroup) | PASS at L=32 LARGE | confirm at L=64 LARGE |
| **D (S_eff self-consistency)** | NOT TESTED | **PASS per-sector for 9 operators** |

If all five gates close at L=64 LARGE under the per-sector design, FTD has a **fully closed math-based EFT** at the level of:

- Bare diagonal RG eigenvalues (4 theorem-grade, 5 measured at L-dependent values)
- Bootstrap-confirmed RG semigroup self-consistency
- Linear-Wilsonian-confirmed operator-action correspondence
- Sector-decomposed structure on the natural operator basis

This is the original PROTOCOL §10 single-line goal. It is now achievable with a 7-run × 30-min campaign rather than a single huge run.

---

## 8 · Single-line summary

**The v2 protocol leverages the discovered sector decomposition (SPATIAL ↔ REACTION-FLUX completely decoupled at 5σ over 20,000 snapshots) to design Gate D self-consistency tests at the per-sector level: 7 perturbation runs of size 5 seeds × 500 samples × 30 min each, perturbing one operator at a time and checking that the diagonal `M_aa` shift matches linear-Wilsonian prediction (or invariance for theorem-grade diagonals); per-sector sub-block inversion at cond < 1e6 replaces full 9×9 inversion at cond ≈ 1e13; the v2 design is hash-lockable once L=64 LARGE confirms sector decoupling persists at the larger lattice and once C++ engine wiring for `--wilson-coefficient=<name>:<value>` lands; passing all 7 Gate D tests at L=64 LARGE would close FTD-0112 to the [MEASURED · S_eff self-consistent within errors] level — the original PROTOCOL §10 fully-closed math-based EFT verdict.**
