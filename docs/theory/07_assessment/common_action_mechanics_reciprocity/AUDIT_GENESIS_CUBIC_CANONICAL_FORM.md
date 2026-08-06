# Audit — Cubic canonical form and genesis bath price (FTD-0573)

**Status:** `[PROVED-SCOPED — CONSTANT ONSITE EQUIVALENT-VECTOR CUBIC CLASS]` +
`[CLOSED — BRANCHWISE DEFECT MINIMA]` +
`[RESOLVED BY FTD-0574 — NATIVE FREE-FIELD ACTION]` +
`[OPEN — GENESIS BATH ACTION/TRANSPORT]`
**Verdict:** `CUBIC_COVARIANCE_SELECTS_STANDARD_PAIRING_AND_PRICES_ONE_BATH_PAIR`
**Date:** 2026-07-26
**Production changes:** none.

## Result

| Gate | Result |
|---|---:|
| full signed-permutation group | `48/48` |
| proper cubic subgroup | `24/24` |
| invariant constraint rank / nullity | `14 / 1` |
| maximum cubic invariance residual | `0.0` |
| registered production arms | `120/120` |
| zero-drain alternative arms | `30/30`, rank `2` |
| generic positive-drain alternatives | `90/90`, rank `4` |
| `a=t` degeneracy controls | `30/30`, rank `6` minimum |
| symmetry-price arms | `120/120`, rank `2` |
| symmetry price | `1` canonical bath pair |
| determinant-formula residual, C++ | `1.1102230246251565e-16` |
| determinant-formula residual, exact proof | `0.0` |
| minimum registered alternative determinant | `0.0001234567901234568` |

## Epistemic consequence

FTD-0572's standard `(J,W)` form is unique up to scale once three explicit
conditions are imposed: the form is constant, it is onsite, and `J` and `W`
carry equivalent cubic vector representations. This removes arbitrariness
only within that class. FTD-0574 subsequently establishes directly from the
frozen source-free field tick that `W` is the discrete Legendre partner of
`J`. It does not make the production genesis event canonical or generate it
from a closed-system action.

The unconstrained comparison is exact. A branch-dependent form can lower the
zero-drain defect rank from four to two and the generic positive-drain rank
from six to four. At `a=t`, the fivefold contracting eigenspace forces rank
six for every nondegenerate form. Since the registered production grid avoids
that degeneracy, cubic covariance costs precisely one additional canonical
bath pair in all 120 arms.

The lower-rank forms are not a loophole that supplies a cheaper native model.
They change with the selected radial direction and, generically, with the
genesis parameters. Treating them as one global phase-space structure would
introduce the very branch information the comparison is meant to expose.

## Scope hazards

- A polar/axial assignment under improper cubic operations is not the
  equivalent-vector representation used by the theorem.
- Derivative-dependent, nonlocal, state-dependent, nonlinear, or
  presymplectic forms are not classified.
- Cubic covariance selects a form but supplies no Hamiltonian or energy.
- The bath count prices a reversible dilation; it does not identify physical
  bath degrees of freedom or a reset/transport mechanism.

## Provenance

Pre-execution preregistration SHA256:

```text
0EABA25DFCE05351FE361AE69920AAA3CD37F79B18A4C2028BB1BCEC7DDE3438
```

Implementation hashes:

```text
header             7C3ECEECCC020453E68FB0A0FE2F7F73CDC1D145EC12C24277E50E44B81EAB54
source             BB11E6F34BC56A615EDAC1DA84E08A27998421AF6ABCCF4DA1DFD98F82BC5A9B
test               9C1F5B68F17F1B742E318B82D62446B4B5A8C0093772BB89E32A4587F37681D1
independent proof  BC419EE1C22A5C8BF17E0FBDBA4C56396F59F9FD01A3FF286B19D0790E91AB69
```

Artifacts:

- `engine/include/ftd/eft/genesis_cubic_canonical_form.h`
- `engine/src/eft/genesis_cubic_canonical_form.cpp`
- `engine/tests/test_genesis_cubic_canonical_form.cpp`
- `scripts/proofs/proof_genesis_cubic_canonical_form.py`
- `engine/results/ftd_0573/windows_msvc_cpu.json`
- `docs/theory/10_eft_program/preregistrations/PREREG_GENESIS_CUBIC_CANONICAL_FORM_v1.md`
- `docs/theory/10_eft_program/derivations/THEOREM_GENESIS_CUBIC_CANONICAL_FORM.md`

## Validation

- independent exact proof: pass;
- focused native test: `1/1` pass;
- focused genesis/action chain: `8/8` pass;
- production golden suite after the full build: `7/7` pass;
- selected-extension suite: `81/81` pass in `355.44 s`;
- canonical MSVC 14.44 Release build: pass (`1,177/1,177` actions).
