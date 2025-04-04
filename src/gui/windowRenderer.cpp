#include "windowRenderer.h"

WindowRenderer::WindowRenderer(SdlWindow* sdlWindow)
	: sdlWindow(sdlWindow) {
	logInfo("Initializing window renderer...");

	// create surface and use it to make sure a vulkan device has been created
	surface = sdlWindow->createVkSurface(VulkanInstance::get().getInstance());
	VulkanInstance::get().ensureDeviceCreation(surface);
	device = VulkanInstance::get().getDevice();

	// get window size
	windowSize = sdlWindow->getSize();
	
	// set up swapchain
	swapchain = new Swapchain(surface, windowSize);
	createRenderPass();
	swapchain->createFramebuffers(renderPass);

	// set up frames
	frames.resize(FRAMES_IN_FLIGHT);

	// start render loop
	running = true;
	renderThread = std::thread(&WindowRenderer::renderLoop, this);
}

WindowRenderer::~WindowRenderer() {
	// stop render thread (not completely sure if this is right for the destructor yet)
	running = false;
	if (renderThread.joinable()) renderThread.join();
	
	vkDestroyRenderPass(device, renderPass, nullptr);
	delete swapchain;
}

void WindowRenderer::resize(std::pair<uint32_t, uint32_t> windowSize) {
	std::lock_guard<std::mutex> lock(windowSizeMux);

	this->windowSize = windowSize;

	swapchainRecreationNeeded = true;
}

void WindowRenderer::prepareForRml() {
	
}
void WindowRenderer::endRml() {
	
}

void WindowRenderer::renderLoop() {
	while(running) {
		/*
		VulkanFrameData& frame = getCurrentFrame();

		// wait for frame completion
		frame.waitAndComplete();

		// recreate swapchain if needed
		if (swapchainRecreationNeeded) {
			swapchain->recreate();
			continue;
		}

		// try to start rendering the frame
		// get next swapchain image to render to (or fail and try again)
		uint32_t imageIndex;
		VkResult imageGetResult = vkAcquireNextImageKHR(device, swapchain->getVkbSwapchain().swapchain, UINT64_MAX, frame.getSwapchainSemaphore(), VK_NULL_HANDLE, &imageIndex);
		if (imageGetResult == VK_ERROR_OUT_OF_DATE_KHR || imageGetResult == VK_SUBOPTIMAL_KHR) {
			// if the swapchain is not ideal, try again but recreate it this time (this happens in normal operation)
			swapchainRecreationNeeded = true;
			continue;
		} else if (imageGetResult != VK_SUCCESS) {
			// if the error was even worse (one could say exceptional), we log an error and pray
			logError("failed to acquire swap chain image!");
			continue;
		}
		
		// tell frame that it is started
		frame.start();

		// get queues
		VkQueue& graphicsQueue = VulkanInstance::get().getGraphicsQueue().queue;
		VkQueue& presentQueue = VulkanInstance::get().getPresentQueue().queue;

		// record command buffer
		vkResetCommandBuffer(frame.getMainCommandBuffer(), 0);
		recordCommandBuffer(frame, imageIndex);

		// start setting up submission
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		// wait semaphore
		VkSemaphore waitSemaphores[] = { frame.getSwapchainSemaphore() };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		
		// command buffers
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &frame.getMainCommandBuffer();

		// signal semaphores
		VkSemaphore signalSemaphores[] = { frame.getRenderSemaphore() };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		// submit to queue
		if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, frame.getRenderFence()) != VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw command buffer!");
		}
		
		// present
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;
		VkSwapchainKHR swapchains[] = { swapchain->getVkbSwapchain().swapchain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapchains;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr; // unused

		VkResult imagePresentResult = vkQueuePresentKHR(presentQueue, &presentInfo);
		if (imagePresentResult == VK_ERROR_OUT_OF_DATE_KHR || imageGetResult == VK_SUBOPTIMAL_KHR) {
			swapchainRecreationNeeded = true;
		} else if (imagePresentResult != VK_SUCCESS) {
			logError("failed to present swap chain image!");
			break;
		}

		//increase the number of frames drawn
		++frameNumber;
		*/
	}

	vkDeviceWaitIdle(device);
}


void WindowRenderer::createRenderPass() {
	// render pass
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = swapchain->getVkbSwapchain().image_format;
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
	
	if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}
}

void WindowRenderer::recreateSwapchain() {
	vkDeviceWaitIdle(device);
	
	std::lock_guard<std::mutex> lock(windowSizeMux);

	swapchain->recreate(surface, windowSize);
	swapchain->createFramebuffers(renderPass);
	
	swapchainRecreationNeeded = false;
}

// -- Rml::RenderInterface --

Rml::CompiledGeometryHandle WindowRenderer::CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices) {
	return 1;
}
void WindowRenderer::ReleaseGeometry(Rml::CompiledGeometryHandle geometry) {
	
}
void WindowRenderer::RenderGeometry(Rml::CompiledGeometryHandle handle, Rml::Vector2f translation, Rml::TextureHandle texture) {
	
}

// Textures
Rml::TextureHandle WindowRenderer::LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source) {
	return 1;
}
Rml::TextureHandle WindowRenderer::GenerateTexture(Rml::Span<const Rml::byte> source, Rml::Vector2i source_dimensions) {
	return 1;
}
void WindowRenderer::ReleaseTexture(Rml::TextureHandle texture_handle) {
	
}

// Scissor
void WindowRenderer::EnableScissorRegion(bool enable) {
	
}
void WindowRenderer::SetScissorRegion(Rml::Rectanglei region) {
	
}
