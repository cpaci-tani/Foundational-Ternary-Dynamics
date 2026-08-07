# PRE-REGISTRATION — Single-slab connection compatibility

**Date locked:** 2026-07-25  
**Identifier:** `FTD-0534`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parents:** `FTD-0484`, `FTD-0529`, `FTD-0530`, `FTD-0531`, `FTD-0533`  
**Scope:** observer-only algebraic compatibility test between the FTD-0531
staggered field embedding and one FTD-0484 gauge-potential slab. No production
state, default, toggle, scenario, force, collision law, phase order, field
ontology, normalization, or tolerance change.

## 1. Exact discriminator

Every FTD-0484 slab satisfies, independent of gauge,

```text
B_1-B_0=-lambda C^T E_slab,
```

because `E_slab=-(A_1-A_0)/lambda-G Phi`, `B_n=C^T A_n`, and
`C^T G=0`.

FTD-0531 uses the staggered magnetic embedding

```text
B_0=lambda C^T E_0,
B_1=0,
```

while its exact energy/work identity uses

```text
E_work=(E_0+E_1)/2=E_0-K/2,
E_1=E_0-K.
```

Therefore the one-slab Faraday mismatch is identically

```text
R_F=(B_1-B_0)+lambda C^T E_work
   =-(lambda/2) C^T K,
||R_F||^2=(lambda^2/4)||C^T K||^2.
```

This is not a numerical fit. Reconstruct the complete exact current `K` for
the final FTD-0531 endpoint and test the identity componentwise.

## 2. Registered arms and gates

Use all 26 signed Moore directions, speeds `1/8` and `1/4`, both polarities,
and three translations (`312` arms total).

1. On all 240 edge/corner arms, the FTD-0531 root and inherited identities
   must remain valid.
2. `||C^T K||^2` must be strictly positive on every edge/corner arm and the
   one-slab mismatch norm must exceed `1e-8`.
3. The componentwise identity
   `R_F=-(lambda/2)C^T K` and its squared-norm consequence must close below
   `1e-12`.
4. On all 72 symmetric axial arms, the aggregate current, `C^T K`, and
   one-slab mismatch must vanish below `1e-12`, reproducing FTD-0530.
5. Translation, polarity mirror, and signed-cubic orbit magnitudes must agree
   below `1e-12`.
6. Invalid sizes, polarities, speeds, and non-Moore directions fail closed.

## 3. Locked verdicts

- If diagonal mismatch is forced while the axial null control closes:
  `MIDPOINT_WORK_AND_STAGGERED_MAGNETIC_HISTORY_REQUIRE_MULTISTAGE_CONNECTION`.
- If every diagonal arm unexpectedly satisfies the one-slab identity with
  `E_slab=E_work`:
  `FTD0531_SINGLE_SLAB_CONNECTION_COMPATIBLE`.
- If the measured mismatch is not exactly the registered curl-current term:
  `SINGLE_SLAB_CONNECTION_COMPATIBILITY_UNRESOLVED`.

The first verdict would not invalidate the FTD-0531 energy ledger or FTD-0533
internal-knot variation. It would prove that they cannot be combined on the
one connection slab implied by a single-field reading. A registered multistage
or phase-space action would then be required. Such an extension may use the
already selected face electric and edge magnetic variables, but it is new
dynamics and cannot be inferred by averaging the two incompatible fields.

## 4. Execution record

Executed 2026-07-25 without changing the locked algebra or gates. All `7/7`
checks pass over 312 arms. Every diagonal mismatch is nonzero and equals the
registered curl-current term; every axial null-current control is compatible.
Locked verdict:

```text
MIDPOINT_WORK_AND_STAGGERED_MAGNETIC_HISTORY_REQUIRE_MULTISTAGE_CONNECTION
```

Canonical audit:
[`AUDIT_SINGLE_SLAB_CONNECTION_COMPATIBILITY.md`](../../../07_assessment/common_action_mechanics_reciprocity/AUDIT_SINGLE_SLAB_CONNECTION_COMPATIBILITY.md).
The SHA256 of this preregistration before this execution annotation was
`D5E822A46CEFE602113BADC6FF417E56348EF3E27856BAABC8B05BF86EE65B06`.
