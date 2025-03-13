#include <QApplication>
#include <QStyleFactory>

#include <kddockwidgets/qtwidgets/ViewFactory.h>
#include <kddockwidgets/core/Platform.h>
#include <kddockwidgets/Config.h>

#include "computerAPI/directoryManager.h"
#include "gui/mainWindow.h"
#include "gpu/vulkanQtWindow.h"
#include "gpu/vulkanPlatformBridge.h"
#include "gpu/vulkanManager.h"

void setupVulkan();
KDDockWidgets::MainWindowOptions setUpKDDockWidgets();

int main(int argc, char* argv[]) {
	DirectoryManager::findDirectories();
	
	// Create QT application
	logInfo("Creating QT Application");
	QApplication app(argc, argv);
	app.setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
	app.setStyle(QStyleFactory::create("Fusion"));
	logInfo("Successfully created QT Application");

	// set up Vulkan
	setupVulkan();

	// set up KDDockWidgets
	KDDockWidgets::MainWindowOptions options = setUpKDDockWidgets();
	
	// Create main window
	logInfo("Creating Main Window");
	MainWindow window(options);
	window.show();
	logInfo("Successfully created Main Window");

	// run app
	int out = app.exec();

	// shutdown vulkan
	Vulkan::getSingleton().destroy();

	return out;
}

KDDockWidgets::MainWindowOptions setUpKDDockWidgets() {
	logInfo("Initializing KDDockWidgets and setting flags");
	KDDockWidgets::initFrontend(KDDockWidgets::FrontendType::QtWidgets);
	KDDockWidgets::Config::self().setSeparatorThickness(5);
	// Set flags
	auto internalFlags = KDDockWidgets::Config::self().internalFlags();
	// internalFlags |= KDDockWidgets::Config::InternalFlag_DontUseQtToolWindowsForFloatingWindows;
    KDDockWidgets::Config::self().setInternalFlags(internalFlags);
	auto flags = KDDockWidgets::Config::self().flags();
	// flags |= KDDockWidgets::Config::Flag_DontUseUtilityFloatingWindows;
	// flags |= KDDockWidgets::Config::Flag_AllowSwitchingTabsViaMenu;
	flags |= KDDockWidgets::Config::Flag_TabsHaveCloseButton;
	flags |= KDDockWidgets::Config::Flag_HideTitleBarWhenTabsVisible;
	flags |= KDDockWidgets::Config::Flag_AlwaysShowTabs;
	KDDockWidgets::Config::self().setFlags(flags);
	// Set options
	KDDockWidgets::MainWindowOptions options = KDDockWidgets::MainWindowOption_None;
	// options = KDDockWidgets::MainWindowOption_HasCentralGroup;
	// options |= KDDockWidgets::MainWindowOption_HasCentralWidget;
	logInfo("Successfully initialized KDDockWidgets and set flags");
	
	return options;
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
