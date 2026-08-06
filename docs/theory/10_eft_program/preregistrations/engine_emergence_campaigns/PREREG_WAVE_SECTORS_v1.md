# PRE-REGISTRATION — Lattice Wave Sectors: dispersion atlas + condensate-compression probe, run of record (FTD-0299)

**Status:** `[PRE-REGISTRATION]` — design lock; the run of record follows the hash-lock.
**Date:** 2026-06-14
**LEDGER id (reserved):** FTD-0299
**Git tag (to be applied at lock):** `preregister-wave-sectors-v1`
**Executes:** the **FTD-0298-SOUND** `[OPEN]` item (does the manifested condensate carry a propagating compression / acoustic mode?) + a directional re-measurement of the light-sector dispersion.
**Prior context:** FTD-0298 (`ANALYSIS_LATTICE_WAVE_SECTORS_v1.md`; light=radio=one flux-wave sector; structural no-acoustic-Goldstone boundary), FTD-0270 (linear lattice dispersion), FTD-0272/0274/0275 (first-order genesis + condensate prep), FTD-0258 PL-4/PL-5 (deviation spine).

---

## §1 · Purpose and narrow targets

Two frozen questions. **No other claims are in scope.**

- **Q1 (light-sector dispersion atlas).** Does the engine's flux-wave dispersion ω(k) match its own 18-point isotropic-Laplacian eigenvalue across the three high-symmetry directions ⟨100⟩/⟨110⟩/⟨111⟩, and is the IR phase speed isotropic at `c = C_WAVE = 1/√3`? (A directional extension of FTD-0270's axial result.)
- **Q2 (condensate-compression probe — the FTD-0298-SOUND open door).** Does the manifested condensate phase support a **propagating compression (acoustic-like) mode** — a density branch `ω_s(k)` distinct from the light branch — or not?

**Prior-favoured outcome (declared, §6):** Q1 LIGHT-CONFIRMED; Q2 **NULL** (FTD-0298 §5: the lattice *is* space ⇒ no spontaneously broken translation symmetry ⇒ no acoustic Goldstone).

## §2 · Frozen definitions

- **Lattice dispersion law** `[DEFINITION]` — the exact 18-pt symbol `λ(k) = (2/3)Σcos kᵢ + (2/3)Σ cos kᵢcos kⱼ − 4`; `ω_theory = √(−c²·λ)`, `c² = 1/3`. Evaluated at per-component `(kx,ky,kz) = q·D`, `q = 2πn/L`, `D ∈ {(1,0,0),(1,1,0),(1,1,1)}`, `|k| = q·|D|`. (The axial law `2c|sin(k/2)|` is the `D=(1,0,0)` special case only.)
- **ω_eig** `[DEFINITION]` — single-tick operator eigenvalue from `wave_vel = c²∇²J = −ω²·J` on a transverse standing plane wave (probe `(1,0,0)`, non-node).
- **Condensate prep** `[DEFINITION]` — toggles `{wave_propagation, gauss_projection, genesis, coupling}` ON, `dual_substrate` OFF, Langevin at `T_cond` (≥ T_up) for `equil` ticks; condensate requires manifestation fraction `m = manifested/L³ ≥ M_MIN`. **`coupling` ON is load-bearing** (the s↔J channel a collective compression mode must propagate in).
- **Compression kick** `[DEFINITION]` — after equilibration, Langevin OFF (microcanonical); add `δ wave_vel.x(x) = kick·sin(k·x)` (a small longitudinal compression), `kick = 0.05` (linear-response; verified amplitude-robust vs 0.10).
- **Density Fourier modes** `[DEFINITION]` — each tick, per-x profiles of three densities are k-projected: energy `e(x)=Σ_{y,z} ½|J|²` (continuous, **primary**), `|J|(x)=Σ_{y,z}|J|`, and the conserved state density `ρ(x)=Σ_{y,z} s`. `X_k(t) = Σ_x X(x)·e^{−ikx}`.
- **Control arm** `[DEFINITION]` — same equilibrated condensate, **no kick**; calibrates the background/breathing spectrum.
- **Propagation test** `[DEFINITION]` — on the detrended (quadratic detrend + Hann window, lowest `EXCLUDE_BINS=2` excluded) complex series: signed-FFT peak with one-sided power asymmetry `≥ ASYM_MIN` AND `arg(z(t))` linear-ramp `R² ≥ PHASE_R2`, AND peak prominence `> PROM_MARGIN × control prominence`, AND not within `HARM_TOL` of `2·ω_light` (threshold-rectification guard).

