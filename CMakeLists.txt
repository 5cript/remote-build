cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++14")
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -g -O0")
set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

## Use the variable PROJECT_NAME for changing the target name
set(remote_build "remote-build")

## Set our project name
project(${remote_build})

## Use all the *.cpp files we found under this folder for the project
file(GLOB sources "*.cpp" 
			      "filesystem/*.cpp"
				  "network/*.cpp"
)

## Define the executable
add_executable(${remote_build} ${sources})

target_include_directories(${remote_build} PUBLIC "..")

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
endif()

find_library(LCURL curl)
find_library(LCRYPTOPP cryptopp)
find_library(LSIMPLEJSON NAMES SimpleJSON PATHS "../SimpleJSON/build" "SimpleJSON/build" STATIC)
find_library(LATTENDEE NAMES attendee PATHS "../attendee/build" "attendee/build" STATIC)

message("-- External libraries")
message("	${LSIMPLEJSON}")
message("	${LATTENDEE}")

target_link_libraries(${remote_build} ${LATTENDEE} ${LSIMPLEJSON} ${Boost_LIBRARIES} ${LCRYPTOPP} ${LCURL})