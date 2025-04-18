#include "vulkanManager.h"

#include <vulkan/vk_enum_string_helper.h>

#include <cassert>
#include <cstring>

void VulkanConfirmErrorStatus(VkResult x) {
	if (x != VK_SUCCESS) {
		std::string msg = "Call failed with result: " + std::string(string_VkResult(x));
		logFatalError(msg, "Vulkan");
		throw std::runtime_error(msg);
	}
}

// Constants ---------------------------------------------------------------------------
const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};
const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

#ifdef DEBUG
const bool USE_VALIDATION_LAYERS = true;
#else
const bool USE_VALIDATION_LAYERS = false;
#endif

static VKAPI_ATTR VkBool32 VKAPI_CALL vulkanDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

	switch (messageSeverity) {
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
		logInfo(pCallbackData->pMessage, "VK Validation");
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		logWarning(pCallbackData->pMessage, "VK Validation");
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		logError(pCallbackData->pMessage, "VK Validation");
		break;
	default:
		break;
	}

    return VK_FALSE;
}
void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = vulkanDebugCallback;
}

// Functions ---------------------------------------------------------------------------
void Vulkan::createInstance() {
	#ifdef DEBUG
	logInfo("Creating Vulkan Instance", "Vulkan");
	#endif

	// confirm we have validation layers if we need them
	if (USE_VALIDATION_LAYERS && !checkValidationLayerSupport()) {
		throw std::runtime_error("validation layers requested, but not available!");
	}

	// set applicaiton information
	VkApplicationInfo appInfo {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Gatality";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	// start instance creation
	VkInstanceCreateInfo createInfo {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR; // this seems like a good thing to have in general, but I did only add it for macOS. In the future should check if this is actually available before enabling.
	createInfo.pApplicationInfo = &appInfo;

	// add extensions
	std::vector<std::string> requiredExtensionsStr = {};//getRequiredInstanceExtensions();
	std::vector<const char*> requiredExtensions;
	for (const std::string& extension : requiredExtensionsStr) requiredExtensions.push_back(extension.c_str());
	if (USE_VALIDATION_LAYERS) { requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME); }
	createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
	createInfo.ppEnabledExtensionNames = requiredExtensions.data();

	// enable validation layers
	if (USE_VALIDATION_LAYERS) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		// instance creation debug messenger
		VkDebugUtilsMessengerCreateInfoEXT createInfo {};
		populateDebugMessengerCreateInfo(createInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &createInfo;

	} else {
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}

	// create the instance
	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
		throw std::runtime_error("failed to create instance!");
	}

	// create debug messenger
	if (USE_VALIDATION_LAYERS) {
		VkDebugUtilsMessengerCreateInfoEXT createInfo {};
		populateDebugMessengerCreateInfo(createInfo);

		// get function pointer
		auto createDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (!createDebugUtilsMessengerEXT) {
			throw std::runtime_error("validation layer requested, but couldn't get debug messenger create function pointer");
		}
		if (createDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}
}

void Vulkan::setupDevice(VkSurfaceKHR surface) {
	pickPhysicalDevice(surface);
	createLogicalDevice();
	createAllocator();
}

void Vulkan::destroy() {
	vmaDestroyAllocator(allocator);
	vkDestroyDevice(device, nullptr);

	// destroy debug messenger
	auto destroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (!destroyDebugUtilsMessengerEXT) {
		throw std::runtime_error("validation layer requested, but couldn't get debug messenger destroy function pointer");
	}
	destroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
	
	vkDestroyInstance(instance, nullptr);
}

// Helper functions
bool Vulkan::checkValidationLayerSupport() {
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

void Vulkan::pickPhysicalDevice(VkSurfaceKHR idealSurface) {
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr); 

	logInfo("Found " + std::to_string(deviceCount) + " GPUs with Vulkan support", "Vulkan");

	if (deviceCount == 0) {
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}
	
	// get vulkan supported devices
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	// find first suitable device
	for (const auto& device : devices) {
		if (isDeviceSuitable(device, idealSurface)) {
			physicalDevice = device;
			queueFamilies = findQueueFamilies(physicalDevice, idealSurface);
			return;
		}
	}

	throw std::runtime_error("failed to find suitable GPU");
}

bool Vulkan::isDeviceSuitable(VkPhysicalDevice physicalDevice, VkSurfaceKHR idealSurface) {
	// check queue graphics feature support
	struct QueueFamilies indices = findQueueFamilies(physicalDevice, idealSurface);
	// check extension support
	bool extensionsSupported = checkDeviceExtensionSupport(physicalDevice, deviceExtensions);
	// check swap chain adequacy
	bool swapChainAdequate = false;
	if (extensionsSupported) {
		SwapchainSupportDetails swapChainSupport = querySwapchainSupport(physicalDevice, idealSurface);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}
	
	return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

void Vulkan::createLogicalDevice() {
	// create all the queues from unique queues that we need
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<QueueFamily> uniqueQueueFamilies = { queueFamilies.graphicsFamily.value(), queueFamilies.presentFamily.value() };
	std::map<QueueFamily, std::vector<float>> queueCreatePriorities;
	float queuePriority = 1.0f;
	for (QueueFamily queueFamily : uniqueQueueFamilies) {
		queueCreatePriorities[queueFamily] = std::vector<float>(queueFamily.queueCount, 1.0f);

		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily.index;
		queueCreateInfo.queueCount = queueFamily.queueCount;
		queueCreateInfo.pQueuePriorities = queueCreatePriorities[queueFamily].data();
		queueCreateInfos.push_back(queueCreateInfo);
	}

	// logical device creation settings
	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.pEnabledFeatures = nullptr;

	// logical device extensions
	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	// logical device validation layers (ignored by newer implementations)
	if (USE_VALIDATION_LAYERS) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	} else {
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device!");
	}

	// get all of the created queues
	graphicsQueues.resize(queueFamilies.graphicsFamily.value().queueCount);
	for (int i = 0; i < graphicsQueues.size(); ++i) {
		vkGetDeviceQueue(device, queueFamilies.graphicsFamily.value().index, i, &graphicsQueues[i]);
	}
	presentQueues.resize(queueFamilies.presentFamily.value().queueCount);
	for (int i = 0; i < presentQueues.size(); ++i) {
		vkGetDeviceQueue(device, queueFamilies.presentFamily.value().index, i, &presentQueues[i]);
	}
}

void Vulkan::createAllocator() {
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = physicalDevice;
    allocatorInfo.device = device;
    allocatorInfo.instance = instance;
    vmaCreateAllocator(&allocatorInfo, &allocator);
}

VkQueue& Vulkan::requestGraphicsQueue(bool important) {
	std::lock_guard<std::mutex> lock(queueMutex);
	
	VkQueue& queue = graphicsQueues[graphicsRoundRobin];
	graphicsRoundRobin = (graphicsRoundRobin + 1) % queueFamilies.graphicsFamily.value().queueCount;
	return queue;
}

VkQueue& Vulkan::requestPresentQueue(bool important) {
	std::lock_guard<std::mutex> lock(queueMutex);
	
	VkQueue& queue = presentQueues[presentRoundRobin];
	presentRoundRobin = (presentRoundRobin + 1) % queueFamilies.presentFamily.value().queueCount;
	return queue;
}
