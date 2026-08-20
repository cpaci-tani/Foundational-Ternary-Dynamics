# Vendored FreeType 2.13.3

Source: https://github.com/freetype/freetype (FTL / GPLv2, dual-licensed)
Pin: `VER-2-13-3` @ `42608f77f20749dd6ddc9e0536788eaad70ea4b5`

Why vendored: the clean-slate native UI (`engine/native`) uses RmlUi, whose
default font engine is FreeType. Vendored so the tree is offline-buildable with
no system FreeType and no transitive dependencies.

## What is here

A build subset of the upstream tag: `include/`, `src/` (minus `src/tools/`,
which are standalone build helpers not part of the library), `builds/`,
`CMakeLists.txt`, `modules.cfg`, `LICENSE.TXT`, and the two license texts under
`docs/`. No tests, no docs manuals, no `subprojects/`, no `objs/`, no `devel/`.

## How it is built

`native_freetype` — a **hand-authored** static library in
`engine/native/CMakeLists.txt`. FreeType's own CMake is *not* driven
(`add_subdirectory`) so the target keeps the repo's `native_*` naming and needs
no `find_package`. The compiled set is FreeType 2.13.3's canonical `BASE_SRCS`
(the module amalgamation files: `autofit.c`, `ftbase.c`, … `winfnt.c`) plus the
Win32 system/debug backends `builds/windows/ftsystem.c` and
`builds/windows/ftdebug.c`, all compiled with `-DFT2_BUILD_LIBRARY`.
`CMakeLists.txt` + `modules.cfg` are kept only so upstream's CMake could be
driven as a fallback; they are not used by our build.

Note: the amalgamation `.c` files `#include` sibling module sources, so the full
`src/` tree must be present even though only `BASE_SRCS` is named on the compile
line. In particular `src/gzip/ftgzip.c` `#include`s the bundled zlib `.c` files
in `src/gzip/`; those zlib files are therefore NOT compiled standalone.

## External dependencies: all OFF

Equivalent to FreeType's `FT_DISABLE_ZLIB / FT_DISABLE_BZIP2 / FT_DISABLE_PNG /
FT_DISABLE_HARFBUZZ / FT_DISABLE_BROTLI`. The shipped
`include/freetype/config/ftoption.h` leaves `FT_CONFIG_OPTION_SYSTEM_ZLIB`,
`…_USE_BZIP2`, `…_USE_PNG`, `…_USE_HARFBUZZ`, `…_USE_BROTLI` commented out, so the
hand-authored build inherits all-deps-off without editing any header. `.gz`
support uses FreeType's bundled zlib (`FT_CONFIG_OPTION_USE_ZLIB` on,
`SYSTEM_ZLIB` off) — self-contained, zero external libraries.

Do not edit these files. Hashes: `MANIFEST.sha256`
(enforced by `engine/cmake/FtdSourceLint.cmake`).
