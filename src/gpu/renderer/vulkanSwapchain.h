#ifndef vulkanSwapchain_h
#define vulkanSwapchain_h

#include "gpu/vulkanInstance.h"

class Swapchain {
public:
	Swapchain(VkSurfaceKHR surface, std::pair<uint32_t, uint32_t> size);
	~Swapchain();

	void createFramebuffers(VkRenderPass renderPass);
	void recreate(VkSurfaceKHR surface, std::pair<uint32_t, uint32_t> size);

	inline vkb::Swapchain& getVkbSwapchain() { return swapchain; }
	inline std::vector<VkFramebuffer>& getFramebuffers() { return framebuffers; }

private:
	void createSwapchain(VkSurfaceKHR surface, std::pair<uint32_t, uint32_t> size, bool useOld);
	void destroyExtraShit();
	
private:
	vkb::Swapchain swapchain;
	std::vector<VkFramebuffer> framebuffers;
	std::vector<VkImageView> imageViews;
};

#endif
