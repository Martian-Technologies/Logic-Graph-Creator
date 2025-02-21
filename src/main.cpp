#include <QApplication>
#include <QStyleFactory>

#include <kddockwidgets/qtwidgets/ViewFactory.h>
#include <kddockwidgets/core/Platform.h>
#include <kddockwidgets/Config.h>

#include "gui/mainWindow.h"

int main(int argc, char* argv[]) {
	// Create QT Application
	logInfo("Creating QT Application (main.cpp)");
	QApplication app(argc, argv);
	app.setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
	app.setStyle(QStyleFactory::create("Fusion"));
	logInfo("Successfully created QT Application (main.cpp)");

	// Initialize KDDockWidgets
	logInfo("Initializing KDDockWidgets and setting flags (main.cpp)");
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
	logInfo("Successfully initialized KDDockWidgets and set flags (main.cpp)");
	
	// Create Main Window
	logInfo("Creating Main Window (main.cpp)");
	MainWindow window(options);
	window.show();
	logInfo("Successfully created Main Window (main.cpp)");

	return app.exec();
}
