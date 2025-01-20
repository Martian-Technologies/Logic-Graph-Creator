#include <QApplication>

#include "gui/mainWindow.h"

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
	// goofy ahh hack to get required extension list
	QVulkanInstance tempInstance;
	tempInstance.create();
	QByteArrayList qExtensions = tempInstance.extensions();
	std::vector<const char*> extensions(qExtensions.begin(), qExtensions.end());
	tempInstance.destroy();
	
	// create instance and qVulkanInstance
	Vulkan::Singleton().createInstance(extensions);
	QVulkanInstance qVulkanInstance;
	qVulkanInstance.setVkInstance(Vulkan::Instance());
	qVulkanInstance.create();
	
	// goofy ahh hack to get temp surface for device selection
	QWindow tempWindow;
	tempWindow.setSurfaceType(QSurface::VulkanSurface);
	tempWindow.setVulkanInstance(&qVulkanInstance);
	tempWindow.show();
	VkSurfaceKHR tempSurface = QVulkanInstance::surfaceForWindow(&tempWindow);
	
	// create instance and device
	Vulkan::Singleton().setupDevice(tempSurface);

	// destroy temp surface
	tempWindow.destroy();
}
