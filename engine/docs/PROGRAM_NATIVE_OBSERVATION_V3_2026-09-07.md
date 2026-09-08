# Native observation provenance and remaining recovery gates

Date: 2026-09-07. Status: **NTR-05/NTR-07 accepted at the declared engineering scope;
physical recovery and production release remain open**.
This is a local successor to the [transport repair](PROGRAM_SCALE0_TRANSPORT_REPAIR_2026-09-07.md).
It implements NTR-05 and NTR-07 under the [version 3 contract](SPEC_NATIVE_OBSERVATION_V3.md).
Production engines remain floating reference models. The strict candidate law,
complete checkpoints and earlier research evidence remain unchanged.

## Implementation and ownership

| Requirement | Implementation owner | Evidence and disposition |
|---|---|---|
| Exact sampled tick, source, publication boundary and process identity | Coordinator: native serializers, binary packers and validation | Independent review accepted; nine focused tests and 16 observation wire cases pass on each host |
| Uniform explicit request correlation, exact uint64 JS metadata and stale-result retirement | JS implementation agent: bridge, codecs, counters and telemetry | Independent adversarial re-probes accepted; 84 Node tests and 11 actual native/client cases pass |
| Cross-field provenance and canonical slice support | JS agent: overlay runtime and slice helper; coordinator: native slice grid | Missing metadata makes combined measurements unavailable; populated odd/even-grid fixtures |
| Actual native/client interoperability | Coordinator: independent Python wire oracle and real JS bridge integration | Forced-CPU Windows/WSL checks; no GPU or performance certification |
| Scientific recovery continuation | Separate recovery author and mathematical reviewer | Scoped pair-formation obstruction accepted; no recovered binding |

Writers have exclusive files and preserve unrelated work. The independent
engineering reviewer inspects native and JS implementations and reproduces
adversarial client cases; it does not implement its own fixes. The mathematical
reviewer independently checks the new scientific statement and local finite
tables. These are AI agent reviews, not external human certification.

The FTN3 envelope allocates its 88-byte prefix with the original payload, so
packing needs no second whole-frame copy. Its metadata captures the actual
reference state before sampling and checks the same owner afterward. Default
anonymous v2 frames retain their layout; legacy binary requests with explicit
IDs reject because those frames cannot carry correlation. Explicit-ID mutations,
tick/run completions and observation errors retain the ID. Anonymous mutation
bursts remain fire-and-forget and are not silently capped as visual requests.

The exact-integer boundary uses safe numbers or canonical decimal strings.
Payloads remain sampled float32 reference observations. The external process
nonce supplies a probabilistic namespace; it is not a proof of globally unique
identity. Native finite-counter exhaustion remains an explicit limitation.
No full-state replay or arbitrary late-exception rollback is claimed.

Point observations, field samples and cached publications must belong to the
expected socket, source and local generation before acceptance. Scientific
field combinations additionally require matching source, state boundary and
sample tick. Current direct/worker samples lack that complete combination
metadata, so knot contribution measurements are unavailable on those paths.
The panel explains that limitation rather than presenting missing data as zero.

## Findings retained through review

- The native midpoint calculation could overflow for INT_MAX and select the
  wrong last plane on an even grid. Canonical field-kind support and clamp-before-
  snap arithmetic repair both. Nonempty L4/L8 fixtures prevent empty comparisons
  from passing accidentally.
- Review reproduced stale inspect/force replies bypassing provenance validation,
  two omitted exact counters, an undeclared error-recovery variable, and old
  error/deferral replies retiring newer requests. Each returns to the JS owner
  with a failing adversarial witness before acceptance.
- A valid sample queued after a tick could be rejected when that tick's ACK
  advanced the client's render epoch. The fix distinguishes acknowledged tick
  advancement from local edits; the real-server regression queues tick before
  particle/volume reads and checks their actual recorded ticks.
- Earlier native failures remain recorded: a test requested the wrong metadata
  key; shared validation initially rejected version 3; a fractional-time fixture
  had not selected the reference integrator that accepts its requested dt.
  The real JS resize fixture initially bypassed the public lifecycle method.
  Those failures are not reclassified as passes.

## Frozen candidate and verification

