#include "vulkanManager.h"

#include <cassert>
#include <cstring>

// Constants ---------------------------------------------------------------------------
const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};
const std::vector<const char *> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

// Functions ---------------------------------------------------------------------------
void VulkanManager::createInstance(const std::vector<const char*>& requiredExtensions, bool enableValidationlayers) {
	// confirm we have validation layers if we need them
	if (enableValidationlayers && !checkValidationLayerSupport()) {
	fail("validation layers requested, but not available!");
	}
	
	// set applicaiton information
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Gatality";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	// start instance creation
	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	// add extensions
	auto extensions = requiredExtensions;
	if (enableValidationlayers) { extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME); }
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	// enable validation layers3
	if (enableValidationlayers) {
	createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
	createInfo.ppEnabledLayerNames = validationLayers.data();
	// TODO - custom message callback
		
	} else {
	createInfo.enabledLayerCount = 0;
	createInfo.pNext = nullptr;
	}
	
	// create the instance
	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
	fail("failed to create instance!");
	}
}

void VulkanManager::createDevice(VkSurfaceKHR surface) {
	
}

void VulkanManager::destroy() {
	vkDestroyInstance(instance, nullptr);
}


// TODO - the application as a whole should have a failing system.
// this also won't work once NDEBUG is re-enabled and asserts are disabled
void VulkanManager::fail(const std::string& reason) {
	std::cerr << reason << std::endl;
	assert(false);
}

// Helper functions

bool VulkanManager::checkValidationLayerSupport() {
	// get supported layers
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	// confirm support of every required layer
	for (const char* layerName : validationLayers) {
	bool layerFound = false;

	for (const auto& layerProperties : availableLayers) {
		if (strcmp(layerName, layerProperties.layerName) == 0) {
		layerFound = true;
		break;
		}
	}

	if (!layerFound) {
		return false;
	}	
	}

	return true;
}
