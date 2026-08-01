# THEOREM — Canonical mass metric for tangent-mode energy

**Identifier:** `FTD-0675`  
**Status:** `[THEOREM — DIAGNOSTIC CORRECTION]`  
**Scope:** finite-dimensional tangent modes of the selected connected matter
action

## Statement

Let the quadratic matter Hamiltonian about a control state be

```text
H_2 = (delta p)^T M^-1 (delta p)/2
      + (delta x)^T K (delta x)/2,                 (1)
```

and let the complete generalized eigenbasis satisfy

```text
K v_m = omega_m^2 M v_m,
v_m^T M v_n = delta_mn.                            (2)
```

The canonical modal coordinates are

```text
q_m = v_m^T M delta x,
P_m = v_m^T delta p.                               (3)
```

They give

```text
H_2 = sum_m (P_m^2 + omega_m^2 q_m^2)/2.           (4)
```

The displacement projection `v_m^T delta x` is not `q_m`. For scalar
`M=mI`, it is `q_m/m`, so using it in (4) overweights modal potential energy
by `1/m^2` while leaving the momentum term unchanged.

## Proof

Writing the eigenvectors as columns of `V`, completeness and (2) give

```text
V^T M V = I,
delta x = V q,
delta p = M V P.                                   (5)
```

The symplectic one-form is preserved because

```text
(delta p)^T d(delta x) = P^T V^T M V dq = P^T dq. (6)
```

Substituting (5) into (1) and using (2) proves (4). For `M=mI`,

```text
v_m^T delta x = (v_m^T M delta x)/m = q_m/m.       (7)
```

This proves the stated overweight.

An exact counterexample makes the consequence explicit. For one isolated
harmonic mode,

```text
q=A sin(theta),  P=omega A cos(theta).
```

The canonical energy is constant, `omega^2 A^2/2`. The unweighted diagnostic
is

```text
E_wrong(theta) = omega^2 A^2
                 [cos^2(theta)+sin^2(theta)/m^2]/2, (8)
```

which has false troughs and recoveries with max/min ratio `1/m^2` even though
no energy transfer occurs.

For the selected FTD constituent mass `m=M_INERTIAL=K_B`, the factor is
approximately `3.83`. Therefore a turning observed only in the unweighted
diagnostic is not evidence for reservoir return.

## Correction consequence

The helper `paired_modal_coordinates` used by FTD-0664, FTD-0665, FTD-0668,
FTD-0670, and FTD-0672 projected displacement as `v^T delta x`. FTD-0673 uses
the canonical mass metric. Consequently:

- FTD-0670's claimed action-envelope turning is retracted;
- the “doublet recovery” interpretation attached to FTD-0672 is retracted;
- FTD-0672's independently exact regional field-transport and current-work
  measurements remain valid as field statements;
- all earlier modal-energy conclusions using that helper require correction
  or replay before citation.

This theorem changes an observer, not the selected or production dynamics.
