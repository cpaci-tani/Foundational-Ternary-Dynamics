# PRE-REGISTRATION — Symmetric diagonal coupled endpoint

**Date locked:** 2026-07-25  
**Identifier:** `FTD-0531`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parents:** `FTD-0478`, `FTD-0496`, `FTD-0527`–`FTD-0530`  
**Scope:** observer-only scalar symmetry reduction for identical, equal-speed,
zero-COM edge/corner contact. No production state, default, toggle, scenario,
force, collision law, phase order, field ontology, normalization, or tolerance
change.

## 1. Registered construction

Start from the already-bounced physical representative one chart tick before
the FTD-0527 common output. Let both outgoing momenta have equal unknown
magnitude `p_1` along their existing opposite Moore normal. For each carrier,
the production dispersion fixes the complete one-tick displacement:

```text
H(p)=sqrt(E_REST^2+C_SPEED^2 p^2),
d_i(p_1)=C_SPEED^2 (p_0,i+p_1,i)/(H_0+H_1).
```

Deposit the two exact straight worldline currents to their momentum-dependent
endpoints and denote their sum by `K(p_1)`.

Use the frozen reference current `K_0=K(p_0)` to define the locked local field
challenge

```text
F_0=C C^T K_0,
E_0=K_0/2+(1/8)F_0.
```

Define one stationary compensating source once from the initial state,

```text
rho_stationary=D E_0-rho_moving,0.
```

It is held fixed throughout the solve. Exact continuity then makes
`E_1=E_0-K(p_1)` satisfy the corresponding final Gauss law for every candidate.

Solve the scalar simultaneous energy equation

```text
R(p_1)=2[H(p_1)-H(p_0)]
       + beta/2 [||E_0-K(p_1)||^2-||E_0||^2]=0,
beta=G_C^2/C_WAVE^2.
```

The field is embedded in the full staggered step with
`B_before=C_SPEED C^T E_0`, as in FTD-0529/0530.

## 2. Registered numerical uniqueness and inverse gates

Bracket on outward momenta from `p_0` through the momentum corresponding to
`0.95 C_SPEED`. Require a sign change, bisection residual below `1e-12`, and
strict increase of `R` on a preregistered 65-point uniform grid over the full
bracket. This is a measured uniqueness gate for the locked field family, not a
global contraction theorem.

At the root require:

1. exact current continuity and absolute Gauss below `1e-12`;
2. full staggered embedding and field midpoint-work identities below `1e-12`;
3. matter-energy gain equals exact current work and total energy closes below
   `1e-12`;
4. the production discrete-gradient displacement identity below `1e-12` and
   speed strictly below `C_SPEED`;
5. `p_1>p_0+1e-8`, proving the field changes the endpoint rather than merely
   relabelling the frozen FTD-0527 output;
6. reversed endpoint currents restore field, density, positions, momenta, and
   energy below `1e-10`;
7. both polarities, three translations, all 20 edge/corner directions, and
   speeds `1/8` and `1/4` (`240` arms);
8. translation, polarity-mirror magnitude, and signed-cubic orbit residuals
   below `1e-12`;
9. invalid inputs fail closed.

## 3. Locked verdicts

- If all gates pass:
  `SYMMETRIC_DIAGONAL_ENERGY_COUPLED_ENDPOINT_CONSTRUCTIVE`.
- If no causal root exists or Gauss/energy/inversion fails:
  `SYMMETRIC_DIAGONAL_COUPLED_ENDPOINT_CLOSED_NEGATIVE`.
- If the root exists but the locked monotonicity gate fails:
  `DIAGONAL_COUPLED_ENDPOINT_EXISTS_UNIQUENESS_UNRESOLVED`.

The constructive verdict would prove that existing relativistic momentum has
enough capacity to absorb the FTD-0529 field work while exact current and
endpoints move self-consistently. It would not derive a three-dimensional force
from the common action, cover arbitrary fields, choose general scattering, or
license production. Those remain separate gates.

## 4. Execution record

Executed 2026-07-25 with pinned MSVC `14.44.35207`, Release, CPU observer.
The locked preregistration SHA256 before this execution annotation was
`BC1A8905A01759D0BFFF6D9371E7F4CE77108FCDCF57D766B1752A74A307DC2F`.

All `6/6` checks passed over 240 arms. Every scalar residual was strictly
increasing on the locked grid, and bisection converged in at most 38 iterations.
Exact field work increased outgoing momentum and moved the endpoint while
continuity, Gauss, total energy, causality, cubic covariance, and reversal
closed. The locked constructive verdict applies:

```text
SYMMETRIC_DIAGONAL_ENERGY_COUPLED_ENDPOINT_CONSTRUCTIVE
```

Canonical result:
[`AUDIT_SYMMETRIC_DIAGONAL_COUPLED_ENDPOINT.md`](../../../07_assessment/common_action_mechanics_reciprocity/AUDIT_SYMMETRIC_DIAGONAL_COUPLED_ENDPOINT.md).
