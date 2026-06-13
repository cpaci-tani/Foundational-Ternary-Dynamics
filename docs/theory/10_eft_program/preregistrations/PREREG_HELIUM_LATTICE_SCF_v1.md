# PRE-REGISTRATION — Helium on the FTD lattice: mean-field SCF (FTD-0279)

**Status:** `[PRE-REGISTRATION]` — design lock; run of record follows the hash-lock.
**Date:** 2026-06-12
**LEDGER id (reserved):** FTD-0279
**Git tag (to be applied at lock):** `preregister-helium-lattice-scf-v1`
**Context:** the helium challenge is what produced FTD-0270's verdict ("atomic dynamics
~0% substrate-derived"). FTD-0278 (HYDROGEN-CONFIRMED, lock `6be49fe9`) opened the
conditional sector. This registration asks the next question with one further declared
import.
**Result class (declared):** `[CONDITIONAL — DERIVED-GIVEN-IMPOSED-INPUT]`. Hartree
mean-field for helium is textbook 1928; NEVER to be cited as "FTD derives helium"
unconditionally. FC-1 stands; FTD-0270's unconditional boundary stands.

---

## §1 · Question

GIVEN the FTD-0278 register (clock + scalar-potential coupling) **plus the
mode-occupancy import below**, does the engine's exact machinery produce a
helium-like two-electron atom — correct screening physics (σ → 0.7154), correct
ionization ordering (I_He/I_He⁺ → 0.4519), with the nuclear attraction AND the
electron-electron repulsion both supplied by the engine's own Green's function?

## §2 · The cumulative import register

| # | Input | Motivation |
|---|---|---|
| I1 | clock scalar ω₀ = 1.5 ∝ M_REST | FTD-0271 `[IMPOSED]`; covariant rate FTD-native |
| I2 | scalar-potential coupling ω_eff² = ω₀² + 2ω₀V | FTD-0278 `[IMPOSED]`; the engine's gravity-clock move applied to the Gauss φ |
| **I3** | **mode-occupancy (NEW):** "two electrons" = two unit-norm occupations of the field's bound modes, each sourcing the engine's Gauss law and feeling the OTHER's Hartree potential | a classical field J(x) carries **no two-particle configuration space (L⁶)**; occupancy-with-mutual-sourcing is the minimal slice of second quantization a 2-electron atom needs. Ground state 1s²: spatially symmetric, spin-singlet bookkeeping imported. **No exchange term, no correlation claimed.** |

**The boundary beyond (declared in advance):** correlation energy (continuum −0.042
Hartree, 1.4% of E_He) is genuine configuration-space **entanglement** the classical
substrate does not carry — FC-1 territory. **Mean-field is the ceiling of I3; finding
that ceiling is part of the deliverable.**

## §3 · Engine-exact ingredients

18-pt Laplacian (G-1-verified symbol); nuclear well `V_nuc = +2q·φ_G` AND e-e Hartree
repulsion `V_H = −q·(φ_G ⊛ ρ)` **both from the same mean-free periodic lattice Green's
function** (OT-1.4 `[THEOREM]` — the engine's Gauss solution; the convolution runs
against the engine symbol). Restricted Hartree SCF (density mixing 0.5, tol 1e-9);
`E_He = 2ε − E_ee`; He⁺ = the one-body problem at 2q (= E_nonint/2 by construction).
Reference: the **spectral periodized continuum Coulomb** (IFFT of −1/|k|² — same torus,
same FFT, full image sum; only the symbol differs from M(k)).

## §4 · Frozen artifact

| Artifact | SHA256 |
|---|---|
| `scripts/exploration/derive_helium_lattice_scf.py` | `ecfa2cd07cc23907867c2d97afcb6c1b1aeb0aa6506dc3e1308b16c912cd7714` |

`record_run()` encodes §6; the run of record is `--record` (mechanical).

## §5 · Run of record (frozen) + prior information (disclosed)

