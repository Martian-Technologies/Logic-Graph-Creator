#include "vulkanRenderer.h"

#include "gpu/vulkanUtil.h"

// obviously todo shader resource passing
void VulkanRenderer::initialize(VulkanGraphicsView view, VkSurfaceKHR surface, int w, int h, std::vector<char> vertCode, std::vector<char> fragCode)
{
	this->view = view;
	this->surface = surface;

	windowWidth = w;
	windowHeight = h;

	// TODO - make all of these vulkan sub functions have uniform syntax (probably after view rework)
	swapchain = createSwapchain(view, surface, w, h);
	createFrameDatas(view.device, frames, FRAME_OVERLAP, view.queueFamilies.graphicsFamily.value().index );

	vertShader = createShaderModule(view.device, vertCode);
	fragShader = createShaderModule(view.device, fragCode);
	pipeline = createPipeline(view.device, swapchain, vertShader, fragShader);
	createSwapchainFramebuffers(view, swapchain, pipeline.renderPass);
}

void VulkanRenderer::destroy() {
	destroySwapchain(view, swapchain);
	destroyFrameDatas(view.device, frames, FRAME_OVERLAP);
	destroyShaderModule(view.device, vertShader);
	destroyShaderModule(view.device, fragShader);
	destroyPipeline(view.device, pipeline);
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
		FrameData& frame = getCurrentFrame();
		// wait for frame end
		vkWaitForFences(view.device, 1, &frame.renderFence, VK_TRUE, UINT64_MAX);
		vkResetFences(view.device, 1, &frame.renderFence);
		
		//increase the number of frames drawn
		++frameNumber;

		// get next swapchain image to render too
		uint32_t imageIndex;
		VkResult imageGetResult = vkAcquireNextImageKHR(view.device, swapchain.handle, UINT64_MAX, frame.swapchainSemaphore, VK_NULL_HANDLE, &imageIndex);
		if (imageGetResult == VK_ERROR_OUT_OF_DATE_KHR) {
			std::cout << "THE IMAGE IS WRONG AAAAAHHH" << std::endl;
		}

		// record command buffer
		vkResetCommandBuffer(frame.mainCommandBuffer, 0);
		recordCommandBuffer(frame, imageIndex);

		// start setting up submission
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		// wait semaphore
		VkSemaphore waitSemaphores[] = { frame.swapchainSemaphore };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		// command buffers
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &frame.mainCommandBuffer;

		// signal semaphores
		VkSemaphore signalSemaphores[] = { frame.renderSemaphore };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		// submit to queue
		if (vkQueueSubmit(view.graphicsQueue, 1, &submitInfo, frame.renderFence) != VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		// present
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;
		VkSwapchainKHR swapChains[] = { swapchain.handle };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr; // unused

		VkResult presentResult = vkQueuePresentKHR(view.presentQueue, &presentInfo);
		if (presentResult == VK_ERROR_OUT_OF_DATE_KHR) {
			std::cout << "ASUDAOIDJWOIDJW" << std::endl;
		}
	}

	vkDeviceWaitIdle(view.device);
}

void VulkanRenderer::recordCommandBuffer(FrameData& frame, uint32_t imageIndex) {
	// start recording
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0; // Optional
	beginInfo.pInheritanceInfo = nullptr; // Optional
	if (vkBeginCommandBuffer(frame.mainCommandBuffer, &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	// begin render pass
	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = pipeline.renderPass;
	renderPassInfo.framebuffer = swapchain.framebuffers[imageIndex];
	renderPassInfo.renderArea.offset = {0, 0};
	renderPassInfo.renderArea.extent = swapchain.extent;

	VkClearValue clearColor = {{{0.0, 0.0f, 0.0f, 1.0f}}};
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;
	
	vkCmdBeginRenderPass(frame.mainCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	// bind render pipeline
	vkCmdBindPipeline(frame.mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.handle);

	// set dynamic state
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(swapchain.extent.width);
	viewport.height = static_cast<float>(swapchain.extent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(frame.mainCommandBuffer, 0, 1, &viewport);
	VkRect2D scissor{};
	scissor.offset = {0, 0};
	scissor.extent = swapchain.extent;
	vkCmdSetScissor(frame.mainCommandBuffer, 0, 1, &scissor);

	// draw
	vkCmdDraw(frame.mainCommandBuffer, 3, 1, 0, 0);

	// end
	vkCmdEndRenderPass(frame.mainCommandBuffer);
	if (vkEndCommandBuffer(frame.mainCommandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer!");
	}

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

