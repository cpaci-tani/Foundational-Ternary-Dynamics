# FTD-0613 — Refined single-core directional boost v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION/EXECUTION]`
**Scope:** default-off compact-rest-state mobility discriminator
**Production change:** forbidden
**Protocol lock:** `protocol_sha256=1A750AA6C557294B6E252A0E77F4B33AD5791A251655EB5787CA946E74A92C35`

## 1. Frozen launch state

Use only the FTD-0612 refined uniform-neutralized rest state, reconstructed
from runner SHA-256
`14EDE67C1A24E137049592D9CFE2575CC67984ED416ECE12914C1E7C397B10B3`
and result-record SHA-256
`2DDF8E4164E4621D7873F03D2A54373B58938A48B1556924D12B24A9242F51C7`.
Require its energy fingerprint `0.0015517955076684577` within `1e-15`, final
gradient at most `1e-10`, nine positive modes, and the complete 64-tick rest
gate before launching any moving arm.

Do not reoptimize after adding momentum. Keep the same constituents, charges,
shape, field, uniform `-1/L^3` neutralizer, binding, fibre, common-action
transaction, tolerances, and production dispersion.

## 2. Directional boost matrix

Apply equal momentum to all three constituents for each Cartesian direction
`+x,-x,+y,-y,+z,-z` and each speed:

| speed | ticks | nominal displacement |
|---:|---:|---:|
| `1/128` | 256 | 2 cells |
| `1/64` | 128 | 2 cells |
| `1/32` | 64 | 2 cells |

This gives 18 independent forward histories. Follow every history by the same
number of state-only inverse ticks. No forward current, impulse, endpoint,
branch, or neutralizer response may be supplied to the inverse.

## 3. Gates

For every arm require:

- complete forward/reverse solver coverage and every common-action gate at
  `1e-12`;
- total-energy drift at most `1e-10` and state recovery at most `1e-9`;
- internal distances in `[0.5,2.0]`, maximum anchor multiplicity two, and
  effective constituent separation at least `1e-3`;
- displacement projected onto the launch direction at least `1.5` cells,
  transverse drift at most `0.25`, and at least three constituent anchor
  changes.

Record shared-anchor states and the field-plus-matter pseudomomentum defect,
but do not gate them. The uniform neutralizer is external, and an arm need not
exercise anchor aliasing merely to demonstrate transport.

Require sign-paired projected displacements and transverse magnitudes to agree
within `0.25`, and the three axis-paired means at each speed to agree within
`0.25`. These are microscopic cubic/sign controls, not Lorentz claims.

## 4. Verdicts

- `REFINED_COMPACT_CORE_DIRECTIONALLY_MOBILE_CONSTRUCTIVE`: the frozen rest
  fingerprint and all 18 directional arms pass;
- `REFINED_COMPACT_CORE_DEPINNING_THRESHOLD_MEASURED`: all numerical,
  common-action, energy, inverse, geometry, and symmetry coverage is complete;
  every `1/32` arm passes, while at least one lower-speed arm fails only a
  displacement/hop gate;
- `REFINED_COMPACT_CORE_DIRECTIONAL_MOBILITY_CLOSED_NEGATIVE`: coverage is
  complete but the result does not satisfy either constructive pattern;
- `REFINED_COMPACT_CORE_DIRECTIONAL_BOOST_NUMERICALLY_UNRESOLVED`: rest
  fingerprint, solver, inverse, record, or symmetry coverage is incomplete.

A depinning verdict establishes a finite microscopic Peierls barrier for this
selected compact family. It directs the next candidate toward an extended
low-momentum carrier whose barrier is tested as a function of width. It does
not authorize force amplification or a favorable launch phase. No physical
particle, production motion, pole, Lorentz recovery, or unitarity claim is
licensed.
