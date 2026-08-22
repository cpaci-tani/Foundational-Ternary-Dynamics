if(EMSCRIPTEN)
    add_executable(ftd_wasm
        ftd_wasm.cpp
        # W1-W3: Embind binding blocks split from ftd_wasm.cpp (2026-04-19)
        bindings_render_bridge.cpp    # RB toggles, injection wrappers, setup_scenario
        bindings_particle.cpp         # ParticleEngine Embind class + toggle map + pe_* helpers
        bindings_atom.cpp             # AtomEngine Embind class + toggle map + ae_* helpers
    )
    target_link_libraries(ftd_wasm PRIVATE ftd_core)

    # ── WASM32 vs WASM64 (Memory64) ───────────────────────────────────
    # Default build is wasm32 (2 GB heap). Configure with -DFTD_MEMORY64=ON
    # to produce the wasm64 variant (8 GB heap, lifts the lattice cap from
    # L~117 to L~187). The two ABIs cannot share libftd_core.a, so build_wasm.bat
    # uses separate build trees (build_wasm/ + build_wasm64/) and deploys both
    # ftd_core.{js,wasm} (wasm32) and ftd_core64.{js,wasm} (wasm64). See
    # engine/web/docs/PLAN_WASM64_UPGRADE.md.
    option(FTD_MEMORY64 "Build the WASM target with 64-bit linear memory (Memory64)" OFF)
    if(FTD_MEMORY64)
        set(_FTD_WASM_OUTPUT "ftd_core64")
        set(_FTD_WASM_EXPORT "createFTDModule64")
        set(_FTD_WASM_MAX_MEM 8589934592)   # 8 GB
    else()
        set(_FTD_WASM_OUTPUT "ftd_core")
        set(_FTD_WASM_EXPORT "createFTDModule")
        set(_FTD_WASM_MAX_MEM 2147483648)   # 2 GB
    endif()

    # ── THREADED variant (Emscripten pthreads) ────────────────────────
    # OpenMP is unavailable under Emscripten 5.0.2 (no omp.h/runtime), so the
    # tick phases parallelize via ftd::parallel_for's std::thread-pool backend
    # (engine/include/ftd/parallel.h), selected by -DFTD_WASM_THREADS. -pthread
    # makes the heap a SharedArrayBuffer (zero-copy reads from the host thread).
    # Distinct output/export so the threaded module coexists with the serial
    # fallback. Built into a separate tree with -DFTD_WASM_THREADS=ON.
    option(FTD_WASM_THREADS "Build the WASM target with the pthread-pool parallel-for backend" OFF)
    if(FTD_WASM_THREADS)
        set(_FTD_WASM_OUTPUT "${_FTD_WASM_OUTPUT}_mt")
        set(_FTD_WASM_EXPORT "${_FTD_WASM_EXPORT}MT")
    endif()

    set_target_properties(ftd_wasm PROPERTIES
        OUTPUT_NAME "${_FTD_WASM_OUTPUT}"
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
    # Memory64 must be set at BOTH compile and link time (it changes the
    # pointer ABI), so it goes into the shared compile opts applied to
    # ftd_core + ftd_wasm below, and again in target_link_options.
    if(FTD_MEMORY64)
        list(APPEND _FTD_WASM_OPTS -sMEMORY64=1)
    endif()
    # Threads: -pthread changes the memory model (shared heap) so EVERY object
    # must compile with it; -DFTD_WASM_THREADS selects parallel.h's pool backend.
    if(FTD_WASM_THREADS)
        list(APPEND _FTD_WASM_OPTS -pthread)
    endif()

    # ftd_core: drop exceptions only. RTTI must stay on because LTO sees
    # ftd_wasm's embind references to type_info for ftd_core classes
    # (DagEngine, ParticleEngine, AtomEngine, RenderBridge). Stripping
    # RTTI here causes "undefined typeinfo" link errors at LTO time.
    target_compile_options(ftd_core PRIVATE ${_FTD_WASM_OPTS} -fno-exceptions)

    # ftd_wasm: keep exceptions AND RTTI (Embind uses both for type dispatch)
    target_compile_options(ftd_wasm PRIVATE ${_FTD_WASM_OPTS})

    # FTD_WASM_THREADS selects parallel.h's pool backend. It MUST be defined for
    # EVERY TU that includes parallel.h — both ftd_core (the tick phases) AND
    # ftd_wasm (bindings_render_bridge.cpp calls ftd::set_pool_threads /
    # parallel_max_threads). Defining it only on ftd_core gave the bindings the
    # serial backend → an ODR mismatch where the embind setter/getter were
    # no-ops. PUBLIC propagates to ftd_wasm (which links ftd_core); the explicit
    # ftd_wasm definition is belt-and-suspenders.
    if(FTD_WASM_THREADS)
        target_compile_definitions(ftd_core PUBLIC  FTD_WASM_THREADS)
        target_compile_definitions(ftd_wasm PRIVATE FTD_WASM_THREADS)
    endif()

    # ftd_eft is a separate static lib that ftd_wasm also links. Under Memory64
    # every object must share the wasm64 pointer ABI, so give ftd_eft the flag
    # too (correctness only — leave its other flags untouched so the wasm32
    # build is unperturbed). Without this: "wasm32 object can't be linked in
    # wasm64 mode" at the final link.
    if(FTD_MEMORY64)
        target_compile_options(ftd_eft PRIVATE -sMEMORY64=1)
    endif()
    if(FTD_WASM_THREADS)
        target_compile_options(ftd_eft PRIVATE -pthread)
    endif()

    target_link_options(ftd_wasm PRIVATE
        -O3 -flto -msimd128 -ffast-math
        -lembind
        -sEXPORT_NAME=${_FTD_WASM_EXPORT}
        -sMODULARIZE=1
        -sALLOW_MEMORY_GROWTH=1
        -sASSERTIONS=1
        # Lattice memory budget (Voxel = 232 bytes after padding):
        #   L=32 → 12 MB total bridge  | L=64 → 95 MB
        #   L=96 → 325 MB              | L=128 → 770 MB
        # wasm32: 2 GB (fits L~117). wasm64: 8 GB (fits L~187).
        -sINITIAL_MEMORY=134217728          # 128 MB — typical L<=64 startup
        -sMAXIMUM_MEMORY=${_FTD_WASM_MAX_MEM}
        -sSTACK_SIZE=1048576                # 1 MB stack (default 64 KB is risky)
        -sNO_EXIT_RUNTIME=1
        -sENVIRONMENT=web
    )
    # Memory64 link flags: 64-bit linear memory + BigInt marshalling of i64
    # across the JS boundary (Embind needs it for 64-bit pointers).
    if(FTD_MEMORY64)
        target_link_options(ftd_wasm PRIVATE -sMEMORY64=1 -sWASM_BIGINT=1)
    endif()
    # Threaded link: pthread runtime + a pre-spawned pool (the parallel.h pool
    # uses std::thread = pthreads). 'node' in ENVIRONMENT lets the Phase-0 GV2
    # determinism/speed harness run under Node (SharedArrayBuffer needs no COI
    # there). The later value of -sENVIRONMENT wins over 'web' set above.
    if(FTD_WASM_THREADS)
        target_link_options(ftd_wasm PRIVATE
            -pthread
            # PRE-SPAWN the pthread pool at module init (Phase 2). The ftd::pool
            # (parallel.h) uses std::threads == pthreads; with POOL_SIZE=0 they
            # were created on-demand at the first parallel_for and the pool's
            # join then blocked on workers that could not finish their async load
            # (the calling thread — now blocked — owns the event loop that would
            # service them): a hard deadlock on the first threaded tick. That is
            # why Phase 1 shipped pool=1 (no threads). Pre-spawning creates the
            # workers at module init, while the main thread is still free, so the
            # first parallel_for grabs READY workers and never blocks.
            #
            # An earlier pre-spawn attempt "hung (16 nested workers)" for a
            # DIFFERENT reason: each em-pthread is spawned as
            # `new Worker(wasm-bridge.worker.js, {name:'em-pthread-N'})`, which
            # re-ran that file top-to-bottom; its unconditional top-level
            # `self.onmessage =` clobbered Emscripten's pthread message handler,
            # so the worker never received its wasm module and init hung. That is
            # now fixed by the em-pthread re-entry guard at the top of
            # wasm-bridge.worker.js (skips the dashboard's onmessage when running
            # as a pthread bootstrap), so pre-spawn is safe. 8 covers the pool
            # cap: ftd::pool tops at min(hardware_concurrency, 8) threads, i.e.
            # <=7 spawned workers (the master runs a chunk itself).
            -sPTHREAD_POOL_SIZE=8
            -sPTHREAD_POOL_SIZE_STRICT=0
            -sENVIRONMENT=node,web,worker
            # Pre-size a large heap so the common L range never triggers growth
            # (ALLOW_MEMORY_GROWTH + pthreads routes heap access through a slower
            # path — emcc warns). Growth stays ON (inherited) as a safety net for
            # very large L; INITIAL_MEMORY 1 GB covers L<=~110 without growing.
            -sINITIAL_MEMORY=1073741824
        )
    endif()
endif()
