###################
## Build targets ##
###################

add_library(AuroraCommon STATIC
        assert.h
        defines.h
        )
# This is required as long as we don't have any actual source files
set_target_properties(AuroraCommon PROPERTIES LINKER_LANGUAGE CXX)

add_library(aurora::common ALIAS AuroraCommon)
