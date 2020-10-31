###################
## Build targets ##
###################

add_library(AuroraCommon STATIC
    assert.h
    assert.cc
    defines.h
    )
add_library(aurora::common ALIAS AuroraCommon)
