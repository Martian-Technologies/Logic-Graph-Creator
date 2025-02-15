#include <QApplication>
#include <QStyleFactory>

#include <kddockwidgets/qtwidgets/ViewFactory.h>
#include <kddockwidgets/core/Platform.h>
#include <kddockwidgets/Config.h>

#include "gui/mainWindow.h"
#include "gpu/vulkanQtWindow.h"
#include "gpu/vulkanPlatformBridge.h"
#include "gpu/vulkanManager.h"

void setupVulkan();

int main(int argc, char* argv[]) {
	// initialize QT
	QApplication app(argc, argv);
	app.setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
	app.setStyle(QStyleFactory::create("Fusion"));

	// initialize vulkan
	setupVulkan();
	
	// initialize kdockwidgets
	KDDockWidgets::initFrontend(KDDockWidgets::FrontendType::QtWidgets);

	KDDockWidgets::Config::self().setSeparatorThickness(5);

	// KDDockWidgets::Core::ViewFactory::s_dropIndicatorType = KDDockWidgets::DropIndicatorType::Segmented;

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

	KDDockWidgets::MainWindowOptions options = KDDockWidgets::MainWindowOption_None;
	// options = KDDockWidgets::MainWindowOption_HasCentralGroup;
	// options |= KDDockWidgets::MainWindowOption_HasCentralWidget;
	
	MainWindow window(options);
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
