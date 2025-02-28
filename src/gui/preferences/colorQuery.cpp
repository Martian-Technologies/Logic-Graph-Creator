#include "colorQuery.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QColor>
#include <QString>

ColorQuery::ColorQuery(QWidget *parent) : QDialog(parent) {
    // Set window title
    setWindowTitle("Color Query");

    // Create the color dialog
    colorDialog = new QColorDialog(this);
	colorDialog->setWindowFlags(Qt::SubWindow);
    colorDialog->setOption(QColorDialog::NoButtons, true);
    colorDialog->setOption(QColorDialog::ShowAlphaChannel, false);

    // Create labels and line edits for RGB and HEX values
    QLabel *rgbLabel = new QLabel("RGB:", this);
    rgbValue = new QLineEdit(this);
    rgbValue->setReadOnly(true);

    QLabel *hexLabel = new QLabel("HEX:", this);
    hexValue = new QLineEdit(this);
    hexValue->setReadOnly(true);

    // Create a button to close the dialog
    QPushButton *closeButton = new QPushButton("Close", this);
    connect(closeButton, &QPushButton::clicked, this, &ColorQuery::accept); // Use accept() to close the dialog

    // Layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(colorDialog);
    layout->addWidget(rgbLabel);
    layout->addWidget(rgbValue);
    layout->addWidget(hexLabel);
    layout->addWidget(hexValue);
    layout->addWidget(closeButton);

    setLayout(layout);

    // Connect color changed signal to update RGB and HEX values
    connect(colorDialog, &QColorDialog::currentColorChanged, this, &ColorQuery::updateColorValues);
}

void ColorQuery::updateColorValues(const QColor &color) {
    // Update RGB value
    rgbValue->setText(QString("R: %1, G: %2, B: %3")
                          .arg(color.red())
                          .arg(color.green())
                          .arg(color.blue()));

    // Update HEX value
    hexValue->setText(color.name(QColor::HexRgb));
}

