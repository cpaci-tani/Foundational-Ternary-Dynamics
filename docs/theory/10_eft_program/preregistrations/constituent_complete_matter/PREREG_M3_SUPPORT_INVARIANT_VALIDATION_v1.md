# FTD-0755 — Support-invariant finite-time matter-family validation v1

**Status:** `[PRE-REGISTRATION — FORMULAS AND ARM MATRIX FROZEN BEFORE
IMPLEMENTATION; NOT RUN]`  
**Date:** 2026-07-30  
**Parents:** FTD-0743 matter-predicate contract; FTD-0753 causal-horizon
witness; FTD-0754 state-only separator; FTD-0754B boundary ledger;
FTD-0754C nested-support flow  
**Scope:** held-out validation of a finite-time selected matter family in the
existing reciprocal compact-pair action; no production, scenario, postulate,
particle, charge, mass, pole, or Lorentz claim

## 1. Question

Does the selected compact-pair dynamics contain a nonzero relative-open family
of complete states whose localized relational core remains classified through
a finite uncontained causal horizon, independently of the observer support
scale and of remote divergence-free environmental data before contact?

This is the first FTD-0755 validation protocol. No trajectory described below
has been run or inspected.

## 2. Frozen state sector and core predicate

The declared one-object sector contains complete states

\[
X=(E,B_{1/2};x_+,p_+,x_-,p_-)
\]

under the unchanged `DerivedCompactPair` common action with exactly two
constituents, charges `{+1,-1}`, no stored binding edge, `dt=1/4`, well depth
`D=0.01`, squared graph cutoff `d_c=3/2`, sparse local current, local residual
evaluation, root tolerance `2e-14`, and the explicit-rounding ordered WSL2
CUDA field path.

Let

\[
d=\lVert x_--x_+\rVert^2,
\]

\[
K=\sum_{a\in\{+,-\}}
\left(\sqrt{E_{\rm REST}^2+C_{\rm SPEED}^2\lVert p_a\rVert^2}
-E_{\rm REST}\right),
\]

and let `V(d)` be the already-selected compact polynomial used by the action.
Freeze the instantaneous margins

\[
\mu_g=d_c-d,
\qquad
\mu_E=-(K+V(d)).
\]

The support-independent core predicate is

\[
P_{\rm core}(X)=1
\Longleftrightarrow
\text{declared sector}\ \land\ \mu_g>0\ \land\ \mu_E>0.
\]

No dressing energy, residual energy, cross term, shell location, outgoing
power, incoming power, global background norm, tick, arm label, preparation
name, or future state enters `P_core`.

At every stored state, validation additionally requires numerical safety
margins

```text
mu_g                                                   >= 1e-6
mu_E                                                   >= 1e-6
common-action residual                                 <= 1e-10
recoil defect                                          <= 1e-9
pair-plus-field energy defect                          <= 1e-8
causal speed excess                                    <= 1e-12
```

The `1e-6` quantities are validation clearance gates, not changes to the
strict mathematical predicate boundary at zero.

## 3. Frozen field and scale conditions

At registered checkpoints, evaluate the unchanged FTD-0754 observer and the
FTD-0754C support ladder at half-widths

```text
R = {4,6,8}.
```

Require:

1. state-only observer validity, Gauss compatibility, exact centered
   reconstruction, and characteristic partition to `1e-12` relative scale;
2. valid primitive support/boundary/readout ledgers under FTD-0754B;
3. valid nested projection, monotonicity, and Pythagorean identities under
   FTD-0754C to `1e-12` relative scale;
4. identical `P_core`, `mu_g`, and `mu_E` for all support choices;
5. no dressing, cross, characteristic, or environmental value used as a
   membership threshold.

Keep four energy entries distinct:

\[
E_{\rm core},\quad I_{\partial K_R},\quad I_{A,R}+I_{B,R},
\quad E_{{\rm env},R}.
\]

Their scale dependence is diagnostic. Exact total reconstruction is the gate.

## 4. Frozen checkpoint and perturbation construction

For each volume and cubic ray, deterministically replay the already-defined
unperturbed preparation from tick 0 through tick 160. Tick 160 was fixed from
the published FTD-0753 record because all three rays are then strictly
graph-inside and negative-energy with substantial margins; no validation
output is used.

At that complete checkpoint create exactly three variants:

