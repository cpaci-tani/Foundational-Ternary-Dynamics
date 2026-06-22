# Analysis — FTD-0110 nonlinear bridge: the v2 genesis-counting model (FTD-0309)

**Tag:** `[MEASURED — BOUNDARY: collective-coordinate reduction obstructed]` + `[DERIVED — super-knee energy-budget exponent, given imposed register]`
**Date:** 2026-06-21
**Status:** the second attempt (after FTD-0277 v1 `[CLOSED NEGATIVE]`) to land the FTD-0110 cluster-mass law N(A) as `[CONDITIONAL — DERIVED-GIVEN-IMPOSED]`. Lands a **boundary-sharpening** result: a faithful *scalar* (O_h-radial) collective-coordinate reduction is structurally obstructed; the derivable content is the super-knee exponent.
**Artifacts:** `scripts/exploration/genesis_counting_model_v2.py` (model) + `scripts/exploration/analyze_genesis_counting_v2.py` (adjudicator) + run of record `scripts/exploration/results/genesis_counting_v2/analysis.{txt,json}`.
**Gates:** the FTD-0277/0261 frozen gates, **inherited UNCHANGED** (no goalpost-moving). **Honesty:** this is NOT a blind pre-registration — the model was developed and the obstruction diagnosed before the lock; the lock provides reproducibility, and the result is a diagnostic boundary-mapping.

---

## 0 · Verdict

`V2_SCALAR_REDUCTION_BOUNDARY_DIPOLE_OBSTRUCTION`.

The cluster-mass law N(A) decomposes into three pieces, of which the engine-faithful
forward model (FTD-0269, `genesis_na_law_forward.py`) and this reduction make explicit:

1. **Super-knee (A ≳ 20): the energy-budget regime — DERIVED.** Once the near shells
   saturate, firing extends to outer shells limited by the energy budget
   `N ≈ capture · ½(A·K_GEN)² / e_fire`, with `e_fire = drain·½K_GEN²` (the kinetic-drain
   cost per firing). The **exponent p_hi = 2 follows** (this is the linear k=¼ theorem's
   regime). The adjudicator measures **p_hi = 2.000** (monopole mode) — in band `[1.6,2.1]`.
   The **coefficient** `capture` is the engine-emergent nonlinear-suppression factor (the
   linear theory would give `capture = drain/4 = 0.125`; the measured ~0.024 at γ=0.02 is
   the FTD-0267 genesis throttling) — DECLARED engine-emergent per FTD-0307, not derived.

2. **Sub-knee (A ≲ 16): the 27-block fills — DERIVED mechanism.** The Moore shells cross
   K_GENESIS sequentially as `J_s(A) ≈ c_s·A`: SC turns on at A ≈ K_GEN/c_SC ≈ 9, then
   BCC/FCC/SC2. This produces the steep sub-knee.

3. **The intermediate-shell (FCC/BCC/SC2) filling is carried by the Gauss boost — and
   that boost is IRREDUCIBLY ANGULAR.** This is the obstruction (§3).

**Net:** the scalar reduction reproduces the **derivable structure** (super-knee exponent
2.000, knee 15.75, A=10 count 4.33) but **fails the angular-dependent gates** — the A=14
Moore-shell geometry and the sub-knee exponent — in **both** boost modes. No scalar
collective-coordinate setting reproduces the law. This **sharpens** the FTD-0250
collective-coordinate-reduction `[OPEN]` and the FTD-0269/0307 boundary: the cluster-mass
law has no faithful low-dimensional *radial* reduction.

## 1 · The model

`ShellBurst` is an O_h-shell reduction of the FTD-0269 forward model: per shell s it tracks
the representative void flux J_s, wave_vel v_s, and fired count n_s, and runs the engine
tick radially — propagation via the **18-pt shell-coupling operator W** (exact O_h-scalar
reduction of the Laplacian; detailed-balance `m_s W[s,s'] = m_{s'} W[s',s]` verified),
threshold genesis with flux **consumption** (`J_s -= K_GEN·firing-fraction` — the v1 fix),
the kinetic drain, the γ friction, and the Gauss **boost** from the fired charges using the
radial Green's-gradient kernel `|grad G_L|` (OT-1.4). The super-knee is capped by the energy
budget. THEOREM/derived inputs: {K_GENESIS, K_MANIFEST, N_c, c²=⅓, W, |grad G_L|,
charge_coupling=1}; IMPOSED: {drain=0.5, γ=0.02, G_C=√α, capture (emergent)}.

