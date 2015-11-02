
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
    src/compute/computeMain.cpp
    src/dependencies/stb_image.cpp
    src/render/colors.cpp
    src/render/displayGrid.cpp
    src/render/window.cpp
    src/render/shader.cpp
    src/render/texture.cpp
    src/render/vertexArray.cpp
    src/sim/grid.cpp
	src/sim/mgsolve.cpp
	src/sim/pressure.cpp
    src/tools/log.cpp
)

set(HEADERS
	src/compute/computeMain.hpp
    src/render/colors.hpp
    src/render/displayGrid.hpp
    src/render/opengl.hpp
    src/render/window.hpp
    src/render/shader.hpp
    src/render/texture.hpp
    src/render/vertexArray.hpp
    src/sim/grid.hpp
	src/sim/mgsolve.hpp    
	src/sim/pressure.hpp
    src/tools/log.hpp
)

file(GLOB SHADER "shader/*")
file(GLOB KERNEL "src/opencl/*.cl")

set_source_files_properties(${SHADER} PROPERTIES HEADER_FILE_ONLY TRUE)
set_source_files_properties(${KERNEL} PROPERTIES HEADER_FILE_ONLY TRUE)
set_source_files_properties(${HEADERS} PROPERTIES HEADER_FILE_ONLY TRUE)

if (WIN32)
	set(AS_FLAGS "/W2 /NODEFAULTLIB:MSVCRT" )
else()
	set(AS_FLAGS "-Wall -std=c++14 -Wfatal-errors" )
endif()

##################################
# External libraries
##################################
    
if (WIN32)
	set(CMAKE_PREFIX_PATH 
		"c:\\code\\glew_1_13"
		"c:\\code\\glew_1_13\\lib\\Release\\x64"
		"c:\\code\\glfw_3_1_2"
		"c:\\code\\glfw_3_1_2\\lib-vc2013"
	)
endif()

set(LIBS)
set(INCPATHS "${CMAKE_SOURCE_DIR}/src")

# OpenGL/CL stuff
find_package(GLEW REQUIRED)
find_package(GLFW REQUIRED)
find_package(OpenGL REQUIRED)
find_package(OpenCL REQUIRED)
list(APPEND LIBS ${OPENGL_LIBRARIES} ${GLFW_LIBRARIES} ${GLEW_LIBRARIES} ${OpenCL_LIBRARIES})
list(APPEND INCPATHS ${OPENGL_INCLUDE_DIR} ${GLFW_INCLUDE_DIR} ${GLEW_INCLUDE_DIR} ${OpenCL_INCLUDE_DIRS})
message(${LIBS})

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
    if (WIN32)
		
	else()
		set(AS_FLAGS "${AS_FLAGS} -g" )	
	endif()
else()
    SET(EXECCMD partikel)
	if (WIN32)
		
	else()
		set(AS_FLAGS "${AS_FLAGS} -O3" )
	endif()
endif()

if(APPLE)
    SET(AS_FLAGS "${AS_FLAGS} -DAPPLE")
endif()

add_executable(${EXECCMD} 
    ${SOURCES} 
	${HEADERS}
	${SHADER}
	${KERNEL}
    ${VERSIONFILE}
)

target_include_directories(${EXECCMD} PUBLIC ${INCPATHS})
target_link_libraries(${EXECCMD} ${LIBS})
set_target_properties(${EXECCMD} PROPERTIES COMPILE_FLAGS ${AS_FLAGS})
target_include_directories(${EXECCMD} PUBLIC ${AS_INCLUDES})

##################################
# Make nice file groups for MSVS
##################################

source_group(Headers FILES ${HEADERS})
source_group(Source FILES ${SOURCES} ${VERSIONFILE})
source_group(Shader FILES ${SHADER})
source_group(OpenCL FILES ${KERNEL})

add_custom_target(SourceFiles SOURCES ${SHADER} ${KERNEL})
