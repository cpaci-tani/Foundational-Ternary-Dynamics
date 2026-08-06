# Connected-block fixed-mass refinement obstruction

**Campaign:** FTD-0647  
**Status:** `[THEOREM — FROZEN SELECTED ACTION]` +
`[ENGINE CERTIFICATE — 12 CUBIC ARMS]` +
`[CLOSED NEGATIVE — UNCHANGED-COEFFICIENT FIXED-MASS REFINEMENT]`  
**Verdict:** `FROZEN_ADDITIVE_CONSTITUENT_FIXED_MASS_REFINEMENT_CLOSED`  
**Production impact:** none

## Result

Increasing the width of the present connected bipole while leaving every
constituent and action coefficient unchanged cannot produce better-resolved
copies of one finite-mass object.

A width-`w` block contains

\[
N(w)=2w^3
\]

constituents. Each constituent carries the unchanged production dispersion

\[
h(p)=\sqrt{E_{\rm REST}^2+C_{\rm SPEED}^2|p|^2},
\]

and hence contributes at least `E_REST` to the action energy. The binding
energy is a sum of nonnegative quartic edge terms. The matched staggered field
energy is also nonnegative at the selected `lambda=C_SPEED=1/sqrt(3)`: the
periodic cubic curl has operator norm at most `2*sqrt(3)`, so

\[
\widetilde H_{EB}
\ge \frac12(1-\lambda\sqrt3)
\bigl(\|E\|^2+\|B\|^2\bigr)=0.
\]

The face normalization `beta` is positive. Therefore

\[
H_{\rm rest}(w)\ge 2w^3E_{\rm REST},
\qquad
M_{\rm rest}(w)=H_{\rm rest}(w)/C_{\rm SPEED}^2
\ge 2w^3M_{\rm INERTIAL}.
\]

Numerically, the exact floors in the current lattice calibration are

\[
H_{\rm rest}(w)\ge0.3406666666666666\,w^3,
\qquad
M_{\rm rest}(w)\ge1.022\,w^3.
\]

These are lower bounds, not fits to the four executed widths.

## Collective inertia

The obstruction is not removed by subtracting a constant rest baseline while
leaving the momentum law unchanged. At fixed total constituent momentum
`P=sum_a p_a`, convexity gives

\[
\sum_a h(p_a)\ge N h(P/N)
=NE_{\rm REST}+\frac{|P|^2}{2NM_{\rm INERTIAL}}+O(|P|^4).
\]

Thus the uniform collective kinetic sector has inertial mass
`N*M_INERTIAL`. A subtraction of `N*E_REST` changes the reported zero of
energy but does not by itself make the collective inertial curvature finite.

## Engine certificate

The locked engine record evaluates widths `1,2,3,4`, all three orientations,
at `L=17`. All 12 arms pass initialization, exact count and neutrality,
connected/local graph, site projection, Gauss, binding, field positivity,
total lower-bound, cubic covariance, and scaling gates.

| diagnostic | worst value | gate |
|---|---:|---:|
| rest-sum relative residual | `1.1407e-15` | `1e-14` |
| negative field energy | `0` | `1e-12` |
| total lower-bound defect | `0` | `1e-12` |
| cubic scalar-energy residual | `4.5608e-15` | `1e-10` |
| rest-floor scaling residual | `0` | `1e-14` |
| inertial-floor scaling residual | `0` | `1e-14` |

The independent certificate also checks the protocol and result hashes, the
curl-norm bound, every CSV row, and the collective-inertia expansion.

## Ontological consequence

FTD-0646 showed that the fixed 16-constituent object is a coherent reversible
quasiparticle with a finite Peierls boundary. FTD-0647 now rules out the
simplest proposed escape: one cannot merely add more identical copies of the
same massive constituent and call the result a refinement of the same
particle.

This supports a sharper matter interpretation:

- a constituent record is a local coordinate of a composite manifestation,
  not automatically one independently calibrated physical particle;
- physical rest mass must be a collective or scale-normalized property of the
  whole finite pattern;
- primitive ternary polarity need not equal a fixed physical charge carried
  by every occupied site;
- a continuum-style refinement, if used, must include the cell measure in the
  action coefficients rather than treating site count as physically free.

No new primitive is forced by this theorem. New dynamics or a new
normalization law is forced if the program insists on a fixed-mass refinement
limit.

## Surviving repair branches

The following branches remain logically distinct:

1. **Cell-measure refinement:** set the per-cell rest coefficient and binding
   measure to scale with cell volume while transforming the field coefficient
   with the discrete Maxwell measure. This treats different `w` values as
   different resolutions of one physical object.
2. **Collective graph mass:** replace the additive constituent rest term by a
   local graph/action functional whose uniform translation curvature is fixed
   by the connected object, not by vertex count.
3. **Background condensation:** make constituents excitations of a background
   whose signed vacuum/binding term participates in both rest energy and
   inertia. A mere energy subtraction is insufficient.
4. **Finite carrier:** keep a finite constituent count and derive another
   mechanism that suppresses its absolute Peierls barrier below the empirical
   infrared scale.

Under the FTD-0598 repair policy these may be proposed and tested as fresh
selected actions. None may be described as already native or used to rewrite
FTD-0646/0647 after the fact.

## Files of record

- protocol:
  `PREREG_CONNECTED_BLOCK_FIXED_MASS_REFINEMENT_OBSTRUCTION_v1.md`, SHA-256
  `5D3A8E64750936A1A437C4F743777297977AA0E6BEBAC241F8FF46BD647706D9`;
- runner: `engine/tests/test_connected_block_fixed_mass_refinement_obstruction.cpp`;
- independent certificate:
  `scripts/proofs/proof_connected_block_fixed_mass_refinement_obstruction.py`;
- JSON and CSV: `engine/results/ftd_0647/`.

