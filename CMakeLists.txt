cmake_minimum_required(VERSION 3.23)
project(saturn LANGUAGES CXX)

find_package(Vulkan REQUIRED)

set(definitions PRIVATE SATURN_BUILD)

set(headers
	"include/saturn/core.hpp"
	"include/saturn/device.hpp"
	"include/saturn/instance.hpp"
	"include/saturn/logger.hpp"
	"include/saturn/physical_device.hpp"
	"include/saturn/swap_chain.hpp"
	"include/saturn/util.hpp"
)

set(sources "src/instance.cpp" "src/device.cpp" "src/logger.cpp"
			"src/physical_device.cpp" "src/swap_chain.cpp" "src/util.cpp"
)

option(SATURN_BUILD_SHARED "Whether to build saturn as a shared library"
	   BUILD_SHARED_LIBS
)
if(SATURN_BUILD_SHARED)
	set(library_type SHARED)
	list(APPEND definitions PUBLIC SATURN_SHARED)
else()
	set(library_type STATIC)
endif()

add_library(${PROJECT_NAME} ${library_type} ${sources} ${headers})

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

target_include_directories(
	${PROJECT_NAME}
	PUBLIC "include"
	PRIVATE "include/saturn" "src"
)

target_link_libraries(${PROJECT_NAME} PUBLIC Vulkan::Vulkan)

target_compile_definitions(${PROJECT_NAME} ${definitions})

option(SATURN_ENABLE_EXAMPLES "Enable examples" ON)
if(SATURN_ENABLE_EXAMPLES)
	add_subdirectory("${PROJECT_SOURCE_DIR}/examples")
endif()
