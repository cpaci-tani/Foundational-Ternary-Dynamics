# Engine CI Gate — local merge-gate bundle

**Audience:** anyone (human or agent) about to commit or push engine changes.
**Origin:** engine revision program ticket 0.3 (plan `i-want-you-to-clever-frost`).

## The fast gate

```
engine\build_native.bat
cd engine/build && ctest -L merge_gate -j 32 -C Release
```

(`build_native.bat` pins MSVC 14.44 via `vcvarsall x64 -vcvars_ver=14.44` — VS 18's
default 14.51 toolset crashes CUDA 13.0's `cudafe++`; a raw `cmake --build` works
only inside that vcvars shell.)

Runs in well under 2 minutes. Composition (labeled in `engine/CMakeLists.txt`,
"merge_gate" block at the end of the file):

| CTest name | What it pins |
|---|---|
| `render_bridge_golden` | Absolute golden gate — pinned `GOLDEN_HASH` (L=17, 100 ticks, seed 42, CPU, minimal profile, ADR-0012) |
| `render_bridge_golden_default` | Default-profile golden — pinned `GOLDEN_HASH_DEFAULT` (same harness, ZERO toggle writes = shipping defaults, extended fold incl. flux_L/R + latency) |
| `knot_tracking_golden` | Knot-tracking observation-only bit-neutrality golden |
| `tick_phase_order` | Tick phase ordering constraints (PO-2/PO-3) |
| `engine_lifecycle` | RAII/clear() lifecycle contract |
| `determinism` | Byte-identical reproducibility across runs/thread counts |
| `gpu_term_contract` | Scale-0 CUDA completeness oracle — every `TOGGLE_SPECS` row classified by live implementation vs declared backend mask |
| `ui_observer_neutrality_cpu` | UI-demand neutrality: per-tick ledger freshness, CPU observer trajectory/RNG neutrality, and single-slot telemetry semantics |
| `ui_command_queue` | FIFO command queue: W4 four-command physics-parameter order plus last-write-at-last-position coalescing |
| `ui_snapshot_publisher` | Immutable snapshot publisher: retained checksum integrity under concurrent publish/acquire |
| `ui_command_boundary` | N4: real queue drain equals a direct between-tick toggle; Step does not tick inside the drain |
| `ui_journal_replay` | Journal replay reproduces TermToggles and the six knobs; `applied` is the post-clamp engine value |

## What the fast gate does NOT cover

- **GPU parity** — run `ctest -L gpu` on the WSL2 build (`engine/build_wsl`).
  Windows-native CUDA is compile-check only (see CLAUDE.md).
- **`ui_observer_neutrality_gpu` / `ui_fixed_flush_order_gpu` (N5)** — interactive CUDA characterization. Run on WSL2 / RTX 5090, never on Windows-native CUDA for multi-tick suites.
- **D3D12 debug-observability and DPI hidden-window tests** — owner-rig
  `interactive` Windows device tests (`d3d12_debug_observability`,
  `native_desktop_dpi_awareness`). Hosted CI excludes `interactive` and
  compensates the two previously unit-covered D3D checks
  (`d3d12_adapter_selection`, `d3d12_shared_buffer`).
- **Campaign/benchmark tests** — long-running; part of the full suite only.
- **Web** — Playwright suite in `engine/web` (incl. `scenario-parity.spec.js`)
  when web/bridge/binding files change.
- **WASM** — `engine\build_wasm.bat` when anything crossing the WASM boundary
  changes.

## The full gate (pre-merge / release)

```
cd engine/build && ctest -j 32 -C Release --output-on-failure
```

Known pre-existing failure: `test_helium_scale1` (FTD-0270 boundary
hypothesis; diagnosis ticket 5.4). Its status must not change character and
never excuses any other failure.

## Policy

- Golden hashes are never re-baselined to make a refactor pass; a moved hash
  means the refactor is wrong (ADR-0012).
- Labels are additive metadata; changing the merge_gate composition requires
  updating the table above in the same commit.
