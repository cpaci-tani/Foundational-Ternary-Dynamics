# Audit — FTD-0610 single-core neutralizer control v1

**Status:** `[AUDIT — EXACT DYNAMICS CONSTRUCTIVE; STATIC REFERENCE CLOSED]`
**Verdict:** `SINGLE_CORE_STATIC_REFERENCE_NOT_ISOLATED`

## Reproducibility record

- protocol SHA-256:
  `DB4363D2A132BB84BFF10218FCE8B4B20BC4C677F6FE813815F368E38A4EED85`;
- runner: `engine/tests/test_single_core_neutralizer_control.cpp`;
- certificate: `scripts/proofs/proof_single_core_neutralizer_control.py`;
- JSON/CSV: `engine/results/ftd_0610/`;
- focused CTests: three of three pass;
- independent certificate: 21/21 checks pass.

## Gate disposition

Both control fixtures, all six forward/reverse arms, and both translation
controls have complete numerical coverage. Current, Gauss, work, total
energy, causal speed, fibre regularity, internal geometry, and inverse gates
pass. The uniform zero-momentum arm fails the registered static gate by large
margins: `0.06473` cells of longitudinal drift and `0.01220` centre-momentum
change in 16 ticks, each against `1e-10`.

The slow uniform boost reaches `0.80715` cells rather than the required `1.5`.
The frozen partner changes that same trajectory to `-0.22442`, while both
fast boosts pass. These are resolved dynamics, but they are not boosts of a
common rest solution because the starting core accelerates at zero launch.

## Audit conclusion

FTD-0610 rules out treating the extracted phase-15 trimer as an isolated
static core. It does not rule out a separately minimized single-core state.
The exact reversibility and nearly fixed internal distances show that neither
the chart fibre nor internal binding is the immediate failure. The missing
object is a stationary solution of the complete one-core control action.

The next campaign must locate or close such a stationary solution before
making another mobility claim. Reinterpreting the nonzero rest trajectory as
spontaneous propulsion would violate the registered static gate and would
confuse a lattice self-force/environment mismatch with inertial motion.
