# ANALYSIS — Lattice Wave Sectors: dispersion atlas + condensate-compression probe, run of record (FTD-0299)

**Status:** `[MEASURED]` (pre-registered run of record). **Date:** 2026-06-14.
**Pre-registration:** [`PREREG_WAVE_SECTORS_v1.md`](../../10_eft_program/preregistrations/PREREG_WAVE_SECTORS_v1.md), git tag `preregister-wave-sectors-v1`, lock commit `8fff0187`.
**Artifacts (SHA256-locked):** `engine/tests/campaign_wave_sectors.cpp` (`e25396b8…`), `scripts/exploration/analyze_wave_sectors.py` (`b76869fe…`).
**Runs of record (local, gitignored):** `engine/results/wave_sectors/wave_sectors_{light_L24,light_L32,light_L48,sound_L24}.csv`.
**Executes:** the FTD-0298-SOUND `[OPEN]` (condensate compression mode). **Confirms:** FTD-0298's structural no-acoustic-sector boundary.

---

## 0 · Verdicts

| Q | Question | Frozen verdict |
|---|---|---|
| Q1 | Does ω(k) match the engine's 18-pt stencil across ⟨100⟩/⟨110⟩/⟨111⟩, IR phase speed isotropic at 1/√3? | **LIGHT-CONFIRMED** |
| Q2 | Does the manifested condensate carry a propagating compression (acoustic) mode? | **NULL** |

`FTD-0299 SUMMARY: LIGHT=LIGHT-CONFIRMED  SOUND=NULL` (OUTCOME A, the pre-declared prior-favoured outcome).

**Nothing is promoted.** FTD-0013 stays `[SMC]`; MC-T4.3 stays `[FOUNDATIONAL OBSTRUCTION]`; FC-1/FC-2 stay `[AXIOM]`-class; FTD-0270/0271/0272/0298 unchanged. This run **engine-confirms** the FTD-0298 boundary; it derives no new physics.

## 1 · Q1 — light-sector dispersion atlas → LIGHT-CONFIRMED

The flux-wave dispersion ω(k) matches the engine's own 18-point isotropic-Laplacian eigenvalue to **machine zero** in every direction at every L, and the IR phase speed is isotropic at `c = 1/√3`, converging with L:

| L | max `|ω_eig−ω_theory|/ω_theory` (all dirs) | c_eff(IR) ⟨100⟩/⟨110⟩/⟨111⟩ | isotropy dev |
|---|---|---|---|
| 24 | 0.00e0 | 0.5757 / 0.5741 / 0.5724 | 8.5e-3 |
| 32 | 0.00e0 | 0.5764 / 0.5755 / 0.5746 | 4.8e-3 |
| 48 | 0.00e0 | 0.5769 / 0.5765 / 0.5761 | **2.1e-3** |

The zone-edge cutoff is `ω_max = 2/√3 ≈ 1.155` rad/tick (`v_g → 0`). This is a directional extension of FTD-0270's axial result: the substrate carries **one** wave sector — light and radio are the same flux wave, differing only in `k` — and it is isotropic in the IR (the residual UV anisotropy is the k⁴ effect of PL-5, not resolvable on this grid by design, M7). The leapfrog temporal frequency `ω_fft` relates to the operator eigenvalue by `sin(ω_fft/2) = ω_eig/2` (a few-% gap at high modes, expected, not instrument disagreement).

## 2 · Q2 — condensate-compression probe → NULL

Across **4 seeds** (all condensed, `m0 = 1.000` at `T_cond = 0.5`) and **5 modes**, the manifested condensate shows **no reproducibly-propagating density branch distinct from the light wave**: `0` modes pass the frozen propagation gate (signed-FFT one-sided asymmetry + arg phase-ramp + prominence-over-control + harmonic guard, with the energy-density primary observable and the conserved state-density cross-check). In the under-powered seeds=1 quick-checks the only response that ever passed sat exactly at `ω_light` (e.g. n=2: `ω_s = 0.4418 ≈ ω_light(0.785) = 0.4419`) — i.e. the density's oscillation is the **shadow of the light wave** (the `2 J_bg·δJ` cross-term), not an independent acoustic mode. The verdict is **amplitude-robust** (NULL at kick 0.05 and 0.10).

**Reading.** This engine-confirms FTD-0298 §5: FTD has light but **no acoustic sector**. The lattice *is* space — there is no spontaneously broken continuous translation symmetry, hence no acoustic Goldstone, and (operationally) no displacement DOF, the Gauss constraint projects out the longitudinal flux, and FTD-0272's first-order genesis admits no Goldstone. With `coupling` ON (the s↔J channel a collective mode *would* propagate in), the condensate still carries no such mode. **FTD-0298-SOUND closes as `[BOUNDARY — engine-confirmed]`.**

**Power + false-NULL guard.** The NULL is powered (4 seeds, 5 modes, condensate verified uniform `m0=1`), and the gates were calibrated against a kick=0 control arm (so a relaxation/breathing transient cannot pass) and set by a pre-lock adversarial review (24 blockers). The pre-reg reserved the heavy adversarial verdict-verification panel for a *surprise* COMPRESSION-FOUND (§7); the conservative NULL does not trigger it.

## 3 · Instrument note (does not affect verdicts)

Read-only `campaign_*.cpp` (no engine-source change) ⇒ **golden-neutral**: `render_bridge_golden` green = `0x56fa28acb5b9fe88` on the build used. Canonical platform CPU `force_cpu()` + `OMP_NUM_THREADS=1`; the post-kick measurement is deterministic regardless of thread count (genesis Loop-2 sequential + stateless index-keyed RNG), and the verdict is seed-ensemble-robust.

## 4 · Epistemic accounting

`[MEASURED]` (pre-registered run of record). **Zero promotions.** Q1 confirms the wave sector is isotropic and matches the stencil; Q2 confirms the no-acoustic-sector boundary. The companion `[VISUALIZATION]` is the docked Scale-0 dispersion panel (`dispersion-panel.js`).

**Next free LEDGER id after this row: FTD-0300.**
