# H2-prime campaign instrument (ftd_hydro_campaign). campaign_main.cpp lives one directory
# up (engine/strict/hydro/campaign_main.cpp), beside the CUDA/native sources it depends on;
# this subdirectory exists only so the target can be added with a single, minimal
# add_subdirectory(campaign) line in the parent CMakeLists.txt (the one line this task is
# allowed to add there), guarded the same way cuda.cmake already guards FTD_HYDRO_CUDA.
find_package(OpenSSL REQUIRED COMPONENTS Crypto)
add_executable(ftd_hydro_campaign "${CMAKE_CURRENT_SOURCE_DIR}/../campaign_main.cpp")
target_link_libraries(ftd_hydro_campaign PRIVATE ftd_hydro_cuda OpenSSL::Crypto)
target_compile_features(ftd_hydro_campaign PRIVATE cxx_std_17)
# campaign.py's RUNNER constant is "engine/build_strict_hydro_cuda/ftd_hydro_campaign" --
# the top-level build directory, not this add_subdirectory()'s own mirrored subdirectory
# (CMake's default per-target output location). Match the sibling CLIs (ftd_hydro_cli,
# ftd_hydro_cuda_cli), whose CMakeLists.txt live directly in the parent directory.
set_target_properties(ftd_hydro_campaign PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
