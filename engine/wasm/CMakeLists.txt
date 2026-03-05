if(EMSCRIPTEN)
    add_executable(ftd_wasm ftd_wasm.cpp)
    target_link_libraries(ftd_wasm PRIVATE ftd_core)
    set_target_properties(ftd_wasm PROPERTIES
        OUTPUT_NAME "ftd_core"
        SUFFIX ".js"
    )
    target_link_options(ftd_wasm PRIVATE
        --bind
        -sEXPORT_NAME=createFTDModule
        -sMODULARIZE=1
        -sALLOW_MEMORY_GROWTH=1
        -sINITIAL_MEMORY=67108864
        -sMAXIMUM_MEMORY=536870912
        -sNO_EXIT_RUNTIME=1
        -sENVIRONMENT=web
    )
endif()
