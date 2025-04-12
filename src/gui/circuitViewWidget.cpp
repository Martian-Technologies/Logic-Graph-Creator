#include "backend/circuitView/tools/other/previewPlacementTool.h"
#include "backend/circuitView/circuitView.h"
#include "computerAPI/directoryManager.h"
#include "circuitViewWidget.h"
#include "backend/backend.h"
#include "interaction/eventPasser.h"

#include <SDL3/SDL.h>

void SaveCallback(void* userData, const char* const* filePaths, int filter) {
	CircuitViewWidget* circuitViewWidget = (CircuitViewWidget*)userData;
	if (filePaths && filePaths[0]) {
		std::cout << "Selected file(s):" << std::endl;
		std::string filePath = filePaths[0];
		if (!circuitViewWidget->getCircuitView()->getCircuit()) {
			logError("Circuit was null, could not save");
			return;
		}
		logWarning("This circuit " + circuitViewWidget->getCircuitView()->getCircuit()->getCircuitName() + " will be saved with a new UUID");
		circuitViewWidget->getFileManager()->saveToFile(filePath, circuitViewWidget->getCircuitView()->getCircuit()->getCircuitId());
	} else {
		std::cout << "File dialog canceled." << std::endl;
	}
}

void LoadCallback(void* userData, const char* const* filePaths, int filter) {
	CircuitViewWidget* circuitViewWidget = (CircuitViewWidget*)userData;
	if (filePaths && filePaths[0]) {
		std::cout << "Selected file(s):" << std::endl;
		std::string filePath = filePaths[0];
		circuit_id_t id = circuitViewWidget->getFileManager()->loadFromFile(filePath);
		if (id == 0) {
			logError("Error", "Failed to load circuit file.");
			return;
		}
		circuitViewWidget->getCircuitView()->getBackend()->linkCircuitViewWithCircuit(circuitViewWidget->getCircuitView(), id);
	} else {
		std::cout << "File dialog canceled." << std::endl;
	}
}

