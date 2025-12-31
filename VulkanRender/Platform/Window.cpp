#include "Window.h"


Window::Window(int widht, int heigth, const char* title) : isClose_(false), setSizeCallBack(nullptr)
{
    windowPtr_.reset(glfwCreateWindow(widht, heigth, title, nullptr, nullptr));
}

void Window::getVkSurface(VkInstance instance, VkSurfaceKHR* surface) const
{
    glfwCreateWindowSurface(instance, windowPtr_.get(), nullptr, surface);
}

void Window::getSize(int* width, int* height) const
{
    glfwGetWindowSize(windowPtr_.get(), width, height);
}

bool Window::getIsClose()
{
    isClose_ = glfwWindowShouldClose(windowPtr_.get());
    return isClose_;
}

void Window::setWindowSetSizeCallBack(SetSizeCallBackPtr cullbackPtr)
{
    if (this->setSizeCallBack != cullbackPtr)
        this->setSizeCallBack = cullbackPtr;
    glfwSetFramebufferSizeCallback(windowPtr_.get(), this->setSizeCallBack);
}

