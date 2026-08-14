# FTD-0893 — dressed-boost momentum-map and inertial-identifiability boundary v1

**Identifier:** `FTD-0893`  
**Status:** `[PRE-REGISTRATION — LOCKED/PRE-RUN]`  
**Date:** 2026-08-11  
**Production status:** unchanged

## 1. Question

The selected connected common action has exact energy bookkeeping and admits
co-moving dressed configurations. FTD-0892 also identifies an exact collective
matter momentum inside selected constituent phase space. Does this information
already determine the inertial mass of the complete dressed object, including
its field coat?

The registered answer is a theorem/no-go pair. Near a stable rest solution,
energy curvature determines dressed inertia only after an independently
defined additive physical momentum map is supplied. The present selected
common action does not yet supply an exact total field--matter momentum map.

## 2. Frozen sources

| source | SHA256 |
|---|---|
| `THEOREM_COLLECTIVE_REACTION_TRIPLET_AND_INERTIAL_CURVATURE_BOUNDARY_v1.md` | `CFD4E0EE4E0193BD435D7A6F9DF42EF589551078322C925A46A7E3693CDB2371` |
| `ANALYSIS_SPLINE_POYNTING_NOETHER_DEFECT_v1.md` | `2D63051782D1648F51FE9EA8A7B90FE9FF38827C119D9C8033A12953F5389DF5` |
| `ANALYSIS_CELL_MEASURE_COMMON_ACTION_CLOSURE_v1.md` | `6F87DE2CC0559492322453E824D971BEBFE680512C6C1A8D4CCCCF5324F48A68` |
| `ANALYSIS_MOBILE_DRESSING_STRUCTURE_FACTOR_v2.md` | `D7859E19D50EE0D6B913D838C60BF1B24146B2AC50B94DA0163645CCB685601C` |
| `ANALYSIS_REST_QUALIFIED_MOVING_DRESSING_RELATIVE_ORBIT_v1.md` | `53B38713C5E545A68C8B0B6D188E2953220E1530F16D1C327FC41608C0CB0371` |
| `connected_moore_block_action.h` | `09328FB23642D3D8AFD165994F8F8B2101A52DD7E0BC5BFEE2E2DF27ABE6EDF8` |
| `connected_moore_block_action.cpp` | `207002636F290E9C55BB33FDFED489C423EEC5BFA3C0986D4E320A460E3F0262` |
| `spline_poynting_momentum.h` | `AEF46732679E23CA187EBCFFAC288AAFAE88BEE0409AE9698C58A532D9728474` |
| `spline_poynting_momentum.cpp` | `C2ECAFEEAA4B77E71673D5560C8606AF34FDE50084E7B3AA44A2199B3929B300` |

Any source-hash mismatch invalidates the certificate.

## 3. Registered local theorem

Let the complete time-odd tangent state near a stable rest configuration be
`y in R^d`, with

```text
H(y) = E0 + (1/2) y^T A y + O(|y|^3),
P(y) = B y + O(|y|^2),
```

where `A` is symmetric positive definite and `B` is a rank-three linearization
of an additive physical total-momentum map. At fixed small physical momentum
`P`, the constrained quadratic minimum must be

```text
y*(P) = A^-1 B^T (B A^-1 B^T)^-1 P,
E_eff(P) = E0 + (1/2) P^T (B A^-1 B^T)^-1 P + O(|P|^3).
```

Therefore the conditional dressed inertial tensor is

```text
M = B A^-1 B^T.
```

This is conditional on both `A` and `B`. It is not an absolute mass
derivation from static energy or stability data.

## 4. Exact two-channel reference realization

For each cubic axis use a matter-like odd amplitude `p` and a field-like odd
amplitude `f`:

```text
A = [[a, g], [g, k]],       a > 0, k > 0, a k - g^2 > 0,
P = b_m p + b_f f.
```

The registered exact result is

```text
M = (k b_m^2 - 2 g b_m b_f + a b_f^2)/(a k - g^2),
[p*, f*]^T = A^-1 [b_m, b_f]^T P/M,
E_min - E0 = P^2/(2M).
```

