# ANALYSIS -- Atomic paradigm campaign FTD-0280 through FTD-0283

> **CROSS-CUTTING CORRECTION** (see
> [`../03_derivations/foundational_mechanics/CORRECTION_FTD0278_HYDROGEN_MULTIPLET.md`](../../03_derivations/foundational_mechanics/CORRECTION_FTD0278_HYDROGEN_MULTIPLET.md)).
> The FTD-0278 hydrogen **"n=2 multiplet / O_h T1u triple / Rydberg ladder"** was found
> OVERCLAIMED and corrected to **HYDROGEN-1s-CONFIRMED**: in most record cells only the 1s
> is bound, and the "exactly degenerate T1u triple" is **torus momentum degeneracy, not
> bound 2p orbitals** (independently verified: the degeneracy survives a *repulsive* core).
> **Bearing on this campaign:** `gap12` is **1s-dominated** (the most-negative term is the
> 1s), so the **FTD-0283** `gap12/Z²` ladder is effectively a *1s-binding-vs-Z²* test — its
> falsification (spread 0.397 > 0.25) **STANDS**, now understood as the finite-lattice 1s
> not scaling cleanly as Z² (consistent with this campaign's NEGATIVE-BOUNDARY verdict).
> **FTD-0282** (exchange/correlation wall) is unaffected in substance (it concerns
> unrepresented physics, not the multiplet) but should not cite a confirmed n=2 multiplet.
> FTD-0281 (the engine hook) and the helium replay are unaffected.

**Status:** `[MIXED RESULT -- CONDITIONAL SECTOR HARDENED; NO-NEW-KNOB LADDER FALSIFIED]`  
**Lock commit:** `fe55b42f`  
**Tags:** `preregister-atomic-sector-hardening-v1`,
`preregister-db-clock-coulomb-spectroscopy-v1`,
`preregister-atomic-exchange-correlation-wall-v1`,
`preregister-atomic-no-new-knob-ladder-v1`  
**Result class:** `[CONDITIONAL -- DERIVED-GIVEN-IMPOSED-INPUT]` for the
atomic sector; `[NEGATIVE-BOUNDARY]` for the failed no-new-knob ladder gate.

---

## 1. Executive verdict

The atomic campaign did not deliver an unrestricted new paradigm. It delivered a
more valuable mixed result:

1. **FTD-0280 replay confirmed.** The locked hydrogen and helium records replayed
   with their expected verdicts: `HYDROGEN-CONFIRMED` and `HELIUM-CONFIRMED`.
2. **FTD-0281 engine hook confirmed.** The default-off `db_clock_coulomb` hook
   validates, reads the live Coulomb potential with the intended sign, and leaves
   the golden default-off engine path unchanged.
3. **FTD-0282 wall confirmed.** Under fixed I1+I2+I3 imports, exchange,
   ortho/para splitting, and correlation remain unrepresented rather than fitted.
4. **FTD-0283 ladder failed.** The fixed no-new-knob ion ladder did not pass its
   frozen Z²-scaling gate: `gap12/Z^2` spread was `0.3973916858`, above the
   preregistered `0.25` ceiling.

So the honest result is:

> **The conditional H/He atomic sector is reproducible and engine-hooked, but the
> simplest no-new-knob extension to a wider ion ladder is falsified at v1.**

No claim is promoted. FTD-0270, FTD-0271, FTD-0278, FTD-0279, FTD-0013,
MC-T4.3, and FC-1 remain unchanged.

## 2. FTD-0280 -- atomic-sector replay

Run artifacts:

- `scripts/exploration/results/atomic_sector_hardening_2026-06-13/manifest.json`
- `scripts/exploration/results/atomic_sector_hardening_2026-06-13/replay_summary.json`
- `scripts/exploration/results/atomic_sector_hardening_2026-06-13/hydrogen_spectrum_replay.csv`
- `scripts/exploration/results/atomic_sector_hardening_2026-06-13/helium_scf_replay.csv`

Frozen gates:

| Gate | Result |
|---|---|
| P-LOCK | PASS -- all inherited FTD-0278/0279 hashes and tags matched |
| P-REPLAY | PASS -- hydrogen returned `HYDROGEN-CONFIRMED`; helium returned `HELIUM-CONFIRMED` |
| P-SCOPE | PASS -- this analysis preserves conditional language and adds no physics claim |

**Verdict:** `ATOMIC-SECTOR-REPLAY-CONFIRMED`.

## 3. FTD-0281 -- live engine hook

Run artifacts:

- `scripts/exploration/results/atomic_next_three_2026-06-13/db_clock_coulomb_ctest.log`
- Engine hook files locked at commit `fe55b42f`.

Command:

```powershell
ctest --test-dir engine/build -j 24 -C Release -R "db_clock_coulomb|render_bridge_golden" --output-on-failure
```

Observed result: `2/2` tests passed.

Frozen gates:

| Gate | Result |
|---|---|
| D-VAL | PASS -- invalid dependency profiles are rejected; preregistered profile validates |
| D-PHI | PASS -- pre-read Coulomb solve populates the intended source potential |
| D-STEP | PASS -- `V=-phi_coulomb` changes the clock step in the attractive-well direction |
| D-GOLDEN | PASS -- `render_bridge_golden` passed with the toggle off |

**Hook verdict:** `DB-CLOCK-COULOMB-HOOK-CONFIRMED`.

This is not yet the FFT spectroscopy verdict. The live time-series peak
comparison against FTD-0278 operator eigenfrequencies remains a downstream run.

## 4. FTD-0282 -- exchange/correlation wall

Run artifact:

- `scripts/exploration/results/atomic_next_three_2026-06-13/exchange_correlation_wall.json`

Frozen gates:

| Gate | Result |
|---|---|
| W-SCOPE | PASS -- fixed I1+I2+I3 imports only |
| W-NEG | PASS -- exchange/correlation are reported as unrepresented, not fitted |
| W-LANG | PASS -- this is stated as a boundary, not a derivation upgrade |

**Verdict:** `EXCHANGE-CORRELATION-WALL-CONFIRMED`.

The wall is exactly where the conditional construction said it should be:
restricted Hartree mean-field is represented; dynamic Pauli exchange,
ortho/para exchange splitting, and correlation energy require additional
statistics/configuration-space structure not present under I1+I2+I3.

## 5. FTD-0283 -- no-new-knob ladder

Run artifact:

- `scripts/exploration/results/atomic_next_three_2026-06-13/no_new_knob_ladder.json`

Frozen inputs:

| Quantity | Value |
|---|---:|
| `omega0` | `1.5` |
| `q_unit` | `0.3490` |
| `L` | `48` |
| hydrogenic `Z` | `{1, 2, 3}` |
| helium-like `Z` | `{2, 3}` |

Frozen gates:

| Gate | Result |
|---|---|
| L-H1 non-tachyonic | PASS |
| L-H2 gap monotone in Z | PASS |
| L-H3 `gap12/Z^2` spread <= 0.25 | **FAIL** (`0.3973916858`) |
| L-He1 SCF converged | PASS |

Hydrogenic records:

| Z | q | n_bound | gap12 | gap12/Z² |
|---:|---:|---:|---:|---:|
| 1 | 0.349 | 1 | 0.0020785283 | 0.0020785283 |
| 2 | 0.698 | 1 | 0.0059071340 | 0.0014767835 |
| 3 | 1.047 | 2 | 0.0202061971 | 0.0022451330 |

Helium-like records:

| Z | E_He-like | E_nonint | sigma | SCF |
|---:|---:|---:|---:|---|
| 2 | -0.0063308831 | -0.0092187020 | 0.6867434390 | converged |
| 3 | -0.0295320533 | -0.0392226666 | 0.7529333402 | converged |

**Verdict:** `NO-NEW-KNOB-LADDER-NOT-CONFIRMED / Z2-SCALING-FAIL`.

The failure is informative. The H/He construction is not automatically a
scale-free atomic ladder. The next admissible move is not to tune q or relax the
gate; it is to pre-register a more precise scale-convention or continuum-limit
question.

## 6. Paradigm status

This campaign hardens a conditional atomic-sector bridge, but it does not close
the paradigm case.

What is stronger now:

- H/He replay is reproducible from locked artifacts.
- The engine now has a default-off live Coulomb-clock hook with golden-neutral
  behavior.
- The exchange/correlation boundary is explicit and confirmed under fixed
  imports.

What is weaker now:

- The simplest no-new-knob extension beyond H/He failed its preregistered
  scaling gate.
- The live FFT spectroscopy comparison is still unrun.
- Full quantum statistics/configuration-space structure remains outside the
  current import register.

The honest headline is:

> **FTD has a reproducible conditional atomic sector and a sharply mapped
> boundary, not yet a completed new atomic paradigm.**

## 7. Banned moves audit

No numerical near-miss search was run. No lab line or NIST comparison was made.
No tolerance was changed after lock. No failed FTD-0283 cell was replaced. No
claim was promoted.
