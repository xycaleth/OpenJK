#pragma once

#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"

#include <array>
#include <unordered_map>
#include <vector>

#include "qcommon/q_math.h"
#include "tr_vertex_formats.h"

struct BspVertex
{
	vec4_t position;
	vec2_t texcoord;
	byte color[4];
	uint32_t pad0;
};

enum DescriptorSetId
{
	DESCRIPTOR_SET_SINGLE_TEXTURE,
	DESCRIPTOR_SET_MULTI_TEXTURE,

	DESCRIPTOR_SET_COUNT
};

struct RenderState
{
    VkShaderModule vertexShader = VK_NULL_HANDLE;
    VkShaderModule fragmentShader = VK_NULL_HANDLE;
    const VkVertexInputAttributeDescription* vertexAttributes = nullptr;
    uint32_t vertexAttributeCount = 0;
    uint32_t vertexSize = 0;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    uint32_t stateBits = 0;
    uint32_t stateBits2 = 0;
};
bool operator==(const RenderState& lhs, const RenderState& rhs);

namespace std {
template <> struct hash<RenderState>
{
    std::size_t operator()(const RenderState& renderState) const
    {
        // MurmurHash 3 32-bit implementation

        // Supposed to be a seed but we don't care about randomness
        uint32_t h = 0xdeadbeef;
        uint32_t k = 0;

        const size_t len = sizeof(renderState);
        const uint8_t* key = reinterpret_cast<const uint8_t*>(&renderState);
        for (size_t i = len >> 2; i; i--)
        {
            memcpy(&k, key, sizeof(uint32_t));
            key += sizeof(uint32_t);
            h ^= scramble(k);
            h = (h << 13) | (h >> 19);
            h = h * 5 + 0xe6546b64;
        }

        k = 0;
        for (size_t i = len & 3; i; i--)
        {
            k <<= 8;
            k |= key[i - 1];
        }

        h ^= scramble(k);
        h ^= len;
        h ^= h >> 16;
        h *= 0x85ebca6b;
        h ^= h >> 13;
        h *= 0xc2b2ae35;
        h ^= h >> 16;
        return h;
    }

  private:
    uint32_t scramble(uint32_t k) const
    {
        k *= 0xcc9e2d51;
        k = (k << 15) | (k >> 17);
        k *= 0x1b873593;
        return k;
    }
};
} // namespace std

struct GpuQueue
{
	uint32_t queueFamily = -1;
	VkQueue queue;
};

struct GpuSwapchainResources
{
	VkImage image;
	VkImageView imageView;
	VkFramebuffer framebuffer;
	VkCommandBuffer gfxCommandBuffer;
	VkDescriptorPool descriptorPool;

	VkBuffer vertexBuffer;
	VmaAllocation vertexBufferAllocation;
	uint32_t vertexBufferOffset;
	void *vertexBufferBase;
	void *vertexBufferData;

	VkBuffer indexBuffer;
	VmaAllocation indexBufferAllocation;
	uint32_t indexBufferOffset;
	void *indexBufferBase;
	void *indexBufferData;
};

struct GpuSwapchain
{
	VkSwapchainKHR swapchain;
	VkFormat surfaceFormat;
	uint32_t width;
	uint32_t height;

	std::vector<GpuSwapchainResources> resources;
};

// How many frames we want to be able to have in the pipeline at any one time
// If we are about to start more than this many frames, we have to wait for
// a frame to finish rendering before proceeding.
const int MAX_FRAMES_IN_FLIGHT = 3;
struct FrameResources
{
	VkFence frameExecutedFence;
	VkSemaphore imageAvailableSemaphore;
	VkSemaphore renderFinishedSemaphore;
};

struct GpuContext
{
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugUtilsMessenger;

	VkPhysicalDevice physicalDevice;
	char physicalDeviceName[VK_MAX_PHYSICAL_DEVICE_NAME_SIZE];
	VkDevice device;
	VkSurfaceKHR windowSurface;

	VmaAllocator allocator;

	GpuSwapchain swapchain;
	std::array<FrameResources, MAX_FRAMES_IN_FLIGHT> frameResources;
	VkFormat depthStencilFormat;

	GpuQueue graphicsQueue;
	GpuQueue computeQueue;
	GpuQueue transferQueue;
	GpuQueue presentQueue;

	VkImage depthImage;
	VkImageView depthImageView;
	VmaAllocation depthImageAllocation;

	VkDescriptorPool globalDescriptorPool;
	VkCommandPool transferCommandPool;
	VkCommandPool gfxCommandPool;
	VkRenderPass renderPass;

	std::array<VkDescriptorSetLayout, DESCRIPTOR_SET_COUNT> descriptorSetLayouts;
	std::array<VkPipelineLayout, DESCRIPTOR_SET_COUNT> pipelineLayouts;

	std::unordered_map<RenderState, VkPipeline> graphicsPipelines;
	std::vector<VkDescriptorSet> descriptorSets;
};

void GpuContextInit(GpuContext& context);
void GpuContextPreShutdown(GpuContext& context);
void GpuContextShutdown(GpuContext& context);

VkShaderModule GpuCreateShaderModuleFromFile(
	GpuContext& context, const char *filePath);
void GpuDestroyShaderModule(GpuContext& context, VkShaderModule shaderModule);
VkPipeline GpuGetGraphicsPipelineForRenderState(
	GpuContext& context, const RenderState& renderState);

typedef struct image_s image_t;
typedef struct shaderStage_s shaderStage_t;
VkDescriptorSet GpuCreateDescriptorSet(GpuContext& context, const image_t* image);
VkDescriptorSet GpuCreateMultitextureDescriptorSet(GpuContext& context, const shaderStage_t *stage);

VkDeviceSize GetBufferOffset(const void* base, const void* pointer);