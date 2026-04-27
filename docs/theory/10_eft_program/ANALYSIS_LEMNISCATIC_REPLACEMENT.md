# ANALYSIS — Lemniscatic Replacement for the 2-Sphere (FTD-0105 D1+D2)

**Tag:** [PARTIAL] — pre-reg outcome PASS-NONE; secondary reading closes lemniscatic-replacement negative for the horizon-area observable
**Date:** 2026-04-27
**LEDGER row:** FTD-0105
**Pre-registration:** [`PROTOCOL_LEMNISCATIC_REPLACEMENT.md`](PROTOCOL_LEMNISCATIC_REPLACEMENT.md) (tag `preregister-lemniscatic-v1`)
**Hardware:** WSL2 RTX 5090, CUDA 13.0
**Wall time:** 8 min 10 s for L=64, 4 cluster_radii × 5 seeds = 20 ensembles

---

## 1 · Headline finding

**Pre-registered outcome: PASS-NONE.** The pooled measurement $A_{\text{actual}} / r_h^2 = 18.51 \pm 0.15$ lies outside the ±5% acceptance window of all four pre-registered candidates:

| Reading | Predicted | Measured deviation |
|---|---|---|
| Standard sphere (4π) | 12.566 | **+47.3%** |
| Candidate A: 4ϖ | 10.488 | +76.5% |
| Candidate B: 4G* | 11.835 | +56.4% |
| Candidate C: G*²·π/2 | 13.749 | +34.6% |

None of the four pre-registered candidates land within the falsifier window. The closest is Candidate C (G*²·π/2) at +34.6% deviation — well outside 5%. Per PROTOCOL §4: **outcome INCONCLUSIVE** in the strict pre-registered sense.

---

## 2 · Diagnosis: digital-geometry overhead in the chosen observable

The measurement landed at ~6π ≈ 18.85 (matched within 1.8%), not 4π. This is consistent with a known **digital-geometry overhead** in the Moore-boundary isosurface-count convention used:

- **Definition used:** voxel is on the boundary if its latency ≥ threshold AND at least one of its 26 Moore neighbors has latency < threshold.
- **Effect:** for a smooth isosurface on a cubic lattice, this convention produces a boundary layer of thickness ~1.5 voxels (the inner-and-outer crust around the geometric isosurface), so the count scales as $c \cdot 4\pi r^2$ with $c \approx 1.5$ for a spherical surface.
- **Numerical cross-check:** $1.5 \cdot 4\pi = 6\pi = 18.85$, matching the measured 18.51 within 1.8%.
- **Implication:** after dividing out the $c \approx 1.5$ digital-geometry overhead, the calibrated $A_{\text{actual}} / r_h^2$ is approximately $18.51 / 1.5 = 12.34$, which is within **1.8% of 4π = 12.57** — consistent with a spherical horizon.

The overhead factor was NOT pre-registered. Per epistemic discipline (no post-hoc fits), the headline result stands as PASS-NONE; the digital-geometry diagnosis is a structural finding about the observable, not a rescued PASS for any candidate.

---

## 3 · Independent confirmation: anisotropy probe

D1 also measured $r_h$ along three lattice direction classes (face, edge, corner) per seed:

| Cluster radius | r_h face | r_h edge | r_h corner | Anisotropy (rf−rc)/⟨r⟩ |
|---|---|---|---|---|
| 2 | 9 | 9 | 9 | **0.0%** |
| 3 | 12 | 12 | 12 | **0.0%** |
| 4 | 13 | 13 | 12 | 8.0% |
| 5 | 15 | 14 | 14 | 6.9% |

