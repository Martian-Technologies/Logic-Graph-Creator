#include "circuitViewWidget.h"

#include <QJsonDocument>
#include <QFileDialog>
#include <QMessageBox>
#include <QJsonArray>
#include <QJsonObject>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QShortcut>
#include <QNativeGestureEvent>
#include <QGestureEvent>

#include "circuitView/circuitView.h"

CircuitViewWidget::CircuitViewWidget(QWidget *parent, CircuitFileManager *fileManager)
    : QWidget(parent), fileManager(fileManager), mouseControls(false), treeWidget(nullptr) {
	// qt settings
	setFocusPolicy(Qt::StrongFocus);
	grabGesture(Qt::PinchGesture);
	setMouseTracking(true);
	setAcceptDrops(true);

	// set up update loop
	updateLoopTimer = new QTimer(this);
	updateLoopTimer->setInterval((int)(updateInterval * 1000.0f));
	updateLoopTimer->start();
	connect(updateLoopTimer, &QTimer::timeout, this, &CircuitViewWidget::updateLoop);

	float w = size().width();
	float h = size().height();

	// set viewmanager aspect ratio to begin with
	circuitView.getViewManager().setAspectRatio(w / h);

	// initialize QTRenderer with width and height + tileset
	circuitView.getRenderer().resize(w, h);
	circuitView.getRenderer().initializeTileSet(":logicTiles.png");

	QShortcut* saveShortcut = new QShortcut(QKeySequence("Ctrl+S"), this);
	connect(saveShortcut, &QShortcut::activated, this, &CircuitViewWidget::save);
}

void CircuitViewWidget::updateLoop() {
	// update for re-render
	update();
}

// input events ------------------------------------------------------------------------------

bool CircuitViewWidget::event(QEvent* event) {
	if (event->type() == QEvent::NativeGesture) {
		QNativeGestureEvent* nge = dynamic_cast<QNativeGestureEvent*>(event);
		if (nge && nge->gestureType() == Qt::ZoomNativeGesture) {
			if (circuitView.getEventRegister().doEvent(DeltaEvent("view zoom", nge->value() - 1))) event->accept();
			return true;
		}
	} else if (event->type() == QEvent::Gesture) {
		QGestureEvent* gestureEvent = dynamic_cast<QGestureEvent*>(event);
		if (gestureEvent) {
			QPinchGesture* pinchGesture = dynamic_cast<QPinchGesture*>(gestureEvent->gesture(Qt::PinchGesture));
			if (circuitView.getEventRegister().doEvent(DeltaEvent("view zoom", pinchGesture->scaleFactor() - 1))) event->accept();

			return true;
		}
	}
	return QWidget::event(event);
}

void CircuitViewWidget::paintEvent(QPaintEvent* event) {
	QPainter* painter = new QPainter(this);

	circuitView.getRenderer().render(painter);

	// rolling average for frame time
	pastFrameTimes.push_back(circuitView.getRenderer().getLastFrameTimeMs());
	int numPops = pastFrameTimes.size() - numTimesInAverage;
	for (int i = 0; i < numPops; ++i) {
		pastFrameTimes.pop_front();
	}
	float average = std::accumulate(pastFrameTimes.begin(), pastFrameTimes.end(), 0.0f) / (float)pastFrameTimes.size();

	// avg frame
	std::stringstream stream1;
	stream1 << std::fixed << std::setprecision(3) << average;
	std::string frameTimeStr = "avg frame: " + stream1.str() + "ms";
	painter->drawText(QRect(QPoint(0, 0), size()), Qt::AlignTop, QString(frameTimeStr.c_str()));

	// tps
	std::stringstream stream2;
	stream2 << std::fixed << std::setprecision(3) << circuitView.getEvaluatorStateInterface().getRealTickrate();
	std::string tpsStr = "tps: " + stream2.str();
	painter->drawText(QRect(QPoint(0, 16), size()), Qt::AlignTop, QString(tpsStr.c_str()));

	delete painter;
}

void CircuitViewWidget::resizeEvent(QResizeEvent* event) {
	int w = event->size().width();
	int h = event->size().height();

	circuitView.getRenderer().resize(w, h);
	circuitView.getViewManager().setAspectRatio((float)w / (float)h);
}

void CircuitViewWidget::wheelEvent(QWheelEvent* event) {
	QPoint numPixels = event->pixelDelta();
	if (numPixels.isNull()) numPixels = event->angleDelta() / 120 * /* pixels per step */ 10;

	if (!numPixels.isNull()) {
		if (mouseControls) {
			if (circuitView.getEventRegister().doEvent(DeltaEvent("view zoom", (float)(numPixels.y()) / 200.f))) event->accept();
		} else {
			if (event->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier)) {
				// do zoom
				if (circuitView.getEventRegister().doEvent(DeltaEvent("view zoom", (float)(numPixels.y()) / 100.f))) event->accept();
			}
			else {
				if (circuitView.getEventRegister().doEvent(DeltaXYEvent(
					"view pan",
					numPixels.x() / getPixelsWidth() * circuitView.getViewManager().getViewWidth(),
					numPixels.y() / getPixelsHeight() * circuitView.getViewManager().getViewHeight()
				))) event->accept();
			}
		}
	}
}

