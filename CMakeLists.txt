cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_FLAGS "--std=c++14 ${CMAKE_CXX_FLAGS} -O3")

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

find_library(LCURL curl)
find_library(LSIMPLEJSON NAMES SimpleJSON PATHS "../SimpleJSON/build" "SimpleJSON/build" STATIC)

message("-- External libraries")
message("	${LSIMPLEJSON}")

target_link_libraries(${remote_build} ${LSIMPLEJSON} ${Boost_LIBRARIES} ${LCURL})