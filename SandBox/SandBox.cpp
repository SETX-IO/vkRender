#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "d3d11.h"

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
    
    context = vkRender::Context::getInstance(extensions);
    
    glfwCreateWindowSurface(context->getVkInstance(), window, nullptr, &context->getSurface());

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
    auto program = vkRender::Program::create("E:/Documents/Project/vkRender/build/bin/Debug/Resouces/vert.spv",
        "E:/Documents/Project/vkRender/build/bin/Debug/Resouces/frag.spv");
    
    program->compile(renderer->getSwapchain()->getRenderPass(), width, width);
    program->addImageInfo(texture_->newDescriptor());
    program->setBinding({
        vk::DescriptorType::eUniformBuffer,
        vk::DescriptorType::eCombinedImageSampler,
    });
    
    renderer->setProgram(program);


    // Release *release = texture_;
    // release->release();
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
