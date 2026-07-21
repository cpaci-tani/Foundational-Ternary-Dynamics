# Analysis — FTD-0271: Given a Rest-Mass Clock, FTD is a Single-Particle Pilot-Wave Theory (CONDITIONAL)

**Tag:** `[CONDITIONAL — DERIVED-GIVEN-IMPOSED-INPUT]`
**LEDGER row:** FTD-0271
**Pre-registration:** `PREREG_DE_BROGLIE_CLOCK_v1.md`, tag `preregister-de-broglie-clock-v1`, lock commit `3df173b1`
**Artifact:** `scripts/exploration/derive_de_broglie_clock_2026-06-11.py` (SHA `a55b5c36`); run of record `scripts/exploration/results/de_broglie_clock_2026-06-11.csv`

---

## 0 · Verdict

Adding a Klein-Gordon rest-mass clock `∂²J/∂t² = c²∇²J − ω₀²J` (`ω₀ ∝ K_B`) to FTD's natively-massless flux turns the substrate into a **single-particle pilot-wave theory**: a moving cluster carries a **de Broglie matter wave** (`λ ∝ 1/v`) and its non-relativistic envelope obeys the **Schrödinger equation**. This is the exact quadratic sector FTD-0270 measured as the only door out of its dispersion boundary. **The result is CONDITIONAL**: the clock is an imposed input (A0), and the de Broglie/Schrödinger consequences are textbook Klein-Gordon — so this is a derivation *given* the clock, not an unconditional FTD result. **FTD-0401/0402 correction (2026-07-21):** FTD-0401 exposed the old unmapped `c=1` implementation. FTD-0402 then normalized raw `Voxel::velocity` by `C_SPEED`, and phase now advances from the selected full budget `B=|u|²/C_SPEED²+L²`. This repairs implementation consistency only: the moving-clock law and scalar `ω₀∝K_B` remain imposed, not covariant substrate theorems. The wave-clock result FTD-0252 remains independent because it used `v=v_g/C_WAVE` and never read `voxel.tau`.

## 1 · A0 — the clock is imposed (gate, on record)

The flux dynamics is `delta_j = c²·Lap18(J) + G_C·∇s + G_C·∇×(s·v)` (`phase_read.cpp:105–164`) — **no restoring term**. Genesis applies a dissipative *drain* (`v.flux *= max(0,1−K_GENESIS/|J|)`), not `−ω₀²J`. FTD-0251's native clock winds at `ω(k)`, **zero at k=0** (massless). So FTD has **no rest-mass oscillation**; `ω₀ ∝ K_B` must be **added**. Verdict: `ω₀∝K_B is [IMPOSED/SELECTION], not [FORCED]`.

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

## 4 · A5 — historical covariance proposal; implementation normalized by FTD-0402

The historical argument said that a de Broglie clock needs a rest frequency `ω₀` plus a relativistically covariant clock rate, and that `accumulate_proper_time` supplied the second. FTD-0401 invalidated that bridge as then represented: raw `Voxel::velocity` was passed directly to `√(1−v²)` despite `C_SPEED=1/√3`. FTD-0402 replaces that legacy formula with the selected raw-lattice budget `B=|u|²/C_SPEED²+L²`, advances `tau` once in the common host post-pass, and tests the causal boundary and CPU/GPU agreement. This closes the implementation mismatch but does **not** prove Lorentz covariance or derive the clock law. The FTD-0402 campaign is `PARTIAL` because its repository-wide aggregate gate was interrupted; `§12-cnorm` therefore remains open.

## 5 · Status of the program & what remains OPEN

- **B (matter wave): DONE** `[CONDITIONAL — DERIVED-GIVEN-IMPOSED-INPUT]`.
- **A (engine mass-term toggle): DONE** `[DERIVED — lattice correctness]`. Default-OFF `de_broglie_clock` toggle + scalar `omega0` added (`term_toggles.h`); `phase_read.cpp` subtracts `flux·ω₀²` from `delta_j` at manifested (`state≠0`) voxels in **both** the dual and single branches. Because `delta_j` is the flux acceleration, the leapfrog integrates `−ω₀²J` into the KG oscillation. **Golden hash `0x56fa28acb5b9fe88` byte-identical** (default-OFF + the new unhashed `phase` field ⇒ golden-neutral). Test `test_de_broglie_clock.cpp`: in the k=0 rest mode (all voxels manifested, uniform flux ⇒ Laplacian ≡ 0) the **centre flux oscillates at ω₀** — clock OFF flat to `4e-15`; clock ON period within **5%** of `2π/ω₀`, amplitude bounded. So a static cluster's flux now carries the rest-frame Compton oscillation it lacked.
- **A5 (proper-time → clock): NORMALIZED IMPLEMENTATION; covariance claim remains WITHDRAWN.** Voxel `phase` advances as `dφ=ω₀·dτ` using FTD-0402's selected full budget. Targeted exact, CPU/GPU, golden, and WASM gates pass, while the interrupted aggregate gate keeps FTD-0402 `PARTIAL`. Both the scalar `ω₀∝K_B` and the moving-clock law remain `[IMPOSED]`. FTD-0252/0268 retain their independent wave-clock findings.
- **E (guidance, the heart of pilot-wave): DONE — verdict `GUIDANCE-ABSENT`** `[MEASURED — BOUNDARY]`. Campaign `campaign_de_broglie_guidance.cpp`: a cluster placed in a pure-phase pilot wave `J = A(cos kx, sin kx, 0)` (uniform `|J|`, so `∇S = k` with no magnitude gradient) acquires velocity `v_meas ≈ 1e-19` (machine zero) for **every** `k`, while the Coulomb control drifts (`+1.4e-3`, proving the force/movement path is live). **The FTD force law has no term that converts a flux phase gradient into a force on the cluster** — matter moves by magnitude-derived forces (Coulomb/gravity/Lorentz), *not* by `∇S`. Guidance is **not** emergent; a working pilot wave would require **adding** the de Broglie guidance equation. This is the audit-predicted boundary and the genuinely non-circular content of the arc: FTD has the pilot-wave *ontology* and (given the imposed clock) the de Broglie *wave*, but the wave does **not** guide the particle.
- **The ℏ scale (inherited):** every quantity is dimensionless; `K_B→ω₀[rad/tick]` is `[SELECTION]` — the substrate fixes the *shape* (`λ∝1/v`), never the absolute scale.
- **Web demo:** a Scale-0 scenario `s0-seed-de-broglie-clock` + interactive panel (`de-broglie-clock-panel.js`) drive the real engine to show the centre flux `J_x(t)` oscillating live, with the analytic de Broglie `λ(v) ∝ 1/v` curve and the `[CONDITIONAL]` disclaimer in the panel footer.

## 6 · No promotions

FTD-0013 `[SMC]`, MC-T4.3, FC-1, FTD-0270 (native massless boundary — still TRUE for unmodified FTD) — all unchanged. FTD-0271 remains `[CONDITIONAL — derived given imposed clock]`; A5 supplies only implementation wiring and E remains guidance-absent. The honest headline is *"given an imposed rest-mass clock and normalization, FTD's flux carries the textbook de Broglie/Klein–Gordon consequences."*
