#include "vulkanSwapchain.h"

Swapchain::Swapchain(VkSurfaceKHR surface, std::pair<uint32_t, uint32_t> size) {
	// Create swapchain
	vkb::SwapchainBuilder swapchainBuilder(VulkanInstance::get().getVkbDevice(), surface);
	swapchainBuilder.set_desired_extent(size.first, size.second);
	auto swapchainRet = swapchainBuilder.build();
	if (!swapchainRet) { throwFatalError("Could not create vulkan swapchain. Error: " + swapchainRet.error().message()); }
	swapchain = swapchainRet.value();

	// Get image views
	auto imageViewRet =  swapchain.get_image_views();
	if (!imageViewRet) { throwFatalError("Could not get vulkan swapchain image views. Error: " + imageViewRet.error().message()); }
	imageViews = imageViewRet.value();
}

Swapchain::~Swapchain() {
	for (VkFramebuffer framebuffer : framebuffers) {
        vkDestroyFramebuffer(VulkanInstance::get().getDevice(), framebuffer, nullptr);
    }
	swapchain.destroy_image_views(imageViews);
	vkb::destroy_swapchain(swapchain);
}

void Swapchain::createFramebuffers(VkRenderPass renderPass) {
	framebuffers.resize(swapchain.image_count);

	for (size_t i = 0; i < swapchain.image_count; i++) {
		VkImageView attachments[] = {
			imageViews[i]
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = swapchain.extent.width;
		framebufferInfo.height = swapchain.extent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(VulkanInstance::get().getDevice(), &framebufferInfo, nullptr, &framebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}

void Swapchain::recreate(std::pair<uint32_t, uint32_t> size) {
	
}

