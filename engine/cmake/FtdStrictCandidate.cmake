# Selected research runtime. Explicit opt-in; never changes legacy engine laws.
option(FTD_BUILD_STRICT_CANDIDATE "Build the separately versioned finite staged candidate" OFF)
if(FTD_BUILD_STRICT_CANDIDATE)
    add_subdirectory(strict)
endif()
