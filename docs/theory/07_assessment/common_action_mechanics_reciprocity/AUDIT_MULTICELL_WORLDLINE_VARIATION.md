# AUDIT — Multi-cell worldline variation

**Date:** 2026-07-25  
**Identifier:** `FTD-0533`  
**Status:** `[CONSTRUCTIVE + NUMERICAL FACT — UNIQUE INTERNAL-KNOT VARIATION]` +
`[RESOLVED — FTD-0532 GLOBAL ACTION DOMAIN]` +
`[UNCHANGED CLOSED NEGATIVE — CHARGED ENDPOINT THRESHOLD]` +
`[CLOSED NEGATIVE BY FTD-0534 — SINGLE-SLAB FTD-0531 COMPOSITION]` +
`[CLOSED NEGATIVE BY FTD-0536 — FTD-0531 ROOT STATIONARITY]` +
`[OPEN — NEW NONLINEAR ATOMIC ROOT]`  
**Verdict:** `GLOBAL_DEPOSITED_ACTION_HAS_UNIQUE_INTERNAL_KNOT_VARIATION`  
**Pre-registration:**
[`PREREG_MULTICELL_WORLDLINE_VARIATION_v1.md`](../../10_eft_program/preregistrations/common_action_mechanics_reciprocity/PREREG_MULTICELL_WORLDLINE_VARIATION_v1.md)  
**Run of record:** `engine/results/ftd_0533/windows_msvc_cpu.json`

## 1. Correction to the apparent missing action

FTD-0532 proved that the compact FTD-0485 evaluator rejects every constructive
diagonal hop because its next segment crosses two or three cell planes. The
global action itself was not missing. FTD-0484 already partitions an arbitrary
straight worldline at every integer plane and contracts the resulting exact
spacetime current with the cubical connection.

What was missing was the derivative of that complete deposited action. For a
joined two-slab history, FTD-0533 evaluates

```text
S_int(x*)=S_int^-(x_-,x*)+S_int^+(x*,x_+)
```

and varies `x*`. Every probe rebuilds the plane intersections, cell sequence,
temporal deposits, and spatial face current. It never selects an incident-cell
force or holds a crossing parameter fixed.

## 2. Internal-knot result

On a strict one-cell path, the recovered gradient agrees with the analytic
FTD-0485 automatic-differentiation result to
`4.721689619152647e-14`.

On nonzero general connected fields, paths containing one internal face, edge,
or corner crossing all converge to a single gradient. Across the registered
step hierarchy, the worst last-level centered-gradient change is
`6.017771170263586e-9`, and the worst mismatch between that gradient and all 26
signed Moore directional derivatives is `4.045788822015783e-9`.

The corner one-sided gap sequence is

```text
9.560565672472876e-10
2.611884042380552e-10
6.834000032540644e-11
1.739408617140725e-11.
```

It decreases rather than approaching a finite cusp. This is numerical evidence
at the registered resolution, not an exact differentiability theorem.

All 240 actual FTD-0532 edge/corner geometries are accepted by the global
evaluator. Their zero-connection gradients and convergence residuals are
exactly zero, while the break classifier recovers one simultaneous crossing of
multiplicity two or three.

## 3. Gauge and threshold discriminators

A connected gauge transformation with one common intermediate gauge value
changes the internal-knot gradient by at most
`1.184239338536397e-13`. A nonzero pure-gauge pair leaves a residual gradient
`4.500103993147301e-14`. This is the expected cancellation of the intermediate
endpoint terms between the two slabs.

The result does not weaken FTD-0487. Placing the varied particle endpoint on
the charged threshold still gives distinct side limits with impulse gap

```text
0.028867513459481284.
```

The distinction is now exact in scope:

- an **internal integration knot** can be crossed by the complete worldline
  action without choosing a force branch;
- a **varied endpoint on a charged threshold** retains the Gauss-forced
  one-sided ambiguity.

## 4. Consequence and remaining gate

FTD-0533 removes the FTD-0532 geometry obstruction to evaluating a vector
interaction impulse along the diagonal hop. It does not prove that the
FTD-0531 scalar energy root is a stationary point of the total matter-field
action.

That comparison requires a dynamical connection history representing the
FTD-0531 staggered electric/magnetic transaction and the free matter action.
The next test must compare the complete vector discrete Euler--Lagrange
residual with the scalar energy root, without choosing a new force gather or
retuning the endpoint.

FTD-0534 proves that no **single** connection slab can do so on a diagonal
root: the midpoint work field and staggered magnetic endpoints violate
Faraday by exactly `-(lambda/2)C^T K`. A multistage or phase-space action is
therefore required before the stationarity comparison can be posed.

FTD-0536 poses that comparison with the minimal atomic action. Its field
equations close, but none of the 240 FTD-0531 scalar roots is stationary. The
remaining problem is a new simultaneous nonlinear root, not action-domain
coverage or an incident-cell derivative.

No production code, default, toggle, scenario, force, collision law, phase
order, field ontology, normalization, or tolerance changed.

## 5. Reproducibility

- checks: `9/9 PASS`, including `240` actual diagonal geometries;
- test SHA256:
  `79935CA2DDA1CCD7CB20EBF879415EA46C95F03DD6C130CC65BC0FA206575D58`;
- header SHA256:
  `D41FBB57BEBB286818811CCA6726DA64961626929103D87A05DEEF91DF2B946B`;
- implementation SHA256:
  `A06C65F9C3587ACDAD96A9003331F9EE8D23C72029D54BB03402FAB819EBB93C`;
- locked preregistration SHA256:
  `3A9AB2FBE62921DB3D847843C12F9A731B9A5B6EE05DD28EBA63FBA915E8CF3F`;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU observer;
- production state and defaults: unchanged.