The [candidate manifest](evidence/native-observation-v3-candidate-manifest-2026-09-07.json)
freezes 2,160 files before the final builds and execution. Its identity is
`f856651397dab813a289469dc174ebe14f75e6a985ea5c89495e35ce1698874b`.
The archive is `engine/build/native_observation_v3_candidate_20260907.zip`, SHA256
`cff9a05c32240d1feb902bfcb01cb7b21c01989f7ee550245ae44e6c3e7bd8aa`.
There are 27 changed or added files relative to the 2,149-file transport
candidate. This is an isolated archive of a dirty checkout, not a clean commit.

| Final check on frozen source | Result | Evidence |
|---|---|---|
| Windows MSVC 14.44 Release build, parallel 32 | Passed | [Build](evidence/native-observation-v3-frozen-native-build-2026-09-07.log) |
| WSL2 Ubuntu build, parallel 32 | Passed; tests forced CPU | [Build](evidence/native-observation-v3-frozen-wsl-build-2026-09-07.log) |
| Focused CTests, parallel 24 | 9/9 on each host | [Windows](evidence/native-observation-v3-frozen-native-tests-2026-09-07.log), [WSL](evidence/native-observation-v3-frozen-wsl-tests-2026-09-07.log) |
| Independent observation wire oracle | 16/16 on each host | [Windows](evidence/native-observation-v3-frozen-native-wire-2026-09-07.json), [WSL](evidence/native-observation-v3-frozen-wsl-wire-2026-09-07.json) |
| Prior transport and command regressions | 27/27 transport and 9/9 command on each host | [Windows transport](evidence/native-observation-v3-frozen-native-transport-2026-09-07.json), [command](evidence/native-observation-v3-frozen-native-command-2026-09-07.json); [WSL transport](evidence/native-observation-v3-frozen-wsl-transport-2026-09-07.json), [command](evidence/native-observation-v3-frozen-wsl-command-2026-09-07.json) |
| Actual JS modules and adversarial fixtures | 84/84 | [Node log](evidence/native-observation-v3-frozen-node-2026-09-07.log) |
| Real production JS bridge against owned native CPU process | 11/11 | [Integration](evidence/native-observation-v3-frozen-js-native-2026-09-07.json) |
| Original serial browser/cache/telemetry/contribution suite | 57 passed, 1 failed stale assertion; failure preserved | [Browser log](evidence/native-observation-v3-frozen-browser-2026-09-07.log), [failure archive record](evidence/native-observation-v3-browser-failure-2026-09-07.json) |
| Corrected v2 scenario/resize browser group | 11/11 passed | [Affected browser group](evidence/native-observation-v3-v2-browser-scenario-2026-09-07.log) |
| Final source, archive, evidence and preservation checks | Zero drift; 2,160 files and 60 evidence hashes | [Verification](evidence/native-observation-v3-verification-2026-09-07.json) |

The actual-client regression includes three repeated tick-then-particle/volume
sequences, both point endpoints, and public resize with a new source at tick
zero. It retries only declared deferred observations, never mutations. The
Python oracle independently parses wire bytes; unit fixtures also cover exact
uint64 boundary serialization through `2^64-1` without executing enormous tick
counts. No wire test supplies a full-state checkpoint or physical certificate.

The native and JS read-only reviewer reproduced every reported client blocker
after repair and accepted the scoped implementation. The final source archive,
all three strict manifests (69/44/73 files), four earlier candidate archives,
the failed browser trace and all six WASM artifact sizes/hashes are preserved.
The new three-file scientific review record also matches. Previous failures
remain available. A coordinator command also initially requested the nonexistent
`ftd_server` build target; the corrected `ws_server` build is recorded above.

The first browser run passed 57/58. Its coalescing fixture expected an
authoritative callback for generation 11 after generations 12, 13 and 14 had
already superseded it. The JS owner reproduced the actual fixture against both
the archived 2,149-file baseline bridge and the new bridge: both emit only the
current generation 14 callback. The dispatch function is unchanged after
line-ending normalization. The independent reviewer confirmed the stale
expectation and approved a stronger test: no authoritative callback after the
superseded ACK, then exactly generation 14 after its own ACK. Both required
allocation commands remain asserted. See the [fixture review](evidence/native-observation-v3-browser-fixture-review-2026-09-07.json).
The failed run and its trace/screenshot archive remain preserved.

