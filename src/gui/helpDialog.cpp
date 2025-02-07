#include "ui_helpDialog.h"
#include "helpDialog.h"

HelpDialog::HelpDialog(QWidget* parent) : QDialog(parent), ui(new Ui::HelpDialog) {
  ui->setupUi(this);
  // layout->addWidget(label);
  // setLayout(layout);
}