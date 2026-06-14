# PRE-REGISTRATION — Halo-exponent forcedness audit (FTD-0300)

**Status:** `[PRE-REGISTRATION]` — design lock; run of record follows the hash-lock.
**Date:** 2026-06-13
**LEDGER id (reserved):** FTD-0300
**Git tag (to be applied at lock):** `preregister-halo-forcedness-v1`
**Executes:** the gate (Step 1) of the dark-matter / SPARC rotation-curve program. Is
the engine's single-particle self-field halo *exponent* a FORCED geometric invariant,
or a finite-size / regime artifact (the FTD-0269 question, applied to the halo)?

---

## §1 · Purpose and narrow target

The engine builds a single-particle self-field flux halo whose magnitude falls as
`|J|(r) ~ norm · r^p`. The canonical GPU L=128 value (DERIV_DARK_SECTOR_DYNAMICS.md
§4.1, fit r∈[7,23]) is **p ≈ −0.69**, and §4.2 identifies the **lossless far field**
(selective damping ON) as the *dark-matter* mechanism. Any downstream dark-matter
rotation-curve prediction (Step 2: confront SPARC / the radial-acceleration relation)
inherits this *shape*. FTD-0269 found the N(A) law's shape forced but its calibration
tuned — a `[BOUNDARY]`. This pre-registration tests whether the **dark-matter halo
exponent** is a forced, L-convergent geometric quantity, or a finite-size artifact.

**Single narrow question:** for the lossless (selective-ON) dark-matter halo, is the
exponent `p` (a) L-convergent and (b) measured on a *localized* envelope (r_eff ≪ L/2),
so that it is a forced shape — or does the lossless field saturate the periodic box
(r_eff ≈ L/2) so its exponent is a finite-size artifact that drifts with L?

**Not in scope:** the halo *amplitude*; the rotation-curve / SPARC comparison itself
(gated on this verdict; its own future pre-reg); promotion of any LEDGER claim.

## §2 · Frozen definitions

- **Halo exponent `p`** `[DEFINITION]` = the log-log least-squares slope of ⟨|J|⟩(r) vs
  r over the **frozen window r∈[7,23]** `[FROZEN]`, computed by the hash-locked campaign
  and carried in the summary CSV `exponent` column.
- **Localization ratio `C = r_eff / (L/2)`** `[DEFINITION]`. **Localized** iff `C < 0.5`;
  **box-filling** iff `C > 0.8`.
- **Lossless regime** = `selective_damping = ON` (the §4.2 dark-matter mechanism).
  **Damped regime** = `selective_damping = OFF` (uniform damping; the forced-control).
- **Toggle stack** `[FROZEN]`: `minimal` = {wave_propagation, coupling, damping,
  selective_damping, gauss_projection, dual_substrate}; validated bit-equal to the
  canonical `full` (enable_all − genesis − movement) for `|J|` (forces/Poisson/weak do
  not touch the flux field). Locked particle, `inject_particle(C,+1,{0,0,K_B})`, ticks
  frozen at 1500 (≫ the box wrap time L·√3/2 at every L below).
- **Frozen thresholds:** `L_CONV_TOL = 0.10` (|Δp| between the two largest L);
  `LOCALIZED = 0.5`, `BOXFILL = 0.8` on `C`; `R2_MIN = 0.95`.

## §3 · Frozen artifacts

| Artifact | SHA256 |
|---|---|
| `engine/tests/campaign_halo_forcedness.cpp` | `84f7c407bbdc3bd8e9530235f828dec68c90a9b48f3a39896635609ae92b188e` |
| `scripts/exploration/analyze_halo_forcedness.py` | `44f09ac4d01b3be40359266bf56bb77526524499e85d165316d60762c8c5ad76` |
| `scripts/exploration/run_halo_constant_sweeps.py` | `384bd0481dd332c3e339f4eed8dcbba1be392b85e7d8076a899d8f89abeee896` |

Observation-only measurement (new TU; no edits to any `phase_*.cpp`, kernel, or constant
default). Golden-neutral: `test_render_bridge_golden` = `0x56fa28acb5b9fe88`, verified
green with the new TU present. The analyzer encodes the §5 verdict and is frozen before
the run of record.

## §4 · Prior information (disclosed for integrity — NOT a blind test)

A directional GPU scout (un-registered, before this lock) already established the
direction; the run of record *quantifies* it on the frozen L-grid (FTD-0276 pattern).
Disclosed:

- **Precedent.** FTD-0269 found the N(A) law `[BOUNDARY]` via the same engine machinery.
- **Three instrument facts (verified in source).** (i) The −0.69 fit is not an existing
  instrument output — the campaign adds it. (ii) `kinetic_drain` fires only in the
  genesis branch (a locked particle is insensitive). (iii) `test_selffield_profile` fits
  in a different (selective-OFF) regime.
- **Scout (GPU/FFT, minimal stack, ticks=1500):**
  - Lossless (selective ON): **p = −0.58 (L=64) → −1.00 (L=96)**, `r_eff ≈ L/2`
    (C≈0.94), R²≈0.89–0.95. **absorbing_boundary ON made no difference.** The field
    saturates the periodic box.
  - Damped (selective OFF): **p = −2.31 (L=64) → −2.19 (L=96)**, `r_eff ≈ 8–10`
    (C≈0.21), R²≈0.999. Localized and L-stable (Coulomb 1/r²-like).
  - The canonical −0.69 reproduces in **neither** default config; it sits inside the
    box-filling drift band of the lossless regime.
