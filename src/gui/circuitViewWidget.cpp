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
#include <QWindow>
#include <QLayout>

#include "circuitView/circuitView.h"

CircuitViewWidget::CircuitViewWidget(QWidget* parent) : QWidget(parent), mouseControls(false), treeWidget(nullptr) {
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

	QShortcut* saveShortcut = new QShortcut(QKeySequence("Ctrl+S"), this);
	connect(saveShortcut, &QShortcut::activated, this, &CircuitViewWidget::save);
}

void CircuitViewWidget::showEvent(QShowEvent* event) {
	float w = size().width();
	float h = size().height();

	// initialize renderer with width and height
	circuitView.getRenderer().resize(w, h);
	
	// set viewmanager aspect ratio to begin with
	circuitView.getViewManager().setAspectRatio(w / h);
}

void CircuitViewWidget::createVulkanWindow(VulkanGraphicsView view, QVulkanInstance* qVulkanInstance) {
	QWindow* window = new QWindow();
	window->setSurfaceType(QSurface::VulkanSurface);
	window->setVulkanInstance(qVulkanInstance);
	window->show();
	QWidget* wrapper = QWidget::createWindowContainer(window, this);
	VkSurfaceKHR surface = QVulkanInstance::surfaceForWindow(window);

	circuitView.getRenderer().initialize(view, surface, size().width(), size().height());
}

void CircuitViewWidget::destroyVulkanWindow() {
	circuitView.getRenderer().destroy();
}

void CircuitViewWidget::updateLoop() {
	// update for re-render
	update();
}

// setter functions -----------------------------------------------------------------------------

void CircuitViewWidget::setSelector(QTreeWidget* treeWidget) {
	// disconnect the old tree
	if (this->treeWidget != nullptr)
		disconnect(this->treeWidget, &QTreeWidget::itemSelectionChanged, this, &CircuitViewWidget::updateSelectedItem);
	// connect the new tree
	this->treeWidget = treeWidget;
	connect(treeWidget, &QTreeWidget::itemSelectionChanged, this, &CircuitViewWidget::updateSelectedItem);
}

void CircuitViewWidget::updateSelectedItem() {
	if (treeWidget) {
		for (QTreeWidgetItem* item : treeWidget->selectedItems()) {
			if (item) {
				QString str = item->text(0);
				if (str == "And") circuitView.getToolManager().selectBlock(BlockType::AND);
				else if (str == "Or") circuitView.getToolManager().selectBlock(BlockType::OR);
				else if (str == "Xor") circuitView.getToolManager().selectBlock(BlockType::XOR);
				else if (str == "Nand") circuitView.getToolManager().selectBlock(BlockType::NAND);
				else if (str == "Nor") circuitView.getToolManager().selectBlock(BlockType::NOR);
				else if (str == "Xnor") circuitView.getToolManager().selectBlock(BlockType::XNOR);
				else if (str == "Switch") circuitView.getToolManager().selectBlock(BlockType::SWITCH);
				else if (str == "Button") circuitView.getToolManager().selectBlock(BlockType::BUTTON);
				else if (str == "Tick Button") circuitView.getToolManager().selectBlock(BlockType::TICK_BUTTON);
				else if (str == "Light") circuitView.getToolManager().selectBlock(BlockType::LIGHT);
				else {
					circuitView.getToolManager().changeTool(str.toStdString());
				}
			}
			return;
		}
	}
}

void CircuitViewWidget::setCircuit(std::shared_ptr<Circuit> circuit) {
	circuitView.setCircuit(circuit);
	updateSelectedItem();
}

