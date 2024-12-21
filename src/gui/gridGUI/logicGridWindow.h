#ifndef logicGridWindow_h
#define logicGridWindow_h

#include <QApplication>
#include <QTreeWidget>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QWidget>
#include <QPixmap>
#include <qevent.h>

#include "backend/container/blockContainer.h"
#include "renderer/QTRenderer.h"
#include "tools/blockContainerTool.h"
#include "gridRenderer.h"
#include "viewMannager.h"

class LogicGridWindow : public QWidget {
    Q_OBJECT
public:
    LogicGridWindow(QWidget* parent = nullptr);
    
    const BlockContainer* getBlockContainer() const {return blockContainer; }
	
    // data checkers
    inline bool insideWindow(const QPoint& point) const {return point.x() >= 0 && point.y() >= 0 && point.x() < size().width() && point.y() < size().height();}

    // dont call this func
    void updateSelectedItem();
    
    // setnup
    inline void initializeRenderer(const std::string& filePath) { renderer.initialize(filePath); };
    void setBlockContainer(BlockContainer* blockContainer);
    void setSelector(QTreeWidget* treeWidget);

protected:
    // events
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
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
    bool doUpdate;
    QTimer* updateLoopTimer;
    void updateLoop();

    // data
    BlockContainer* blockContainer;
    QPoint lastMousePos;

    // helper classes
    BlockContainerTool* tool;
    QTRenderer renderer;
    ViewMannager viewMannager;

    // ui elements
    QTreeWidget* treeWidget;
};

#endif /* logicGridWindow_h */
