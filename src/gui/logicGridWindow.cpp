#include "logicGridWindow.h"

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

#include "blockContainerView/blockContainerView.h"

LogicGridWindow::LogicGridWindow(QWidget* parent) : QWidget(parent), mouseControls(true), treeWidget(nullptr) {
	// qt settings
	setFocusPolicy(Qt::StrongFocus);
	grabGesture(Qt::PinchGesture);
	setMouseTracking(true);
	setAcceptDrops(true);

	// set up update loop
	updateLoopTimer = new QTimer(this);
	updateLoopTimer->setInterval((int)(updateInterval * 1000.0f));
	updateLoopTimer->start();
	connect(updateLoopTimer, &QTimer::timeout, this, &LogicGridWindow::updateLoop);

	QShortcut* saveShortcut = new QShortcut(QKeySequence("Ctrl+S"), this);
	connect(saveShortcut, &QShortcut::activated, this, &LogicGridWindow::save);
}

void LogicGridWindow::showEvent(QShowEvent* event) {
	float w = size().width();
	float h = size().height();

	// initialize renderer with width and height
	blockContainerView.getRenderer().resize(w, h);
	
	// set viewmanager aspect ratio to begin with
	blockContainerView.getViewManager().setAspectRatio(w / h);
}

void LogicGridWindow::createVulkanWindow(VulkanGraphicsView view, QVulkanInstance* qVulkanInstance) {
	QWindow* window = new QWindow();
	window->setSurfaceType(QSurface::VulkanSurface);
	window->setVulkanInstance(qVulkanInstance);
	QWidget* wrapper = QWidget::createWindowContainer(window, this);
	VkSurfaceKHR surface = QVulkanInstance::surfaceForWindow(window);

	blockContainerView.getRenderer().initialize(view, surface);
}

void LogicGridWindow::updateLoop() {
	// update for re-render
	update();
}

// setter functions -----------------------------------------------------------------------------

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
				else if (str == "Light") blockContainerView.getToolManager().selectBlock(BlockType::LIGHT);
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

// input events ------------------------------------------------------------------------------

bool LogicGridWindow::event(QEvent* event) {
	if (event->type() == QEvent::NativeGesture) {
		QNativeGestureEvent* nge = dynamic_cast<QNativeGestureEvent*>(event);
		if (nge && nge->gestureType() == Qt::ZoomNativeGesture) {
			if (blockContainerView.getEventRegister().doEvent(DeltaEvent("view zoom", nge->value() - 1))) event->accept();
			return true;
		}
	} else if (event->type() == QEvent::Gesture) {
		QGestureEvent* gestureEvent = dynamic_cast<QGestureEvent*>(event);
		if (gestureEvent) {
			QPinchGesture* pinchGesture = dynamic_cast<QPinchGesture*>(gestureEvent->gesture(Qt::PinchGesture));
			if (blockContainerView.getEventRegister().doEvent(DeltaEvent("view zoom", pinchGesture->scaleFactor() - 1))) event->accept();

			return true;
		}
	}
	return QWidget::event(event);
}

void LogicGridWindow::paintEvent(QPaintEvent* event) {
	QPainter* painter = new QPainter(this);
	
	// rolling average for frame time
	pastFrameTimes.push_back(blockContainerView.getRenderer().getLastFrameTimeMs());
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
	stream2 << std::fixed << std::setprecision(3) << blockContainerView.getEvaluatorStateInterface().getRealTickrate();
	std::string tpsStr = "tps: " + stream2.str();
	painter->drawText(QRect(QPoint(0, 16), size()), Qt::AlignTop, QString(tpsStr.c_str()));

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
			if (blockContainerView.getEventRegister().doEvent(DeltaEvent("view zoom", (float)(numPixels.y()) / 200.f))) event->accept();
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

void LogicGridWindow::keyReleaseEvent(QKeyEvent* event) { }

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
	// grab focus so key inputs work without clicking
	setFocus(Qt::MouseFocusReason);

	Vec2 viewPos = pixelsToView(mapFromGlobal(QCursor::pos()));
	if (blockContainerView.getEventRegister().doEvent(PositionEvent("pointer enter view", blockContainerView.getViewManager().viewToGrid(viewPos)))) event->accept();
}

void LogicGridWindow::leaveEvent(QEvent* event) {
	Vec2 viewPos = pixelsToView(mapFromGlobal(QCursor::pos()));
	if (blockContainerView.getEventRegister().doEvent(PositionEvent("pointer exit view", blockContainerView.getViewManager().viewToGrid(viewPos)))) event->accept();
}

void saveJsonToFile(const QJsonObject& jsonObject);

void LogicGridWindow::save() {
	// std::cout << "save" << std::endl;
	BlockContainerWrapper* blockContainerWrapper = blockContainerView.getBlockContainer();
	if (!blockContainerWrapper) return;
	Difference difference = blockContainerWrapper->getBlockContainer()->getCreationDifference();
	const auto modifications = difference.getModifications();
	QJsonObject modificationsJson;
	QJsonArray placeJson;
	QJsonArray connectJson;
	Position center;
	for (const auto& modification : modifications) {
		const auto& [modificationType, modificationData] = modification;
		switch (modificationType) {
		case Difference::PLACE_BLOCK:
		{
			QJsonObject placement;
			const auto& [position, rotation, blockType] = std::get<Difference::block_modification_t>(modificationData);
			center += position;
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
	center /= blockContainerWrapper->getBlockContainer()->getBlockCount();
	centerJson["x"] = center.x;
	centerJson["y"] = center.y;
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

void LogicGridWindow::dragEnterEvent(QDragEnterEvent* event) {
	// Accept the drag if it contains a file
	if (event->mimeData()->hasUrls()) {
		event->acceptProposedAction();
	}
}

void LogicGridWindow::dropEvent(QDropEvent* event) {
	QPoint point = event->position().toPoint();
	if (insideWindow(point)) {
		Vec2 viewPos = pixelsToView(point);
		if (blockContainerView.getEventRegister().doEvent(PositionEvent("pointer enter view", blockContainerView.getViewManager().viewToGrid(viewPos)))) event->accept();

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

void LogicGridWindow::load(const QString& filePath) {
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
	Position pointer = blockContainerView.getViewManager().getPointerPosition().snap();

	Position offset = pointer - center;

	// load container
	BlockContainerWrapper* container = blockContainerView.getBlockContainer();
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