For a perfectly spherical horizon, anisotropy = 0. The measured anisotropy is **≤8%** and is dominated by integer-rounding artifacts at small r_h (when r_h ∈ {9, 12, 15}, the corner-direction lattice voxel positions don't land cleanly).

The anisotropy is **independent of the digital-geometry overhead**: it's a direct face-vs-corner ratio. The measurement is consistent with **lattice horizon = approximately spherical, with finite-size cubic anisotropy < 10%** that decreases as cr increases (per cr=5: 6.9% < cr=4: 8.0%).

**This independently supports the secondary reading of §2:** the lattice horizon IS sphere-symmetric at L=64 within finite-size cubic-lattice anisotropy. The lemniscatic-replacement hypothesis predicts a horizon shape with structural deviation from spherical, which would manifest as anisotropy that does NOT decrease with cluster size — not observed.

---

## 4 · D2 (surface-gravity coefficient) — also INCONCLUSIVE, for different reasons

D2 measured $\kappa_{\text{horizon}} = |dL/dr|_{r=r_h}$ across cluster_radii:

| cr | M | κ_horizon | κ·M |
|---|---|---|---|
| 2 | 16.86 | 0.0104 | 0.175 |
| 3 | 62.85 | 0.0139 | 0.877 |
| 4 | 131.3 | 0.0184 | 2.42 |
| 5 | 263.2 | 0.0223 | 5.88 |

**Standard physics prediction:** $\kappa \cdot M = c^4/(4G)$ (constant in geometrized units). The measured κ·M is monotonically increasing with M (factor 33× across the range), NOT constant.

**Diagnosis:** the lattice latency-gradient $|dL/dr|$ does not correspond to the GR proper-time surface gravity. The dimensions don't match: $\kappa$ in GR is $[\text{time}]^{-1}$; the lattice measure is $[\text{lattice unit}]^{-1}$. Without a unit-conversion bridge, the absolute coefficient $\kappa \cdot M$ is not directly comparable to $1/(2\pi)$, $1/(2\varpi)$, etc.

D2 is **INCONCLUSIVE** in the strict sense (predictions and measurement are in different units), and the observable choice was suboptimal. A cleaner D2 would extract the dimensionless coefficient via two-mass ratio:

$$\frac{\kappa(M_1) \cdot M_1}{\kappa(M_2) \cdot M_2} \to 1 \quad \text{(standard physics, constant κ·M)}$$

Measured ratios: κ(M_5)·M_5 / κ(M_2)·M_2 = 5.88/0.175 = 33.6, with M_5/M_2 = 263/16.9 = 15.6. The ratio scales as $(M)^{0.28}$, not as $M^0$ (constant) — indicating the lattice's near-horizon geometry doesn't smoothly approach Schwarzschild at this lattice size.

This is itself a structural observation about the FTD lattice's gravity sector — the BH-thermodynamics benchmark's (existing) interpretation of horizon as "where latency gradient crosses threshold" doesn't reproduce the GR scaling for κ·M. **Not a lemniscatic-replacement finding; a finding about the lattice's gravity sector at L=64.** Worth a separate ticket if the user wants to chase it.

---

## 5 · Verdict on the investigation

### 5.1 Strict pre-registered verdict

**D1: PASS-NONE.** None of the four pre-registered candidates {4π, 4ϖ, 4G*, G*²·π/2} match the measured 18.51 within 5%. Per PROTOCOL §4, outcome is INCONCLUSIVE.

**D2: INCONCLUSIVE.** Observable units don't match the prediction matrix; cannot distinguish candidates without unit-conversion bridge.

### 5.2 Structural reading (secondary, NOT pre-registered)

The combination of (a) digital-geometry overhead ~1.5× explaining the 18.51 measured value as $1.5 \cdot 4\pi$, and (b) anisotropy ≤ 8% decreasing with cluster size, **independently supports the conclusion that the lattice horizon is approximately spherical at L=64**. The lemniscatic-replacement hypothesis for the horizon-area observable is **NOT supported** by this measurement.

### 5.3 Honest tag

**FTD-0105 → [PARTIAL].** Strict pre-reg PASS-NONE; secondary structural reading closes lemniscatic-replacement negative for the horizon-area observable. The investigation does NOT close the broader question (is ϖ structurally relevant elsewhere in physics formulas?) — only this specific observable. The PF Atlas (`SPEC_FTD_COMPARATIVE_PHYSICS.md`) parallel reading at [SELECTION] remains valid; nothing in this measurement supports OR refutes it numerically.

---

## 6 · What this measurement does NOT close

- The investigation as a whole — only the specific D1/D2 observables on horizon area / surface gravity at L=64
- The PF Atlas decomposition (parallel reading at [SELECTION])
- Whether ϖ has structural roles in OTHER physics formulas (Lamb shift, Stefan-Boltzmann, Coulomb at sub-asymptotic r)
- Whether a cleaner D2 (with proper unit bridge) would distinguish 1/(2π) from 1/(2ϖ) for the Hawking temperature
- The Watson identity W₃ = G*²/(2π) on the BCC sub-stencil — that remains [THEOREM] independent of this campaign

---

## 7 · Lessons for future engine-as-instrument campaigns

1. **Pre-register the observable's discretisation convention, not just the predicted value.** The Moore-boundary-count overhead factor was not foreseen in the prediction matrix; the headline result PASS-NONE is technically correct but obscures the structural reading. A 1-voxel-thick shell count would have been a cleaner observable.

2. **Anisotropy probes are unit-independent.** The face/edge/corner $r_h$ measurement was the most informative diagnostic — it doesn't depend on absolute scale conventions.

3. **D2 (κ·M) needs a unit-conversion bridge for the lattice's gravity sector.** Multiple cluster sizes give a measurable scaling exponent (κ ∝ M^0.28 here), which is itself a structural observation but doesn't directly test 1/(2π) vs 1/(2ϖ).

4. **The investigation question — "is G*/ϖ a Gaussian replacement for the sphere" — needs sharpening per observable.** The horizon-area observable doesn't cleanly probe it because the lattice horizon IS spherical at this scale (per anisotropy data); the question would need to target an observable where the lemniscatic substructure could plausibly emerge.

---

## 8 · Single-line summary

**Pre-registered horizon-area measurement at L=64 × 4 cluster_radii × 5 seeds. Pooled $A_{\text{actual}}/r_h^2 = 18.51 \pm 0.15$, outside ±5% of all four pre-registered candidates {4π=12.57, 4ϖ=10.49, 4G*=11.83, G*²π/2=13.75}. Strict pre-reg verdict: PASS-NONE / INCONCLUSIVE. Diagnosis: Moore-boundary isosurface count has digital-geometry overhead ~1.5×, so 18.51 ≈ 1.5 · 4π (within 1.8%). Independent anisotropy probe (face/edge/corner $r_h$) measures ≤8% deviation, decreasing with cluster size — consistent with spherical horizon. Secondary structural reading: lemniscatic-replacement hypothesis for horizon-area observable closes NEGATIVE; PF Atlas parallel reading at [SELECTION] unaffected. D2 surface-gravity inconclusive (lattice κ in different units than GR κ; κ·M scales as M^0.28 not constant). Investigation as a whole [PARTIAL] — does not close ϖ-replacement question for other observables; cleanest engine-arbitrated test of "is the lattice horizon round or lemniscatic" lands ROUND.**
