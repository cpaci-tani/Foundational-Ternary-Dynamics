# PROTOCOL — Lemniscatic Replacement for the 2-Sphere Horizon (D1 + D2)

**Tag:** [PROTOCOL · pre-registration]
**Date:** 2026-04-27
**LEDGER row:** FTD-0105
**Companion:** [`AUDIT_LEMNISCATIC_SPHERE_REPLACEMENT.md`](AUDIT_LEMNISCATIC_SPHERE_REPLACEMENT.md)
**Plan:** `~/.claude/plans/let-s-plan-a-way-ethereal-sonnet.md` (lemniscatic-replacement investigation)

This protocol is **pre-registered before measurement** per CLAUDE.md epistemic discipline. The git tag `preregister-lemniscatic-v1` will be applied at commit, BEFORE any engine code extension or production run.

---

## 1 · Why this protocol exists

The AUDIT companion document reduces the lemniscatic-replacement investigation to **one load-bearing engine measurement**: the FTD lattice horizon area scaling. The existing benchmark `engine/tests/benchmark_black_hole_thermo.cpp` (line 245) hardcodes $A = 4\pi r_h^2$ as an assumption — it never directly measures area. Direct lattice measurement (voxel-counting at the half-max-latency shell) would test whether the lattice horizon is structurally sphere-symmetric (4π) or lemniscatic-symmetric (4ϖ, 4G*, or another candidate).

This is the cleanest single test of the user's investigation question: "is G* a Gaussian / lemniscatic replacement for the sphere in foundational physics formulas?"

**Why pre-register:** all four candidate values {4π, 4ϖ, 4G*, G*²·π/2} are mathematically real and distinguishable at the 5% level. Picking the candidate that matches AFTER seeing the measurement would be exactly the post-hoc-fit failure mode CLAUDE.md prohibits. Pre-registration locks the prediction matrix and falsifier before the engine runs.

---

## 2 · Pre-registered measurements

### D1 — Direct lattice horizon area

**Setup:** identical to existing `benchmark_horizon_area` (BH2). For each cluster_radius cr ∈ {2, 3, 4, 5}, build a spherical mass cluster, run `ticks` evolution steps with gravity ON / latency_field ON / forces OFF / movement OFF.

**Standard measurement (existing):** measure $r_h$ as outermost radius where averaged-shell latency $\bar L(r) > \tau \cdot L_{\text{peak}}$ for threshold $\tau = 0.5$. Compute $A_{\text{assumed}} = 4\pi r_h^2$.

**New measurement (D1):** count voxels in the 3D shell $r \in [r_h - \delta, r_h + \delta]$ with $\delta = 0.5$ whose individual latency satisfies $|\mathcal{L}_i - \tau \cdot L_{\text{peak}}| < \epsilon \cdot L_{\text{peak}}$ for $\epsilon = 0.05$. Multiply by per-voxel cross-sectional area $a^2 = 1$ (in lattice units) → gives $A_{\text{actual}}$.

**Quantity reported:** $A_{\text{actual}} / r_h^2$ across cluster_radius values, with bootstrap stderr from ≥3 seeds per cluster_radius.

### D2 — Surface-gravity coefficient

**Setup:** same as D1; reuse the latency profile $\bar L(r)$ already measured.

**Standard measurement:** estimate the surface gravity $\kappa = c^2 \cdot |d\mathcal{L}/dr|_{r=r_h}$ via finite differences. In geometrized units ($G = c = 1$), the Hawking T is $T_H = \kappa / (2\pi)$, so $T_H \cdot M = \kappa \cdot M / (2\pi)$.

**Quantity reported:** the empirical coefficient $C_T = (\kappa \cdot M)$ across mass values. In standard physics, $\kappa = c^4/(4GM)$ so $\kappa \cdot M$ is independent of M. The dimensionless quantity $T \cdot M = \kappa M / (2\pi)$ is the testable; its inverse is $1/(2\pi)$ for a round-circle Euclidean argument, $1/(2\varpi)$ for a lemniscate-circumference argument, $1/(2G^*)$ for a G*-bridge argument.

---

## 3 · Pre-registered prediction matrix

### D1 — A_actual / r_h²

| Reading | Predicted A / r_h² | Distance from standard |
|---|---|---|
| **Standard sphere (4π)** | 12.566 | 0.0% (baseline) |
| **PF Atlas decomposition (16·PF)** | 12.566 | 0.0% (numerically identical to standard) |
| **Candidate A: 4ϖ** | 10.488 | -16.6% |
| **Candidate B: 4G*** | 11.835 | -5.8% |
| **Candidate C: G*²·π/2** | 13.749 | +9.4% |

### D2 — Surface-gravity coefficient (T·M = const / X)

| Reading | Predicted T·M | Distance from standard |
|---|---|---|
| **Standard (1/(2π))** | 0.1592 | 0.0% (baseline) |
| **PF Atlas** | 0.1592 | 0.0% |
| **Candidate A: 1/(2ϖ)** | 0.1907 | +19.8% |
| **Candidate B: 1/(2G*)** | 0.1690 | +6.2% |
| **Candidate C: 1/(G*²)** | 0.1142 | -28.2% |

