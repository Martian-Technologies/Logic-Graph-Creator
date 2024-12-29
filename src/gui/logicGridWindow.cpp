#include <QNativeGestureEvent>
#include <QGestureEvent>

#include "logicGridWindow.h"
#include "blockContainerView/blockContainerView.h"

// viewmanager is responsible for saying when the view is changed, which will
// trigger an update, right now the tool system is not. When the tool system
// is fed an input, we update. The tool system should work similar to viewManager

LogicGridWindow::LogicGridWindow(QWidget* parent) : QWidget(parent), blockContainerView(), mouseControls(true), treeWidget(nullptr) {
    // QT
    setFocusPolicy(Qt::StrongFocus);
    grabGesture(Qt::PinchGesture);
    setMouseTracking(true);

    // Loop
    updateLoopTimer = new QTimer(this);
    updateLoopTimer->setInterval((int)(updateInterval * 1000.0f));
    updateLoopTimer->start();
    connect(updateLoopTimer, &QTimer::timeout, this, &LogicGridWindow::updateLoop);


    float w = size().width();
    float h = size().height();

    // ViewManager
    blockContainerView.getViewManager().setAspectRatio(w / h);

    // Renderer
    blockContainerView.getRenderer().resize(w, h);
    blockContainerView.getRenderer().initializeTileSet(":logicTiles.png");
}

// update loop

void LogicGridWindow::updateLoop() {
    // update for re-render
    update();
}

// getters

Vec2 LogicGridWindow::pixelsToView(QPointF point) {
    return Vec2((float)point.x() / (float)rect().width(), (float)point.y() / (float)rect().height());
}

// setter functions

void LogicGridWindow::setSelector(QTreeWidget* treeWidget) {
    // disconnect the old tree
    if (this->treeWidget != nullptr)
        disconnect(this->treeWidget, &QTreeWidget::itemSelectionChanged, this, &LogicGridWindow::updateSelectedItem);
    // connect the new tree
    this->treeWidget = treeWidget;
    connect(treeWidget, &QTreeWidget::itemSelectionChanged, this, &LogicGridWindow::updateSelectedItem);
}

void LogicGridWindow::updateSelectedItem() {
    if (treeWidget) {
        for (QTreeWidgetItem* item : treeWidget->selectedItems()) {
            if (item) {
                QString str = item->text(0);
                if (str == "And") blockContainerView.getToolManager().selectBlock(BlockType::AND);
                else if (str == "Or") blockContainerView.getToolManager().selectBlock(BlockType::OR);
                else if (str == "Xor") blockContainerView.getToolManager().selectBlock(BlockType::XOR);
                else if (str == "Nand") blockContainerView.getToolManager().selectBlock(BlockType::NAND);
                else if (str == "Nor") blockContainerView.getToolManager().selectBlock(BlockType::NOR);
                else if (str == "Xnor") blockContainerView.getToolManager().selectBlock(BlockType::XNOR);
                else if (str == "Switch") blockContainerView.getToolManager().selectBlock(BlockType::SWITCH);
                else if (str == "Button") blockContainerView.getToolManager().selectBlock(BlockType::BUTTON);
                else if (str == "Tick Button") blockContainerView.getToolManager().selectBlock(BlockType::TICK_BUTTON);
                else {
                    blockContainerView.getToolManager().changeTool(str.toStdString());
                }
            }
            return;
        }
    }
}

void LogicGridWindow::setBlockContainer(std::shared_ptr<BlockContainerWrapper> blockContainer) {
    blockContainerView.setBlockContainer(blockContainer);
    updateSelectedItem();
}

void LogicGridWindow::setEvaluator(std::shared_ptr<Evaluator> evaluator) {
    blockContainerView.setEvaluator(evaluator);
}

// input events

bool LogicGridWindow::event(QEvent* event) {
    if (event->type() == QEvent::NativeGesture) {
        QNativeGestureEvent* nge = dynamic_cast<QNativeGestureEvent*>(event);
        if (nge && nge->gestureType() == Qt::ZoomNativeGesture) {
            if (blockContainerView.getEventRegister().doEvent(DeltaEvent("view zoom", 1 - nge->value()))) event->accept();
            return true;
        }
    } else if (event->type() == QEvent::Gesture) {
        QGestureEvent* gestureEvent = dynamic_cast<QGestureEvent*>(event);
        if (gestureEvent) {
            QPinchGesture* pinchGesture = dynamic_cast<QPinchGesture*>(gestureEvent->gesture(Qt::PinchGesture));
            if (blockContainerView.getEventRegister().doEvent(DeltaEvent("view zoom", 1 - pinchGesture->scaleFactor()))) event->accept();

            return true;
        }
    }
    return QWidget::event(event);
}

