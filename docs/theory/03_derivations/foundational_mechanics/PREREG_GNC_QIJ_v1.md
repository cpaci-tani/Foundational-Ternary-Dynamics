# PRE-REGISTRATION — The GNC-w discriminator: Q_ij on locked, Gauss-dressed engine clusters (FTD-0349 §9)

**Status:** PROTOCOL — to be hash-locked (SHA256 + git tag `preregister-gnc-qij-v1`) **before** the canonical measurement run.
**Date:** 2026-07-02 · **LEDGER (on verdict):** row minted by the controller · **Arc:** FTD-0349 cluster-inertia reduction, the §9 v2 measurement, as refined by FTD-0354.
**Frozen artifact (§1):** `engine/tests/campaign_gnc_qij.cpp` — SHA256 to be recorded at the lock commit.
**Read first:** [`DERIV_CLUSTER_COLLECTIVE_COORDINATE_v1.md`](DERIV_CLUSTER_COLLECTIVE_COORDINATE_v1.md) (FTD-0349) §2–§5, §9; [`LEMMA_GNC_RIGIDITY.md`](LEMMA_GNC_RIGIDITY.md) (FTD-0354) §4.4, §5.2.

---

## 0 · Purpose and honesty ceiling

FTD-0349 reduced the imposed cluster-inertia law (FTD-0250, `a_COM = F/(N·M_REST)`) to a single load-bearing gap: the **Gradient-Normalization Condition**. The summed form,

> **GNC-w:**  Σ_{x∈members} (Δ⁺_i J_a)(x)·(Δ⁺_j J_a)(x) = N·K_B²·δ_ij,

is exactly what the Newtonian-limit reduction needs at O(V²). Nothing in the action or the Gauss constraint forces it; the two framework-pinned profiles fail it in different ways; GNC-satisfying textures exist (FTD-0349 §7, uniqueness and the improper stratum in FTD-0354 §4). Whether **real engine cluster profiles** realize GNC-w is **[OPEN]** — that is the only question this measurement addresses.

**Scope limits, frozen up front (FTD-0354 reading aids):**

1. **Q_ij gates the SUMMED form GNC-w only.** GNC-w is necessary-not-sufficient for pointwise GNC-s (FTD-0354 §4.4, the two-site counterexample): a Q ≈ δ result licenses the Newtonian-limit step, NOT the full γ_FTD resummation.
2. **A Q ≈ δ result licenses no inference to the affine texture family** (FTD-0354 §5.2 — divergence-free non-affine GNC-s folds exist on the lattice).
3. This campaign measures **constructed** clusters (injected, locked, zero-velocity, Gauss-dressed, relaxed under the native wave+coupling+damping dynamics), not post-genesis clusters. The construction is stated in §2 and is part of the frozen protocol; a post-genesis replication is a possible v2, not this measurement.

**Zero promotions under any outcome.** FTD-0110 and FTD-0250 move **only** via the outcomes stated in §4, and the GNC-CONSISTENT outcome itself moves nothing without a fresh adversarial pass (§4 note). x₊=1/α stays [SMC]; MC-T4.3 stays [FOUNDATIONAL OBSTRUCTION]; the clock hypothesis (FTD-0208) stays [AXIOM]; no α is derived anywhere; the golden hash `0xb604d81a3d79366e` is untouched (the instrument is read-only over public API and runs no default-ON toggle off-profile).

---

## 1 · Instrument

`engine/tests/campaign_gnc_qij.cpp`, CPU (`force_cpu()`), deterministic **seed 42** (pinned for provenance; the frozen toggle set consumes no RNG). Registered as CTest `campaign_gnc_qij` (the no-arg invocation is a CI smoke, NOT a measurement). Canonical run:

```
OMP_NUM_THREADS=1 engine/build_wsl/campaign_gnc_qij --sweep --output-dir engine/results/gnc_qij
```

CSV of record: `engine/results/gnc_qij/gnc_qij_v1.csv`.

