# EXPLR — FTD-0110 Mechanism α (multi-block irrep leakage): quantified and CLOSED as the drift mechanism

**Document type:** Exploratory quantitative test (closes one mechanism class; redirects the queued calculation)
**Status:** `[VERIFIED]` (the 2/3 leakage lemma) + `[CLOSED NEGATIVE — for the leakage-aggregation family of Mechanism α as the k(A)-drift mechanism]` + `[METHODOLOGICAL]` (log-vs-power undecidability) + `[OBSERVATION]` (the Langevin knee) + `[OPEN — redirected]` (the bridge itself). **Nothing promoted; FTD-0110's tags unchanged (linear [DERIVED]; nonlinear bridge [OPEN]; cluster↔mass identification [SMC]).**
**Date:** 2026-06-09
**LEDGER row:** FTD-0259
**Runner:** [`scripts/exploration/explore_ftd0110_mechanism_alpha_leakage.py`](../../../scripts/exploration/explore_ftd0110_mechanism_alpha_leakage.py) (pre-stated predictions P1–P3 in the docstring, written before compute — in-session pre-registration)
**Depends on:** [`EXPLR_FTD_0110_NONLINEAR_BRIDGE_ANALYSIS.md`](EXPLR_FTD_0110_NONLINEAR_BRIDGE_ANALYSIS.md) (the mechanism menu), [`DERIV_K_FROM_OH_A1G_MULTIPLICITY.md`](DERIV_K_FROM_OH_A1G_MULTIPLICITY.md) (linear theorem), [`../../07_assessment/audits/AUDIT_FTD0110_2026-05-27_RESOLUTION.md`](../../07_assessment/audits/AUDIT_FTD0110_2026-05-27_RESOLUTION.md) (the reverted §6 closure; canonical honest position), [`../../02_foundations/FOUND_LATTICE_SPACING_GAUGE_FREEDOM.md`](../../02_foundations/FOUND_LATTICE_SPACING_GAUGE_FREEDOM.md) §6.5 (the 11-point k(A) table)

---

## 0 · One-paragraph result

Mechanism α — the EXPLR menu's "likely dominant" candidate for the empirical k(A) drift, and the route the 2026-05-27 audit pointed at for "the actual ~1-week perturbation calculation" — is here **computed and closed as the drift mechanism**. The per-block non-A₁g leakage of the lattice Green profile is evaluated exactly: the continuum lemma `λ(r) → (18/27)/r² = (2/3)/r²` is **verified** (λ·r² = 0.6642 at r = 24, monotone convergence; the 18/27 is pure 27-block geometry — the T₁ᵤ gradient term). Feeding the exact λ(r) into the parameter-free shell-dephasing model `k(A) = ¼·exp(−Σ_{r=2}^{R(A)} λ(r))` **fails the pre-stated success criterion decisively** (4/11 points within the empirical σ = 0.018 vs the pre-stated bar ≥ 9/11; RMS 0.039 ≈ 2.2σ) — and the failure is family-wide: because the computed λ(r) is steep (∝ 1/r²), *any* aggregation rule that produces the observed ~18 % drift at A ≈ 118 necessarily over-drifts at A ≈ 30, where the data sits at its *highest* values (0.245–0.262). A structural diagnosis explains why the picture was wrong from the start: **genesis thresholds on `|J|` — a basis-free quantity — so energy re-projected into non-A₁g irreps of off-center blocks is not lost to harvest.** Two bonus results: the log and small-power drift forms are **operationally indistinguishable** (they differ by 0.0008 at A = 2000 — "is it really log?" is unanswerable by curve-fitting), and the drift *onset* matches the Langevin thermal crossover `A* = √(L³·T_L) = 12.8` computed from engine constants with no tuning — elevating Mechanism γ, and making the **thermostat-OFF amplitude re-sweep** the decisive next experiment.

---

## 1 · The leakage lemma (P1) — `[VERIFIED]`

For a block centered at distance r from the injection, the local Green profile is `G(r) + δ·∇G + O(∂²G)`. The constant term is A₁g; the gradient term is pure T₁ᵤ with squared norm `|∇G|²·Σ_δ δ_z² = 18·|∇G|²` against the constant's `27·G²`. With `G ∼ C/r`:

```
λ(r) ≡ 1 − ‖P_{A1g} G|_block‖²/‖G|_block‖² → (18/27)·(G′/G)² = (2/3)/r²
```

Continuum check (exact 1/r profile): λ·r² = 0.5843, 0.6445, 0.6567, 0.6611, **0.6642** at r = 4, 8, 12, 16, 24 → 2/3. `[VERIFIED]`
Lattice values (periodic 18-pt 2:1 stencil, L = 96, full-shell averages, blocks containing the source excluded): λ(2) = 0.1429, λ(3) = 0.0868, λ(4) = 0.0525, λ(6) = 0.0250, λ(10) = 0.0114 — the λ·r² product grows past 2/3 at large r from periodic-image contamination (the same finite-L environment the engine campaigns ran in), which only *strengthens* the §2 falsification (more leakage available, still not the observed shape). `[MEASURED — quick-check platform]`

## 2 · The parameter-free test (P2) — `[CLOSED NEGATIVE for the family]`

**Pre-stated model M1** (flux conservation in 3D: each unit shell-crossing dephases fraction λ(r) of the through-flowing harvestable energy): `k(A) = ¼·exp(−Σ_{r=2}^{R(A)} λ(r))`, `R(A) = (3A²/16π)^{1/3}`, zero free parameters. **Pre-stated bar:** ≥ 9/11 points within σ = 0.018.

