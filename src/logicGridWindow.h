#ifndef logicGridWindow_h
#define logicGridWindow_h

#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QPixmap>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QTreeWidget>

#include "blockContainer.h"

class LogicGridWindow : public QWidget {
    Q_OBJECT
public:
    LogicGridWindow(QWidget* parent = nullptr);

    inline float getViewWidth() const { return viewWidth; }
    inline float getViewHeight() const { return (float)size().height() / (float)size().width() * viewWidth; }
    inline float getViewToPix() const { return (float)size().width() / viewWidth; }
    inline float getPixToView() const { return viewWidth / (float)size().width(); }
    QString getSelectedItem() const;
    Position gridPos(QPoint point) const;

    void zoom(float amount);
    // setup
    static const QPixmap& loadTileMap(const QString& filePath = QString());
    void setBlockContainer(BlockContainer* blockContainer);
    inline void setSelector(QTreeWidget* treeWidget) { this->treeWidget = treeWidget; }

protected:
    void paintEvent(QPaintEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    bool event(QEvent* event) override;

private:
    void updateLoop();
    BlockContainer* blockContainer;
    QTreeWidget* treeWidget;
    float viewCenterX;
    float viewCenterY;
    float viewWidth;
    bool movingLeft = false;
    bool movingRight = false;
    bool movingUp = false;
    bool movingDown = false;
    QTimer* updateLoopTimer;
};

#endif /* logicGridWindow_h */