## §3 · Frozen artifacts (hash-lock)

| Artifact | SHA256 |
|---|---|
| `engine/tests/campaign_wave_sectors.cpp` | `e25396b8c6552d4bf7e03436b169d40e991de908a26330cca046eb3f5e92dd30` |
| `scripts/exploration/analyze_wave_sectors.py` | `b76869fee3046aa134221abd0972da5ba339b8a2f7c059d0209801696ee75936` |

The campaign instrument is **golden-neutral**: it is a read-only `campaign_*.cpp` with no engine-source change; `render_bridge_golden` verified green = `0x56fa28acb5b9fe88` on the build used (2026-06-14).

## §4 · Runs of record (frozen invocations)

Executed only AFTER the hash-lock tag. CPU, `OMP_NUM_THREADS=1` (reproducibility, M11).

- **Light atlas:** `campaign_wave_sectors --arm=light --L=24 --ticks=256`; `--L=32`; `--L=48`.
- **Sound probe:** `OMP_NUM_THREADS=1 campaign_wave_sectors --arm=sound --L=24 --seeds=4 --nmodes=5 --ticks=256 --equil=200 --kick=0.05 --Tcond=0.5`.
- **Analysis:** `python scripts/exploration/analyze_wave_sectors.py <results_dir>` → the `FTD-0299 SUMMARY` token.

Outputs (local, gitignored): `engine/results/wave_sectors/`.

## §5 · Frozen verdict logic (analyzer-encoded)

**Q1 (light):** `LIGHT-CONFIRMED` iff max `|ω_eig−ω_theory|/ω_theory < LIGHT_RELERR_BOUND (0.02)` across all directions AND IR phase-speed isotropy dev `< ISO_BOUND (0.02)`; else `LIGHT-DEVIATION`.

**Q2 (sound), four outcomes:**
- `INVALID` — condensate not formed (mean min-`m < M_MIN=0.50`) or m-drift `> M_DRIFT=0.15`.
- `NULL` — fewer than `MIN_MODES=3` reproducibly-propagating modes (cross-seed `CV < SEED_CV=0.30`, primary energy mode propagating AND state-density `ρ_k` agreeing), OR the fitted branch tracks the light curve (`mean |ω_s−ω_light|/ω_light < LIGHT_CURVE_TOL=0.10`).
- `GAPPED-MODE` — ≥3 propagating modes; fit `ω_s = √(Δ²+(c_s k)²)` with intercept `Δ > GAP_BINS×(FFT resolution)`.
- `COMPRESSION-FOUND` — ≥3 propagating modes, distinct from light, gapless (`Δ ≤ GAP_BINS×res`), with a reported `c_s` + R².

Frozen thresholds (in the SHA-locked analyzer): `LIGHT_RELERR_BOUND=0.02, ISO_BOUND=0.02, M_MIN=0.50, M_DRIFT=0.15, EXCLUDE_BINS=2, PROM_MARGIN=3.0, ASYM_MIN=0.30, PHASE_R2=0.80, SEED_CV=0.30, MIN_MODES=3, LIGHT_CURVE_TOL=0.10, HARM_TOL=0.15, GAP_BINS=2`.

## §6 · Pre-declared outcomes

- **OUTCOME A (prior-favoured):** Q1 LIGHT-CONFIRMED + Q2 **NULL** — confirms FTD-0298's structural no-acoustic-sector boundary; the FTD-0298-SOUND `[OPEN]` closes as `[BOUNDARY — engine-confirmed]`.
- **OUTCOME B:** Q2 GAPPED-MODE — a gapped scalar (Higgs-like) compression oscillation, not a gapless sound; FTD-0298-SOUND sharpened.
- **OUTCOME C (surprise):** Q2 COMPRESSION-FOUND — a genuine gapless acoustic branch; would be a major result and is gated behind the adversarial verdict-verification (Phase D) before any promotion.
- **Prior-favoured: OUTCOME A.**

