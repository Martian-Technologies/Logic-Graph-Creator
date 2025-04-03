#ifndef vulkanSwapchain_h
#define vulkanSwapchain_h

#include "gpu/vulkanInstance.h"

class Swapchain {
public:
	Swapchain(VkSurfaceKHR surface);
	~Swapchain();

	void createFramebuffers(VkRenderPass renderPass);
	void recreate();

	inline vkb::Swapchain& getVkbSwapchain() { return swapchain; }
	
private:
	vkb::Swapchain swapchain;
	std::vector<VkFramebuffer> framebuffers;
	std::vector<VkImageView> imageViews;
};

#endif
