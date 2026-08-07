# AUDIT — Knot Legendre branch

**Date:** 2026-07-25  
**Identifier:** `FTD-0491`  
**Status:** `[THEOREM — EIGHT EXACT SYMMETRIC KNOT BRANCHES]` +
`[CONSTRUCTIVE CONTROL — GENERIC BIAS SELECTS ONE]` +
`[CLOSED NEGATIVE — UNCONDITIONAL ALGEBRAIC INVERSION]`  
**Verdict:** `SYMMETRIC_KNOT_HAS_EIGHT_LEGENDRE_BRANCHES`  
**Pre-registration:**
[`PREREG_KNOT_LEGENDRE_BRANCH_v1.md`](../../10_eft_program/preregistrations/common_action_mechanics_reciprocity/PREREG_KNOT_LEGENDRE_BRANCH_v1.md)  
**Run of record:** `engine/results/ftd_0491/windows_msvc_cpu.json`

## 1. Exact counterexample

At a manifested site `i`, the registered matched field has

```text
E_a(i)=q/6,
E_a(i-e_a)=-q/6.
```

Hence

```text
D E(i)=sum_a(E_a(i)-E_a(i-e_a))=q.
```

Each of the eight incident cells has constant local electric vector

```text
E_sigma=q sigma/6,
sigma in {-1,+1}^3.
```

For `A0=0`, `A1=-lambda E`, the exact initial discrete Legendre equation at
zero input momentum is

```text
P0=p(d)-(g q lambda/2)E_sigma=0.
```

It has the analytic solution

```text
p_sigma=(g lambda/12)sigma,
d_sigma=lambda c p_sigma/sqrt(E_REST^2+c^2|p_sigma|^2).
```

Every `d_sigma` points strictly into its own incident cell. Therefore the same
physical knot state, the same zero kinetic momentum, and the same field admit
eight distinct outgoing endpoints. This is an existence proof, not a search.

## 2. Measurement closure

Both polarities and all eight cells were evaluated at
`epsilon={1e-4,1e-6,1e-8}`. All 48 symmetric branch arms close.

| diagnostic | result |
|---|---:|
| Gauss residual | `2.22e-16` |
| initial kinetic-momentum residual | `8.47e-16` |
| analytic matter-momentum residual | `8.33e-16` |
| gauge kinetic residual | `1.11e-16` |
| cubic orbit magnitude residual | `0` |
| epsilon endpoint residual | `0` |
| polarity-mirror residual | `0` |
| symmetric solved branches | `8` |

The arbitrary gauge changes canonical momenta and connection traces together;
it does not remove or relabel any branch.

## 3. Generic-bias control

Adding the divergence-free uniform field `E_bias=(0.4,0.5,0.6)` leaves the
central Gauss source unchanged and yields exactly one sign-consistent incident
cell for each polarity. The canonical equations can select a unique branch
when the physical state itself breaks the knot symmetry.

This control localizes the defect: gauge covariance and algebraic inversion
are not generally broken. Uniqueness fails on an allowed symmetric state.

## 4. Consequence

The frozen variables carry no datum distinguishing the eight symmetric
solutions. An unconditional deterministic update must add a rule not contained
in the differentiable action:

- choose the symmetry-fixed rest solution using a generalized derivative;
- retain an incoming/history direction;
- choose randomly;
- smooth the compact shape;
- or sum branch amplitudes in a new quantum ontology.

The first option is the only local deterministic cubic-covariant candidate and
is tested separately as a centered weak-trace selection. It is not silently
part of FTD-0490.

No production toggle, scenario, or IR claim is licensed.

## 5. Reproducibility

- test SHA256:
  `5662BA3AA3308B447ED9E6C2BC78C1D382599391EBDFAA1F00C9D3302F95ACAE`;
- header SHA256:
  `386AA9E110CE967483B69297A325B02167819A2F0A1EA8A6EE98FC40AE5B78CC`;
- implementation SHA256:
  `39BF37D57179093B7B22560417EB9772D3CEA88D8452D0EA7ED6610EA244547E`;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU observer;
- production state: unchanged.
