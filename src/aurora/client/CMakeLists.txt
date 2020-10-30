###################
## Build targets ##
###################

add_executable(Aurora main.cc)

add_dependencies(Aurora aurora::gateway)

target_link_libraries(Aurora PRIVATE aurora::gateway)
