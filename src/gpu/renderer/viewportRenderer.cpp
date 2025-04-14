#include "viewportRenderer.h"

#include <glm/ext/matrix_clip_space.hpp>

#include "gpu/renderer/windowRenderer.h"

ViewportRenderer::ViewportRenderer(WindowRenderer* windowRenderer, Rml::Element* element)
	: windowRenderer(windowRenderer), element(element) {

	windowRenderer->registerViewportRenderInterface(this);
}

ViewportRenderer::~ViewportRenderer() {
	windowRenderer->deregisterViewportRenderInterface(this);
	destroyVulkan();
}


void ViewportRenderer::initializeVulkan(VkRenderPass renderPass) {
	chunkRenderer = std::make_unique<VulkanChunkRenderer>(renderPass);
}

void ViewportRenderer::destroyVulkan() {
	chunkRenderer.reset();
}


void ViewportRenderer::render(VulkanFrameData& frame) {

	viewMux.lock();
	ViewportViewData frameViewData = viewData;
	viewMux.unlock();
	
	
	chunkRenderer->render(frame, frameViewData.viewport, frameViewData.viewportViewMat, frameViewData.viewBounds);
	// logInfo("({}, {})", "Vulkan", element->GetBox().GetSize().x, element->GetBox().GetSize().y);
}


void ViewportRenderer::setCircuit(Circuit* circuit) {
	chunkRenderer->setCircuit(circuit);
}

void ViewportRenderer::setEvaluator(Evaluator* evaluator) {
	
}

void ViewportRenderer::updateView(ViewManager* viewManager) {
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

void ViewportRenderer::updateCircuit(DifferenceSharedPtr diff) {
	chunkRenderer->updateCircuit(diff);
}

float ViewportRenderer::getLastFrameTimeMs() const {
	return 0.0f;
}

ElementID ViewportRenderer::addSelectionElement(const SelectionObjectElement& selection) {
	return 0;
}

ElementID ViewportRenderer::addSelectionElement(const SelectionElement& selection) {
	return 0;
}

void ViewportRenderer::removeSelectionElement(ElementID selection) {
	
}

ElementID ViewportRenderer::addBlockPreview(const BlockPreview& blockPreview) {
	return 0;
}

void ViewportRenderer::removeBlockPreview(ElementID blockPreview) {
	
}

ElementID ViewportRenderer::addConnectionPreview(const ConnectionPreview& connectionPreview) {
	return 0;
}

void ViewportRenderer::removeConnectionPreview(ElementID connectionPreview) {
	
}

ElementID ViewportRenderer::addHalfConnectionPreview(const HalfConnectionPreview& halfConnectionPreview) {
	return 0;
}

void ViewportRenderer::removeHalfConnectionPreview(ElementID halfConnectionPreview) {
	
}

void ViewportRenderer::spawnConfetti(FPosition start) {

}

