#include "viewportRenderInterface.h"

#include <glm/ext/matrix_clip_space.hpp>

#include "gpu/renderer/windowRenderer.h"

ViewportRenderInterface::ViewportRenderInterface(WindowRenderer* windowRenderer, Rml::Element* element)
	: windowRenderer(windowRenderer), element(element) {

	windowRenderer->registerViewportRenderInterface(this);
}

ViewportRenderInterface::~ViewportRenderInterface() {
	windowRenderer->deregisterViewportRenderInterface(this);
}


void ViewportRenderInterface::initializeVulkan(VkRenderPass renderPass) {
	chunkRenderer = std::make_unique<VulkanChunkRenderer>(renderPass);
}

void ViewportRenderInterface::render(VulkanFrameData& frame) {

	// logInfo("({}, {})", "Vulkan", element->GetBox().GetSize().x, element->GetBox().GetSize().y);
}


void ViewportRenderInterface::setCircuit(Circuit* circuit) {
	
}
void ViewportRenderInterface::setEvaluator(Evaluator* evaluator) {
	
}
void ViewportRenderInterface::updateView(ViewManager* viewManager) {
	std::lock_guard<std::mutex> lock(viewMux);
	
	// Update vulkan viewport
	viewData.viewport.x = element->GetAbsoluteOffset().x;
	viewData.viewport.y = element->GetAbsoluteOffset().y;
	viewData.viewport.width = element->GetBox().GetSize().x;
	viewData.viewport.height = element->GetBox().GetSize().y;
	viewData.viewport.minDepth = 0.0f;
	viewData.viewport.maxDepth = 1.0f;

	// Create view mat
	FPosition topLeft = viewManager->getTopLeft();
	FPosition bottomRight = viewManager->getBottomRight();
	viewData.viewportViewMat = glm::ortho(topLeft.x, bottomRight.x, topLeft.y, bottomRight.y);
	viewData.viewBounds = { topLeft, bottomRight };
}
void ViewportRenderInterface::updateCircuit(DifferenceSharedPtr diff) {
	
}
float ViewportRenderInterface::getLastFrameTimeMs() const {
	return 0.0f;
}
ElementID ViewportRenderInterface::addSelectionElement(const SelectionObjectElement& selection) {
	return 0;
}
ElementID ViewportRenderInterface::addSelectionElement(const SelectionElement& selection) {
	return 0;
}
void ViewportRenderInterface::removeSelectionElement(ElementID selection) {
	
}
ElementID ViewportRenderInterface::addBlockPreview(const BlockPreview& blockPreview) {
	return 0;
}
void ViewportRenderInterface::removeBlockPreview(ElementID blockPreview) {
	
}
ElementID ViewportRenderInterface::addConnectionPreview(const ConnectionPreview& connectionPreview) {
	return 0;
}
void ViewportRenderInterface::removeConnectionPreview(ElementID connectionPreview) {
	
}
ElementID ViewportRenderInterface::addHalfConnectionPreview(const HalfConnectionPreview& halfConnectionPreview) {
	return 0;
}
void ViewportRenderInterface::removeHalfConnectionPreview(ElementID halfConnectionPreview) {
	
}
void ViewportRenderInterface::spawnConfetti(FPosition start) {

}

