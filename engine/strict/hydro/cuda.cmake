# FTD_HYDRO_CUDA is declared by the parent CMakeLists.txt (guarded by if(EXISTS ...) before
# this file is even included), so it is not redeclared here.
if(FTD_HYDRO_CUDA)
  enable_language(CUDA)
  add_library(ftd_hydro_cuda STATIC hydro_cuda.cu)
  target_link_libraries(ftd_hydro_cuda PUBLIC ftd_hydro_candidate)
  target_include_directories(ftd_hydro_cuda PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
  set_target_properties(ftd_hydro_cuda PROPERTIES CUDA_STANDARD 17 CUDA_STANDARD_REQUIRED ON)
  add_executable(ftd_hydro_cuda_cli cuda_main.cpp)
  target_link_libraries(ftd_hydro_cuda_cli PRIVATE ftd_hydro_cuda)
endif()
