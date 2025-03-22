#include "computerAPI/directoryManager.h"

#include "platform/sdlInstance.h"
#include "gpu/vulkanManager.h"
#include "gui/window.h"

void setupVulkan();

int main(int argc, char* argv[]) {
	DirectoryManager::findDirectories();

	// Create SDL Instance 
	SdlInstance sdl;

	// set up vulkan
	setupVulkan();
	
	// Create window
	Window window;

	window.runLoop();

	
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
