#ifndef vulkanSwapchain_h
#define vulkanSwapchain_h

#include "gpu/vulkanInstance.h"

class Swapchain {
public:
	Swapchain(VkSurfaceKHR surface, std::pair<uint32_t, uint32_t> size);
	~Swapchain();

	void createFramebuffers(VkRenderPass renderPass);
	void recreate(std::pair<uint32_t, uint32_t> size);

	inline vkb::Swapchain& getVkbSwapchain() { return swapchain; }
	
private:
	vkb::Swapchain swapchain;
	std::vector<VkFramebuffer> framebuffers;
	std::vector<VkImageView> imageViews;
};

#endif
