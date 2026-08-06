# Audit — FTD-0623 connected Moore-block repeated dynamics

**Status:** `[AUDIT — REPEATED FINITE-BOOST MOBILITY CONSTRUCTIVE /
TRANSLATIONAL REST STABILITY OPEN / PHYSICAL PARTICLE OPEN]`  
**Date:** 2026-07-27

## Findings

1. **Repeated coherent mobility is constructive for the selected object.**
   The 16-constituent, 72-bond state advances more than three cells, performs
   48 legitimate constituent hops, remains sharply shape-bounded, and reverses
   to its initial complete state.

2. **The result is not centre-coordinate kinematics.** Every constituent has
   independent position and momentum. Every current segment and bond impulse
   enters the simultaneous solve, and unique site projection is enforced on
   every step.

3. **The rest arm proves stationarity, not stability.** It remains centred
   below `2.2e-15`, but it starts exactly at the integer-centred Peierls
   maximum proved by FTD-0553. The test applies no centre perturbation. Calling
   this arm a stable rest solution is an overstatement.

4. **The result is high-speed depinned mobility, not a free infrared mode.**
   The locked momentum `0.12` produces free speed `0.217528`. The test contains
   no lower momentum ladder and cannot establish zero-threshold motion.

5. **Continuous translation remains defective.** Accumulated normalized spline
   reaction is nonzero and anisotropic: `0.08015` parallel versus `0.16646`
   transverse. The motion cannot be called isolated momentum-conserving
   propulsion.

6. **The selected graph is ontological debt.** Reference bonds preserve
   constituent relations through motion. They supply a concrete material
   memory but are not derived from native ternary events. The result therefore
   supports a candidate matter ontology, not the five-postulate production
   ontology by itself.

## Correct statement

FTD-0623 constructs one finite exact-ternary connected object that is
stationary at the exact integer phase, coherently mobile under a finite boost,
cubic covariant, energy exact, and state-only reversible for the registered
16-tick trajectories. Translational rest stability is open because the
unperturbed centre is a Peierls maximum. The result does not derive physical
particles, fixed mass, gapless translation, exact continuous momentum, or
native bond formation.

## Verification

- Protocol SHA-256:
  `7AA42C401938C48F134A1BF95C70FD8C6026B24B0FE2979173BBEF598800A3F7`.
- Focused CTest: pass, `414.53 s`.
- Run-record hashes: JSON `4E86C850...69926`, arms
  `1C102BF2...40E08`, ticks `C810B1B8...1D80D`.
- Independent certificate: `75/75` checks pass.
