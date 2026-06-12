# SCOPE — The genesis-counting model: a conditional derivation of the N(A) law (FTD-0277, Arc 3)

**Tag:** `[SCOPE — design document]` (no claims; defines the target, the imposed register, and the frozen falsifier classes for the downstream pre-registration)
**Date:** 2026-06-12
**LEDGER id (reserved):** FTD-0277
**Program:** Arc 3 of the 4-arc emergent-physics forward program (the multi-session centerpiece). Arcs 1 (FTD-0275) and 2 (FTD-0276) complete.
**Owner posture (2026-06-12, on record):** *"I don't mind imposing stuff if we have the motivation and derivation of things. Our framework is cutting edge. We must be willing to do that."* — motivated `[IMPOSED]` inputs + a rigorous conditional derivation is a first-class result (the FTD-0271 `[CONDITIONAL — DERIVED-GIVEN-IMPOSED-INPUT]` pattern).

---

## §1 · Target statement

**Derive, analytically, the N(A) law of the genesis burst — GIVEN the engine's imposed
calibration constants — from the framework's own theorems.** Concretely: produce a
counting model (a recursion in *firing rank*, NOT a tick-by-tick re-simulation) that
predicts, with no per-target tuning:

- the broken-power-law **shape** (knee location band, sub- and super-knee exponents),
- the **firing geometry** (shell occupancies center/SC/FCC/BCC at sub-knee A),
- the **firing count** (~5 events at A=10, FTD-0267),
- and — **out of sample** — the **drain law** `k_eff ∝ drain^{−0.93±band}` (FTD-0276 Leg A)
  and the **γ-map direction** (knee/exponent trends of FTD-0276 Leg B).

Success lands the N(A) law at `[CONDITIONAL — DERIVED-GIVEN-IMPOSED-INPUT]`, upgrading
the bare FTD-0269 `[BOUNDARY]`. Failure (model under-determines or misses the frozen
bands) leaves the boundary as mapped and closes the counting-model route honestly.

**What distinguishes this from FTD-0269's forward model:** FTD-0269 *re-simulated* the
engine dynamics in Python (tick-by-tick) — that is a consistency check, not a derivation.
The counting model must be **analytic/semi-analytic**: an energy-budget + threshold-cascade
recursion whose every term is either a framework theorem, an axiom, or a registered
imposed input. No time stepping.

## §2 · The imposed register (each input motivated, none silently promoted)

| Input | Value | Motivation | Tag |
|---|---|---|---|
| kinetic drain | 0.5 | latent heat of manifestation (fraction of wave KE consumed at a genesis flip); measured to set k_eff linearly (`∝ 1/drain`, FTD-0276) | `[IMPOSED]` |
| Langevin friction γ | 0.02 | thermostat friction of the canonical ic1 stack; measured super-knee calibrator (FTD-0276 Leg B: γ=0.02 → knee 16, p_hi 1.81) | `[IMPOSED]` |
| coupling G_C | √α | the engine's state-flux coupling; α from the master quadratic is `[SMC]` — the *value* is imposed here, only its provenance is FTD-structural | `[IMPOSED]` (value) |
| charge_coupling | 1.0 | Gauss-law source normalization (Phase H) | `[IMPOSED]` |

## §3 · The derivation inputs (theorems/axioms/measurements the model may use freely)

| Ingredient | Status | Source |
|---|---|---|
| Lattice Poisson Green's function G_L(r) (the Gauss boost) | `[THEOREM]` | OT-1.4 / Phase G; exact symbol `M(k) = (2/3)Σcos kᵢ + (2/3)Σcos kᵢ cos kⱼ − 4` |
| Genesis threshold \|J\| > K_GENESIS = N_c·K_MANIFEST | `[AXIOM/framework]` | postulate-level manifestation rule |
| Genesis flux cost: \|J\| → \|J\| − K_GENESIS per firing | engine rule (deterministic) | `phase_write.cpp` flux drain |
| One-shot burst termination; cluster N ≈ genesis-firing count; evaporation ≈ 0 | `[MEASURED]` | FTD-0267 |
| First-order transition background (no critical scaling to exploit) | `[MEASURED]` | FTD-0272 |
| 18-pt O_h Laplacian wave spreading at c² = 1/3 | `[AXIOM/SELECTION]` | CFL + stencil |
| Center back-reaction suppressing neighbour onset | `[CONFIRMED]` | FTD-0263 β v2 |
| Radiative loss channel (most injected energy escapes as halo) | `[MEASURED]` | FTD-0273 (halo-dominated flux; M_local → 0 as ~L⁻³·⁸) |

