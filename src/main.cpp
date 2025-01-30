#include <QApplication>

#include "gui/mainWindow.h"
#include "gpu/vulkanQtWindow.h"
#include "gpu/vulkanPlatformBridge.h"
#include "gpu/vulkanManager.h"

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
	Vulkan::getSingleton().destroy();

	return out;
}

void setupVulkan() {
	// create instance and qVulkanInstance
	Vulkan::getSingleton().createInstance();
	
	// goofy ahh hack to get temp surface for device selection
	VulkanQtWindow tempWindow;
	tempWindow.setSurfaceType(QSurface::VulkanSurface);
	tempWindow.show();
	
	// create instance and device
	Vulkan::getSingleton().setupDevice(tempWindow.createSurface());

	// destroy temp surface
	tempWindow.destroySurface();
	tempWindow.destroy();
}
