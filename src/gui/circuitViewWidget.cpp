#include "backend/circuitView/tools/other/previewPlacementTool.h"
#include "backend/circuitView/circuitView.h"
#include "computerAPI/directoryManager.h"
#include "circuitViewWidget.h"
#include "backend/backend.h"

CircuitViewWidget::CircuitViewWidget(CircuitFileManager* fileManager, Rml::ElementDocument* document, Rml::Element* parent) : fileManager(fileManager), document(document), parent(parent) {
	// create circuitView
	renderer = std::make_unique<RendererTMP>();
	circuitView = std::make_unique<CircuitView>(renderer.get());


	// float w = size().width();
	// float h = size().height();

	// set viewmanager aspect ratio to begin with
	// circuitView->getViewManager().setAspectRatio(w / h);

	// initialize QTRenderer with width and height + tileset
	// renderer->resize(w, h);
	// renderer->initializeTileSet((DirectoryManager::getResourceDirectory() / "logicTiles.png").string());

	// create keybind shortcuts and connect them
	parent->AddEventListener("keydown", &keybindHandler);
	keybindHandler.addListener(
		Rml::Input::KeyIdentifier::KI_Z,
		Rml::Input::KeyModifier::KM_CTRL,
		[this]() { logInfo("undo"); if (circuitView->getCircuit()) circuitView->getCircuit()->undo(); }
	);
	keybindHandler.addListener(
		Rml::Input::KeyIdentifier::KI_Z,
		Rml::Input::KeyModifier::KM_CTRL + Rml::Input::KeyModifier::KM_SHIFT,
		[this]() { logInfo("redo"); if (circuitView->getCircuit()) circuitView->getCircuit()->redo(); }
	);
	keybindHandler.addListener(
		Rml::Input::KeyIdentifier::KI_S,
		Rml::Input::KeyModifier::KM_CTRL,
		[this]() { logInfo("save"); save(); }
	);
	keybindHandler.addListener(
		Rml::Input::KeyIdentifier::KI_C,
		Rml::Input::KeyModifier::KM_CTRL,
		[this]() { logInfo("Copy"); circuitView->getEventRegister().doEvent(Event("Copy")); }
	);
	keybindHandler.addListener(
		Rml::Input::KeyIdentifier::KI_Q,
		[this]() { logInfo("Tool Rotate Block CCW"); circuitView->getEventRegister().doEvent(Event("Tool Rotate Block CCW")); }
	);
	keybindHandler.addListener(
		Rml::Input::KeyIdentifier::KI_E,
		[this]() { logInfo("Tool Rotate Block CW"); circuitView->getEventRegister().doEvent(Event("Tool Rotate Block CW")); }
	);
	keybindHandler.addListener(
		Rml::Input::KeyIdentifier::KI_E,
		[this]() { logInfo("Confirm"); circuitView->getEventRegister().doEvent(Event("Confirm")); }
	);
	keybindHandler.addListener(
		Rml::Input::KeyIdentifier::KI_B,
		[this]() { logInfo("setupBlockData"); if (circuitView->getCircuit()) circuitView->getBackend()->getCircuitManager().setupBlockData(circuitView->getCircuit()->getCircuitId()); }
	);

	// connect buttons and actions
	// connect(ui->StartSim, &QPushButton::clicked, this, &CircuitViewWidget::setSimState);
	// connect(ui->UseSpeed, &QCheckBox::checkStateChanged, this, &CircuitViewWidget::simUseSpeed);
	// connect(ui->Speed, &QDoubleSpinBox::valueChanged, this, &CircuitViewWidget::setSimSpeed);

	// connect(circuitSelector, &QComboBox::currentIndexChanged, this, [&](int index) {
	// 	Backend* backend = this->circuitView->getBackend();
	// 	if (backend && this->circuitSelector) {
	// 		backend->linkCircuitViewWithCircuit(this->circuitView.get(), this->circuitSelector->itemData(index).value<int>());
	// 		logInfo("CircuitViewWidget linked to new circuit view: {}", "", this->circuitSelector->itemData(index).value<int>());
	// 	}
	// });
	// connect(ui->NewCircuitButton, &QToolButton::clicked, this, [&](bool pressed) {
	// 	Backend* backend = this->circuitView->getBackend();
	// 	if (backend) {
	// 		backend->createCircuit();
	// 	}
	// });
	// connect(evaluatorSelector, &QComboBox::currentIndexChanged, this, [&](int index) {
	// 	Backend* backend = this->circuitView->getBackend();
	// 	if (backend && this->evaluatorSelector) {
	// 		backend->linkCircuitViewWithEvaluator(this->circuitView.get(), this->evaluatorSelector->itemData(index).value<int>(), Address());
	// 		logInfo("CircuitViewWidget linked to evalutor: {}", "", this->evaluatorSelector->itemData(index).value<int>());
	// 	}
	// });
	// connect(ui->NewEvaluatorButton, &QToolButton::clicked, this, [&](bool pressed) {
	// 	Backend* backend = this->circuitView->getBackend();
	// 	if (backend && this->circuitView->getCircuit()) {
	// 		backend->createEvaluator(this->circuitView->getCircuit()->getCircuitId());
	// 	}
	// });
}

void CircuitViewWidget::setSimState(bool state) {
	if (circuitView->getEvaluator())
		circuitView->getEvaluator()->setPause(!state);
}

void CircuitViewWidget::simUseSpeed(bool state) {
	if (circuitView->getEvaluator())
		circuitView->getEvaluator()->setUseTickrate(state);
}

void CircuitViewWidget::setSimSpeed(double speed) {
	if (circuitView->getEvaluator())
		circuitView->getEvaluator()->setTickrate(std::round(speed * 60));
}

// save current circuit view widget we are viewing. Right now only works if it is the only widget in application.
// Called via Ctrl-S keybind
void CircuitViewWidget::save() {
	logInfo("Trying to save Circuit");
	if (fileManager && circuitView->getCircuit()) {
		circuit_id_t circuitId = circuitView->getCircuit()->getCircuitId();
		if (!fileManager->saveCircuit(circuitId)) {
			// if failed to save the circuit with out a path
			// QString filePath = QFileDialog::getSaveFileName(this, "Save Circuit", "", "Circuit Files (*.cir);;All Files (*)");
			// if (!filePath.isEmpty()) {
			// 	logWarning("This circuit "+ circuitView->getCircuit()->getCircuitName() +" will be saved with a new UUID");
			// 	fileManager->saveToFile(filePath.toStdString(), circuitId);
			// }
		}
	}
}

// for drag and drop load directly onto this circuit view widget
void CircuitViewWidget::load(const std::string& filePath) {
	if (!fileManager) return;

	if (!fileManager->loadFromFile(filePath)) {
		logError("Failed to load circuit file.");
		return;
	}
}
