# Set a default value, for systems without git.
set(AURORA_GIT_REVISION "unknown")

find_package(Git)
if(GIT_FOUND)
    execute_process(
        COMMAND ${GIT_BINARY} rev-parse --short HEAD
        OUTPUT_VARIABLE AURORA_GIT_REVISION
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
endif()
