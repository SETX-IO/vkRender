
cmake_minimum_required(VERSION 3.6)

include(VulkanRender/CMakeLists.txt)
include(extension/CMakeLists.txt)

set(lib_name VulkanRender)
project(${lib_name})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(RES_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/Shaders")
set(FMT_INSTALL OFF)

find_package(Vulkan REQUIRED)

list(APPEND RENDER_SRC ${RENDER_HEADER} ${EXTENSION} ${RES_FOLDER}/DefultPipeLine.vert ${RES_FOLDER}/DefultPipeLine.frag)
add_subdirectory(library)

find_program(GLSLC_PROFRAM glslc REQUIRE)

execute_process(COMMAND ${GLSLC_PROFRAM} ${RES_FOLDER}/DefultPipeLine.vert -o ${RES_FOLDER}/vert.spv)
execute_process(COMMAND ${GLSLC_PROFRAM} ${RES_FOLDER}/DefultPipeLine.frag -o ${RES_FOLDER}/frag.spv)

add_library(${lib_name} STATIC ${RENDER_SRC})
target_include_directories(${lib_name} PUBLIC VulkanRender extension)
# target_precompile_headers(${lib_name} PRIVATE VulkanRender/vkRender.h)
target_link_libraries(${lib_name} PUBLIC Vulkan::Vulkan fmt freetype imgui)
target_compile_features(${lib_name} PUBLIC cxx_std_17)

add_subdirectory(SandBox)