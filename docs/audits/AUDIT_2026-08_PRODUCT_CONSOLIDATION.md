# FTD Product Consolidation — 2026-08-28

**Scope:** repository structure and executable product surfaces only. No
physics claims, constants, epistemic tags, or numerical results were changed.

## Retained product boundary

| Surface | Canonical paths | Role |
|---|---|---|
| Native desktop | `engine/native/` | Win64 RmlUi + D3D12 application with in-process CPU/CUDA physics |
| Web engine | `engine/web/`, `engine/wasm/` | Browser dashboard and its Emscripten bridge |
| Web native bridge | `engine/src/ws_server*.cpp` | Optional WebSocket transport for the retained web dashboard |

The shared engine libraries, CUDA backend, C++/web tests, canonical build
wrappers, and WSL2 campaign support remain because the two retained surfaces
depend on them or use them for verification.

## Removed product branches

- WPF/WebView2 desktop shell (`engine/desktop/`) and its launch/build wrappers.
- Qt6 Test Bench GUI (`engine/tools/test_runner/`).
- Standalone `ftd_sim` CLI, CLI demos, smoke harness, CLI-only visualizers, and
  the browser button that copied a now-invalid CLI VTK command.
- Tritium header-only ternary-compute library and its C++ test. This does not
  remove the unrelated nuclear-tritium web scenario or the mathematical trit
  information-theory document and verifier.
- Quarantined DagEngine, cognition sidecar, and discrete-universe CUDA
  prototype, including their opt-in CMake boundary and tests.
- Math Studio (`apps/math-studio/`) and its launcher.
- Superseded native-port plans/specifications and the unused Dear ImGui
  compressed-font artifact.
- `engine/archive/`; the deleted sources remain recoverable from repository
  history through baseline commit `21566b63`.

## Generated-tree cleanup

Kept: `engine/build/`, `engine/build_wasm/`, and `engine/build_wsl/`.

Removed as redundant/reproducible: `build_cleanup_default`,
`build_cleanup_experimental`, `build_cpu`, `build_desktop`,
`build_pages_check`, `build_wasm_mt`, and `build_wasm64`.

## Root consolidation

- Removed the now-empty `apps/` product namespace after deleting Math Studio.
- Relocated the loose engine implementation plans to
  `docs/audits/engine_agent_plans/`, preserving the original plan text and
  archive manifest.
- Relocated tracked experiment JSON from the repository root to
  `scripts/experiments/recorded_results/` and updated the producing runners.
- Removed ignored `scratch/` content and Playwright `test-results/` trees as
  reproducible local output.
- Retained `tools/`: its small runner set is cited by locked pre-registrations,
  the canonical LEDGER, and the standing pre-registration census gate, so it
  is provenance rather than random root debris.

## Navigation and verification contract

CMake, CI, engine maps, active specs, web UI/tests, ADR navigation, and the
open-item tracker were reconciled in the same cleanup. The engine file manifest
must be regenerated after the removals. Verification is limited to structural,
link, configure/build, and retained-surface tests; this cleanup performs no
numerical searches or claim promotion.
