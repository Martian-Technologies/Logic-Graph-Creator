#ifndef viewportRenderer_h
#define viewportRenderer_h

#include "gpu/renderer/subrenderers/gridRenderer.h"
#include "gpu/renderer/subrenderers/vulkanChunkRenderer.h"
#include "gpu/renderer/viewportRenderInterface.h"
#include "gpu/renderer/vulkanFrame.h"

class ViewportRenderer {
public:
	ViewportRenderer(VkRenderPass renderPass);

	void render(VulkanFrameData& frame, ViewportRenderInterface* viewport);

private:
	GridRenderer gridRenderer;
	VulkanChunkRenderer chunkRenderer;
};

#endif
