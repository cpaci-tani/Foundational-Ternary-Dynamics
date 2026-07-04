# Engine CI Gate — local merge-gate bundle

**Audience:** anyone (human or agent) about to commit or push engine changes.
**Origin:** engine revision program ticket 0.3 (plan `i-want-you-to-clever-frost`).

## The fast gate

```
cmake --build engine/build --config Release --parallel 32
cd engine/build && ctest -L merge_gate -j 32 -C Release
```

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

## What the fast gate does NOT cover

- **GPU parity** — run `ctest -L gpu` on the WSL2 build (`engine/build_wsl`).
  Windows-native CUDA is compile-check only (see CLAUDE.md).
- **Campaign/benchmark tests** — long-running; part of the full suite only.
- **Web** — Playwright suite in `engine/web` (incl. `scenario-parity.spec.js`)
  when web/bridge/binding files change.
- **WASM** — `engine\build_wasm.bat` when anything crossing the WASM boundary
  changes.

## The full gate (pre-merge / release)

```
cd engine/build && ctest -j 32 -C Release        # 211/211 (+ later additions)
```

Known pre-existing failure: `test_helium_scale1` (FTD-0270 boundary
hypothesis; diagnosis ticket 5.4). Its status must not change character and
never excuses any other failure.

## Policy

- Golden hashes are never re-baselined to make a refactor pass; a moved hash
  means the refactor is wrong (ADR-0012).
- Labels are additive metadata; changing the merge_gate composition requires
  updating the table above in the same commit.
