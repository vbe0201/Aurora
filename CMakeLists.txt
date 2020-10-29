cmake_minimum_required(VERSION 3.8)

######################
## Project settings ##
######################

# Inject relevant project directories into CMAKE_MODULE_PATH.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Declare the project.
project(Aurora
    VERSION "0.1.0"
    LANGUAGES CXX
    )

include(GitRevision)
message(STATUS "Compiling Aurora ${CMAKE_PROJECT_VERSION}, git revision ${AURORA_GIT_REVISION}")

#########################
## CMake configuration ##
#########################

# Set the CMake build type to release, if nothing has been set previously yet.
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose build type." FORCE)
endif()

# Set the C++ standard.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Relent on using C++ extensions, except within Cygwin environments.
if(CMAKE_SYSTEM_NAME MATCHES "CYGWIN")
    set(CMAKE_CXX_EXTENSIONS ON)
else()
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()

# Nicely support folders in IDEs.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

####################
## Subdirectories ##
####################

add_subdirectory(third-party)
add_subdirectory(src/aurora)
