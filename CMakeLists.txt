cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++17")
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -g -O0")
set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

## Use the variable PROJECT_NAME for changing the target name
set(remote_build "remote-build")
set(remote_build_lib "remotebuild")

## Set our project name
project(${remote_build})

## Use all the *.cpp files we found under this folder for the project
file(GLOB sources "*.cpp" 
			      "filesystem/*.cpp"
				  "network/*.cpp"
)

set(lib_sources ${sources})
list(REMOVE_ITEM lib_sources ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

## Define the executable
add_executable(${remote_build} ${sources})
add_library(${remote_build_lib} SHARED ${lib_sources})

target_include_directories(${remote_build} PRIVATE "..")
target_include_directories(${remote_build_lib} PRIVATE "..")
target_compile_definitions(${remote_build_lib} PRIVATE BUILD_DLL)

## Libraries
### Boost
find_package(Boost REQUIRED COMPONENTS
	system
	filesystem
	program_options
)

if (WIN32)
	find_library(LDEBUG dbghelp)
	
	target_link_libraries(${remote_build} ${LDEBUG})
	target_link_libraries(${remote_build_lib} ${LDEBUG})
endif()

find_library(LCURL curl)
find_library(LCRYPTOPP cryptopp)
find_library(LSIMPLEJSON NAMES SimpleJSON PATHS "../SimpleJSON/build" "SimpleJSON/build" STATIC)
find_library(LATTENDEE NAMES attendee PATHS "../attendee/build" "attendee/build" STATIC)

message("-- External libraries")
message("	${LSIMPLEJSON}")
message("	${LATTENDEE}")

target_link_libraries(${remote_build} ${LATTENDEE} ${LSIMPLEJSON} ${Boost_LIBRARIES} ${LCRYPTOPP} ${LCURL})
target_link_libraries(${remote_build_lib} ${LATTENDEE} ${LSIMPLEJSON} ${Boost_LIBRARIES} ${LCRYPTOPP} ${LCURL})