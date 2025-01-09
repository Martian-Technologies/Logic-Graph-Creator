#include "vulkanDevice.h"

bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR idealSurface, const std::vector<std::string>& requiredExtensions) {
	// check queue graphics feature support
	QueueFamilyIndices indices = findQueueFamilies(device, idealSurface);
	// check extension support
	bool extensionsSupported = checkDeviceExtensionSupport(device, requiredExtensions);
	// check swap chain adequacy
	bool swapChainAdequate = false;
	if (extensionsSupported) {
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, idealSurface);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}
	
	return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR idealSurface) {
	SwapChainSupportDetails details;

	// get capabilities
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, idealSurface, &details.capabilities);

	// get formats
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, idealSurface, &formatCount, nullptr);
	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, idealSurface, &formatCount, details.formats.data());
	}

	// get present modes
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, idealSurface, &presentModeCount, nullptr);
	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, idealSurface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

bool checkDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<std::string>& requiredExtensions) {
	// get available extensions
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	// check if all required extensions are available
	std::set<std::string> uniqueExtensions(requiredExtensions.begin(), requiredExtensions.end());
	for (const auto& extension : availableExtensions) {
		uniqueExtensions.erase(extension.extensionName);
	}

	return uniqueExtensions.empty();

}

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR idealSurface) {
	QueueFamilyIndices indices;

	// get list of queue families
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	// find one that supports graphics
	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, idealSurface, &presentSupport);
		if (presentSupport) {
			indices.presentFamily = i;
		}

		// break out if we found everything we need
		if (indices.isComplete()) {
			break;
		}
	    
		++i;
	}
	
	return indices;
}
