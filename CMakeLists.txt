cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)
set(PROJECT_NAME WolfTL)
project(${PROJECT_NAME} C CXX)

# Check the current build type and set it to Release if it's not specified
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build" FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Set C++ standard to C++20
set(CMAKE_CXX_STANDARD 20)

set(src
	"${CMAKE_CURRENT_SOURCE_DIR}/WolfTL/3rdParty/lz4/lz4.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/WolfTL/WolfTL.cpp"
)

if(MSVC)
	# Add several preprocessor defines for MSVC
	add_compile_definitions(
		"NOMINMAX"
		"_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING"
		"UNICODE"
		"_UNICODE"
	)

	# Set the application icon
	set(APP_ICON_RESOURCE "${CMAKE_CURRENT_SOURCE_DIR}/WolfTL/WolfTL.rc")
	set(src ${src} ${APP_ICON_RESOURCE})
endif()

# Set the output directory for the executable
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

add_executable(${PROJECT_NAME} ${src})
target_include_directories(${PROJECT_NAME} PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/WolfTL/3rdParty"
	"${CMAKE_CURRENT_SOURCE_DIR}/WolfTL"
)

if (MSVC)
	# Set MT / MTd for static runtime linking
	set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()