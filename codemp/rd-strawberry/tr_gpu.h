#include <vulkan/vulkan.h>

#include <vector>

struct GpuQueue
{
	uint32_t queueFamily = -1;
	VkQueue queue;
};

struct GpuSwapchainResources
{
	VkImage image;

	VkSemaphore imageAvailableSemaphore;
	VkImageView imageView;
	VkFramebuffer framebuffer;

	VkCommandBuffer gfxCommandBuffer;
};

struct GpuSwapchain
{
	VkSwapchainKHR swapchain;
	VkFormat surfaceFormat;
	uint32_t width;
	uint32_t height;

	std::vector<GpuSwapchainResources> swapchainResources;

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
	VkRenderPass renderPass;
};
