if(EMSCRIPTEN)
  add_executable(ftd_strict_wasm wasm_bindings.cpp)
  target_link_libraries(ftd_strict_wasm PRIVATE ftd_strict_candidate)
  target_compile_options(ftd_strict_candidate PRIVATE -fexceptions)
  target_compile_options(ftd_strict_wasm PRIVATE -fexceptions)
  target_link_options(ftd_strict_wasm PRIVATE
    --bind -fexceptions
    -sMODULARIZE=1 -sEXPORT_ES6=1 -sEXPORT_NAME=createStrictModule
    -sENVIRONMENT=worker,node -sALLOW_MEMORY_GROWTH=1
    -sDISABLE_EXCEPTION_CATCHING=0)
  set_target_properties(ftd_strict_wasm PROPERTIES SUFFIX ".mjs")
endif()
