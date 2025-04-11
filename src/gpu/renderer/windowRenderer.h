#ifndef windowRenderer_h
#define windowRenderer_h

#include <vulkan/vulkan.h>

#include "gpu/renderer/vulkanSwapchain.h"
#include "vulkanDescriptor.h"
#include "rmlRenderer.h"

class WindowRenderer {
public:
	WindowRenderer(Swapchain* swapchain, std::vector<VulkanFrameData>& frames);
	~WindowRenderer();
	
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
