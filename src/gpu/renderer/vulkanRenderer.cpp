#include "vulkanRenderer.h"

#include "gpu/vulkanManager.h"
#include <glm/gtc/matrix_transform.hpp>

void VulkanRenderer::initialize(VkSurfaceKHR surface, int w, int h)
{
	this->surface = surface;
	windowWidth = w;
	windowHeight = h;

	// set up swapchain
	swapchain = createSwapchain(surface, windowWidth, windowHeight);
	createFrameDatas(frames, FRAME_OVERLAP);
	
	// create render pass and framebuffers
	createRenderPass(swapchain);
	createSwapchainFramebuffers(swapchain, renderPass);

	// create pipeline
	blockRenderer.initialize(renderPass);
	
	initialized = true;
	logInfo("Renderer initialized", "Vulkan");
}

void VulkanRenderer::destroy() {
	stop();

	initialized = false;

	destroySwapchain(swapchain);
	destroyFrameDatas(frames, FRAME_OVERLAP);
	blockRenderer.destroy();
	vkDestroyRenderPass(Vulkan::getDevice(), renderPass, nullptr);
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

void VulkanRenderer::resize(int w, int h) {
	if (!initialized) return;
	
	// lock rendering mutex (synchronized to start of frame)
	std::lock_guard<std::mutex> guard(cpuRenderingMutex);
	
	windowWidth = w;
	windowHeight = h;

	finishAllFrames();

	destroySwapchain(swapchain);
	swapchain = createSwapchain(surface, windowWidth, windowHeight);
	createSwapchainFramebuffers(swapchain, renderPass);
}

void VulkanRenderer::updateView(ViewManager* viewManager) {
	// lock rendering mutex (synchronized to start of frame)
	std::lock_guard<std::mutex> guard(cpuRenderingMutex);
	
	FPosition topLeft = viewManager->getTopLeft();
	FPosition bottomRight = viewManager->getBottomRight();
	// this function was designed for a slightly different coordinate system so it's a little wonky, but it works
	orthoMat = glm::ortho(topLeft.x, bottomRight.x, topLeft.y, bottomRight.y);
}

void VulkanRenderer::setCircuit(Circuit* circuit) {
	// no fancy synchronization needed, that's what the buffer ring is for
	blockRenderer.setCircuit(circuit);
		
	if (circuit) {
		logInfo("Renderer circuit assigned and setup", "Vulkan");
	}
	else {
		logInfo("Renderer circuit set to nothing", "Vulkan");
	}
}

void VulkanRenderer::updateCircuit(DifferenceSharedPtr diff) {
	// no fancy synchronization needed, that's what the buffer ring is for	
	blockRenderer.updateCircuit(diff);
}

void VulkanRenderer::setEvaluator(Evaluator* evaluator) {
	
}

void VulkanRenderer::renderLoop() {
	while(running) {
		FrameData& frame = getCurrentFrame();
		
		// wait until current frame is avaiable for rendering (slumber)
		vkWaitForFences(Vulkan::getDevice(), 1, &frame.renderFence, VK_TRUE, UINT64_MAX);
		// lock rendering mutex (confirm nothing else is fucking with renderer internals)
		std::lock_guard<std::mutex> guard(cpuRenderingMutex);

		// see what's knew after we awake from slumber
		// we can exit if we no longer want to render
		if (!running) break;

		// try to start rendering the frame
		// get next swapchain image to render to (or fail and try again)
		uint32_t imageIndex;
		VkResult imageGetResult = vkAcquireNextImageKHR(Vulkan::getDevice(), swapchain.handle, UINT64_MAX, frame.swapchainSemaphore, VK_NULL_HANDLE, &imageIndex);
		if (imageGetResult == VK_ERROR_OUT_OF_DATE_KHR) {
			// wait for a resize event by going to the next frame
			continue;
		} else if (imageGetResult != VK_SUCCESS && imageGetResult != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		// actually start rendering the frame
		// reset render fence
		vkResetFences(Vulkan::getDevice(), 1, &frame.renderFence);
		// update frame times
		auto currentTime = std::chrono::system_clock::now();
		lastFrameTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - frame.lastStartTime).count();
		frame.lastStartTime = currentTime;

		// get queues
		VkQueue& graphicsQueue = Vulkan::getSingleton().requestGraphicsQueue();
		VkQueue& presentQueue = Vulkan::getSingleton().requestPresentQueue();

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

	finishAllFrames();
}

void VulkanRenderer::finishAllFrames() {
	for (int i = 0; i < FRAME_OVERLAP; ++i) {
		FrameData& frame = getCurrentFrame(i);

		// wait for frame to finish
		vkWaitForFences(Vulkan::getDevice(), 1, &frame.renderFence, VK_TRUE, UINT64_MAX);
	}
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
	renderPassInfo.renderPass = renderPass;
	renderPassInfo.framebuffer = swapchain.framebuffers[imageIndex];
	renderPassInfo.renderArea.offset = {0, 0};
	renderPassInfo.renderArea.extent = swapchain.extent;

	VkClearValue clearColor = {{{0.93f, 0.93f, 0.93f, 1.0f}}};
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;
	
	vkCmdBeginRenderPass(frame.mainCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	// render all renderers
	blockRenderer.render(frame.mainCommandBuffer, swapchain.extent, orthoMat);

	// end
	vkCmdEndRenderPass(frame.mainCommandBuffer);
	if (vkEndCommandBuffer(frame.mainCommandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer!");
	}
}

void VulkanRenderer::createRenderPass(SwapchainData& swapchain) {
	// render pass
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = swapchain.imageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	// subpass
	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	// subpass dependency
	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	// create pass
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;
	if (vkCreateRenderPass(Vulkan::getDevice(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}
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

