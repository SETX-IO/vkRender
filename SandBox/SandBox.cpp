#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Context.h"
#include "Renderer.h"
#include "fmt/args.h"
#include "fmt/color.h"

GLFWwindow* window = nullptr;
vkRender::Context *context = nullptr;
vkRender::Renderer *renderer = nullptr;

void mainLoop();
static void init();

int main(int argc, char* argv[])
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(640, 480, "Vulkan Render", nullptr, nullptr);

    init();

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        mainLoop();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

void init()
{
    unsigned int count;
    const char** extensionArray = glfwGetRequiredInstanceExtensions(&count);
    std::vector<const char*> extensions(count);
    for (int i = 0; i < count; ++i)
    {
        const char *extension = extensionArray[i];
        // std::cout << extension << "\n";
        
        extensions[i] = extension;
    }
    
    context = vkRender::Context::getInstance(extensions,
        [&](vk::Instance instance) {
            VkSurfaceKHR surface;
            if (glfwCreateWindowSurface(instance, window, nullptr, &surface))
                return surface;
            return surface;
    });

    renderer = vkRender::Renderer::create(context);
    
    // int width = 0;
    // int height = 0;
    // glfwGetWindowFrameSize(window, nullptr, nullptr, &width, &height);
    
    // context->setFrameSize(glm::vec2(width, height));

    fmt::print(fg(fmt::color::crimson) | fmt::emphasis::bold,
             "[{}] Hello, {}!\n", "Error", "world");
}


void mainLoop()
{
    renderer->draw();
    context->draw();
}
