# Cell-measure fixed-mass refinement

**Campaign:** FTD-0648  
**Status:** `[SELECTED RESOLUTION MAP]` + `[DERIVED SCALING]` +
`[MEASURED — STATIC FIXED-MASS DEPINNING CONSTRUCTIVE]`  
**Verdict:** `CELL_MEASURE_FIXED_MASS_STATIC_DEPINNING_CONSTRUCTIVE`  
**Production impact:** none

## Result

FTD-0647 closed width refinement with frozen per-site coefficients. FTD-0648
tests the distinct interpretation that increasing `w` refines the same
physical object. Anchoring at the constructive width-two carrier, the relative
cell length is `a_w=2/w`. Three-dimensional cell measure and the discrete
Maxwell sum then give

\[
r_m=r_q=r_\kappa=a_w^3=(2/w)^3,
\qquad r_\beta=a_w^{-1}=w/2.
\]

These factors keep the total rest energy, inertial mass, and integrated
positive/negative polarity exactly fixed:

\[
E_{\rm rest}=16E_{\rm REST},\qquad
M=16M_{\rm INERTIAL},\qquad Q_+=Q_-=8.
\]

All 54 locked width/orientation/translation arms pass. The scaled static field
energy is asymptotically finite while the absolute half-cell Peierls barrier
strictly decreases at every registered width.

| diagnostic | result | gate |
|---|---:|---:|
| exact normalization residual | `2.2205e-16` | `1e-13` |
| scaled field-energy slope | `0.0822254` worst absolute | `<=0.25` |
| absolute barrier slope | `-2.64518..-2.52638` | `[-3.5,-2.5]` |
| width-8/width-4 energy ratio | `1.059391` | `[0.8,1.2]` |
| cubic residual | `1.8834e-13` | `1e-10` |

The leading exponent follows directly from the earlier unit-source scaling:
unit field energy grows as `w^5`, unit Peierls barrier as `w^2`, and the
declared field/source measure contributes `r_beta*r_q^2~w^-5`. Hence

\[
E_{\rm field}^{\rm phys}\sim w^0,
\qquad B_{\rm Peierls}^{\rm phys}\sim w^{-3}.
\]

The finite-width barrier slopes approach the predicted `-3` from above.

## Ontological meaning

This is the first coherent interpretation in which additional ternary records
mean greater resolution rather than additional matter. Primitive polarity
remains exactly signed; only its physical cell measure changes across the
resolution family. A constituent is therefore a local cell/coordinate of the
manifested pattern, while rest mass and integrated polarity belong to the
whole pattern.

This is a selected cross-resolution action map, not a result forced by the
five postulates. On a single fundamental lattice the cell length does not
dynamically change.

## Successor dynamical gate

FTD-0649 subsequently installed the same four factors in constituent
dispersion, deposited current/source, binding, field energy, and force gather.
All 45 one-step reciprocal-action arms pass exact continuity, Gauss, work,
energy, causality, covariance, and state-only inversion. Long-horizon
depinning across widths remains the live successor gate.
