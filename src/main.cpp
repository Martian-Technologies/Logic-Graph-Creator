#include "computerAPI/directoryManager.h"

#include "gpu/vulkanManager.h"
#include "platform/sdlInstance.h"
#include "platform/sdlWindow.h"

void setupVulkan();

int main(int argc, char* argv[]) {
	DirectoryManager::findDirectories();

	// Create SDL Instance 
	SdlInstance sdl;
	// Create SDL window
	SdlWindow window;
	
	// set up vulkan
	setupVulkan();

	
	// shutdown vulkan
	//Vulkan::getSingleton().destroy();

	return 0;
}

void setupVulkan() {
	// create instance and qVulkanInstance
	//Vulkan::getSingleton().createInstance();
	
	/*
	
	// create instance and device
	Vulkan::getSingleton().setupDevice(tempWindow.createSurface());

	// destroy temp surface
	tempWindow.destroySurface();
	tempWindow.destroy();*/
}