## 2 · The measurement (frozen protocol)

**Engine config.** Toggles ON: `wave_propagation`, `coupling`, `gauss_projection`, `damping` (with `selective_damping` OFF so vacuum damps and transients relax). Toggles OFF: `genesis` (cluster integrity — exactly the constructed N members), `forces`, `movement`, `dual_substrate`, `langevin`, everything else. SOR iterations 150. dt = 1 (default integrator; this instrument does not use the E1 toggle).

**Clusters.** Same-sign (+1), `locked = true`, zero-velocity, injected with zero flux (the dressing develops dynamically). Frozen geometry tables, centered in the box:

| N | cube | rod | L-shape |
|---|------|-----|---------|
| 8 | 2×2×2 | 8×1×1 | 1×1 arms, 4 + 4 sites |
| 27 | 3×3×3 | 27×1×1 | 1×1 arms, 14 + 13 sites |
| 64 | 4×4×4 | 16×2×2 | 2×2 arms, 32 + 32 sites |

Lattices L ∈ {32, 48}. (The 1×1×N rods and thin L-arms fit both lattices; the N=64 rod and L-shape use 2×2 cross-sections so no arm approaches the box size.)

**Equilibration (gate E1).** Advance ticks; compare consecutive 64-tick window means of S_m(t) = Σ_members Σ|Δ⁺J|²; converged when the relative change < 1e-6; cap 20 000 ticks. Not converged ⇒ that row is INVALID.

**Observable.** Read-only, forward differences with periodic wrap, D_ai(x) = J_a(x+e_i) − J_a(x):

> Q_ij(support) = (1/(N·K_B²)) · Σ_{x∈support} Σ_a D_ai(x)·D_aj(x),

**time-averaged over 256 ticks** (one sample per tick) after equilibration, reported for **two supports separately**: (m) the N member sites; (s) the dressing shell = non-member sites within Chebyshev (Moore) distance ≤ 3 of any member. Plus the all-site raw trace Σ_allsites Σ|Δ⁺J|² as the FTD-0349 Eq. 4 identity channel (for the minimal Coulomb dressing this equals N·q²·(1−N/L³) **exactly**, q = 1 lattice charge unit).

**Per-row outputs:** full Q matrix (6 components) per support; trace; anisotropy = max|off-diag|/trace; eigenvalue spread = (λ_max−λ_min)/(trace/3); all-site Q-trace and raw trace; the Coulomb Eq.-4 prediction; equilibration ticks; Gauss residual; integrity flag.

## 3 · Gates (all must pass for a row to count)

- **E1 — equilibration:** converged per §2 within the cap.
- **E2 — Gauss residual:** `energy_audit().max_gauss_error` < 1e-6 at measurement start.
- **E3 — cluster integrity:** manifested count == N and every member still `state=+1`, `locked=true` at measurement end.
- **E4 — determinism:** the first sweep config re-run in-process is bit-identical in all 12 Q components (the runner prints `GATE_E4_DETERMINISM,PASS|FAIL`).

Any gate failure ⇒ that row INVALID; if more than a third of rows are INVALID, the run as a whole is INVALID (re-scope before re-running).

## 4 · Frozen prediction bands and outcome table

Reference values (q = 1, K_B = 0.511, so q²/K_B² = 3.8295):

| Quantity | Minimal-Coulomb dressing (FTD-0349 §4) | GNC-w (FTD-0349 §3) |
|---|---|---|
| member Q_trace | ≈ 3 × (0.39…0.46) × q²/(3K_B²)·3 ⇒ **1.2–1.4**, **N-drifting upward** (T5d band ×3: 1.18, 1.30, 1.36 at N = 8, 27, 64) | **3.0**, N-flat |
| member anisotropy (rod, 1×1) | **~87%** M_xx vs M_yy (T5e) ⇒ off-diag/eig-spread large | **isotropic** (< 5% spread) for every shape |
| all-site raw trace | **N·q²(1−N/L³)** exactly (Eq. 4) | 3N·K_B² + dressing (≥ 3N·K_B² on members alone) |

