#include "vulkanSwapchain.h"

void Swapchain::init(VulkanDevice* device, VkSurfaceKHR surface, std::pair<uint32_t, uint32_t> size) {
	this->device = device;
	
	createSwapchain(surface, size, false);
}

void Swapchain::cleanup() {
	destroyFramebuffers();
	vkb::destroy_swapchain(swapchain);

	imageViews.clear();
}

void Swapchain::recreate(VkSurfaceKHR surface, std::pair<uint32_t, uint32_t> size) {
	destroyFramebuffers();
	createSwapchain(surface, size, true);
}

void Swapchain::createSwapchain(VkSurfaceKHR surface, std::pair<uint32_t, uint32_t> size, bool useOld) {
	// Create swapchain
	vkb::SwapchainBuilder swapchainBuilder(device->getDevice(), surface);
	swapchainBuilder.set_desired_extent(size.first, size.second);
	swapchainBuilder.set_desired_format({VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR});
	if (useOld) swapchainBuilder.set_old_swapchain(swapchain);
	
	auto swapchainRet = swapchainBuilder.build();
	if (!swapchainRet)
		throwFatalError("Could not create vulkan swapchain. Error: " + swapchainRet.error().message());
	if (useOld)
		vkb::destroy_swapchain(swapchain);
	swapchain = swapchainRet.value();

	// Get image views
	auto imageViewRet = swapchain.get_image_views();
	if (!imageViewRet) { throwFatalError("Could not get vulkan swapchain image views. Error: " + imageViewRet.error().message()); }
	imageViews = imageViewRet.value();
}

// Destroy Frame buffers and image views
void Swapchain::destroyFramebuffers() {
	for (VkFramebuffer framebuffer : framebuffers) {
        vkDestroyFramebuffer(device->getDevice(), framebuffer, nullptr);
    }
	framebuffers.clear();
	swapchain.destroy_image_views(imageViews);
}

// Create and initialize framebuffers for use with the swapchain
void Swapchain::createFramebuffers(const VkRenderPass renderPass) {
	framebuffers.resize(swapchain.image_count);

	for (size_t i = 0; i < swapchain.image_count; i++) {
		const VkImageView attachments[] = {
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

		if (vkCreateFramebuffer(device->getDevice(), &framebufferInfo, nullptr, &framebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}

