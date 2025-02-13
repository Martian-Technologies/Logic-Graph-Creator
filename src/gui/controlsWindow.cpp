#include "ui_controls.h"

#include "controlsWindow.h"

ControlsWindow::ControlsWindow(QWidget* parent) : QWidget(parent), ui(new Ui::Controls) {
	ui->setupUi(this);

	connect(ui->StartSim, &QPushButton::clicked, this, &ControlsWindow::updateSimState);
	connect(ui->UseSpeed, &QCheckBox::checkStateChanged, this, &ControlsWindow::updateSimUseSpeed);
	connect(ui->Speed, &QDoubleSpinBox::valueChanged, this, &ControlsWindow::updateSimSpeed);
}

ControlsWindow::~ControlsWindow() {
	delete ui;
}

void ControlsWindow::updateSimState(bool state) {
	emit setSimState(state);
}

void ControlsWindow::updateSimUseSpeed(Qt::CheckState state) {
	emit simUseSpeed(state);
}

void ControlsWindow::updateSimSpeed(double speed) {
	emit setSimSpeed(speed);
}
