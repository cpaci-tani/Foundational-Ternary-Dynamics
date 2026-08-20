# Vendored Dear ImGui (docking)

Source: https://github.com/ocornut/imgui (MIT)
Pin: v1.92.9b-docking @ b48d1afbe8ee8b238e2961dc363a949dd7304e23
Minimum: 1.91.5 docking (descriptor alloc/free callbacks)

Why vendored: native desktop UI v2 (SPEC_UI_V2 §10). The tree is a lean
subset of the upstream tag (no examples/). Do not edit these files.

`imconfig.h` is upstream and unmodified. Compile with
`IMGUI_USER_CONFIG="native_desktop/ftd_imconfig.h"`.

Compiled into `ftd_imgui` (Phase 1a): imgui.cpp, imgui_draw.cpp,
imgui_tables.cpp, imgui_widgets.cpp.
Vendored for Phase 1b, not compiled yet: backends/imgui_impl_win32.*,
backends/imgui_impl_dx12.*.
Tooling: misc/fonts/binary_to_compressed_c.cpp (Inter embed).

Hashes: MANIFEST.sha256 (enforced by FtdSourceLint.cmake).