| A | 2 | 10 | 15 | 20 | 28.77 | 30 | 33.05 | 50 | 62.42 | 85.70 | 117.93 |
|---|---|---|---|---|---|---|---|---|---|---|---|
| k_eng | .250 | .252 | .224 | .234 | .253 | .262 | .245 | .222 | .224 | .212 | .206 |
| k_M1 | .250 | .250 | .217 | .217 | .199 | .199 | .189 | .182 | .178 | .174 | .169 |

**Verdict: 4/11, RMS 0.039 — FALSIFIED.** The failure generalizes beyond M1's specific aggregation: fixed-fraction-per-block (~R growth), coherent amplitude accumulation (~ln²R), and front-only (λ(R), wrong sign of trend) all fail in the same direction or worse — the computed λ(r) is too front-loaded in r to reproduce a drift that is *flat through A = 10* and still *gentle at A = 30*. `[CLOSED NEGATIVE — leakage-aggregation family]`

## 3 · Why the picture was wrong — the basis-free-genesis diagnosis — `[STRUCTURAL OBSERVATION]`

Mechanism α treats energy arriving in non-A₁g irreps *of off-center blocks* as lost to the harvest. But the engine's genesis trigger is `density() = |J|` (`engine/include/ftd/voxel.h:169`; `phase_write.cpp:219/263`) — a **basis-free** scalar per voxel. Re-projecting the same field into another block's irrep basis changes no voxel's `|J|` and therefore no manifestation decision. The harvest fraction ¼ originates in the *injection-block mode-energy partition* (the linear theorem), not in per-block re-projections during growth — so irrep leakage at off-center blocks was never the right loss channel. This is the same lesson as the audit's defect 1.5 (eigenmode/orbit conflation), one level up: *projections that don't enter the dynamics don't cost energy.* The EXPLR menu's "α likely dominant" prior is reversed.

## 4 · Log vs power is operationally undecidable (P3) — `[METHODOLOGICAL]`

On the 11 points: `k = ¼(1 − 0.0257·ln(A/2))` (the doc's 0.030 re-fits to 0.0257) vs `k = ¼·(A/2)^{−0.0278}` give RMS 0.0146 vs 0.0147 — and at A = 2000 the two forms still differ by only **0.0008**. At this drift magnitude the functional-form question cannot be settled by any amplitude scan; only *mechanism-killing* experiments (a knob that switches a candidate off) and *shape features* (knee location, saturation) discriminate. Future work should stop asking "is it log?" `[METHODOLOGICAL]`

## 5 · The Langevin knee, and the decisive discriminator — `[OBSERVATION]` + `[OPEN — redirected]`

The data is **flat through A = 10** (k = 0.250, 0.252) with first drop at A = 15. The Langevin thermal crossover from engine constants — `A* = √(L³·T_L) = √(32³ × 0.005) = 12.8` (γ_L = 0.02, T_L = 0.005 per FTD-0051) — lands exactly in that onset window, with **no tuning**. The k(A) campaign carried "~3 % scatter from Langevin variance" (`FOUND_LATTICE_SPACING_GAUGE_FREEDOM.md` §6.5 — the thermostat was active in the data runs; the "Langevin T=0" line in §12.6 describes the separate WASM visualization scenarios only). Mechanism γ now holds two untuned signatures: the knee location and the sub-knee flatness. (The parameter-free friction-formation variant `k = ¼·exp(−γ_L√3·R(A))` alone scores 5/11, RMS 0.023 — disfavored as the *whole* story; the thermal-crossover variant has no closed form yet.) `[OBSERVATION — suggestive, not confirmed]`

**The redirect.** The queued "~1-week Mechanism-α projection calculation" is hereby retired (it computes a quantity the dynamics doesn't price — §3). The decisive next experiment is a **pre-registered thermostat-OFF amplitude re-sweep** (γ_L = 0, T_L = 0, otherwise the FTD-0110 protocol; plus a γ_L ∈ {0, 0.01, 0.02, 0.04} arm at fixed A = 50). Outcome tree, stated in advance: **(A)** drift vanishes/flattens → the drift is thermostat physics; the substrate-native coefficient is the linear theorem's k = ¼ *exactly*, and the nonlinear bridge reduces to "linear theorem + thermal correction" — a major simplification of FTD-0110; **(B)** drift persists unchanged → Mechanism γ dies; with α's family closed here, only β (genesis-kink, predicted power-law) and front-energetics survive; **(C)** partial → decompose by arm. Engine platform (WSL2 GPU) per the measurement-platform rule; **not run in this session — design lock first.**

## 6 · Epistemic ledger

- λ(r) lemma (2/3, gradient/T₁ᵤ): `[VERIFIED]` (continuum exact; lattice computed).
- Mechanism α leakage-aggregation family as the k(A)-drift mechanism: `[CLOSED NEGATIVE]` (pre-stated criterion; parameter-free; family-bracketed; structural diagnosis §3).
- Log-vs-power: `[METHODOLOGICAL — undecidable by form-fitting at this drift size]`.
- Langevin knee A* = 12.8 match: `[OBSERVATION]` — elevates Mechanism γ; not a confirmation.
- FTD-0110 status: **unchanged** — linear k = ¼ `[DERIVED]`; nonlinear bridge `[OPEN]` (per the 2026-05-27 audit; the stale TRACKER "CLOSED" line is corrected in this commit); cluster↔mass identification `[STRONGLY MOTIVATED CONJECTURE]`. **Nothing promoted.**
- Python is the quick-check platform here; every verdict-bearing number above is reproducible from the runner; the canonical discriminator is the engine experiment of §5.
