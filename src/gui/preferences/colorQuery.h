#ifndef COLOR_QUERY_H
#define COLOR_QUERY_H

#include <QColorDialog>
#include <QLineEdit>
#include <QWidget>


class ColorQuery : public QDialog {
    Q_OBJECT

public:
    ColorQuery(QWidget *parent = nullptr);

private slots:
     void updateColorValues(const QColor &color);

private:
    QColorDialog *colorDialog;
    QLineEdit *rgbValue;
    QLineEdit *hexValue;
};

#endif
