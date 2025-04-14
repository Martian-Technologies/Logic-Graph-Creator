#include "windowSubrendererManager.h"

#include <glm/gtc/matrix_transform.hpp>

#include "gpu/vulkanInstance.h"

struct GPUViewData {
	glm::mat4 pixelViewMat;
};

WindowSubrendererManager::WindowSubrendererManager(Swapchain* swapchain, std::vector<VulkanFrameData>& frames)
	: swapchain(swapchain), descriptorAllocator(100, {{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 }, { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1}}) {

	createRenderPass();

	// descriptors
	DescriptorLayoutBuilder builder;
	builder.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	viewDataLayout = builder.build(VK_SHADER_STAGE_VERTEX_BIT);
	for (VulkanFrameData& frame : frames) {
		frame.getViewDataDescriptorSet() = descriptorAllocator.allocate(viewDataLayout);
		frame.getViewDataBuffer() = createBuffer(sizeof(GPUViewData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
	}

	// rml
	rmlRenderer = std::make_unique<RmlRenderer>(renderPass, viewDataLayout);
}

WindowSubrendererManager::~WindowSubrendererManager() {
	vkDestroyDescriptorSetLayout(VulkanInstance::get().getDevice(), viewDataLayout, nullptr);
	vkDestroyRenderPass(VulkanInstance::get().getDevice(), renderPass, nullptr);
}

void WindowSubrendererManager::createRenderPass() {
	// render pass
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = swapchain->getVkbSwapchain().image_format;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
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
	
	if (vkCreateRenderPass(VulkanInstance::get().getDevice(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}
}

void WindowSubrendererManager::renderCommandBuffer(VulkanFrameData& frame, uint32_t imageIndex) {
	// preparation
	VkExtent2D windowSize = swapchain->getVkbSwapchain().extent;
	// update view data
	GPUViewData viewData{ glm::ortho(0.0f, (float)windowSize.width, 0.0f, (float)windowSize.height) };
	vmaCopyMemoryToAllocation(VulkanInstance::get().getAllocator(), &viewData, frame.getViewDataBuffer().allocation, 0, sizeof(GPUViewData));
	// update view data descriptor
	DescriptorWriter writer;
	writer.writeBuffer(0, frame.getViewDataBuffer().buffer, sizeof(GPUViewData), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	writer.updateSet(VulkanInstance::get().getDevice(), frame.getViewDataDescriptorSet());
	
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
	renderPassInfo.clearValueCount = 0;
	renderPassInfo.pClearValues = nullptr;
	
	vkCmdBeginRenderPass(frame.getMainCommandBuffer(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	// do actual rendering...
	{
		// viewports
		std::lock_guard<std::mutex> lock(viewportRenderersMux);
		for (ViewportRenderer* viewportRenderer : viewportRenderers) {
			viewportRenderer->render(frame);
		}
		
		// rml rendering
		rmlRenderer->render(frame, windowSize, frame.getViewDataDescriptorSet());
	}

	// end render pass
	vkCmdEndRenderPass(frame.getMainCommandBuffer());
	if (vkEndCommandBuffer(frame.getMainCommandBuffer()) != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer!");
	}
}

void WindowSubrendererManager::registerViewportRenderInterface(ViewportRenderer *renderInterface) {
	renderInterface->initializeVulkan(renderPass);
	
	std::lock_guard<std::mutex> lock(viewportRenderersMux);
	viewportRenderers.insert(renderInterface);
}

void WindowSubrendererManager::deregisterViewportRenderInterface(ViewportRenderer* renderInterface) {
	std::lock_guard<std::mutex> lock(viewportRenderersMux);
	viewportRenderers.erase(renderInterface);
}
