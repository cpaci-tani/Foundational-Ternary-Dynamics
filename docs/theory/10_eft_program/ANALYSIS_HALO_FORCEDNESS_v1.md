# ANALYSIS: Halo-exponent forcedness audit (FTD-0300)

**FTD ID:** FTD-0300
**Status:** `[MEASUREMENT ANALYSIS — INDETERMINATE (frozen); SPARC boundary]`
**Pre-registration:** [`PREREG_HALO_FORCEDNESS_v1.md`](preregistrations/PREREG_HALO_FORCEDNESS_v1.md)
**Lock tag:** `preregister-halo-forcedness-v1`  **Lock commit:** `168148e0`
**Artifacts (SHA256 in the pre-reg §3):** `engine/tests/campaign_halo_forcedness.cpp`,
`scripts/exploration/analyze_halo_forcedness.py`, `scripts/exploration/run_halo_constant_sweeps.py`
**Golden:** `test_render_bridge_golden = 0x56fa28acb5b9fe88` green with the new TU (observation-only).

---

## 0 · Verdict

**Frozen analyzer verdict: INDETERMINATE** (R0 PASS, R1 INDETERMINATE). The scout
(L=64/96 only) suggested a clean finite-size drift; the run of record (L=64/96/128/160)
overturned that lean, and the hash-locked analyzer reported the honest in-between case —
**not** the "HALO-TUNED" the prior leaned toward.

The **lossless self-field** — the §4.2 dark-matter halo (`selective_damping = ON`) —
**box-fills** the periodic lattice (`r_eff ≈ L/2`, C ≈ 0.91–0.96 at every L: not a
localized object), **yet its windowed exponent over r∈[7,23] converges** to **−1.25** at
L ≥ 128 (Δp(160−128) = 0.0008). So it is neither a forced *localized* halo (it is not
localized) nor a simple L-divergent artifact (the windowed slope has a large-L limit) —
it falls between the pre-registered FORCED and TUNED bins → **INDETERMINATE**.

Two conclusions are firm regardless of the bin:

1. **The doc's −0.69 is falsified.** −0.69 is the L=64 transient; the converged windowed
   exponent is **−1.25** (−0.58 → −1.00 → −1.25 → −1.25 across L = 64/96/128/160).
2. **The SPARC target is not founded.** The dark-matter halo box-fills — `r_eff ≈ L/2`
   has no intrinsic localized scale (the scale is the box), so it cannot produce a
   galaxy-anchored rotation curve. The **only forced, localized self-field** is the
   damped (`selective_damping = OFF`) Coulomb near-field: localized (C ≈ 0.16–0.26),
   L-convergent to **−2.15**, R² ≈ 0.999 — the lattice Green's-function gradient, which
   is *not* the dark-matter object.

**Zero promotions.**

## 1 · Run of record

GPU/FFT-Gauss build (`engine/build_wsl`), minimal toggle stack (validated bit-equal to the
canonical `full` stack for `|J|`), locked +1 particle, ticks = 1500 (≫ box wrap time at
every L). `C = r_eff / (L/2)`.

```
campaign_halo_forcedness --arm=det --Ls=64,96,128,160 --selective=on,off --toggles=minimal --ticks=1500 --knob=Lgrid --tag=v1
campaign_halo_forcedness --arm=det --L=128 --selective=off --stencil=sc,fcc,bcc --toggles=minimal --ticks=1500 --knob=stencil --tag=v1
python scripts/exploration/analyze_halo_forcedness.py --csv .../halo_forcedness_v1.csv --shells .../halo_forcedness_shells_v1.csv
```

| L | LOSSLESS (sel ON) p | C | kind | DAMPED (sel OFF) p | C | kind |
|---|---|---|---|---|---|---|
| 64 | −0.577 | 0.96 | box-fill | −2.305 | 0.26 | localized |
| 96 | −1.002 | 0.94 | box-fill | −2.187 | 0.21 | localized |
| 128 | **−1.247** | 0.92 | box-fill | −2.160 | 0.18 | localized |
| 160 | **−1.248** | 0.91 | box-fill | −2.150 | 0.16 | localized |
| Δ(160−128) | **0.0008** | | (converged) | 0.0094 | | (converged) |
| R² | 0.89–0.97 | | | 0.998–0.9995 | | |

- **R0 — forced-control:** damped localized at the two largest L and convergent
  (Δp = 0.0094 ≤ 0.10) → **PASS**. The instrument resolves a forced exponent.
- **R1 — lossless gate:** box-fill at the two largest L (C > 0.8) AND convergent
  (Δp = 0.0008 ≤ 0.10) → **INDETERMINATE** (box-fill ⇒ not HALO-FORCED; convergent ⇒ not
  HALO-TUNED).
