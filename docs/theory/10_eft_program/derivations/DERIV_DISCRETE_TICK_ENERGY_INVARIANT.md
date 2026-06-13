# DERIV: Discrete Tick Energy Invariant

**Date:** 2026-06-13
**Status:** [THEOREM -- SOURCE-FREE LINEAR TICK] / [OPEN -- LOCAL CURRENT]
**Campaigns:** FTD-0292, FTD-0293
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

## 4. Consequence

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

## 5. Open Work

The global invariant is not yet a local current theorem.

Next target:

```text
Derive a finite-volume identity for Delta E_tick(V) + Flux_tick(boundary V) = 0
```

for the source-free linear tick, then add state-coupling source/work terms only
after the free-wave identity closes.

No radiation, Thomson cross-section, QED amplitude, or alpha claim follows
from this theorem alone.