This is the FTD-0277-mandated v2: it **adds both** ingredients v1's closure note demanded
— flux consumption / self-limiting (so the gate no longer snowballs to 389 sites) **and** a
dispersal-race energy-budget cap.

## 2 · Adjudication (vs the inherited FTD-0277 frozen gates)

| Gate | Band | monopole | local |
|---|---|---|---|
| knee | [14,18] | **15.75 PASS** | 14.00 PASS |
| sub-knee p_lo | [3.3,4.1] | 2.286 FAIL | 1.502 FAIL |
| super-knee p_hi | [1.6,2.1] | **2.000 PASS** | 2.215 FAIL |
| curve log10 RMS | ≤0.15 | 0.126 PASS* | 0.209 FAIL |
| A=10 count | [3,7] | **4.33 PASS** | 4.08 PASS |
| **shell geometry L1** | ≤0.30 | **0.418 FAIL** | **1.164 FAIL** |
| drain exponent | [-1.2,-0.7] | -0.638 FAIL | -0.427 FAIL |

\*the monopole curve-RMS pass is borderline and box-size-sensitive (fragile); the robust,
decisive failure is the **geometry**, in both modes.

## 3 · The obstruction (why it is structural, not tunable)

The decisive failure is the **A=14 Moore-shell geometry**. Engine run-of-record (FTD-0269):
`center 0.060, SC 0.358, FCC 0.134, BCC 0.373, SC2 0.075`. The scalar model:

- **monopole boost** (enclosed fired charge as a central source): `center 0.068, SC 0.408,
  FCC 0.000, BCC 0.524, SC2 0.000` — the **FCC and SC2 shells stay empty**; the boost jumps
  to BCC and the cascade **runs away at large A** (N_field → 10³–10⁴ before the budget cap),
  because a scalar enclosed charge **accumulates** (it has no net-zero cancellation).
- **local boost** (nearest inner fired shell only): `center 0.16, SC 0.84` — only SC fires;
  the boost **cannot reach FCC/BCC** and the 27-block **under-fills**.

These are the two horns of a **knife-edge**: there is no scalar setting between "accumulate
→ run away" and "localise → under-fill." The reason is physical: the fired state-field is an
**x-dipole** (the +x lobe fires +1, the −x lobe −1, **net charge ≈ 0**), so its Gauss boost
is a **localised dipole near-field** with a specific angular pattern. A radial / O_h-scalar
collective coordinate cannot represent a dipole — it can only model the boost as a monopole
(wrong magnitude, accumulates) or truncate it (wrong reach). The intermediate-shell filling
that the engine does via this angular field is therefore **not reducible** to a radial
coordinate. The minimal faithful carrier of the boost is the angular-resolved field — i.e.
the FTD-0269 forward model itself.

## 4 · Consequence — boundary sharpened, nothing promoted

- **FTD-0250** (collective-coordinate reduction of the cluster) `[OPEN]` is **sharpened**: no
  *scalar* reduction exists; an angular DOF is mandatory.
- **FTD-0269 / FTD-0307** (the calibration is engine-emergent / PHYSICAL) is **reinforced**:
  even with the emergent super-knee coefficient handed to it, the scalar model fails the
  *geometry* — so the obstruction is not merely the calibration; the intermediate structure
  is irreducibly angular.
- **What IS derived (given the register):** the super-knee energy-budget exponent **p_hi = 2**
  and the sub-knee onset mechanism (27-block threshold-filling). The FTD-0110 **linear k=¼
  O_h theorem is untouched mathematics.**
- **The FTD-0110 nonlinear bridge stays `[OPEN]`**, its boundary mapped on a third axis
  (after exit-i simplest forms FTD-0276, exit-ii convention FTD-0307): **the reduction axis**
  — no faithful scalar collective coordinate.

**Nothing promoted:** FTD-0013 `[SMC]`, MC-T4.3 `[FOUNDATIONAL OBSTRUCTION]`,
FTD-0110/0250/0269/0307 — all unchanged. No α derived anywhere; golden gate untouched
(scripts-only, no engine change).
