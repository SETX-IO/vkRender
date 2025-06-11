#pragma once

#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "glm/gtc/matrix_transform.hpp"
#include <vulkan/vulkan.hpp>
#include <functional>
#include <iostream>
#include <fstream>
#include <optional>
#include <chrono>
#include <stack>
#include <new>
#include <map>
#include <set>

#define US_VKN using namespace vk

namespace vkRender {}

typedef unsigned char uByte;

constexpr int MAX_FRAME_IN_FLIGHT = 3;