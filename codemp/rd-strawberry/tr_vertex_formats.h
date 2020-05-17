#pragma once

#include <array>
#include <vulkan/vulkan.h>

struct SingleTextureVertexFormat
{
    static constexpr std::array<VkVertexInputAttributeDescription, 3> vertexAttributes = {
        {
            // position
            {0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, 0},

            // texcoord0
            {1, 0, VK_FORMAT_R32G32_SFLOAT, 16},

            // color
            {2, 0, VK_FORMAT_R8G8B8A8_UNORM, 24},
        }
    };

    static constexpr size_t vertexSize = 32;
};

struct MultiTextureVertexFormat
{
    static constexpr std::array<VkVertexInputAttributeDescription, 4> vertexAttributes = {
        {
            // position
            {0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, 0},

            // texcoord0
            {1, 0, VK_FORMAT_R32G32_SFLOAT, 16},

            // texcoord1
            {2, 0, VK_FORMAT_R32G32_SFLOAT, 24},

            // color
            {3, 0, VK_FORMAT_R8G8B8A8_UNORM, 32},
        }
    };

    static constexpr size_t vertexSize = 36;
};