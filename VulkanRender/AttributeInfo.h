#pragma once

namespace vk
{
enum class Format;
}

namespace vkRender
{
struct AttributeInfo
{
    unsigned int location;
    unsigned int bind;
    vk::Format format;
    unsigned int size;

    AttributeInfo(const vk::Format format, unsigned int size): location(0), bind(0), format(format), size(size) { }
    AttributeInfo(const unsigned int bind, const vk::Format format, unsigned int size): location(0), bind(bind), format(format), size(size) { }
    AttributeInfo(const unsigned int location, const unsigned int bind, const vk::Format format, unsigned int size): location(location), bind(bind), format(format), size(size) { }
};
}