# Four physical ticks for the finite causal candidate

Date: 2026-09-05. Status: **[IMPLEMENTATION CANDIDATE]**. Law identifier:
`phi-v2-staged-candidate-1`. This prototype does not amend the v3 constitution,
adopt a new canonical law, or recover material or continuum physics.

## Why a distinct candidate is required

The selected Python Phi-v2 implementation composes neighbor-dependent admission,
local collision, and streaming inside one stated global tick. On an otherwise
blank periodic L=7 lattice, place channels {0,34} at (3,3,3). Adding channel 2
at (4,3,3) changes the outgoing bank at (2,3,3) from {113} to {115} after one
baseline tick. The frozen collision maps (0,34) to (2,32); the added channel
blocks absorption of channel 34. This is a site-to-site distance-two dependency,
independent of how relations are assigned to storage anchors. The executable
witness is `scripts/tests/phi_v2_lattice/test_causal_witness.py`.

The candidate charges separate physical ticks for the composed operations.
Its four-tick boundary trace is compared to the baseline by tests; it does
not retain the baseline's time calibration or inherit continuum coefficients.

## Complete state and schedule

`scripts/phi_v2_lattice/staged.py` defines `StagedState(microtick, lattice,
admitted_sc, gate_sc, gate_fcc)`. The phase is `microtick % 4`. The global
ordinal clock is explicit, and its uniform four-stage schedule is a declared
choice of this candidate. It is not a hidden solver iteration. Local records
are finite; the global ordinal is not claimed to have a finite alphabet.

The current `lattice` has the original finite s, ell, bank, SC, and FCC arrays.
All token payload remains in these active arrays at every phase. Pending
controls are boolean arrays: admitted_sc and gate_sc have shape (L^3,3);
gate_fcc has shape (L^3,3,2). No prior token bank, arrival identity, inverse
tape, event dictionary, or random generator is part of the complete state.

| Input phase | One physical tick performs | Output phase |
|---|---|---|
| 0 | Read original endpoint occupation parity into gates; resolve unique SC admission; remove admitted bank bits and write rotated reserve tokens | 1 |
| 1 | Collide each remaining site bank using ell; cross/rotate each relation using its saved gate, skipping absorbed SC records; advance ell | 2 |
| 2 | Apply Hodge tick and the old manifestation half-turn, then stream one SC hop | 3 |
| 3 | Recompute s from current relation incidence; clear pending controls | 0 |

Each step copies input arrays and returns a new complete state plus external
`TickEvents`. Events are emitted only at the stage that performs the action:
admissions in phase 0; collisions/crossings/holds in phase 1. Callers comparing
a cycle with baseline events concatenate the four external event lists.

The prototype supports periodic L>=3 test domains. These are declared finite
simulation preparations, not an implementation of the constitution's full
undefined-boundary extension requirement. Initialization accepts every valid
ternary s, including an independently prepared lagged readout. Subsequent
phase-0 boundaries recompute s=bal3(Q); intermediate s is deliberately retained
until the commit stage. Consumers must not assume it equals instantaneous Q
at intermediate ticks.

Validation rejects wrong shapes/dtypes, values outside the finite alphabets,
invalid dimensions/clocks, uncleared phase-0 controls, and absorbed-mask entries
without the corresponding blank primary/rotated reserve. It validates carrier
admissibility, not reachability from a historical preparation. The frozen
collision hash is checked before every collision stage.

## Structural dependency and conservation arguments

The support claim is relative to the following explicit storage convention:
site fields sit at site indices; SC controls/tokens sit at their edge owner;
FCC controls/tokens sit at their square's lower-corner owner, including the
negative diagonal whose endpoints differ from that owner. Each endpoint is
within Chebyshev distance one of the owner; each owner is within distance one
of each endpoint. Global clock, boundary choice, and law identity are held fixed
when comparing local perturbations.

* Admission of a bank channel at x reads its local channel presentation, the
  other endpoint bank, and its SC relation. All are within radius one of x.
  The corresponding relation write reads the two endpoint banks and its own
  pair, within radius one of the owner. Competing presentations target only
  that edge; dictionary iteration order is not a dynamical input.
* A saved gate reads only its two original endpoint banks, within radius one
  of its owner. Computing gates and admission from the same pre-state does
  not feed an updated neighbor bank into another operation in this tick.
* Collision, relation crossing, and ell advance in phase 1 read only records
  at their own site/owner. Saved gate and admission bits turn the previously
  nonlocal predicates into explicitly retained local inputs.
* Streaming in phase 2 reads an incoming channel and manifestation at a
  departure site one SC hop away. The channel permutation and displacement
  introduce no second admission or neighbor-dependent collision in this tick.
* Manifestation reads incident primary relations, all of whose owners lie
  within its Moore neighborhood. Clearing controls has zero support.

These read/write dependencies establish radius at most one for each accepted
local stage, so composition over m physical ticks has radius at most m under
the same periodic metric. There is no claim that sampled tests exhaust all
states. Tests independently perturb every payload/control family at central
and seam anchors, inspect every output array, and retain the original live
distance-two example with its now-explicit elapsed time. Independent audit of
this argument and storage interpretation was accepted for the finite periodic
research candidate on 2026-09-05 by the separate continuum implementation
agent. Canonical adoption remains separate. Global
input validation may reject a malformed whole state; rejection is an API
error, not an additional permitted physical transition or local signal.

