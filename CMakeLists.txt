cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

set(CMAKE_SYSTEM_VERSION 10.0 CACHE STRING "" FORCE)

set(PROJECT_NAME WolfTL)

project(${PROJECT_NAME} C CXX)

# Set C++ standard to C++20
set(CMAKE_CXX_STANDARD 20)

set(src "WolfTL/3rdParty/lz4/lz4.c" "WolfTL/WolfTL.cpp")

# Set the output directory for the executable
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

add_executable(${PROJECT_NAME} ${src})
target_include_directories(${PROJECT_NAME} PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/WolfTL/3rdParty"
	"${CMAKE_CURRENT_SOURCE_DIR}/WolfTL"
)
