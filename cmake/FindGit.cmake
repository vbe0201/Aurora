include(FindPackageHandleStandardArgs)

find_program(GIT_BINARY
    NAMES git
    DOC "git CLI client."
    )

find_package_handle_standard_args(Git
    FOUND_VAR GIT_FOUND
    REQUIRED_VARS GIT_BINARY
    )

mark_as_advanced(GIT_BINARY)
