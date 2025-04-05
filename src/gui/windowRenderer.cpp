#include "windowRenderer.h"

#include "gpu/vulkanShader.h"
#include "computerAPI/directoryManager.h"
#include "computerAPI/fileLoader.h"

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
	swapchain = std::make_unique<Swapchain>(surface, windowSize);
	createRenderPass();
	swapchain->createFramebuffers(renderPass);

	// set up frames
	frames.resize(FRAMES_IN_FLIGHT);

	// set up pipeline
	VkShaderModule rmlVertShader = createShaderModule(readFileAsBytes(DirectoryManager::getResourceDirectory() / "shaders/rml.vert.spv"));
	VkShaderModule rmlFragShader = createShaderModule(readFileAsBytes(DirectoryManager::getResourceDirectory() / "shaders/rml.frag.spv"));
	PipelineInformation rmlPipelineInfo{};
	rmlPipelineInfo.vertShader = rmlVertShader;
	rmlPipelineInfo.fragShader = rmlFragShader;
	rmlPipelineInfo.renderPass = renderPass;
	rmlPipelineInfo.vertexBindingDescriptions = RmlVertex::getBindingDescriptions();
	rmlPipelineInfo.vertexAttributeDescriptions = RmlVertex::getAttributeDescriptions();
	rmlPipelineInfo.pushConstantSize = sizeof(RmlPushConstants);
	rmlPipelineInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rmlPipeline = std::make_unique<Pipeline>(rmlPipelineInfo);
	destroyShaderModule(rmlVertShader);
	destroyShaderModule(rmlFragShader);

	// start render loop
	running = true;
	renderThread = std::thread(&WindowRenderer::renderLoop, this);
}

WindowRenderer::~WindowRenderer() {
	// stop render thread (not completely sure if this is right for the destructor yet)
	running = false;
	if (renderThread.joinable()) renderThread.join();

	// manual deletion (RAII blues)
	rmlPipeline.reset();
	vkDestroyRenderPass(device, renderPass, nullptr);
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

void WindowRenderer::recordCommandBuffer(VulkanFrameData& frame, uint32_t imageIndex) {
	// start recording
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0; // Optional
	beginInfo.pInheritanceInfo = nullptr; // Optional
	if (vkBeginCommandBuffer(frame.getMainCommandBuffer(), &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	// begin render pass
	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = renderPass;
	renderPassInfo.framebuffer = swapchain->getFramebuffers()[imageIndex];
	renderPassInfo.renderArea.offset = {0, 0};
	renderPassInfo.renderArea.extent = swapchain->getVkbSwapchain().extent;

	VkClearValue clearColor = {0.93f, 0.93f, 0.93f, 1.0f};
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;
	
	vkCmdBeginRenderPass(frame.getMainCommandBuffer(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	// do actual rendering...
	{
		// rml rendering

		// bind pipeline
		vkCmdBindPipeline(frame.getMainCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, rmlPipeline->getHandle());
		// bind dynamic state
		VkExtent2D& extent = swapchain->getVkbSwapchain().extent;
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(extent.width);
		viewport.height = static_cast<float>(extent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(frame.getMainCommandBuffer(), 0, 1, &viewport);
		VkRect2D scissor{};
		scissor.offset = {0, 0};
		scissor.extent = extent;
		vkCmdSetScissor(frame.getMainCommandBuffer(), 0, 1, &scissor);
		
		std::lock_guard<std::mutex> lock(rmlInstructionMux);
		for (const auto& instruction : rmlInstructions) {
			if (std::holds_alternative<RmlRenderInstruction>(instruction)) {
				// If we have a render instruction
				const RmlRenderInstruction& renderInstruction = std::get<RmlRenderInstruction>(instruction);
				frame.getRmlAllocations().push_back(renderInstruction.geometry);

				// upload push constants
				RmlPushConstants pushConstants{ renderInstruction.translation };
				vkCmdPushConstants(frame.getMainCommandBuffer(), rmlPipeline->getLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(RmlPushConstants), &pushConstants);

				// bind vertex buffer
				VkBuffer vertexBuffers[] = { renderInstruction.geometry->getVertexBuffer().buffer };
				VkDeviceSize offsets[] = { 0 };
				vkCmdBindVertexBuffers(frame.getMainCommandBuffer(), 0, 1, vertexBuffers, offsets);

				// bind index buffer
				vkCmdBindIndexBuffer(frame.getMainCommandBuffer(), renderInstruction.geometry->getIndexBuffer().buffer, offsets[0], VK_INDEX_TYPE_UINT32);

				// draw
				vkCmdDrawIndexed(frame.getMainCommandBuffer(), renderInstruction.geometry->getNumIndices(), 1, 0, 0, 0);
			}
		}
	}

	// end render pass
	vkCmdEndRenderPass(frame.getMainCommandBuffer());
	if (vkEndCommandBuffer(frame.getMainCommandBuffer()) != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer!");
	}
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

void WindowRenderer::prepareForRml() {
	// clear list of temp RML instructions so we can start adding
	tempRmlInstructions.clear();
}
void WindowRenderer::endRml() {
	// swap the real instructions out for the new ones
	std::lock_guard<std::mutex> lock(rmlInstructionMux);
	rmlInstructions = std::move(tempRmlInstructions);
}

// -- Rml::RenderInterface --

Rml::CompiledGeometryHandle WindowRenderer::CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices) {
	// get and increment handle
	Rml::CompiledGeometryHandle newHandle = currentGeometryHandle++;
	// alocate new geometry
	rmlGeometryAllocations[newHandle] = std::make_shared<RmlGeometryAllocation>(vertices, indices);
	
	return newHandle;
}
void WindowRenderer::ReleaseGeometry(Rml::CompiledGeometryHandle geometry) {
	rmlGeometryAllocations.erase(geometry);
}
void WindowRenderer::RenderGeometry(Rml::CompiledGeometryHandle handle, Rml::Vector2f translation, Rml::TextureHandle texture) {
	// find geometry
	auto geometryItr = rmlGeometryAllocations.find(handle);
	if (geometryItr == rmlGeometryAllocations.end()) {
		logError("tried to render non existent RML geometry", "Vulkan");
		return;
	}
	
	tempRmlInstructions.push_back(RmlRenderInstruction(geometryItr->second, {translation.x, translation.y}));
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
	logInfo("enable scissor");
}
void WindowRenderer::SetScissorRegion(Rml::Rectanglei region) {
	logInfo("set scissor");
}