1. `center`: unchanged checkpoint, a replay/control that does not count as
   held-out perturbation evidence;
2. `energy_hostile`: the already-selected FTD-0734 minimum-energy sign tuple;
3. `graph_hostile`: the distinct already-selected FTD-0734 minimum-graph sign
   tuple.

Freeze the FTD-0734 perturbation coordinates and magnitudes unchanged:

```text
relative impulse norm                                  0.0006
radial squared-distance coordinate                     half of the nearest
                                                       exact K-dependent
                                                       negative-energy margin
dynamic divergence-free field residual scale           0.95
```

The hostile names are:

| ray | energy_hostile | graph_hostile |
|---|---|---|
| face `(0,0,1)` | `srp_s1p_s2m_rin_fminus` | `srp_s1m_s2m_rin_fminus` |
| edge `(0,1,-1)` | `srp_s1m_s2m_rin_fminus` | `srp_s1m_s2p_rin_fminus` |
| body `(1,1,1)` | `srp_s1m_s2m_rin_fminus` | `srp_s1p_s2m_rin_fminus` |

For each perturbed checkpoint, recompute the exact two roots of
`K+V(d)=0` and place the radial coordinate at half the nearest admissible
margin. The field operation is fixed by the current FTD-0754 representation:

1. compute the parent's selected radius-four compact dressing `b4_parent`;
2. compute the perturbed matter state's selected radius-four compact dressing
   `b4_perturbed`;
3. set

   `F_perturbed = b4_perturbed + 0.95 (F_parent - b4_parent)`

   in both the registered face-electric and reconstructed edge-magnetic
   representation, with the bound magnetic half-field zero as in FTD-0754.

This retains FTD-0734's energy-adapted phase-space coordinates without
silently importing its older quotient-wide redress into the FTD-0754
finite-support ontology. Any failed initialization is a registered failure
and is not replaced.

## 5. Frozen volume, horizon, and checkpoint matrix

Use odd periodic volumes

```text
L = {321,385}.
```

The candidate matrix is

```text
2 volumes x 3 cubic rays x
  {center, energy_hostile, graph_hostile} = 18 histories.
```

Run each checkpoint state from absolute tick 160 through tick 312 inclusive,
giving 152 accepted transactions and 153 stored states. The `L=321` endpoint
is one tick before the registered FTD-0753 first possible environmental return
to the central candidate; `L=385` supplies a larger causal buffer.

Evaluate `P_core` and all transaction margins every tick. Evaluate the full
state-only observer and `{4,6,8}` ladder only at absolute ticks

```text
{160,200,240,280,312}.
```

The checkpoint thinning is instrumentation-only; it does not thin the core,
action, energy, recoil, speed, or root-regularity histories.

## 6. Frozen root-regularity and open-neighborhood gate

At every accepted transaction, evaluate the unchanged implicit residual
Jacobian by centered differences at `h=2e-7` and `h/2`, without feeding it to
the solver. Reuse the FTD-0735 gates exactly:

```text
minimum singular value                                 >= 1e-3
condition number                                       <= 1e4
two-scale relative sigma_min difference                <= 1e-5
observer-on/off endpoint difference                    <= 1e-12
```

The declared perturbation manifold uses four relative coordinates: the
three-component equal-and-opposite relative impulse inside norm `0.0006`, the
energy-adapted radial coordinate inside half the nearest exact shell margin,
and the one-dimensional divergence-free residual scale in `(0.95,1.05)`.
It is relative-open in the fixed-count, fixed-polarity, Gauss-admissible
complete-state constraint manifold.

On a fixed graph/chart branch, the spline current and field step are smooth,
the compact interaction is polynomial, and a nonsingular implicit root gives
a locally unique continuous step map. Strict finite-horizon margins then make
the inverse image intersection open. Passing the registered gates therefore
supports a nonzero finite-time neighborhood around each passing held-out
checkpoint state, conditional on the numerical Jacobian measurements exactly
as in FTD-0735. It does not prove one uniform analytic radius for the whole
continuous perturbation manifold.

## 7. Frozen nested-volume gate

For the same ray and variant, compare `L=321` and `L=385` after translating
their centers to a common origin. Before tick 313 require:

- identical classification and event branch at every tick;
- corresponding core margins within `2e-13` relative scale;
- constituent relative positions and momenta within `2e-13`;
- local face/edge field values inside Chebyshev radius 48 within `2e-13` at
  registered checkpoints;
