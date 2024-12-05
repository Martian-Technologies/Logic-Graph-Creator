#ifndef logicGridWindow_h
#define logicGridWindow_h

#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QPixmap>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QTreeWidget>

#include "../../backend/blockContainer.h"
#include "tools/blockContainerTool.h"
#include "viewMannager.h"
#include "blockRenderer.h"

class LogicGridWindow : public QWidget {
    Q_OBJECT
public:
    LogicGridWindow(QWidget* parent = nullptr);

    // data getter
    inline float getViewWidth() const { return viewMannager.getViewWidth(); }
    inline float getViewHeight() const { return (float)size().height() / (float)size().width() * viewMannager.getViewWidth(); }
    inline float getViewToPix() const { return (float)size().width() / viewMannager.getViewWidth(); }
    inline float getPixToView() const { return viewMannager.getViewWidth() / (float)size().width(); }
    inline float getViewCenterX() const { return viewMannager.getViewCenterX(); }
    inline float getViewCenterY() const { return viewMannager.getViewCenterY(); }
    Position gridPos(const QPoint& point) const;
    QPoint windowPos(const Position& point, bool center = false) const;

    // dont call this func
    void updateSelectedItem();
    
    // setup
    inline void loadTileMap(const QString& filePath) {blockRenderer.loadTileMap(filePath);};
    void setBlockContainer(BlockContainer* blockContainer);
    void setSelector(QTreeWidget* treeWidget);

protected:
    // events
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
    BlockRenderer blockRenderer;
    BlockContainerTool* tool;
    ViewMannager viewMannager;

    // ui elements
    QTreeWidget* treeWidget;
};

#endif /* logicGridWindow_h */