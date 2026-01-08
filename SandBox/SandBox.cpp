#include <fmt/color.h>

#include "AttributeInfo.h"
#include "Camera.h"
#include "Context.h"
#include "Device.h"
#include "Module.h"
#include "Program.h"
#include "Swapchain.h"
#include "Texture.h"
#include "Renderer.h"
#include "Platform/Application.h"
#include "Platform/Window.h"

vkRender::Renderer *renderer = nullptr;

void init();
void mainLoop();
void resetCallback(GLFWwindow*, int, int);

int main(int argc, char* argv[])
{
    Application* app = new Application(true);
    Window* window_ = new Window(640, 480, "Vulkan Render");
    
    // vkRender::Context::Init(app->getInstanceExtensions());
    // context = vkRender::Context::getInstance();
    
    vkRender::Context context(app->getInstanceExtensions());
    
    window_->setWindowSetSizeCallBack(resetCallback);
    window_->getVkSurface(context.getVkInstance(), &context.getSurface());
    
    vkRender::Device::init(&context);
    
    app->setWindow(window_);
    app->setLoopCullBack(mainLoop);
    app->setResouceDir("E:/Documents/Project/vkRender/build/bin/Debug/Resouces/");
    
    init();
    
    app->run();
    
    renderer->release();
    
    return 0;
}

void init()
{
    vkRender::Camera::Instance()->setPos({2.f, 2.f, 2.f});

    const std::vector<vkRender::Vertex> vertexes =
    {
        {-0.5f, -0.5f, 0.5f, 1.f, 0.f},
        {0.5f, -0.5f, 0.5f, 0.f, 0.f},
        {0.5f, 0.5f, 0.5f, 0.f, 1.f},
        {-0.5f, 0.5f, 0.5f, 1.f, 1.f},
        {-0.5f, -0.5f, -0.5f, 1.f, 0.f},
        {0.5f, -0.5f, -0.5f, 0.f, 0.f},
        {0.5f, 0.5f, -0.5f, 0.f, 1.f},
        {-0.5f, 0.5f, -0.5f, 1.f, 1.f},
        {-0.5f, 0.5f, 0.5f, 1.f, 0.f},
        {0.5f, 0.5f, 0.5f, 0.f, 0.f},
        {0.5f, -0.5f, 0.5f, 1.f, 0.f},
        {-0.5f, -0.5f, 0.5f, 0.f, 0.f},
        {-0.5f, -0.5f, -0.5f, 0.f, 1.f},
        {0.5f, -0.5f, -0.5f, 1.f, 1.f},
        {-0.5f, 0.5f, 0.5f, 0.f, 0.f},
        {-0.5f, 0.5f, -0.5f, 0.f, 1.f},
        {-0.5f, -0.5f, -0.5f, 1.f, 1.f},
        {0.5f, 0.5f, 0.5f, 1.f, 0.f},
        {0.5f, -0.5f, -0.5f, 0.f, 1.f},
        {0.5f, 0.5f, -0.5f, 1.f, 1.f},
    };

    const std::vector<uint32_t> indices = {
        0, 1, 2,        2, 3, 0, 
        4, 5, 6,        6, 7, 4,
        8, 9, 6,        6, 7, 8,
        10, 11, 12,     12, 13, 10,
        0, 14, 15,      15, 16, 0,
        17, 1, 18,      18, 19, 17
    };
    
    const std::vector<glm::vec3> instancePos = {{0, 0, -1.2f}, {0, 0, 0}, {0, 0, 1.2f}};
    
    auto module = vkRender::Module::createFormData(vertexes, indices, "image.jpg");
    // auto module2 = vkRender::Module::createFormData(frame, {}, "");
    // auto module2 = vkRender::Module::create("E:/Documents/Project/vkRender/build/bin/Debug/Resouces/viking_room.obj",
    //     "E:/Documents/Project/vkRender/build/bin/Debug/Resouces/viking_room.png");
    
    renderer = vkRender::Renderer::create();

    const auto program = vkRender::Program::create("vert.spv", "frag.spv");
    
    // program->configAttribute()
    
    program->setBinding({
        {0, sizeof(vkRender::Vertex), vk::VertexInputRate::eVertex},
        {1, sizeof(glm::vec3), vk::VertexInputRate::eInstance}
    });
    
    program->addAttribute({vk::Format::eR32G32B32Sfloat, sizeof(float) * 3})
        .addAttribute({vk::Format::eR32G32Sfloat, sizeof(float) * 2})
        .addAttribute({1, vk::Format::eR32G32B32Sfloat, sizeof(float) * 3})
        .compile(renderer->getSwapchain()->getRenderPass());
    
    program->addImageInfo(module->getTexture().newDescriptor());
    program->setDescriptor({
        vk::DescriptorType::eUniformBuffer,
        vk::DescriptorType::eCombinedImageSampler,
    });
    
    renderer->setProgram(program)
        .addModule(module)
        .addVertexData(instancePos);
}

void resetCallback(GLFWwindow* window, int wight, int height)
{
    fmt::print(fg(fmt::color::white), "[glfw] window Size Width: {} Height: {}\n", wight, height);
    // context->setFrameSize(wight, height);
}

void mainLoop()
{
    renderer->update();
    renderer->draw();
}
