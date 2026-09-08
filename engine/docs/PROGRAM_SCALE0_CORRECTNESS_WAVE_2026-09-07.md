# Scale 0 production correctness follow-up

Date: 2026-09-07. Disposition: **BOUNDED ENGINEERING REPAIRS ACCEPTED; RELEASE AND PHYSICAL GATES REMAIN OPEN**.

This wave follows `AUDIT_SCALE0_COMPREHENSIVE_2026-09-07.md` and
`AUDIT_SCALE0_PERFORMANCE_FOLLOWUP_2026-09-07.md`. Their frozen candidate
`2166608709d498f908e60042333c6cd845e27e53df176354c0af8f9d7089f11f` remains
the identity of those earlier measurements. New source changes do not inherit
those performance passes. Strict finite-law/recovery archives remain unchanged.

## Ownership and acceptance

| Requirement | Exclusive implementation owner | Required evidence | Independent reviewer | Disposition |
|---|---|---|---|---|
| Typed, bounded native JSON and command preflight | Causal agent: ws_protocol, ws_json, ws_command_validation, ws_server_commands, two new native tests | Exact integer/Unicode/malformed input tests and actual dispatch rejection with unchanged state | Runtime agent / coordinator | Accepted: Windows and WSL tests plus independent wire oracle |
| Paused Lagrangian demand publication | Runtime agent: wasm-bridge.worker and two new tests | Actual producer Node tests; actual L97 worker activation/deactivation, unchanged physical clock | Causal agent | Accepted: 27 selected Node tests and actual rebuilt L97 worker test pass |
| Producer intervals apply to started but retired observations | Coordinator: native_telemetry_scheduler and deterministic clock test | Empty cache, stale epoch, per-group deadline, demand churn, eventual refresh, source replacement | Causal agent | Accepted: deterministic injected-clock regression passes on both hosts |
| Typed telemetry demand and finite JSON serialization | Coordinator: ws_server_telemetry; runtime agent: new test | Invalid requests preserve demand; nonfinite readouts serialize as null; external JSON parser verification | Runtime agent | Accepted: native tests and independent Python JSON parser; bulk slice preserves one stream |
| Error reporting safely extracts command identity | Coordinator: ws_server_runtime | Native build and protocol integration rejection checks | Causal agent | Accepted at validation-exception scope; allocation failure during error reporting is not certified |
| Tick preflight rejects before changing production records | Physics agent: render_bridge and new test | Invalid strict/matched profiles and finite counter horizons preserve affected records; successful tick regression | Runtime agent | Accepted at named early-rejection scope; Windows and WSL regression pass |
| Build, integration and evidence | Coordinator: CMake, wave ledger and new evidence | Focused native tests, ownership checks, independent reviews, exact changed-source identity | Agents rotate across implementations | Local candidate accepted; three clean-source release assertions fail as recorded below |

These are engineering repairs to the floating-point production reference
engine. They do not certify it as the strict microscopic law or establish
continuum, particle, atomic, molecular or gravitational recovery.

## Reproducible evidence and review

The final [v2 source manifest](evidence/scale0-correctness-candidate-v2-manifest-2026-09-07.json)
freezes 2,145 files with identity
`292800fd8dc3f9971d55300eadc8feea18c887402fcd8d794d57ad26ad0c44bc`.
Its replay archive is `engine/build/scale0_audit_correctness_candidate_v2_20260907.zip`,
SHA256 `e8d0797ce62b7e845ccc928bc64ba0e003f9051d0cec2a8d847fd2a29ed8bb60`.
The first candidate and its failed Linux test evidence remain preserved. The
only v2 change is the existing Unix handshake test: its socketpair has no IP
loopback identity, so the casing fixture now supplies a trusted lowercase
Origin header and asserts that absent Origin remains rejected. The production
allowlist was not changed.

The [post-run verification](evidence/scale0-correctness-v2-verification-2026-09-07.json)
reports zero drift across all 2,145 candidate files, matching current and prior
archive hashes, and zero drift in the 69/44/73 files of the three strict
foundation/recovery manifests. `git diff --check` passed with existing line-ending
notices. No files were staged or committed.

