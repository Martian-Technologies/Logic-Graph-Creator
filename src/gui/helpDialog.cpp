#include "ui_helpDialog.h"
#include "helpDialog.h"

HelpDialog::HelpDialog(QWidget* parent) : QDialog(parent), ui(new Ui::HelpDialog) {
  ui->setupUi(this);
  ui->tableWidget->setColumnWidth(0, 300);
  ui->tableWidget->setColumnWidth(1, 500);
  ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
  setWindowTitle("Help");
  resize(400, 200);
}