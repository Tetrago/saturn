cmake_minimum_required(VERSION 3.24)
project(saturn LANGUAGES CXX)

find_package(Vulkan REQUIRED)

set(libraries PUBLIC Vulkan::Vulkan)

set(definitions PRIVATE SATURN_BUILD)

set(headers
	"include/saturn/command.hpp"
	"include/saturn/core.hpp"
	"include/saturn/device.hpp"
	"include/saturn/error.hpp"
	"include/saturn/framebuffer.hpp"
	"include/saturn/instance.hpp"
	"include/saturn/physical_device.hpp"
	"include/saturn/pipeline.hpp"
	"include/saturn/render_pass.hpp"
	"include/saturn/shader.hpp"
	"include/saturn/swapchain.hpp"
	"include/saturn/sync.hpp"
	"src/local.hpp"
)

set(sources
	"src/command.cpp"
	"src/device.cpp"
	"src/error.cpp"
	"src/framebuffer.cpp"
	"src/instance.cpp"
	"src/physical_device.cpp"
	"src/pipeline.cpp"
	"src/render_pass.cpp"
	"src/shader.cpp"
	"src/swapchain.cpp"
	"src/sync.cpp"
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

target_link_libraries(${PROJECT_NAME} ${libraries})

target_compile_definitions(${PROJECT_NAME} ${definitions})

option(SATURN_ENABLE_EXAMPLES "Enable examples" ON)
if(SATURN_ENABLE_EXAMPLES)
	add_subdirectory("${PROJECT_SOURCE_DIR}/examples")
endif()
