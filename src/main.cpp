#include <QApplication>
#include <QStyleFactory>

#include <kddockwidgets/qtwidgets/ViewFactory.h>
#include <kddockwidgets/core/Platform.h>
#include <kddockwidgets/Config.h>

#include "gui/mainWindow.h"

#include "util/config/config.h"

int main(int argc, char* argv[]) {
	// Creates config
	createConfig();


	// Create QT Application
	QApplication app(argc, argv);
	app.setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
	app.setStyle(QStyleFactory::create("Fusion"));

	// Initialize KDDockWidgets
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
	
	// Create Main Window
	MainWindow window(options);
	window.show();

	return app.exec();
}
