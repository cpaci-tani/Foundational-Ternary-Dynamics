# Audit — Minimum symplectic bath for accepted genesis (FTD-0572)

**Status:** `[PROVED-SCOPED — FTD-0570 CANONICAL PAIRING] + [CONSTRUCTIVE WITH ADDED BATH PAIRS] + [CLOSED NEGATIVE — ZERO-RESET PASSIVE RESERVOIR]`
**Verdict:** `MINIMAL_FEEDBACK_DILATION_REQUIRES_RESET_OR_ACTIVE_ENERGY_RESERVOIR`
**Date:** 2026-07-26
**Production changes:** none.

## Result

| Gate | Result |
|---|---:|
| matrix arms | `120/120` pass |
| canonical-pair arms | `360/360` pass |
| defective-pair dilation arms | `330/330` pass |
| zero-drain rank-four arms | `30/30` |
| positive-drain rank-six arms | `90/90` |
| minimum bath pairs at zero drain | `2` |
| minimum bath pairs at positive drain | `3` |
| maximum pair symplectic residual | `2.220446049250313e-16` |
| maximum prepared projection residual | `0.0` |
| maximum two-step formula residual | `1.7763568394002505e-15` |
| minimum nonzero two-step deviation | `0.3583225665910466` |
| minimum registered passive commutator | `0.20370370370370372` |
| primal/dual defect-rank gap | `0` |
| independent symbolic identities | `5/5` pass |

## Epistemic consequence

FTD-0571 proved that bath feedback is necessary under the FTD-0570 standard
`(J,W)` canonical pairing. FTD-0572 now prices that selected pairing sharply.
Symplectic block identities force both the feedback rank and the
record-transfer rank to be at least the raw genesis defect rank. This requires
two canonical bath pairs even when kinetic drain is disabled, and three for
every positive drain.

That lower bound is attained by an explicit direct sum of one-pair canonical
dilations. With a zero bath, one projected step reproduces genesis exactly.
The construction is therefore mathematically positive, not merely another
no-go.

It also exposes why the construction is not a native closure. The first event
loads the bath with the system record, and the second event feeds it back with
a nonzero analytic deviation in every defective arm. A fixed zero-bath section
cannot be invariant under any symplectic dilation of a noncanonical map.
Moreover, an equal-weight positive quadratic system-plus-bath energy would
make the dilation orthogonal-symplectic and force the system block to commute
with the canonical complex structure; genesis does not.

The correct status is therefore:

- minimum local symplectic architecture: **derived and constructed**;
- native bath identification: **open**;
- repeated production behavior: **requires reset, replacement, transport, or
  explicit later feedback**;
- passive quadratic reservoir: **closed negative**;
- active/cross-coupled/nonlinear environment: **open**;
- frozen production common action: **not recovered**.

## Scope

The passive-energy result is specific to the equal-weight Euclidean quadratic
energy. It does not exclude weighted or cross-coupled quadratic forms,
nonlinear bath energy, active squeezing, a state-dependent constrained bath
graph, a separately derived nonstandard symplectic structure on `(J,W)`, or
irreversible fundamental dynamics. Those alternatives require new
physical structure and cannot be inferred from the existing spectators.

The exact count `2/3` is tied to the standard FTD-0570 pairing. At zero drain,
an alternative form pairing the four unit-eigenvalue directions internally
reduces the defect rank from four to two. This does not remove the bath:
`det M=t^2a^3<1` forbids the raw derivative from preserving any nondegenerate
symplectic form. The invariant result is therefore “enlargement required”; the
precise minimum channel count awaits a native canonical-structure derivation.

## FTD-0573 follow-up

FTD-0573 removes the arbitrary-pairing caveat only within the constant onsite
class where `J` and `W` transform as equivalent cubic vectors. In that class
the standard pairing is unique up to scale, and its `2/3` bath-pair count is
the cubic-covariant minimum. Branchwise non-cubic forms lower the count by one
pair on every registered production arm, so the difference is now an exact
symmetry price rather than an unquantified ambiguity. A native action and bath
transport remain open.

## Provenance

Pre-execution preregistration SHA256:

```text
26C87DB4BFF2800D07C687031A606728F2982933ABBAD55A73E0BF010DEB4B1C
```

Implementation hashes:

```text
header             CCD7B09967D194498B50A7AFA449E04ED21FF06746F6B3E98651A18FD4AA1B42
source             47664A3A83BDC7125DF8C5C84FB23B09EA1F159EC2F471AD368D9697BFA83223
test               203AB4ACC71B1E9DA0C872A7981A5283EB4307CFBAC82143FE4273F4D8EE9891
independent proof  2D10C82E50873065924F15D0E3B0BD6AE3D76BDFE2478F74560FFA6FB56ED3D9
```

Artifacts:

- `engine/include/ftd/eft/genesis_minimal_bath.h`
- `engine/src/eft/genesis_minimal_bath.cpp`
- `engine/tests/test_genesis_minimal_bath.cpp`
- `scripts/proofs/proof_genesis_minimal_bath.py`
- `engine/results/ftd_0572/windows_msvc_cpu.json`
- `docs/theory/10_eft_program/preregistrations/common_action_mechanics_reciprocity/PREREG_GENESIS_MINIMAL_BATH_v1.md`
- `docs/theory/10_eft_program/derivations/common_action_mechanics_reciprocity/THEOREM_GENESIS_MINIMAL_BATH.md`