The reference is replicated identically on the three axes, so the inertial
tensor is `M I_3` and is covariant under signed cubic permutations. This model
is an `[IMPOSED reference realization]`, not a production claim.

## 5. Identifiability controls

The exact certificate must establish all of the following.

1. `E0` and any static binding offset disappear from `M`.
2. With the same `A`, replacing `B` by `s B` sends `M` to `s^2 M`.
3. Hence static energy, static stability, and the complete energy Hessian do
   not identify physical inertia without the momentum map.
4. Dressing contributes to inertia only when a field-like odd direction is
   included in the admissible tangent state and participates through `A`,
   `B`, or both.
5. The matter-only control `b=(1,0)`, `g=0`, `a=1/m` returns `M=m`.
6. A chosen moving path `y(zeta)` does not replace the momentum map: its
   apparent curvature is path- and parameter-normalization dependent.
7. Exact `Z^3` translation covariance does not by itself create a continuous
   `R^3` Noether generator.
8. The frozen spline-Poynting candidate remains a failed coupled-recoil
   ledger, not an exact total momentum charge.

## 6. Certificate gates

The source-locked SymPy certificate must test:

- all nine source hashes and source-scope markers;
- the general KKT minimizer, constraint, stationary equation, minimum energy,
  symmetry, and positive definiteness of `M`;
- an exact higher-dimensional rank-three rational witness;
- the two-channel inverse, allocation, constraint, energy, and mass formula;
- matter-only, field-participating, coupling, static-offset, rescaling,
  path-normalization, and cubic-covariance controls;
- exact statement of the present common-action/spline-Poynting boundary;
- terminal scope markers and fail-closed aggregate verdict.

The certificate may prove conditional mathematics and the identifiability
no-go. It may not infer a production momentum map from numerical co-motion.

## 7. Outcome map

- **Outcome A:** exact conditional theorem and identifiability boundary pass.
  Book `M=B A^-1 B^T` as the local dressed-mass theorem and keep the current
  total-momentum map/absolute mass debt open.
- **Outcome B:** algebra passes but the frozen corpus already supplies a valid
  exact total field--matter momentum map. Identify it explicitly before any
  promotion.
- **Outcome C:** the algebra or source boundary fails. No theorem is booked.
- **Execution invalid:** any hash, certificate, or terminal-gate failure.

## 8. Post-certificate implementation

Only after a passing locked certificate, add an isolated EFT analyzer for the
two-channel reference realization. It must fail closed outside the
positive-definite domain and expose these negative flags explicitly:

```text
TOTAL_MOMENTUM_MAP_DERIVED=FALSE
ABSOLUTE_MASS_DERIVED=FALSE
COMMON_ACTION_NOETHER_CLOSURE=FALSE
PRODUCTION_COUPLING=FALSE
BORN_TARGET_USED=FALSE
NATIVE_GSTAR_SYNCHRONIZATION=FALSE
```

No production `Voxel`, tick phase, default toggle, Born selector, or clock path
may change.

## 9. Next acceptance gate

A physical dressed mass is promotable only after an independent local
substrate stress/momentum state or exact operational quasimomentum ledger is
derived and the same tensor is recovered from:

1. constrained energy curvature;
2. impulse divided by center velocity; and
3. matter--field momentum partition.

Disagreement is a stop condition.

## 10. Scope firewall

```text
DRESSED_MASS_FORMULA=EXACT_CONDITIONAL_ON_A_AND_B
ENERGY_HESSIAN_ALONE_IDENTIFIES_MASS=FALSE
STATIC_REST_OFFSET_CONTRIBUTES_TO_INERTIA=FALSE
TOTAL_FIELD_MATTER_MOMENTUM_MAP=OPEN
SPLINE_POYNTING_COUPLED_RECOIL_LEDGER=FAILED_CANDIDATE
ABSOLUTE_MASS_SCALE=NOT_DERIVED
STABLE_MATTER_POLE=OPEN
PRODUCTION_INTEGRATION=FORBIDDEN
NO_NEW_SELECTED_VECTOR_TYPE=TRUE
GSTAR_BORN_BELL_LORENTZ_COMPLETENESS=UNTOUCHED
```

The exact SHA256 of this protocol and its certificate must be entered in the
preregistration manifest before first execution.
