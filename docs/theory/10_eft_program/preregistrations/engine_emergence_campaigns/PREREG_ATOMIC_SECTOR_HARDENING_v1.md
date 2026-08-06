# PRE-REGISTRATION -- Atomic-sector hardening and replay package (FTD-0280)

**Status:** `[PRE-REGISTRATION -- LOCK PENDING]` -- design authored; commit/tag must
precede any run of record.
**Date:** 2026-06-13
**LEDGER id (reserved):** FTD-0280
**Git tag (to be applied at lock):** `preregister-atomic-sector-hardening-v1`
**Context:** FTD-0278 (hydrogen-like operator spectroscopy) and FTD-0279 (helium
restricted Hartree SCF) opened a conditional atomic sector. This registration does
not add a physics claim. It turns those results into a reproducible package and
prevents the next steps from drifting into post-hoc spectroscopy.
**Result class (declared):** `[REPRODUCIBILITY / HARDENING]` for FTD-0278/0279,
with the underlying physics status unchanged:
`[CONDITIONAL -- DERIVED-GIVEN-IMPOSED-INPUT]`.

---

## §1 · Question

Can the conditional atomic sector be replayed and audited from locked artifacts,
with no hidden changes to the import register, no laboratory-line calibration, and no
unconditional "FTD derives QM/helium" language?

FTD-0280 answers only that reproducibility question. It is a bridgehead for the next
paradigm-pressure experiments; it is not itself a new atomic derivation.

## §2 · Frozen import register

This run inherits the FTD-0278/0279 register exactly:

| # | Input | Status |
|---|---|---|
| I1 | clock scalar `omega0 proportional to M_REST` | `[IMPOSED]` (FTD-0271; covariant rate FTD-native per FTD-0252/0271-A5) |
| I2 | scalar-potential coupling `omega_eff^2 = omega0^2 + 2 omega0 V` | `[IMPOSED -- motivated]` (FTD-0278) |
| I3 | mode-occupancy for two-electron atoms | `[IMPOSED -- motivated]` (FTD-0279); no exchange, no correlation |

The FTD-exact content remains the engine 18-point operator and the mean-free Gauss
Green's function (OT-1.4), used for Coulomb binding and, in FTD-0279, Hartree
repulsion. Nothing in this protocol may promote I1, I2, or I3.

## §3 · Frozen artifact

| Artifact | SHA256 |
|---|---|
| `scripts/exploration/atomic_sector_hardening.py` | `0575bc9154f3760f1cd6049f24f3ce5bf18bed6eeb5d3a0bd0c9b04585fdc83d` |

Locked dependencies checked by the harness:

| Artifact | Expected tag / SHA256 |
|---|---|
| `scripts/exploration/derive_hydrogen_lattice_spectrum.py` | tag `preregister-hydrogen-lattice-spectrum-v1` -> `6be49fe98a63164d50bb3c4dc6250ab4e9e2a33a`; SHA256 `8e953fac6b7dc251c21290f6e21d416c6e2a9d0e78d923a94e8953c73654573f` |
| `scripts/exploration/derive_helium_lattice_scf.py` | tag `preregister-helium-lattice-scf-v1` -> `310ad4ee2a275e0e6c5ecd00cbfd30b55e65f551`; SHA256 `ecfa2cd07cc23907867c2d97afcb6c1b1aeb0aa6506dc3e1308b16c912cd7714` |
| `PREREG_HYDROGEN_LATTICE_SPECTRUM_v1.md` | SHA256 `6d644aabce4cbd6e54fb159e776eb4e046dfd422ed115dd5710dd7fb2d21c792` |
| `PREREG_HELIUM_LATTICE_SCF_v1.md` | SHA256 `f49262b727fb9c29fc08af52f0f432cd052fc326b32a77c90cd4b9ad3691019a` |

## §4 · Run of record

After this file and the artifact are committed and tagged:

```powershell
python scripts/exploration/atomic_sector_hardening.py --verify-locks --manifest `
  --out scripts/exploration/results/atomic_sector_hardening_2026-06-13/manifest.json

python scripts/exploration/atomic_sector_hardening.py --replay-records `
  --out-dir scripts/exploration/results/atomic_sector_hardening_2026-06-13
```

