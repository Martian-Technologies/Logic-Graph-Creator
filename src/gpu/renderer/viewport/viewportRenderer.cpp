#include "viewportRenderer.h"

void ViewportRenderer::init(VulkanDevice* device, VkRenderPass renderPass) {
	gridRenderer.init(device, renderPass);
	chunkRenderer.init(device, renderPass);
	elementRenderer.init(device, renderPass);
}

void ViewportRenderer::cleanup() {
	elementRenderer.cleanup();
	chunkRenderer.cleanup();
	gridRenderer.cleanup();
}

void ViewportRenderer::render(Frame& frame, ViewportRenderInterface* viewport) {
	// get view data
	ViewportViewData viewData = viewport->getViewData();

	// set dynamic state
	vkCmdSetViewport(frame.mainCommandBuffer, 0, 1, &viewData.viewport);
	VkRect2D scissor{};
	scissor.offset = {static_cast<int32_t>(viewData.viewport.x), static_cast<int32_t>(viewData.viewport.y)};
	scissor.extent = {static_cast<uint32_t>(viewData.viewport.width), static_cast<uint32_t>(viewData.viewport.height)};
	vkCmdSetScissor(frame.mainCommandBuffer, 0, 1, &scissor);

	// render subrenderers
	gridRenderer.render(frame, viewData.viewportViewMat, viewData.viewScale, viewport->hasCircuit());
	chunkRenderer.render(frame, viewData.viewportViewMat, viewport->getEvaluator(), viewport->getChunker().getAllocations(viewData.viewBounds.first.snap(), viewData.viewBounds.second.snap()));
	elementRenderer.render(frame, viewData.viewportViewMat, viewport->getBlockPreviews(), viewport->getBoxSelections(), viewport->getConnectionPreviews());
}
