#include "vulkanRenderer.h"

void VulkanRenderer::initialize(VulkanView view, VkSurfaceKHR surface)
{
    this->view = view;
}

void VulkanRenderer::resize(int w, int h) {
    
}

void VulkanRenderer::run() {
    
}

void VulkanRenderer::setBlockContainer(BlockContainerWrapper* blockContainer) {
    
}

void VulkanRenderer::setEvaluator(Evaluator* evaluator) {
    
}

void VulkanRenderer::updateView(ViewManager* viewManager) {
    
}

void VulkanRenderer::updateBlockContainer(DifferenceSharedPtr diff) {
    
}

// elements -----------------------------

ElementID VulkanRenderer::addSelectionElement(const SelectionObjectElement& selection) {
    return 0;
}

ElementID VulkanRenderer::addSelectionElement(const SelectionElement& selection) {
    return 0;
}

void VulkanRenderer::removeSelectionElement(ElementID selection) {
    
}

ElementID VulkanRenderer::addBlockPreview(const BlockPreview& blockPreview) {
    return 0;
}

void VulkanRenderer::removeBlockPreview(ElementID blockPreview) {
    
}

ElementID VulkanRenderer::addConnectionPreview(const ConnectionPreview& connectionPreview) {
    return 0;
}

void VulkanRenderer::removeConnectionPreview(ElementID connectionPreview) {
    
}

ElementID VulkanRenderer::addHalfConnectionPreview(const HalfConnectionPreview& halfConnectionPreview) {
    return 0;
}

void VulkanRenderer::removeHalfConnectionPreview(ElementID halfConnectionPreview) {
    
}

void VulkanRenderer::spawnConfetti(FPosition start) {}

