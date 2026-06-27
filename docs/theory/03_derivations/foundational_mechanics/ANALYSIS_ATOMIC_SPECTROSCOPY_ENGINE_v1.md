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
- **But the engine's native FFT readout does NOT extract that ladder — the cause is a
  numerical INSTABILITY, not blending (corrected 2026-06-20; mechanism further corrected
  2026-06-27 `[from-recon]`):** at L=64..256 the autocorrelation→FFT yields **one** bound
  peak at every L. A three-step diagnosis (He+ `--Z 2` + off-center `--offset` + finer dt)
  overturned two earlier reads: (i) the **deep He+ well resolves a sharp, clean 1s**
  (binding 0.21, FWHM ~23× narrower than the 1s–2s gap) ⇒ **not** resolution/blending;
  (ii) an off-center packet excites the 2s/2p **~2500× more** than the 1s, and the
  symmetric probe does not cancel them, yet still one peak ⇒ **not** excitation. The real
  cause: **bare-wave leapfrog amplitude growth** — a discretization instability of the
  *homogeneous* wave integrator, NOT the inhomogeneous KG well. A coupling-OFF isolation
  recon found ρ unchanged with the coupling/well removed, and a no-cluster wavepacket
  (`s≡0`, no KG well possible) still drifts; C(t) grows ~13 orders of magnitude
  (recon ρ≈1.00119/tick — absolute value is window-dependent; the relative controls carry
  the verdict) and one growing unphysical mode (ω≈1.28, near but not at the true 1s)
  swamps the spectrum. The growth is **dt-reducible (~dt²)** on the symplectic path;
  the earlier "Finer dt did **not** remove the growth" / dt-invariance was a
  `RenderBridge::set_dt` **clamp artifact** (the default non-symplectic leapfrog ignores
  `dt_`). So the engine-native FFT is **dynamics-limited**; the accurate spectrum comes
  from the **operator-on-φ_C path** (validated, ladder-confirmed). The earlier
  "wavepacket-blending" reading is **retracted**.

**Net:** the engine spectroscopy is validated as an engine↔operator consistency check in
the sparse regime; the hydrogen excited ladder is finite-size-resolvable (operator side);
the engine-native time-domain FFT readout is a `[BOUNDARY]` at large L (a **bare-wave
leapfrog amplitude growth** — a discretization instability of the homogeneous integrator,
coupling-independent; excitation and resolution are both fine; growth is dt-reducible ~dt²
on the symplectic path; the prior "dt does not cure it" / dt-invariance was a `set_dt`
clamp artifact; absolute ρ magnitude is window-dependent — recon ρ≈1.00119 vs prior
ρ≈1.00275; relative controls carry the verdict `[from-recon]`) and would need a **stable
bare-wave integrator** (implicit / energy-conserving scheme, or a strong absorbing layer)
to resolve it; the
operator-on-φ_C path is the accurate route meanwhile. He+ (`--Z`) and off-center
(`--offset`) knobs were added (golden-neutral, Z=1/offset=0 reproduce the anchors). **Never "FTD derives hydrogen":** ω₀ and the scalar-potential coupling are
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
