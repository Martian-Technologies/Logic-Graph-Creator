#include "vulkanRenderer.h"

#include "computerAPI/fileLoader.h"
#include "gpu/vulkanManager.h"
#include "gpu/vulkanUtil.h"
#include <glm/gtc/matrix_transform.hpp>

void VulkanRenderer::initialize(VkSurfaceKHR surface, int w, int h)
{
	this->surface = surface;
	windowWidth = w;
	windowHeight = h;

	// set up swapchain
	swapchain = createSwapchain(surface, windowWidth, windowHeight);
	createFrameDatas(frames, FRAME_OVERLAP);

	// create pipeline
	vertShader = createShaderModule(readFileAsBytes(":/shaders/shader.vert.spv"));
	fragShader = createShaderModule(readFileAsBytes(":/shaders/shader.frag.spv"));
	pipeline = createPipeline(swapchain, vertShader, fragShader);

	// create framebuffer
	createSwapchainFramebuffers(swapchain, pipeline.renderPass);

	initialized = true;
	logInfo("Renderer initialized", "Vulkan");
}

void VulkanRenderer::setCircuit(Circuit* circuit) {
	// lock rendering mutex
	std::lock_guard<std::mutex> guard(cpuRenderingMutex);

	circuitBufferRing.setCircuit(circuit);
	
	if (circuit) {
		logInfo("Renderer circuit assigned and setup", "Vulkan");
	}
	else {
		logInfo("Renderer circuit set to nothing", "Vulkan");
	}
}

void VulkanRenderer::destroy() {
	stop();

	circuitBufferRing.destroy();
	destroySwapchain(swapchain);
	destroyFrameDatas(frames, FRAME_OVERLAP);
	destroyShaderModule(vertShader);
	destroyShaderModule(fragShader);
	destroyPipeline(pipeline);
}

void VulkanRenderer::resize(int w, int h) {
	if (!initialized) return;
	
	// lock rendering mutex
	std::lock_guard<std::mutex> guard(cpuRenderingMutex);
	
	windowWidth = w;
	windowHeight = h;

	handleResize();
}

void VulkanRenderer::setEvaluator(Evaluator* evaluator) {
	
}

void VulkanRenderer::updateView(ViewManager* viewManager) {
	// lock rendering mutex
	std::lock_guard<std::mutex> guard(cpuRenderingMutex);
	
	FPosition topLeft = viewManager->getTopLeft();
    FPosition bottomRight = viewManager->getBottomRight();
	// this function was designed for a slightly different coordinate system so it's a little wonky, but it works
	orthoMat = glm::ortho(topLeft.x, bottomRight.x, topLeft.y, bottomRight.y);
}

void VulkanRenderer::updateCircuit(DifferenceSharedPtr diff) {
	// lock rendering mutex
	std::lock_guard<std::mutex> guard(cpuRenderingMutex);
	
	circuitBufferRing.updateCircuit(diff);
}

// TODO - fix ghetto render thread
void VulkanRenderer::run() {
	if (!initialized) return;
	if (running) return;
	
	running = true;
	renderThread = std::thread(&VulkanRenderer::renderLoop, this);
	logInfo("Renderer started", "Vulkan");
}

void VulkanRenderer::stop() {
	if (!running) return;
	
	running = false;
	if (renderThread.joinable()) renderThread.join();
	
	logInfo("Renderer stopped", "Vulkan");
}

void VulkanRenderer::renderLoop() {
	while(running) {
		// get next frame
		FrameData& frame = getCurrentFrame();		
		// wait for frame end
		vkWaitForFences(Vulkan::getDevice(), 1, &frame.renderFence, VK_TRUE, UINT64_MAX);

		// lock rendering mutex
		std::lock_guard<std::mutex> guard(cpuRenderingMutex);

		VkQueue& graphicsQueue = Vulkan::getSingleton().requestGraphicsQueue();
		VkQueue& presentQueue = Vulkan::getSingleton().requestPresentQueue();

		// get next swapchain image to render to
		uint32_t imageIndex;
		VkResult imageGetResult = vkAcquireNextImageKHR(Vulkan::getDevice(), swapchain.handle, UINT64_MAX, frame.swapchainSemaphore, VK_NULL_HANDLE, &imageIndex);
		if (imageGetResult == VK_ERROR_OUT_OF_DATE_KHR) {
			// wait for a resize event
			continue;
		} else if (imageGetResult != VK_SUCCESS && imageGetResult != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		// reset render fence
		vkResetFences(Vulkan::getDevice(), 1, &frame.renderFence);

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
		if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, frame.renderFence) != VK_SUCCESS) {
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

		vkQueuePresentKHR(presentQueue, &presentInfo);

		//increase the number of frames drawn
		++frameNumber;
	}

	vkDeviceWaitIdle(Vulkan::getDevice());
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

	VkClearValue clearColor = {{{0.93f, 0.93f, 0.93f, 1.0f}}};
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;
	
	vkCmdBeginRenderPass(frame.mainCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	// only draw with pipeline if we have a circuit (vertex buffer)
	if (circuitBufferRing.hasCircuit()) {
		// bind render pipeline
		vkCmdBindPipeline(frame.mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.handle);

		// bind push constants
		VertexPushConstants pushConstants{};
		pushConstants.mvp = orthoMat;
		vkCmdPushConstants(frame.mainCommandBuffer, pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(VertexPushConstants), &pushConstants);

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

		// bind vertex buffers
		const CircuitBuffer& circuitBuffer = circuitBufferRing.getAvaiableBuffer();
		VkBuffer vertexBuffers[] = { circuitBuffer.blockBuffer.buffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(frame.mainCommandBuffer, 0, 1, vertexBuffers, offsets);

		// draw
		// TODO - something else should (maybe) be in charge of making draw calls here?
		vkCmdDraw(frame.mainCommandBuffer, static_cast<uint32_t>(circuitBuffer.numBlockVertices), 1, 0, 0);
	}

	// end
	vkCmdEndRenderPass(frame.mainCommandBuffer);
	if (vkEndCommandBuffer(frame.mainCommandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer!");
	}
}

void VulkanRenderer::handleResize() {
	vkDeviceWaitIdle(Vulkan::getDevice());

	destroySwapchain(swapchain);
	swapchain = createSwapchain(surface, windowWidth, windowHeight);
	createSwapchainFramebuffers(swapchain, pipeline.renderPass);
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

