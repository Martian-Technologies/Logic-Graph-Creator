#ifndef vulkanSwapchain_h
#define vulkanSwapchain_h

#include <vulkan/vulkan.h>

struct SwapchainData {
	VkSwapchainKHR handle;
	VkFormat imageFormat;
	VkExtent2D extent;
	std::vector<VkImage> images;
	std::vector<VkImageView> imageViews;
	std::vector<VkFramebuffer> framebuffers;
};

SwapchainData createSwapchain(VkSurfaceKHR surface, int windowWidth, int windowHeight);
void createSwapchainFramebuffers(SwapchainData& swapchain, VkRenderPass renderPass);
void destroySwapchain(SwapchainData& swapchain);

VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, int realWidth, int realHeight);

#endif