- **R2 — stencil sub-check (report):** `bcc_stencil ∈ {sc,fcc,bcc}` returned values
  **byte-identical** to `full` (p = −2.159674 in all four). **Non-probative:** the toggle
  is a no-op in the selective-OFF / minimal path, so this is not evidence of geometric
  invariance — it is evidence the sub-stencil did not engage here. (The DAMPING sub-check
  via `run_halo_constant_sweeps.py` is a registered follow-up, not run in v1.)

## 2 · The two regimes

- **Lossless (selective ON) — the dark-matter halo (§4.2).** Coupling `G_C·∇s` injects
  flux at the locked charge's neighbours; selective damping removes flux only at r ≤ 1,
  so the far field propagates undamped and, in a periodic box, saturates it (front wraps
  at t ≈ L·√3/2 ≪ 1500). Hence `r_eff ≈ L/2` at every L. The exponent over the fixed
  window r∈[7,23] measures the near-source gradient on the box-filled background; it
  *does* approach a large-L limit (−1.25) because the window is small and fixed while the
  box-filled background dilutes, but the field itself is never localized.
  `absorbing_boundary = ON` produced identical numbers (the sponge did not drain the
  saturated box at this tick count).
- **Damped (selective OFF) — the forced control.** Uniform damping at rate `DAMPING = α`
  gives the localized lattice Coulomb field: `r_eff ≈ 8–13` (C ≈ 0.16–0.26),
  L-convergent to −2.15, R² ≈ 0.999. This forced-control proves the instrument can
  resolve a forced, localized exponent — so the lossless INDETERMINATE is a property of
  the lossless field, not an instrument failure.

## 3 · Interpretation

The dark-matter halo, as a *localized object with a forced radial shape*, does not exist
in the engine's periodic box: the lossless field has no localized steady state there. Its
windowed slope has a large-L limit (−1.25), which is what makes the verdict INDETERMINATE
rather than a clean finite-size-drift TUNED — but a convergent *windowed* slope on a
*box-filling* field is not a forced halo profile. The doc's −0.69 / r_eff = 15 figure is
a particular small-L/early-tick state, falsified by the converged −1.25.

This sits beside the FTD-0269 result: the engine's forced Green's-function shape is real
(the damped Coulomb −2.15, localized, L-stable), but the *dark-matter halo* the framework
wants for SPARC is set by the box/regime, not by a forced localized law. The algebraic
spine, framework integers, and the dark-sector mechanism's *existence* (flux gravitates)
are untouched; what is bounded is the specific claim that the dark-matter halo has a
forced radial exponent suitable for a rotation-curve prediction.

## 4 · Consequence for the dark-matter / SPARC program

Step 2 (blind SPARC rotation-curve confrontation) was gated on **HALO-FORCED**. The
verdict is INDETERMINATE, and operationally the dark-matter halo **box-fills** (no
intrinsic localized scale), so **Step 2 does not proceed on the lossless halo as-is** — a
rotation curve built from a box-saturated, scale-free profile would not test a forced
prediction. Honest open routes, each its own future pre-registration:

1. Give the lossless halo a genuine localized steady state — a physically motivated
   open boundary that demonstrably drains the box (the thin sponge tested here did not),
   or a confinement mechanism — then re-test forcedness and re-measure the exponent.
2. Adopt the **damped Coulomb** field (−2.15, forced, localized) as the halo and ask
   whether *that* shape, with its `DAMPING`-set localization length, has SPARC content
   (its windowed exponent depends on `DAMPING` — the registered R2 DAMPING sub-check).
3. Accept the boundary: the engine's dark-matter halo is not a forced localized shape,
   and the SPARC target is not founded — the map is the deliverable.

## 5 · Claim status (no promotions)

| Claim | Status after FTD-0300 |
|---|---|
| Dark-matter halo exponent (lossless regime) | `INDETERMINATE` — box-filling (not localized) but windowed slope converges to −1.25 |
| `DERIV_DARK_SECTOR_DYNAMICS.md` §4.1 value −0.69 | **falsified** — the converged windowed exponent is −1.25; −0.69 is an L=64 transient |
| Damped Coulomb near-field (≈ −2.15) | forced/localized control (lattice Green's function), L-convergent |
| Dark-sector mechanism *existence* (§4.2) | unchanged — flux gravitates; only the *exponent's forcedness/usability* is bounded |
| FTD-0013 `[SMC]`, MC-T4.3, FTD-0110/0261/0269 | unchanged |
| SPARC rotation-curve target (Step 2) | not opened (gated on HALO-FORCED; halo box-fills, no localized scale) |
