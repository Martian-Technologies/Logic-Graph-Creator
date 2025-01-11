#include "vulkanRenderer.h"

#include "gpu/vulkanDevice.h"
#include "gpu/vulkanSwapchain.h"

void VulkanRenderer::initialize(VulkanGraphicsView view, VkSurfaceKHR surface, int w, int h)
{
	this->view = view;
	this->surface = surface;

	windowWidth = w;
	windowHeight = h;

	createSwapChain();
}

void VulkanRenderer::destroy() {
	vkDestroySwapchainKHR(view.device, swapchain, nullptr);
}

void VulkanRenderer::resize(int w, int h) {
	windowWidth = w;
	windowHeight = h;	
}

void VulkanRenderer::run() {
	
}

// Vulkan Setup

void VulkanRenderer::createSwapChain() {
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(view.physicalDevice, surface);

	// choose best values for swap chain
	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities, windowWidth, windowHeight);

	// set image count to one above the minimum, but don't exceed the maximum if there is one
	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
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
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	// if we need our queues to share this swapchain (they are different queues), update the settings
	QueueFamilies indices = findQueueFamilies(view.physicalDevice, surface);
	uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value().index, indices.presentFamily.value().index};
	if (indices.graphicsFamily.value().index != indices.presentFamily.value().index) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	} else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}

	// additional settings
	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	// create the swapChain
	if (vkCreateSwapchainKHR(view.device, &createInfo, nullptr, &swapchain) != VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain!");
	}

	swapchainImageFormat = surfaceFormat.format;
	swapchainExtent = extent;

	vkGetSwapchainImagesKHR(view.device, swapchain, &imageCount, nullptr);
	swapchainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(view.device, swapchain, &imageCount, swapchainImages.data());
}

// INTERFACE
// ======================================================================

void VulkanRenderer::setCircuit(Circuit* circuit) {
	
}

void VulkanRenderer::setEvaluator(Evaluator* evaluator) {
	
}

void VulkanRenderer::updateView(ViewManager* viewManager) {
	
}

void VulkanRenderer::updateCircuit(DifferenceSharedPtr diff) {
	
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

