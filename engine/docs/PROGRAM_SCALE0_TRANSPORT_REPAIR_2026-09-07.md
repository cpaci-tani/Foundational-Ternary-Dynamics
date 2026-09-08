# Scale 0 native transport repair — 2026-09-07

This wave addresses NTR-03 (stalled clients) and NTR-04 (control-frame
admission) from the [native transport audit](AUDIT_SCALE0_NATIVE_TRANSPORT_2026-09-07.md).
It follows the [correctness wave](PROGRAM_SCALE0_CORRECTNESS_WAVE_2026-09-07.md).
The user accepted deferring a complete production reference-engine replay and
rollback retrofit in favor of concrete transport, performance and strict-law
recovery work. The existing strict candidate's complete checkpoint/replay
requirements remain in force. The [production replay inventory](AUDIT_SCALE0_REPLAY_DESIGN_2026-09-07.md)
is retained for future work; this change does not make its missing state complete.

## Contract and ownership

The native server still has one engine owner and one ordered socket writer.
One absolute monotonic deadline covers each HTTP upgrade, complete input frame,
or complete output frame, including all header, mask and payload segments.
Partial progress never restarts that deadline. Defaults are five seconds;
`--client-timeout-ms N` applies a validated integer in `[100,60000]` to all three
operations. An upgraded idle connection has no outstanding frame deadline.
Polling normally waits up to eight milliseconds between readiness checks.

The scoped socket remains nonblocking until its owner closes it. While a
socket operation waits, its callback may only pump observations; it cannot
dispatch input, advance a tick, flush another frame or enter any socket helper.
Partial failed writes poison that connection against subsequent frame writes,
including recovery/error output during exception handling. A real observation
exception retains the scheduler's existing suspended/restart-required state;
a network timeout alone does not suspend the retained engine.

Completed commands remain completed when their acknowledgement is lost.
No automatic retry, rollback or extra tick occurs on reconnect. A command
behind a failed output response is not dispatched. Old client demand, cached
publications and invalidations retire on disconnect; source ownership remains.

