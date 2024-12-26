#ifndef logicGridWindow_h
#define logicGridWindow_h

#include <QApplication>
#include <QTreeWidget>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QWidget>
#include <QPixmap>

#include "../blockContainerView/blockContainerView.h"
#include "connectionRenderer.h"
#include "blockRenderer.h"
#include "gridRenderer.h"
#include "viewMannager.h"

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
    const BlockRenderer& getBlockRenderer() const {return blockRenderer;}
    const ConnectionRenderer& getConnectionRenderer() const {return connectionRenderer;}
    const BlockContainer* getBlockContainer() const {return blockContainer->getBlockContainer();}


    // data checkers
    Position gridPos(const QPoint& point) const;
    QPoint windowPos(const Position& point, bool center = false) const;
    inline bool insideWindow(const QPoint& point) const {return point.x() >= 0 && point.y() >= 0 && point.x() < size().width() && point.y() < size().height();}
    bool insideWindow(const Position& point) const {return insideWindow(windowPos(point));}

    // dont call this func
    void updateSelectedItem();
    
    // setup
    inline void loadTileMap(const QString& filePath) {blockRenderer.loadTileMap(filePath);};
    void setBlockContainer(BlockContainerWrapper* blockContainer);
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
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    bool event(QEvent* event) override;

private:
    // update loop
    float dt;
    void updateLoop();
    QTimer* updateLoopTimer;
    bool doUpdate;

    // data
    BlockContainerView blockContainerView;
    BlockContainerWrapper* blockContainer;
    QPoint lastMousePos;

    // helper classes
    ConnectionRenderer connectionRenderer;
    BlockRenderer blockRenderer;
    GridRenderer gridRenderer;
    ViewMannager viewMannager;

    // ui elements
    QTreeWidget* treeWidget;
};

#endif /* logicGridWindow_h */
