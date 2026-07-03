# ANALYSIS — GNC-w discriminator (Q_ij on locked clusters), v1: INVALID (instrument defect; re-scope to v2)

**Tag:** `[MEASURED — INVALID / re-scope]`. **LEDGER id:** FTD-0363.
**Pre-registration (hash-locked before the run):** `PREREG_GNC_QIJ_v1.md`, git tag `preregister-gnc-qij-v1`, lock commit `ec48707f`.
**Instrument:** `engine/tests/campaign_gnc_qij.cpp`. **Data of record:** `engine/results/gnc_qij/gnc_qij_v1.csv` (18 rows, all `converged=1`, `integrity=1`, `GATE_E4_DETERMINISM=PASS`).
**Adjudicates:** FTD-0349 §9 (the GNC-w engine question) / FTD-0250 cluster inertia / FTD-0110.

---

## §0 · Verdict

Do real locked, Gauss-dressed engine clusters realize the **Gradient-Normalization Condition** GNC-w (`Σ_members ∂ᵢJ_a ∂ⱼJ_a = N·K_B²·δ_ij`)? **Undetermined — this run is INVALID.** All 18 rows fail the frozen **E2 Gauss-residual gate** (`energy_audit().max_gauss_error < 1e-6`): the measured residual is `0.041–0.154` on every row, 4–5 orders of magnitude over threshold. Per §3, 18/18 > one-third ⇒ the run as a whole is INVALID. Per §4-INVALID: **re-scope; no tag moves; postmortem before re-run.**

**FTD-0110 and FTD-0250 tags UNCHANGED; GNC-w stays `[OPEN]`.** x₊=1/α `[SMC]`; MC-T4.3 `[FOUNDATIONAL OBSTRUCTION]`; clock hypothesis (FTD-0208) `[AXIOM]`; no α derived; golden hash `0xb604d81a3d79366e` untouched (read-only instrument, no default-ON toggle run off-profile).

This mirrors the v1 mass-gap precedent (FTD-0333): a frozen gate turned out unmeetable under the instrument's own conditions. The honest response is INVALID → diagnosed → v2, not a post-hoc reinterpretation of the gate (a §5 banned move).

## §1 · The gate failure (adjudicated strictly)

`gauss_max_err` (CSV col 6) **is** the E2 quantity — the instrument writes `rb.energy_audit().max_gauss_error` verbatim. Across all 18 rows it spans `[0.0413, 0.1543]`, monotone-increasing with N within each geometry (cube 0.049→0.103→0.154) and L-independent to 4 sig figs (L=32 ≈ L=48). The `converged=1` and `GATE_E4_DETERMINISM=PASS` flags discharge only E1 (equilibration) and E4 (determinism); E2 and E3 are independent, and the runner records E2 as a scalar without auto-invalidating rows — so adjudication falls to this analysis, which applies the frozen §3 threshold literally. **E2 fails 18/18 ⇒ INVALID.**

## §2 · Postmortem — why E2 is unreachable here (two candidate mechanisms, one v2 fix)

A four-lens adversarial verification (independent gate-literalist, C++ instrument-forensics, physics/arithmetic, protocol root-cause; all four recorded `supports_any_promotion = false`) converged on "INVALID / zero-promotion" but split on the *mechanism* of the E2 failure. Both readings are recorded; they are not yet distinguished, and the verdict does not depend on which dominates:

- **Measurement-stencil / adjoint mismatch (instrument-forensics).** `max_gauss_error` measures the residual with a **central-difference divergence** (`(J₊−J₋)·0.5` per axis), but `gauss_project`'s SOR inverts a **compact 18-point Laplacian** (INV3 face + INV6 edge) and corrects flux via a central-difference gradient. `div_central ∘ grad_central ≠` the compact 18-pt Laplacian the solver drives to zero — so at the sharp locked `+1` cluster boundary (order-1 flux jump across one voxel) the two stencils disagree at a fixed high-frequency floor of exactly the observed `O(10⁻²–10⁻¹)`, L-independent and iteration-insensitive. On this reading the constraint **is** satisfied to the solver's own stencil tolerance, and the `1e-6` threshold on this particular metric is unmeetable by construction — even for a frozen, fully-converged field.
- **Driven-steady-state under live coupling (protocol root-cause).** With `coupling` ON, the source `g_c·∇s` re-injects longitudinal divergence every tick, and only 150 warm-started SOR sweeps run before the next injection; injection-per-tick balanced against finite-removal-per-tick pins the residual at a `~0.04–0.15` dynamical fixed point. On this reading a **frozen** source could be hard-projected to `1e-6` by a standard static Poisson solve; the failure is the in-loop live-drive, not the stencil.