void CircuitViewWidget::keyPressEvent(QKeyEvent* event) {
	if (/*event->modifiers() & Qt::MetaModifier && */event->key() == Qt::Key_Z) {
		circuitView.getCircuit()->undo();
		event->accept();
	} else if (/*event->modifiers() & Qt::MetaModifier && */event->key() == Qt::Key_Y) {
		circuitView.getCircuit()->redo();
		event->accept();
	} else if (event->key() == Qt::Key_Q) {
		if (circuitView.getEventRegister().doEvent(Event("tool rotate block ccw"))) {
			event->accept();
		}
	} else if (event->key() == Qt::Key_E) {
		if (circuitView.getEventRegister().doEvent(Event("tool rotate block cw"))) {
			event->accept();
		}
	}
}

void CircuitViewWidget::keyReleaseEvent(QKeyEvent* event) { }

void CircuitViewWidget::mousePressEvent(QMouseEvent* event) {
	if (event->button() == Qt::LeftButton) {
		if (QGuiApplication::keyboardModifiers().testFlag(Qt::AltModifier)) {
			if (circuitView.getEventRegister().doEvent(PositionEvent("view attach anchor", circuitView.getViewManager().getPointerPosition()))) { event->accept(); return; }
		}
		if (circuitView.getEventRegister().doEvent(PositionEvent("tool primary activate", circuitView.getViewManager().getPointerPosition()))) event->accept();
	} else if (event->button() == Qt::RightButton) {
		if (circuitView.getEventRegister().doEvent(PositionEvent("tool secondary activate", circuitView.getViewManager().getPointerPosition()))) event->accept();
	}
}

void CircuitViewWidget::mouseReleaseEvent(QMouseEvent* event) {
	if (event->button() == Qt::LeftButton) {
		if (circuitView.getEventRegister().doEvent(PositionEvent("view dettach anchor", circuitView.getViewManager().getPointerPosition()))) event->accept();
		else if (circuitView.getEventRegister().doEvent(PositionEvent("tool primary deactivate", circuitView.getViewManager().getPointerPosition()))) event->accept();
	} else if (event->button() == Qt::RightButton) {
		if (circuitView.getEventRegister().doEvent(PositionEvent("tool secondary deactivate", circuitView.getViewManager().getPointerPosition()))) event->accept();
	}
}

void CircuitViewWidget::mouseMoveEvent(QMouseEvent* event) {
	QPoint point = event->pos();
	if (insideWindow(point)) { // inside the widget
		Vec2 viewPos = pixelsToView(point);
		if (circuitView.getEventRegister().doEvent(PositionEvent("pointer move", circuitView.getViewManager().viewToGrid(viewPos)))) event->accept();
	}
}

void CircuitViewWidget::enterEvent(QEnterEvent* event) {
	// grab focus so key inputs work without clicking
	setFocus(Qt::MouseFocusReason);

	Vec2 viewPos = pixelsToView(mapFromGlobal(QCursor::pos()));
	if (circuitView.getEventRegister().doEvent(PositionEvent("pointer enter view", circuitView.getViewManager().viewToGrid(viewPos)))) event->accept();
}

void CircuitViewWidget::leaveEvent(QEvent* event) {
	Vec2 viewPos = pixelsToView(mapFromGlobal(QCursor::pos()));
	if (circuitView.getEventRegister().doEvent(PositionEvent("pointer exit view", circuitView.getViewManager().viewToGrid(viewPos)))) event->accept();
}

void CircuitViewWidget::save() {
    if (fileManager) {
        QString filePath = QFileDialog::getSaveFileName(this, "Save Circuit", "", "Circuit Files (*.circuit);;All Files (*)");
        if (!filePath.isEmpty()) {
            fileManager->saveToFile(filePath, circuitView.getCircuit()->getCircuitId());
        }
    }
}

void CircuitViewWidget::load(const QString& filePath) {
    if (!fileManager) return;

    std::shared_ptr<ParsedCircuit> parsed = std::make_shared<ParsedCircuit>();
    if (!fileManager->loadFromFile(filePath, parsed)) {
        QMessageBox::warning(this, "Error", "Failed to load circuit file.");
        return;
    }

    circuitView.getToolManager().setPendingPreviewData(parsed);
    circuitView.getToolManager().changeTool("Preview Placement");
}

void CircuitViewWidget::dragEnterEvent(QDragEnterEvent* event) {
	// Accept the drag if it contains a file
	if (event->mimeData()->hasUrls()) {
		event->acceptProposedAction();
	}
}

void CircuitViewWidget::dropEvent(QDropEvent* event) {
	QPoint point = event->position().toPoint();
	if (insideWindow(point)) {
		Vec2 viewPos = pixelsToView(point);
		if (circuitView.getEventRegister().doEvent(PositionEvent("pointer enter view", circuitView.getViewManager().viewToGrid(viewPos)))) event->accept();

		// Get the list of URLs from the event
		const QList<QUrl> urls = event->mimeData()->urls();
		if (urls.isEmpty()) {
			return;
		}

		// Load the first file in the list
		QString filePath = urls.first().toLocalFile();
		if (filePath.isEmpty()) {
			QMessageBox::warning(this, "Error", "Invalid file path.");
			return;
		}

		load(filePath);
	}
}
