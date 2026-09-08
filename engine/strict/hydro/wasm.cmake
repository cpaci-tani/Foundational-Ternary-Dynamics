if(EMSCRIPTEN)
  add_executable(ftd_hydro_wasm wasm_bindings_hydro.cpp)
  target_link_libraries(ftd_hydro_wasm PRIVATE ftd_hydro_candidate)
  target_compile_options(ftd_hydro_candidate PRIVATE -fexceptions)
  target_compile_options(ftd_hydro_wasm PRIVATE -fexceptions)
  target_link_options(ftd_hydro_wasm PRIVATE
    --bind -fexceptions
    -sMODULARIZE=1 -sEXPORT_ES6=1 -sEXPORT_NAME=createHydroModule
    -sENVIRONMENT=worker,node -sALLOW_MEMORY_GROWTH=1 -sMAXIMUM_MEMORY=1073741824
    -sDISABLE_EXCEPTION_CATCHING=0)
  set_target_properties(ftd_hydro_wasm PROPERTIES SUFFIX ".mjs")
endif()
