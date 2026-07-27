# AUDIT — Open-worldline hop selection

**Date:** 2026-07-25  
**Identifier:** `FTD-0489`  
**Status:** `[THEOREM — OPEN ACTION ORDERING IS GAUGE-DEPENDENT]` +
`[THEOREM — CUBIC-SYMMETRIC NONZERO SELECTOR IMPOSSIBLE]` +
`[CONSTRUCTIVE CONTROL — PRIOR KINEMATICS SELECTS ENDPOINT]`  
**Verdict:** `OPEN_WORLDLINE_ACTION_NOT_A_HOP_SELECTOR`  
**Pre-registration:**
[`PREREG_OPEN_WORLDLINE_HOP_SELECTION_v1.md`](../10_eft_program/preregistrations/PREREG_OPEN_WORLDLINE_HOP_SELECTION_v1.md)  
**Run of record:** `engine/results/ftd_0489/windows_msvc_cpu.json`

## 1. Result

The FTD-0484 straight-worldline action assigns an exact gauge-covariant
coupling and current to a **fixed** charged history. It does not define a
gauge-invariant cost ordering among histories with different charged
endpoints.

For candidate endpoint `d`, the exact transformation is

```text
S_d' = S_d + g(<rho_d,chi_1>-<rho_0,chi_0>).
```

For candidates `d,e` sharing their start,

```text
(S_d-S_e)' = (S_d-S_e) + g<rho_d-rho_e,chi_1>.
```

The endpoint gauge `chi_1` is arbitrary. Whenever `rho_d!=rho_e`, the last
term can be made positive or negative with arbitrarily large magnitude while
the electric and magnetic fields are unchanged. An `argmin`, `argmax`, sign,
or threshold rule applied to the open interaction action is therefore gauge
dependent.

The locked equal-cost `+x/+y` fixture gives

```text
Delta S in gauge + = +2.92
Delta S in gauge - = -2.92
```

with exactly zero electric, magnetic, and endpoint-identity residual. Adding
unequal gauge-invariant matter costs does not fix it:

```text
Delta S_total in gauge + = +5.04
Delta S_total in gauge - = -6.64.
```

This is not numerical instability. It is the boundary term required by gauge
covariance of an open charged worldline.

## 2. Cubic no-selector result

The 26 nonzero Moore displacements split into exact cubic orbits of `6` face,
`12` edge, and `8` corner vectors. A fully symmetric rest/zero-field input is
fixed by every coordinate reflection. An `O_h`-equivariant deterministic
selector must therefore return a displacement fixed by all three reflections.
Only the zero vector has that property; none of the 26 nonzero hops does.

Consequently a nonzero hop from the fully symmetric state requires a
symmetry-breaking input or an additional branch/selection rule. Choosing one
member of a degenerate orbit is not cubic covariant.

## 3. What remains constructive

The existing physical `velocity` and continuous `remainder` already provide a
cubic-covariant kinematic endpoint map: component thresholding commutes with
proper cubic axis permutations. Conditional on that externally supplied
endpoint, the FTD-0484 straight segment uniquely deposits the exact current
without choosing an x/y/z face ordering.

This narrows the meaning of the positive worldline result:

1. it is a valid fixed-history coupling and current generator;
2. it may be used after a history has been selected by kinematics;
3. its open action value cannot be minimized across different endpoints;
4. it has not generated the velocity change or hop selection required for
   reciprocal mobile matter.

## 4. Correct completion boundary

A classical variational completion would need gauge-covariant endpoint
momentum data and a declared solution concept for the FTD-0487 nonsmooth
threshold (`one-sided`, subgradient, or another explicitly selected rule). A
quantum completion would instead compare amplitudes carrying transforming
endpoint matter phases, not bare open-path action values. Neither structure is
present in the frozen mobile-matter candidate.

Same-endpoint path differences are not covered by this no-go: their endpoint
gauge terms cancel, so a closed-loop/curvature comparison can be gauge
invariant. FTD-0489 closes endpoint selection by bare open action, not every
possible route-selection mechanism.

No production toggle, scenario, force, or IR claim is licensed.

## 5. Reproducibility

- checks: `10/10 PASS`;
- worst endpoint identity residual: `0`;
- worst field-invariance residual: `0`;
- translation residual: `0`;
- test SHA256:
  `15A91695BE3785E7DD8B85BE06442C0CDBEC5DCE3F4C340230410F2C3EF5C53A`;
- header SHA256:
  `D73C7AC6408565A4230580617FD1764FF3D124900E7C9AE4CC677A88814F9B3A`;
- implementation SHA256:
  `C5AD99C0930887B21995C30DD9E4DA18ECD53FEDCEEA731BE8A45348DD7A5963`;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU observer;
- production state: unchanged.