## §4 · Model sketch (v0 structure — development is free to revise; the falsifiers are not)

Energy-budget + shell-threshold recursion in firing rank n:

1. **Budget:** injection E₀ = ½(A·K_GENESIS)² at the center voxel (flux quadrature).
2. **Center fires** (p ≈ 1 for A ≥ 2): retains (A−1)·K flux; kinetic drain takes ½ of
   site KE; the manifested charge becomes a Gauss source.
3. **Field at radius r** = (spreading residual flux)·w(r) + (Gauss boost)·Σ_fired ∇G_L,
   where w(r) is the wave-spreading weight (analytic, from the lattice wave kernel —
   not a simulation) and the Gauss term uses the exact Green's-function gradient.
4. **Threshold cascade:** sites with combined field > K fire, each consuming K from the
   spreading budget + drain·KE_local, each adding a Gauss source (β v2 back-reaction
   enters as the sign structure of the added sources).
5. **Radiative partition:** the fraction of E₀ that disperses past the cascade front
   before thresholding (the FTD-0273 halo) is the loss channel; the cascade terminates
   (one-shot, FTD-0267) when the front's spectral density falls below threshold.
6. **Output:** N(A) = total firings; shell occupancies; and the same recursion run at
   other drain values must yield the FTD-0276 drain law *as a prediction*.

The sub-knee A²-class scaling should emerge from the quadratic energy budget vs the
~constant per-firing cost; the knee from the crossover where the Gauss-boost-driven
local cascade saturates the 27-block and growth hands over to the budget-limited
(super-knee, exponent ≈ 2) regime; the drain law from the per-firing cost's
drain-proportional term. **Whether it actually does is the test — these are the
falsifiable load-bearing steps, not assumptions.**

## §5 · Frozen falsifier classes (to be numerically locked in the pre-registration)

All against current-stack data only (FTD-0261/0267/0269/0276); historical k=¼ numbers banned.

- **F-1 (shape):** knee ∈ [14, 18]; sub-knee exponent ∈ [3.3, 4.1]; super-knee ∈ [1.6, 2.1] (the FTD-0269 bands).
- **F-2 (count):** genesis firings at A=10 within a declared band around FTD-0267's ~5.
- **F-3 (geometry):** shell-occupancy L1 distance ≤ 0.30 at A=14 vs the engine profile.
- **F-4 (out-of-sample, decisive):** predicted k_eff(drain) exponent within a declared band around −0.93 (FTD-0276) — the model may NOT be fit to the drain scan; the drain enters only through the §2 register + §4 cost accounting.
- **F-5 (direction):** γ-map trend signs (knee rises with γ; super-knee ratio falls) per FTD-0276 Leg B.

**Banned re-attempts** (CLOSED NEGATIVE provenance): Mechanism α leakage-aggregation
(FTD-0259), Mechanism β pre-genesis kinetics (FTD-0265/0266), Mechanism γ
thermal-crossover knee (FTD-0261). Allowed: β v2 center back-reaction (FTD-0263,
CONFIRMED), genesis-stage throttling (FTD-0267).

## §6 · Honest ceiling (stated in advance)

Even full success yields **"the N(A) law is [CONDITIONAL — DERIVED-GIVEN-IMPOSED-INPUT]"**
— conditioned on the §2 register. It does NOT: derive the drain 0.5 or γ 0.02 (FTD-0276
closed those), promote the SM cluster-mass identification (FTD-0262 IDENT-NULL stands),
or touch FTD-0013/MC-T4.3. The FTD-0110 *unconditional* nonlinear bridge stays `[OPEN]`
unless the §2 register itself is someday derived.

## §7 · Process discipline

Model **development is iterative and declared free** (theory work; no verdicts during
development). The **comparison protocol is pre-registered**: when the model's final form
stabilizes, its source + the falsifier numbers above are SHA256-hash-locked and tagged
(`preregister-genesis-counting-v1`) BEFORE the mechanical comparison run. The prior
information (all target data already published in FTD-0261/0267/0269/0276) is disclosed;
the integrity guard is F-4's out-of-sample structure plus the no-per-target-tuning rule:
every model constant must trace to the §2 register or a §3 ingredient.

**Artifacts:** `scripts/exploration/genesis_counting_model.py` (the model),
`PREREG_GENESIS_COUNTING_v1.md` (the lock, downstream), analysis doc + LEDGER FTD-0277
(the verdict, downstream).
