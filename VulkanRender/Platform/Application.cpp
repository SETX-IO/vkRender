#include "Application.h"

#include <cassert>

#include "Window.h"

std::unique_ptr<Application> Application::currentPtr_ = nullptr;

Application::Application(bool useVulkan): 
window_(nullptr), 
useVulkan(useVulkan),
loopCullBack_(nullptr)
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, !useVulkan);
    glfwWindowHint(GLFW_RESIZABLE, !useVulkan);
    
    currentPtr_.reset(this);
}

Application& Application::Current()
{
    return *currentPtr_;
}

void Application::run(Window* window)
{
    if (!window_)
        window_.reset(window);
    
    while (!window_->getIsClose())
    {
        loopCullBack_();
        
        glfwPollEvents();
    }
    glfwTerminate();
}

void Application::run()
{
    run(window_.get());
}

std::vector<const char*> Application::getInstanceExtensions() const
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

void Application::setResouceDir(const char* path)
{
    assert(strlen(path));
    
    resoucesDir_ = path;
}

std::string Application::getResouceDir() const
{
    return resoucesDir_.string();
}

void Application::setWindow(Window* window)
{
    assert(window);
    window_.reset(window);
}

Window& Application::getWindow() const
{
    return *window_;
}
