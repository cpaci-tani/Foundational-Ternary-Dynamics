# FTD-0674 — Recovery-reservoir donor discriminator v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION]`  
**Production status:** unchanged; observer-only large-buffer execution  
**Parent FTD-0672 JSON SHA256:**
`E3EFB78EC36F32FEFE7627A3EE368E2A5A700BCE0890FBEF1E27D2D8E9B414D3`  
**Parent FTD-0672 tick CSV SHA256:**
`C4339D5985F4EB36DFE2F0DDF28A4151C805D4EC5913ED08EAF1A601F84F5C8E`  
**Observer theorem:** `FTD-0673`

## 1. Question and prediction

FTD-0672 excludes inward field return through radii 8, 16, and 24 during the
fixed recovery window, while the source continues transferring energy into an
outward field channel. Conservation then requires another selected reservoir
to fund the mode-6/7 doublet rise. FTD-0673 supplies the exact nonoverlapping
ledger needed to identify that donor.

Run one fresh half-amplitude history. The prior-favoured prediction is that the
exact nonlinear matter remainder is the dominant donor, with the exact binding
difference decreasing over the same interval. Other tangent modes, dynamic
field self energy, field interference, a distributed donor, and a mixed result
remain live outcomes.

## 2. Frozen protocol

- Use `L=97`, horizon `T=80`, the same unexcited control and recentered
  FTD-0638 geometry, the selected connected-block common action, the
  default-off exact sparse-current path, and the FTD-0640 complete tangent
  basis about the control.
- Use both signs of the mode-6 momentum kick at maximum constituent momentum
  amplitude `1e-6`, one half of FTD-0672. Initial face/edge fields must be
  bitwise equal to the control.
- The control and two sign histories may run concurrently. Each path retains
  an independent state and solve cache. Within-path arithmetic and transaction
  order remain serial; verdict reduction remains serial.
- Require the parent fingerprints above, valid normalization and complete
  mass-orthonormal mode basis, sector/fibre preservation, common residual and
  complete-energy drift `<=1e-10`, exact-reservoir residual `<=1e-10`,
  state-only forward/reverse recovery `<=1e-8`, complete schemas, and polarity
  agreement below.
- At ticks `0..80`, evaluate FTD-0673 between the common control path and each
  sign path with target modes `{6,7}`. Normalize every reservoir by that sign
  arm's tick-zero target-mode energy.
- Record the exact additive ledger

  ```text
  Delta E = E_target + E_other + R_matter + H_dynamic + I_field
  ```

  together with the second, non-additive macroscopic description

  ```text
  Delta E = Delta K_rel + Delta V_bind + Delta H_field.
  ```

  Never add terms across those two descriptions.
- Freeze the donor interval to tick `72` through tick `78`, the primary and
  second-post troughs fixed by FTD-0670/0672 before this new amplitude is run.
  For each additive reservoir `R_i`, define

  ```text
  delta_i = R_i(78)-R_i(72),
  D_i = max(-delta_i,0),
  D_total = sum_(i != target) D_i.
  ```

  The target is a recipient and is excluded from the donor sum. The four donor
  candidates are `E_other`, `R_matter`, `H_dynamic`, and `I_field`.
- Require target recovery `delta_target>=0.05`, at least one normalized donor
  contribution `D_i>=0.01`, and interval closure

  ```text
  |sum_i delta_i| <= 1e-8.
  ```

- Define donor fractions only when `D_total>0`:
  `f_i=D_i/D_total`. Between signs require every tick-72 and tick-78
  normalized additive reservoir, every `delta_i`, every `f_i`, and each of
  `Delta K_rel`, `Delta V_bind`, and `Delta H_field` to agree within `1e-4`.
  The same donor class must be obtained for both signs.
- Also record the complete fixed window `68..80`, but do not use it to replace
  or repair the primary `72..78` verdict.

No amplitude, endpoint, basis, normalization, donor list, threshold, or class
may change after viewing this history.

## 3. Locked donor classes

Among the four donor candidates, choose a unique dominant reservoir only if
its fraction is at least `0.60` and every other fraction is below `0.60`:

- `OTHER_TANGENT_MODE_DONOR`;
- `NONLINEAR_MATTER_REMAINDER_DONOR`;
- `DYNAMIC_FIELD_SELF_ENERGY_DONOR`;
- `FIELD_INTERFERENCE_DONOR`.

If no reservoir is dominant:

- `DISTRIBUTED_RESERVOIR_DONOR` if at least two donor fractions are each at
  least `0.20`;
- `RESERVOIR_DONOR_MIXED` otherwise.

The exact binding cross-check is reported separately:

- `BINDING_DECREASE` if `Delta V_bind(78)-Delta V_bind(72)<=-0.01`;
- `BINDING_INCREASE` if that change is `>=+0.01`;
- `BINDING_BALANCED` otherwise.

These normalized binding thresholds do not identify `R_matter` with binding.

## 4. Locked verdicts

- Any fingerprint, initialization, basis, observer, action, energy, sector,
  inverse, schema, polarity, recovery, donor-size, or interval-closure failure:
  `RECOVERY_RESERVOIR_DONOR_EXECUTION_INVALID`.
- Execution passes and the signs return the same one of the six donor classes:
  `RECOVERY_RESERVOIR_<CLASS>` with the binding cross-class recorded
  independently.

A valid mixed or distributed verdict is a result and may not be relabelled by
changing the interval or combining the non-additive ledgers.

## 5. Interpretation boundary

This campaign can identify which term in the selected control-relative energy
ledger decreases while the target doublet rises. It cannot by itself turn the
nonlinear remainder into a primitive, name it as binding, prove a constituent
ontology, establish a pole, or show separability. A dominant nonlinear matter
remainder would justify the narrower statement that anharmonic/relativistic/
mode-coupling matter structure is the immediate selected donor; the exact
binding cross-check would then determine whether binding decrease is part of
that description.

No outcome changes the production tick or promotes a scenario.
