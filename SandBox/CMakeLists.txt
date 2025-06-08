macro(CopyShader target_name)
    add_custom_command(
        TARGET ${target_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/Resouces $<TARGET_FILE_DIR:${target_name}>/Resouces)
endmacro(CopyShader)


set(BUILD_SHARED_LIBS ON)
set(GLFW_INSTALL OFF)

add_executable(sandbox SandBox.cpp)
target_link_libraries(sandbox PUBLIC glfw VulkanRender)

CopyShader(sandbox)
#execute_process( COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/src ${save_src_dir})