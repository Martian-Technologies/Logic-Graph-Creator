#include "vulkanInstance.h"

VulkanInstance* VulkanInstance::singleton = nullptr;

static VKAPI_ATTR VkBool32 VKAPI_CALL vulkanDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData);

VulkanInstance::VulkanInstance() {
	logInfo("Initializing Vulkan...", "Vulkan");

	// Set singleton (kind of goofy)
	singleton = this;

	// Start creating vulkan instance
	vkb::InstanceBuilder instanceBuilder(vkGetInstanceProcAddr);
	
	// Get Vulkan system information
	auto systemInfoRet = vkb::SystemInfo::get_system_info(vkGetInstanceProcAddr);
	if (!systemInfoRet) { throwFatalError("Could not fetch Vulkan system info. Error: " + systemInfoRet.error().message()); }
	auto systemInfo = systemInfoRet.value();

#ifdef DEBUG
	#if not __APPLE__
		// Enable validation layers
		if (systemInfo.validation_layers_available){
			instanceBuilder.enable_validation_layers().set_debug_callback(&vulkanDebugCallback);
		}
	#endif
#endif
	
	// Create Vulkan Instance
	instanceBuilder.set_app_name("Gatality");
	instanceBuilder.set_engine_name("Jack Jamison's Wacky-n-Wonderful Gatality Render-a-tron 3000 million!");
	instanceBuilder.require_api_version(1,0,0);
	auto instanceRet = instanceBuilder.build();	
	if (!instanceRet) { throwFatalError("Failed to create Vulkan instance. Error: " + instanceRet.error().message()); }
	instance = instanceRet.value();
}

VulkanInstance::~VulkanInstance() {
	logInfo("Shutting down Vulkan..", "Vulkan");

	if (allocator.has_value()) vmaDestroyAllocator(allocator.value());
	if (device.has_value()) {
		vkDestroyFence(device->device, immediateFence, nullptr);
		vkDestroyCommandPool(device->device, immediateCommandPool, nullptr);
		vkb::destroy_device(device.value());
	}
	vkb::destroy_instance(instance);
}

void VulkanInstance::createAllocator() {
	VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = physicalDevice.value();
    allocatorInfo.device = device.value().device;
    allocatorInfo.instance = instance;
	
	VmaAllocator alloc;
    if (vmaCreateAllocator(&allocatorInfo, &alloc) != VK_SUCCESS) { throwFatalError("Could not create Vulkan VMA allocator.");}
	allocator = alloc;
}

void VulkanInstance::initializeImmediateSubmission() {
	VkCommandPoolCreateInfo commandPoolInfo = {};
	commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolInfo.pNext = nullptr;
	commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	commandPoolInfo.queueFamilyIndex = graphicsQueue->index;
	vkCreateCommandPool(device->device, &commandPoolInfo, nullptr, &immediateCommandPool);

	VkCommandBufferAllocateInfo commandBufferInfo = {};
	commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferInfo.pNext = nullptr;
	commandBufferInfo.commandPool = immediateCommandPool;
	commandBufferInfo.commandBufferCount = 1;
	commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	vkAllocateCommandBuffers(device->device, &commandBufferInfo, &immediateCommandBuffer);

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.pNext = nullptr;
	vkCreateFence(device->device, &fenceInfo, nullptr, &immediateFence);
}

void VulkanInstance::ensureDeviceCreation(VkSurfaceKHR surfaceForPresenting) {
	if (!device.has_value()) {
		logInfo("Creating Vulkan Device...", "Vulkan");
		
		// Select physical device
		vkb::PhysicalDeviceSelector physicalDeviceSelector(instance);
		physicalDeviceSelector.set_surface(surfaceForPresenting);
		auto physicalDeviceRet = physicalDeviceSelector.select();
		if (!physicalDeviceRet) { throwFatalError("Could not select Vulkan physical device. Error: " + physicalDeviceRet.error().message()); }
		physicalDevice = physicalDeviceRet.value().physical_device;

		// Build device
		vkb::DeviceBuilder deviceBuilder(physicalDeviceRet.value());
		auto deviceRet = deviceBuilder.build();
		if (!deviceRet) { throwFatalError("Could not create Vulkan device. Error: " + deviceRet.error().message()); }
		device = deviceRet.value();

		// Get queues
		graphicsQueue = { device->get_queue(vkb::QueueType::graphics).value(),
			device->get_queue_index(vkb::QueueType::graphics).value() };
		presentQueue = { device->get_queue(vkb::QueueType::present).value(),
			device->get_queue_index(vkb::QueueType::present).value() };

		// Create vma allocator
		createAllocator();

		// Initialize immediate submission
		initializeImmediateSubmission();
	}
}

void VulkanInstance::immediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function) {
	// set up
	vkResetFences(device->device, 1, &immediateFence);
	vkResetCommandBuffer(immediateCommandBuffer, 0);

	// start command buffer
	VkCommandBufferBeginInfo cmdBeginInfo{};
	cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	vkBeginCommandBuffer(immediateCommandBuffer, &cmdBeginInfo);

	// run commands
	function(immediateCommandBuffer);

	// end command buffer
	vkEndCommandBuffer(immediateCommandBuffer);

	// submit command buffer
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &immediateCommandBuffer;
	{
		std::lock_guard<std::mutex> lock(graphicsSubmitMux);
		if (vkQueueSubmit(graphicsQueue->queue, 1, &submitInfo, immediateFence) != VK_SUCCESS) {
			throwFatalError("failed to submit draw command buffer!");
		}
	}

	// wait for completion
	vkWaitForFences(device->device, 1, &immediateFence, VK_TRUE, UINT64_MAX);
}

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
