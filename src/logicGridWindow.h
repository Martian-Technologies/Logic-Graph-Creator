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
#include "blockContainerTools.h"
#include "viewMannager.h"

class LogicGridWindow : public QWidget {
    Q_OBJECT
public:
    LogicGridWindow(QWidget* parent = nullptr);

    inline float getViewWidth() const { return viewMannager.getViewWidth(); }
    inline float getViewHeight() const { return (float)size().height() / (float)size().width() * viewMannager.getViewWidth(); }
    inline float getViewToPix() const { return (float)size().width() / viewMannager.getViewWidth(); }
    inline float getPixToView() const { return viewMannager.getViewWidth() / (float)size().width(); }
    Position gridPos(QPoint point) const;

    void updateSelectedItem();
    void zoom(float amount);
    // setup
    static const QPixmap& loadTileMap(const QString& filePath = QString());
    void setBlockContainer(BlockContainer* blockContainer);
    void setSelector(QTreeWidget* treeWidget);

protected:
    void paintEvent(QPaintEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    bool event(QEvent* event) override;

private:
    // update loop
    float dt = 0.016f;
    void updateLoop();
    QTimer* updateLoopTimer;

    // data
    BlockContainer* blockContainer;

    // helper classes
    BlockContainerTools blockContainerTools;
    ViewMannager viewMannager;

    // ui elements
    QTreeWidget* treeWidget;
};

#endif /* logicGridWindow_h */