#include <QApplication>

#include "gui/mainWindow.h"

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);
	app.setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);

	logInfo("wooho");
	logInfo("woohotwo");
	logError("oopsie");
	logFatalError("big oopsie");
	logWarning("no oopsie?");
	logWarning("no oopsie?", "Vulkan");

	MainWindow window;
	window.show();

	return app.exec();
}
