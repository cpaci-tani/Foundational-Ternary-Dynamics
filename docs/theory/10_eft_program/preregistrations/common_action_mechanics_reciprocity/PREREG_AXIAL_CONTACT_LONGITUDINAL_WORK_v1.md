# PRE-REGISTRATION — Axial contact longitudinal work

**Date locked:** 2026-07-25  
**Identifier:** `FTD-0530`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN; NONZERO-WORK HYPOTHESIS REJECTED]`  
**Parents:** `FTD-0497`, `FTD-0527`, `FTD-0528`, `FTD-0529`  
**Scope:** observer-only fixed-path longitudinal-work audit of the 72 curl-free
axial FTD-0527 histories. No production state, default, toggle, scenario,
force, collision law, phase order, field ontology, normalization, or tolerance
change.

## 1. Registered theorem

FTD-0529 measured `C^T K=0` on every axial contact current. The two equal and
opposite carrier displacements also give zero harmonic current:

```text
sum_faces K = q(d_1+d_2)=0.
```

On the periodic matched complex, any two fields with the same divergence differ
by a curl plus a constant harmonic face field. Therefore

```text
<K,E_2-E_1>
  = <C^T K,A> + h dot sum_faces K
  = 0.
```

Unlike the edge/corner sector, axial current work is uniquely fixed by the
Gauss source. A transverse or constant compatible perturbation cannot tune it.

## 2. Registered fixed-path energy test

Use the same uniformly neutralized fractional density and deterministic routed
Gauss witness as FTD-0529. Embed the field in the full staggered step with
`B_before=C_SPEED C^T E_star`, then deposit the exact axial current. Compare:

1. the baseline field;
2. the baseline plus `C A`, where `A` is the deterministic transverse edge
   challenge at amplitude `0.1`;
3. the baseline plus a constant harmonic face field of amplitude `0.1` along
   the contact axis.

The exact normalized work and field-energy change must agree across all three
inputs below `1e-12`.

The unchanged FTD-0527 rebase keeps both matter energies fixed. If the common
field change is nonzero, define the zero-COM equal-energy magnitude required to
pay it on the frozen current path:

```text
H_required = H_initial - Delta H_field/2,
p_required = sqrt(H_required^2-E_REST^2)/C_SPEED,
v_required = C_SPEED^2 p_required/H_required.
```

This is only a fixed-path correction diagnostic. Changing the momentum changes
the displacement and exact current, so it does not construct the required
self-consistent transaction.

## 3. Registered arms and gates

Use both polarities, all six face directions, speeds `1/8` and `1/4`, and three
translations (`72` arms). Require:

1. exact history quotient, continuity, and absolute before/after Gauss below
   `1e-12`;
2. `||C^T K||^2` and the total harmonic current below `1e-12`;
3. transverse and harmonic work differences below `1e-12`;
4. all three staggered midpoint-energy identities below `1e-12`;
5. the unchanged elastic total-energy residual above `1e-10` on every arm;
6. a real `H_required>E_REST`, `0<v_required<C_SPEED`, and nonzero required
   impulse on every arm;
7. substituting `2(H_required-H_initial)` closes the frozen-path energy ledger
   below `1e-12`;
8. translation, polarity-mirror magnitude, and signed-cubic axis covariance
   below `1e-12`;
9. invalid inputs fail closed.

## 4. Locked verdicts

- If every gate passes:
  `AXIAL_ELASTIC_CONTACT_ALSO_REQUIRES_FIELD_DEPENDENT_LONGITUDINAL_IMPULSE`.
- If the common work vanishes and unchanged matter closes:
  `AXIAL_ELASTIC_CONTACT_IS_RECIPROCAL_ON_FIXED_PATH`.
- If work changes under same-Gauss perturbations or an algebraic identity fails:
  `AXIAL_LONGITUDINAL_WORK_AUDIT_UNRESOLVED`.

The first verdict would close unchanged FTD-0527 elastic composition in all 26
Moore directions: diagonals by FTD-0529's transverse counterfamily and axes by
Gauss-fixed longitudinal work. It would not close reciprocal contact itself.
The next construction would be the self-consistent simultaneous solve for
outgoing pair momenta, endpoints, exact currents, and field update.

## 5. Execution record

Executed 2026-07-25 with pinned MSVC `14.44.35207`, Release, CPU observer.
The locked preregistration SHA256 before this execution annotation was
`2B61DD31DF9E020488DEA6087C3036DB122646FB3E1D61147FD514FD0266AE77`.

The preregistered nonzero-work gates failed. On all 72 arms the summed exact
face current and endpoint-density change were identically zero, so transverse
and harmonic field challenges changed the work by exactly zero. The apparent
energy defect was only `1.89e-15..7.66e-15` floating-point cancellation. The
locked alternative verdict applies:

```text
AXIAL_ELASTIC_CONTACT_IS_RECIPROCAL_ON_FIXED_PATH
```

Canonical result:
[`AUDIT_AXIAL_CONTACT_LONGITUDINAL_WORK.md`](../../../07_assessment/common_action_mechanics_reciprocity/AUDIT_AXIAL_CONTACT_LONGITUDINAL_WORK.md).
