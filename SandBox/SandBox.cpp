#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <fmt/color.h>

#include "Context.h"
#include "Device.h"
#include "Module.h"
#include "Renderer.h"

GLFWwindow* window = nullptr;
vkRender::Context *context = nullptr;
vkRender::Renderer *renderer = nullptr;

void init();
void mainLoop();
void resetCallback(GLFWwindow*, int, int);

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

    vkRender::Gui::init(window);
    glfwSetFramebufferSizeCallback(window, resetCallback);
    glfwCreateWindowSurface(context->getVkInstance(), window, nullptr, &context->getSurface());
    glfwGetWindowFrameSize(window, nullptr, nullptr, &width, &height);
    context->setFrameSize(width, height);

    const std::vector<vkRender::Vertex> vertexes =
    {
        // 上面
        {-0.8f, -0.8f, 0.f, 1.f, 0.f},
        {0.8f, -0.8f, 0.f, 0.f, 0.f},
        {0.8f, 0.8f, 0.f, 0.f, 1.f},
        {-0.8f, 0.8f, 0.f, 1.f, 1.f},

        // 下面
        {-0.8f, -0.8f, -0.8f, 1.f, 0.f},
        {0.8f, -0.8f, -0.8f, 0.f, 0.f},
        {0.8f, 0.8f, -0.8f, 0.f, 1.f},
        {-0.8f, 0.8f, -0.8f, 1.f, 1.f},

        // 正面
        {-0.8f, 0.8f, 0.f, 1.f, 0.f},
        {0.8f, 0.8f, 0.f, 0.f, 0.f},
        {0.8f, 0.8f, -0.8f, 0.f, 1.f},
        {-0.8f, 0.8f, -0.8f, 1.f, 1.f},

        // 背面
        {0.8f, -0.8f, 0.f, 1.f, 0.f},
        {-0.8f, -0.8f, 0.f, 0.f, 0.f},
        {-0.8f, -0.8f, -0.8f, 0.f, 1.f},
        {0.8f, -0.8f, -0.8f, 1.f, 1.f},
        
        // 左侧面
        {-0.8f, -0.8f, 0.f, 1.f, 0.f},
        {-0.8f, 0.8f, 0.f, 0.f, 0.f},
        {-0.8f, 0.8f, -0.8f, 0.f, 1.f},
        {-0.8f, -0.8f, -0.8f, 1.f, 1.f},
        
        // 右侧面
        {0.8f, 0.8f, 0.f, 1.f, 0.f},
        {0.8f, -0.8f, 0.f, 0.f, 0.f},
        {0.8f, -0.8f, -0.8f, 0.f, 1.f},
        {0.8f, 0.8f, -0.8f, 1.f, 1.f},
    };

    const std::vector<uint32_t> indices = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4,
        8, 9, 10, 10, 11, 8,
        12, 13, 14, 14, 15, 12,
        16, 17, 18, 18, 19, 16,
        20, 21, 22, 22, 23, 20
    };
    
    auto module = vkRender::Module::createFormData(vertexes, indices, "E:/Documents/Project/vkRender/build/bin/Debug/Resouces/image.jpg");
    // auto module = vkRender::Module::create("E:/Documents/Project/vkRender/build/bin/Debug/Resouces/viking_room.obj",
    //     "E:/Documents/Project/vkRender/build/bin/Debug/Resouces/viking_room.png");
    
    renderer = vkRender::Renderer::create();
    
    auto program = vkRender::Program::create("E:/Documents/Project/vkRender/build/bin/Debug/Resouces/vert.spv",
        "E:/Documents/Project/vkRender/build/bin/Debug/Resouces/frag.spv");
    
    renderer->setProgram(program);
    renderer->addModule(module);
    program->setBinding({
        {0, sizeof(vkRender::Vertex), vk::VertexInputRate::eVertex},
    });
    program->setAttribute({
        {0, 0, vk::Format::eR32G32B32Sfloat, 0},
        {1, 0, vk::Format::eR32G32Sfloat, sizeof(float) * 3}
    });
    program->compile(renderer->getSwapchain()->getRenderPass());
    
    program->addImageInfo(module->getTexture().newDescriptor());
    program->setDescriptor({
        vk::DescriptorType::eUniformBuffer,
        vk::DescriptorType::eCombinedImageSampler,
    });

    // vkRender::Logger log;
    //
    // log.log("Hello World");
}

void resetCallback(GLFWwindow* window, int wight, int height)
{
    fmt::print(fg(fmt::color::white), "[glfw] window Size Width: {} Height: {}\n", wight, height);
    context->setFrameSize(wight, height);
}

void mainLoop()
{
    renderer->update();
    renderer->draw();
}
