#include "vulkanPlatformBridge.h"

#include "vulkanManager.h"

// this file exists because apple is stupid
std::vector<std::string> getRequiredInstanceExtensions() {
#ifdef __APPLE__
	// we don't have a library for this on apple, so this will have to work. May not age very well
	// we should expand the device picking code to set these more accurately
	std::vector<std::string> extensions = { "VK_KHR_surface", "VK_KHR_portability_enumeration", "VK_KHR_portability_subset", "VK_MVK_macos_surface"};
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
