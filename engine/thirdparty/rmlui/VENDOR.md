# Vendored RmlUi 6.2 (Core only)

Source: https://github.com/mikke89/RmlUi (MIT)
Pin: `6.2` @ `2230d1a6e8e0848ed87a5761e2a5160b2a175ba4`

Why vendored: the clean-slate native UI (`engine/native`) is built on RmlUi —
an RML/RCSS (HTML/CSS-like) retained-mode UI. RmlUi Core is renderer-agnostic;
we supply our own Direct3D 12 `Rml::RenderInterface` in a later phase, so **no
RmlUi backend is vendored**.

## What is here

Only the renderer-agnostic **Core**:

* `Include/` — public headers, trimmed to `RmlUi/Core/**`, `RmlUi/Core.h`, and
  `RmlUi/Config/**` (includes the header-only third-party containers
  `Core/Containers/robin_hood.h` + `itlib/`, used by default).
* `Source/Core/` — Core sources incl. `Elements/`, `Layout/`, and the default
  FreeType font engine `FontEngineDefault/`. Upstream's `CMakeLists.txt` files
  were removed from this subtree (we hand-author the build).
* `LICENSE.txt`.

Excluded: `Backends/`, `Samples/`, `Tests/`, `Utilities/`, `Dependencies/`, and
the `Lua` / `Lottie` / `SVG` / `Debugger` plugins (both their `Source/` and their
public headers). Core has no references to any of these.

## How it is built

`native_rmlui` — a **hand-authored** static library in
`engine/native/CMakeLists.txt`, mirroring the repo's vendored-`ftd_imgui`
pattern. It compiles every `.cpp` under `Source/Core/{,Elements,Layout,
FontEngineDefault}` (190 files), puts `Include/` on the public include path, and
links `native_freetype` (privately) so `FreeTypeInterface.cpp` finds
`<ft2build.h>`.

Required compile definitions:

| Define | Scope | Why |
|--------|-------|-----|
| `RMLUI_STATIC_LIB` | PUBLIC | `Include/RmlUi/Core/Header.h` expands `RMLUICORE_API` to `__declspec(dllexport/dllimport)` unless this is set; public because consuming headers also test it. |
| `RMLUI_FONT_ENGINE_FREETYPE` | PRIVATE | Selects + compiles the default FreeType font engine (upstream sets this when `RMLUI_FONT_ENGINE=freetype`). `Core.cpp` guards `FontEngineInterfaceDefault` on it. |
| `RMLUI_VERSION="6.2"` | PRIVATE | Otherwise `Core.cpp` falls back to reporting version `"custom"`. |

Standard: C++17 (upstream minimum C++14; the engine is C++17). Third-party
containers are left ON (upstream default) — no `RMLUI_NO_THIRDPARTY_CONTAINERS`.
No RmlUi backend, no Lua/Lottie/SVG/Debugger plugin, no precompiled header.

Do not edit these files. Hashes: `MANIFEST.sha256`
(enforced by `engine/cmake/FtdSourceLint.cmake`).
