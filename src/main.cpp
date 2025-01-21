#include <QApplication>

#include "gui/mainWindow.h"
#include "gpu/vulkanPlatformBridge.h"

void setupVulkan();

int main(int argc, char* argv[]) {
	// initialize QT
	QApplication app(argc, argv);
	app.setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);

	// initialize vulkan
	setupVulkan();
	
	// create main window
	MainWindow window;
	window.show();

	// run app
	int out = app.exec();

	// shutdown vulkan
	Vulkan::Singleton().destroy();

	return out;
}

void setupVulkan() {
	// create instance and qVulkanInstance
	Vulkan::Singleton().createInstance();
	
	// goofy ahh hack to get temp surface for device selection
	QWindow tempWindow;
	tempWindow.setSurfaceType(QSurface::VulkanSurface);
	tempWindow.show();
	VulkanSurface surface(&tempWindow);
	
	// create instance and device
	Vulkan::Singleton().setupDevice(surface.getVkSurfaceKHR());

	// destroy temp surface
	tempWindow.destroy();
}
