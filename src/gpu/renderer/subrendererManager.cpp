#include "subrendererManager.h"

#include "gpu/vulkanInstance.h"

SubrendererManager::SubrendererManager(Swapchain* swapchain)
	: swapchain(swapchain), descriptorAllocator(100, {{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 }, { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1}}) {

	createRenderPass();

	// descriptors
	DescriptorLayoutBuilder builder;
	builder.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	viewDataLayout = builder.build(VK_SHADER_STAGE_VERTEX_BIT);

	// rml
	rmlRenderer = std::make_unique<RmlRenderer>(renderPass, viewDataLayout);
}

SubrendererManager::~SubrendererManager() {
	vkDestroyDescriptorSetLayout(VulkanInstance::get().getDevice(), viewDataLayout, nullptr);
	vkDestroyRenderPass(VulkanInstance::get().getDevice(), renderPass, nullptr);
}

// pixelViewMat = glm::ortho(0.0f, (float)windowSize.first, 0.0f, (float)windowSize.second);

void SubrendererManager::createRenderPass() {
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

void SubrendererManager::renderCommandBuffer(VulkanFrameData& frame, uint32_t imageIndex) {
	// preparation
	VkExtent2D windowSize = swapchain->getVkbSwapchain().extent;
	
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
		// rml rendering
		rmlRenderer->render(frame, windowSize);
	}

	// end render pass
	vkCmdEndRenderPass(frame.getMainCommandBuffer());
	if (vkEndCommandBuffer(frame.getMainCommandBuffer()) != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer!");
	}
}
