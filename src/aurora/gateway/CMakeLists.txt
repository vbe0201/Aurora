###################
## Build targets ##
###################

add_library(AuroraGateway)
add_library(aurora::gateway ALIAS AuroraGateway)

target_sources(AuroraGateway
    PRIVATE
        gateway.h
        gateway.cc
        opcodes.h
    )

target_include_directories(AuroraGateway PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

target_link_libraries(AuroraGateway PUBLIC Boost::beast)
target_link_libraries(AuroraGateway PRIVATE nlohmann_json::nlohmann_json)