CircuitViewWidget::CircuitViewWidget(CircuitFileManager* fileManager, Rml::ElementDocument* document, Rml::Element* parent, SDL_Window* window, SDL_Renderer* sdlRenderer) : fileManager(fileManager), document(document), window(window), parent(parent) {
	// create circuitView
	renderer = std::make_unique<SdlRenderer>(sdlRenderer);
	circuitView = std::make_unique<CircuitView>(renderer.get());

	int w = parent->GetClientWidth();
	int h = parent->GetClientHeight();
	int x = parent->GetAbsoluteLeft() + parent->GetClientLeft();
	int y = parent->GetAbsoluteTop() + parent->GetClientTop();

	circuitView->getViewManager().setAspectRatio((float)w / (float)h);
	renderer->resize(w, h);
	renderer->reposition(x, y);
	renderer->initializeTileSet((DirectoryManager::getResourceDirectory() / "logicTiles.png").string());

	// create keybind shortcuts and connect them
	parent->AddEventListener(Rml::EventId::Keydown, &keybindHandler);
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
		Rml::Input::KeyIdentifier::KI_O,
		Rml::Input::KeyModifier::KM_CTRL,
		[this]() { logInfo("load"); load(); }
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
	keybindHandler.addListener(
		Rml::Input::KeyIdentifier::KI_R,
		[this]() {
			int w = this->parent->GetClientWidth();
			int h = this->parent->GetClientHeight();
			int x = this->parent->GetAbsoluteLeft() + this->parent->GetClientLeft();
			int y = this->parent->GetAbsoluteTop() + this->parent->GetClientTop();

			circuitView->getViewManager().setAspectRatio((float)w / (float)h);

			renderer->resize(w, h);
			renderer->reposition(x, y);
		}
	);

	parent->AddEventListener(Rml::EventId::Resize, new EventPasser(
		[this](Rml::Event& event) {
			int w = this->parent->GetClientWidth();
			int h = this->parent->GetClientHeight();
			int x = this->parent->GetAbsoluteLeft() + this->parent->GetClientLeft();
			int y = this->parent->GetAbsoluteTop() + this->parent->GetClientTop();

			circuitView->getViewManager().setAspectRatio((float)w / (float)h);

			renderer->resize(w, h);
			renderer->reposition(x, y);
		}
	));

	parent->AddEventListener(Rml::EventId::Mousedown, new EventPasser(
		[this](Rml::Event& event) {
			int button = event.GetParameter<int>("button", 0);
			if (button == 0) { // left
				const bool* state = SDL_GetKeyboardState(nullptr);
				if (state[SDL_SCANCODE_LALT] || state[SDL_SCANCODE_RALT]) {
					if (circuitView->getEventRegister().doEvent(PositionEvent("View Attach Anchor", circuitView->getViewManager().getPointerPosition()))) { event.StopPropagation(); return; }
				}
				if (circuitView->getEventRegister().doEvent(PositionEvent("Tool Primary Activate", circuitView->getViewManager().getPointerPosition()))) event.StopPropagation();
			} else if (button == 1) { // right
				if (circuitView->getEventRegister().doEvent(PositionEvent("Tool Secondary Activate", circuitView->getViewManager().getPointerPosition()))) event.StopPropagation();
			}
		}
	));

	Rml::Element* root = document->GetElementById("main-container");
	root->AddEventListener(Rml::EventId::Mouseup, new EventPasser(
		[this](Rml::Event& event) {
			int button = event.GetParameter<int>("button", 0);
			if (button == 0) { // left
				circuitView->getEventRegister().doEvent(PositionEvent("View Dettach Anchor", circuitView->getViewManager().getPointerPosition()));
				circuitView->getEventRegister().doEvent(PositionEvent("tool primary deactivate", circuitView->getViewManager().getPointerPosition()));
			} else if (button == 1) { // right
				circuitView->getEventRegister().doEvent(PositionEvent("tool secondary deactivate", circuitView->getViewManager().getPointerPosition()));
			}
		}
	));

	parent->AddEventListener(Rml::EventId::Mousemove, new EventPasser(
		[this](Rml::Event& event) {
			SDL_FPoint point(event.GetParameter<int>("mouse_x", 0), event.GetParameter<int>("mouse_y", 0));
			if (insideWindow(point)) { // inside the widget
				Vec2 viewPos = pixelsToView(point);
				circuitView->getEventRegister().doEvent(PositionEvent("Pointer Move", circuitView->getViewManager().viewToGrid(viewPos)));
			}
		}
	));

	parent->AddEventListener(Rml::EventId::Mouseover, new EventPasser(
		[this](Rml::Event& event) {
			// // grab focus so key inputs work without clicking
			// setFocus(Qt::MouseFocusReason);
			SDL_FPoint point(event.GetParameter<int>("mouse_x", 0), event.GetParameter<int>("mouse_y", 0));
			Vec2 viewPos = pixelsToView(point);
			if (viewPos.x < 0 || viewPos.y < 0 || viewPos.x > 1 || viewPos.y > 1) return;
			circuitView->getEventRegister().doEvent(PositionEvent("pointer enter view", circuitView->getViewManager().viewToGrid(viewPos)));
		}
	));

	parent->AddEventListener(Rml::EventId::Mouseout, new EventPasser(
		[this](Rml::Event& event) {
			SDL_FPoint point(event.GetParameter<int>("mouse_x", 0), event.GetParameter<int>("mouse_y", 0));
			Vec2 viewPos = pixelsToView(point);
			if (viewPos.x >= 0 && viewPos.y >= 0 && viewPos.x <= 1 && viewPos.y <= 1) return;
			circuitView->getEventRegister().doEvent(PositionEvent("pointer exit view", circuitView->getViewManager().viewToGrid(viewPos)));
		}
	));

	parent->AddEventListener(Rml::EventId::Mousescroll, new EventPasser(
		[this](Rml::Event& event) {
			SDL_FPoint delta(event.GetParameter<float>("wheel_delta_x", 0)*4, event.GetParameter<float>("wheel_delta_y", 0)*-4);
			// logInfo("{}, {}", "", delta.x, delta.y);
			if (mouseControls) {
				if (circuitView->getEventRegister().doEvent(DeltaEvent("view zoom", (float)(delta.y) / 200.f))) event.StopPropagation();
			} else {
				const bool* state = SDL_GetKeyboardState(nullptr);
				if (state[SDL_SCANCODE_LSHIFT] || state[SDL_SCANCODE_LSHIFT]) {
					// do zoom
					if (circuitView->getEventRegister().doEvent(DeltaEvent("view zoom", (float)(delta.y) / 100.f))) event.StopPropagation();
				} else {
					if (circuitView->getEventRegister().doEvent(DeltaXYEvent(
						"view pan",
						delta.x / getPixelsWidth() * circuitView->getViewManager().getViewWidth(),
						delta.y / getPixelsHeight() * circuitView->getViewManager().getViewHeight()
					))) event.StopPropagation();
				}
			}
		}
	));

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
			static SDL_DialogFileFilter filter;
			filter.name = "Circuit Files";
			filter.pattern = "*.cir";
			SDL_ShowSaveFileDialog(SaveCallback, this, window, &filter, 0, nullptr);
		}
	}
}

// for drag and drop load directly onto this circuit view widget
void CircuitViewWidget::load() {
	if (!fileManager) return;

	static SDL_DialogFileFilter filter[2];
	filter[0].name = "Circuit Files";
	filter[0].pattern = "*.cir";
	filter[1].name = "OpenCircuit Files";
	filter[1].pattern = "*.circiut";
	SDL_ShowOpenFileDialog(LoadCallback, this, window, filter, 0, nullptr, true);
}


inline Vec2 CircuitViewWidget::pixelsToView(const SDL_FPoint& point) const {
	return Vec2((point.x - getPixelsXPos()) / getPixelsWidth(), (point.y - getPixelsYPos()) / getPixelsHeight());
}

inline bool CircuitViewWidget::insideWindow(const SDL_FPoint& point) const {
	int x = point.x - getPixelsXPos();
	int y = point.y - getPixelsYPos();
	return x >= 0 && y >= 0 && x < getPixelsWidth() && y < getPixelsHeight();
}

inline float CircuitViewWidget::getPixelsWidth() const {
	return parent->GetClientWidth();
}

inline float CircuitViewWidget::getPixelsHeight() const {
	return parent->GetClientHeight();
}

inline float CircuitViewWidget::getPixelsXPos() const {
	return parent->GetAbsoluteLeft() + parent->GetClientLeft();
}

inline float CircuitViewWidget::getPixelsYPos() const {
	return parent->GetAbsoluteTop() + parent->GetClientTop();
}
