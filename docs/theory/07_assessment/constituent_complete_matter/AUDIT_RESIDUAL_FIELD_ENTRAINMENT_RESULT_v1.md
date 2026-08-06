# FTD-0765 — Residual-field entrainment result v1

**Status:** `[CERTIFIED DERIVED OBSERVER IDENTITY + POST-HOC NUMERICAL FACT; WAKE CREATION NOT ESTABLISHED]`  
**Date:** 2026-07-31  
**Production dynamics:** unchanged  
**New engine run:** none

## Verdict

The independent certificate reports:

```text
FTD-0765 entrainment certificate: 80/80 checks
face: core=0.40424658062224239 residual=-0.004309856472843876 entrainment=-0.010661454368296368 lag=0.40855643709508627
edge: core=0.40005326208963665 residual=0.062633851920472355 entrainment=0.15656378251563538 lag=0.33741941016916432
body: core=0.41567332180485217 residual=0.070243369258695273 entrainment=0.16898695579908471 lag=0.34542995254615688
rigid_residual_entrainment=false
independent_wake_creation_established=false
finite_scale_orientation_dependence=true
```

FTD-0764's registered trailing moment is exactly the core-minus-residual
centroid lag after rest subtraction. It is therefore not an independent wake
observable. The radius-48 residual-energy centroid follows less than 17% of
the manifested-core displacement on every ray and has a large orientation
dependence at this finite scale.

## Scope correction

FTD-0764 remains correct that its preregistered trailing-lag predicate fired.
The physical reading is narrowed:

- **supported:** a moving core separates from a mostly unentrained residual
  field environment;
- **not established:** creation of a new dynamical wake;
- **not established:** a co-moving field aura;
- **still open:** late-time attractor formation after preparation aging;
- **still open:** an independently derived total translation ledger.

This is a post-hoc discriminator motivated by the FTD-0764 result. It is not a
preregistered new prediction. The algebraic identity is exact; the measured
entrainment fractions are numerical facts conditional on the FTD-0763 bound
selection and the radius-48 window.

## Reproducibility

- source artifacts are the hash-locked FTD-0764 face/edge/body JSON files;
- certificate: `scripts/proofs/proof_residual_field_entrainment.py`;
- certificate result: 80/80;
- no search, fit, retuning, new field download, or engine execution occurs;
- production, defaults, primitives, toggles, scenarios, and `RenderBridge`
  remain unchanged.

