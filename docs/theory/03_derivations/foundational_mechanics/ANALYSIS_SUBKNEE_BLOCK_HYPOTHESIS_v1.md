# ANALYSIS — Sub-Knee Onset Mechanism (FTD-0263): GEOM-PARTIAL — the sharp 27-block reading fails its own kill-line; the onset is local, smooth, and elbowed at N ≈ 15

**Tag:** `[MEASURED — GEOM-PARTIAL]`: C1 (knee-N in the block band) **FAILED**; C2 (direction invariance) **PASSED**; C3 (sub-knee L-invariance) **PASSED 6/6**. The **sharp 27-block-boundary hypothesis is disfavored by its own pre-registered kill-line**; what survives is a *local-geometry onset* — direction-invariant, L-invariant, smooth — with the elbow at N ≈ 14.6. The sub-knee mechanism stays `[OPEN]`, now under the tightest constraint set it has had. **Nothing promoted; FTD-0110/0013/MC-T4.3 untouched.**
**Date:** 2026-06-10
**Pre-registration:** [`PREREG_SUBKNEE_BLOCK_HYPOTHESIS_v1.md`](PREREG_SUBKNEE_BLOCK_HYPOTHESIS_v1.md) (lock `5e26ac7b`, tag `preregister-subknee-block-hypothesis-v1`; priors GEOM-CONFIRMED 45 / **PARTIAL 30 — landed** / DISFAVORED 20 / UNDETERMINED 5)
**Run of record:** `engine/results/subknee_block_2026-06-10/` (37 CSVs + frozen `verdict.txt`; 147 seed-runs, 0 failures; canonical protocol; WSL2 build).
**LEDGER:** FTD-0263.

---

## 0 · One-paragraph result

The half-step fine grid resolved the onset structure the coarse FTD-0261 grid could not: the broken-power elbow sits at **knee_A = 13.5, knee_N = 14.6** (p_lo = 4.52, p_hi = 1.66, log₁₀-RMS 0.042) — **outside the pre-registered block band [19, 33]**, so C1 fired against the hypothesis. The geometry tests both passed: **C2** — body-diagonal injection reproduces the axial cluster sizes through the onset (N_diag/N_axial = 1.159 at A = 14, 0.941 at A = 16; the diagonal arm's own descriptive elbow at N ≈ 21); **C3** — the sub-knee curve is **lattice-size invariant 6/6** (L = 24 and L = 48 vs 32 at A ∈ {10, 12, 14}, all within band). The descriptive staircase table shows **no orbit staircase at all**: N̄ climbs smoothly 1.8 → 24.6 across A = 8.5 → 18 with no flats at the milestones {7, 19, 27} — growth is voxel-by-voxel accretion (Boltzmann-smeared), not shell-filling. **Verdict per the frozen map: GEOM-PARTIAL** — the onset is *local* (it cannot see the box and barely sees the injection direction) but it is **not** the 27-block boundary being filled. The framework's most attractive reading was put in front of its own kill-tests and lost the decisive one; the aesthetic-capture guard worked as designed.

## 1 · Component verdicts (numbers from the frozen analysis)

| Criterion | Result | Detail |
|---|---|---|
| **C1 knee-N ∈ [19, 33]** | **FAIL** | knee_A = 13.5, **knee_N = 14.6** (between the face-shell 7 and edge-shell 19 milestones, not at the block edge); p_lo = 4.52, p_hi = 1.66 |
| **C2 direction invariance** | **PASS** | N_diag/N_axial = 1.159 (A = 14), 0.941 (A = 16); descriptive diag elbow N ≈ 21.1 |
| **C3 sub-knee L-invariance** | **PASS 6/6** | L24/L32 and L48/L32 ratios ∈ [0.95, 1.50] at A ∈ {10, 12, 14} |

**Descriptive observations (non-verdict, per pre-reg F-3):** (i) no staircase — the fine-grid N̄(A) is smooth through every orbit milestone; (ii) a *shoulder* appears above the fitted elbow (N̄ flattens 21.6 → 24.2 → 24.6 across A = 16 → 18), hinting the onset region may carry two features rather than one — a candidate target for any future design, noted post-hoc and unweighted; (iii) **bulk-branch L-invariance**: N̄(A = 30) = 45.0 / 45.0 / 45.0 at L = 24 / 32 / 48 (exact), N̄(A = 20) = 26.3 / 27.4 / 28.7 — the current-stack law is **intrinsic/local physics, not finite-size physics**, at least for L ∈ [24, 48] and A ≤ 30. This echoes the historical stack's celebrated absolute-scale locality (FTD-0107) on the corrected engine.

## 2 · Where the mechanism question now stands

The sub-knee onset remains `[OPEN]`, but the constraint set is now sharp enough to discriminate future proposals mechanically. Any candidate must produce: **(a)** an elbow at N ≈ 15 (not 27); **(b)** direction-invariance at the few-percent-to-tens-of-percent level; **(c)** L-invariance from 24 to 48; **(d)** smooth voxel-wise growth with no orbit-shell staircase; **(e)** survival without the thermostat (the onset exists in the friction-free arm, FTD-0261); **(f)** T-independence (FTD-0261 dose arms). The naive block-filling picture fails (a) and (d). The live candidates — the genesis-kink threshold acting on the local injection envelope (β), and front-energetics — must now be evaluated against this six-point profile; β's natural prediction (manifestation radius set by where the *smooth local envelope* crosses K_GENESIS, with Boltzmann smearing erasing shell structure) is qualitatively compatible with (b)–(f) and owes a quantitative account of (a).

## 3 · Scope

Engine-level, canonical stack, stated config; the 27-block's *mathematical* roles (N_base, the A₁g linear theorem, the Moore-layer decomposition) are untouched — what failed is one proposed *engine realization*, not any spine claim.
