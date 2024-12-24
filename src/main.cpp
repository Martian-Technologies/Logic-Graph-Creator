#include <QApplication>

#include "gui/mainWindow.h"

#include "gui/events/positionEvent.h"
#include "gui/events/eventRegister.h"

#include <iostream>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);

    MainWindow window;
    window.show();

    return app.exec();
}
