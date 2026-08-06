# Audit — Orientation degree versus Gauss charge (FTD-0564)

**Verdict:** `[PROVED-SCOPED — ORIENTATION AND GAUSS OBSERVABLES ARE INDEPENDENT]`  
**Date:** 2026-07-26  
**Production changes:** none.

## Locked result

The preregistered observer executed 60 synthetic arms:

```text
2 field families x 5 dyadic amplitudes x 2 polarities x 3 cubic rotations.
```

The independent Python proof and the C++ observer agree:

| Diagnostic | Result |
|---|---:|
| arms | 60/60 |
| exact rank/routing witnesses | 2/2 (`L=3,5`) |
| maximum direction-degree residual | 0 |
| maximum affine closed-flux residual | `8.881784197001252e-16` |
| maximum equal-flux counterexample residual | `8.881784197001252e-16` |
| maximum scale-linearity residual | 0 |
| maximum polarity-mirror residual | 0 |
| maximum cyclic-covariance residual | `8.881784197001252e-16` |
| maximum exact tree-routing residual | 0 |

Focused CTest `orientation_gauss_independence` passed.

## Decisive counterexamples

On the same octahedral surface,

```text
J = A n             has degree +1 and flux 4A,
J = A(n + 2 e_z)    has degree  0 and flux 4A.
```

The second field never vanishes on the surface and its normalized image lies in one hemisphere. Equal nonzero flux therefore does not imply equal degree. Varying `A` over `1,1/2,1/4,1/8,1/16` leaves the hedgehog degree fixed while producing five distinct fluxes, so degree does not determine flux.

## Provenance lock

Pre-execution preregistration SHA-256:

```text
25DB8EA8343E165FE4EFC3FB2D83C4520BEC76CC97A05F907412A7E029C58663
```

The run record stores SHA-256 hashes for `voxel.h`, `gauge_field.h`, `render_bridge.cpp`, `phase_read.cpp`, `test_gauge_links.cpp`, and the preregistration. Source inspection confirms the regular real `J/W/L/R` field scope and the imposed, default-off, substrate-write-only SU(2)/SU(3) link scope.

Artifacts:

- `engine/include/ftd/eft/orientation_gauss_independence.h`
- `engine/src/eft/orientation_gauss_independence.cpp`
- `engine/tests/test_orientation_gauss_independence.cpp`
- `scripts/proofs/proof_orientation_gauss_independence.py`
- `engine/results/ftd_0564/windows_msvc_cpu.json`
- `docs/theory/10_eft_program/derivations/constituent_complete_matter/THEOREM_ORIENTATION_GAUSS_INDEPENDENCE.md`
- `docs/theory/10_eft_program/preregistrations/constituent_complete_matter/PREREG_ORIENTATION_GAUSS_INDEPENDENCE_v1.md`

## Licensed interpretation

FTD-0392/0398 studied a direction-map degree and supplied no electric-charge result. FTD-0564 now proves why: even a robust nonzero degree would not set the Gauss-flux magnitude. The topology-only electric-charge route is closed for the frozen variables.

The surviving route is a topological core plus a separately derived nonlinear common action that locks magnitude and supplies energy, recoil, mobility, and protection. It remains open and unimplemented.

