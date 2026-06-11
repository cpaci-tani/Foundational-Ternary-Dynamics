# Analysis — FTD-0271: Given a Rest-Mass Clock, FTD is a Single-Particle Pilot-Wave Theory (CONDITIONAL)

**Tag:** `[CONDITIONAL — DERIVED-GIVEN-IMPOSED-INPUT]`
**Date:** 2026-06-11
**LEDGER row:** FTD-0271
**Pre-registration:** `PREREG_DE_BROGLIE_CLOCK_v1.md`, tag `preregister-de-broglie-clock-v1`, lock commit `3df173b1`
**Artifact:** `scripts/exploration/derive_de_broglie_clock_2026-06-11.py` (SHA `a55b5c36`); run of record `scripts/exploration/results/de_broglie_clock_2026-06-11.csv`

---

## 0 · Verdict

Adding a Klein-Gordon rest-mass clock `∂²J/∂t² = c²∇²J − ω₀²J` (`ω₀ ∝ M_REST`) to FTD's natively-massless flux turns the substrate into a **single-particle pilot-wave theory**: a moving cluster carries a **de Broglie matter wave** (`λ ∝ 1/v`) and its non-relativistic envelope obeys the **Schrödinger equation**. This is the exact quadratic sector FTD-0270 measured as the only door out of its dispersion boundary. **The result is CONDITIONAL**: the clock is an imposed input (A0), and the de Broglie/Schrödinger consequences are textbook Klein-Gordon — so this is a derivation *given* the clock, not an unconditional FTD result. The genuinely non-circular content is the A5 argument below (FTD's own proper-time supplies the *covariant clock rate*) and the open guidance test E.

## 1 · A0 — the clock is imposed (gate, on record)

The flux dynamics is `delta_j = c²·Lap18(J) + G_C·∇s + G_C·∇×(s·v)` (`phase_read.cpp:105–164`) — **no restoring term**. Genesis applies a dissipative *drain* (`v.flux *= max(0,1−K_GENESIS/|J|)`), not `−ω₀²J`. FTD-0251's native clock winds at `ω(k)`, **zero at k=0** (massless). So FTD has **no rest-mass oscillation**; `ω₀ ∝ M_REST` must be **added**. Verdict: `ω₀∝M_REST is [IMPOSED/SELECTION], not [FORCED]`.

## 2 · B core — measured (run of record)

Both gates passed: G-1 operator `|eig(L18)−M(k)| = 1.33e-15`; **G-2 causal control** `ω₀=0` reproduces FTD-0270 (`s = 0.944`, linear).

**D1 — Schrödinger envelope.** Box ground mode vs L, energy above rest `E_env = ω₁−ω₀`:

| L | 12 | 16 | 20 | 24 | 32 |
|---|---|---|---|---|---|
| ω₁ (massless control) | 0.2399 | 0.1840 | 0.1492 | 0.1254 | 0.0951 |
| E_env (KG envelope) | 0.05458 | 0.03279 | 0.02178 | 0.01549 | 0.00896 |

`E_env ∝ L^−1.843 ± 0.017` → **s_env ≈ 2 (quadratic, Schrödinger)** vs the massless `s = 0.944` (linear). The √(−M) is what makes the *carrier* linear; subtracting the rest frequency leaves the *envelope* quadratic — the Schrödinger sector. (Numerically `E_env ∝ (−M) ∝` the FTD-0270 Schrödinger-diagnostic `E₁ ∝ L^−1.887`, as the NR limit requires.) `[DERIVED FROM the imposed clock]`.

**D2 — de Broglie.** Moving KG packets, `λ` vs group velocity:

| carrier k₀ | 0.10 | 0.16 | 0.24 | 0.34 | 0.46 |
|---|---|---|---|---|---|
| v_group | 0.065 | 0.104 | 0.153 | 0.207 | 0.262 |
| λ | 64.0 | 39.4 | 26.9 | 18.3 | 13.5 |

`λ ∝ v^−1.113` → **de Broglie `λ ∝ 1/v` CONFIRMED** (vs FTD-0270's massless `r = 0`). The matter wave is real once the clock exists. `[DERIVED FROM the imposed clock]`.

## 3 · The honest reading (and the laundering check)

Schrödinger and de Broglie are **analytic consequences of any Klein-Gordon field** (de Broglie 1924, Schrödinger 1926). Adding `−ω₀²J` and recovering them is *circular as physics*; the only `[DERIVED]` content here is **lattice correctness** — that FTD's exact 18-pt operator carries these consequences faithfully. The claim ceiling is therefore: **"FTD is a single-particle pilot-wave theory GIVEN an imposed rest-mass clock."** That conditional is not removable by B.

## 4 · A5 — what makes it more than an import (the FTD-native half)

The imposed part is smaller than it looks. A de Broglie clock needs two things: a **rest frequency** `ω₀` and a **relativistically covariant clock rate** (a moving clock must red-shift as `√(1−v²)` — that is what makes the phase wave come out `λ = h/p` in every frame). FTD **already supplies the second, natively and measured**: `accumulate_proper_time` (`transmutation_phases.cpp`) integrates `dτ/dt = √(f²−v²)/√f` per particle, and **FTD-0252 measured (IR-confirmed) that this clock dilates as `√(1−v²)`**. So the covariant clock *rate* is FTD-native machinery (FTD-0252), currently computed but discarded (read-only). Wiring the clock phase as `dφ/dt = ω₀·dτ/dt` would source the de Broglie clock's *relativistic behavior* from FTD's own proper-time, leaving **only the scalar `ω₀ ∝ M_REST` imposed**. That shrinks the import from "the whole quantum clock" to "one rest-frequency constant," and upgrades the covariant-clock half from `[IMPOSED]` toward `[SELECTION]`. (Demonstrating it in the engine — toggle + τ→phase wiring + the moving-clock red-shift test — is the next phase; the analytic case rests on the already-measured FTD-0252.)

## 5 · Status of the program & what remains OPEN

- **B (matter wave): DONE** `[CONDITIONAL — DERIVED-GIVEN-IMPOSED-INPUT]`.
- **A (engine mass-term toggle):** default-OFF `de_broglie_clock` in `phase_read.cpp` so a static cluster's flux oscillates at ω₀ (it does not today) — golden-neutral; pending.
- **A5 (proper-time → clock):** analytic case made above (FTD-0252 supplies the covariant rate); engine wiring + red-shift demonstration pending — the route from `[IMPOSED]` to `[SELECTION]`.
- **E (guidance, the heart of pilot-wave):** does the cluster follow `v ∝ ∇S`? The audits found the cluster moves by **forces, not phase** — so guidance is likely **not** currently emergent and would have to be shown or added. This is the genuinely uncertain (~40%), highest-value remaining test. `[OPEN]`.
- **The ℏ scale (inherited):** every quantity is dimensionless; `M_REST→ω₀[rad/tick]` is `[SELECTION]` — the substrate fixes the *shape* (`λ∝1/v`), never the absolute scale.

## 6 · No promotions

FTD-0013 `[SMC]`, MC-T4.3, FC-1, FTD-0270 (native massless boundary — still TRUE for unmodified FTD) — all unchanged. FTD-0271 is `[CONDITIONAL]`; it does not promote unless A5 makes the clock FTD-sourced and E confirms guidance. The honest headline is *"given a rest-mass clock, FTD's flux is a de Broglie pilot wave"* — and FTD-0252 already supplies the covariant half of that clock.
