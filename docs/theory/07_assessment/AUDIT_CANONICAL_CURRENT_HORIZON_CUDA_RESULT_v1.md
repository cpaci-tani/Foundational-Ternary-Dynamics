# FTD-0748 canonical net-current horizon CUDA result audit v1

**Status:** `[MIXED — FACE CONSTRUCTIVE; EDGE/BODY PREFIX DRIFT]`  
**Overall conjunction:** `[CLOSED NEGATIVE FOR THE FROZEN CUDA SUCCESSOR]`  
**Date:** 2026-07-29  
**Protocol SHA-256:**
`D01039341BCA3098C9F837549A26199CCE5BB6660C84A7C86C5037D17A2B0C46`

## Execution record

The frozen WSL2 CUDA executable was invoked exactly once for each held-out
`face`, `edge`, and `body` arm, serially and without rebuild, retuning, or
early stopping. Every arm reached tick 312 and serialized a 313-row main CSV,
a 313-row canonical-support CSV, and the two corresponding JSON summaries.
Recorded internal runtimes were 559.34 s, 639.74 s, and 446.07 s.

The independent serialized-record certificate passes `65/65` integrity and
reconstruction checks. Its successful exit certifies the frozen mixed result;
the printed H1 and three-arm failures remain failed physics/protocol gates.

After the result was frozen and certified, follow-on deterministic-deposition
development relinked the mutable build-path executable. The original ELF is
not retained and a fresh CUDA device-link is not byte-reproducible. Its actual
pre-run SHA-256 remains frozen in the pre-execution audit; the durable
certificate hashes that immutable audit, all frozen sources, the baseline, and
all twelve result records rather than pretending a reconstructed ELF is the
executed binary.

The FTD-0748 source later received a compile-harness-only `main`-macro
ownership guard so FTD-0749 could include it without a duplicate entry point.
The historical executed-source hash remains in the pre-execution audit; the
certificate hashes the guarded source and the audit. The guard does not alter
the FTD-0748 run function, gates, constants, or serialization.

## Registered verdict

| arm | corrected discrete prefix | maximum scalar prefix difference | A0, H0, H2--H5 | verdict |
|---|---:|---:|---:|---|
| face | exact | `6.657785434072e-11` at tick 72, `separation` | all pass | `CANONICAL_HORIZON_ENVIRONMENTAL_PERSISTENCE_CONSTRUCTIVE` |
| edge | exact | `2.180744473890e-10` at tick 88, `separation` | all pass | `CANONICAL_HORIZON_PREFIX_DRIFT` |
| body | exact | `1.208215749671e-10` at tick 113, `separation` | all pass | `CANONICAL_HORIZON_PREFIX_DRIFT` |

The locked scalar tolerance is `1e-10`. Edge exceeds it by a factor of 2.18
and body by a factor of 1.21. The three-ray constructive conjunction is
therefore closed negative. The tolerance is not relaxed after observation.

## What the observer correction established

The representation-independent current observer removes FTD-0747's invalid
raw-container-cardinality comparison. Every arm exactly reproduces the
FTD-0745 prefix for `valid`, `common`, `regional_valid`, canonical net
`source_radius`, and `graph_inside`. Maximum net support is 36, 36, and 54 for
face, edge, and body. Maximum aggregation-moment residuals are
`3.47e-18`, `2.10e-18`, and `1.87e-18`; maximum discarded L1 is
`3.41e-13`, `1.82e-12`, and zero, all below the locked `1e-10` gate.

Thus `sparse_current.size()` was not a physical support observable. Canonical
net oriented-face support is the valid observer for this selected dynamics.
That correction does not rescue the frozen full conjunction because the
remaining edge/body failure is in the trajectory itself.

## CUDA reproducibility exposure

A direct comparison of the frozen FTD-0747 and FTD-0748 main records finds
nonzero run-to-run differences despite unchanged physical equations. Their
largest differences are:

- face: `2.7853e-12` at tick 310 in `separation`;
- edge: `1.3945e-10` at tick 88 in `separation`;
- body: `3.5756e-11` at tick 113 in `separation`.

The CUDA current kernel applies raw sparse entries with floating-point
`atomicAdd`. The support probes prove that multiple raw entries can target the
same canonical oriented face, so the accumulated value depends on a
non-associative device addition order. The existing determinism regression is
a short one-step fixture and does not establish bitwise or `1e-10` trajectory
reproducibility over 312 coupled steps. The FTD-0748 host-side read-only
observer does not mutate physics state, but its additional work can change
launch timing. The frozen records therefore expose the collision-prone atomic
deposition path as the leading source-level explanation for the long-horizon
drift. This diagnosis is strongly localized but not yet a proof that no other
CUDA ordering or toolchain effect contributes.

The correct CUDA contract is now stricter: aggregate the complete ungated
current to unique canonical oriented faces before device deposition, then use
a collision-free deterministic update and demonstrate repeated long-horizon
record identity. Increasing H1 tolerance is not an admissible repair.

## Constructive behavior below the failed gate

All three arms pass exact execution, canonical aggregation, persistent-core,
stable-near-field, radius-48-arrival, and post-arrival persistence gates.
Radius-48 arrival remains tick 297 in every arm. Persistent negative-core
onset remains tick 80, 96, and 115. Final radius-48 outside energies are
`1.63705e-7`, `1.64394e-7`, and `1.64604e-7`.

Maximum common-action residuals stay below `5.37e-14`, energy residuals below
`5.88e-15`, recoil defects below `8.67e-15`, and regional residuals below
`5.56e-17`. These are constructive properties of the selected finite-time
dynamics, but they do not override the ordered H1 failure or establish an
asymptotic particle, Lorentz recovery, unitarity, production adoption, or a
fundamental electromagnetic ontology.

## Frozen result hashes

- face CSV:
  `78B4BD60D5F910C28A2FF42DE4D102D882BE436C50FAF7FF6F5F3510967B2F66`
- face JSON:
  `BBA7088709F4182D5D54654F875E460C7DCCB0758F8885B76CE3200B11192024`
- face support CSV:
  `65F8F3C9F6CBBAF4DF1ED43FDB8C56FAC8E16B0289579BB9167295DC20C5F0D9`
- face support JSON:
  `2D724DB2A5554C8437FBD94030360C6DBFD41C8F15CDC7CB8C1D13ED28BFD450`
- edge CSV:
  `A7EF36D0BCC16CE08966EC8C17FE605492E8E835984851B1F580B0FB4D1AB728`
- edge JSON:
  `7289D8EE93E048EEE7DBEFCE47933C50E2249CFA6064AE384AC8751C8FC1B2BE`
- edge support CSV:
  `DC564895697CA87383A65631A99A9AE87EBE4F9C4E702B046A6BA065BB4EF33C`
- edge support JSON:
  `4151EC4FCCFCD99E40E9703D9E00F66455829697B09BBE69BD47089D7BB1349D`
- body CSV:
  `CE5C0F0AAA9E2E6F7568EB3CA03A5C6EB5932C25F18C384EF20F89F47A077ADD`
- body JSON:
  `36E8C6C4D75E4A682A231DA0C9EA22D1CEDDE9550D74FEE17807E1EF8FFC93AF`
- body support CSV:
  `2BDF0989E0C955D913CFD6B8586E91C02C577F4026BD4E5532560CBB0E9110B7`
- body support JSON:
  `4E0A9126C60A38C89E391B87CD0E5614A94A62BC09D7B7415B7B7922326515DB`
