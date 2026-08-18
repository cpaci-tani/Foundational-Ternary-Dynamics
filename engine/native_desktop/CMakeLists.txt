add_library(ftd_native_session STATIC
    src/engine_session.cpp
)
target_include_directories(ftd_native_session
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(ftd_native_session PUBLIC ftd_core)

if(WIN32)
    add_executable(ftd_native_desktop
        src/main.cpp
        src/d3d12_presenter.cpp
    )
    target_include_directories(ftd_native_desktop PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    target_link_libraries(ftd_native_desktop PRIVATE
        ftd_native_session
        d3d12
        dxgi
        d3dcompiler
        comctl32
    )
endif()
