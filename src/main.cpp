#include <iostream>

#include "mainwindow.h"

#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);

    MainWindow window;
    window.show();

    return app.exec();
}
