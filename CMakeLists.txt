
project (Partikel)

cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/src/cmake/")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
    
##################################
# Build options
##################################

option(DEBUG "Enable debug compilation" OFF)

message(STATUS "Options - "
    " -DDEBUG='${DEBUG}' "
    )

##################################
# Source files
##################################

set(SOURCES
    src/main.cpp
    src/render/init.cpp
    src/tools/log.cpp
)

set(AS_FLAGS "-Wall -std=c++14 -Wfatal-errors" )

##################################
# External libraries
##################################

set(LIBS)
set(INCPATHS "${CMAKE_SOURCE_DIR}/src")

# OpenGL stuff
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLFW REQUIRED)
list(APPEND LIBS ${OPENGL_LIBRARIES} ${GLFW_LIBRARIES} ${GLEW_LIBRARIES})
list(APPEND INCPATHS ${OPENGL_INCLUDE_DIR} ${GLFW_INCLUDE_DIR} ${GLEW_INCLUDE_DIR})

##################################
# Add git revision
##################################

if (EXISTS "${CMAKE_SOURCE_DIR}/.git/index")
    include(GetGitRevisionDescription)
    get_git_head_revision(BRANCH HASH)
else()
    set(HASH "unknown")
    set(BRANCH "unknown")
endif()
if (DEBUG)
    set(BUILDS "debug")
else()
    set(BUILDS "release")
endif()
string(SUBSTRING ${HASH} 0 7 HASH_SHORT)

set(VERSIONFILE ${CMAKE_CURRENT_BINARY_DIR}/version.cpp)
configure_file(src/gitid_template.cpp ${VERSIONFILE})

##################################
# Build executable
##################################

if (DEBUG)
    SET(EXECCMD partikeld)
    set(AS_FLAGS "${AS_FLAGS} -g" )
else()
    SET(EXECCMD partikel)
    set(AS_FLAGS "${AS_FLAGS} -O3 -Wreturn-type" )
endif()

add_executable(${EXECCMD} 
    ${SOURCES} 
    ${VERSIONFILE}
)

target_include_directories(${EXECCMD} PUBLIC ${INCPATHS})
target_link_libraries(${EXECCMD} ${LIBS})
set_target_properties(${EXECCMD} PROPERTIES COMPILE_FLAGS ${AS_FLAGS})
target_include_directories(${EXECCMD} PUBLIC ${AS_INCLUDES})

