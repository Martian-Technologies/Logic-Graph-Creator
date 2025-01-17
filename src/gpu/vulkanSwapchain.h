#ifndef vulkanSwapchain_h
#define vulkanSwapchain_h

#include <vulkan/vulkan.h>

#include "vulkanManager.h"

struct SwapchainData {
	VkSwapchainKHR handle;
	VkFormat imageFormat;
	VkExtent2D extent;
	std::vector<VkImage> images;
	std::vector<VkImageView> imageViews;
	std::vector<VkFramebuffer> framebuffers;
};

SwapchainData createSwapchain(VulkanGraphicsView view, VkSurfaceKHR surface, int windowWidth, int windowHeight);
void createSwapchainFramebuffers(VulkanGraphicsView view, SwapchainData& swapchain, VkRenderPass renderPass);
void destroySwapchain(VulkanGraphicsView view, SwapchainData& swapchain);

VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, int realWidth, int realHeight);

#endif
