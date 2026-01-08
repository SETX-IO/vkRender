#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <memory>

struct DestroyglfwWin
{
    void operator()(GLFWwindow* ptr) const
    {
        glfwDestroyWindow(ptr);
    }
};

typedef void(*SetSizeCallBackPtr)(GLFWwindow* window, int wight, int height);

class Window final
{
public:
    Window(int widht, int heigth, const char* title);
    
    void getVkSurface(VkInstance instance, VkSurfaceKHR* surface) const;
    void getSize(int* width, int* height) const;
    void getSize(unsigned int* width, unsigned int* height) const;
    bool getIsClose();
    
    void setWindowSetSizeCallBack(SetSizeCallBackPtr cullbackPtr);
private:
    std::unique_ptr<GLFWwindow, DestroyglfwWin> windowPtr_;
    
    bool isClose_;
    SetSizeCallBackPtr setSizeCallBack;
};