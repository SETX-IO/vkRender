#pragma once

#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "glm/gtc/matrix_transform.hpp"
#include <vulkan/vulkan.hpp>
#include <functional>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <optional>
#include <chrono>
#include <stack>
#include <new>
#include <map>
#include <set>

#define US_VKN using namespace vk

namespace vkRender {}

typedef unsigned char uByte;
struct UniformObj
{
    glm::mat4 module;
    glm::mat4 view;
    glm::mat4 proj;
};

constexpr int MAX_FRAME_IN_FLIGHT = 3;