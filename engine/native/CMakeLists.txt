# engine/native — clean-slate native rebuild.
#
# The UI is a custom Direct3D 12 + Direct2D/DirectWrite layer built in the
# engine's own device (owner decision 2026-08-20 — no Dear ImGui). This tree
# therefore carries only the ImGui-free salvage spine: the D3D12 presenter +
# CUDA<->D3D12 interop, the transport (command queue / snapshot publisher /
# journal), and the engine session. The shell/panels/theme are rebuilt on the
# custom UI in a later phase. Distinct native_* target names let this tree
# coexist with native_desktop until that tree is retired.
#
# See native/docs/PLAN_NATIVE_REBUILD.md + SPEC_NATIVE_REBUILD_R0R1.md.

# ── native_cli: command-line options ──
add_library(native_cli STATIC src/cli_options.cpp)
target_include_directories(native_cli PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(native_cli PUBLIC ftd_core)

# ── native_model: transport spine (command queue, snapshot publisher) ──
add_library(native_model STATIC
    src/command_queue.cpp
    src/snapshot_publisher.cpp
)
target_include_directories(native_model PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(native_model PUBLIC ftd_core)

# ── native_session: engine ownership + command applier + journal + snapshot builder ──
add_library(native_session STATIC
    src/engine_session.cpp
    src/command_applier.cpp
    src/parameter_journal.cpp
    src/ui_snapshot_builder.cpp
)
target_include_directories(native_session PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(native_session PUBLIC ftd_core native_model)
if(FTD_ENABLE_CUDA)
    target_link_libraries(native_session PUBLIC ftd_cuda)
endif()

if(WIN32)
    if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
        message(FATAL_ERROR
            "native D3D12/CUDA interop is Win64-only (got pointer size ${CMAKE_SIZEOF_VOID_P})")
    endif()

    # ── Vendored UI toolkit: FreeType + RmlUi (RML/RCSS) ──────────────────────
    # Hand-authored static libs over engine/thirdparty/{freetype,rmlui}, mirroring
    # the vendored-ftd_imgui pattern in native_desktop/CMakeLists.txt. Distinct
    # native_* names let this tree coexist with native_desktop. No RmlUi backend
    # is built here — the UI is rendered later through our own D3D12
    # Rml::RenderInterface, so RmlUi Core (renderer-agnostic) is all we need.
    # See engine/thirdparty/{freetype,rmlui}/VENDOR.md.

    # native_freetype — FreeType 2.13.3 with ALL external deps OFF. The shipped
    # ftoption.h leaves SYSTEM_ZLIB / USE_BZIP2 / USE_PNG / USE_HARFBUZZ /
    # USE_BROTLI commented out, so this build has zero transitive dependencies
    # (gzip uses FreeType's bundled zlib in src/gzip/). Sources are FreeType's
    # canonical BASE_SRCS amalgamation + the Win32 system/debug backends. The
    # amalgamation .c files #include sibling module sources, so the whole
    # vendored src/ tree must be present even though only these are named.
    #
    # The engine project() declares only CXX/CUDA; FreeType is C, so enable it
    # here (idempotent, MSVC cl serves both C and CXX).
    enable_language(C)
    set(FT_DIR "${CMAKE_SOURCE_DIR}/thirdparty/freetype")
    add_library(native_freetype STATIC
        ${FT_DIR}/src/autofit/autofit.c
        ${FT_DIR}/src/base/ftbase.c
        ${FT_DIR}/src/base/ftbbox.c
        ${FT_DIR}/src/base/ftbdf.c
        ${FT_DIR}/src/base/ftbitmap.c
        ${FT_DIR}/src/base/ftcid.c
        ${FT_DIR}/src/base/ftfstype.c
        ${FT_DIR}/src/base/ftgasp.c
        ${FT_DIR}/src/base/ftglyph.c
        ${FT_DIR}/src/base/ftgxval.c
        ${FT_DIR}/src/base/ftinit.c
        ${FT_DIR}/src/base/ftmm.c
        ${FT_DIR}/src/base/ftotval.c
        ${FT_DIR}/src/base/ftpatent.c
        ${FT_DIR}/src/base/ftpfr.c
        ${FT_DIR}/src/base/ftstroke.c
        ${FT_DIR}/src/base/ftsynth.c
        ${FT_DIR}/src/base/fttype1.c
        ${FT_DIR}/src/base/ftwinfnt.c
        ${FT_DIR}/src/bdf/bdf.c
        ${FT_DIR}/src/bzip2/ftbzip2.c
        ${FT_DIR}/src/cache/ftcache.c
        ${FT_DIR}/src/cff/cff.c
        ${FT_DIR}/src/cid/type1cid.c
        ${FT_DIR}/src/gzip/ftgzip.c
        ${FT_DIR}/src/lzw/ftlzw.c
        ${FT_DIR}/src/pcf/pcf.c
        ${FT_DIR}/src/pfr/pfr.c
        ${FT_DIR}/src/psaux/psaux.c
        ${FT_DIR}/src/pshinter/pshinter.c
        ${FT_DIR}/src/psnames/psnames.c
        ${FT_DIR}/src/raster/raster.c
        ${FT_DIR}/src/sdf/sdf.c
        ${FT_DIR}/src/sfnt/sfnt.c
        ${FT_DIR}/src/smooth/smooth.c
        ${FT_DIR}/src/svg/svg.c
        ${FT_DIR}/src/truetype/truetype.c
        ${FT_DIR}/src/type1/type1.c
        ${FT_DIR}/src/type42/type42.c
        ${FT_DIR}/src/winfonts/winfnt.c
        ${FT_DIR}/builds/windows/ftsystem.c
        ${FT_DIR}/builds/windows/ftdebug.c
    )
    target_include_directories(native_freetype PUBLIC ${FT_DIR}/include)
    target_compile_definitions(native_freetype PRIVATE FT2_BUILD_LIBRARY)
    if(MSVC)
        target_compile_definitions(native_freetype PRIVATE _CRT_SECURE_NO_WARNINGS)
        target_compile_options(native_freetype PRIVATE /W3)
    endif()

    # native_rmlui — RmlUi 6.2 Core (renderer-agnostic). Default FreeType font
    # engine on; static-lib export macro on; no backend / plugins / PCH.
    set(RMLUI_DIR "${CMAKE_SOURCE_DIR}/thirdparty/rmlui")
    file(GLOB RMLUI_CORE_SRCS CONFIGURE_DEPENDS
        ${RMLUI_DIR}/Source/Core/*.cpp
        ${RMLUI_DIR}/Source/Core/Elements/*.cpp
        ${RMLUI_DIR}/Source/Core/Layout/*.cpp
        ${RMLUI_DIR}/Source/Core/FontEngineDefault/*.cpp
    )
    add_library(native_rmlui STATIC ${RMLUI_CORE_SRCS})
    target_include_directories(native_rmlui PUBLIC ${RMLUI_DIR}/Include)
    target_compile_definitions(native_rmlui
        PUBLIC  RMLUI_STATIC_LIB
        PRIVATE RMLUI_FONT_ENGINE_FREETYPE RMLUI_VERSION="6.2"
    )
    target_compile_features(native_rmlui PUBLIC cxx_std_17)
    target_link_libraries(native_rmlui PRIVATE native_freetype)
    if(MSVC)
        target_compile_definitions(native_rmlui PRIVATE _CRT_SECURE_NO_WARNINGS)
        target_compile_options(native_rmlui PRIVATE /W3)
    endif()

    # ── native_windowing: DPI ──
    add_library(native_windowing STATIC src/dpi_support.cpp)
    target_include_directories(native_windowing PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

    # ── Device / interop test executables ──
    # Each compiles the presenter (ImGui-free — takes an opaque OverlayRecorder)
    # plus the self-contained test_telemetry.cpp TU directly (ftd_test_support is
    # defined in the parent AFTER this add_subdirectory runs). CTest add_test
    # registration lives in the parent engine/CMakeLists.txt.
    set(NATIVE_TT "${CMAKE_SOURCE_DIR}/tests/support/test_telemetry.cpp")

    foreach(t IN ITEMS
        test_d3d12_adapter_selection
        test_d3d12_shared_buffer
        test_d3d12_render_frame_fencing
        test_d3d12_debug_observability
        test_d3d12_overlay_once
        test_d3d12_capture_lifecycle
    )
        add_executable(native_${t} tests/${t}.cpp src/d3d12_presenter.cpp ${NATIVE_TT})
        target_include_directories(native_${t} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
        target_link_libraries(native_${t} PRIVATE native_session ftd_core d3d12 dxgi d3dcompiler)
    endforeach()

    # DPI awareness (windowing only).
    add_executable(native_test_dpi_awareness
        tests/test_native_desktop_dpi_awareness.cpp ${NATIVE_TT})
    target_include_directories(native_test_dpi_awareness PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_link_libraries(native_test_dpi_awareness PRIVATE native_windowing ftd_core)

    # CUDA<->D3D12 interop device tests.
    if(FTD_ENABLE_CUDA)
        foreach(t IN ITEMS
            test_cuda_d3d12_adapter_match
            test_cuda_import_shared_buffer
            test_interop_gather
            test_interop_fence_roundtrip
            test_interop_reload_reset
            test_interop_visual_parity
        )
            add_executable(native_${t} tests/${t}.cpp src/d3d12_presenter.cpp ${NATIVE_TT})
            target_include_directories(native_${t} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
            target_link_libraries(native_${t} PRIVATE native_session ftd_cuda d3d12 dxgi d3dcompiler)
        endforeach()
    endif()

    # ── native_ui_rml: RmlUi (RML/RCSS) rendered through our own D3D12 ──────────
    # M-UI-1 (native/docs/SPEC_NATIVE_UI_RMLUI.md). RmlD3D12Renderer implements
    # Rml::RenderInterface + a small Rml::SystemInterface on the engine's D3D12
    # device: compiled geometry -> immutable VB/IB, font-atlas/image textures ->
    # SRVs, scissor, one UI PSO (ortho VS + textured x vertex-colour PS from
    # shaders/rmlui.hlsl, compiled at runtime via d3dcompiler). Links native_rmlui
    # (which brings native_freetype). windowscodecs is WIC (LoadTexture decode).
    add_library(native_ui_rml STATIC src/ui/rml_d3d12_renderer.cpp)
    target_include_directories(native_ui_rml PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
    target_link_libraries(native_ui_rml
        PUBLIC  native_rmlui
        PRIVATE d3d12 dxgi d3dcompiler windowscodecs)
    if(MSVC)
        target_compile_definitions(native_ui_rml PRIVATE _CRT_SECURE_NO_WARNINGS)
    endif()

    # ── native_test_ui_rml_smoke: headless RML render -> PNG ────────────────────
    # Renders shell.rml + ftd.rcss into an offscreen 1280x800 RGBA8 target, reads
    # it back, asserts a meaningful fraction of pixels differ from the shell bg,
    # and writes a PNG. Asset + output paths are baked in as compile definitions.
    add_executable(native_test_ui_rml_smoke tests/test_ui_rml_smoke.cpp)
    target_link_libraries(native_test_ui_rml_smoke
        PRIVATE native_ui_rml native_rmlui d3d12 dxgi d3dcompiler windowscodecs)
    target_compile_definitions(native_test_ui_rml_smoke PRIVATE
        FTD_RML_SHELL_PATH="${CMAKE_CURRENT_SOURCE_DIR}/ui/rml/shell.rml"
        FTD_RML_FONT_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/Inter-Regular.ttf"
        FTD_RMLUI_HLSL_PATH="${CMAKE_CURRENT_SOURCE_DIR}/shaders/rmlui.hlsl"
        FTD_RML_PNG_OUT="C:/Users/cpaci/AppData/Local/Temp/claude/C--Users-cpaci-Desktop-ftd-engine/3ccfaca6-eaf5-4dbc-bd71-f3eded0ce5c6/scratchpad/rml_smoke.png")
    if(MSVC)
        target_compile_definitions(native_test_ui_rml_smoke PRIVATE _CRT_SECURE_NO_WARNINGS)
    endif()

    # ── native_app: the live windowed application ───────────────────────────────
    # A real Win32 window rendering the RmlUi shell (shell.rml + ftd.rcss) OVER
    # the live D3D12 lattice/particle/flux scene, driven by NativeEngineSession on
    # a sim thread (M-UI-1..M-UI-3 fused; native/docs/SPEC_NATIVE_UI_RMLUI.md §3).
    # Compiles the presenter TU directly (as the device tests + native_desktop do
    # — there is no standalone presenter library). Links the RmlUi renderer
    # (native_ui_rml → native_rmlui → native_freetype), the session/transport
    # spine, the CLI, and DPI awareness. windowscodecs = WIC (the --capture-frames
    # PNG writer); shell32 = CommandLineToArgvW.
    add_executable(native_app WIN32 src/app/main.cpp src/d3d12_presenter.cpp)
    target_include_directories(native_app PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src)
    target_link_libraries(native_app PRIVATE
        native_session
        native_windowing
        native_cli
        native_ui_rml
        native_rmlui
        d3d12 dxgi d3dcompiler windowscodecs shell32)
    target_compile_definitions(native_app PRIVATE
        FTD_RML_SHELL_PATH="${CMAKE_CURRENT_SOURCE_DIR}/ui/rml/shell.rml"
        FTD_RML_FONT_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/Inter-Regular.ttf"
        FTD_RMLUI_HLSL_PATH="${CMAKE_CURRENT_SOURCE_DIR}/shaders/rmlui.hlsl"
        FTD_APP_PNG_OUT="C:/Users/cpaci/AppData/Local/Temp/claude/C--Users-cpaci-Desktop-ftd-engine/3ccfaca6-eaf5-4dbc-bd71-f3eded0ce5c6/scratchpad/live_app.png")
    if(MSVC)
        target_compile_definitions(native_app PRIVATE _CRT_SECURE_NO_WARNINGS)
    endif()
endif()
