#include "Shader.h"
#include "Device.h"

namespace vkRender
{
US_VKN;
PipelineShaderStageCreateInfo Shader::createShader(const std::string &fileName, ShaderStageFlagBits shaderType)
{
    auto code = readFile(fileName);

    PipelineShaderStageCreateInfo shaderCreateInfo;
    ShaderModuleCreateInfo createInfo;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    ShaderModule shaderModule = Device::getInstance()->getDevice().createShaderModule(createInfo);

    shaderCreateInfo
        .setModule(shaderModule)
        .setStage(shaderType)
        .setPName("main");
    
    return shaderCreateInfo;
}

Shader* Shader::create(const std::string& vertFile, const std::string& fragFile)
{
    Shader *shader = new (std::nothrow) Shader();
    if (shader && shader->init(vertFile, fragFile))
    {
        return shader;
    }
    return nullptr;
}

bool Shader::init(const std::string& vertFile, const std::string& fragFile)
{
    vert = createShader(vertFile, ShaderStageFlagBits::eVertex);
    frag = createShader(fragFile, ShaderStageFlagBits::eFragment);

    createSetLayout();
    
    
    return true;
}

void Shader::release() const
{
    Device::getInstance()->getDevice().destroyShaderModule(vert.module);
    Device::getInstance()->getDevice().destroyShaderModule(frag.module);
    Device::getInstance()->getDevice().destroyDescriptorSetLayout(setLayout_);
}

void Shader::createSetLayout()
{
    std::array binds = {
        DescriptorSetLayoutBinding {0, DescriptorType::eUniformBuffer, 1, ShaderStageFlagBits::eVertex},            // Uniform Buffer Binding
        DescriptorSetLayoutBinding {1, DescriptorType::eCombinedImageSampler, 1, ShaderStageFlagBits::eFragment}    // sampler Binding
    };
    
    DescriptorSetLayoutCreateInfo createInfo;
    createInfo.setBindings(binds);

    setLayout_ = Device::getInstance()->getDevice().createDescriptorSetLayout(createInfo);
}

// std::vector<uByte> Shader::compileShader(const std::string &code, shaderc_shader_kind shaderKind)
// {
//     shaderc::Compiler compiler;
//     shaderc::CompileOptions options;
//
//     auto result = compiler.CompileGlslToSpv(code.c_str(), code.size(), shaderKind, "shader", options);
//     if (result.GetCompilationStatus() != shaderc_compilation_status_success)
//     {
//         return {};
//     }
//
//     return {result.cbegin(), result.cend()};
// }
}
