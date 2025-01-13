#include "vulkanDevice.h"

SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR idealSurface) {
	SwapchainSupportDetails details;

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

bool checkDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char*>& requiredExtensions) {
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

QueueFamilies findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR idealSurface) {
	QueueFamilies indices;

	// get list of queue families
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	// find one that supports graphics
	uint32_t i = 0;
	for (const auto& queueFamily : queueFamilies) {
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = { i, queueFamily.queueCount };
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, idealSurface, &presentSupport);
		if (presentSupport) {
			indices.presentFamily = { i, queueFamily.queueCount };
		}

		// break out if we found everything we need
		if (indices.isComplete()) {
			break;
		}
	    
		++i;
	}
	
	return indices;
}
