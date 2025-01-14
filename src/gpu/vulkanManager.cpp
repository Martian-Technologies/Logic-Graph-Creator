#include "vulkanManager.h"

#include "vulkanDevice.h"

#include <cassert>
#include <cstring>

// Constants ---------------------------------------------------------------------------
const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};
const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

// Functions ---------------------------------------------------------------------------
void VulkanManager::createInstance(const std::vector<const char*>& requiredExtensions) {
	// confirm we have validation layers if we need them
	if (DEBUG && !checkValidationLayerSupport()) {
		fail("validation layers requested, but not available!");
	}

	// set applicaiton information
	VkApplicationInfo appInfo {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Gatality";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_3;

	// start instance creation
	VkInstanceCreateInfo createInfo {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	// add extensions
	auto extensions = requiredExtensions;
	if (DEBUG) { extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME); }
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	// enable validation layers3
	if (DEBUG) {
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

void VulkanManager::setUpDevice(VkSurfaceKHR surface) {
	pickPhysicalDevice(surface);
	createLogicalDevice(surface);
}

VulkanGraphicsView VulkanManager::createGraphicsView() {
	int graphicsIndex = graphicsRoundRobin++ % graphicsQueues.size();
	int presentIndex = presentRoundRobin++ % presentQueues.size();

	return { device, physicalDevice, queueFamilies, graphicsQueues[graphicsIndex], presentQueues[presentIndex] };
}

void VulkanManager::destroy() {
	vkDestroyDevice(device, nullptr);
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

void VulkanManager::pickPhysicalDevice(VkSurfaceKHR idealSurface) {
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if (deviceCount == 0) {
		fail("failed to find GPUs with Vulkan support!");
	}
	
	// get vulkan supported devices
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	// find first suitable device
	for (const auto& device : devices) {
		if (isDeviceSuitable(device, idealSurface)) {
			physicalDevice = device;
			return;
		}
	}

	fail("failed to find suitable GPU");
}

bool VulkanManager::isDeviceSuitable(VkPhysicalDevice physicalDevice, VkSurfaceKHR idealSurface) {
	// check queue graphics feature support
	QueueFamilies indices = findQueueFamilies(physicalDevice, idealSurface);
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

void VulkanManager::createLogicalDevice(VkSurfaceKHR surface) {
	// create all the queues from unique queues that we need
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	queueFamilies = findQueueFamilies(physicalDevice, surface);
	std::set<QueueFamily> uniqueQueueFamilies = { queueFamilies.graphicsFamily.value(), queueFamilies.presentFamily.value() };
	float queuePriority = 1.0f;
	for (QueueFamily queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily.index;
		queueCreateInfo.queueCount = queueFamily.queueCount;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	// no features to enable for now
	VkPhysicalDeviceFeatures deviceFeatures{};

	// logical device creation settings
	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.pEnabledFeatures = &deviceFeatures;

	// logical device extensions
	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	// logical device validation layers (ignored by newer implementations)
	if (DEBUG) {
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
