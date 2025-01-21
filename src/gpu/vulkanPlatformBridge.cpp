#include "vulkanPlatformBridge.h"

#include "vulkanManager.h"

#ifdef __APPLE__
#include <vulkan/vulkan_metal.h>
#endif

// this file exists because apple is stupid

std::vector<std::string> getRequiredInstanceExtensions() {
#ifdef __APPLE__
	// we don't have a library for this on apple, so this will have to work. May not age very well
	std::vector<std::string> extensions = { "VK_KHR_surface", "VK_KHR_portability_enumeration", "VK_EXT_DEBUG_UTILS", "VK_EXT_metal_surface"};
	return extensions;
	
#else
	// goofy ahh hack to get required extension list
	QVulkanInstance tempInstance;
	tempInstance.create();
	QByteArrayList qExtensions = tempInstance.extensions();
	std::vector<std::string> extensions(qExtensions.begin(), qExtensions.end());
	tempInstance.destroy();
	
	return extensions;
	
#endif
}

VulkanSurface::VulkanSurface(QWindow* window) {
#ifdef __APPLE__
	VkMetalSurfaceCreateInfoEXT surfaceInfo = {};
	surfaceInfo.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
	auto winID = window->winId();
	// we're going to hope this works
	surfaceInfo.pLayer = static_cast<CAMetalLayer*>(&winID);

	PFN_vkCreateMetalSurfaceEXT vkCreateMetalSurfaceEXT =
		(PFN_vkCreateMetalSurfaceEXT)vkGetInstanceProcAddr(Vulkan::Instance(), "vkCreateMetalSurfaceEXT");

	vkCreateMetalSurfaceEXT(Vulkan::Instance(), &surfaceInfo, nullptr, &surface);
#else
	qVulkanInstance.setVkInstance(Vulkan::Instance());
	qVulkanInstance.create();
	window->setVulkanInstance(&qVulkanInstance);
	surface = QVulkanInstance::surfaceForWindow(window);
#endif
}

VulkanSurface::~VulkanSurface() {
#ifdef __APPLE__
	vkDestroySurfaceKHR(Vulkan::Instance(), surface, nullptr);
#else
	// QT does it
#endif
}
