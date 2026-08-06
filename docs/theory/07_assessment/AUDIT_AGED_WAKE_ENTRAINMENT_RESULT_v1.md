# FTD-0766 aged wake/entrainment discriminator — result audit v1

**Status:** `[CERTIFIED INVALID REGISTERED EXECUTION + DESCRIPTIVE NUMERICAL FACTS]`  
**Date:** 2026-07-31  
**Official verdict:** `AGED_WAKE_EXECUTION_INVALID`; wake creation not established

## 1. Record of execution

The locked FTD-0766 WSL2 RTX 5090 executable completed all 21 registered
branches in 4838.8 seconds. It used the unchanged FTD-0761/0763 parent,
`L=321`, preparation ages `{0,64,128}`, boosts
`{0,+/-0.0075,+/-0.015,+/-0.030}`, 64 ticks per branch, and checkpoints
`{0,16,32,48,64}`.

Run-of-record artifact:

```text
engine/results/ftd_0766/ftd_0766_aged_wake_entrainment_v1.json
SHA256 116F47B0CD0092F1F814B151084C436E719DA2F278E957CD5083AF44AF38C090
```

The independent certificate
`scripts/proofs/proof_aged_wake_entrainment_cuda.py` passes 404/404 checks. It
hash-locks the artifact and reconstructs the arm matrix, checkpoint gates,
half-space partitions, signed-pair mirror residuals, final asymmetries,
rest-subtracted entrainment fractions, and official invalid labels.

## 2. Registered execution failure

The age-zero `q=+/-0.030` branches fail before evolution because the selected
support-independent core predicate rejects initialization. A common boost does
not alter constituent positions, separation, or graph margin. Within the
predicate, the only boost-dependent membership term is constituent kinetic
energy; therefore the failure is an energy-margin rejection, not a graph,
CUDA, or implicit-root failure.

All 14 age-64/128 arms and five of seven age-zero arms initialize and execute.
Across the 19 valid arms:

| Gate | Maximum/minimum |
|---|---:|
| maximum common-action residual | `4.55001e-14` |
| maximum energy residual | `5.07754e-15` |
| maximum causal-speed excess | `0` |
| maximum one-step inverse residual | `2.84217e-14` |
| minimum root singular value | `0.984076` |
| maximum condition number | `1.08720` |
| maximum morphology reconstruction residual | `4.18502e-17` |
| maximum longitudinal partition residual | `1.38778e-17` |

All rest centers remain fixed within `2.84217e-14`. Thus aging broadens the
selected core's admissible common-boost energy basin: `|q|=0.030` is rejected
at age zero but accepted at ages 64 and 128. This is a descriptive finite-time
fact, not a dispersion law or generic mobility theorem.

## 3. Signed-pair symmetry failure

Every evaluable signed pair passes core-trajectory reflection with maximum
residual `1.27106e-13`, but every pair fails the locked `1e-10` raw
trailing/leading field-triple mirror gate.

| Age | `|q|` | max core mirror | max field-triple mirror |
|---:|---:|---:|---:|
| 0 | 0.0075 | `8.99e-14` | `2.54523e-1` |
| 0 | 0.015 | `6.96e-14` | `2.54523e-1` |
| 64 | 0.0075 | `6.36e-14` | `5.56997e-9` |
| 64 | 0.015 | `6.36e-14` | `5.62412e-9` |
| 64 | 0.030 | `6.36e-14` | `2.57659e-8` |
| 128 | 0.0075 | `8.99e-14` | `5.48327e-6` |
| 128 | 0.015 | `5.68e-14` | `5.50811e-6` |
| 128 | 0.030 | `1.27e-13` | `6.62265e-6` |

At age zero the maximum occurs at `tau=0`: the same unboosted field is viewed
through opposite aligned directions, so a directional preparation bias swaps
trailing and leading. The protocol separately says such a static term should
cancel in the signed average, yet its raw-triple mirror gate also demands
equality before subtraction. This is a contract-level overconstraint. It does
not repair the consumed run. A successor must compare rest-subtracted dynamic
increments or explicitly reflect the complete oriented parent, and must lock
that choice before execution.

The age-64/128 failures are not only this initial condition. Their raw pair
differences grow during evolution above `1e-10`, reaching the values in the
table. Hence removing `tau=0` alone would not qualify either aged panel.

## 4. Wake discriminator

The final signed-pair trailing asymmetry `D_pair` and rest-subtracted residual
entrainment are:

| Age | `|q|` | `D_pair` | entrainment |
|---:|---:|---:|---:|
| 0 | 0.0075 | `0.0842306` | `-0.0412499` |
| 0 | 0.015 | `0.0744118` | `-0.0106615` |
| 64 | 0.0075 | `0.0552359` | `-0.00421517` |
| 64 | 0.015 | `0.0480590` | `0.0128352` |
| 64 | 0.030 | `0.0349433` | `0.0323266` |
| 128 | 0.0075 | `0.0402139` | `0.00847496` |
| 128 | 0.015 | `0.0333536` | `0.0236014` |
| 128 | 0.030 | `0.0161127` | `0.0464151` |

These values are descriptive because the registered execution is invalid.
They nevertheless reject the proposed signature on two independent grounds:

1. at every age with multiple evaluable magnitudes, asymmetry is strictly
   decreasing rather than increasing with boost magnitude;
2. the age-64 to age-128 relative changes are `27.20%`, `30.60%`, and
   `53.89%`, all above the locked `25%` stability gate.

Residual entrainment remains below 5% in magnitude. For the registered
`|q|=0.015` branch it changes only from `-1.07%` to `2.36%`, far below the
selected age-improvement criterion of a ten-percentage-point increase and
final majority entrainment. The data therefore continue to support a mobile
manifested core moving through a mostly unentrained environmental field, not a
rigid co-moving dressing or stable velocity-generated wake.

## 5. Correct statement

FTD-0766 does not establish a dynamical wake. Its official result is an
invalid registered execution because two arms fail initialization and every
evaluable signed field pair fails the mirror gate. Independently, the measured
finite-horizon asymmetry has the wrong amplitude ordering and insufficient age
stability. The constructive residue is a preparation-age-dependent admissible
boost basin plus precise evidence of persistent residual-field
under-entrainment.

No production dynamics, primitive, default, toggle, scenario, constant,
Lorentz claim, or `RenderBridge` path changed.

