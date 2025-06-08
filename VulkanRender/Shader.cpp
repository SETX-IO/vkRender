#include "Shader.h"
#include "Device.h"

namespace vkRender
{
US_VKN;
PipelineShaderStageCreateInfo Shader::create(const std::string &fileName, ShaderStageFlagBits shaderType)
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
