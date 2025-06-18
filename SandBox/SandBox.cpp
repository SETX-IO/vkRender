#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Context.h"
#include "Renderer.h"
#include "fmt/args.h"
// #include "fmt/color.h"
// #include "Logger/Logger.h"

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
    
    renderer->release();
    context->release();
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
        
        extensions[i] = extension;
    }
    
    context = vkRender::Context::getInstance(extensions,
        [&](vk::Instance instance) {
            VkSurfaceKHR surface;
            if (glfwCreateWindowSurface(instance, window, nullptr, &surface))
                return surface;
            return surface;
    });

    const std::vector<vkRender::Vertex> vertexes =
{
        {-0.8f, -0.8f, 0.f, 1.f, 0.f},
        {0.8f, -0.8f, 0.f, 0.f, 0.f},
        {0.8f, 0.8f, 0.f, 0.f, 1.f},
        {-0.8f, 0.8f, 0.f, 1.f, 1.f},

        {-0.8f, -0.8f, -0.8f, 1.f, 0.f},
        {0.8f, -0.8f, -0.8f, 0.f, 0.f},
        {0.8f, 0.8f, -0.8f, 0.f, 1.f},
        {-0.8f, 0.8f, -0.8f, 1.f, 1.f},

        {-0.8f, 0.8f, 0.f, 1.f, 0.f},
        {0.8f, 0.8f, 0.f, 0.f, 0.f},
        {0.8f, 0.8f, -0.8f, 0.f, 1.f},
        {-0.8f, 0.8f, -0.8f, 1.f, 1.f},
    };

    const std::vector<uint16_t> indices = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4,
        8, 9, 10, 10, 11, 8
    };
    
    renderer = vkRender::Renderer::create();
    renderer->addVertexData(vertexes);
    renderer->addIndexData(indices);

    int width = 0;
    int height = 0;
    glfwGetWindowFrameSize(window, nullptr, nullptr, &width, &height);

    auto texture_ = vkRender::Texture::createFormFile("E:/Documents/Project/vkRender/build/bin/Debug/Resouces/image.jpg");
    auto program = vkRender::Program::create(renderer->getSwapchain()->getRenderPass(), width, width);
    program->addBufferInfo();
    program->addImageInfo(texture_->newDescriptor());
    program->buildDescriptorSet();

    renderer->setProgram(program);
    
    // texture_->release();
    

    
    // context->setFrameSize(glm::vec2(width, height));

    // vkRender::Logger log;
    //
    // log.log("Hello World");
    
}


void mainLoop()
{
    renderer->draw();
}
