#include "viewportRenderInterface.h"

#include <glm/ext/matrix_clip_space.hpp>

#include "gpu/renderer/windowRenderer.h"

ViewportRenderInterface::ViewportRenderInterface(VulkanDevice* device, Rml::Element* element)
	: element(element), chunker(device) {
}

ViewportRenderInterface::~ViewportRenderInterface() {
	if (linkedWindowRenderer != nullptr) linkedWindowRenderer->deregisterViewportRenderInterface(this);
}


void ViewportRenderInterface::linkToWindowRenderer(WindowRenderer* windowRenderer) {
	linkedWindowRenderer = windowRenderer;
	linkedWindowRenderer->registerViewportRenderInterface(this);
}

ViewportViewData ViewportRenderInterface::getViewData() {
	std::lock_guard<std::mutex> lock(viewMux);
	return viewData;
}

// ====================================== INTERFACE ==========================================

void ViewportRenderInterface::setCircuit(Circuit* circuit) {
	std::lock_guard<std::mutex> lock(circuitMux);
	
	this->circuit = circuit;
	chunker.setCircuit(circuit);
}

void ViewportRenderInterface::setEvaluator(std::shared_ptr<Evaluator> evaluator) {
	std::lock_guard<std::mutex> lock(evaluatorMux);
	this->evaluator = evaluator;
}

void ViewportRenderInterface::setAddress(const Address& address) {
	
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

	viewData.viewScale = viewManager->getViewScale();
}

void ViewportRenderInterface::updateCircuit(DifferenceSharedPtr diff) {
	chunker.updateCircuit(diff);
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
	std::lock_guard<std::mutex> lock(blockPreviewMux);

	ElementID newElement = ++currentElementID;
	BlockPreviewRenderData newPreview;
	newPreview.position = blockPreview.position;
	newPreview.rotation = blockPreview.rotation;
	{
		std::lock_guard<std::mutex> lock(circuitMux);
		newPreview.size = circuit->getBlockContainer()->getBlockDataManager()->getBlockSize(blockPreview.type, blockPreview.rotation);
	}
	newPreview.type = blockPreview.type;

	// insert new block preview into map
	blockPreviews[newElement] = newPreview;
	
	return newElement;
}

void ViewportRenderInterface::removeBlockPreview(ElementID blockPreview) {
	std::lock_guard<std::mutex> lock(blockPreviewMux);

	blockPreviews.erase(blockPreview);
}

std::vector<BlockPreviewRenderData> ViewportRenderInterface::getBlockPreviews() {
	std::lock_guard<std::mutex> lock(blockPreviewMux);

	std::vector<BlockPreviewRenderData> returnBlockPreviews;
	returnBlockPreviews.reserve(blockPreviews.size());

	for (const auto& preview : blockPreviews) {
		returnBlockPreviews.push_back(preview.second);
	}

	return returnBlockPreviews;
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

