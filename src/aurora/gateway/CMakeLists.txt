###################
## Build targets ##
###################

add_library(AuroraGateway)
add_library(aurora::gateway ALIAS AuroraGateway)

target_sources(AuroraGateway
    PRIVATE
        gateway.h
        gateway.cc
    )

target_include_directories(AuroraGateway PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

target_link_libraries(AuroraGateway PUBLIC Boost::beast)
