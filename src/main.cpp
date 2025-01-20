#include <QApplication>

#include <kddockwidgets/core/Platform.h>

#include "gui/mainWindow.h"

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);
	app.setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);

	KDDockWidgets::initFrontend(KDDockWidgets::FrontendType::QtWidgets);

	MainWindow window;
	window.show();

	return app.exec();
}
