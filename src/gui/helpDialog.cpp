#include "ui_helpDialog.h"
#include "helpDialog.h"

HelpDialog::HelpDialog(QWidget* parent) : QDialog(parent), ui(new Ui::HelpDialog) {
  ui->setupUi(this);
  //setWindowTitle("Help");
  //resize(300, 200);

  // QVBoxLayout *layout = new QVBoxLayout(this);
  // QLabel *label = new QLabel("Where help dialog will go", this);
  // layout->addWidget(label);
  // setLayout(layout);
}