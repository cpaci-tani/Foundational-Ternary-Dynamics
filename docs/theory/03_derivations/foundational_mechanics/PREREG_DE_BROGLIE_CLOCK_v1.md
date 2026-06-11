# PREREG — FTD-0271: The de Broglie Internal Clock (single-particle pilot-wave)

**Status:** `[PRE-REGISTRATION — design locked before the run of record]`
**Date:** 2026-06-11
**LEDGER row:** FTD-0271 (reserved)
**Git tag:** `preregister-de-broglie-clock-v1` (applied at the lock commit)

## 0 · Purpose & honesty ceiling

Test whether adding a Klein-Gordon rest-mass clock to FTD's flux turns the substrate into a single-particle **pilot-wave** theory. FTD's native flux is massless (FTD-0270 `[MEASURED — BOUNDARY]`, linear dispersion). **A0 audit (this session): the flux dynamics is `delta_j = c²·Lap + G_C·∇s` — NO restoring term — so `ω₀ ∝ M_REST` is `[IMPOSED]`, not forced.** Schrödinger + de Broglie are analytic KG consequences; confirming them after adding `−ω₀²J` is *correctness*, not an FTD discovery. **Claim ceiling: "FTD is a single-particle pilot-wave theory GIVEN an imposed rest-mass clock."** The non-circular content is A5 (can FTD's own proper-time source ω₀?) and E (does the wave guide the particle?), pursued downstream.

## 1 · Frozen artifact

| Role | Path | SHA256 |
|---|---|---|
| Analysis (B core) | `scripts/exploration/derive_de_broglie_clock_2026-06-11.py` | `a55b5c3649c415bdf263ae4b3e7fa1bee79c5fa04f325d75094267c62ec57b1a` |

Run of record: `python scripts/exploration/derive_de_broglie_clock_2026-06-11.py --box-Ls 12,16,20,24,32 --omega0 0.5 --out scripts/exploration/results/de_broglie_clock_2026-06-11.csv`. First valid run is the run of record. (A reduced 2-point smoke at `--box-Ls 10,14` was run pre-lock for code validation; the criteria below are theory-fixed by the KG dispersion, not data-derived.)

## 2 · Physics (criteria are theory-fixed)

KG dispersion `ω² = c²(−M(k)) + ω₀²`. (i) **Envelope:** `E_env = ω − ω₀ ≈ c²(−M)/(2ω₀) ∝ k²` (Schrödinger, quadratic) ⇒ box finite-size exponent `s_env ≈ 2` (vs massless `s ≈ 0.94`). (ii) **de Broglie:** packet `v_group = c²k/ω ≈ c²k/ω₀` (NR) ⇒ `λ = 2π/k ∝ 1/v` ⇒ exponent `r ≈ 1` (vs massless `r ≈ 0`).

## 3 · Gates (must pass or run INVALID)

- **G-1 operator:** `|eig(L18) − M(k)| < 1e-10`.
- **G-2 causal control (the load-bearing gate):** with `ω₀=0` the massless box exponent must reproduce FTD-0270, `s ∈ [0.8, 1.2]`. The flip to `s_env≈2`/`r≈1` is only meaningful *because* the same harness gives FTD-0270's massless result. G-2 fail ⇒ harness broken, STOP.

## 4 · Frozen discriminators

- **D1 SCHRODINGER-ENVELOPE-CONFIRMED** if `s_env ∈ [1.7, 2.3]`; NULL if `s_env ∈ [0.75, 1.25]`; else AMBIGUOUS.
- **D2 DE-BROGLIE-CONFIRMED** if `r ∈ [0.75, 1.25]` (clean monotone λ(v)); FAILED if `r < 0.3` / no packet; else AMBIGUOUS.

**Verdict map:** (G-2 PASS, D1 CONFIRMED, D2 CONFIRMED) → **`[CONDITIONAL — DERIVED-GIVEN-IMPOSED-INPUT]`** "GIVEN the rest-mass clock, FTD yields de Broglie matter waves + a single-particle Schrödinger envelope." (D1 NULL / D2 FAILED) → `[NULL/BOUNDARY]`. (G-2 FAIL) → `[INVALID]`.

## 5 · Tags & priors

- `ω₀∝M_REST`: `[IMPOSED/SELECTION]` (A0: not forced; native rest freq = 0). Envelope/de Broglie: `[DERIVED FROM the imposed input]` = textbook KG, conditional. Lattice reproduction: `[DERIVED — correctness]`. M_REST→ω₀ scale: `[SELECTION]` (no ℏ).
- **[OPEN]:** is ω₀ forced (no, expected) / can proper-time τ source it (A5) / does it guide the cluster (E)?
- Priors (disclosed): D1+D2 CONFIRMED ~90% (textbook KG — weak evidence for an FTD claim); the real uncertainty is A0 (imposed ~85%), A5 (~60%), E (~40%).

## 6 · Scope

Pre-registers the **B core** only (the matter-wave dispersion/de Broglie/Schrödinger-envelope measurement). The engine mass-term toggle (A, golden-neutral), the proper-time→clock wiring (A5), and the guidance test (E) are downstream phases of the same FTD-0271 arc, each with its own gate. Nothing promoted above `[CONDITIONAL]`.
