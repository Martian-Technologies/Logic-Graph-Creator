#include "helpDialog.h"

helpDialog::helpDialog(QWidget* parent) {
  setWindowTitle("Help");
  resize(300, 200);

  QVBoxLayout *layout = new QVBoxLayout(this);
  QLabel *label = new QLabel("Where help dialog will go", this);
  layout->addWidget(label);
  setLayout(layout);
}