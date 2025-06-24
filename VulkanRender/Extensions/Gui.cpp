#include "Gui.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "Context.h"
#include "Device.h"

namespace vkRender
{
US_VKN;
Gui* Gui::create(const vkRender::Renderer& renderer)
{
    Gui* gui = new (std::nothrow) Gui();
    if (gui && gui->init(renderer))
    {
        // Context::getInstance()->addReleaseObj(gui);
        return gui;
    }
    return nullptr;
}

void Gui::init(GLFWwindow* window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForVulkan(window, true);
}

bool Gui::init(const vkRender::Renderer& renderer)
{
    ImGui::StyleColorsDark();
    
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    auto device = Device::Instance();
    
    ImGui_ImplVulkan_InitInfo info = {};
    info.Instance = Context::getInstance()->getVkInstance();
    info.PhysicalDevice = device->getGPU();
    info.Device = device->getDevice();
    info.QueueFamily = device->indices_.graphicsFamily.value();
    info.Queue = device->graphicsQueue;
    info.PipelineCache = device->getPipelineCache();
    info.DescriptorPoolSize = 9;
    info.RenderPass = renderer.getSwapchain()->getRenderPass();
    info.Subpass = 0;
    info.MinImageCount = renderer.getSwapchain()->info.imageCount;
    info.ImageCount = renderer.getSwapchain()->info.imageCount;
    info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    info.Allocator = nullptr;
    ImGui_ImplVulkan_Init(&info);
    
    return true;
}

void Gui::release() const
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void Gui::Renderer(const CommandBuffer& cmdBuf)
{
    // Start the Dear ImGui frame
    ImGui_ImplGlfw_NewFrame();
    ImGui_ImplVulkan_NewFrame();
    ImGui::NewFrame();

    OnGui();
    
    ImGui::Render();
    ImDrawData* draw_data = ImGui::GetDrawData();
    const bool is_minimized = draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f;
    if (!is_minimized) {
        ImGui_ImplVulkan_RenderDrawData(draw_data, cmdBuf);
    }
}

Gui::Gui()
{}

Gui::~Gui()
{}
}
