if(EMSCRIPTEN)
    add_executable(ftd_wasm ftd_wasm.cpp)
    target_link_libraries(ftd_wasm PRIVATE ftd_core)
    set_target_properties(ftd_wasm PROPERTIES
        OUTPUT_NAME "ftd_core"
        SUFFIX ".js"
    )
    # Optimization flags: -O3 + WebAssembly SIMD + link-time optimization.
    # Previous build was -O2 without SIMD/LTO — audited as 2-4x slower
    # than necessary on the wave equation tick (engine-expert audit
    # 2026-04-14). Apply to both compile and link.
    target_compile_options(ftd_wasm PRIVATE
        -O3
        -msimd128
        -flto
    )
    target_link_options(ftd_wasm PRIVATE
        -O3
        -flto
        -lembind
        -sEXPORT_NAME=createFTDModule
        -sMODULARIZE=1
        -sALLOW_MEMORY_GROWTH=1
        -sINITIAL_MEMORY=67108864
        -sMAXIMUM_MEMORY=536870912
        -sNO_EXIT_RUNTIME=1
        -sENVIRONMENT=web
    )
    # ftd_core (static library) must also be built with matching flags
    # or LTO will complain. Apply the same -O3 -msimd128 -flto.
    target_compile_options(ftd_core PRIVATE -O3 -msimd128 -flto)
endif()
