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

# OpenSSL
find_package(OpenSSL REQUIRED)

# boost-cmake
add_subdirectory(boost-cmake)

# Boost.ASIO
add_library(Boost_asio INTERFACE)
add_library(Boost::asio ALIAS Boost_asio)
if(WIN32)
    set(_asio_platform_libs ws2_32)
endif()
target_link_libraries(Boost_asio
    INTERFACE
        Boost::boost
        Boost::system
        ${_asio_platform_libs}
        OpenSSL::SSL
    )

# Boost.Beast
add_library(Boost_beast INTERFACE)
add_library(Boost::beast ALIAS Boost_beast)
target_link_libraries(Boost_beast
    INTERFACE
        Boost::asio
    )

# json
set(JSON_BuildTests OFF CACHE INTERNAL "Build json without tests.")
add_subdirectory(json EXCLUDE_FROM_ALL)
