#include "vulkanPlatformBridge.h"

#include <QVulkanInstance>

#include "vulkanManager.h"

// this file exists because apple is stupid

std::vector<std::string> getRequiredInstanceExtensions() {
	#if __APPLE__
	
	#else

	// goofy ahh hack to get required extension list
	QVulkanInstance tempInstance;
	tempInstance.create();
	QByteArrayList qExtensions = tempInstance.extensions();
	std::vector<std::string> extensions(qExtensions.begin(), qExtensions.end());
	// std::cout << extensions[0] << std::endl;
	// std::cout << extensions[1] << std::endl;
	// std::cout << extensions[2] << std::endl;
	// std::cout << extensions[3] << std::endl;
	tempInstance.destroy();
	
	return extensions;
	
	#endif
}

VkSurfaceKHR createSurface(QWindow *window) {
	#if __APPLE__
	
	#else
	
	#endif
}

void destroySurface(VkSurfaceKHR surface) {
	#if __APPLE__
	vkDestroySurfaceKHR(Vulkan::Instance(), surface, nullptr);
	#else
	// qt automatically does it
	#endif
}