The [final v2 candidate](evidence/native-observation-v3-candidate-v2-manifest-2026-09-07.json)
changes only that test file. It retains all 2,160 paths, identity
`a833ab9bc9c163872edd554cd349453c02a8c5d934a4d9bfbdac423c9267167b`.
Archive `engine/build/native_observation_v3_candidate_v2_20260907.zip` has SHA256
`a5eb995ba69787a1a2f445b0a3be68462e33bd56afeb79838fa855caf523b8d9`.
Every production, native, codec, Node-test and scientific source is byte-identical
to frozen v1. Their passing evidence above is retained without repeating
unaffected builds and checks. The affected scenario/resize browser group is
rerun against v2: **11/11 pass in 27.3 seconds**. The original 57/58 run is not
relabeled as a fully passing run. The [final independent evidence review](evidence/native-observation-v3-independent-review-2026-09-07.json)
records the audit's scope and preservation checks. NTR-05 and NTR-07 close
within this versioned reference-observation contract; the remaining limitations
below prevent interpreting that closure as a full production release.

## Original roadmap disposition

The [strict stack ledger](PROGRAM_STRICT_DISCRETE_STACK.md) and
[wave 2 ledger](PROGRAM_STRICT_RECOVERY_WAVE_2.md) retain the detailed frozen
evidence. This table carries their scopes forward without upgrading a gate.

| Phase | Current disposition | Required next evidence |
|---|---|---|
| 0: freeze and acceptance | Reference causal witness and continuum limitations recorded | Preserve identities and rejected claims |
| 1: finite causal law | Accepted for the declared periodic staged candidate | Undefined-boundary behavior remains open; no canonical adoption |
| 2: exact observations | Exact nested accounting and diagnostic observations accepted | Autonomous closure must be proved or explicitly bounded |
| 3: continuum | Restricted response/continuity results; general recovery open | Spatially varying correlations, leakage, relaxation and executable error budgets |
| 4: backends and owner | Research C++/WASM/WSL2-CUDA parity accepted in prior scopes | Production strict owner integration and its complete lifecycle matrix |
| 5: material/macroscopic | Prepared transport and SC remote feedback demonstrated | Formation, restoring response, spectra, chemistry, constitutive laws and causal gravity |
| 6: product and acceleration | Microscopic/block laboratory accepted in its measured scope | Shared production lineage and certified reductions; remaining hardware gates |

The new [pair-formation theorem](AUDIT_STRICT_PAIR_FORMATION_OBSTRUCTION.md)
shows that the known co-located equal-flag pair family cannot form from outside
it in the registered two-token homogeneous occupied-relation preparation.
Independent review checked all 55,008 collision rows, all 384 channels,
192 additional bounded preparations and the all-time structural argument.
The coordinator reproduced 129 tests. The [review record](evidence/strict-pair-formation-review-2026-09-07.json)
freezes the three new files without altering prior law or campaign manifests.
This rules out one proposed formation mechanism; it does not rule out all
many-body matter and does not recover atomic or planetary dynamics.

## Performance and release limits

The previous hardware campaign retains 15 failed callback rows out of 64;
all 64 passed the measured frame, ownership and visibility gates. The previous
overlay campaign retains two failed frame rows among 140 measured rows, plus
four unmeasured Damping Zones cases across backends/states. No current functional
test renews those hardware measurements.

Instrumentation clarification: the panel subscriber wrapper includes nested
owner-read time. Excluding standalone `method:ownerRead.*` rows from the callback
gate does not subtract that time from subscriber durations. The previous raw
measurements stand; describing the subscriber results as DOM/canvas-only would
misstate the measurement.

The next performance implementation requires asynchronous spectrum analysis
with unchanged numerical definitions, bounded worker plane/probe snapshots,
and profiling of remaining DOM/canvas callbacks. A new registered profile must
preserve previous artifacts and rerun affected workloads on hardware ANGLE.
The recorded direct Entropy Density and worker Dual Amplitude frame gaps still
need attribution; the worker record also retains an ancillary connection error.
Read-only source review separately found that the invalid dual-amplitude maximum
branch in `field-quantum-renderer.js` clears `_chiralityField` instead of its own
`_dualFluxVolume`. That is a concrete pending repair, not an explanation of the
recorded long frame gap. Existing clean-source
browser release failures and incomplete GPU campaign gates remain open.

Full production replay remains deferred by the accepted priority. Strict
checkpoints remain mandatory. No public deployment, commit, staged changes,
canonical law adoption or all-scales recovery certification is part of this wave.