void CircuitViewWidget::setEvaluator(std::shared_ptr<Evaluator> evaluator) {
	circuitView.setEvaluator(evaluator);
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

void saveJsonToFile(const QJsonObject& jsonObject);

void CircuitViewWidget::save() {
	// std::cout << "save" << std::endl;
	Circuit* circuit = circuitView.getCircuit();
	if (!circuit) return;
	Difference difference = circuit->getBlockContainer()->getCreationDifference();
	const auto modifications = difference.getModifications();
	QJsonObject modificationsJson;
	QJsonArray placeJson;
	QJsonArray connectJson;
	int centerX = 0;
	int centerY = 0;
	for (const auto& modification : modifications) {
		const auto& [modificationType, modificationData] = modification;
		switch (modificationType) {
		case Difference::PLACE_BLOCK:
		{
			QJsonObject placement;
			const auto& [position, rotation, blockType] = std::get<Difference::block_modification_t>(modificationData);
			centerX += position.x;
			centerY += position.y;
			placement["x"] = position.x;
			placement["y"] = position.y;
			placement["r"] = (char)rotation;
			placement["t"] = (char)blockType;
			placeJson.push_back(placement);
			break;
		}
		case Difference::CREATED_CONNECTION:
		{
			QJsonObject connection;
			const auto& [outputPosition, inputPosition] = std::get<Difference::connection_modification_t>(modificationData);
			connection["ox"] = outputPosition.x;
			connection["oy"] = outputPosition.y;
			connection["iy"] = inputPosition.y;
			connection["ix"] = inputPosition.x;
			connectJson.push_back(connection);
			break;
		}
		default:
			throw std::invalid_argument("save: invalid modificationType");
		}
	}
	modificationsJson["place"] = placeJson;
	modificationsJson["connect"] = connectJson;
	QJsonObject centerJson;
	centerX /= (int)(circuit->getBlockContainer()->getBlockCount());
	centerY /= (int)(circuit->getBlockContainer()->getBlockCount());
	centerJson["x"] = centerX;
	centerJson["y"] = centerY;
	modificationsJson["center"] = centerJson;
	saveJsonToFile(modificationsJson);
}

void saveJsonToFile(const QJsonObject& jsonObject) {
	// Convert JSON object to QJsonDocument
	QJsonDocument jsonDoc(jsonObject);

	// Open a save file dialog
	QString fileName = QFileDialog::getSaveFileName(nullptr, "Save JSON File", "", "JSON Files (*.json);;All Files (*)");

	if (fileName.isEmpty()) {
		QMessageBox::information(nullptr, "No File Selected", "No file was selected to save the JSON.");
		return;
	}

	QFile file(fileName);
	if (!file.open(QIODevice::WriteOnly)) {
		QMessageBox::critical(nullptr, "Error", "Could not open the file for writing.");
		return;
	}

	// Write JSON to the file
	file.write(jsonDoc.toJson());
	file.close();

	QMessageBox::information(nullptr, "Success", "JSON file saved successfully!");
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

void CircuitViewWidget::load(const QString& filePath) {
	// open file
	QFile file(filePath);
	if (!file.open(QIODevice::ReadOnly)) {
		qWarning("Couldn't open JSON file.");
		return;
	}
	QByteArray fileData = file.readAll();
	file.close();

	// validate data
	QJsonDocument jsonDoc = QJsonDocument::fromJson(fileData);
	if (!jsonDoc.isObject()) {
		qWarning("Invalid JSON format: Expected an object");
		return;
	}

	QJsonObject data = jsonDoc.object();
	if (!data["place"].isArray() || !data["connect"].isArray()) {
		qWarning("Invalid JSON format: Expected an object->[place/connect]array");
		return;
	}

	Position center = Position(0, 0);
	if (data["center"].isObject()) {
		QJsonObject centerJson = data["center"].toObject();
		if (!(centerJson["x"].isDouble() && centerJson["y"].isDouble())) {
			qWarning("Invalid JSON format: Expected an object->[center]object->'x','y'");
			return;
		}
		center = Position(centerJson["x"].toInt(), centerJson["y"].toInt());
	}
	Position pointer = circuitView.getViewManager().getPointerPosition().snap();

	Vector offset = pointer - center;

	// load container
	Circuit* container = circuitView.getCircuit();
	if (!container) return;

	// place blocks
	QJsonArray placeJson = data["place"].toArray();
	for (const QJsonValue& value : placeJson) {
		if (!value.isObject()) {
			qWarning("Invalid JSON format: Expected an object->[place]array->object");
			return;
		}
		QJsonObject placement = value.toObject();
		if (!(placement["x"].isDouble() && placement["y"].isDouble() && placement["r"].isDouble() && placement["t"].isDouble())) {
			qWarning("Invalid JSON format: Expected an object->[place]array->object->'x','y','r','t'");
			return;
		}
		container->tryInsertBlock(Position(placement["x"].toInt(), placement["y"].toInt()) + offset, (Rotation)(placement["r"].toInt()), (BlockType)(placement["t"].toInt()));
	}

	// connect blocks
	QJsonArray connectJson = data["connect"].toArray();
	for (const QJsonValue& value : connectJson) {
		if (!value.isObject()) {
			qWarning("Invalid JSON format: Expected an object->[connect]array->object.");
			return;
		}
		QJsonObject connection = value.toObject();
		if (!(connection["ox"].isDouble() && connection["oy"].isDouble() && connection["ix"].isDouble() && connection["iy"].isDouble())) {
			qWarning("Invalid JSON format: Expected an object->[connect]array->object->'ox','oy','ix','iy'");
			return;
		}
		container->tryCreateConnection(Position(connection["ox"].toInt(), connection["oy"].toInt()) + offset, Position(connection["ix"].toInt(), connection["iy"].toInt()) + offset);
	}

}
