#include "tr_vertex_formats.h"

#include "tr_gpu.h"
#include "tr_local.h"

#include <cstdint>
#include <vulkan/vulkan.h>

namespace {
struct SingleTextureVertex
{
	vec4_t position;
	vec2_t texcoord;
	byte color[4];
	uint32_t pad0;
};

struct MultiTextureVertex
{
	vec4_t position;
	vec2_t texcoord0;
	vec2_t texcoord1;
	byte color[4];
};
} // namespace

constexpr std::array<VkVertexInputAttributeDescription, 3>
    SingleTextureVertexFormat::vertexAttributes;
constexpr size_t SingleTextureVertexFormat::vertexSize;

constexpr std::array<VkVertexInputAttributeDescription, 4>
    MultiTextureVertexFormat::vertexAttributes;
constexpr size_t MultiTextureVertexFormat::vertexSize;

VkDeviceSize UploadSingleTextureVertexData(
    GpuSwapchainResources* swapchainResources, int numVertexes)
{
    static_assert(
        sizeof(SingleTextureVertex) == SingleTextureVertexFormat::vertexSize,
        "vertex must match SingleTextureVertexFormat::vertexSize");

    VkDeviceSize vertexOffset = GetBufferOffset(
        swapchainResources->vertexBuffer->base,
        swapchainResources->vertexBuffer->data);

    const size_t dataSize = sizeof(SingleTextureVertex) * numVertexes;

    // NOTE: How can this be done better? I don't really want to copy/paste this
    // code into every vertex format writer
    if ((vertexOffset + dataSize) > swapchainResources->vertexBuffer->size)
    {
        backEndData->maxVertexBufferSize *= 2;
        swapchainResources->vertexBuffer = GpuGetTransientVertexBuffer(
            gpuContext.transientBuffers, backEndData->maxVertexBufferSize);

        vertexOffset = 0;
    }

    auto* out = static_cast<SingleTextureVertex*>(
        swapchainResources->vertexBuffer->data);
    for (int i = 0; i < numVertexes; ++i)
    {
        SingleTextureVertex* v = out + i;
        v->position[0] = tess.xyz[i][0];
        v->position[1] = tess.xyz[i][1];
        v->position[2] = tess.xyz[i][2];
        v->position[3] = 1.0f;

        v->texcoord[0] = tess.svars.texcoords[0][i][0];
        v->texcoord[1] = tess.svars.texcoords[0][i][1];

        v->color[0] = tess.svars.colors[i][0];
        v->color[1] = tess.svars.colors[i][1];
        v->color[2] = tess.svars.colors[i][2];
        v->color[3] = tess.svars.colors[i][3];
    }

    swapchainResources->vertexBuffer->data = out + numVertexes;

    return vertexOffset;
}

VkDeviceSize UploadMultiTextureVertexData(
    GpuSwapchainResources* swapchainResources, int numVertexes)
{
    static_assert(
        sizeof(MultiTextureVertex) == MultiTextureVertexFormat::vertexSize,
        "vertex must match MultiTextureVertexFormat::vertexSize");

    VkDeviceSize vertexOffset = GetBufferOffset(
        swapchainResources->vertexBuffer->base,
        swapchainResources->vertexBuffer->data);

    const size_t dataSize = sizeof(MultiTextureVertex) * numVertexes;

    if ((vertexOffset + dataSize) > swapchainResources->vertexBuffer->size)
    {
        backEndData->maxVertexBufferSize *= 2;
        swapchainResources->vertexBuffer = GpuGetTransientVertexBuffer(
            gpuContext.transientBuffers, backEndData->maxVertexBufferSize);

        vertexOffset = 0;
    }

    auto* out = static_cast<MultiTextureVertex*>(
        swapchainResources->vertexBuffer->data);
    for (int i = 0; i < numVertexes; ++i)
    {
        MultiTextureVertex* v = out + i;
        v->position[0] = tess.xyz[i][0];
        v->position[1] = tess.xyz[i][1];
        v->position[2] = tess.xyz[i][2];
        v->position[3] = 1.0f;

        v->texcoord0[0] = tess.svars.texcoords[0][i][0];
        v->texcoord0[1] = tess.svars.texcoords[0][i][1];
        v->texcoord1[0] = tess.svars.texcoords[1][i][0];
        v->texcoord1[1] = tess.svars.texcoords[1][i][1];

        v->color[0] = tess.svars.colors[i][0];
        v->color[1] = tess.svars.colors[i][1];
        v->color[2] = tess.svars.colors[i][2];
        v->color[3] = tess.svars.colors[i][3];
    }

    swapchainResources->vertexBuffer->data = out + numVertexes;

    return vertexOffset;
}