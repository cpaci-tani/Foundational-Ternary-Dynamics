# Audit — FTD-0606 global orientation × strain core v1

**Status:** `[AUDIT — STATIC CORE CONSTRUCTIVE; SITE PROJECTION UNRESOLVED]`
**Verdict:** `GLOBAL_ORIENTATION_STRAIN_NUMERICALLY_UNRESOLVED`

## Reproducibility record

- locked protocol prefix SHA-256:
  `EC0CECED1CCF40187BCE0C4B38DA34039B5CAD94069AFD05F16420D25D99494A`;
- observer runner:
  `engine/tests/test_global_orientation_strain_core.cpp`;
- independent certificate:
  `scripts/proofs/proof_global_orientation_strain_core.py`;
- record:
  `engine/results/ftd_0606/ftd_0606_global_orientation_strain_core_v1.json`;
- per-phase record:
  `engine/results/ftd_0606/ftd_0606_global_orientation_strain_core_samples_v1.csv`;
- focused CTest: `global_orientation_strain_core`, pass.

## Gate disposition

| gate | result |
|---|---|
| exact centroid/rotation/rigid-distance identities | pass; worst `2.54e-16` |
| literal “same eigenvalues up to congruence” wording | fail; congruence preserves inertia, while the two coordinate spectra differ |
| periodic Green kernel | pass; `5.43e-16` |
| 24-start coverage at 32 phases | pass |
| interior strain | pass; maximum `1.66e-4` |
| tangent stationarity | pass; worst gradient `1.08e-7` |
| full six-mode stability | pass; minimum eigenvalue `2.02e-4` |
| direct field reconstruction | pass; worst `9.02e-16` |
| unique-anchor site projection | fail at 24/32 phases |
| common action | pass on all eight site-admissible phases; incomplete globally |
| state-only inverse | pass on all eight site-admissible phases; incomplete globally |
| force sign | mixed, four attractive and four repulsive among admissible phases |
| integer periodicity | not executable because phase-zero minimum is not site-admissible |
| barrier report | withheld because transaction/periodicity coverage is incomplete |

## Audit conclusion

The local FTD-0605 failure cannot be promoted to a failure of arbitrary
orientation. Global orientation produces small-strain, positive-Hessian
static minima at every sampled phase. However, most minima collapse two
continuous constituent records onto one ternary anchor. The static variational
space is therefore larger than the transaction's state space.

The current record supports a stable continuous constituent pattern but does
not support a globally mobile site-ontic matter object. It also does not prove
that no admissible nearby minimum exists, because the registered search did
not impose the unique-anchor constraint during minimization. The next test
must constrain that domain before optimization; changing the verdict of this
run would be post hoc.

The preregistration's literal eigenvalue sentence is also mathematically
incorrect. The independent certificate preserves the exact spectra and treats
the gate as failed. This defect does not change the unresolved classification,
but it forbids claiming that every locked algebra sentence passed.
