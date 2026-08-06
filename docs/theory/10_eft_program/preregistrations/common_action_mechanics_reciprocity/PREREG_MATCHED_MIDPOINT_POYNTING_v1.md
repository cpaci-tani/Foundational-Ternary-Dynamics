# PRE-REGISTRATION — Matched midpoint Poynting identity

**Date locked:** 2026-07-26  
**Identifier:** `FTD-0544`  
**Status:** `[PRE-REGISTRATION — LOCKED/NOT YET RUN]`  
**Parents:** `FTD-0427`, `FTD-0478`, `FTD-0541`, `FTD-0542`, `FTD-0543`  
**Scope:** observer-only proof and evaluation of the normalized matched
Maxwell midpoint energy identity. It changes no field update, energy
normalization, production phase, primitive, default, toggle, or scenario.

## 1. Locked update

For arbitrary finite midpoint fields `Ebar` on faces, `Bbar` on edges, exact
coat current `K`, and duration `h>0`, define

```text
Delta E=h C Bbar-K,
Delta B=-h C^T Ebar,
E0=Ebar-Delta E/2,   E1=Ebar+Delta E/2,
B0=Bbar-Delta B/2,   B1=Bbar+Delta B/2.             (1)
```

No projection or solve is allowed. The existing `matched_curl` and its exact
transpose must be used.

## 2. Locked identities

Prove and test

```text
E1-E0=h C[(B0+B1)/2]-K,
B1-B0=-h C^T[(E0+E1)/2],                            (2)
```

and, for `U=(||E||^2+||B||^2)/2`,

```text
U1-U0=-<Ebar,K>.                                    (3)
```

Equation (3) must follow from the exact adjoint cancellation
`<Ebar,C Bbar>=<C^T Ebar,Bbar>`. Record that the magnetic exchange performs
zero net scalar work in the total field ledger; do not infer a particle force
from that cancellation.

Set `rho0=div E0`, `rho1=div E1`. Require

```text
rho1-rho0+div K=0                                   (4)
```

without Gauss projection. This isolates the field-energy side of the mobile
matter gate.

## 3. Locked arms and verdicts

Use FTD-0541 currents for both polarities, stationary, axial, two-axis,
three-axis, integer-plane, and periodic crossings on `L=17`, with deterministic
nonzero face/edge midpoint fields and `h=C_SPEED`. Require update, midpoint,
adjoint, Poynting, Gauss/continuity, polarity, and reversal residuals below
`1e-12`. Invalid current, field size, duration, and nonfinite input fail closed.

- all gates close: `MATCHED_MIDPOINT_POYNTING_EXACT`;
- update closes but energy fails: `MATCHED_FIELD_ENERGY_IDENTITY_CLOSED_NEGATIVE`;
- energy closes but Gauss transport fails: `MATCHED_POYNTING_GAUSS_INCOMPATIBLE`;
- only floating/covariance gates fail: `MATCHED_MIDPOINT_POYNTING_UNRESOLVED`.

A constructive result proves that the field sector can exchange exactly the
scalar work `<Ebar,K>`. It does not prove the action-derived particle impulse
changes production-dispersion energy by that amount, nor license dynamics.
