#include "vulkanRenderer.h"

#include "gpu/vulkanSwapchain.h"
#include "gpu/vulkanUtil.h"

void VulkanRenderer::initialize(VulkanGraphicsView view, VkSurfaceKHR surface, int w, int h)
{
	this->view = view;
	this->surface = surface;

	windowWidth = w;
	windowHeight = h;

	swapchain = createSwapchain(view, surface, w, h);
	createFrameDatas(view.device, frames, FRAME_OVERLAP, view.queueFamilies.graphicsFamily.value().index );
}

void VulkanRenderer::destroy() {
	destroySwapchain(view, swapchain);
	destroyFrameDatas(view.device, frames, FRAME_OVERLAP);
}

void VulkanRenderer::resize(int w, int h) {
	windowWidth = w;
	windowHeight = h;
}

// TODO - ghetto render thread
void VulkanRenderer::run() {
	running = true;
	renderThread = std::thread(&VulkanRenderer::renderLoop, this);
}
void VulkanRenderer::stop() {
	running = false;
	if (renderThread.joinable()) renderThread.join();
}

void VulkanRenderer::renderLoop() {
	while(running) {
		// wait for frame end
		vkWaitForFences(view.device, 1, &getCurrentFrame().renderFence, true, 1000000000);
		vkResetFences(view.device, 1, &getCurrentFrame().renderFence);
		
		//increase the number of frames drawn
		++frameNumber;

	}

	vkDeviceWaitIdle(view.device);
}

// Vulkan Setup

// INTERFACE
// ======================================================================

void VulkanRenderer::setCircuit(Circuit* circuit) {
	
}

void VulkanRenderer::setEvaluator(Evaluator* evaluator) {
	
}

void VulkanRenderer::updateView(ViewManager* viewManager) {
	
}

void VulkanRenderer::updateCircuit(DifferenceSharedPtr diff) {
	
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

void VulkanRenderer::spawnConfetti(FPosition start) {
	
}

