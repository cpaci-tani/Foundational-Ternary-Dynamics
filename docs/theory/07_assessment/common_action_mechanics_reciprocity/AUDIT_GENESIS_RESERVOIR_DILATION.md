# Audit — Genesis reservoir dilation (FTD-0569)

**Status:** `[PROVED-SCOPED] + [CLOSED NEGATIVE — FINITE LOCAL REVERSIBLE PRODUCTION LIFT]`
**Verdict:** `ONE_EVENT_DILATION_OPEN_SYSTEM_ONLY`
**Date:** 2026-07-26
**Production changes:** none.

## Result

The registered observer separates a positive local result from the negative
cycle result:

| Gate | Result |
|---|---:|
| accepted single-genesis inverse | 540/540 arms pass |
| maximum genesis inverse residual | `1.1102230246251565e-16` |
| Bernoulli phase dilation | 16/16 arms pass |
| maximum Bernoulli inverse residual | `0` |
| 20-step history inverse | exact |
| erased 20-step preimages | `1,048,576` |
| minimum retained history | `20` bits |
| maximum withdrawal residual | `2.220446049250313e-16` |
| maximum withdrawal-slope residual | `0` |
| genesis→evaporation flux distance | exactly `kg=1` |
| maximum wave-distance residual | `1.1102230246251565e-16` |

Ordinary accepted genesis is conditionally invertible for canonical inputs and
`d<1`. The finite-cycle claim nevertheless fails because:

1. `d=1` erases the complete incoming wave state;
2. discarded Bernoulli outcomes cost one retained bit per trial;
3. evaporation does not restore the genesis field/wave drain;
4. the exact reverse event has zero probability in the frozen event kernel;
5. energy exchange varies continuously with overshoot and differs between the
   single and dual branches.

## Epistemic consequence

FTD-0567's surviving “explicit reservoir” route is narrowed, not eliminated.
A one-event probability dilation exists, but the reservoir must be an open
environmental channel or an indefinitely extensible history/energy carrier.
There is no fixed finite local sidecar that makes the existing production cycle
an autonomous reversible action.

This supports only a selected nonequilibrium-pattern interpretation. It does
not establish a physical bath, stochastic thermodynamics, unitarity of the
enlarged system, or emergent charge conservation.

## FTD-0570 follow-up

FTD-0570 constructs the exact-real exception already acknowledged by the
theorem: a fixed two-coordinate generalized baker phase can encode an
unlimited symbolic past in arbitrarily fine digits. Accordingly, “finite local
reservoir” here means a finite-information/finite-precision reservoir. It does
not mean that finite-dimensional exact-real natural extensions are impossible.

That construction does not change this audit's production verdict. Binary64
histories collide, the raw `(J,W)` genesis map is not symplectic, and the
branchwise common-action lift requires ten added continuous variables absent
from the frozen ontology.

## Provenance

Pre-execution preregistration SHA256:

```text
F0E03DBA0FCB2D757881DDF10AFC115E9A89647B056AE734CD90D07B442C0A66
```

Implementation hashes:

```text
header             377472A157BBC17C9EAE1C8A646E0B8FD06076C36F139AF2C897F3C06E1D4C67
source             DE56A0EE1E74F588B2E66AF19B82E1AB48877DC18A3BAD8A6C50CCAC6F27A176
test               3D7D43D03ACF26D2D921A7BDA049DF331F1B3E7E6DFF579A1B68B66A23F2FB58
independent proof  60AA92CEA295370D612C878A0985ED5A82B97C3C5FFE6B91D05CF420CE4B32E4
```

Artifacts:

- `engine/include/ftd/eft/genesis_reservoir_dilation.h`
- `engine/src/eft/genesis_reservoir_dilation.cpp`
- `engine/tests/test_genesis_reservoir_dilation.cpp`
- `scripts/proofs/proof_genesis_reservoir_dilation.py`
- `engine/results/ftd_0569/windows_msvc_cpu.json`
- `docs/theory/10_eft_program/preregistrations/common_action_mechanics_reciprocity/PREREG_GENESIS_RESERVOIR_DILATION_v1.md`
- `docs/theory/10_eft_program/derivations/common_action_mechanics_reciprocity/THEOREM_GENESIS_RESERVOIR_DILATION.md`