Note the all-site trace numbers are **not** the discriminator (q²/3 vs K_B² differ by only 28% at this calibration — FTD-0349 §4.1's anti-target caveat); the member-support trace (factor > 2 separation), the rod anisotropy, and the N-drift are.

**Outcome table (frozen):**

| Outcome | Condition (member support, across all valid rows) | Consequence |
|---|---|---|
| **GNC-CONSISTENT** | Q_trace/3 ∈ [0.90, 1.10] AND eig-spread < 0.15 AND rod rows isotropic like the rest AND max/min of Q_trace across N ∈ {8,27,64} < 1.15 | GNC-w is engine-real at this scope. FTD-0349's conditional chain becomes *eligible* for "FTD-0250 → [DERIVED given engine-verified GNC-w] at Newtonian order" — **only after** a fresh adversarial red-team pass and a separate LEDGER edit; this outcome alone promotes nothing. GNC-s remains open (§0 scope limit 1). |
| **COULOMB-CONSISTENT** | Q_trace/3 ∈ [0.30, 0.60] with monotone N-drift AND rod eig-spread > 0.50 AND all-site raw trace within 10% of N·q²(1−N/L³) | The wall is confirmed: the engine's relaxed dressed profile is the minimal Coulomb dressing; FTD-0250 stays **[IMPOSED]**, and GNC joins the clock hypothesis as a named imported type (modulus/argument bookkeeping per FTD-0349 §9). |
| **NEITHER** | Valid rows fall outside both bands, or bands are shape-inconsistent (e.g. cubes GNC-like, rods Coulomb-like) | Genuinely new information: the engine profile is neither pinned texture nor minimal dressing. Record as [MEASURED — UNDERDETERMINED]; design a v2 with the specific structure found; **no tag moves**. |
| **INVALID** | Any §3 gate pattern fails per §3 | Re-scope; no tag moves; postmortem before any re-run (v1 mass-gap precedent). |

**UNDERDETERMINED conditions, explicit:** rows that pass all gates but sit between the bands (member Q_trace/3 ∈ (0.60, 0.90) or (1.10, ∞) without the GNC isotropy signature) adjudicate as NEITHER, not as a partial match to either prediction. No interpolation, no post-hoc band widening.

## 5 · Banned moves

1. Do NOT read the all-site trace proximity (q²/3 vs K_B², 28%) as structure — FTD-0349 §4.1's anti-target caveat is binding.
2. Do NOT infer GNC-s or the affine texture family from a GNC-CONSISTENT outcome (FTD-0354 §4.4/§5.2).
3. Do NOT tune the equilibration protocol, damping profile, or geometry tables after seeing data; they are frozen in §2.
4. Do NOT promote FTD-0110/FTD-0250 directly from this run under any outcome — the GNC-CONSISTENT consequence is *eligibility* for a separately red-teamed promotion, nothing more.
5. Do NOT run GPU for the verdict — CPU is canonical (WSL2 binary, `OMP_NUM_THREADS=1`).

## 6 · Priors

Pre-registered priors: COULOMB-CONSISTENT ~55% (the damped relaxation fixed point is plausibly the minimal dressing), NEITHER ~25% (the live coupling source g_c·∇s adds boundary structure the §4 idealization lacks), INVALID ~15% (equilibration may limit-cycle under the wave+coupling drive), GNC-CONSISTENT ~5% (nothing forces the texture; FTD-0349 §7 shows only existence).

## 7 · Hash-lock

`campaign_gnc_qij.cpp` SHA256 recorded and git-tagged `preregister-gnc-qij-v1` at the lock commit, **before** any `--sweep` invocation (only the no-arg CI smoke, which measures an L=16 cube and produces no §4 band reading, may run pre-lock). Run-of-record, analysis, and verdict go in a separate `ANALYSIS_GNC_QIJ_v1.md` after the lock.
