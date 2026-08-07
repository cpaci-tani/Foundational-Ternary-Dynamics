# AUDIT — Poisson cold-start memory

**Date:** 2026-07-24  
**Identifier:** `FTD-0441`  
**Status:** `[MEASURED — POISSON INITIALIZATION]` + `[CLOSED — FTD-0439 POISSON LEAK EXPLAINED]`  
**Verdict:** `COLD_START_TRANSIENT_EXPLAINS_POISSON_LEAK`  
**Pre-registration:** [`PREREG_POISSON_COLD_START_MEMORY_v1.md`](../../10_eft_program/preregistrations/common_action_mechanics_reciprocity/PREREG_POISSON_COLD_START_MEMORY_v1.md)  
**Run of record:** `engine/results/ftd_0441/windows_msvc_cpu_L33.csv`

## 1. Result

The cold arms exactly reproduce FTD-0439's Poisson leakage. A 96-sweep
potential preparation followed by velocity reset removes it:

| observable | cold maximum | prepared maximum | prepared/cold |
|---|---:|---:|---:|
| particle momentum | `8.11011e-9` | `1.12317e-13` | `1.38490e-5` |
| common displacement | `1.56007e-6` | `2.19052e-11` | `1.40412e-5` |

Every prepared arm passes the locked `1e-10` momentum and `1e-8` motion gates.
Suppression exceeds a factor of `7.1e4`. Neither particle executes a voxel hop
in any arm, and both particles survive.

The relative attraction remains: prepared-pair minimum separation is about
`7.32528`, essentially the cold value. Preparation removes only the spurious
center-of-mass component, not the intended equal-and-opposite attraction.

## 2. Mechanism

The Poisson potential begins at zero. Each production tick performs six SOR
sweeps, evaluates force, and permanently integrates that force into particle
velocity. FTD-0440 showed that intermediate cold SOR iterates have nonmonotonic
pair-force imbalance even though the converged static solution is reciprocal.
The early imbalanced iterates therefore leave a small momentum memory that
survives after the potential converges.

Because no source voxel hops, neither moving-source lag nor collision ordering
is required. FTD-0439's locked `MOVEMENT_OR_PHASE_ORDER_DEFECT` label is
superseded as a causal diagnosis for the Poisson branch. The measured effect is
a cold-start initialization transient.

## 3. Force-branch picture after FTD-0441

The three branches now separate cleanly:

1. `G_C s grad|J|`: large intrinsic nonreciprocity, `6.405e-3` momentum,
   no central field recoil; closed negative as conservative mechanics.
2. `-alpha s grad(div J)`: balanced to `3.50e-18` in the registered pair
   protocol, but force usefulness and physical scaling remain unestablished.
3. Poisson `-alpha s grad(phi)`: reciprocal after solver preparation, with an
   imported instantaneous mechanism and a cold-start artifact at interactive
   initialization.

The selected-force defect cannot be attributed to the common particle
integrator: the other two branches balance once their own numerical preparation
requirements are respected.

## 4. Scenario consequence

Any scenario that enables Poisson force and mobile particles immediately from
a zero potential must be treated as numerically under-prepared during its
initial relaxation. Physics qualification must either:

- prepare the potential with movement disabled and reset the induced particle
  velocity before the measurement window; or
- exclude the relaxation interval and demonstrate that stored center-of-mass
  momentum is absent.

Merely waiting after movement begins is insufficient because the transient
momentum has already been integrated and persists.

## 5. Epistemic boundary

This closes the specific small Poisson anomaly from FTD-0439. It does not make
Poisson exchange native, causal, retarded, or gauge-derived. It does not repair
the selected magnitude-gradient branch. No production dynamics were changed.

## 6. Reproducibility

- source SHA256: `2d0ddcf0a87ae895241573711e91dc21c8df6b7c4d8187fe244c797dfc62cd1b`
- record SHA256: `03c116807fb42511243d6208f5f70bedfc950494409f2660995a82a2c7d2930e`
- compiler: pinned MSVC `14.44.35207`, Release
- backend: forced CPU, periodic `L=33`
- focused result: `COLD_START_TRANSIENT_EXPLAINS_POISSON_LEAK`
- focused CTest set: `3/3` passed
- golden merge-gate battery: `7/7` passed
