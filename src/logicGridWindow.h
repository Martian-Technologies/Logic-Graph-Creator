#ifndef logicGridWindow_h
#define logicGridWindow_h

#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QPixmap>
#include <QWheelEvent>

#include "blockContainer.h"

class LogicGridWindow : public QWidget {
    Q_OBJECT
public:
    LogicGridWindow(QWidget* parent = nullptr) : QWidget(parent), blockContainer(), viewCenterX(0), viewCenterY(0), viewWidth(10) {
        grabGesture(Qt::PinchGesture);
    }

    inline float getViewWidth() const { return viewWidth; }
    inline float getViewHeight() const { return (float)size().height() / (float)size().width() * viewWidth; }
    inline float getViewToPix() const { return (float)size().width() / viewWidth; }
    inline float getPixToView() const { return viewWidth / (float)size().width(); }
    void zoom(float amount);

    // setup
    static const QPixmap& loadTileMap(const QString& filePath = QString());
    void setBlockContainer(BlockContainer* blockContainer);


protected:
    void paintEvent(QPaintEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    bool event(QEvent* event) override;

private:
    BlockContainer* blockContainer;
    float viewCenterX;
    float viewCenterY;
    float viewWidth;
};

#endif /* logicGridWindow_h */