# PRE-REGISTRATION -- DB-clock Coulomb live-engine spectroscopy (FTD-0281)

**Status:** `[PRE-REGISTRATION -- LOCK PENDING]` -- design and hook smoke
authored; commit/tag must precede any spectroscopy run of record.  
**Date:** 2026-06-13  
**LEDGER id (reserved):** FTD-0281  
**Git tag (to be applied at lock):** `preregister-db-clock-coulomb-spectroscopy-v1`  
**Result class (declared):** `[ENGINE-DIAGNOSTIC / CONDITIONAL-SPECTROSCOPY]`;
underlying atomic status remains
`[CONDITIONAL -- DERIVED-GIVEN-IMPOSED-INPUT]`.

---

## 1. Question

Given the FTD-0278 import register, can the live CPU `RenderBridge` time
evolution reproduce the same Coulomb-clock mode content as the frozen operator
experiment, without fitting laboratory lines or changing the import surface?

FTD-0281 is not a new derivation of quantum mechanics. It asks whether the
operator construction can be moved into the engine tick loop as a live diagnostic.

## 2. Frozen semantic choice

The v1 phase-order choice is **pre-read Coulomb solve**:

1. `tick()` validates toggles and syncs the ternary state.
2. If `db_clock_coulomb` is enabled, `solve_coulomb_poisson()` runs before
   `phase_read()`.
3. `phase_read()` applies the all-site diagonal KG term
   `delta_j -= (omega0^2 - 2*omega0*phi_coulomb[i]) * J`.

This matches the FTD-0278 convention `omega_eff^2 = omega0^2 + 2*omega0*V`
with `V = -phi_coulomb` in the engine force sign convention.

## 3. Scope locks

- CPU/RenderBridge only in v1.
- Single-substrate only: `dual_substrate=false`.
- Forces off: `forces=false`, so the pre-read solve is the only same-tick
  Coulomb solve.
- `wave_propagation=true`, `poisson_coulomb=true`, `de_broglie_clock=true`,
  `db_clock_coulomb=true`.
- No dashboard/UI exposure, GPU parity, lab-energy calibration, or NIST-line
  comparison in v1.

## 4. Frozen artifacts

| Artifact | SHA256 |
|---|---|
| `engine/include/ftd/term_toggles.h` | `07986de09a8babc6a49d1bf31eb55849da8131c44ab1cb613a0712ed5a5c0884` |
| `engine/src/render_bridge.cpp` | `c375c398a6b155ba5eb201e4609d7f03956ed6be367ce8395e4686fdaf24d46c` |
| `engine/src/render_bridge_phases/phase_read.cpp` | `c6f109cd553086fbc5168e4c252b7db9e05ec4ef459da2b0048a4f53a3327cdd` |
| `engine/tests/test_db_clock_coulomb.cpp` | `1be528966c02d21b739677e46735d39ad22e3c636d709b4254c49f5017b8fd28` |
| `scripts/exploration/atomic_next_three_campaigns.py` | `f7ef3f73427a90674d70695bbc875fb2de9984e77e691ea7160fd27c31925df8` |

## 5. Run of record

After this file and the artifacts are committed and tagged:

```powershell
cmake --build engine/build --config Release --target test_db_clock_coulomb --parallel 24
cd engine/build
ctest -j 24 -C Release -R "db_clock_coulomb|render_bridge_golden" --output-on-failure
```

The hook smoke is not the spectroscopy verdict. The later FFT campaign must use
the same toggle profile and compare live time-series peaks against the FTD-0278
operator frequencies at the same `(L, source charge convention, omega0)`.

## 6. Verdict logic

- **D-VAL:** PASS iff the preregistered toggle profile validates and invalid
  dependency profiles are rejected.
- **D-PHI:** PASS iff the pre-read Coulomb solve populates a source potential
  with the correct attractive-well sign.
- **D-STEP:** PASS iff a uniform positive clock field steps down less near the
  positive source than far away, as required by `V=-phi_coulomb`.
- **D-GOLDEN:** PASS iff `render_bridge_golden` remains unchanged with the
  toggle off.

**Hook verdict:** `DB-CLOCK-COULOMB-HOOK-CONFIRMED` iff D-VAL, D-PHI, D-STEP,
and D-GOLDEN pass.  
**Spectroscopy verdict:** reserved for a later locked FFT campaign; this v1
document does not claim it.

## 7. Banned moves

1. No lab-line matching or unit calibration.
2. No changing `omega0`, source convention, or tolerances after lock.
3. No dual-substrate, force-on, GPU, or UI claim in v1.
4. No treating a hook-smoke pass as an atomic-spectrum verdict.
5. No promotion of I1, I2, I3, FTD-0013, MC-T4.3, FC-1, or FTD-0270.

## 8. Hash-lock declaration

This document and the artifacts in Section 4 must be committed and tagged
`preregister-db-clock-coulomb-spectroscopy-v1` before any FTD-0281 run of
record. Any post-lock edit to Sections 2-7 or to the artifacts invalidates v1
and requires a v2.
