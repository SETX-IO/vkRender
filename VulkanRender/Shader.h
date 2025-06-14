#pragma once

#include "vkRender.h"
#include "shaderc/shaderc.hpp"

namespace vkRender
{
static std::vector<char> readFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::binary | std::ios::ate);

    if (!file.is_open())
    {
        std::cout << "not fount File: " << filename << std::endl;
        return {};
    }

    size_t fileSize = file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

class Shader
{
public:
    static vk::PipelineShaderStageCreateInfo createShader(const std::string &fileName, vk::ShaderStageFlagBits shaderType);
    static Shader *create(const std::string &vertFile, const std::string &fragFile);

    bool init(const std::string& vertFile, const std::string& fragFile);

    void release() const;

    const vk::DescriptorSetLayout& getSetLayout() const {return setLayout_;}
    
    vk::PipelineShaderStageCreateInfo vert;
    vk::PipelineShaderStageCreateInfo frag;
private:
    vk::DescriptorSetLayout setLayout_;

    void createSetLayout();
    
    // static std::vector<uByte> compileShader(const std::string &code, shaderc_shader_kind shaderKind);
};
}
