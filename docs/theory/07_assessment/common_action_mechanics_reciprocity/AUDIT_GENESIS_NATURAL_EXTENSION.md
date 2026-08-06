# Audit — Genesis natural extension and symplectic boundary (FTD-0570)

**Status:** `[PROVED-SCOPED] + [CONSTRUCTIVE WITH ADDED PRIMITIVES] + [CLOSED NEGATIVE — FROZEN COMMON ACTION]`
**Verdict:** `EXACT_REAL_NATURAL_EXTENSION_ADDITIONAL_PRIMITIVES_REQUIRED`
**Date:** 2026-07-26
**Production changes:** none.

## Result

| Gate | Result |
|---|---:|
| generalized-baker arms | `48/48` pass |
| maximum baker inverse residual | `2.220446049250313e-16` |
| exact rational variable-probability depth | `100` steps |
| branchwise generating-function arms | `4,320/4,320` pass |
| accepted lift arms | `2,160` |
| maximum C++ lift inverse residual | `1.887379141862766e-15` |
| independent lift inverse residual | `1.637578961322106e-15` |
| maximum extended-energy residual | `8.881784197001252e-16` |
| minimum raw tangential symplectic defect | `0.4444444444444443` |
| maximum raw six-volume Jacobian | `0.3086419753086421` |
| binary64 64-event history pair | exact collision at `1.0` |
| projected forward/reverse log ratio | `+infinity` |

## Epistemic consequence

FTD-0569's finite-reservoir wording is now disambiguated. A fixed pair of
exact real phase coordinates can retain an unlimited symbolic branch history,
so finite dimensionality alone is not the obstruction. The obstruction for
the engine is finite information capacity.

That positive mathematical result does not repair the native action. The
accepted production drain has nonzero symplectic defect and contracts raw
`(J,W)` phase volume. A branchwise symplectic and energy-conserving lift exists
only after adding six new conjugates, an exact-real phase pair, and a
time--energy pair. In the constructive lift, the state-dependent probability
also produces a mandatory conjugate backreaction `-g_b grad p` absent from
production.

The correct status is therefore:

- exact environmental dilation: **constructed**;
- native environmental degrees of freedom: **not derived**;
- frozen production common action: **closed negative**;
- full genesis/evaporation cycle: **open-system only**.

## Provenance

Pre-execution preregistration SHA256:

```text
1C5EB97350D49AC03F63CD5BF995BDB31E9D300CFF71E4180339AE4D5CD3E0D8
```

Implementation hashes:

```text
header             07FE4D2FDA22DB221BB1F22683F402FD7E8AAA8E6B075472C9DA1CE6179D21F1
source             9572106322C83383AD087DCBD7EA5EFBBE5F5E3B10A5B49923A89BEDDEFA24BD
test               F1B7381F9A8CB7D299234E2E50BFF5DD28A61E148009656008F0F03CC093D155
independent proof  01E6208C6B8D01AFCF43DC52FE76578A26AECCABCC15881AFB15B280483B87EB
```

Artifacts:

- `engine/include/ftd/eft/genesis_natural_extension.h`
- `engine/src/eft/genesis_natural_extension.cpp`
- `engine/tests/test_genesis_natural_extension.cpp`
- `scripts/proofs/proof_genesis_natural_extension.py`
- `engine/results/ftd_0570/windows_msvc_cpu.json`
- `docs/theory/10_eft_program/preregistrations/common_action_mechanics_reciprocity/PREREG_GENESIS_NATURAL_EXTENSION_v1.md`
- `docs/theory/10_eft_program/derivations/THEOREM_GENESIS_NATURAL_EXTENSION.md`

## FTD-0571 follow-up

FTD-0571 rules out identifying the added reservoir with production fields that
remain untouched by genesis. Its block-triangular symplectic theorem proves
that a noncanonical projected event requires nonzero bath-to-system feedback;
the source audit finds no such feedback among the 34 continuous `Voxel`
spectators. The constructive lift in this audit therefore remains an enlarged
open-system model until a native feedback and reset/export channel is derived.
