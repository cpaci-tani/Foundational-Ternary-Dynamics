# AUDIT — Minimum-norm local transaction selector

**Date:** 2026-07-24  
**Identifier:** `FTD-0458`  
**Status:** `[THEOREM — UNIQUE MINIMUM-NORM SELECTOR FOR THE REGISTERED QUADRATIC SHELL]` + `[MEASURED — FULL CUBIC COVARIANCE]` + `[SELECTION — NORM PRINCIPLE NOT NATIVE]`  
**Verdict:** `MINIMUM_NORM_LOCAL_SELECTOR_UNIQUE_CUBIC_COVARIANT`  
**Pre-registration:** [`PREREG_MINIMUM_NORM_TRANSACTION_SELECTOR_v1.md`](../10_eft_program/preregistrations/PREREG_MINIMUM_NORM_TRANSACTION_SELECTOR_v1.md)  
**Run of record:** `engine/results/ftd_0458/windows_msvc_cpu.csv`

## 1. The 104-dimensional ambiguity has one minimum-norm point

For a supported paired field impulse `S`, FTD-0454 established the complete
event functional

```text
E(S) = 1/2 ||S||^2 + c.S,      A S = p,
```

where the three rows of `A` impose the required field recoil. On the registered
36-site support, `S` has 108 real components. The Gram matrix has rank three,
so the fixed-momentum affine space has dimension 105.

Strict convexity gives a unique constrained minimizer `S_min`. Completing the
square on the affine space yields

```text
E(S_min + z) = E_min + 1/2 ||z||^2,    z in ker(A).
```

When `E_min<0`, zero energy therefore fixes a sphere of radius

```text
r = sqrt(-2 E_min)
```

inside the 105-dimensional nullspace. The zero-energy shell has dimension 104.

Let `n=P_ker(A)c`. The KKT equation implies

```text
P_ker(A) S_min = -n.
```

For every point on the zero-energy sphere,

```text
||S_min+z||^2
  = ||S_min||^2 + r^2 - 2 n.z
  >= ||S_min||^2 + r^2 - 2 r ||n||.
```

Cauchy-Schwarz has one equality point whenever `n!=0`:

```text
S_* = S_min + r n/||n||.
```

This is exactly the covariant-null construction already used by the prior
campaigns. Thus the existing solver was not choosing an arbitrary point on the
shell: it was implicitly choosing the unique minimum-impulse-norm point.

## 2. Independent certificate

The new certificate independently reconstructs `A`, `c`, the Gram inverse,
`P_ker(A)c`, and the shell geometry from native pre-event fields. It does not
trust the solver's internal projected vector.

For the frozen FTD-0457 packet event:

| Quantity | Value |
|---|---:|
| Ambient dimension | `108` |
| Constraint rank | `3` |
| Nullity | `105` |
| Zero-shell dimension | `104` |
| `E_min` | `-0.00431376107709` |
| Shell radius `r` | `0.0928844559341` |
| `||P_ker(A)c||` | `0.103260475023` |
| Selected/lower-bound norm residual | `2.60e-18` |
| Solver/certificate direction residual | `2.28e-18` |

Five independent alternatives at angles from `pi/8` through `pi` remained on
the same energy-and-momentum shell but had norm-squared at least
`0.00146018742` above the selected point. The registered degeneracy falsifier
did not fire.

## 3. The selector is cubic covariant

The complete native seed—coordinates, `J`, `W`, support, oriented hop, recoil,
and impulse—was transformed under all 48 signed axis permutations. Every arm
was independently re-solved and re-certified.

- covariance arms passed: `48/48`;
- worst transformed minimum-energy residual: `2.60e-18`;
- worst selected-impulse residual: `2.10e-17`;
- worst complete event-energy residual: `5.57e-17`;
- worst momentum residual: `7.29e-17`;
- worst add/remove reversal residual: `9.70e-19`.

Minimum Euclidean impulse norm introduces no preferred face or coordinate axis.
It commutes with the full cubic symmetry of the lattice for this native event.

## 4. What is theorem and what remains selected

The following statement is theorem-grade:

> Given the registered quadratic event energy, a full-rank momentum map,
> `E_min<0`, and `P_ker(A)c != 0`, the fixed-momentum zero-energy shell has a
> unique minimum-Euclidean-norm impulse, equal to the covariant-null formula.

The following statement is not derived:

> Nature executes the minimum-Euclidean-norm impulse.

Euclidean norm is natural because it is local, positive, strictly convex, and
cubic invariant. Those properties make it a disciplined `[SELECTION]`, not a
sixth theorem forced by the five postulates. A different additional functional
could choose another shell point while preserving energy and momentum.

## 5. Ontological consequence

The event story can now be stated without a hidden continuous tuning:

1. The incoming packet and local bound field define `A`, `c`, and the required
   recoil.
2. If `E_min>0`, no conserving transaction exists in the registered support.
3. If `E_min<=0`, a zero-energy shell exists.
4. The minimum-norm principle selects one deterministic, local, cubic-covariant
   transaction from that shell.
5. Applying and subsequently removing that transaction is reversible.

The remaining ambiguity has moved upward: not *which impulse* to use once the
principle is adopted, but *why the substrate adopts that principle* and *which
candidate hop is evaluated*.

## 6. Next decisive gate

Test sequential self-consistency without resetting the field. Starting from the
FTD-0457 packet, the observer must:

1. compute actual endpoint work for a candidate hop;
2. update particle momentum with the corrected production dispersion;
3. apply the unique minimum-norm local recoil;
4. move the manifested site;
5. evolve the resulting `J/W` state one tick;
6. ask whether the next forward transaction exists using only that evolved
   state—not a freshly inserted bound dressing.

Compare forward, reverse, and competing-neighbor transaction costs. If several
neighbours remain eligible, a direction rule is still missing. If the first
event destroys the capacity for the second, FTD-0457 was a one-shot
counterfactual rather than mechanics.

## 7. Scope and reproducibility

The native covariance measurement transforms one face-hop packet event; it does
not test primitive edge/corner transaction supports, sequential motion, or the
production movement phase. No production dynamics changed.

- campaign SHA256: `984FE695762F7EB3F434B37D58F4614804F5904E56A0D99177116628E6B7A50E`
- certificate SHA256: `B9971BD60068B2806B7C97A914274E2B9C1A909F316579391CD45833EBBAFF60`
- record SHA256: `B8235188592E425807B92D88E650CBD034A0B9BE735DE9031931B57DB0C4293A`
- compiler: pinned MSVC `14.44.35207`, Release

