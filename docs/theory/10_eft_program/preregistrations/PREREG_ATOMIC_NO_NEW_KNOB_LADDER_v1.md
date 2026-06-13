# PRE-REGISTRATION -- No-new-knob atomic ladder (FTD-0283)

**Status:** `[PRE-REGISTRATION -- LOCK PENDING]` -- design authored; commit/tag
must precede the ladder run of record.  
**Date:** 2026-06-13  
**LEDGER id (reserved):** FTD-0283  
**Git tag (to be applied at lock):** `preregister-atomic-no-new-knob-ladder-v1`  
**Result class (declared):** `[FIXED-CELL DIMENSIONLESS SCALING TEST]`.

---

## 1. Question

With no new atomic knobs beyond FTD-0278/0279, does the conditional atomic sector
survive outside H/He on fixed dimensionless scaling tests?

FTD-0283 is not a laboratory spectroscopy campaign. It asks whether the same
frozen lattice operator and Hartree machinery remain coherent across a small,
predeclared ion ladder.

## 2. Frozen inputs

| Quantity | Value | Origin |
|---|---:|---|
| `omega0` | `1.5` | FTD-0278/0279 record clock scalar |
| `q_unit` | `0.3490` | FTD-0279 shallow record electron coupling |
| `L` | `48` | fixed tractable lattice size |
| hydrogenic `Z` | `{1, 2, 3}` | predeclared ladder |
| helium-like `Z` | `{2, 3}` | predeclared restricted-Hartree ladder |

No q scan, omega scan, L scan, tolerance scan, or target-line matching is
allowed in v1.

## 3. Frozen artifact

| Artifact | SHA256 |
|---|---|
| `scripts/exploration/atomic_next_three_campaigns.py` | `f7ef3f73427a90674d70695bbc875fb2de9984e77e691ea7160fd27c31925df8` |

## 4. Run of record

After this file and the artifact are committed and tagged:

```powershell
python scripts/exploration/atomic_next_three_campaigns.py --ftd-0283-ladder-record `
  --out scripts/exploration/results/atomic_next_three_2026-06-13/no_new_knob_ladder.json
```

## 5. Frozen verdict logic

- **L-H1:** PASS iff every hydrogenic cell is non-tachyonic.
- **L-H2:** PASS iff hydrogenic `gap12` is strictly monotone increasing with Z.
- **L-H3:** PASS iff hydrogenic `gap12/Z^2` spread is <= 0.25.
- **L-He1:** PASS iff every helium-like restricted-Hartree SCF cell converges.
- **L-SCOPE:** PASS iff the result document reports only dimensionless lattice
  quantities and makes no lab-unit or line-identification claim.

**Verdict:** `NO-NEW-KNOB-LADDER-CONFIRMED` iff L-H1, L-H2, L-H3, L-He1, and
L-SCOPE pass. Otherwise the result is a boundary/falsifier for no-new-knob
generalization, not an invitation to tune the ladder.

## 6. Banned moves

1. No q, omega0, L, or Z-set changes after lock.
2. No adding ion-specific constants.
3. No laboratory eV, wavelength, or NIST comparison.
4. No replacing failed cells with nearby cells.
5. No promotion of I1, I2, I3, FTD-0013, MC-T4.3, FC-1, or FTD-0270.

## 7. Hash-lock declaration

This document and `scripts/exploration/atomic_next_three_campaigns.py` must be
committed and tagged `preregister-atomic-no-new-knob-ladder-v1` before the
ladder run. Any post-lock edit to Sections 2-6 or to the artifact invalidates
v1 and requires a v2.
