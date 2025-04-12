#include "viewportRenderInterface.h"

#include "gpu/renderer/windowRenderingManager.h"

ViewportRenderInterface::ViewportRenderInterface(WindowRenderingManager* windowRenderer, Rml::Element* element)
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
	// Get viewport size
	VkViewport viewport;
	viewport.x = element->GetAbsoluteOffset().x;
	viewport.y = element->GetAbsoluteOffset().y;
	viewport.width = element->GetBox().GetSize().x;
	viewport.height = element->GetBox().GetSize().y;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	
	// logInfo("({}, {})", "Vulkan", element->GetBox().GetSize().x, element->GetBox().GetSize().y);
}


void ViewportRenderInterface::setCircuit(Circuit* circuit) {
	
}
void ViewportRenderInterface::setEvaluator(Evaluator* evaluator) {
	
}
void ViewportRenderInterface::updateView(ViewManager* viewManager) {
	logInfo("update view");
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

