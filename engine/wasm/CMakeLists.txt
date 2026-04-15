if(EMSCRIPTEN)
    add_executable(ftd_wasm ftd_wasm.cpp)
    target_link_libraries(ftd_wasm PRIVATE ftd_core)
    set_target_properties(ftd_wasm PROPERTIES
        OUTPUT_NAME "ftd_core"
        SUFFIX ".js"
    )

    # ── PERSISTED WASM OPTIMIZATION FLAGS ─────────────────────────────
    # Without these in the source tree, a clean reconfigure loses SIMD/LTO
    # and the deployed binary regresses to scalar code.
    #
    #   -O3                 highest optimization
    #   -flto               cross-TU inlining (huge for header-heavy engine)
    #   -msimd128           enables WASM SIMD (v128 ops)
    #   -mbulk-memory       memory.copy / memory.fill instructions
    #   -mnontrapping-fptoint  faster fp→int conversions
    #   -msign-ext          sign-extension instructions
    #   -ffast-math         FP reassociation, fused muls, vectorizable reductions
    #
    # ftd_core ALSO gets -fno-rtti -fno-exceptions (no dynamic_cast or throw
    # used). ftd_wasm CANNOT have either: Embind uses both internally.
    set(_FTD_WASM_OPTS
        -O3 -flto -msimd128 -mbulk-memory -mnontrapping-fptoint -msign-ext
        -ffast-math
    )

    # ftd_core: drop exceptions only. RTTI must stay on because LTO sees
    # ftd_wasm's embind references to type_info for ftd_core classes
    # (DagEngine, ParticleEngine, AtomEngine, RenderBridge). Stripping
    # RTTI here causes "undefined typeinfo" link errors at LTO time.
    target_compile_options(ftd_core PRIVATE ${_FTD_WASM_OPTS} -fno-exceptions)

    # ftd_wasm: keep exceptions AND RTTI (Embind uses both for type dispatch)
    target_compile_options(ftd_wasm PRIVATE ${_FTD_WASM_OPTS})

    target_link_options(ftd_wasm PRIVATE
        -O3 -flto -msimd128 -ffast-math
        -lembind
        -sEXPORT_NAME=createFTDModule
        -sMODULARIZE=1
        -sALLOW_MEMORY_GROWTH=1
        -sINITIAL_MEMORY=134217728   # 128 MB — headroom for L=128 flux fields
        -sMAXIMUM_MEMORY=536870912
        -sSTACK_SIZE=1048576         # 1 MB stack (default 64 KB is risky)
        -sNO_EXIT_RUNTIME=1
        -sENVIRONMENT=web
    )
endif()
