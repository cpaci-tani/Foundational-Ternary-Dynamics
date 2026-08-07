# AUDIT — Existing-force-branch reciprocity

**Date:** 2026-07-24  
**Identifier:** `FTD-0439`  
**Status:** `[MEASURED — EXISTING FORCE BRANCHES]` + `[CLOSED NEGATIVE — MAGNITUDE GRADIENT]` + `[PASS — LEGACY DIVERGENCE GRADIENT, FINITE PROTOCOL]` + `[FAILED GATE — POISSON NUMERICAL RECIPROCITY]`  
**Locked verdict:** `MOVEMENT_OR_PHASE_ORDER_DEFECT`  
**Pre-registration:** [`PREREG_FORCE_BRANCH_RECIPROCITY_v1.md`](../../10_eft_program/preregistrations/common_action_mechanics_reciprocity/PREREG_FORCE_BRANCH_RECIPROCITY_v1.md)  
**Run of record:** `engine/results/ftd_0439/windows_msvc_cpu_L33.csv`

## 1. Result

The three existing force branches do not share one reciprocity behavior.

| branch | max common motion | max particle momentum | max central-total residual | gate |
|---|---:|---:|---:|---|
| `G_C s grad|J|` | `0.628381254469` | `6.40504e-3` | `6.40504e-3` | fail |
| `-alpha s grad(div J)` | `2.45319e-17` | `2.59853e-19` | `3.50234e-18` | pass |
| Poisson `-alpha s grad(phi)` | `1.56007e-6` | `8.11011e-9` | `8.11011e-9` | fail |

All 18 arms completed with both particles present. The magnitude-gradient
result exactly reproduces FTD-0437/0438. The divergence-gradient branch is
balanced far below every locked tolerance. The Poisson branch misses the
particle and total gates, but by roughly six orders of magnitude less than the
magnitude-gradient branch.

## 2. What the locked verdict means

The preregistered verdict `MOVEMENT_OR_PHASE_ORDER_DEFECT` fires whenever the
Poisson branch fails. That label excludes a defect confined to the two direct
flux-force formulas. It does **not** establish movement as the unique cause.
The Poisson solver, its cold-start/warm-start policy, finite six-sweep default,
force evaluation, and phase ordering all remain candidate causes.

The Poisson drift is exactly unchanged under simultaneous polarity reversal.
Its direction depends on the pair axis and includes small cross-axis
components. This is the symmetry signature of a numerical/update-orientation
leak, not a polarity-mediated physical force. The production solver uses only
six warm-started SOR sweeps per tick; its own header says scientific Coulomb
benchmarks should use `20–30` iterations. SOR-convergence dependence is therefore
the next mandatory discriminator.

## 3. Strong conclusion about the selected force

The dominant defect is specific to the selected magnitude-gradient branch, not
to the common velocity integrator alone:

- the divergence-gradient branch uses the same force integration and movement
  machinery yet balances to `3.50e-18`;
- the selected branch fails at `6.41e-3` with exact cubic rotation and polarity-
  odd reversal;
- the selected failure is about `7.9e5` times larger than the maximum Poisson
  residual.

This strengthens FTD-0438. `G_C s grad|J|` is not an admissible conservative
matter-force law in the tested ontology. The failure cannot be blamed on the
particle integrator or lattice scan order generally.

## 4. Limited positive result

The legacy divergence-gradient rule passes the registered static-pair
reciprocity test without rescaling. That is structurally compatible with the
native field source because both use signed divergence/gradient information.
It is not thereby established as empirical electromagnetism, a correct Coulomb
law, or a variationally complete interaction. Its pair remains at separation
`8` over 200 ticks, so this campaign establishes balance but not useful force
strength or binding dynamics.

**Successor correction (FTD-0442):** source-level re-derivation subsequently
found sign and coupling-power disagreement between this production branch and
the declared Lagrangian. “Structurally compatible” here means only operator
shape; it does not license variational provenance.

The Poisson branch remains an imported instantaneous potential mechanism. Even
if its small leakage vanishes with solver convergence, it would be mechanically
balanced phenomenology rather than native retarded field exchange.

## 5. Reproducibility

- source SHA256: `31fdf537f1318f8771e515bde29f07b15e7652a28d0471d8bb08b2124595ec68`
- record SHA256: `5208ce64a627e128ecd86a208bcaa665c0bfae4b2be8f1390a2be8295f17c70e`
- compiler: pinned MSVC `14.44.35207`, Release
- backend: forced CPU, periodic `L=33`
- result: `MOVEMENT_OR_PHASE_ORDER_DEFECT`

## 6. Next discriminating test

Freeze the pair and measure the Poisson net force as a function of registered
SOR iteration count, both from a cold potential and after explicit pre-
relaxation. If the residual falls with solver convergence, the FTD-0439 Poisson
failure is a numerical initialization error. If it plateaus above roundoff, the
Poisson force/stencil has a structural reciprocity defect.

**Successor result:** FTD-0440/0441 close this question. The converged static
floor passes, and matched trajectory preparation suppresses the leak by more
than `7.1e4`; it is cold-start momentum memory.