void LogicGridWindow::paintEvent(QPaintEvent* event) {
    QPainter* painter = new QPainter(this);
    blockContainerView.getRenderer().render(painter);

    uint64_t currentTime = QDateTime::currentMSecsSinceEpoch();
    if (lastFrameTime != 0) {
        std::string frameTime = std::to_string(currentTime - lastFrameTime) + "ms";
        painter->drawText(QRect(QPoint(0, 0), size()), Qt::AlignTop, QString(frameTime.c_str()));
    }
    lastFrameTime = currentTime;

    delete painter;
}

void LogicGridWindow::resizeEvent(QResizeEvent* event) {
    int w = event->size().width();
    int h = event->size().height();

    blockContainerView.getRenderer().resize(w, h);
    blockContainerView.getViewManager().setAspectRatio((float)w / (float)h);
}

void LogicGridWindow::wheelEvent(QWheelEvent* event) {
    QPoint numPixels = event->pixelDelta();
    if (numPixels.isNull()) numPixels = event->angleDelta() / 120 * /* pixels per step */ 10;

    if (!numPixels.isNull()) {
        if (mouseControls) {
            if (blockContainerView.getEventRegister().doEvent(DeltaEvent("view zoom", (float)(numPixels.y())/200.f))) event->accept();
        } else {
            if (blockContainerView.getEventRegister().doEvent(DeltaXYEvent(
                "view pan",
                numPixels.x() / getPixelsWidth() * blockContainerView.getViewManager().getViewWidth(),
                numPixels.y() / getPixelsHight() * blockContainerView.getViewManager().getViewHeight()
            ))) event->accept();
        }
    }
}

void LogicGridWindow::keyPressEvent(QKeyEvent* event) {
    if (/*event->modifiers() & Qt::MetaModifier && */event->key() == Qt::Key_Z) {
        blockContainerView.getBlockContainer()->undo();
        event->accept();
    } else if (/*event->modifiers() & Qt::MetaModifier && */event->key() == Qt::Key_Y) {
        blockContainerView.getBlockContainer()->redo();
        event->accept();
    } else if (event->key() == Qt::Key_Q) {
        if (blockContainerView.getEventRegister().doEvent(Event("tool rotate block ccw"))) {
            event->accept();
        }
    } else if (event->key() == Qt::Key_E) {
        if (blockContainerView.getEventRegister().doEvent(Event("tool rotate block cw"))) {
            event->accept();
        }
    }
}

void LogicGridWindow::keyReleaseEvent(QKeyEvent* event) {}

void LogicGridWindow::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        if (QGuiApplication::keyboardModifiers().testFlag(Qt::AltModifier)) {
            if (blockContainerView.getEventRegister().doEvent(PositionEvent("view attach anchor", blockContainerView.getViewManager().getPointerPosition()))) { event->accept(); return; }
        }
        if (blockContainerView.getEventRegister().doEvent(PositionEvent("tool primary activate", blockContainerView.getViewManager().getPointerPosition()))) event->accept();
    } else if (event->button() == Qt::RightButton) {
        if (blockContainerView.getEventRegister().doEvent(PositionEvent("tool secondary activate", blockContainerView.getViewManager().getPointerPosition()))) event->accept();
    }
}

void LogicGridWindow::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        if (blockContainerView.getEventRegister().doEvent(PositionEvent("view dettach anchor", blockContainerView.getViewManager().getPointerPosition()))) event->accept();
        else if (blockContainerView.getEventRegister().doEvent(PositionEvent("tool primary deactivate", blockContainerView.getViewManager().getPointerPosition()))) event->accept();
    } else if (event->button() == Qt::RightButton) {
        if (blockContainerView.getEventRegister().doEvent(PositionEvent("tool secondary deactivate", blockContainerView.getViewManager().getPointerPosition()))) event->accept();
    }
}

void LogicGridWindow::mouseMoveEvent(QMouseEvent* event) {
    QPoint point = event->pos();
    if (insideWindow(point)) { // inside the widget
        Vec2 viewPos = pixelsToView(point);
        if (blockContainerView.getEventRegister().doEvent(PositionEvent("pointer move", blockContainerView.getViewManager().viewToGrid(viewPos)))) event->accept();
    }
}

void LogicGridWindow::enterEvent(QEnterEvent* event) {
    Vec2 viewPos = pixelsToView(mapFromGlobal(QCursor::pos()));
    if (blockContainerView.getEventRegister().doEvent(PositionEvent("pointer enter view", blockContainerView.getViewManager().viewToGrid(viewPos)))) event->accept();
}

void LogicGridWindow::leaveEvent(QEvent* event) {
    Vec2 viewPos = pixelsToView(mapFromGlobal(QCursor::pos()));
    if (blockContainerView.getEventRegister().doEvent(PositionEvent("pointer exit view", blockContainerView.getViewManager().viewToGrid(viewPos)))) event->accept();
}
