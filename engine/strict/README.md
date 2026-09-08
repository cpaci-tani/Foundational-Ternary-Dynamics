# Strict candidate backend

This isolated C++17 library executes `phi-v2-staged-candidate-1`. Four elapsed
physical ticks are compared with one selected Python reference tick at cycle boundaries.
It does not inherit that reference's physical time calibration or wave speed.
No production engine fallback or physics adoption is included.

`State` owns the eight complete finite arrays and the global clock.
`step(const State&)` is pure; `advance(State&, uint64_t)` commits only after
the entire batch succeeds. Validation rejects malformed arrays, alphabets,
pending controls, dimensions and overflow. Events are external observations.
The library itself has no shared state owner or internal locks: callers must
serialize mutation of the same `State`; independent `State` objects are separate.

## Standalone Windows build

From the repository root, using the pinned MSVC toolset:

```text
engine\build_native.bat shell cmake -S C:/Users/cpaci/Desktop/ftd/engine/strict -B C:/Users/cpaci/Desktop/ftd/engine/build_strict_native -G Ninja
engine\build_native.bat shell cmake --build C:/Users/cpaci/Desktop/ftd/engine/build_strict_native --parallel 24
ctest --test-dir engine/build_strict_native --output-on-failure -j 24
python -m pytest scripts/tests/phi_v2_lattice/test_native_parity.py -q
```

Standalone single-configuration builds default to Release. The CLI is
`ftd_strict_cli INPUT OUTPUT TICKS EVENTS_JSON`; ticks are unsigned decimal.
Input validation and the whole batch finish before either output file opens.
I/O failures are reported, but two output files are not a transactional
filesystem commit. Set `FTD_STRICT_NATIVE_CLI` for another built binary path.
Missing native binaries explicitly skip parity tests; skips are not evidence.

## Stable binary interchange

The header is exactly 116 bytes:

| Offset | Encoding | Meaning |
|---:|---|---|
| 0 | 8 bytes | `FTDSC01` followed by NUL |
| 8 | little-endian uint32 | periodic lattice side L, at least 3 |
| 12 | little-endian uint64 | elapsed candidate physical microticks |
| 20 | 32 raw bytes | frozen collision-table SHA256 |
| 52 | 32 raw bytes | Python A9/channel encoding SHA256 |
| 84 | 32 raw bytes | SHA256 of UTF-8 law identifier |

The body is the following arrays flattened in C order: `s`, `ell`, `bank`,
`sc`, `fcc`, `admitted_sc`, `gate_sc`, `gate_fcc`. Their byte lengths are
respectively `N,N,384N,6N,12N,3N,3N,6N`, where `N=L^3`; total `416N`.
Only `s` uses signed two's-complement int8; all other bytes are unsigned finite
indices or canonical Boolean 0/1. No trailing bytes are permitted.
The transport rejects Python clocks outside uint64 instead of truncating them.
This codec is separate from the original JSON checkpoint schema and is not an
authentication or payload checksum format.

Event JSON is one object per physical tick, with `absorptions`, `collisions`,
`crossings`, and `gate_holds` in exact Python list order. Absorptions contain
`[site,channel,relation_owner,axis]`; collisions contain
`[site,polarity,[before_a,before_b],[after_a,after_b]]`; relation events contain
`[kind,owner,[axis_or_plane,optional_diagonal],direction]` and holds omit direction.

## Reproducibility and acceptance scope

`generate_tables.py` transcribes the existing hash-validated Python tables.
`python engine/strict/generate_tables.py --check` detects drift without writing.
No table is selected or numerically fitted here. The generated header freezes
all collision layers, channel maps, finite A9 readouts and identity hashes.

Initial CPU validation: **35 Python parity/codec tests passed**. These include
120 individually compared stages across L=3,4,7 and five preparation families,
plus a 12-tick batch, malformed-state rejection preserving outputs, and exact
clock boundaries. The native CTest separately verifies exception atomicity and
finite Boolean validation. This is bounded backend parity, not universal
physics recovery or a rendering/performance certificate. CUDA and WASM use
their separately audited optional build fragments and verification suites.

## Compiled WASM and local laboratory

From the repository root in PowerShell (Emscripten must be installed):

```powershell
& C:/emsdk/upstream/emscripten/emcmake.bat cmake -G Ninja -S engine/strict -B engine/build_strict_wasm -DCMAKE_BUILD_TYPE=Release
cmake --build engine/build_strict_wasm --target ftd_strict_wasm --parallel 24
python engine/strict/prepare_lab.py
python engine/strict/serve_lab.py --port 8092
```

Open [the local laboratory](http://127.0.0.1:8092/strict/web/).
The dedicated server sets JavaScript module and WASM MIME types explicitly on
Windows. It exposes the engine directory so the local compiled module,
worker source and preparations share one origin. Artifacts are generated in
ignored build directories, not copied over production WASM. The laboratory
offers L=3,4,7,9 with relation-only and sparse finite preparations. Reset creates
a fresh owner; block width only changes the exact observation. Run advances
four required physical microticks per submitted batch; Step advances one.
Leaving the page disposes the owner. A restored page requires an explicit Reset.

Rendering uses Canvas2D with exact counts from the compiled worker. It is an
external diagnostic, not an observer propagating within the lattice. There is
no autonomous block evolution, invented refinement history, calibrated physical
unit, particle catalog, matter recovery or gravitational identification.

Compiled parity (missing module otherwise explicitly skips):

```powershell
$env:FTD_STRICT_WASM_MODULE=(Resolve-Path engine/build_strict_wasm/ftd_strict_wasm.mjs).Path
python -m pytest scripts/tests/phi_v2_lattice/test_wasm_parity.py -q
```

Browser validation, from `engine/web/tests` after generating the artifacts:

```powershell
npx playwright test -c playwright.strict.config.js --grep-invert 'hardware browser matrix'
$env:FTD_HARDWARE_WEBGL='1'
npx playwright test -c playwright.strict.config.js --grep 'hardware browser matrix'
```

The isolated config starts the local server itself. Stop any manually started
server on port 8092 before invoking it. The hardware matrix records at least
600 frame intervals and 12 seconds per each of 20 size/preparation/width
combinations, verifies unchanged artifact hashes, and rejects software WebGL
provenance. Its Canvas2D frame measurements and the separate hardware WebGL
probe are labeled distinctly. Release disposition is in the integrated audit.

The parent engine can opt into this separate native library with
`FTD_BUILD_STRICT_CANDIDATE=ON`. Its default is OFF; no reference engine is
replaced. See [program ledger](../docs/PROGRAM_STRICT_DISCRETE_STACK.md),
[candidate specification](../docs/SPEC_PHI_STAGED_CANDIDATE.md), and
[continuum obstruction audit](../docs/AUDIT_STRICT_CONTINUUM_RECOVERY.md).
