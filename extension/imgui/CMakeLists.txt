cmake_minimum_required(VERSION 3.1...3.6)
project(imgui LANGUAGES CXX)

set(HEADER
	imconfig.h
	imgui.h
	imgui_impl_glfw.h
	imgui_impl_vulkan.h
	imgui_internal.h
	imstb_rectpack.h
	imstb_textedit.h
	imstb_truetype.h
)

set(SRC
    imgui.cpp
	imgui_demo.cpp
	imgui_draw.cpp
	imgui_impl_glfw.cpp
	imgui_impl_vulkan.cpp
	imgui_tables.cpp
	imgui_widgets.cpp
)

find_package(Vulkan REQUIRED)

list(APPEND SRC ${HEADER})

add_library(imgui STATIC ${SRC})
target_include_directories(imgui PUBLIC .)
target_link_libraries(imgui PUBLIC glfw Vulkan::Vulkan)