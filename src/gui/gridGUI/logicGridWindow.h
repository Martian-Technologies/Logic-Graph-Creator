#ifndef logicGridWindow_h
#define logicGridWindow_h

#include <QApplication>
#include <QTreeWidget>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QWidget>
#include <QTimer>
#include <qvectornd.h>

#include <memory>

#include "../blockContainerView/blockContainerView.h"
#include "../blockContainerView/renderer/QTRenderer.h"

class LogicGridWindow : public QWidget {
    Q_OBJECT
public:
    LogicGridWindow(QWidget* parent = nullptr);

    // getters
    const BlockContainer* getBlockContainer() const { return blockContainerView.getBlockContainer()->getBlockContainer(); }

    // data checkers
    inline bool insideWindow(const QPoint& point) const { return point.x() >= 0 && point.y() >= 0 && point.x() < size().width() && point.y() < size().height(); }

    // setup
    void setBlockContainer(std::shared_ptr<BlockContainerWrapper> blockContainer);
    void setSelector(QTreeWidget* treeWidget);

    // dont call this func
    void updateSelectedItem();

    QVector2D pixelsToView(QPoint point);
    inline float getPixelsWidth() { return (float)rect().width(); }
    inline float getPixelsHight() { return (float)rect().height(); }


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
    QTimer* updateLoopTimer;
    float updateInterval = 0.016f;
    void updateLoop();

    // data
    BlockContainerView<QTRenderer> blockContainerView;
    bool mouseControls;

    // ui elements
    QTreeWidget* treeWidget;
};

#endif /* logicGridWindow_h */
