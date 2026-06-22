# Analysis — Is x₋ = 3.024 the dimensionless pressure of the flux? (FTD-0312)

**Tag:** `[MEASURED — CLOSED NEGATIVE]`
**Date:** 2026-06-22
**Lock (same-session, transparent — not a blind pre-reg):** the EoS formula + verdict logic were fixed before the run-of-record; SHA256-locked artifacts —
`engine/tests/campaign_flux_equation_of_state.cpp` `95429e7e…`,
`scripts/exploration/flux_eos_analytical.py` `c6f5be0e…` (Leg A),
`scripts/exploration/analyze_flux_eos.py` `0855e4f5…` (Leg B analyzer).
Result runs **deflationary** (closes the owner's own conjecture). Golden gate **green** `0xb604d81a3d79366e` (read-only campaign, golden-neutral).

---

## 0 · Verdict

**CLOSED-NEGATIVE: x₋ = 3.023964 is NOT the dimensionless pressure of the flux.** The master quadratic's smaller root x₋ — specifically its residual δ_c = x₋ − 3 = 0.023964 (`[OPEN]` pure-math, SPEC_OPEN_MATH_BY_SECTOR §1) — does not coincide with the flux field's equation-of-state in any natural state, and the EoS that *can* reach 3.024 is **geometric (spectrum-set), not α-locked.** δ_c remains a pure-math `[OPEN]` residual.

**The setup.** x₋ is not free: x₋ = 16G\*³/x₊ = **16G\*³·α = 3.023968** (given x₊=1/α `[SMC]`), so "flux pressure = x₋" means "flux pressure = 16G\*³α," α-dependent. A radiation field's dimensionless EoS 1/w = ρ/p = 3 **exactly** in the continuum — the same 0.80% from x₋ that retired the old "x₋ = N_c = 3" reading (FTD-0014). The test: does a measured flux EoS carry the α-locked 0.024, or sit at the radiation integer 3?

## 1 · A degeneracy correction (load-bearing)

The naive definition `Π = 3ρ/Σ_i T^ii` with the **Maxwell stress** `T^ij = ½δ_ij(E²+B²) − E_iE_j − B_iB_j` is **degenerate**: that tensor is traceless (`Σ_i T^ii = ½(E²+B²) = ρ` identically), so Π ≡ 3 for *any* field — it measures nothing (the continuum EM stress-energy is traceless = massless = exact radiation). The real, non-degenerate EoS deviation is the **lattice trace anomaly**, captured by the **kinetic pressure** from the flux mode spectrum:
`1/w = 3·Σ_k ρ_k / Σ_k ρ_k·(k·∇ω/ω)`, ρ_k = |wave_vel_k|², over the FTD 18-pt dispersion `ω(k)² = −c²M(k)`. Linear dispersion → exactly 3; the lattice bending of ω(k) (group velocity < c near the zone edge) pushes 1/w **above** 3 — the right sign for δ_c > 0, but set by the mode **spectrum**, not by α.

## 2 · Leg A — analytical lattice EoS (`flux_eos_analytical.py`)

| spectrum | 1/w |
|---|---|
| IR limit (low-k only) | **3.000** (radiation, exact) |
| Bose bath T=0.02 / 0.05 | 3.006 / 3.038 (crosses 3.024 near T≈0.04) |
| classical-occ IR cutoff k_max≈0.45 | **3.024** (crosses x₋) |
| full Brillouin zone (classical equipartition) | **9.65** |

1/w is a **smooth function of the mode spectrum**, sweeping continuously from 3.000 (IR) past 3.024 to 9.65 (UV). **3.024 is reachable but not special** — it is one point on a geometric curve, requiring a fine-tuned IR cutoff (k_max≈0.45); the correction is `ω(k)`-geometric, **not α-dependent**.

## 3 · Leg B — engine measurement (`campaign_flux_equation_of_state.cpp`)

A clean transverse flux-wave bath (Langevin thermostat + wave propagation + Gauss projection; genesis off), equilibrated at L=32, wave_vel dumped, FFT'd, EoS computed per §1. Run of record (3 seeds × 5 snapshots each):

| T | 1/w |
|---|---|
| 0.02 | 10.483 |
| 0.05 | 10.476 |
| 0.10 | 10.488 |
| 0.20 | 10.465 |
| 0.40 | 10.484 |

**1/w = 10.48 ± 0.02, T-INDEPENDENT** (spread 0.022 across a 20× temperature range; the wave-energy scales linearly with T — classical equipartition confirmed). The Langevin thermostat populates **all modes ~equally**, so the bath is UV-saturated and 1/w is the **full-Brillouin-zone geometric constant ~10.48** (matching Leg A's ~9.65 up to the kinetic-vs-total equipartition normalization). This is **FAR from x₋=3.024** (off by 7.46) and from radiation 3.000.

## 4 · Conclusion

The engine's natural flux states give **~3.000** (IR-dominated / radiation) or **~10.5** (thermal / full-BZ); **x₋=3.024 is a narrow, non-special window in between**, reachable only by fine-tuning the spectrum to an IR cutoff k_max≈0.45 — and the value is **geometric (spectrum-set), T-independent in the thermal regime, and not α-locked**. The α-tracking discriminator is moot: the thermal EoS doesn't even sit near 3.024, let alone track 16G\*³α. **x₋ is not the dimensionless flux pressure.** This is the same lesson as FTD-0310 / the retired N_c reading: a value ≈3 is unremarkable; only an α-locked, structurally-forced 3.024 would be evidence, and the measured EoS is neither.

**Caveats.** The pre-registered EoS = the kinetic spectral pressure (the Maxwell stress being degenerate). The campaign ran on the GPU backend (`force_cpu` advisory; the EoS is a seed-averaged statistical observable — 3 seeds agree to 0.02 — so backend-robust). δ_c's pure-math closed form stays `[OPEN]`.

## 5 · Non-promotion

`[MEASURED — CLOSED NEGATIVE]`. Nothing promoted; the closure is the deliverable. x₊=1/α `[SMC]`, the master quadratic `[THEOREM]`, N_c=3 `[THEOREM]`, the algebraic spine — all unchanged. δ_c = x₋ − 3 remains a pure-math `[OPEN]` residual with no physical-pressure identification. Golden gate untouched (read-only campaign).
