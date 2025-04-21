#include "viewportRenderer.h"

ViewportRenderer::ViewportRenderer(VkRenderPass renderPass)
	: gridRenderer(renderPass), chunkRenderer(renderPass) {
	
}

void ViewportRenderer::render(VulkanFrameData& frame, ViewportRenderInterface* viewport) {
	// get view data
	ViewportViewData viewData = viewport->getViewData();

	// set dynamic state
	vkCmdSetViewport(frame.getMainCommandBuffer(), 0, 1, &viewData.viewport);
	VkRect2D scissor{};
	scissor.offset = {static_cast<int32_t>(viewData.viewport.x), static_cast<int32_t>(viewData.viewport.y)};
	scissor.extent = {static_cast<uint32_t>(viewData.viewport.width), static_cast<uint32_t>(viewData.viewport.height)};
	vkCmdSetScissor(frame.getMainCommandBuffer(), 0, 1, &scissor);

	// render subrenderers
	gridRenderer.render(frame, viewData.viewportViewMat, viewData.viewScale);
	chunkRenderer.render(frame, viewData.viewportViewMat, viewport->getChunker().getAllocations(viewData.viewBounds.first.snap(), viewData.viewBounds.second.snap()));
}