These differ on whether a source-frozen hard projection would reach `1e-6`: the mismatch reading says no (unless the measurement stencil is also matched), the drive reading says yes. **The v2 re-spec is robust to both:** measure Q on a field whose source is frozen at measurement time and hard-projected with a **stencil-matched** solver — precisely the Phase-F matched-stencil CG Poisson solve that reaches a `~1e-8` Ward floor (measurement stencil = solve stencil, so no adjoint gap). Re-spec E2 as a *post-freeze hard-projection* tolerance, not an in-loop live tolerance.

## §3 · The Q data is not a trustworthy band-reading either (secondary finding)

Even setting E2 aside, the member Q_trace does not deliver a clean band verdict. Measured member `Q_trace/3 = 0.0018–0.016` — **~60–500× below the GNC prediction (3.0) and ~25–230× below the minimal-Coulomb prediction (1.2–1.4)** — and the all-site raw trace is only 4–12% of the Coulomb Eq.4 value (the frozen Coulomb clause needs within 10% of it). By the letter of §4 this sits in **NEITHER** territory (rows outside both bands ⇒ `[MEASURED — UNDERDETERMINED]`, no tag moves). But three lenses independently read the ~100× shortfall as a **field-under-development artifact**, not information: flux is injected at **zero** and must build the dressing via `g_c·∇s` against `damping` ON (`selective_damping` OFF, so vacuum bleeds too), so the equilibrium dressing saturates far below the fully-developed minimal-Coulomb profile the §4 bands assume. So the honest statement is not "the physics is NEITHER" but "**the run is INVALID and its Q columns are not a trustworthy band-reading**" — under-projected (or stencil-mis-measured) *and* under-developed. Both defects point the same way: **zero promotions**, and a v2 that develops the dressing self-consistently.

## §4 · v2 re-spec (frozen design for the next pre-registration)

1. **Freeze + hard-project before measuring.** Equilibrate under the live §2 dynamics, then at measurement time snapshot the source (`s` + coupling contribution), freeze it, and hard-project with a **matched-stencil CG Poisson solve** to `max_gauss_error < 1e-8` measured on the *same* stencil the solver inverts. Re-spec E2 accordingly.
2. **Develop the dressing.** Fix the ~100× shortfall by one of: `selective_damping` ON (spare the vacuum); initialize flux with the minimal-Coulomb dressing and relax from there rather than from zero; or measure on a **genesis-grown** self-consistent cluster (the post-genesis replication flagged as a possible v2 in PREREG §0 scope-limit 3).
3. **Keep everything else frozen** (geometry tables, N-grid, member/shell supports, seed 42) so v2 remains a clean discriminator, and pre-register with a hash-lock before the canonical `--sweep`.

## §5 · Provenance

Instrument + pre-reg hash-locked (`preregister-gnc-qij-v1`, `ec48707f`) before the canonical `--sweep`; bands/gates frozen and applied literally (no post-hoc widening — a §5 banned move). Canonical run: `OMP_NUM_THREADS=1`, `force_cpu()`, seed 42, SOR 150, 256-tick measurement window, RTX-5090/WSL2 host (CPU-bound; GPU idle). Verdict verified by a four-lens adversarial workflow (`wf_038262a2-f9c`); all four lenses recorded zero-promotion; three of four adjudicated INVALID on the literal gate, the fourth judged the E2 failure a spurious stencil artifact but adjudicated no physics band and licensed no promotion.
