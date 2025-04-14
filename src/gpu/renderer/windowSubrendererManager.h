#ifndef windowSubRendererManager_h
#define windowSubRendererManager_h

#include <vulkan/vulkan.h>

#include "gpu/renderer/viewportRenderer.h"
#include "gpu/renderer/vulkanSwapchain.h"
#include "vulkanDescriptor.h"
#include "rmlRenderer.h"

class WindowSubrendererManager {
public:
	WindowSubrendererManager(Swapchain* swapchain, std::vector<VulkanFrameData>& frames);
	~WindowSubrendererManager();
	
	void renderCommandBuffer(VulkanFrameData& frame, uint32_t imageIndex);

	// viewport renderers
	void registerViewportRenderInterface(ViewportRenderer* renderInterface);
	void deregisterViewportRenderInterface(ViewportRenderer* renderInterface);

	// getters
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
	std::set<ViewportRenderer*> viewportRenderers;
	std::mutex viewportRenderersMux;

	// references
	Swapchain* swapchain = nullptr;
};

#endif