- matching first-failed gate and no lifetime-margin collapse.

Global field energies need not match because the volumes contain different
exterior zero regions. Local causal data and classification must match.

## 8. Frozen causal-fibre discriminator

For each volume and ray, fork the unperturbed tick-160 checkpoint and add one
compact Gauss-free electric plaquette of amplitude `1e-3`, with zero added
magnetic field, centered at Chebyshev displacement

| ray | displacement |
|---|---|
| face | `(0,0,96)` |
| edge | `(0,96,-96)` |
| body | `(96,96,96)` |

The oriented four-face loop is the same closed plaquette used by the
FTD-0754B algebraic control. It is a remote divergence-free environmental
fibre, not labelled a photon or incoming wave.

Run baseline and fibre states for exactly 64 transactions. The initial nearest
Chebyshev separation from support radius eight is 88, so one-site-per-tick
local causality forbids contact with the candidate or shell radius 24 during
the discriminator.

Require:

- the remote branch has strictly different global residual/environmental
  energy at its initial state;
- `P_core`, both core margins, constituent state, and every face/edge value
  inside radius 24 remain identical to the baseline at all 65 states;
- both branches pass their own global energy and common-action ledgers;
- support-ladder dressings and bound energies remain identical while global
  residual and cross ledgers may differ.

Failure closes state-fibre independence for this classifier. No remote
amplitude, distance, horizon, or comparison radius may be changed afterward.

## 9. Frozen controls

The same predicate implementation must give:

| control | required result |
|---|---|
| empty/free field with no constituents | reject sector |
| unbound tick-0 prepared pair | reject core |
| early graph contact with positive pair energy | reject core |
| quiet negative-energy reciprocal pair with its compact dressing | accept core; quiet environment |
| fixed imposed source outside `DerivedCompactPair` action | reject sector |
| four-constituent/two-pair state | reject one-object sector |
| legitimate core plus pure incoming or standing residual control | preserve core membership; change environmental diagnostics only |
| translated, proper-cubic-rotated, or polarity-conjugated state | preserve/conjugate classification and margins |

Incoming, standing, and remote content may never be relabelled matter merely
because it is near or energetic.

## 10. Verdict map

Apply the first matching outcome:

1. Parent replay, perturbation construction, CUDA, observer isolation, or
   artifact-integrity failure:
   `M3_VALIDATION_INFRASTRUCTURE_UNRESOLVED`.
2. Any algebraic, symmetry, negative-control, support-ladder, or causal-fibre
   gate fails:
   `M3_STATE_ONLY_CLASSIFIER_INVALID`.
3. Any valid held-out hostile state exits `P_core` before tick 312:
   `M3_FINITE_TIME_FAMILY_CLOSED_NEGATIVE`.
4. All sampled histories survive but a root-regularity or strict-margin gate
   fails:
   `M3_SAMPLED_ROBUSTNESS_ONLY`.
5. All held-out histories, controls, nested-volume comparisons, causal-fibre
   comparisons, strict margins, and regularity gates pass:
   `M3_FINITE_TIME_SELECTED_MATTER_FAMILY`.

The final verdict establishes only FTD-0743 level 3 on the declared selected
finite-time manifold and registered volume/horizon ladder. It does not prove
an invariant basin, asymptotic stability, a fundamental particle, generic
formation measure, autonomous mobility, charge, mass, spin, statistics,
unitarity, or Lorentz recovery.

## 11. Execution firewall

Before any registered trajectory runs, the implementation, independent
certificate, executable, protocol hash, source hashes, output schema, and
absence of `engine/results/ftd_0755/` must be recorded in a pre-execution
audit. Qualification is restricted to at most eight ticks on `L=321`, writes
no registered artifact, and cannot count as validation evidence. The larger
qualification volume is a pre-execution infrastructure correction: the
finite-support CUDA transaction requires the same non-self-contact geometry
as its established parent runner; it does not change a registered arm.

No arm, selector, radius, volume, tick, amplitude, tolerance, predicate term,
or verdict may be altered under FTD-0755 after output is inspected. A failure
may motivate an explicitly new identifier; suggestions and ontology changes
remain permitted there, consistent with the research ledger.

Production defaults, established CUDA, scenarios, and ontology remain
unchanged.