The read parser validates the finite frame prefix before allocating the body:
known opcodes, final frames, no reserved bits, client masking, canonical length
encoding, the existing 64 KiB input cap, control payloads at most 125 bytes,
and no one-byte close body. Legal ping payloads are echoed; unsolicited pong
is inert. These restrictions follow the relevant [RFC 6455 framing rules](https://www.rfc-editor.org/rfc/rfc6455.html#section-5.5).
This server continues to reject fragmented messages. Complete HTTP upgrade
conformance, close status/reason validation, text UTF-8 conformance and extension
negotiation are outside this repair's certification boundary.

| Requirement | Exclusive implementation ownership | Verification | Disposition |
|---|---|---|---|
| Absolute operation deadlines, nonblocking I/O, failure status and poisoned partial writes | Protocol agent: `ws_protocol.h/.cpp` | Real TCP deadline, segmented input, callback/reentry and backpressure CTests on Windows and WSL | Accepted for cooperative network operations |
| Early frame prefix rejection and outgoing control limits | Protocol agent: two new tests and shared loopback fixture | Boundary matrix plus actual prefix-only reads and exact valid payloads | Accepted for the declared restricted frame policy |
| Single-owner polling, EOF consumption, disconnect cleanup and timeout option | Coordinator: `ws_server_runtime.cpp` | Independent real-server Python oracle; separate source review | Accepted for the tested CPU preparation and source contract |
| Build registration and source/evidence identity | Coordinator: CMake, Python oracle, ledger and new manifest | Both native configurations, focused regressions and post-run hashes | Local candidate accepted; clean release not certified |

The coordinator independently reviews the protocol author's changes. The
protocol author and a separate read-only reviewer inspect the coordinator's
runtime and wire oracle. Reviews are AI agent reviews, not external human
certification. No auditor approves its own implementation.

## Evidence and scope

The [frozen candidate manifest](evidence/scale0-transport-candidate-manifest-2026-09-07.json)
contains 2,149 files, identity
`c9ddcdb26612eb9ad13ab19f1d2522bcf90d5f354dfbf555f639417cc98bdf33`.
The source archive is `engine/build/scale0_transport_candidate_20260907.zip`,
SHA256 `a5200558dc8912f81d0d2585cb8e6893521b6bd016a7f2ef79da851aa3e6139b`.
Only four existing files differ from the previous correctness candidate; four
new test/oracle files are added. This snapshot was frozen before the final
builds and checks below. It is an isolated source archive of a dirty local
checkout, not a committed or deployable clean revision.

| Final gate | Result | Evidence |
|---|---|---|
| Windows MSVC 14.44 Release build, parallel 32 | Passed | [Build](evidence/scale0-transport-final-native-build-2026-09-07.log) |
| WSL2 Ubuntu CUDA-capable build, parallel 32 | Passed; executions forced CPU | [Build](evidence/scale0-transport-final-wsl-build-2026-09-07.log) |
| Focused CTests, parallel 24 | 8/8 passed on each host | [Windows](evidence/scale0-transport-final-native-tests-2026-09-07.log), [WSL](evidence/scale0-transport-final-wsl-tests-2026-09-07.log) |
| Actual-server transport oracle | 27/27 named cases passed on each host | [Windows](evidence/scale0-transport-final-native-wire-2026-09-07.json), [WSL](evidence/scale0-transport-final-wsl-wire-2026-09-07.json) |
| Prior command-boundary wire regression | 9/9 checks passed on each host | [Windows](evidence/scale0-transport-final-native-command-2026-09-07.json), [WSL](evidence/scale0-transport-final-wsl-command-2026-09-07.json) |
| Source ownership configure | 804 sources, 148 explicitly quarantined, 46 conditionally targeted | [Initial configure/build](evidence/scale0-transport-native-build-2026-09-07.log) |
| Post-run identity and preservation | Zero drift in 2,149 source files or archive contents; all three strict manifests and prior archives preserved; six WASM artifact sizes/hashes match | [Verification](evidence/scale0-transport-verification-2026-09-07.json) |

`git diff --check` passed with existing line-ending notices. The six unchanged
WASM artifacts retain bundle identity
`97dbc15f61ba8952e68be8012279e50bf0bc650e5806f6bb5ca705952df5d4e3`.
They were not rebuilt because this repair changes native transport outside the
WASM/core target. The previous three failed clean-source browser assertions
remain failed release evidence; this wave does not waive or rerun that gate.

The first Windows CTest run is retained
in [its failed log](evidence/scale0-transport-native-tests-2026-09-07.log): seven
tests passed and the deadline fixture failed to force the intended output
backpressure, then aborted after an unhandled callback exception. The separate
real-server Windows wire test did establish actual output backpressure and
passed. Winsock accepted a single 8 MiB send without waiting even with a small
configured buffer. The corrected fixture sends bounded repeated 64 KiB frames,
checks socket buffer setup, drains every completed frame and checks the failed
frame's own emitted prefix. The same partial-send poisoning assertions remain.
Both focused tests then passed five consecutive executions on Windows before
the final single run on each host. This failed run is not reclassified as a pass.

The read-only reviewer also caught two weak oracle boundaries before freeze:
slow-drip thresholds could admit per-progress deadline resets, and the first
wire script did not enable telemetry demand. The coordinator tightened those
thresholds, prepared nonzero flux, and added actual partial-read publication
and reconnect-demand checks. Both implementation reviewers accepted the revised
runtime/oracle; the coordinator accepted the corrected loopback fixture. The
final evidence supports closing NTR-03 and NTR-04 at the scope stated here.

The wire oracle owns an ephemeral forced-CPU server at L17 with a 400 ms
configured policy and a nonzero three-component flux preparation. It tests
silent/partial upgrades, partial-frame EOF, absolute slow-input retirement,
prefix-only rejection, valid controls, TCP chunking, coalesced commands,
completed-tick retention, real send backpressure with a queued tick, ordered
binary/JSON output, all four observation groups during a partial read, and
retirement of demand/cache across reconnect. Its digest is the existing scoped
readout, not a complete-state checkpoint. Timing tolerances and preparation are
specified in `engine/tools/verify_native_transport_liveness.py` before the final
frozen run. The six invalid command-line inputs constitute one aggregate named
case, not six independent server campaigns.

## Remaining limits and priorities

- Deadlines bound cooperative network waiting. They do not preempt an active
  engine command, a backend callback, or operating-system descheduling. They
  are not a universal hard five-second server response guarantee.
- With fixed source/tick/demand and the current backend metadata contract,
  pumping during one blocked operation adds at most one publication per group
  (four total), and no invalidations. This is a bound on additional objects
  during that operation, not on arbitrary scheduler history or all memory.
- No new GPU execution, WASM/dashboard lifecycle matrix, or hardware FPS
  campaign is included. Previous failed callback budgets, unmeasured overlays
  and interrupted long GPU parity checks remain open. Compilation of a
  CUDA-capable binary is not a GPU parity result.
- NTR-05 (binary sample tick/source provenance) and NTR-07 (uniform endpoint
  correlation and exact 64-bit JavaScript metadata) remain open.
- Production replay/late-exception rollback is deferred by the accepted work
  priority. Strict-runtime checkpoints remain required. Persistent matter,
  bound composites, spectra, chemistry, continuum closure and gravity recovery
  retain the dispositions in the strict program ledgers.
- No microscopic law, physical claim, successor adoption or public deployment
  is changed by this transport repair. Unrelated working-tree changes and all
  earlier evidence archives remain preserved.
