#ifndef helpDialog_h
#define helpDialog_h

#include <QDialog>

namespace Ui {
	class HelpDialog;
}

class HelpDialog : public QDialog {
  Q_OBJECT
public:
  HelpDialog(QWidget* parent = nullptr);

private:
  Ui::HelpDialog* ui;
};

#endif /* helpDialog_h */