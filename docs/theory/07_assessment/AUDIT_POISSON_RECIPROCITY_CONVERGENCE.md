# AUDIT — Poisson reciprocity convergence

**Date:** 2026-07-24  
**Identifier:** `FTD-0440`  
**Status:** `[MEASURED — STATIC POISSON FORCE]` + `[PASS — PRE-RELAXED RECIPROCITY]` + `[INVALIDATED — MONOTONE COLD-RESIDUAL MODEL]`  
**Locked verdict:** `NO_SOR_DEPENDENCE`  
**Valid pre-registration:** [`PREREG_POISSON_RECIPROCITY_CONVERGENCE_v2.md`](../10_eft_program/preregistrations/PREREG_POISSON_RECIPROCITY_CONVERGENCE_v2.md)  
**Invalid unexecuted v1:** [`PREREG_POISSON_RECIPROCITY_CONVERGENCE_v1.md`](../10_eft_program/preregistrations/PREREG_POISSON_RECIPROCITY_CONVERGENCE_v1.md)  
**Run of record:** `engine/results/ftd_0440/windows_msvc_cpu_L33.csv`

## 1. Result

The fixed-pair Poisson force has no resolved converged reciprocity floor at the
registered `1e-12` tolerance. After 96 warmup sweeps, every measured axis/count
arm satisfies the gate; the worst net force is

$$
|F_++F_-|_{\max}=1.42915\times10^{-13}.
$$

At the production default of six sweeps from a cold potential, the net force is
already only `1.13308e-19`, despite individual opposing forces of
`1.07100e-5`. Thus the static default solve is reciprocal to roughly fourteen
decimal orders relative to the force scale in this arm.

## 2. Why the locked verdict is not the literal interpretation

The cold net-force sequence is nonmonotonic:

- iterations `1,2,4,6` give zero-to-`2.04e-19` net force;
- iteration `12` gives `2.14e-10`;
- iteration `24` gives `3.08e-9`;
- iteration `48` gives `1.86e-10`;
- iteration `96` returns to `1.66e-13`.

Consequently the locked monotonic gate fails, and the ratio
`cold96/cold6=1.47e6` triggers `NO_SOR_DEPENDENCE`. The name is not a valid
mechanistic summary: the force magnitudes and cancellation errors plainly
depend on SOR progress. The defect is in the preregistered assumption that net
pair-force error must decrease monotonically from an empty potential. Partial
SOR solutions can preserve or break pair cancellation accidentally while the
physical force itself is still propagating through the iterative solve.

The robust statement comes from the explicit pre-relaxed gate, not the cold
ratio: all pre-relaxed arms pass.

## 3. Consequence for FTD-0439

FTD-0439's `8.11e-9` Poisson momentum leak is not a static force/stencil
reciprocity floor. It requires the dynamic trajectory. The remaining live
causes are:

1. a warm potential lagging behind moved source voxels;
2. force-before-movement phase ordering combined with discrete hops;
3. the movement/collision implementation itself.

This narrows the next test to the full moving pair as a function of SOR count.
If higher per-tick iteration counts suppress the center-of-mass momentum, the
leak is a moving-source solver-lag error. If not, the movement/phase-order route
remains.

**Successor result (FTD-0441):** the particles execute no voxel hops. Preparing
the unchanged potential for 96 sweeps before the measurement window suppresses
the momentum/common drift by over `7.1e4`. The FTD-0439 leak is cold-start
momentum memory, not a moving-source or movement-order effect in this protocol.

## 4. Epistemic boundary

Static pre-relaxed reciprocity is a numerical property of the imported
instantaneous Poisson branch. It does not make that branch native, retarded, or
gauge-derived. FTD-0440 does not alter the closed-negative status of
`G_C s grad|J|`.

Revision 1 was never executed: compilation failed because it called private
`RenderBridge` subphases. Revision 2 uses complete public ticks. Its binary
banner retained the cosmetic string `v1`; the exact v2 source hash below binds
the admitted record and prevents ambiguity.

## 5. Reproducibility

- source SHA256: `77885372f09ec86b679689f316f25f418331d5126f168c0510451365b9351fab`
- record SHA256: `a69ffc246bf9aef5de92b998877da113ab7bf199f0e69389f826200a9227ca31`
- compiler: pinned MSVC `14.44.35207`, Release
- backend: forced CPU, periodic `L=33`
- locked result: `NO_SOR_DEPENDENCE`
- admitted interpretation: static pre-relaxed reciprocity passes; cold
  monotonic-residual model is invalid
