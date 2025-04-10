#include "windowRenderer.h"

#include <glm/gtc/matrix_transform.hpp>

WindowRenderer::WindowRenderer(SdlWindow* sdlWindow)
	: sdlWindow(sdlWindow) {
	logInfo("Initializing window renderer...");

	// create surface and use it to make sure a vulkan device has been created
	surface = sdlWindow->createVkSurface(VulkanInstance::get().getInstance());
	VulkanInstance::get().ensureDeviceCreation(surface);
	device = VulkanInstance::get().getDevice();

	// get window size
	windowSize = sdlWindow->getSize();
	
	// set up swapchain and subrenderer
	swapchain = std::make_unique<Swapchain>(surface, windowSize);
	subrenderer = std::make_unique<SubrendererManager>(swapchain.get());
	swapchain->createFramebuffers(subrenderer->getRenderPass());

	// set up frames
	frames.reserve(FRAMES_IN_FLIGHT);
	for (int i = 0; i < FRAMES_IN_FLIGHT; ++i) {
		frames.emplace_back();
	}
	
	// start render loop
	running = true;
	renderThread = std::thread(&WindowRenderer::renderLoop, this);
}

WindowRenderer::~WindowRenderer() {
	// stop render thread (not completely sure if this is right for the destructor yet)
	running = false;
	if (renderThread.joinable()) renderThread.join();

	// manual deletion (RAII blues)
	subrenderer.reset();
}

void WindowRenderer::resize(std::pair<uint32_t, uint32_t> windowSize) {
	std::lock_guard<std::mutex> lock(windowSizeMux);

	this->windowSize = windowSize;

	swapchainRecreationNeeded = true;
}

void WindowRenderer::renderLoop() {
	while(running) {
		VulkanFrameData& frame = getCurrentFrame();

		// wait for frame completion
		frame.waitAndComplete();
		
		// recreate swapchain if needed
		if (swapchainRecreationNeeded) {
			recreateSwapchain();
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
		
		// tell frame that it has started
		frame.start();

		// get queues
		VkQueue& graphicsQueue = VulkanInstance::get().getGraphicsQueue().queue;
		VkQueue& presentQueue = VulkanInstance::get().getPresentQueue().queue;

		// record command buffer
		vkResetCommandBuffer(frame.getMainCommandBuffer(), 0);
		subrenderer->renderCommandBuffer(frame, imageIndex);

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
			throwFatalError("failed to submit draw command buffer!");
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
		if (imagePresentResult == VK_ERROR_OUT_OF_DATE_KHR || imagePresentResult == VK_SUBOPTIMAL_KHR) {
			swapchainRecreationNeeded = true;
		} else if (imagePresentResult != VK_SUCCESS) {
			logError("failed to present swap chain image!");
		}

		//increase the number of frames drawn
		++frameNumber;
	}

	vkDeviceWaitIdle(device);
}

void WindowRenderer::recreateSwapchain() {
	vkDeviceWaitIdle(device);
	
	std::lock_guard<std::mutex> lock(windowSizeMux);

	swapchain->recreate(surface, windowSize);
	swapchain->createFramebuffers(subrenderer->getRenderPass());
	
	swapchainRecreationNeeded = false;
}

void WindowRenderer::prepareForRml(RmlRenderInterface& renderInterface) {
	renderInterface.pointToRenderer(&subrenderer->getRmlRenderer());
	subrenderer->getRmlRenderer().prepareForRmlRender();
}
void WindowRenderer::endRml() {
	subrenderer->getRmlRenderer().endRmlRender();
}
