#include "Application.h"

#include <cassert>

#include "Window.h"

Application::Application(bool useVulkan) : window_(nullptr), useVulkan(useVulkan), loopCullBack_(nullptr)
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, !useVulkan);
    glfwWindowHint(GLFW_RESIZABLE, !useVulkan);
}

void Application::run(Window * window)
{
    window_.reset(window);
    while (!window_->getIsClose())
    {
        loopCullBack_();
        
        glfwPollEvents();
    }
    glfwTerminate();
}

std::vector<const char*> Application::getInstanceExtensions()
{
    assert(useVulkan);
    
    uint32_t count;
    const char** extensionArray = glfwGetRequiredInstanceExtensions(&count);
    std::vector extensions(extensionArray, extensionArray + count);
#ifdef  _DEBUG
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
    
    return extensions;
}

void Application::setLoopCullBack(LoopCullBackPtr cullBcak)
{
    this->loopCullBack_ = cullBcak;
}
