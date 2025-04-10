#ifndef subrendererManager
#define subrendererManager

#include <vulkan/vulkan.h>

#include "gpu/renderer/vulkanSwapchain.h"
#include "vulkanDescriptor.h"
#include "rmlRenderer.h"

class SubrendererManager {
public:
	SubrendererManager(Swapchain* swapchain);
	~SubrendererManager();
	
	void renderCommandBuffer(VulkanFrameData& frame, uint32_t imageIndex);

	inline VkRenderPass getRenderPass() { return renderPass; }
	inline RmlRenderer& getRmlRenderer() { return *rmlRenderer; }

private:
	void createRenderPass();
	
private:
	// big data
	DescriptorAllocator descriptorAllocator;
	VkRenderPass renderPass;
	VkDescriptorSetLayout viewDataLayout;

	// subrenderers
	std::unique_ptr<RmlRenderer> rmlRenderer = nullptr;

	// references
	Swapchain* swapchain = nullptr;
};

#endif
