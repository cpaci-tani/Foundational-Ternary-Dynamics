# PREREGISTRATION — Localized-basin relaxation v2

**Identifier:** `FTD-0679`  
**Status:** `[PREREGISTERED — NOT YET EXECUTED]`  
**Date locked:** 2026-07-28  
**Branch:** selected connected Moore-block common action; observer only  
**Production changes:** forbidden

## Correction relative to v1

FTD-0678 stopped before recording tick zero.  Its preregistered identity
`D_phase=2 E_target` incorrectly equated a translation/boost-quotiented metric
with a target-mode energy that is not demeaned.  V2 removes that identity; it
does not loosen its tolerance.

The v1 invalid result is frozen by:

- protocol SHA256
  `3876E9CBF7017E68426C26E6829D4751513F183D3CA3EB6536C73E0718FFD156`;
- runner SHA256
  `2B99AA2B1F72628FEC8503EA65ADE57C81126857A138E43311F97846A6FF3D81`;
- invalid JSON SHA256
  `63348AEA5319376F1D2800ED1BB4B1CD7BEFA55C269C0FCB8E7A990706A733BE`;
- header-only CSV SHA256
  `298D0F86CA7A9C5150726013FC09BFBF77A1C28BE9E5CB681EDD2CD23CA6AF18`.

Because v1 produced no trajectory records, v2 retains the same `2.5e-7`
launch.  It is still fresh with respect to dynamical data.

## Frozen dynamics and observer

All state, dynamics, fit, shell, polarity, classifier, output, and toolchain
choices in FTD-0678 remain unchanged:

- periodic `L=97`, ticks `0..80`, first self-contact at tick 81;
- evolving control plus polarity signs `-1,+1`;
- first internal cubic doublet `{6,7}`;
- maximum constituent momentum `2.5e-7`;
- matched normalization, shared-anchor chart, sparse local current, exact
  selected common-action forward and state-only reverse;
- FTD-0677 localized observer with instantaneous control center,
  `R_in=8`, `R_out=24`, `omega=(omega_6+omega_7)/2`, matched `beta`,
  `c=C_SPEED`, and `m=M_INERTIAL`;
- `R_core=D_phase(t)/D_phase(0)`;
- fixed log-linear fit over ticks `8..64` versus a constant BIC model;
- core decline `>=0.20`; `Gamma_core>0`; `DeltaBIC>=10`; `R^2>=0.995`;
- remote-field gate at tick 80: `H_far>H_near` and `H_far>0`;
- polarity gates: relative rate difference `<=1e-4`, core-history RMS
  `<=1e-5`, far-fraction difference `<=1e-4`;
- unchanged ordered physical verdicts from FTD-0678, with the identifier
  updated to FTD-0679 in output records.

## Corrected initialization gates

For both signs require:

1. target energy is finite and positive;
2. quotiented `D_phase(0)` is finite and positive;
3. `|p_max-2.5e-7|<=1e-15`;
4. `R_core(0)=1` within `1e-12`;
5. the relative difference between the two signed values of `D_phase(0)` is
   `<=1e-12`;
6. the initial fields remain bitwise identical.

The historical quantity

```text
|D_phase(0)-2 E_target(0)| / max(D_phase(0),2 E_target(0))
```

is recorded only as the **quotient-comparison diagnostic**.  It is neither an
error nor a gate.

## Unchanged exact execution gates

- parent fingerprints, degenerate doublet, and observer preflight pass;
- every tick retains graph/charge/sector identity and a valid observer;
- maximum shell-partition residual `<=1e-12`;
- maximum selected-energy drift and common-action residual `<=1e-10`;
- complete forward/reverse recovery `<=1e-8`.

Any failure yields `LOCALIZED_BASIN_RELAXATION_V2_EXECUTION_INVALID` and no
physical classification.  Otherwise the ordered classifications are:

1. `LOCALIZED_BASIN_V2_INTERNAL_RELAXATION_ABSENT`;
2. `LOCALIZED_BASIN_V2_INTERNAL_RELAXATION_NONEXPONENTIAL`;
3. `LOCALIZED_BASIN_V2_REMOTE_FIELD_NOT_DOMINANT`;
4. `LOCALIZED_BASIN_V2_RELAXATION_SIGN_DEPENDENT`;
5. `LOCALIZED_BASIN_V2_RELAXATION_TOWARD_CONSTRUCTIVE`.

Even the constructive verdict states only finite-window approach toward the
registered control family plus a far-dominant positive difference-field norm.
It does not establish attraction, bound dressing, radiation, a particle pole,
or asymptotic object/environment separability.

## Run of record

- runner source and Release executable hashes are locked separately before
  invocation;
- CTest: `localized_basin_relaxation_v2`;
- outputs:
  `engine/results/ftd_0679/ftd_0679_localized_basin_relaxation_v2.json` and
  `engine/results/ftd_0679/ftd_0679_localized_basin_relaxation_ticks_v2.csv`;
- an independent standard-library Python verifier must recompute all fits,
  gates, and the verdict from the run of record.
