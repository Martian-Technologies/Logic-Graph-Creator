#include "vulkanSwapchain.h"

#include <limits>

#include "gpu/vulkanManager.h"
#include "gpu/vulkanDevice.h"

SwapchainData createSwapchain(VkSurfaceKHR surface, int windowWidth, int windowHeight) {
	SwapchainData swapchain;
	
	SwapchainSupportDetails swapchainSupport = querySwapchainSupport(Vulkan::getPhysicalDevice(), surface);

	// choose best values for swap chain
	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapchainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapchainSupport.presentModes);
	VkExtent2D extent = chooseSwapExtent(swapchainSupport.capabilities, windowWidth, windowHeight);

	// set image count to one above the minimum, but don't exceed the maximum if there is one
	uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;
	if (swapchainSupport.capabilities.maxImageCount > 0 && imageCount > swapchainSupport.capabilities.maxImageCount) {
		imageCount = swapchainSupport.capabilities.maxImageCount;
	};

	// create creator struct
	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

	// if we need our queues to share this swapchain (they are different queues), update the settings
	uint32_t queueFamilyIndices[] = {Vulkan::getQueueFamilies().graphicsFamily.value().index, Vulkan::getQueueFamilies().presentFamily.value().index};
	if (queueFamilyIndices[0] != queueFamilyIndices[1]) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	} else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}

	// additional settings
	createInfo.preTransform = swapchainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	// create the swapChain
	if (vkCreateSwapchainKHR(Vulkan::getDevice(), &createInfo, nullptr, &swapchain.handle) != VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain!");
	}

	// save info about the swapchain
	swapchain.imageFormat = surfaceFormat.format;
	swapchain.extent = extent;

	// get swapchain images
	vkGetSwapchainImagesKHR(Vulkan::getDevice(), swapchain.handle, &imageCount, nullptr);
	swapchain.images.resize(imageCount);
	vkGetSwapchainImagesKHR(Vulkan::getDevice(), swapchain.handle, &imageCount, swapchain.images.data());

	// create an image view for each image
	swapchain.imageViews.resize(swapchain.images.size());
	for (size_t i = 0; i < swapchain.images.size(); i++) {
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swapchain.images[i];

		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = swapchain.imageFormat;

		// no swizzle
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		// no mipmaps or layers
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(Vulkan::getDevice(), &createInfo, nullptr, &swapchain.imageViews[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image view!");
		} 
	}

	return swapchain;
}

void createSwapchainFramebuffers(SwapchainData& swapchain, VkRenderPass renderPass) {
	swapchain.framebuffers.resize(swapchain.imageViews.size());

	for (size_t i = 0; i < swapchain.imageViews.size(); i++) {
		VkImageView attachments[] = {
			swapchain.imageViews[i]
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = swapchain.extent.width;
		framebufferInfo.height = swapchain.extent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(Vulkan::getDevice(), &framebufferInfo, nullptr, &swapchain.framebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}

void destroySwapchain(SwapchainData &swapchain) {
	for (VkFramebuffer framebuffer : swapchain.framebuffers) {
        vkDestroyFramebuffer(Vulkan::getDevice(), framebuffer, nullptr);
    }
	
	for (VkImageView imageView : swapchain.imageViews) {
		vkDestroyImageView(Vulkan::getDevice(), imageView, nullptr);
	}
	
	vkDestroySwapchainKHR(Vulkan::getDevice(), swapchain.handle, nullptr);
}

// UTIL

VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
	// look for our favorite :D
	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}

	// I guess we could settle for whatever...
	return availableFormats[0];
}

VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
	// mailbox is the best!
	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		}
	}

	// but vsync is chill too...
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, int realWidth, int realHeight) {
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	} else {
		// sometimes the width and height are fucked so we do this instead

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(realWidth),
			static_cast<uint32_t>(realHeight)
		};

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
}
