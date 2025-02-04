#ifndef helpDialog_h
#define helpDialog_h

#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>

class helpDialog : public QDialog {
  Q_OBJECT
public:
  helpDialog(QWidget* parent = nullptr);
};

#endif /* helpDialog_h */