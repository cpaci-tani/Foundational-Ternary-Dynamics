# DERIV: Discrete Tick Energy Invariant

**Date:** 2026-06-13
**Status:** [THEOREM -- SOURCE-FREE LINEAR TICK ENERGY AND LOCAL CURRENT] / [OPEN -- COUPLED SOURCE WORK]
**Campaigns:** FTD-0292, FTD-0293, FTD-0294, FTD-0295
**Engine anchors:** `phase_read` + `phase_write`

---

## 1. Scope

This note applies to the source-free, single-substrate, no-damping, no-genesis
wave update:

```text
W' = W + c^2 L J
J' = J + W'
```

where `L` is the engine's symmetric 18-neighbor Laplacian and `c = C_WAVE`.
It does not include state coupling, particle motion, damping, Gauss projection,
dual substrate, pair production, weak terms, or de Broglie clock terms.

---

## 2. Mode Proof

Let:

```text
K = -c^2 L
```

For any eigenmode of `K` with eigenvalue `k`, the update is:

```text
w' = w - k q
q' = q + w' = (1-k) q + w
```

or:

```text
[q']   [1-k  1] [q]
[w'] = [ -k  1] [w]
```

Call this matrix `A`. A quadratic invariant has a symmetric matrix `M`
satisfying:

```text
A^T M A = M
```

Solving gives, up to an overall scale:

```text
M = [[ k, -k/2 ],
     [ -k/2, 1 ]]
```

Therefore each mode preserves:

```text
E_mode = 0.5 w^2 + 0.5 k q^2 - 0.5 k q w
```

Because the lattice operator is symmetric, summing over modes gives the global
source-free tick invariant:

```text
E_tick = 0.5 W^T W + 0.5 J^T K J - 0.5 W^T K J
```

Since `delta = c^2 L J = -KJ`, the engine-observable form is:

```text
E_tick = 0.5 W^2 + E_grad + 0.5 W dot delta
```

with:

```text
E_grad = 0.5 J^T K J
```

---

## 3. Engine Confirmation

FTD-0292 v1 used ordinary double accumulation. It showed the right pattern but
missed the frozen relative gate:

```text
max_abs_modified_drift = 2.1104895608914376e-11
max_rel_modified_drift = 2.5509106971021582e-12
verdict = DISCRETE_TICK_INVARIANT_INVALIDATED
```

FTD-0293 v2 kept the same update, initial condition, formula, and gates, but
used long-double Kahan accumulation:

```text
max_abs_naive_drift = 0.798666323156913549042
max_abs_modified_drift = 7.10542735760100185871e-15
max_rel_modified_drift = 8.58820199343822426235e-16
verdict = DISCRETE_TICK_MODIFIED_ENERGY_CONFIRMED
```

---

## 4. Local Finite-Volume Current

Use the per-site density:

```text
h_i = 0.5 |W_i|^2 + 0.5 J_i dot (KJ)_i - 0.5 W_i dot (KJ)_i
```

For one source-free tick:

```text
W' = W - KJ
J' = J + W'
```

The local change is:

```text
Delta h_i
  = 0.5 [J_i dot (KW')_i - W'_i dot (KJ)_i]
```

On the weighted graph, this is an antisymmetric edge sum:

```text
Delta h_i
  = sum_j 0.5 c^2 w_ij [W'_i dot J_j - J_i dot W'_j]
```

Therefore the outward current from inside site `i` to outside site `j` is:

```text
Phi_i->j = 0.5 c^2 w_ij [J_i(old) dot W_j(next) - W_i(next) dot J_j(old)]
```

and every finite volume satisfies:

```text
Delta H_V + Phi_out(boundary V) = 0
```

where:

```text
H_V = sum_{i in V} h_i
```

## 5. Engine Confirmation

FTD-0294 v1 tested this current with the correct density/current but an
exchange-relative denominator that becomes degenerate on quiet exchanges:

```text
max_abs_balance = 4.66471208448310248329e-16
max_exchange_rel_balance = 1
verdict = SOURCE_FREE_LOCAL_TICK_CONTINUITY_INVALIDATED
```

FTD-0295 v2 kept the density/current unchanged and used a finite-volume
energy-scale relative denominator:

```text
max_abs_balance = 4.66471208448310248329e-16
max_scale_rel_balance = 2.98137309593416839599e-16
verdict = SOURCE_FREE_LOCAL_TICK_CONTINUITY_CONFIRMED
```

## 6. Consequence

The naive continuum energy:

```text
E_naive = 0.5 W^2 + E_grad
```

is not the conserved energy of the actual tick. Any native finite-volume
continuity law must account for the cross term:

```text
0.5 W dot c^2 L J
```

This explains why FTD-0291's finite-volume current candidate failed the
free-wave control: it balanced the wrong energy.

---

## 7. Open Work

Next target:

```text
Derive source/work terms for state coupling in
Delta H_V + Phi_out = Work/source
```

Then return to the charge-plus-beam recoil setup with the source-free boundary
current already fixed.

No radiation, Thomson cross-section, QED amplitude, or alpha claim follows
from this theorem alone.
