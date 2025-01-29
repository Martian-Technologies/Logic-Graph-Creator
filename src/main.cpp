#include <QApplication>

#include "gui/mainWindow.h"

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);
	app.setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);

	logInfo("wooho");

	MainWindow window;
	window.show();

	return app.exec();
}