```
python scripts/exploration/derive_helium_lattice_scf.py --record \
    --out scripts/exploration/results/helium_scf_2026-06-12.csv
```

Grid: ω₀ = 1.5; q ∈ {0.4654, 0.3490} (hydrogen a₀ = {6, 8}; He 1s ≈ {3.6, 4.7});
L ∈ {48, 64}; lattice + spectral-reference arms; V_ee-off controls; independent-eigenpath
He⁺ cross-checks.

**Disclosed development (git history before this lock):** L = 48 cells measured —
σ_ENG = {0.6767, 0.6867}, ion_ENG = {0.3534, 0.3735}, dimensionless ENG−REF diffs
|dσ| = {0.0144, 0.0022}, |d_ion| = {0.0288, 0.0044}. **The L = 64 cells have never been
run** — the F-He-B trend leg at L = 64 is the blind component. Also disclosed: absolute
E_He is core-convention-sensitive (three reference conventions gave three absolute
energies — minimum-image, core-matched, spectral), while the dimensionless observables
agree across conventions to ~2%; the falsifiers therefore live on dimensionless
observables only, and the F-He-A bands are set ~2× the largest observed L=48 deviation.

## §6 · Frozen falsifiers and verdict logic (encoded in `record_run()`)

- **F-He-A (the engine's potential does helium like 1/r — dimensionless):**
  |σ_ENG − σ_REF| ≤ 0.03 AND |ion_ENG − ion_REF| ≤ 0.05 in **all 4** cells.
- **F-He-B (screening — the Z_eff = 27/16 physics):** σ_ENG ∈ (0.60, 0.80) in all
  cells AND, at L = 64 (**blind**), σ(a₀=8) is at least as close to the continuum
  0.7154 as σ(a₀=6) — the convergence-to-continuum trend.
- **F-He-C (ionization ordering):** I_He/I_He⁺ ∈ (0.30, 0.60) in all cells
  (continuum 0.4519; the second electron is bound but ~half as tightly).
- **F-He-D (independent-eigenpath cross-check):** the He⁺ energy from the locked
  hydrogen module's eigenpath equals this script's E_nonint/2 to 1e-6 relative,
  all cells.

**Verdict:** HELIUM-CONFIRMED iff F-He-A ∧ F-He-B ∧ F-He-C ∧ F-He-D; PARTIAL if
F-He-A ∧ F-He-D but ¬(F-He-B ∧ F-He-C); CLOSED-NEGATIVE otherwise.

## §7 · Pre-declared exclusions (banned moves)

1. No absolute-energy falsifiers (core-convention-sensitive — measured, disclosed);
   absolute ENG/REF reported descriptively only.
2. No post-hoc band adjustment; protocol changes require v2.
3. No exchange/correlation claims; the ortho/para (1s2s singlet-triplet) splitting
   requires the exchange integral — a deeper statistics import, explicitly NOT
   attempted here (queued as a possible FTD-0280-class follow-up).
4. No unconditional language: the verdict conditions on the full I1+I2+I3 register.
5. HELIUM-CONFIRMED promotes nothing: FTD-0013 [SMC], MC-T4.3, FTD-0270/0271/0278
   unchanged; FC-1 stands — indeed I3's ceiling (no correlation) is an FC-1
   *illustration*.

## §8 · Honest ceiling

Even HELIUM-CONFIRMED yields: "GIVEN the clock + the coupling + mode-occupancy, the
engine's exact lattice machinery produces a mean-field helium atom with the right
screening and ionization structure." It does NOT derive the occupancy structure (no
Fock space in the substrate), does NOT reach correlation (the declared FC-1 boundary),
and claims no absolute calibration.

## §9 · Hash-lock declaration

This document and the §4 artifact are committed together; the commit is tagged
`preregister-helium-lattice-scf-v1` BEFORE the §5 run executes. Any post-lock edit to
§§2–7 or the artifact invalidates the lock and requires a v2.
