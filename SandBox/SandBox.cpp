#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "fmt/args.h"
// #include "fmt/color.h"

#include "Context.h"
#include "Renderer.h"
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
    int width = 0;
    int height = 0;
    uint32_t count;
    const char** extensionArray = glfwGetRequiredInstanceExtensions(&count);
    std::vector extensions(extensionArray, extensionArray + count);
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    context = vkRender::Context::getInstance(extensions);
    
    glfwCreateWindowSurface(context->getVkInstance(), window, nullptr, &context->getSurface());
    glfwGetWindowFrameSize(window, nullptr, nullptr, &width, &height);
    context->setFrameSize(width, height);

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
    
    auto texture_ = vkRender::Texture::createFormFile("E:/Documents/Project/vkRender/build/bin/Debug/Resouces/image.jpg");
    auto program = vkRender::Program::create("E:/Documents/Project/vkRender/build/bin/Debug/Resouces/vert.spv",
        "E:/Documents/Project/vkRender/build/bin/Debug/Resouces/frag.spv");
    
    program->compile(renderer->getSwapchain()->getRenderPass());
    program->addImageInfo(texture_->newDescriptor());
    program->setBinding({
        vk::DescriptorType::eUniformBuffer,
        vk::DescriptorType::eCombinedImageSampler,
    });
    
    renderer->setProgram(program);


    // Release *release = texture_;
    // release->release();
    // texture_->release();

    // vkRender::Logger log;
    //
    // log.log("Hello World");
    
}

void mainLoop()
{
    renderer->draw();
}
