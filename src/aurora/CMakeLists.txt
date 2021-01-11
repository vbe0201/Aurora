# Enable modules to include each other's files.
include_directories(.)

#######################
## Compiler settings ##
#######################

# Set compilation flags.
if(MSVC)
    macro(get_WIN32_WINNT_value version)
        if(CMAKE_SYSTEM_VERSION)
            # Extract the WIN32_WINNT version value.
            set(__win32_winnt_version ${CMAKE_SYSTEM_VERSION})
            string(REGEX MATCH "^([0-9]+).([0-9])" __win32_winnt_version ${__win32_winnt_version})
            string(REGEX MATCH "^([0-9]+)" __win32_winnt_version_major ${__win32_winnt_version})
            if("${__win32_winnt_version_major}" MATCHES "10")
                # Windows 10 needs a fix, since version starts with hex "A", not "10".
                set(__win32_winnt_version_major "A")
                string(REGEX REPLACE
                    "^([0-9]+)"
                    ${__win32_winnt_version_major}
                    __win32_winnt_version
                    ${__win32_winnt_version}
                    )
            endif()

            # Strip away all the "." characters and prepend each digit with "0".
            string(REPLACE "." "" __win32_winnt_version ${__win32_winnt_version})
            string(REGEX REPLACE "([0-9A-Z])" "0\\1" __win32_winnt_version ${__win32_winnt_version})
            set(${version} "0x${__win32_winnt_version}")
        endif()
    endmacro()

    # Silence "deprecation" warnings.
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_DEPRECATE -D_SCL_SECURE_NO_WARNINGS)

    # Circumvent certain windows.h issues.
    add_definitions(-DNOMINMAX -DWIN32_LEAN_AND_MEAN)

    # Make sure the project builds with Unicode support.
    add_definitions(-DUNICODE -D_UNICODE)

    # General compilation options.
    add_compile_options(
        /W3
        /MP
        /permissive-
        /EHsc
        /volatile:iso
        /Zc:inline
    )

    # Read and define WIN32_WINNT version to silence MSVC's crying.
    get_WIN32_WINNT_value(WIN32_WINNT_VERSION)
    add_definitions(-D_WIN32_WINNT=${WIN32_WINNT_VERSION})
else()
    # Add correct libc++ stdlib definition for Clang on Apple platforms.
    if(APPLE AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        add_compile_options(-stdlib=libc++)
    endif()

    # General compilation options.
    add_compile_options(
        -Wall
        -Werror=implicit-fallthrough
        -Werror=missing-declarations
        -Werror=reorder
        -Werror=unused-result
        -Wextra
        -Wmissing-declarations
        -Wno-attributes
        -Wno-unused-parameter
    )
endif()

####################
## Subdirectories ##
####################

add_subdirectory(client)
# Common is only included
#add_subdirectory(common)
add_subdirectory(gateway)
