# Connected-block analytic matter modes

**Campaign:** FTD-0640  
**Status:** `[DERIVED — FIXED-SECTOR LINEAR PREDICTION] + [MEASURED —
REVERSIBLE CLASSICAL MATTER MODES]`  
**Production impact:** none

## Question

FTD-0638/0639 supplied a positive 48-coordinate static center and a reversible
fixed tick. FTD-0640 asks the next narrower question: do all infinitesimal
constituent directions behave as the normal modes predicted by that same
static action and the unchanged constituent inertia?

This is a matter-coordinate spectrum. Independent face/edge field
perturbations are deliberately excluded.

## Linear prediction

Let `u` be the 48-coordinate displacement from the analytic center, let `H`
be the FTD-0637 analytic Poisson-envelope plus binding Hessian evaluated at the
FTD-0638 center, and let

\[
M=M_{\rm INERTIAL}I_{48}.
\]

The registered generalized modes satisfy

\[
Hv_m=\lambda_m Mv_m,
\qquad
v_m^TMv_n=\delta_{mn}.
\]

For the unit-step implicit midpoint map, a scalar eigenmode obeys a
three-point recurrence with

\[
\cos\Omega_m=\frac{1-\lambda_m/4}{1+\lambda_m/4},
\qquad
\Omega_m=2\arctan\!\left(\frac{\sqrt{\lambda_m}}{2}\right).
\]

No effective mass is fitted. Degenerate analytic eigenvalues within `1e-10`
define one purity subspace; a basis vector inside such a subspace has no
individual ontological meaning.

## Locked campaign

The primary campaign displaces all 48 sorted `x`-orientation modes by a
maximum constituent-coordinate amplitude of `8e-6`. Thirteen registered
indices also receive half-amplitude, sign-mirror, and cyclic-orientation
controls. All 87 arms run 256 forward ticks and 256 state-only reverse ticks,
for 44,544 exact common-action steps.

The nonlinear root solver uses an observer-only repeated-Jacobian cache. The
cache changes only the route to the root: every accepted endpoint is evaluated
with the exact common-action residual and the unchanged `2e-11` root
tolerance. A separate cached/direct equivalence CTest passes. Production calls
without a cache follow the original solver path.

## Result

The registered verdict is
`CONNECTED_BLOCK_ANALYTIC_MATTER_MODES_CONSTRUCTIVE`.

| gate or diagnostic | result | gate |
|---|---:|---:|
| completed arms | `87/87` | `87/87` |
| worst primary phase error | `1.1267%` | `2%` |
| worst out-of-eigenspace leakage | `1.5226%` | `10%` |
| worst common-action residual | `2.0000e-11` | `1e-10` |
| worst energy drift | `1.9585e-13` | `1e-12` |
| worst inverse recovery | `5.1017e-11` | `1e-10` |
| cyclic spectrum mismatch | `2.2538e-12` | `1e-9` |
| sign-trajectory residual | `2.6786e-5` | `5e-2` |
| largest half/full quadratic-energy defect | `0.00923` | `0.1` |

Every trajectory stays in its starting quadratic-spline sector, makes no site
hop, keeps anchor multiplicity at most two, and keeps same-anchor effective
positions separated by at least `0.9989`.

## Post-result mode geometry

The registered eigenvectors permit an additional structural diagnostic. Form
the six-dimensional rigid-coordinate space from three uniform constituent
translations and three infinitesimal rotations about the center. This was not
an FTD-0640 verdict gate and is reported separately.

- Sorted modes `1,2,5` are translation-dominated.
- Sorted modes `0,3,4` are rotation-dominated.
- The worst squared principal-angle defect between the first-six eigenspace
  and the rigid-coordinate space is `4.3862e-7`.
- The small nonzero defect is carried principally by modes `6,7`; the cubic
  coat weakly mixes rigid and deformational motion.
- The largest soft eigenvalue is `0.00339785`; the first internal eigenvalue is
  `0.75321764`, a stiffness ratio of `221.67476`.

Thus the finite object has a clean but not exact separation between
lattice-dressed position/orientation motion and internal deformation. The six
rigid modes are not zero modes because continuous translations and rotations
are not exact microscopic symmetries of the cubic coat. Their positive
curvature is the local Peierls/orientational pinning scale.

## Ontological reading

Within the selected research branch, the minimal supported matter story is
now:

1. Matter is not one `+1` or `-1` site. It is a finite neutral organization of
   signed ternary manifestations.
2. The manifestations retain continuous effective positions through the
   finite local chart fibre; these coordinates are constituents' phase space,
   not fractional primitive polarity.
3. A connected Moore-local relational graph and self-consistent longitudinal
   face-flux dressing define a positive local energy basin.
4. The same discrete action generates reversible rest and a complete
   small-amplitude classical spectrum. Location, orientation, and internal
   shape are dynamical coordinates of one object.
5. A small excitation is a reversible deformation of the entire dressed
   configuration, not the creation of another matter voxel.

This supports a finite classical composite ontology under selected dynamics.
It does not derive that ontology uniquely from the five postulates.

## Remaining boundary

FTD-0640 does not establish a quantum state, particle mass, spin, statistics,
field pole, photon, physical charge, freely translating carrier, or common
matter/field cone. The immediate next gate is the independent face/edge field
spectrum about the same center. It must distinguish field-only waves from
matter-coordinate modes before a coupled pole or finite-boost claim is
meaningful.
