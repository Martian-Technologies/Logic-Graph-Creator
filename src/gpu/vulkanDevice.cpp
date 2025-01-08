#include "vulkanDevice.h"

VulkanDevicePicker::VulkanDevicePicker(VkInstance instance, VkSurfaceKHR idealSurface)
	: instance(instance), idealSurface(idealSurface) {
	
}

std::optional<VkPhysicalDevice> VulkanDevicePicker::pick() {
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if (deviceCount == 0) {
		error = "failed to find GPUs with Vulkan support!";
		return std::nullopt;
	}

	// get vulkan supported devices
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	// find first suitable device
	for (const auto& device : devices) {
		if (isDeviceSuitable(device)) {
			return device;
		}
	}

	error = "failed to find suitable GPU";
	return std::nullopt;
}

bool VulkanDevicePicker::isDeviceSuitable(VkPhysicalDevice device) {
	// check queue graphics feature support
	QueueFamilyIndices indices = findQueueFamilies(device);
	// check extension support
	bool extensionsSupported = checkDeviceExtensionSupport(device);
	// check swap chain adequacy
	bool swapChainAdequate = false;
	if (extensionsSupported) {
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}
	
	return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

SwapChainSupportDetails VulkanDevicePicker::querySwapChainSupport(VkPhysicalDevice device) {
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

bool VulkanDevicePicker::checkDeviceExtensionSupport(VkPhysicalDevice device) {
	if (requiredDeviceExtensions.has_value()) { return true; }
	
	// get available extensions
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	// check if all required extensions are available
	std::set<std::string> requiredExtensions(requiredDeviceExtensions.value().begin(), requiredDeviceExtensions.value().end());
	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();

}

QueueFamilyIndices VulkanDevicePicker::findQueueFamilies(VkPhysicalDevice device) {
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
