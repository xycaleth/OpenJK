#pragma once

#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"

#include <array>
#include <map>
#include <vector>

enum DescriptorSetId
{
	DESCRIPTOR_SET_SINGLE_TEXTURE,
	DESCRIPTOR_SET_MULTI_TEXTURE,

	DESCRIPTOR_SET_COUNT
};

#define BIT(n) (1u << (n))
enum VertexAttributeBit
{
	VERTEX_ATTRIBUTE_POSITION_BIT = BIT(0),
	VERTEX_ATTRIBUTE_TEXCOORD0_BIT = BIT(1),
	VERTEX_ATTRIBUTE_TEXCOORD1_BIT = BIT(2),
	VERTEX_ATTRIBUTE_COLOR_BIT = BIT(3),
};
using VertexAttributeBits = uint32_t;

#undef BIT

struct RenderState
{
	uint32_t stateBits;
	uint32_t stateBits2;
	VertexAttributeBits attributes;
	bool multitexture;
};
bool operator<(const RenderState& lhs, const RenderState& rhs);

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

	std::map<RenderState, VkPipeline> graphicsPipelines;
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

typedef struct shaderStage_s shaderStage_t;
VkDescriptorSet GpuCreateDescriptorSet(GpuContext& context, const shaderStage_t* stage);