(Note: T·M coefficient depends on units convention. In the lattice's natural units where mass is in K_B units and lengths in lattice spacing, the absolute value of T·M is engine-convention-dependent. The **scaling with M** and the **ratio of D2 to D1 coefficients** are the unit-independent quantities; falsifier focuses on those.)

---

## 4 · Pre-registered falsifier (mandatory)

**For D1 (load-bearing):**

- **Outcome PASS-Standard:** measured $A_{\text{actual}} / r_h^2$ falls within ±5% of 4π = 12.566 across all 4 cluster_radii. Conclusion: **lattice horizon is sphere-symmetric**; lemniscatic-replacement closes negative for horizon area; downstream formulas (§§1.1, 1.5 of the AUDIT) follow.

- **Outcome PASS-Candidate A/B/C:** measured value falls within ±5% of one of {4ϖ, 4G*, G*²·π/2} across all cluster_radii, AND is >5σ separated from 4π. Conclusion: **structural finding for that candidate**; downstream formulas need re-evaluation.

- **Outcome INCONCLUSIVE:** measured value lands outside all four predictions, OR varies non-monotonically across cluster_radii by more than 10%. Conclusion: lattice anisotropy or finite-size effect dominates; need larger L or different parameter regime.

**For D2 (secondary check):**

- D2 outcome should be CONSISTENT with D1: if D1 picks Candidate A (4ϖ), D2 should pick 1/(2ϖ); same for B and C. **Internal consistency between D1 and D2 is itself a sanity check** on the candidate selection.

- If D1 and D2 pick DIFFERENT candidates, the lemniscatic-replacement hypothesis is internally inconsistent at the lattice level; report as "no clean ϖ-native replacement structure at L=64" — a structurally informative null.

---

## 5 · Pre-registered campaign parameters

- **Lattice size L:** 64 (matches existing benchmark default; gives r_h ∈ [2, 5] with adequate shell statistics)
- **Cluster radii:** {2, 3, 4, 5} (matches existing benchmark sweep)
- **Ticks:** 200 (allows latency field to relax)
- **Seeds:** 5 per cluster_radius (provides bootstrap stderr)
- **GPU:** mandatory per CLAUDE.md (`engine/build_wsl/`)
- **Output dir:** `engine/results/lemniscatic_replacement_2026-04-27/`
- **Output schema:**
  - `meta.json`: campaign metadata + per-cluster summary + four-candidate verdict
  - `per_cluster.csv`: cluster_radius, seed, r_h, peak_latency, A_assumed, A_actual, voxel_count_at_shell, A_ratio = A_actual/r_h², kappa_horizon, T_M_product
  - `verdict.csv`: one row per candidate with summary stats and outcome verdict

---

## 6 · Anti-targets (explicit, copied from AUDIT §0)

This protocol **WILL NOT**:

- Adjust the prediction matrix after seeing measurement results
- Promote any candidate to [SELECTION] or above without engine measurement landing within ±5% AND being >5σ separated from standard
- Conflate D1 and D2 outcomes; each is reported independently
- Treat the existing PF Atlas as wrong; its [SELECTION] decomposition stays valid as parallel reading
- Edit any other LEDGER row, AUDIT doc, or paper draft based on this measurement until the ANALYSIS document is committed

---

## 7 · Implementation checklist (before running production)

1. [x] AUDIT_LEMNISCATIC_SPHERE_REPLACEMENT.md committed
2. [x] PROTOCOL_LEMNISCATIC_REPLACEMENT.md committed (this file)
3. [ ] `git tag preregister-lemniscatic-v1` applied — **MANDATORY GATE**
4. [ ] Tag pushed to origin
5. [ ] `benchmark_black_hole_thermo.cpp` extended with `--lemniscatic-mode` flag (or new function `benchmark_horizon_area_lemniscatic`)
6. [ ] Smoke build at L=16 confirms compile + per_cluster.csv emission
7. [ ] Production run at L=64, ticks=200, 5 seeds, cluster_radii {2,3,4,5} on RTX 5090
8. [ ] ANALYSIS_LEMNISCATIC_REPLACEMENT.md written with measurement-vs-prediction table

Steps 1-4 are done in this commit. Steps 5-8 follow.

---

## 8 · Single-line summary

**Pre-registered horizon-area measurement on the FTD lattice. Existing benchmark assumes $A = 4\pi r_h^2$; this protocol measures $A_{\text{actual}}$ directly via voxel-counting at the half-max-latency shell. Four candidate predictions for $A/r_h^2$: 4π = 12.566 (standard sphere); 4ϖ = 10.488; 4G* = 11.835; G*²·π/2 = 13.749. Falsifier: ±5% of standard across all cluster_radii closes lemniscatic-replacement negative; ±5% of one candidate (with >5σ separation from standard) is structural finding. D2 surface-gravity coefficient as internal consistency check. L=64, 5 seeds × 4 cluster_radii, ~1-2 GPU hours on RTX 5090. Engine arbitrates; ANALYSIS doc post-measurement.**