| Gate | Observed result | Evidence |
|---|---|---|
| MSVC 14.44 Release, parallel 32 | Build passed; 10/10 focused CPU CTests passed, parallel 24 | [v2 native tests](evidence/scale0-correctness-v2-native-tests-2026-09-07.log) |
| WSL2 Ubuntu, parallel 32 | CUDA-capable build passed; 9/9 focused CPU CTests passed, parallel 24 | [v2 WSL tests](evidence/scale0-correctness-v2-wsl-tests-2026-09-07.log) |
| Independent Python JSON and actual WebSocket oracle | 18 checks each on Windows and WSL: 9 serializer outputs, 7 invalid commands, escaped profile label/exact ID, successful tick | [Windows](evidence/scale0-correctness-native-wire-2026-09-07.json), [WSL](evidence/scale0-correctness-v2-wsl-wire-2026-09-07.json) |
| Actual worker handler, paused cache, lifecycle and inspection | 27/27 Node tests passed | [Node log](evidence/scale0-correctness-frozen-node-tests-2026-09-07.log) |
| Three rebuilt WASM ABIs and L97 worker | 6/9 selected browser tests passed; 3 failed the explicit clean-source assertion because `source.dirty` is true | [Browser log](evidence/scale0-correctness-v2-browser-tests-2026-09-07.log) |
| WASM bytes and bundle identity | All 6 artifact sizes/hashes and aggregate hash passed independent verification | [Artifact hashes](evidence/scale0-correctness-v2-artifact-hashes-2026-09-07.json) |
| CPU-only, desktop-disabled configure | Configure and source-ownership gate passed: 802 test sources, 148 quarantined, 46 conditionally owned | [Configure log](evidence/scale0-correctness-lean-configure-v3-2026-09-07.log) |

The six browser passes include bitwise electric-sampler checks against the
full-EM oracle on all three ABIs, paused Lagrangian activation/deactivation at
L97, generated-loader attributes, and rejection of a one-byte mixed generation.
The three failures are release assertions requiring a clean source tree; they
are not passes and were not weakened. The independent artifact check verifies
bytes without claiming clean-source provenance. The rebuilt bundle is
`97dbc15f61ba8952e68be8012279e50bf0bc650e5806f6bb5ca705952df5d4e3`.
These functional runs do not renew the hardware FPS/callback gate.

The configure request also supplied `FTD_BUILD_EXPERIMENTAL=OFF`, but the
current CMake code does not consume that variable and emitted a warning. This
is evidence for the actual CPU-only/desktop-disabled configuration and source
ownership gate, not certification of an experimental on/off option matrix.
Two preceding configure-launch attempts failed in the wrapper before CMake;
their logs are retained separately from the successful configure.

Reviews were performed by separate agents: the runtime agent reviewed command,
telemetry and preflight changes; the causal agent reviewed the worker,
scheduler and error extraction; the coordinator reviewed integration and the
Unix fixture repair. The physics agent independently checked the claims in
these reports, without approving its own preflight implementation. The review
caught and returned the profile-label escaping defect to its author before
acceptance. These are AI agent reviews, not external human certification.

The runtime agent's final integrated review independently verified all 2,145
live source hashes, the archive, seven evidence hashes, all three strict
manifests, and the recorded test counts. It accepted this scoped disposition
while retaining the three failed clean-release assertions and the open gates.

## Native transport disposition

| Original finding | Disposition after this wave |
|---|---|
| NTR-01 / NTR-02 | Closed for bounded typed command input and safe integer conversion; see [command repair](AUDIT_SCALE0_NATIVE_COMMAND_REPAIR_2026-09-07.md) |
| NTR-06 | Closed for the audited scalar metadata, voxel, force and slice JSON serializers; nonfinite values are explicit null |
| NTR-08 | Closed for minimum intervals between successfully started producer passes within one source generation, including retired results |
| NTR-03 | Open: client retirement and network deadlines |
| NTR-04 | Open: WebSocket control-frame limits |
| NTR-05 | Open: binary sample tick/generation provenance |
| NTR-07 | Open: endpoint correlation and exact 64-bit metadata across JavaScript |

## Explicit remaining limits

- Tick preflight covers named early rejection paths. Full production Scale 0
  checkpoint/restore and arbitrary late-exception rollback remain open; see
  `AUDIT_SCALE0_REPLAY_DESIGN_2026-09-07.md` for the complete-state inventory.
- Producer intervals bound successfully started snapshots within a source
  generation. Failed begins retain existing epoch/mask retry suppression;
  replacing the source starts a new scheduling entitlement.
- Previous callback-budget failures, unmeasured performance configurations and
  interrupted long GPU parity checks remain unresolved until rerun and passed.
- No GPU execution parity campaign ran in this wave. WSL execution above forced
  CPU ownership; CUDA compilation is not GPU parity evidence.
- Finite-time preflight does not require `t + dt > t`; floating-point clock
  resolution at extremely large times remains unchanged. ID reservation,
  late strong/matched phase failures and atomic `run(N)` remain open.
- This wave does not change any physical gate, adopt a successor law, or deploy
  the public website. Prior unrelated checkout changes are preserved.
