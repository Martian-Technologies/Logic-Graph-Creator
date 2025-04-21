#include "viewportRenderer.h"

ViewportRenderer::ViewportRenderer(VkRenderPass renderPass)
	: chunkRenderer(renderPass) {
	
}

void ViewportRenderer::render(VulkanFrameData& frame, ViewportRenderInterface* viewport) {
	// get view data
	ViewportViewData viewData = viewport->getViewData();

	// render subrenderers
	chunkRenderer.render(frame, viewData.viewport, viewData.viewportViewMat, viewport->getChunker().getAllocations(viewData.viewBounds.first.snap(), viewData.viewBounds.second.snap()));
}
