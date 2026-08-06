# FTD-0758 — M3 fixed-chart held-out validation v1

**Status:** `[PRE-REGISTRATION — FROZEN BEFORE IMPLEMENTATION; NOT RUN]`  
**Date:** 2026-07-30  
**Parents:** FTD-0755 frozen support-invariant validation contract; FTD-0756
wrapper localization; FTD-0757 fixed-chart parent qualification  
**Scope:** fresh held-out validation of the selected finite-time matter-family
candidate; no production, action, dynamics, predicate, parameter, tolerance,
scenario, particle, charge, mass, pole, or Lorentz claim

## 1. Locked question

After repairing only the invalid regional-observer chart, does the unchanged
FTD-0755 support-independent core classifier identify a nonzero relative-open
family of complete reciprocal-pair states that remains classified through the
frozen finite causal horizon, agrees across nested volumes, and is independent
of a remote divergence-free environmental fibre before contact?

FTD-0758 is fresh validation. No candidate, hostile continuation, or fibre
described here has run or been inspected.

## 2. Inherited frozen contract

Inherit the following FTD-0755 protocol sections without modification:

- declared `DerivedCompactPair` sector and support-independent predicate
  `P_core`;
- graph and internal-energy margins and all numerical safety gates;
- selected support ladder `{4,6,8}` and state-only field observer;
- tick-160 parent construction and the `center`, `energy_hostile`, and
  `graph_hostile` variants;
- all six FTD-0734 selector names, impulse/radial coordinates, and residual
  scale `0.95`;
- volumes `{321,385}`, continuation ticks `160,...,312`, checkpoints
  `{160,200,240,280,312}`, root-regularity gates, and nested-volume gates;
- the remote Gauss-free plaquette amplitudes, displacements, 64-transaction
  causal-fibre horizon, and local comparison radius;
- all algebraic, symmetry, negative, and environmental controls;
- first-failed epistemic verdict map and exact scope exclusions.

The inherited source of record is
`PREREG_M3_SUPPORT_INVARIANT_VALIDATION_v1.md`, SHA-256
`1E713DB4B997DAED0D55F098A6E7D63FC0F2D773391CE44FFE03AADD92A504BC`.
No FTD-0755 result artifact is reused as physics evidence.

## 3. Sole correction

In the per-transaction CUDA common-action diagnostic, replace only

```text
regional center = continuously valued constituent midpoint
```

by

```text
C_L = (floor(L/2), floor(L/2), floor(L/2)).
```

Keep the FTD-0755 selected diagnostic radius `{8}` and tolerance `1e-10`.
Record the continuous midpoint only as derived state data; do not round it,
feed it into dynamics, or use it as a membership margin.

The state-only support/checkpoint observer remains unchanged. This correction
is exactly the interface boundary qualified by FTD-0757. The action, implicit
root, ordered current, field update, state transfer, perturbations, predicate,
and every physical gate are byte/formula-identical to FTD-0755.

## 4. Frozen matrix and execution grouping

The physical matrix remains

```text
2 volumes x 3 rays x 3 candidate variants = 18 histories
2 volumes x 3 rays x {baseline, remote fibre} = 12 histories
```

For computational reuse of the already-qualified tick-160 parent, group these
into exactly six registered modes:

```text
--candidates face
--candidates edge
--candidates body
--fibre face
--fibre edge
--fibre body
```

Each candidate mode constructs one parent per volume, copies that complete
state into the three frozen variants, runs them sequentially, and writes three
independent CSV/JSON pairs. Each fibre mode constructs one parent per volume,
then runs the frozen baseline and remote branches. Sharing a deterministic
parent within one mode changes neither a state nor a gate.

The output contract is twelve CSV/JSON pairs under
`engine/results/ftd_0758/`, with the FTD-0755 schemas and the new FTD/protocol
identities. Every metadata record must state `held_out_validation = true` and
`dynamics_changed = false`.

## 5. Frozen qualification and verdict map

Before registered execution, qualification may build the face `L=321`
tick-160 parent and run the unperturbed center for one continuation
transaction. It writes no artifact and cannot count as validation evidence.

Apply the inherited FTD-0755 first-matching verdict map exactly:

1. parent, perturbation, CUDA, observer, or artifact failure:
   `M3_VALIDATION_INFRASTRUCTURE_UNRESOLVED`;
2. algebraic, classifier, support-ladder, nested-volume, or causal-fibre
   failure: `M3_STATE_ONLY_CLASSIFIER_INVALID`;
3. any valid held-out hostile state exits `P_core` before tick 312:
   `M3_FINITE_TIME_FAMILY_CLOSED_NEGATIVE`;
4. sampled histories survive but strict margin or root-regularity fails:
   `M3_SAMPLED_ROBUSTNESS_ONLY`;
5. every candidate, nested-volume, causal-fibre, margin, and regularity gate
   passes: `M3_FINITE_TIME_SELECTED_MATTER_FAMILY`.

The last verdict establishes only the selected FTD-0743 level-3 finite-time
family on this declared constraint manifold, volumes, horizon, and
perturbations. It does not establish an invariant/asymptotic basin, generic
formation measure, autonomous mobility, charge, mass, spin, statistics,
unitarity, or Lorentz recovery.

## 6. Execution firewall

Before any registered mode runs, freeze the protocol, implementation,
independent certificate, WSL2 executable, schemas, qualification, and absence
of `engine/results/ftd_0758/`. Each registered mode may run exactly once. A
failed or interrupted mode may not be rerun, tuned, or replaced under FTD-0758.

FTD-0755 remains consumed and inconclusive. FTD-0756/0757 artifacts remain
immutable. Production defaults, established CUDA libraries, scenarios, and
ontology remain unchanged.
