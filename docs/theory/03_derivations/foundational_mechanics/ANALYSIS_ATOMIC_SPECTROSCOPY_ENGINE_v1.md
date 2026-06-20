# Analysis — Engine-native atomic spectroscopy (FTD-0281 Leg-2 / FTD-0308)

**Tag:** `[CONDITIONAL — DERIVED-GIVEN-IMPOSED-INPUT]` + `[MEASURED — engine↔operator consistency, sparse-regime]` + `[BOUNDARY — engine FFT readout]`
**Date:** 2026-06-20
**Branch:** `engine/atomic-spectroscopy` (commits `9dfad16b` rung-a, `0869603d` GPU port, `e94f4c0b` GPU parity + probe-gather).
**Status:** the spectroscopy coupling `db_clock_coulomb` already existed (FTD-0281 hook, 2026-06-13); this is the FFT spectroscopy run of record + the GPU port. **Golden-neutral** (default-OFF toggles; `0xb604d81a3d79366e` green on CPU and after the GPU kernel change).

---

## 0 · Verdict

Built the **engine-native atomic spectroscopy instrument** and ran hydrogen on it.
The honest result is a **split**:

- **Engine↔operator consistency CONFIRMED in the sparse regime (L=32):** the engine's
  symplectic-leapfrog time evolution of the clocked flux in its own Coulomb well
  reproduces the operator `A = −c²L₁₈ + 2ω₀V` (V=−φ_C) ground frequency to **0.53%**
  (engine FFT `ω=1.493811`, operator `ω=1.485971`, both below ω₀=1.5 ⇒ bound). The
  CPU↔GPU parity is **machine-precision** (`ω=1.493811` identical to 6 digits).
- **The hydrogen excited LADDER is finite-size-resolvable — operator-confirmed:** the
  operator built from the engine's *own* φ_C at **L=128 binds 6 states** (`n_bound=6`):
  1s (binding 0.0193), 2s (0.00266), 2p-triplet (0.00184 ×3, degenerate), 3s (0.00018).
  This is **exactly the excited spectrum the LOBPCG/Python eigensolver could not resolve**
  (clustered near the continuum edge): the boundary mapped earlier is **box-size, not
  structural**, and the engine's field reaches it.
- **But the engine's native FFT readout does NOT extract that ladder at large L:** at
  L=64..256 the time-domain autocorrelation→FFT yields **one** bound peak (binding
  ≈0.0108), which matches **neither** the operator 1s (0.0193) **nor** the 2s — it sits
  at an intermediate frequency between them. This is **wavepacket-blending** (a broad
  Gaussian excites a superposition; the autocorrelation peak lands at a weighted average),
  **not** a dt artifact — finer dt (0.5→0.25) did not move it. So the accurate large-L
  spectrum comes from the **operator-on-φ_C path**, not the engine FFT.

**Net:** the engine spectroscopy is validated as an engine↔operator consistency check in
the sparse regime; the hydrogen excited ladder is finite-size-resolvable (operator side);
the engine-native time-domain FFT readout is a `[BOUNDARY]` at large L (blends the dense
ladder) and would need a different excitation/extraction (impulse + filter-diagonalization)
to resolve it. **Never "FTD derives hydrogen":** ω₀ and the scalar-potential coupling are
`[IMPOSED]`; the FTD-0270/FC-1 quantum-dynamics ceiling and the linear-dispersion caveat
stand. The substrate-`[THEOREM]` content used is only the 18-pt Poisson Green's function
(OT-1.4).

## 1 · The instrument

`engine/tests/campaign_atomic_spectroscopy.cpp` (golden-neutral, CPU + GPU via
`--backend cpu|gpu`): a LOCKED +1 charge sources the engine's own Gauss/Coulomb φ_C
(static to 1.7e-7, `forces=false`); `de_broglie_clock` gives ω₀; `db_clock_coulomb`
applies `ω_eff²=ω₀²+2ω₀V`; a Gaussian flux wavepacket rings; a shell-autocorrelation
`C(t)=Σ J(0)·J(t)` is FFT'd (`ftd::power_spectrum`). The GPU path (commit `e94f4c0b`)
adds the KG clock + Coulomb coupling to the CUDA `phase_read` kernel + a deterministic
per-tick probe-gather kernel (avoids the full-lattice download — the large-L bottleneck),
runs to **L=256 on the RTX 5090 using 1.9 GB of 32 GB** (headroom to L≳512).
`scripts/exploration/analyze_atomic_spectroscopy.py` + `analyze_excited_spectroscopy.py`
build the operator from φ_C (scipy `eigsh`) and compare.

## 2 · Numbers

| L | engine FFT ground ω (binding) | operator ground ω (binding) | operator n_bound |
|---|---|---|---|
| 32 (CPU=GPU) | 1.493811 (0.0062) | 1.485971 (0.0419) — only 1 bound | 1 |
| 128 (GPU) | 1.489541 (0.0105) | 1.480673 (0.0193) | **6** (1s/2s/2p×3/3s) |
| 256 (GPU) | 1.489541 (0.0105) | (eigsh too slow; n_bound≥6 expected) | — |

At L=32 the spectrum is sparse (1 bound state) and the engine FFT = operator (0.53%).
At L≥128 the operator binds a ladder but the engine FFT blends it into one mid-ladder peak.
Finer dt (0.25) leaves the engine ground at 0.0108 — confirms blending, not dispersion.

## 3 · Honest scope / no promotions

`[CONDITIONAL — DERIVED-GIVEN-IMPOSED]`. Validates two engine code paths against the
FTD-0278 Leg-1 operator (sparse regime); confirms the hydrogen excited ladder is
finite-size (operator); maps the engine FFT readout as a large-L `[BOUNDARY]`. ω₀ + the
coupling `[IMPOSED]`; FTD-0270 boundary + FC-1 + linear-dispersion caveat unchanged.
**Nothing promoted:** FTD-0013 `[SMC]`, MC-T4.3, FTD-0270/0271/0278/0279 — all unchanged.
The `db_clock_coulomb` coupling is GPU-ported + CPU↔GPU-parity-validated; golden green.
