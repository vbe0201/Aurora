# Enable modules to include each other's files.
include_directories(.)

#######################
## Compiler settings ##
#######################

# Set compilation flags.
if(MSVC)
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
add_subdirectory(gateway)
