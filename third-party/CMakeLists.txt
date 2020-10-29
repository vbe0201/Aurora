# A macro to ensure that all submodules are present.
macro(check_submodules)
    # Read and extract all modules from .gitmodules.
    file(READ "${CMAKE_SOURCE_DIR}/.gitmodules" gitmodules)
    string(REGEX MATCHALL "path *= *[^\t\r\n]*" modules ${gitmodules})

    # Verify that each module has a .git directory in its path,
    # i.e. ensure that all submodules are initialized and available.
    foreach(module ${modules})
        string(REGEX REPLACE "path *= *" "" module ${module})
        if(NOT EXISTS "${CMAKE_SOURCE_DIR}/${module}/.git")
            message(FATAL_ERROR "${module} not found. Run git submodule update --init --recursive")
        endif()
    endforeach()
endmacro()

check_submodules()

####################
## Subdirectories ##
####################

# zlib
add_subdirectory(zlib)
set(ZLIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/zlib)
set(ZLIB_LIBRARY zlib)

# bzip2
add_subdirectory(bzip2)
set(BZIP2_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bzip2)
set(BZIP2_LIBRARIES libz2)

# boost-cmake
add_subdirectory(boost-cmake)
