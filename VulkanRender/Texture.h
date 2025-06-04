#pragma once

#include "vkRender.h"

namespace vkRender
{
class Texture
{
public:
    static Texture *createFormFile(const std::string &fileName);

    bool init(const std::string& fileName);

    void release();
private:
    int width_ = 0;
    int height_ = 0;
    int TexChannels = 0;
    
    uByte *data_ = nullptr;

    vk::Image texture_;

    void createImage();
    bool allocateMemory();
};
}