- **Prior-favoured outcome: HALO-TUNED (finite-size box artifact).** A FORCED lossless
  exponent would require the dark-matter halo to have a localized steady state in a
  periodic box — which the scout did not find. The §5 thresholds are fixed independent
  of the scout magnitudes.

## §5 · Frozen verdict logic (analyzer-encoded)

The verdict reads the **exponent** and the **localization ratio C** only; amplitude is
report-only. Over the frozen L-grid {64, 96, 128, 160}:

- **R0 — Forced-control (the instrument can resolve a forced exponent):** the DAMPED
  regime is localized at every L (`C < 0.5`) and L-convergent
  (`|p_off(160) − p_off(128)| ≤ 0.10`). PASS demonstrates the instrument *can* see a
  forced, L-stable exponent — so a null result in the lossless regime is meaningful.
- **R1 — Dark-matter (lossless) halo gate:**
  - **HALO-FORCED** iff the lossless regime is localized (`C < 0.5` at the two largest L)
    AND L-convergent (`|p_on(160) − p_on(128)| ≤ 0.10`).
  - **HALO-TUNED (finite-size)** iff the lossless regime is box-filling (`C > 0.8`) AND
    its exponent drifts (`|p_on(160) − p_on(128)| > 0.10`, or monotone drift across the
    grid).
- **R2 — Shape sub-check (on whichever regime is forced):** stencil ∈ {SC,FCC,BCC} and
  DAMPING ×{0.5,2} at L=128; report whether the forced exponent moves > 0.10 (a forced
  *geometric* invariant vs a calibration-dependent one).
- **Composite verdict:** **HALO-TUNED-BOUNDARY** if R0 passes (damped is forced) and R1
  returns HALO-TUNED (lossless box-fills) — i.e. the dark-matter halo shape is a
  finite-size artifact while the damped Coulomb near-field is the only forced self-field.
  **HALO-FORCED** if R1 returns HALO-FORCED. **INDETERMINATE** otherwise.

## §6 · Run of record (frozen invocation)

GPU build (`engine/build_wsl`, FFT-Gauss). The L=128/160 cells run ~5–10 min each.

```
# both regimes across the L-grid (the core L-convergence + localization test)
campaign_halo_forcedness --arm=det --Ls=64,96,128,160 --selective=on,off --toggles=minimal --ticks=1500 --knob=Lgrid --tag=v1
# shape sub-check (forced-regime geometry vs calibration), L=128
campaign_halo_forcedness --arm=det --L=128 --selective=off --stencil=sc,fcc,bcc --toggles=minimal --ticks=1500 --knob=stencil --tag=v1
python scripts/exploration/run_halo_constant_sweeps.py --constants=DAMPING --factors=0.5,2.0 --L=96 --ticks=1500 --tag=v1   # DAMPING on the localized regime
python scripts/exploration/analyze_halo_forcedness.py --csv engine/results/halo_forcedness/halo_forcedness_v1.csv --shells engine/results/halo_forcedness/halo_forcedness_shells_v1.csv
```
(The constant-sweep wrapper runs the damped/localized baseline; for the boundary the
selective-OFF regime is the forced control. L=160 cells may be split out with longer
timeouts.)

## §7 · Pre-declared outcomes

- **OUTCOME A:** R0 passes and R1 returns HALO-TUNED → **HALO-TUNED-BOUNDARY** (the
  prior-favoured result): the dark-matter halo shape is a periodic-box finite-size
  artifact; the SPARC target lacks a forced-shape foundation; only the damped Coulomb
  near-field is forced. A publishable boundary (cf. FTD-0269).
- **OUTCOME B:** R1 returns HALO-FORCED → the lossless halo *does* have a forced
  localized exponent; Step 2 (SPARC) is opened on that shape.
- **OUTCOME C (indeterminate):** R0 fails (the instrument cannot resolve a forced
  exponent even in the damped regime) → fix the instrument and re-register (v2).

## §8 · Pre-declared exclusions (banned moves)

1. The fit window r∈[7,23], `L_CONV_TOL=0.10`, `LOCALIZED=0.5`, `BOXFILL=0.8`, and the
   L-grid {64,96,128,160} are frozen — no post-hoc adjustment to move a verdict.
2. No sliding the fit window / hand-picking an r-subrange.
3. The lossless (selective-ON) regime is the dark-matter halo per §4.2; the verdict on
   it is not evaded by relabeling the damped regime as "the halo".
4. No near-miss search for a "nicer" exponent (−2/3, −0.7, −1).
5. Amplitude (`norm`, `J_peak`, `E_field`) is NEVER cited for or against forcedness.
6. A HALO-TUNED-BOUNDARY does **not** demote any algebraic-spine result, FTD-0110's
   linear k=¼ theorem, the dark-sector mechanism's *existence*, or the gravitational
   activity of the flux — it adjudicates only whether the halo *exponent* is forced.
7. Zero promotions: FTD-0013 `[SMC]`, MC-T4.3, FTD-0110/0261/0269, and the dark-sector
   claims are unchanged regardless of outcome.

## §9 · Hash-lock declaration

This document, the campaign instrument, the analyzer, and the constant-sweep wrapper are
committed together and tagged `preregister-halo-forcedness-v1` BEFORE the §6 run of
record executes. The §3 SHA256 hashes bind the artifact versions. Any post-lock edit to
§§2, 5, 6, 8 or to the artifacts invalidates the lock and requires a v2. Step 2 (SPARC
rotation-curve confrontation) is a separate deliverable under its own pre-registration,
gated on a HALO-FORCED verdict here.