The replay invokes only the locked record commands:

```powershell
python scripts/exploration/derive_hydrogen_lattice_spectrum.py --record `
  --out <out-dir>/hydrogen_spectrum_replay.csv

python scripts/exploration/derive_helium_lattice_scf.py --record `
  --out <out-dir>/helium_scf_replay.csv
```

## §5 · Frozen verdict logic

- **P-LOCK (provenance):** PASS iff every locked dependency in §3 exists, matches its
  SHA256, and any prereg tag resolves to the expected commit. Any mismatch gives
  `PROVENANCE-FAIL` and blocks all replay interpretation.
- **P-REPLAY (record replay):** PASS iff the hydrogen replay returns
  `HYDROGEN-CONFIRMED` and the helium replay returns `HELIUM-CONFIRMED` using the
  locked record scripts, with no edits to those scripts.
- **P-SCOPE (scope integrity):** PASS iff the result document states the cumulative
  import register, states that FTD-0270 stands unconditionally, and makes no
  physical-line, exchange, correlation, or unconditional-QM claim.

**Verdict:** `ATOMIC-SECTOR-REPLAY-CONFIRMED` iff P-LOCK and P-REPLAY and P-SCOPE
all pass. `PROVENANCE-FAIL` if P-LOCK fails. `REPLAY-FAIL` if P-LOCK passes but
P-REPLAY fails. `SCOPE-FAIL` if P-SCOPE fails. `ENV-INDETERMINATE` if the replay
cannot run because the local Python/SciPy environment is absent; P-LOCK still stands
or fails independently.

## §6 · Follow-on preregistrations this hardening enables

These are named here to prevent scope creep. They are not executed by FTD-0280.
Separate lock-pending preregistrations now own the implementation details.

1. **FTD-0281 -- engine-native live-clock Coulomb spectroscopy.**
   Default-off `db_clock_coulomb` toggle, live Gauss potential around a locked charge,
   time-series FFT peaks cross-checked against FTD-0278 operator eigenfrequencies at
   the same `(L, q)`. Golden gate must remain neutral with the toggle off. The
   pre-implementation audit says this is single-patch-feasible only if scoped to
   CPU/RenderBridge diagnostics: add the toggle in `engine/include/ftd/term_toggles.h`,
   use the existing `de_broglie_clock` KG branch in
   `engine/src/render_bridge_phases/phase_read.cpp`, and solve/populate
   `phi_coulomb_` through the existing Poisson path. The separate FTD-0281 v1
   prereg freezes the phase-order choice as a pre-read Coulomb solve. GPU parity,
   dashboard/UI exposure, and quantitative lab-energy claims are out of scope for
   v1.
2. **FTD-0282 -- exchange/correlation wall.**
   Keep I1+I2+I3 fixed and test the declared boundary: mean-field atoms should remain
   in scope, while correlation energy and ortho/para exchange splitting should remain
   absent unless a deeper statistics/configuration-space import is declared.
3. **FTD-0283 -- no-new-knob atomic ladder.**
   Using a single frozen scale convention, extend from H/He to a blind ladder of
   hydrogenic and helium-like ions. The admissible targets are dimensionless
   same-lattice ratios and scaling trends only; no laboratory eV/wavelength
   calibration and no NIST-line matching in the pre-calibration phase.

## §7 · Banned moves

1. No numerical near-miss or coincidence searches.
2. No post-hoc tolerance changes to FTD-0278 or FTD-0279 gates.
3. No edits to locked FTD-0278/0279 scripts as part of this replay.
4. No laboratory helium-line comparisons, no unit calibration, and no NIST matching in
   FTD-0280.
5. No exchange/correlation/ortho-para claim in FTD-0280.
6. No unconditional language: the atomic sector remains conditional on I1+I2+I3.
7. No promotions: FTD-0013 `[SMC]`, MC-T4.3, FTD-0270/0271/0278/0279, and FC-1 are
   unchanged regardless of outcome.

## §8 · Hash-lock declaration

This document and `scripts/exploration/atomic_sector_hardening.py` must be committed
and tagged `preregister-atomic-sector-hardening-v1` before §4 replay. Any post-lock
edit to §§2-7 or to the artifact invalidates v1 and requires a v2.
