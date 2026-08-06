# THEOREM — Exact connected-state reservoir decomposition

**Identifier:** `FTD-0673`  
**Status:** `[THEOREM — EXACT SELECTED PERTURBATION LEDGER]`  
**Scope:** the selected connected matter action and matched face/edge field;
observer only

## Statement

Let `X_c` be a connected control state and `X_e=X_c+delta X` an excited
state with the same constituent graph, charge labels, width, orientation, and
periodic volume. Let the exact selected energy be

```text
E(X) = K_rel(X) + V_bind(X) + H(E,B),              (1)
```

where `H` is the matched modified field energy

```text
H(E,B) = (||E||^2+||B||^2)/2
         - (lambda/2)<B,C^T E>.                   (2)
```

Choose a complete mass-orthonormal tangent basis `{v_m}` with positive
frequencies `omega_m`. Define

```text
q_m = <v_m, M delta x>,
p_m = <v_m, delta p>,
e_m = (p_m^2 + omega_m^2 q_m^2)/2.                (3)
```

For any nonempty subset `T` of target modes, set

```text
E_T     = sum_(m in T) e_m,
E_other = sum_(m not in T) e_m,
R_matter = Delta(K_rel+V_bind) - sum_m e_m.       (4)
```

Writing `E_e=E_c+delta E` and `B_e=B_c+delta B`, define

```text
H_dyn = H(delta E,delta B),
I_field = <E_c,delta E> + <B_c,delta B>
 - (lambda/2)[<B_c,C^T delta E>+<delta B,C^T E_c>]. (5)
```

Then the complete excited-minus-control energy has the exact, nonoverlapping
five-term decomposition

```text
Delta E = E_T + E_other + R_matter + H_dyn + I_field. (6)
```

## Proof

Equation (4) is an exact partition by definition:

```text
E_T + E_other + R_matter = Delta(K_rel+V_bind).    (7)
```

Polarizing the quadratic form (2) gives

```text
H(E_c+delta E,B_c+delta B)-H(E_c,B_c)
  = H(delta E,delta B) + I_field.                 (8)
```

The two cross terms in (8) are exactly those in (5). Adding (7) and
(8) proves (6). No continuum energy density, fitted donor, or post-hoc source
term enters the identity.

The exact-rational certificate checks (8) and (6) for two independent linear
curl witnesses and every nonempty target subset (`22` subsets total). The C++
observer independently evaluates full relativistic kinetic energy, graph
binding energy, complete tangent coordinates, matched modified field energy,
and field interference.

## Why the decomposition is needed

The target-mode diagnostic is part of matter energy. Therefore

```text
target mode + binding + field
```

is not an energy ledger: it counts the target mode twice. Equation (6) is the
minimal registered split that preserves the useful tangent-mode diagnostic
while closing against the exact nonlinear matter and field energies.

## Boundary of the theorem

- `R_matter` is a basis- and control-state-dependent nonlinear remainder. It
  is not an additional ontic substance.
- A negative change in one term can identify an energy donor only over a
  preregistered time interval in a conservative control/excited comparison.
- The theorem does not identify `R_matter` with binding. Exact kinetic and
  exact binding differences must be reported separately as a second,
  non-additive description.
- The field interference term is physical within the selected quadratic field
  form but depends on the chosen control background.
- Completeness and mass orthonormality of the tangent basis are required. The
  implementation fails closed on incomplete or nonorthogonal bases.
- This theorem does not establish a particle, a matter pole, separability,
  radiation, or production dynamics.

The observer applies to the selected connected common-action branch. It does
not modify production state, defaults, forces, scenarios, or the five
postulates.