The exact work counter is occupied bank bits plus nonblank SC/FCC slots.
Admission removes one bank bit and fills one blank reserve; collisions
preserve pair cardinality; relation maps preserve occupied-token cardinality;
streaming is a permutation; manifestation changes no tokens. Saved boolean
controls carry no additional token units. Work is conserved at every stage,
including intermediate states. This counter is not identified as physical
energy. Relation incidence continuity changes only during crossing; the lagged
ternary s is not a substitute for integer Q.

Expiry occurs during admission. Distinct same-polarity phase-2 channels with
the same targeted edge but different internal presentations are removed and
yield the same complete post-admission state, including gates and masks.
The exact checkpoint-equality witness tests this many-to-one map. Gates retain
endpoint occupation parity only, and the admitted flag retains successful
admission only; neither stores the expired presentation. Clearing controls
later introduces further explicit information loss. Forensic event outputs
are external observations and are not fed back into the law.

## Checkpoint and acceptance contract

`initialize(lattice)`, `validate(state)`, `step(state, tables=None)`, and
`work_units(state)` are the execution interface. `checkpoint(state)->bytes`
and `restore(bytes)->StagedState` are also re-exported from staged.py.
`checkpoint.py` uses deterministic JSON with base64 arrays, exact dimensions,
global microtick, periodic boundary identity, schema and law identifiers,
collision hash, and channel/A9 encoding hash. It saves every intermediate
record. Restoration rejects missing/extra/duplicate keys, incompatible
identities, bad lengths, noncanonical boolean bytes, and invalid carriers.
Serialization is not an authentication or arbitrary-corruption checksum.

Focused acceptance tests cover all four checkpoint phases and eight-step
replay, identical events, nonmutation, finite validation, exact stage work,
three independently seeded multi-cycle boundary comparisons, causal
interventions, and complete-state expiry convergence. Canonical adoption,
undefined-boundary execution, observer delivery latency, common matter/light
cones, and continuum convergence remain outside this prototype's certification.
Native, compiled WASM and CUDA parity are now tested separately as recorded in
the program ledger; parity does not expand the physical scope. All observations
and dashboards must preserve the
candidate label and actual physical microtick count.

## Optional CUDA research backend

`engine/strict/staged_cuda.cu` implements each physical stage as a real CUDA
kernel using separate immutable input and output buffers. It shares the native
finite table transcription, state validation, and FTDSC01 binary transport.
It never invokes the CPU evolution function. The host extracts deterministic
external event rows from downloaded before/after records, verifies carrier
validity and token counts, and commits a batch only after all its ticks succeed.
Device initialization or execution failure raises an error; there is no
reference-backend fallback. The batch guarantee concerns the state API and
computation failures; the CLI's two output files are not a filesystem transaction.

CUDA write ownership is explicit: admission has one thread per SC/FCC anchor,
and each admitted source bank bit targets exactly one SC edge; collision and
relation updates have one thread per owner; streaming scatters injectively to
a cleared destination bank; manifestation has one writer per site. Kernels
read only the preceding physical state's buffers. Copy/clear operations prepare
storage, and do not add physical updates. Every kernel completes before its
result is validated or used as the next input. No floating-point arithmetic
appears in the token law. These statements are source arguments, separate from
the sampled device tests and CUDA memory tools.

Build and execute on the required WSL2 Ubuntu-22.04 GPU environment:

```text
cmake -S engine/strict -B engine/build_strict_cuda -DFTD_STRICT_CUDA=ON \
  -DCMAKE_CUDA_COMPILER=/usr/local/cuda/bin/nvcc \
  -DCMAKE_CUDA_ARCHITECTURES=120 -DCMAKE_BUILD_TYPE=Release
cmake --build engine/build_strict_cuda --target ftd_strict_cuda_cli --parallel 24
engine/build_strict_cuda/ftd_strict_cuda_cli --device
engine/build_strict_cuda/ftd_strict_cuda_cli INPUT OUTPUT TICKS EVENTS_JSON
```

The optional target is disabled by default. Architecture 120 is the measured
RTX 5090 build selection, not a universal target requirement. On Windows,
`test_cuda_parity.py` invokes the binary through WSL2. Set
`FTD_STRICT_CUDA_REQUIRED=1` to make missing device/build a failure instead of
an optional skip; `FTD_STRICT_CUDA_CLI` may select a separately built binary.

2026-09-05 measured evidence: CUDA 13.0.88, WSL2 Ubuntu-22.04, NVIDIA GeForce
RTX 5090 (compute capability 12.0), driver 610.88. All 38 required-device tests
passed: eight preparations at all four starting phases, nine physical ticks
each, with byte-identical final complete records and per-tick events versus
Python, plus rejection/no-op checks. Preparations include dense and sparse
banks, a live L=9 preparation spanning multiple CUDA thread blocks, expiry,
the distance-two baseline witness, SC/FCC periodic seams, and the prepared R5
vacuum. The independent reviewer also accepted the explicit launch-grid bounds
check before narrowing to CUDA's launch index type. Compute Sanitizer 2025.3.1 memcheck and initcheck
each reported zero errors for eight ticks of a live sparse L=5 preparation.
These runs do not establish a general race-freedom theorem, large-domain
performance, production/browser certification, or 60 FPS rendering.
