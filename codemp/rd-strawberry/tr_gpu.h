#pragma once

#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"

#include <array>
#include <vector>

struct GpuQueue
{
	uint32_t queueFamily = -1;
	VkQueue queue;
};

struct GpuSwapchain
{
	struct Resources
	{
		VkImage image;
		VkImageView imageView;
		VkFramebuffer framebuffer;
		VkCommandBuffer gfxCommandBuffer;
	};

	VkSwapchainKHR swapchain;
	VkFormat surfaceFormat;
	uint32_t width;
	uint32_t height;

	std::vector<Resources> resources;
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

	GpuQueue graphicsQueue;
	GpuQueue computeQueue;
	GpuQueue transferQueue;
	GpuQueue presentQueue;

	VkCommandPool transferCommandPool;
	VkCommandPool gfxCommandPool;
	VkRenderPass renderPass;
};

void GpuContextInit(GpuContext& context);
void GpuContextPreShutdown(GpuContext& context);
void GpuContextShutdown(GpuContext& context);