## §7 · Pre-declared exclusions (banned moves)

1. No tuning of thresholds, kick, `T_cond`, seeds, or modes after the lock to move a verdict.
2. No reading a propagating mode that tracks `ω_light` as "compression" (the branch-shape discriminator + harmonic guard are frozen).
3. No claim of `c_s` without the ≥3-mode fit + reported R².
4. No promotion of OUTCOME C without surviving the adversarial verdict-verification panel.
5. No SI/physical-scale claim; this is a substrate measurement at the stated L, stencil, protocol.
6. **Zero promotions:** FTD-0013 stays `[SMC]`, MC-T4.3 stays `[FOUNDATIONAL OBSTRUCTION]`, FC-1/FC-2 stay `[AXIOM]`-class, FTD-0270/0271/0272/0298 unchanged.

## §8 · Method specification

Toggles per §2. Seeds: `seed_base = 0x73E12000`, decorrelated by Knuth `×2654435761`. Per seed: one control (all modes) + one kick run per mode (re-equilibrated from the same seed). `set_sor_iterations(150)`. `OMP_NUM_THREADS=1`. The measurement phase (post-kick, Langevin OFF) is deterministic regardless of thread count (genesis Loop-2 sequential + stateless index-keyed RNG); the Langevin equilibration is thread-sensitive, so reproducibility is asserted at fixed (seed, thread count) and the verdict is seed-ensemble-robust.

## §9 · Hash-lock declaration

This document, the campaign instrument, and the analyzer are committed together; the commit is tagged `preregister-wave-sectors-v1` BEFORE any §4 run-of-record executes. The §3 SHA256 hashes bind the instrument and analyzer versions. Any post-lock edit to §§2–8 or to either artifact invalidates the lock and requires a v2.

---

## Appendix A · Adversarial hardening provenance

This v1 instrument is the **hardened** product of a pre-lock adversarial review (4 skeptic lenses + synthesis; 34 findings, 24 lock-blockers, verdict LOCK-READY-AFTER-FIXES). The load-bearing fixes folded in before this lock:

- **M9** — `coupling` ON in Arm 2 (without the s↔J channel a collective mode is structurally impossible → NULL was near-tautological).
- **M6** — Arm 1 compares ω_eig vs the engine's OWN 18-pt stencil eigenvalue (the axial law manufactured spurious 41%/73% "errors" for ⟨110⟩/⟨111⟩; with the correct law rel-err → 0).
- **M7** — the k⁴ anisotropy exponent is unmeasurable at L≤256 and is dropped; IR phase-speed isotropy is the isotropy evidence.
- **M1/M3** — kick=0 control arm + signed-FFT propagation test (one-sided asymmetry + phase-ramp) + Hann window + detrend + prominence-vs-control (a bare PROM=4 passed 100% of white-noise trials).
- **M2** — continuous primary observable (energy density) with the state density as an agreement cross-check; 2·ω_light harmonic guard (Σs is a threshold-rectified observable).
- **M4/M5** — gap tested on the ≥3-mode fitted intercept (not a single k); branch-shape light discriminator (a `c_s≈c` gapless mode coincides with light at small k and must not be auto-excluded).
- **M8** — kick reduced 0.3→0.05 (linear response); NULL verified amplitude-robust at 0.05 and 0.10.
- **M10** — `T_cond=0.5` (condenses at L=24; T=0.12 did not) + full-window m(t) stability gate.
- **M11** — `OMP_NUM_THREADS=1` + the determinism note above.

(Quick-checks pre-lock: Arm 1 LIGHT-CONFIRMED, max rel-err 0.00e0 all directions, isotropy dev 4.8e-3; Arm 2 NULL at kick 0.05 and 0.10. These validated the path; the §4 runs are the run of record.)
