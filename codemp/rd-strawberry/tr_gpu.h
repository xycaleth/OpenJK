#include <vulkan/vulkan.h>

#include <vector>

struct GpuQueue
{
	uint32_t queueFamily = -1;
	VkQueue queue;
};

struct GpuSwapchain
{
	VkSwapchainKHR swapchain;
	VkFormat surfaceFormat;
	uint32_t width;
	uint32_t height;

	uint32_t imageCount;
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkImage> images;
	std::vector<VkImageView> imageViews;

	int frameIndex;
};

struct GpuContext
{
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugUtilsMessenger;

	VkPhysicalDevice physicalDevice;
	char physicalDeviceName[VK_MAX_PHYSICAL_DEVICE_NAME_SIZE];
	VkDevice device;
	VkSurfaceKHR windowSurface;

	GpuSwapchain swapchain;

	GpuQueue graphicsQueue;
	GpuQueue computeQueue;
	GpuQueue transferQueue;
	GpuQueue presentQueue;

	VkCommandPool transferCommandPool;
	VkCommandPool gfxCommandPool;
};
