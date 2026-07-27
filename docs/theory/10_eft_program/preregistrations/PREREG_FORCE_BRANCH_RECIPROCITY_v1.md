# PRE-REGISTRATION — Existing-force-branch reciprocity v1

**Date locked:** 2026-07-24  
**Identifier:** `FTD-0439`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parent:** `FTD-0438` central-generator field-momentum recoil  
**Engine artifact:** `engine/tests/campaign_force_branch_reciprocity.cpp`  
**Artifact SHA256:** `31fdf537f1318f8771e515bde29f07b15e7652a28d0471d8bb08b2124595ec68`

## 1. Question

FTD-0438 established that the selected magnitude-gradient force produces
particle momentum with no compensating momentum in the native central local
flux generator. FTD-0439 asks:

> Is the failure specific to `G_C s grad|J|`, shared by both direct flux-force
> branches, or caused by the common movement/phase-order machinery?

The comparison uses only force modes already present in the production engine.
It does not add or tune a force.

## 2. Frozen matrix

| Quantity | Value |
|---|---:|
| lattice | periodic `L=33` |
| ticks | `200` |
| pair separation | `8` |
| pair axes | `x,y,z` |
| pair orientations | low-coordinate `+1` and low-coordinate `-1` |
| RNG seed | `4390` |
| particle-momentum balance gate | `1e-10` |
| common-displacement balance gate | `1e-8` |
| total central-momentum balance gate | `1e-10` |

Every arm enables `wave_propagation`, `coupling`, `forces`, `movement`, and
`strict_validation`. The three registered branches are:

1. magnitude-gradient: `emergent_forces=true`, `poisson_coulomb=false`;
2. divergence-gradient: both force-selection toggles false;
3. Poisson: `emergent_forces=false`, `poisson_coulomb=true`.

Every other Boolean extension is disabled. All arms use positive-first
injection and forced CPU execution. FTD-0437 already established injection-order
independence for the magnitude-gradient branch; this campaign does not broaden
that conclusion to the other branches.

## 3. Frozen observables

For each branch/axis/orientation arm, record pair center-of-mass displacement,
minimum separation, production particle momentum, and the FTD-0438 field term

$$
P_i^{\rm field}=-\sum_x W\cdot D_iJ.
$$

A branch is particle-balanced only if both its maximum particle momentum and
maximum common displacement pass their gates in every arm. It is central-total-
balanced only if

$$
\max_t|P^{\rm particle}+P^{\rm field}-P^{\rm total}(0)|\le10^{-10}
$$

in every arm. No multiplicative matching coefficient is permitted.

## 4. Locked outcomes

- `MAGNITUDE_GRADIENT_SPECIFIC_DEFECT`: magnitude-gradient fails while both
  divergence-gradient and Poisson pass particle and total balance.
- `FLUX_FORCE_FAMILY_DEFECT`: both direct flux-force branches fail while
  Poisson passes.
- `MOVEMENT_OR_PHASE_ORDER_DEFECT`: the Poisson branch also fails either
  balance test.
- `NO_DEFECT_REPRODUCED`: all three branches pass.
- `MIXED_BRANCH_OUTCOME`: a valid pattern not covered above.
- `INVALID_PROTOCOL`: any particle loss, nonfinite output, toggle/backend
  mismatch, or incomplete arm.

## 5. Interpretation boundary

A balanced Poisson result establishes only pairwise mechanical symmetry for an
instantaneous imported potential branch. It does not make that branch native,
retarded, or gauge-derived. A balanced divergence-gradient result establishes
only finite-protocol reciprocity, not empirical electromagnetism. A failed
branch is disqualified from conservative mechanics in this protocol unless a
separately derived momentum channel is preregistered.

## 6. Banned moves

- No coefficient, stencil, force, source, normalization, duration, separation,
  or gate changes after first execution.
- No post-hoc subtraction of center-of-mass drift.
- No symmetrized movement order.
- No promotion of a mechanically balanced branch to physical electromagnetism.
